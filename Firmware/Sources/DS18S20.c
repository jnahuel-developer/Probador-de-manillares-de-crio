/****************************************************************************************************/
/****************************************************************************************************/
/* 						LIBRERIA PARA MANEJAR LOS SENSORES DE TEMPERATURA 							*/
/****************************************************************************************************/
/****************************************************************************************************/

// Includes
#include "DS18S20.h"
#include "Timer.h"

// Variables para el algoritmo de deteccion de ROMS
unsigned char vectorColisiones[ LONGITUD_ROM_BYTES ];
unsigned char vectorBits[ LONGITUD_ROM_BYTES ];
unsigned char ROM_Leida[ LONGITUD_ROM_BYTES ];
unsigned char maximaColisionActual;
unsigned char maximaColisionPrevia;

// Variables globales para poder verlas en el debug
char indiceBitGeneral;
char indiceBit;
char indiceByte;
char auxiliarGlobal;

// Variable para almacenar la respuesta completa del sensor y luego extraer los datos de interes
unsigned char respuestaSensor[ LONGITUD_RESPUESTA ];

// Array de estructuras para manejar los senores. Para el probador de manillares, solo se pueden manejar 5 sensores
sensorDS18S20	sensores[ TOTAL_DE_SENSORES ];
sensorDS18S20	sensoresPostEnumeracion[ TOTAL_DE_SENSORES ];

// Variable para contemplar los codigos de error
erroresDS18S20  errorDS18S20;

// Contador para saber la cantidad de sensores que se encuentran conectados y registrados
char	totalDeSensoresPrevios;			// Borrar
//char 	totalDeSensoresEliminados;		// Borrar
//char 	totalDeSensoresAgregados;		// Borrar


// sensores[0] es el que esta soldado a la placa
// sensores[1] es el que va a la manguera
// sensores[2] es el que va al lado caliente del manillar
// sensores[3] es el que va al manillar o al zocalo externo para pruebas



/****************************************************************************************************/
/* 								Funciones de acceso general 										*/
/****************************************************************************************************/



/************************************************************/
/* EnviarResetDS18S20										*/
/*  														*/
/*  Manda la orden de reset. Si no hay pulsos de respuesta,	*/
/*  devuelve un indicador de error.							*/
/*  														*/
/*  Recibe: Nada											*/
/*  Devuelve:     											*/
/*	 - True : Se inicio el bus correctamente				*/
/*	 - False : No se detecto el pulso de respuesta			*/
/************************************************************/
bool EnviarResetDS18S20( void )
{
	// Variables auxiliares
	char intentos = 0;
	char pulsoRespuesta = 0;
	
	// Se define el pin como entrada para testear que este en alto. Si esta en bajo, se espera un tiempo hasta que cambie. Sino, se declara error
	DS18S20_DIRECCION = 0;

	// Se realizan 10 intentos espacidos 480 uSeg. Si hay problemas, el contador llegara a 10 intentos
	for( intentos = 0; intentos < TOTAL_DE_INTENTOS; intentos++ )
	{
		if( getRegBits( DS18S20_PUERTO, DS18S20_MASCARA ) > 0 )
			break;
		DEMORA_480US;
	}
	
	// Luego del for, si el contador esta en 10 intentos, se considera error porque la linea estubo baja durante 4.8 mili Segundos
	if( intentos == 10 )
		return( FALSE );
	
	// Una vez verificado que la linea este correcta, se manda el pulso de reset
	DS18S20_DIRECCION = 1;			// Se define el pin como salida
	DS18S20_PIN = 0;				// Se pone la linea en estado bajo
	DEMORA_480US;					// Se deben esperar 480uS
	
	// Se define el pin como entrada para que la resistencia de pull up eleve la linea
	DS18S20_DIRECCION = 0;			// Se define el pin como entrada
	DEMORA_70US;					// Se demora 10uS mas que el maximo segun la hoja de datos, por las dudas
	
	// Luego de la demora, se debe sensar la linea y tiene que estar baja producto del pulso de respuesta de los sensores
	pulsoRespuesta = getRegBits( DS18S20_PUERTO, DS18S20_MASCARA );
	DEMORA_410US;					// Se debe demorar hasta cumplir 480uS luego de finalizado el pulso de reset
	
	// Se deja la linea en alto, definiendo el pin como salida y poniendolo a 1
	DS18S20_PIN = 1;				// Se pone la linea en estado alto
	DS18S20_DIRECCION = 1;			// Se define el pin como salida
	
	// Por ultimo, si se leyo un 0 como respuesta, se devuelve TRUE. Caso contrario, hubo una falla y se devuelve FALSE 
	if( pulsoRespuesta == 0 )
		return( TRUE );
	else
		return( FALSE );
	
}



/************************************************************/
/* EnviarInstruccionDS18S20									*/
/*  														*/
/*  Verifica que la instruccion sea valida y envia el		*/
/*	codigo necesario para llevarla a cabo.					*/
/*  														*/
/*  Recibe: La instruccion a realizar						*/
/*  Devuelve: El codigo de error, si lo hay					*/
/************************************************************/
erroresDS18S20 EnviarInstruccionDS18S20( char instruccion )
{
	switch( instruccion )
	{
		case INSTRUCCION_DS18S20_INICAR_CONVERSION_T:
			escribirByteDS18S20( CODIGO_DS18S20_INICIAR_CONVERSION_T );
			break;
		case INSTRUCCION_DS18S20_USAR_ROM_ID:
			escribirByteDS18S20( CODIGO_DS18S20_IDENTIFICAR_ROM );
			break;
		case INSTRUCCION_DS18S20_NO_USAR_ROM_ID:
			escribirByteDS18S20( CODIGO_DS18S20_NO_IDENTIFICAR_ROM );
			break;
		case INSTRUCCION_DS18S20_LEER_TEMPERATURA:
			escribirByteDS18S20( CODIGO_DS18S20_LEER_TEMPERATURA );
			break;
		default:
			return( ORDEN_INCORRECTA );		// La unica forma de volver por aca, es que el comando suministrado no sea valido
	}
	
	return( SIN_ERROR );
}



/************************************************************/
/* LeerTemperaturaDS18S20									*/
/*  														*/
/*  Identifica al sensor solicitado y registra su valor de	*/
/*	temperatura.											*/
/*  														*/
/*  Recibe: La posicion del sensor a leer					*/
/*  Devuelve: El codigo de error, si lo hay					*/
/************************************************************/
erroresDS18S20 LeerTemperaturaDS18S20( posicionSensores sensor )
{
	// Se verifica que el parametro suministrado este dentro de los admisibles
	switch( sensor )
	{
		case SENSOR_SOLDADO_EN_PLACA:		// Cualquiera de los "case" prosigue despues del switch
		case SENSOR_PARA_LA_MANGUERA:
		case SENSOR_PARA_LADO_CALIENTE:
		case SENSOR_MANILLAR_O_EXTRA:
			break;
		default:
			return( SENSOR_INCORRECTO );	// El parametro suministrado no esta dentro de los valores aceptados. Se devuelve un error
	}
	
	// Primero se debe mandar el pulso de reset y detectar que haya sensores
	if( EnviarResetDS18S20() == FALSE )
		return( PULSO_RESET );
	
	// Se manda la orden para indicar que se identificara la ROM de un sensor en particular
	escribirByteDS18S20( CODIGO_DS18S20_IDENTIFICAR_ROM );
	
	// Se manda el valor de la ROM del sensor con el que se quiere comunicar
	escribirArrayDS18S20( sensores[sensor].ROM, LONGITUD_ROM_BYTES );
	
	// Se manda la orden para que ese sensor realice la conversion de temperatura
	escribirByteDS18S20( CODIGO_DS18S20_INICIAR_CONVERSION_T );
	
	// Se tiene que dejar la linea en alto y realizar una demora para que el sensor pueda realizar la conversion
	DS18S20_DIRECCION = 1;			// Se define el pin como salida
	DS18S20_PIN = 1;				// Se deja la linea en alto. Esto seria necesario solo para el modo parasito, pero por las dudas se lo usa igual
	
	// Se realiza una demora de mayor de 750mS, que es el maximo que podria demorar el sensor en realizar la conversion de temperatura
	DemoraParaConversionDS18S20();
	
	// Luego se debe mandar el pulso de reset y detectar que haya sensores
	if( EnviarResetDS18S20() == FALSE )
		return( PULSO_RESET );
	
	// Se manda la orden para indicar que se identificara la ROM de un sensor en particular
	escribirByteDS18S20( CODIGO_DS18S20_IDENTIFICAR_ROM );
	
	// Se manda el valor de la ROM del sensor con el que se quiere comunicar
	escribirArrayDS18S20( sensores[ sensor ].ROM, LONGITUD_ROM_BYTES );
	
	// Se manda la orden para indicar que se leera el valor de temperatura almacenado en el sensor
	escribirByteDS18S20( CODIGO_DS18S20_LEER_TEMPERATURA );
	
	// Se leen los 9 bytes del sensor
	leerArrayDS18S20();
	
	// Se extraen los bytes de la temperatura y se los almacena en la variable del sensor
	interpretarDatosSensor( sensor );
	
	return( SIN_ERROR );
	
}



/************************************************************/
/* LeerTodasLasTemperaturasDS18S20							*/
/*  														*/
/*  Envia la orden de conversion de temperatura a todos los */
/*	sensores conectados al bus y luego lee el valor de cada */
/*  uno														*/
/*  														*/
/*  Recibe: Nada											*/
/*  Devuelve: El codigo de error, si lo hay					*/
/************************************************************/
erroresDS18S20 LeerTodasLasTemperaturasDS18S20( void )
{
	// Variable auxiliar
	char	sensor;
	bool	errorSensoresProbador;
	
	// Primero se debe mandar el pulso de reset y detectar que haya sensores
	if( EnviarResetDS18S20() == FALSE )
		return( PULSO_RESET );
	
	// Se manda la orden para indicar que se no identificara la ROM 
	escribirByteDS18S20( CODIGO_DS18S20_NO_IDENTIFICAR_ROM );
	
	// Se manda la orden para que to-dos los sensores realicen la conversion de temperatura
	escribirByteDS18S20( CODIGO_DS18S20_INICIAR_CONVERSION_T );
	
	// Se tiene que dejar la linea en alto y realizar una demora para que los sensores puedan realizar la conversion
	DS18S20_DIRECCION = 1;			// Se define el pin como salida
	DS18S20_PIN = 1;				// Se deja la linea en alto. Esto seria necesario solo para el modo parasito, pero por las dudas se lo usa igual
	
	// Se realiza una demora de mayor de 750mS, que es el maximo que podria demorar el sensor en realizar la conversion de temperatura
	DemoraParaConversionDS18S20();
	
	// Se borra el flag
	errorSensoresProbador = FALSE;
	
	// Se leen todas las temperaturas mediante un loop
	for( sensor = 0; sensor < TOTAL_DE_SENSORES; sensor++ )
	{
		if( sensores[ sensor ].ROM[ BYTES_TIPO_DE_SENSOR ] != 0 )
		{
			// Luego se debe mandar el pulso de reset y detectar que haya sensores
			if( EnviarResetDS18S20() == FALSE )
				return( PULSO_RESET );
			
			// Se manda la orden para indicar que se identificara la ROM de un sensor en particular
			escribirByteDS18S20( CODIGO_DS18S20_IDENTIFICAR_ROM );
			
			// Se manda el valor de la ROM del sensor con el que se quiere comunicar
			escribirArrayDS18S20( sensores[ sensor ].ROM, LONGITUD_ROM_BYTES );
			
			// Se manda la orden para indicar que se leera el valor de temperatura almacenado en el sensor
			escribirByteDS18S20( CODIGO_DS18S20_LEER_TEMPERATURA );
			
			// Se leen los 9 bytes del sensor
			leerArrayDS18S20();
			
			// Se extraen los bytes de la temperatura y se los almacena en la variable del sensor
			interpretarDatosSensor( sensor );
			
			// El sensor soldado en la placa del probador no puedn registrar -0,5°C, o se lo considera un error
			if( sensor == SENSOR_SOLDADO_EN_PLACA )
			{
				if( sensores[ SENSOR_SOLDADO_EN_PLACA ].temperatura == -5 )
				{
					errorSensoresProbador = TRUE;
				}
			}
		}
	}
	
	if( errorSensoresProbador == FALSE )
		return( SIN_ERROR );
	
	return( TEMPERATURA_PROHIBIDA );
	
}



/************************************************************/
/* ActualizarListadoDeSensores								*/
/*  														*/
/*  Se enumeran los dispositivos y se revisa si se 			*/
/*	agregaron o quitaron respecto de la enumeracion			*/
/*	anterior.												*/
/*  														*/
/*  Recibe: Nada											*/
/*  Devuelve: False si hay algun problema					*/
/************************************************************/
bool ActualizarListadoDeSensores( void )
{
	// Variable auxiliar
	bool error;
	
	// Se enumeran los dispositivos con la disposicion actual
	if( enumerarDispositivos() == FALSE)
	{
		// Si hay una falla, se borran los datos
		borrarRegistrosSensores();
		return( FALSE );
	}
	
	// Se reordenan los sensores y se indica cuales estan presentes
	error = reordenarSensores();
	
	return( TRUE );
}









/****************************************************************************************************/
/* 						Funciones para la prueba reducida del manillar 								*/
/****************************************************************************************************/



/************************************************************/
/* InicializarSensoresProbador								*/
/*  														*/
/*  La direccion de los sensores esta definida de forma		*/
/*  fija con "#defines", ya que se los considera fijos en	*/
/*  la instalacion.											*/
/*  														*/
/*  Recibe:	Nada											*/
/*  Devuelve: Nada											*/
/************************************************************/
void InicializarSensoresProbador ( void )
{
	// Direccion del sensor soldado a la placa
	sensores[SENSOR_SOLDADO_EN_PLACA].ROM[ 0 ] = DS18S20_SOLDADO_BYTE_0;
	sensores[SENSOR_SOLDADO_EN_PLACA].ROM[ 1 ] = DS18S20_SOLDADO_BYTE_1;
	sensores[SENSOR_SOLDADO_EN_PLACA].ROM[ 2 ] = DS18S20_SOLDADO_BYTE_2;
	sensores[SENSOR_SOLDADO_EN_PLACA].ROM[ 3 ] = DS18S20_SOLDADO_BYTE_3;
	sensores[SENSOR_SOLDADO_EN_PLACA].ROM[ 4 ] = DS18S20_SOLDADO_BYTE_4;
	sensores[SENSOR_SOLDADO_EN_PLACA].ROM[ 5 ] = DS18S20_SOLDADO_BYTE_5;
	sensores[SENSOR_SOLDADO_EN_PLACA].ROM[ 6 ] = DS18S20_SOLDADO_BYTE_6;
	sensores[SENSOR_SOLDADO_EN_PLACA].ROM[ 7 ] = DS18S20_SOLDADO_BYTE_7;
	sensores[SENSOR_SOLDADO_EN_PLACA].usado = TRUE;

	// Direccion del sensor para la manguera
	sensores[SENSOR_PARA_LA_MANGUERA].ROM[ 0 ] = DS18S20_MANGUERA_BYTE_0;
	sensores[SENSOR_PARA_LA_MANGUERA].ROM[ 1 ] = DS18S20_MANGUERA_BYTE_1;
	sensores[SENSOR_PARA_LA_MANGUERA].ROM[ 2 ] = DS18S20_MANGUERA_BYTE_2;
	sensores[SENSOR_PARA_LA_MANGUERA].ROM[ 3 ] = DS18S20_MANGUERA_BYTE_3;
	sensores[SENSOR_PARA_LA_MANGUERA].ROM[ 4 ] = DS18S20_MANGUERA_BYTE_4;
	sensores[SENSOR_PARA_LA_MANGUERA].ROM[ 5 ] = DS18S20_MANGUERA_BYTE_5;
	sensores[SENSOR_PARA_LA_MANGUERA].ROM[ 6 ] = DS18S20_MANGUERA_BYTE_6;
	sensores[SENSOR_PARA_LA_MANGUERA].ROM[ 7 ] = DS18S20_MANGUERA_BYTE_7;
	sensores[SENSOR_PARA_LA_MANGUERA].usado = TRUE;

	// Direccion del sensor para el lado caliente del manillar
	sensores[SENSOR_PARA_LADO_CALIENTE].ROM[ 0 ] = DS18S20_LADO_CALIENTE_BYTE_0;
	sensores[SENSOR_PARA_LADO_CALIENTE].ROM[ 1 ] = DS18S20_LADO_CALIENTE_BYTE_1;
	sensores[SENSOR_PARA_LADO_CALIENTE].ROM[ 2 ] = DS18S20_LADO_CALIENTE_BYTE_2;
	sensores[SENSOR_PARA_LADO_CALIENTE].ROM[ 3 ] = DS18S20_LADO_CALIENTE_BYTE_3;
	sensores[SENSOR_PARA_LADO_CALIENTE].ROM[ 4 ] = DS18S20_LADO_CALIENTE_BYTE_4;
	sensores[SENSOR_PARA_LADO_CALIENTE].ROM[ 5 ] = DS18S20_LADO_CALIENTE_BYTE_5;
	sensores[SENSOR_PARA_LADO_CALIENTE].ROM[ 6 ] = DS18S20_LADO_CALIENTE_BYTE_6;
	sensores[SENSOR_PARA_LADO_CALIENTE].ROM[ 7 ] = DS18S20_LADO_CALIENTE_BYTE_7;
	sensores[SENSOR_PARA_LADO_CALIENTE].usado = TRUE;
	
}



/************************************************************/
/* TomarTemperaturaSensoresPruebaReducida					*/
/*  														*/
/*  Toma el valor de temperatura de los 2 sensores fijos en	*/
/*  la placa para la prueba reducida. Esto es para			*/
/*  informarlo al usuario y que pueda detectar alguna		*/
/*  anomalia												*/
/*  														*/
/*  Recibe:	Nada											*/
/*  Devuelve: 												*/
/*	 - 0: Estan todos los sensores bien						*/
/*	 - Algun numero: La ubicacion del sensor fallido + 1	*/
/************************************************************/
char TomarTemperaturaSensoresPruebaReducida( void )
{
	// Se verifica que se pueda leer la temperatura del sensor soldado en la placa
	if( LeerTemperaturaDS18S20( SENSOR_SOLDADO_EN_PLACA ) != SIN_ERROR )
		return( SENSOR_SOLDADO_EN_PLACA + 1 );
	
	// Luego se verifica que la temperatura inicial no sea de -0,5°C, porque indicaria un error. La temperatura se almacena * 10
	if( sensores[ SENSOR_SOLDADO_EN_PLACA ].temperatura == -5 )
		return( SENSOR_SOLDADO_EN_PLACA + 1 );

	
	// Se verifica que se pueda leer la temperatura del sensor para el lado caliente del manillar
	if( LeerTemperaturaDS18S20( SENSOR_PARA_LADO_CALIENTE ) != SIN_ERROR )
		return( SENSOR_PARA_LADO_CALIENTE + 1 );
	
	// Luego se verifica que la temperatura inicial no sea de -0,5°C, porque indicaria un error. La temperatura se almacena * 10
	if( sensores[ SENSOR_PARA_LADO_CALIENTE ].temperatura == -5 )
		return( SENSOR_PARA_LADO_CALIENTE + 1 );

	
	// Al llegar a este punto, los 2 sensores estan correctos
	return( 0 );
}





/****************************************************************************************************/
/* 								Funciones de acceso interno 										*/
/****************************************************************************************************/



/* ***  	FUNCIONES DE ESCRITURA DE DATOS		  *** */

/************************************************************/
/* escribirArrayDS18S20										*/
/*  														*/
/*  Escribe varios bytes sobre el bus de los sensores.		*/
/*  														*/
/*  Recibe:													*/
/*	 - *array: Los bytes a escribir							*/
/*	 - totalDeBytes: La cantidad de bytes a escribir		*/
/*  Devuelve: Nada											*/
/************************************************************/
void escribirArrayDS18S20 ( char *array, char totalDeBytes )
{
	// Variable auxiliar
	char indiceByte = 0;
	
	// Se define el pin como salida, ya que se deben escribir varios datos
	DS18S20_DIRECCION = 1;
	
	// Se escriben los bytes mediante un loop
	for( indiceByte = 0; indiceByte < totalDeBytes; indiceByte++ )
		escribirByteDS18S20( array[indiceByte] );
}



/************************************************************/
/* escribirByteDS18S20										*/
/*  														*/
/*  Escribe un byte sobre el bus de los sensores.			*/
/*  														*/
/*  Recibe: El byte a escribir								*/
/*  Devuelve: Nada											*/
/************************************************************/
void escribirByteDS18S20 ( char byte )
{
	// Variable auxiliar
	char indiceBit = 0;
	
	// Se escriben los bits mediante un loop
	for( indiceBit = 0; indiceBit < 8; indiceBit++ )
	{
		auxiliarGlobal = byte & ( 1 << indiceBit);
		escribirBitDS18S20( auxiliarGlobal  );
	}
}



/************************************************************/
/* escribirBitDS18S20										*/
/*  														*/
/*  Escribe un bit sobre el bus de los sensores.			*/
/*  														*/
/*  Recibe: El bit a escribir								*/
/*  Devuelve: Nada											*/
/************************************************************/
void escribirBitDS18S20 ( char bit )
{
	DS18S20_DIRECCION = 1;				// Se define el pin como salida
	DS18S20_PIN = 0;					// Se pone en bajo a la linea
	
	if( ( bit && 0x01 ) > 0 )
	{
		// Se debe escribir un 1. Para esto se debe bajar la linea entre 1uS y 15uS, y luego subirla hasta completar 60uS como minimo
		DEMORA_10US;					// Se demoran 10uS con la linea baja
		DS18S20_PIN = 1;				// Se pone en alto a la linea
		DEMORA_55US;					// Se demoran 55uS con la linea alta
	}
	else
	{
		// Se debe escribir un 0. Para esto se debe bajar la linea entre 60uS y 120uS
		DEMORA_65US;					// Se demoran 65uS con la linea baja
		DS18S20_PIN = 1;				// Se pone en alto a la linea
		DEMORA_1US;						// Se demoran 5uS con la linea alta
		DEMORA_1US;
		DEMORA_1US;
		DEMORA_1US;
		DEMORA_1US;
	}
}



/* ***  	FUNCIONES DE LECTURA DE DATOS	  *** */

/************************************************************/
/* leerArrayDS18S20											*/
/*  														*/
/*  Lee varios bytes en el bus de los sensores y los		*/
/*  almacena en la variable interna "respuestaSensor"		*/
/*  														*/
/*  Recibe:	Nada											*/
/*  Devuelve: Nada											*/
/************************************************************/
void leerArrayDS18S20 ( void )
{
	// Variable auxiliar
	char indiceByte = 0;
	
	// Se leen los bytes mediante un loop
	for( indiceByte = 0; indiceByte < LONGITUD_RESPUESTA; indiceByte++ )
		respuestaSensor[ indiceByte ] = leerByteDS18S20();
}



/************************************************************/
/* leerByteDS18S20											*/
/*  														*/
/*  Lee un byte en el bus de los sensores.					*/
/*  														*/
/*  Recibe: Nada											*/
/*  Devuelve: El valor del byte leido						*/
/************************************************************/
char leerByteDS18S20 ( void )
{
	// Variable auxiliar
	char indiceBit = 0;
	char auxiliar = 0;
	char byteLeido = 0;
	
	// Se leen los bits mediante un loop
	for( indiceBit = 0; indiceBit < 8; indiceBit++ )
	{
		auxiliar = leerBitDS18S20();
		auxiliar <<= indiceBit;
		byteLeido |= auxiliar;
	}
	
	// Se devuelve el byte leido
	return( byteLeido );
}



/************************************************************/
/* leerBitDS18S20											*/
/*  														*/
/*  Lee un bit en el bus de los sensores.					*/
/*  														*/
/*  Recibe: Nada											*/
/*  Devuelve: El valor del bit leido						*/
/************************************************************/
char leerBitDS18S20 ( void )
{
	// Variables auxiliares
	char bit = 0;
	
	// Se debe definir el pin como salida, poner en bajo la linea algo mas de 1uS, pasarlo a entrada y leer el estado de la linea luego de unos uS
	DS18S20_DIRECCION = 1;			// Se define el pin como salida
	DS18S20_PIN = 0;				// Se pone en bajo a la linea
	
	// Se demora 1uS para que los sensores tomen el pulso
	DEMORA_1US;						// Se demora 1uS con la linea baja
	
	// Se define el pin como entrada para que la resistencia de pull up eleve la linea
	DS18S20_DIRECCION = 0;			// Se define el pin como entrada
	DEMORA_10US;					// Se demoran 10uS para que la linea puede pasar a alto, de corresponder
	
	// Se lee el estado de la linea
	bit = getRegBits( DS18S20_PUERTO, DS18S20_MASCARA );
	
	// Se debe realizar una demora hasta cumplir los 60uS del slot de lectura mas unos uS de recuperacion
	DEMORA_55US;
	
	return( bit );
	
}



/************************************************************/
/* interpretarDatosSensor									*/
/*  														*/
/*  En el vector de respuesta del sensor se tienen todos	*/
/*  los bytes de informacion. Resta interpretar los			*/
/*  referentes a la temperatura y almacenarlos en la		*/
/*  estructura del sensor en cuestion.						*/
/*  														*/
/*  Recibe: El sensor al que corresponde el valor leido		*/
/*  Devuelve: Nada											*/
/************************************************************/
void interpretarDatosSensor( posicionSensores sensor )
{
	// Variable auxiliar
	char auxiliar;
	
	// Si el byte alto es 1, el valor es negativo
	if( respuestaSensor[ TEMPERATURA_BYTE_ALTO ] > 0 )
	{
		auxiliar = respuestaSensor[ TEMPERATURA_BYTE_BAJO ] - 1; 
		auxiliar = ~auxiliar;
		sensores[ sensor ].temperatura = auxiliar * 10;
		sensores[ sensor ].temperatura >>= 1;
		sensores[ sensor ].temperatura *= -1;
	}
	else
	{
		// Se multiplica el valor por 10 y luego se lo desplaza una posicion para dividirlo por 2
		sensores[ sensor ].temperatura = respuestaSensor[ TEMPERATURA_BYTE_BAJO ] * 10;
		sensores[ sensor ].temperatura >>= 1;
	}
	
}



/* ***  	FUNCIONES PARA IDENTIFICAR A LOS SENSORES	  *** */

/************************************************************/
/* enumerarDispositivos										*/
/*  														*/
/*  Realiza la identificacion de todos los sensores en el	*/
/*	bus y almacena sus valores de ROMS.						*/
/*  														*/
/*  Recibe: Nada											*/
/*  Devuelve: False si hay algun problema					*/
/************************************************************/
bool enumerarDispositivos( void )
{
	// Variables auxiliares
	char indice;
	erroresDS18S20  estadoLecturaROM;
	
	// Primero se resetean todos los contadores, que son 8 bytes, para tener 64 bits
	for( indice = 0; indice < LONGITUD_ROM_BYTES; indice++ )
	{
		vectorColisiones[indice] = 0;
		vectorBits[indice] = 0;
	}
	
	// Se resetean los demas contadores importantes
	maximaColisionActual = 0;
	maximaColisionPrevia = 0;
	
	// Despues se leen todas las ROMs de los dispositivos presentes en el bus
	for( indice = 0; indice < TOTAL_DE_SENSORES; )
	{
		// Primero se lee el valor de ROM de algun sensor en el bus
		estadoLecturaROM = leerROM_DS18S20();
		
		// Si la funcion indica que hay un problema en el pulso de presencia, se devuelve error. Aun si se leyeron cosas, se dan por invalidas
		if( estadoLecturaROM == PULSO_RESET )
			return( FALSE );
		
		// Se verifica si la ROM leida ya se encontraba en uso. Caso contrario, se la agrega al vector de sensores actuales
		if( verificarROM_EnUso() == FALSE )
		{
			agregarRom( indice );
			indice++;
		}
		
		// Se verifica si quedan mas ROMS por leer o ya se atendieron todas
		if( estadoLecturaROM == NO_MAS_SENSORES )
			return( TRUE );
	}
	
	// Si el loop no registro nada, se indica el error
	if( indice == 0 )
		return( FALSE );
	
}



/************************************************************/
/* leerROM_DS18S20											*/
/*  														*/
/*  Lee un codigo de ROM y lo almacena en "ROM_Leida"		*/
/*  														*/
/*  Recibe: Nada											*/
/*  Devuelve: Un codigo indicando el posible error			*/
/************************************************************/
erroresDS18S20 leerROM_DS18S20 ( void )
{
	// Variables auxiliares
	char bit;
	char bitComplementario;
	char auxiliar;

	// Se borra el contador
	maximaColisionActual = 0;
	
	// Se limpia el vector para almacenar la ROM leida
	for( indiceByte = 0; indiceByte < LONGITUD_ROM_BYTES; indiceByte++ )
		ROM_Leida[indiceByte] = 0;
	
	// Primero se debe mandar el pulso de reset y ver que los sensores respondan
	if( EnviarResetDS18S20() == FALSE )
		return( PULSO_RESET );
	
	// Si se detecto el pulso de respuesta en el bus, se debe enviar el comando para que los sensores respondan con sus ROMs 
	escribirByteDS18S20( CODIGO_DS18S20_LEER_ROMS );
	
	// Luego se inicia un loop para leer los 64 bits de cada ROM
	for( indiceBitGeneral = 0, indiceBit = 0, indiceByte = 0; indiceBitGeneral < LONGITUD_ROM_BITS; indiceBitGeneral++ )
	{
		// Se debe leer un bit y luego el complementario
		bit = leerBitDS18S20();
		bitComplementario = leerBitDS18S20();
		
		// En base a estos 2 bits, puede pasar lo siguiente:
			// 11: No hay sensores respondiendo en el bus. Es una falla
			// 10: Todos los sensores que esten respondiendo tienen un 1 en este bit de su ROM
			// 01: Todos los sensores que esten respondiendo tienen un 0 en este bit de su ROM
			// 00: Hay al menos 2 sensores respondiendo y tienen valores opuestos en este bit de su ROM
		
		// 11: No hay sensores respondiendo en el bus. Es una falla
		if( bit > 0 && bitComplementario > 0 )
			return( BUS_SIN_SENSORES );
		
		// 00: Hay al menos 2 sensores respondiendo y tienen valores opuestos en este bit de su ROM
		if( bit == 0 && bitComplementario == 0 )
		{
			// Se debe marcar que en esta posicion hay un conflicto, porque se deben tomar los 2 caminos para identificar a ambos.
			vectorColisiones[indiceByte] |= 0x01 << indiceBit;
			maximaColisionActual = indiceBitGeneral;
			bit = vectorBits[indiceByte] >> indiceBit;
			bit &= 0x01;
		}
		
		// Luego se debe escribir un bit sobre el bus para seleccionar que sensores deben seguir respondiendo
		escribirBitDS18S20( bit );
		ROM_Leida[indiceByte] |= bit << indiceBit;
		
		// Para poder llevar mas facil la cuenta de los indices, se calcula que bit de que byte es el contador de bits generales
		indiceBit = ( indiceBitGeneral + 1 ) % 8;
		if( indiceBit == 0 )
			indiceByte++;
	}
	
	// Solo interesa cuando la maxima colision actual es igual a la previa
	
	if( maximaColisionActual == maximaColisionPrevia )
	{
		indiceBit = maximaColisionActual % 8;
		indiceByte = maximaColisionActual / 8;
		bit = vectorBits[indiceByte] >> indiceBit;
		bit &= 0x01;
		
		// Si el bit del vector de bits es 0, entonces se volvio a leer la misma ROM de la vez anterior e indica que no hay mas colisiones
		// por este camino, con lo que se debe cambiar de rama poniendo en 1 ese bit
		if( bit == 0 )
		{
			vectorBits[indiceByte] |= 0x01 << indiceBit;
		}
		
		// Por el contrario, si el bit vale 1, se leyeron las 2 ramas, con lo que se debe borrar la colision actual y buscar
		// la colision previa que no haya sido atendida
		else
		{
			// Se debe borrar el bit para registrar que se atendio
			bit = 1 << indiceBit;
			bit = ~bit;
			vectorBits[indiceByte] &= bit;
			vectorColisiones[indiceByte] &= bit;
			
			// Bucle para buscar la colision previa
			indiceBitGeneral = indiceByte * 8 + indiceBit - 1;
			for( ; indiceBitGeneral > 0; indiceBitGeneral-- )
			{
				indiceBit = indiceBitGeneral % 8;
				indiceByte = indiceBitGeneral / 8;
				auxiliar = 0x01 << indiceBit;
				bit = vectorColisiones[indiceByte] & auxiliar;
				if( bit > 0 )
				{
					auxiliar = 0x01 << indiceBit;
					bit = vectorBits[indiceByte] & auxiliar;
					// Se debe revisar que la colision no haya sido atendida previamente
					if( bit > 0 )
					{
						// En este caso, ya se habia atendido, con lo que se lo debe borrar y buscar la anterior
						bit = 1 << indiceBit;
						bit = ~bit;
						vectorBits[indiceByte] &= bit;
						vectorColisiones[indiceByte] &= bit;
					}
					else
					{
						// Si se encontro una colision previa, pero el vector de bits estaba en cero, no habia sido atendida
						break;
					}
				}
			}
			
			// Se verifica si hay mas caminos para tomar o ya se atendieron todas las bifurcaciones
			if( indiceBitGeneral == 0 )
				return( NO_MAS_SENSORES );
			
			// En base a la colision previa se debe cambiar el camino tomado en este punto
			indiceBit = indiceBitGeneral % 8;
			indiceByte = indiceBitGeneral / 8;
			vectorBits[indiceByte] |= 0x01 << indiceBit;
			vectorColisiones[indiceByte] |= 0x01 << indiceBit;
			maximaColisionPrevia = indiceBitGeneral; 
			
		}
	}
	
	maximaColisionPrevia = maximaColisionActual;
	
	return( SIN_ERROR );
}



/************************************************************/
/* verificarROM_EnUso										*/
/*  														*/
/*  Verifica si el valor almacenado en la variable auxiliar	*/
/*  "ROM_Leida" ya estaba almacenado.						*/
/*  														*/
/*  Recibe: Nada											*/
/*  Devuelve: 												*/
/*	 - TRUE: El valor ya estaba almacenado					*/
/*	 - FALSE: Es un valor no registrado anteriormente		*/
/************************************************************/
bool verificarROM_EnUso ( void )
{
	// Variables auxiliares
	char indiceSensor;
	char indiceByte;
	
	// Mediante un loop se recorren las posiciones del vector de sensores
	for( indiceSensor = 0; indiceSensor < TOTAL_DE_SENSORES; indiceSensor++ )
	{
		// Con otro loop se recorren todos los bytes de cada ROM
		for( indiceByte = 0; indiceByte < LONGITUD_ROM_BYTES; indiceByte++ )
		{
			if( sensoresPostEnumeracion[ indiceSensor ].ROM[ indiceByte ] != ROM_Leida[ indiceByte ] )
				break;
		}
		
		// Si alguno de los bytes es distinto, el loop se corta antes de llegar al maximo valor
		if( indiceByte == LONGITUD_ROM_BYTES )
			return( TRUE );		// Para entrar a este if, tienen que coincidir los 8 bytes de la ROM leida con alguna almacenada
	}
	
	// Si llego aca, es porque la ROM leida no coincide con ninguna de las almacenadas
	return( FALSE );
}



/************************************************************/
/* agregarRom												*/
/*  														*/
/*  Almacena en el vector de sensores, el valor de la		*/
/*  variable auxiliar "ROM_Leida" en la posicion indicada.	*/
/*  														*/
/*  Recibe: La ubicacion del sensor en el vector			*/
/*  Devuelve: Nada											*/
/************************************************************/
void agregarRom( posicionSensores indiceSensor )
{
	// Variable auxiliar
	char indiceBytes;
	
	// Mediante un loop se agregan los bytes
	for( indiceBytes = 0; indiceBytes < LONGITUD_ROM_BYTES; indiceBytes++ )
		sensoresPostEnumeracion[ indiceSensor ].ROM[ indiceBytes ] = ROM_Leida[ indiceBytes ];
}



/************************************************************/
/* reordenarSensores										*/
/*  														*/
/*  Reorganiza los sensores que se hayan listado luego de	*/
/*  una enumeracion mediante "enumerarDispositivos".		*/
/*  Primero se revisa que esten los sensores propios del	*/
/*  probador, asumiendo que el sensor restante es el		*/
/*  externo.												*/
/*  														*/
/*  Recibe: Nada											*/
/*  Devuelve: Nada											*/
/************************************************************/
bool reordenarSensores( void )
{
	// Variables auxiliares
	char	indiceSensorPost;
	char	indiceByteROM;
	bool	error;
	
	// Primero se revisa que esten los sensores propios del probador
	error = FALSE;		// Se resetea el indicador de errores
	
	
	// Sensor soldado en la placa
	sensores[ SENSOR_SOLDADO_EN_PLACA ].usado = FALSE;		// Se borra el indicador, que sera renovado si se lo encuentra
	for( indiceSensorPost = 0; indiceSensorPost < TOTAL_DE_SENSORES; indiceSensorPost++ )
	{
		// Si los datos de la ubicacion no se corresponden con algo valido, no se los toma en cuenta
		if( sensoresPostEnumeracion[ indiceSensorPost ].ROM[ BYTES_TIPO_DE_SENSOR ] == 0x10 )
		{
			// Se deben comparar todos los bytes de la ROM
			for( indiceByteROM = 0; indiceByteROM < LONGITUD_ROM_BYTES; indiceByteROM++ )
			{
				// Si algun byte difiere, se corta el loop
				if( sensoresPostEnumeracion[ indiceSensorPost ].ROM[ indiceByteROM ] != sensores[ SENSOR_SOLDADO_EN_PLACA ].ROM[ indiceByteROM ] )
					break;
			}
			
			// Si todos los bytes coindicen, el sensor ya estaba registrado, con lo que simplemente se lo borra de los post enumeracion
			if( indiceByteROM == LONGITUD_ROM_BYTES )
			{
				for( indiceByteROM = 0; indiceByteROM < LONGITUD_ROM_BYTES; indiceByteROM++ )
					sensoresPostEnumeracion[ indiceSensorPost ].ROM[ indiceByteROM ] = 0;
				
				// Luego de borrarlo, se corta el loop para dejar de comparar respecto de los demas sensores
				sensores[ SENSOR_SOLDADO_EN_PLACA ].usado = TRUE;
				break;
			}
		}
	}
	
	// Se revisa que el loop haya terminado anticipadamente. Sino, no se encontro el sensor soldado en la placa
	if( indiceSensorPost == TOTAL_DE_SENSORES )
		error = TRUE;
	
	
	
	// Sensor para el lado caliente
	sensores[ SENSOR_PARA_LADO_CALIENTE ].usado = FALSE;		// Se borra el indicador, que sera renovado si se lo encuentra
	for( indiceSensorPost = 0; indiceSensorPost < TOTAL_DE_SENSORES; indiceSensorPost++ )
	{
		// Si los datos de la ubicacion no se corresponden con algo valido, no se los toma en cuenta
		if( sensoresPostEnumeracion[ indiceSensorPost ].ROM[ BYTES_TIPO_DE_SENSOR ] == 0x10 )
		{
			// Se deben comparar todos los bytes de la ROM
			for( indiceByteROM = 0; indiceByteROM < LONGITUD_ROM_BYTES; indiceByteROM++ )
			{
				// Si algun byte difiere, se corta el loop
				if( sensoresPostEnumeracion[ indiceSensorPost ].ROM[ indiceByteROM ] != sensores[ SENSOR_PARA_LADO_CALIENTE ].ROM[ indiceByteROM ] )
					break;
			}
			
			// Si todos los bytes coindicen, el sensor ya estaba registrado, con lo que simplemente se lo borra de los post enumeracion
			if( indiceByteROM == LONGITUD_ROM_BYTES )
			{
				for( indiceByteROM = 0; indiceByteROM < LONGITUD_ROM_BYTES; indiceByteROM++ )
					sensoresPostEnumeracion[ indiceSensorPost ].ROM[ indiceByteROM ] = 0;
				
				// Luego de borrarlo, se corta el loop para dejar de comparar respecto de los demas sensores
				sensores[ SENSOR_PARA_LADO_CALIENTE ].usado = TRUE;
				break;
			}
		}
	}
	
	// Se revisa que el loop haya terminado anticipadamente. Sino, no se encontro el sensor para el lado caliente
	if( indiceSensorPost == TOTAL_DE_SENSORES )
		error = TRUE;

	
	
	
	// Sensor para la manguera
	sensores[ SENSOR_PARA_LA_MANGUERA ].usado = FALSE;		// Se borra el indicador, que sera renovado si se lo encuentra
	for( indiceSensorPost = 0; indiceSensorPost < TOTAL_DE_SENSORES; indiceSensorPost++ )
	{
		// Si los datos de la ubicacion no se corresponden con algo valido, no se los toma en cuenta
		if( sensoresPostEnumeracion[ indiceSensorPost ].ROM[ BYTES_TIPO_DE_SENSOR ] == 0x10 )
		{
			// Se deben comparar todos los bytes de la ROM
			for( indiceByteROM = 0; indiceByteROM < LONGITUD_ROM_BYTES; indiceByteROM++ )
			{
				// Si algun byte difiere, se corta el loop
				if( sensoresPostEnumeracion[ indiceSensorPost ].ROM[ indiceByteROM ] != sensores[ SENSOR_PARA_LA_MANGUERA ].ROM[ indiceByteROM ] )
					break;
			}
			
			// Si todos los bytes coindicen, el sensor ya estaba registrado, con lo que simplemente se lo borra de los post enumeracion
			if( indiceByteROM == LONGITUD_ROM_BYTES )
			{
				for( indiceByteROM = 0; indiceByteROM < LONGITUD_ROM_BYTES; indiceByteROM++ )
					sensoresPostEnumeracion[ indiceSensorPost ].ROM[ indiceByteROM ] = 0;
				
				// Luego de borrarlo, se corta el loop para dejar de comparar respecto de los demas sensores
				sensores[ SENSOR_PARA_LA_MANGUERA ].usado = TRUE;
				break;
			}
		}
	}
	
	// Se revisa que el loop haya terminado anticipadamente. Sino, no se encontro el sensor para el lado caliente
	if( indiceSensorPost == TOTAL_DE_SENSORES )
		error = TRUE;
	
	
	
	// Finalmente, el sensor que quede en la post enumeracion es el sensor externo
	sensores[ SENSOR_MANILLAR_O_EXTRA ].usado = FALSE;		// Se borra el indicador, que sera renovado si se lo encuentra
	for( indiceSensorPost = 0; indiceSensorPost < TOTAL_DE_SENSORES; indiceSensorPost++ )
	{
		// Si los datos de la ubicacion se corresponden con el de un sensor utilizado, se lo almacena
		if( sensoresPostEnumeracion[ indiceSensorPost ].ROM[ BYTES_TIPO_DE_SENSOR ] == 0x10 )
		{
			// Se deben almacenar todos los bytes de la ROM
			for( indiceByteROM = 0; indiceByteROM < LONGITUD_ROM_BYTES; indiceByteROM++ )
			{
				sensores[ SENSOR_MANILLAR_O_EXTRA ].ROM[ indiceByteROM ] = sensoresPostEnumeracion[ indiceSensorPost ].ROM[ indiceByteROM ];
			}
			
			// Luego de almacenar el dato del nuevo sensor, se termina la funcion
			sensores[ SENSOR_MANILLAR_O_EXTRA ].usado = TRUE;
			break;
		}
	}
	
	// Si llega hasta aca, es porque no se registro un sensor externo
	if( indiceSensorPost == TOTAL_DE_SENSORES )
		error = TRUE;
	
	// Se borran los datos ya utilizados
	for( indiceSensorPost = 0; indiceSensorPost < TOTAL_DE_SENSORES; indiceSensorPost++ )
	{
		// Se deben almacenar todos los bytes de la ROM
		for( indiceByteROM = 0; indiceByteROM < LONGITUD_ROM_BYTES; indiceByteROM++ )
			{	sensoresPostEnumeracion[ indiceSensorPost ].ROM[ indiceByteROM ] = 0;	}
		
		sensoresPostEnumeracion[ indiceSensorPost ].usado = FALSE;
		sensoresPostEnumeracion[ indiceSensorPost ].temperatura = 0;
	}
	
	return( error );
}



/************************************************************/
/* borrarRegistrosSensores									*/
/*  														*/
/*  Borra los datos registrados de los sensores. Se usa		*/
/*  para volver a cero los contadores ante una falla.		*/
/*  														*/
/*  Recibe: Nada											*/
/*  Devuelve: Nada											*/
/************************************************************/
void borrarRegistrosSensores( void )
{
	// Variables auxiliares
	char indiceSensor;
	char indiceByte;
	
	for( indiceSensor = 0; indiceSensor < TOTAL_DE_SENSORES; indiceSensor++ )
	{
		// Se borran los datos de las ROMS
		for( indiceByte = 0; indiceByte < LONGITUD_ROM_BYTES; indiceByte++ )
		{
			sensores[ indiceSensor ].ROM[ indiceByte ] = 0;
			sensoresPostEnumeracion[ indiceSensor ].ROM[ indiceByte ] = 0;
		}
		
		// Se borra el dato de la temperatura
		sensores[ indiceSensor ].temperatura = 0;
		
		// Se marca como que estan libres
		sensores[ indiceSensor ].usado = FALSE;
	}
}













/****************************************************************************************************/
/* 							Funciones auxiliares solo para pruebas rapidas							*/
/****************************************************************************************************/



/************************************************************/
/* leerUnaROM												*/
/*  														*/
/*  Funcion auxiliar para leer la ROM cuando hay conectado	*/
/*  solo un sensor. Reescribe el bit leido sin revisar si	*/
/*  hay fallas o no.										*/
/*  														*/
/*  Recibe: Nada											*/
/*  Devuelve: Nada											*/
/************************************************************/
bool leerUnaROM ( void )
{
	// Variables auxiliares
	char bit;
	char bitComplementario;

	// Se limpia el vector para almacenar la ROM leida
	for( indiceByte = 0; indiceByte < LONGITUD_ROM_BYTES; indiceByte++ )
		ROM_Leida[ indiceByte ] = 0;
	
	// Primero se debe mandar el pulso de reset y ver que los sensores respondan
	if( EnviarResetDS18S20() == FALSE )
		return( PULSO_RESET );
	
	DS18S20_PIN = 1;				// Se pone la linea en estado alto
	
	// Si se detecto el pulso de respuesta en el bus, se debe enviar el comando para que los sensores respondan con sus ROMs 
	escribirByteDS18S20( CODIGO_DS18S20_LEER_ROMS );
	
	// Luego se inicia un loop para leer los 64 bits de cada ROM
	for( indiceBitGeneral = 0, indiceBit = 0, indiceByte = 0; indiceBitGeneral < LONGITUD_ROM_BITS; indiceBitGeneral++ )
	{
		// Se debe leer un bit y luego el complementario
		bit = leerBitDS18S20();
		bitComplementario = leerBitDS18S20();
		
		// Luego se debe escribir un bit sobre el bus para seleccionar que sensores deben seguir respondiendo
		escribirBitDS18S20( bit );
		ROM_Leida[indiceByte] |= bit << indiceBit;
		
		// Para poder llevar mas facil la cuenta de los indices, se calcula que bit de que byte es el contador de bits generales
		indiceBit = ( indiceBitGeneral + 1 ) % 8;
		if( indiceBit == 0 )
			indiceByte++;
	}
}




