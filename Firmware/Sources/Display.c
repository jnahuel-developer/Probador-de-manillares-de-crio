/****************************************************************************************************/
/****************************************************************************************************/
/* 								LIBRERIA PARA MANEJAR LOS LCD 										*/
/****************************************************************************************************/
/****************************************************************************************************/

// Includes
#include "Display.h"
#include "Timer.h"


LCD_Objeto LCD;

// Variable auxiliar para indicar algun error, pero no se contempla su uso todavia
bool error;


/****************************************************************************************************/
/* 								Funciones de acceso general 										*/
/****************************************************************************************************/



/****************************************************************/
/* SeleccionarLCD												*/
/*  															*/
/*  Permite configurar el tipo de LCD que se usara				*/
/*  															*/
/*  Recibe: 													*/
/*	 - totalDeFilas: Cuantas filas tiene el LCD					*/
/*	 - totalDeCaracteres: Cuantos caracteres por fila			*/
/*	 - interfaz_4bits: Para manejarlo con un bus de 4 u 8 bits	*/
/*	 - puertoExterno: Si se usa la tira de pines de 16 para un  */
/*  		LCD esterno a la placa o la tira de 14 con el LCD	*/
/*  		sobre la placa										*/
/*	 - probador: Si se usa la configuracion de pines del		*/
/*			probador o del mango de vacio						*/
/*  Devuelve: False, si hay algun parametro mal					*/
/****************************************************************/
bool SeleccionarLCD( char totalDeFilas, char totalDeCaracteres, bool fuente, bool interfaz_4bits, bool puertoExterno, bool probador )
{
	// Se verifica que los parametros esten dentro de los valores correctos
	switch( totalDeFilas )
	{
		case LCD_2_FILAS:
		case LCD_4_FILAS:
			LCD.filas = totalDeFilas;
			break;
		default:
			return( FALSE );
	}
	
	switch( totalDeCaracteres )
	{
		case LCD_20_CARACTERES:
		case LCD_16_CARACTERES:
		case LCD_8_CARACTERES:
			LCD.caracteres = totalDeCaracteres;
			break;
		default:
			return( FALSE );
	}
	
	switch( fuente )
	{
		case LCD_FUENTE_CHICA:
		case LCD_FUENTE_GRANDE:
			LCD.filas = totalDeFilas;
			break;
		default:
			return( FALSE );
	}
	
	switch( interfaz_4bits )
	{
		case LCD_8_BITS:
		case LCD_4_BITS:
			LCD.interfaz = interfaz_4bits;
			break;
		default:
			return( FALSE );
	}
	
	switch( puertoExterno )
	{
		case LCD_PUERTO_EXTERNO:
		case LCD_PUERTO_INTERNO:
			LCD.puerto = puertoExterno;
			break;
		default:
			return( FALSE );
	}
	
	switch( probador )
	{
		case LCD_MANGO_VACIO:
		case LCD_PROBADOR:
			LCD.probador = probador;
			break;
		default:
			return( FALSE );
	}
	
	// Si llega aca, es porque todos los parametros estan correctos
	return( TRUE );
}



/****************************************************************/
/* InicializarLCD												*/
/*  															*/
/*  Muestra la mitad de la ROM del sensor en el LCD				*/
/*  															*/
/*  Recibe: La ROM a mostrar y que parte						*/
/*  Devuelve: Nada												*/
/****************************************************************/
void InicializarLCD( void )
{
	// Primero se revisa el tipo de interfaz que se utilice
	if( LCD.interfaz == LCD_4_BITS )
		inicializarLCD_4bits();
	
	// Resta hacer la inicializacion para manejarlo con un bus de 8 bits, pero no se va a usar en los probadores. Queda por posibles futuras mejoras
}



/****************************************************************/
/* MostrarMensajeLCD											*/
/*  															*/
/*  Muestra algun mensaje predefinido							*/
/*  															*/
/*  Recibe: Un indicador del tipo de mensaje					*/
/*  Devuelve: Nada												*/
/****************************************************************/
void MostrarMensajeLCD( char tipoDeMensaje, char parametroExtra )
{
	switch( tipoDeMensaje )
	{
		case INICIADO:
			mensajeLCD_Iniciado();
			break;
		case SIN_SENSORES:
			mensajeLCD_SinSensores();
			break;
		case BUSCANDO:
			mensajeLCD_BuscandoSensores();
			break;
		case SENSORES_ENCONTRADOS:
			mensajeLCD_SensoresEncontrados(parametroExtra);
			break;
		case TEXTO_FUNCION:
			mensajeLCD_FuncionDelSensor(parametroExtra);
			return;
	}
	
	// Los casos que terminen el "switch" por "break", tienen una demora de 1 segundo
	DemoraEnSegundos( 1 );
	
}



/****************************************************************/
/* EscribirMensajeLCD											*/
/*  															*/
/*  Imprime en el LCD el texto suministrado, en el renglon 		*/
/*  solicitado y con el offset requerido.						*/
/*  															*/
/*  Recibe: 													*/
/*	 - renglon: Para indicar el numero del renglon, siendo 1 el */
/*  		superior y numerando en orden cresciente los demas  */
/*   - inicioTexto: En que posicion del renglo empieza el texto */
/*   - totalDeCaracteres: Cuantos se van a imprimir del total	*/
/*  		del texto suministrado								*/
/*   - *pTexto: Un puntero al vector donde se almacena el		*/
/*  		mensaje a mostrar									*/
/*  Devuelve: 													*/
/*	 - True: Todos los parametros estaba bien y no hubo			*/
/*			problemas											*/
/*	 - False: Algun parametro estaba mal						*/
/****************************************************************/
bool EscribirMensajeLCD( unsigned char renglon, unsigned char inicioTexto, unsigned char totalDeCaracteres, unsigned char *pTexto)
{
	// Variables auxiliares
	char caracter = 0;
	char instruccion;
	
	// Se verifica que el parametro no este fuera de rango
	if( totalDeCaracteres == 0 )
		return( FALSE );
	
	// Se manda la orden para posicionarse segun el renglon que se vaya a utilizar
	switch( renglon )
	{
		case RENGLON_SUPERIOR:
			instruccion = INSTRUCCION_LCD_RENGLON_SUPERIOR;
			break;
		case RENGLON_MEDIO_SUPERIOR:
			instruccion = INSTRUCCION_LCD_RENGLON_MEDIO_SUPERIOR;
			break;
		case RENGLON_MEDIO_INFERIOR:
			instruccion = INSTRUCCION_LCD_RENGLON_MEDIO_INFERIOR;
			break;
		case RENGLON_INFERIOR:
			instruccion = INSTRUCCION_LCD_RENGLON_INFERIOR;
			break;
		
		default:
			return( FALSE );
	}
	
	// Se le agrega el inicio del cursor
	if( inicioTexto < LCD.caracteres )
		instruccion += inicioTexto;
	
	// Se envia la instruccion para posicionar el cursor
	enviarInstruccion( instruccion );
	
	// Por las dudas, se recorta el mensaje si es que se pasa del total de caracteres
	if( ( inicioTexto + totalDeCaracteres ) > LCD.caracteres )
		totalDeCaracteres = LCD.caracteres - inicioTexto;
	
	// Se envia el mensaje
	for( caracter = 0; caracter < totalDeCaracteres; caracter++ )
		enviarDato( pTexto[ caracter ] );
	
}



/****************************************************************/
/* EscribirTemperatura											*/
/*  															*/
/*  Muestra la temperatura suministrada en el LCD				*/
/*  															*/
/*  Recibe: La temperatura a mostrar							*/
/*  Devuelve: Nada												*/
/****************************************************************/
void EscribirTemperatura( int16_t temperatura, char reglon, char offset )
{
	// Variables auxiliares
	unsigned char 	digito = 0;
	unsigned char 	pTexto[6];
	
	// Se toma el signo
	if( temperatura >= 0 )
		pTexto[0] = ' ';
	else
	{
		temperatura *= -1;
		pTexto[0] = '-';
	}
	
	// Se toman los decimales
	digito = temperatura % 10;
	pTexto[4] = digito + '0';
	
	// El valor suministrado esta multiplicado por 10
	temperatura /= 10;
	
	// Se toman las unidades
	digito = temperatura % 10;
	pTexto[2] = digito + '0';
	
	// Se toman las decenas
	if( temperatura >= 10 )
	{
		digito = temperatura % 100;
		digito /= 10;
		pTexto[1] = digito + '0';
	}
	else
		pTexto[1] = ' ';
	
	// Se coloca el .
	pTexto[3] = '.';
	
	// Se coloca la unidad
	pTexto[5] = 'C';
	
	// Se imprime el valor
	if( LCD.filas == LCD_2_FILAS )
	{
		borrarRenglon( RENGLON_MEDIO_SUPERIOR );
		error = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 6, pTexto );
	}
	else
		error = EscribirMensajeLCD( reglon, offset, 6, pTexto );
}



/****************************************************************/
/* EscribirVariacionesDeTemperatura								*/
/*  															*/
/*  Muestra la temperatura suministrada en el LCD				*/
/*  															*/
/*  Recibe: La temperatura a mostrar							*/
/*  Devuelve: Nada												*/
/****************************************************************/
void EscribirVariacionesDeTemperatura( int16_t variacionDeTemperatura, char reglon, char offset )
{
	// Variables auxiliares
	unsigned char 	digito = 0;
	unsigned char 	pTexto[6];
	
	// Se toma el signo
	if( variacionDeTemperatura >= 0 )
		pTexto[0] = ' ';
	else
	{
		variacionDeTemperatura *= -1;
		pTexto[0] = '-';
	}
	
	// Se toman los decimales
	digito = variacionDeTemperatura % 10;
	pTexto[4] = digito + '0';
	
	// El valor suministrado esta multiplicado por 10
	variacionDeTemperatura /= 10;
	
	// Se toman las unidades
	digito = variacionDeTemperatura % 10;
	pTexto[2] = digito + '0';
	
	// Se toman las decenas
	if( variacionDeTemperatura > 10 )
	{
		digito = variacionDeTemperatura % 100;
		digito /= 10;
		pTexto[1] = digito + '0';
	}
	else
		pTexto[1] = ' ';
	
	// Se coloca el .
	pTexto[3] = '.';
	
	// Se coloca la unidad
	pTexto[5] = 'C';
	
	// Se imprime el valor
	if( LCD.filas == LCD_2_FILAS )
	{
		borrarRenglon( RENGLON_MEDIO_SUPERIOR );
		error = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 6, pTexto );
	}
	else
		error = EscribirMensajeLCD( reglon, offset, 6, pTexto );
}



/****************************************************************/
/* EscribirCorriente											*/
/*  															*/
/*  Muestra la corriente suministrada en el LCD					*/
/*  															*/
/*  Recibe: La corriente a mostrar								*/
/*  Devuelve: Nada												*/
/****************************************************************/
void EscribirCorriente( int corriente_mA, char reglon, char offset )
{
	// Variable auxiliar
	char	corriente[4];
	
	// Los signos en el vector de caracteres para la corriente siempre seran fijos
	corriente[1] = '.';
	corriente[3] = 'A';
	
	// Se obtienen los digitos en ASCII para mostrarlos en el LCD
	corriente_mA /= 100;
	corriente[2] = ( corriente_mA % 10 ) + '0';
	corriente_mA /= 10;
	corriente[0] = ( corriente_mA % 10 ) + '0';
	EscribirMensajeLCD( reglon, offset, 4, corriente );
}



/****************************************************************/
/* EscribirROM_EnLCD											*/
/*  															*/
/*  Muestra la ROM del sensor en el LCD							*/
/*  															*/
/*  Recibe: La ROM a mostrar									*/
/*  Devuelve: Nada												*/
/****************************************************************/
void EscribirROM_EnLCD( unsigned char *ROM )
{
	// Si el LCD es de 8 caracteres, debe imprimirse en 2 partes
	if( LCD.caracteres == LCD_8_CARACTERES )
	{
		// Se escribe la parte alta
		escribirMediaROM_EnLCD( RENGLON_SUPERIOR, ROM, ROM_PARTE_ALTA );
		
		// Se escribe la parte baja
		escribirMediaROM_EnLCD( RENGLON_MEDIO_SUPERIOR, ROM, ROM_PARTE_BAJA );
		
		// En el caso de un LCD de 8 caracteres, se hace una pausa para poder ver el dato
		DemoraEnSegundos( 1 );
	}
	else
	{
		// Se escribe la ROM completa
		escribirTodaROM_EnLCD( RENGLON_MEDIO_SUPERIOR, ROM );
	}
}



/************************************************************/
/* BorrarLCD												*/
/*  														*/
/*  Simplemente borra el LCD								*/
/*  														*/
/*  Recibe: Nada											*/
/*  Devuelve: Nada											*/
/************************************************************/
void BorrarLCD( void )
{
	// Se envia la instruccion para borar el LCD
	enviarInstruccion( INSTRUCCION_LCD_BORRAR_PANTALLA );
	
	// Se genera una demora para que el LCD pueda llevar a cabo el borrado de la pantalla
	DemoraPorInterrupcion( CUARENTA_MILI_SEGUNDOS );
}





/****************************************************************************************************/
/* 						Funciones para la prueba reducida del manillar 								*/
/****************************************************************************************************/



/************************************************************/
/* ImprimirMensajeErrorPruebaReducida						*/
/*  														*/
/*  Indica que sensor no se inicio correctamente			*/
/*  														*/
/*  Recibe:	El numero del sensor fallido					*/
/*  Devuelve: Nada											*/
/************************************************************/
void ImprimirMensajeErrorPruebaReducida ( char sensor )
{
	// Se muestra un mensaje indicando la falla
	EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 20, "Revise por favor el " );
	EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 20, "sensor de temperatur" );
	EscribirMensajeLCD( RENGLON_INFERIOR, 0, 20, "  Presione DERECHA  " );

	// Se debe indicar el numero del sensor que falle
	switch( sensor )
	{
		case 1:
			EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 20, "      SOLDADO       " );
			break;
		case 2:
			EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 20, "   LADO CALIENTE    " );
			break;
	}
}



/************************************************************/
/* MostrarTemperaturasPruebaReducida						*/
/*  														*/
/*  Imprime las temperaturas de los sensores propios del	*/
/*  probador												*/
/*  														*/
/*  Recibe:	Las temperaturas								*/
/*  Devuelve: Nada											*/
/************************************************************/
void MostrarTemperaturasPruebaReducida ( int16_t sensorSoldado, int16_t sensorLadoCaliente )
{
	// Se imprime la temperatura del sensor soldado en la placa
	EscribirTemperatura( sensorSoldado, RENGLON_MEDIO_SUPERIOR, 10 );

	// Se imprime la temperatura del sensor para el lado caliente del manillar
	EscribirTemperatura( sensorLadoCaliente, RENGLON_MEDIO_INFERIOR, 10 );
}










/****************************************************************************************************************************/
/****************************************************************************************************************************/
/****************************************************************************************************************************/
/* 											Funciones de acceso interno 													*/
/****************************************************************************************************************************/
/****************************************************************************************************************************/
/****************************************************************************************************************************/



/* ***  	FUNCIONES DE COMUNICACION		  *** */

/****************************************************************/
/* enviarInstruccion											*/
/*  															*/
/*  Se encarga de mandar el dato sobre el bus del puerto.    	*/
/*  Primero se manda la parte alta del dato y luego la baja. 	*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void enviarInstruccion( char dato )
{
	// Se verifican los bits del bus, para saber si hay que enviar el dato por partes (4 bits) o de una (8 bits)
	if( LCD.interfaz == LCD_4_BITS )
	{
		// Primero se deben deshabilitar las lineas de RS y E para poder colocar el dato en el puerto y luego habilitar la linea de E
		LCD_RS = DESACTIVADO;			// Es una instruccion
		LCD_E = DESACTIVADO;			// para que no tome las modificaciones que se van a realizar sobre el puerto de datos
		
		// Se escribe la parte alta de los datos sobre el puerto
		mandarNibbleAltoLCD_Probador( dato );
		
		LCD_E = ACTIVADO;				// Se habilita la linea de E para que el LCD tome la parte del dato
		DEMORA_200US;
		LCD_E = DESACTIVADO;			// Se deshabilita la linea de E
		
		// Se escribe la parte baja de los datos sobre el puerto
		mandarNibbleBajoLCD_Probador( dato );
		
		LCD_E = ACTIVADO;				// Se habilita la linea de E para que el LCD tome la parte del dato
		DEMORA_200US;
		LCD_E = DESACTIVADO;			// Se deshabilita la linea de E
	}
	
	// Se contempla una demora final, para darle tiempo al LCD a reponerse para los siguientes comandos
	DEMORA_200US;
}



/****************************************************************/
/* enviarDato													*/
/*  															*/
/*  Se encarga de mandar el dato sobre el bus del puerto.    	*/
/*  Primero se manda la parte alta del dato y luego la baja. 	*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void enviarDato( char dato )
{
	// Se verifican los bits del bus, para saber si hay que enviar el dato por partes (4 bits) o de una (8 bits)
	if( LCD.interfaz == LCD_4_BITS )
	{
		// Primero se deben deshabilitar las lineas de RS y E para poder colocar el dato en el puerto y luego habilitar la linea de E
		LCD_RS = ACTIVADO;				// Es un dato
		LCD_E = DESACTIVADO;			// para que no tome las modificaciones que se van a realizar sobre el puerto de datos
		
		// Se escribe la parte alta de los datos sobre el puerto
		mandarNibbleAltoLCD_Probador( dato );
		
		LCD_E = ACTIVADO;				// Se habilita la linea de E para que el LCD tome la parte del dato
		DEMORA_200US;
		LCD_E = DESACTIVADO;			// Se deshabilita la linea de E
		
		// Se escribe la parte baja de los datos sobre el puerto
		mandarNibbleBajoLCD_Probador( dato );
		
		LCD_E = ACTIVADO;				// Se habilita la linea de E para que el LCD tome la parte del dato
		DEMORA_200US;
		LCD_E = DESACTIVADO;			// Se deshabilita la linea de E
	}
	
	// Se contempla una demora final, para darle tiempo al LCD a reponerse para los siguientes comandos
	DEMORA_200US;
}





/****************************************************************/
/* mandarNibbleAltoLCD_Vacio									*/
/*  															*/
/*  Envia la parte alta del dato utilizando la configuracion de */
/*  pines que tiene el LCD en el mango de vacio.				*/
/*  															*/
/*  Recibe: El dato a enviar									*/
/*  Devuelve: Nada												*/
/****************************************************************/
void mandarNibbleAltoLCD_Vacio( char dato )
{
	// Variables auxiliares
	char auxiliar;

	// Se escribe la parte alta de los datos sobre el puerto
	auxiliar = dato & MASCARA_DB654_DATO_MANGO_VACIO;	// Se enmascaran los datos de DB6, DB5 y DB4 
	auxiliar <<= ROTAR_DB654_MANGO_VACIO;				// Se lo desplaza una unidad para que coincidan con los pines PTED7, PTED6 y PTED5
	LCD_PUERTO &= BORRAR_DBX_PUERTO_MANGO_VACIO;		// Se borran los pines del puerto para escribir los datos
	LCD_PUERTO |= auxiliar;								// Se escriben los datos en el puerto. Resta escribir DB7
	auxiliar = dato & MASCARA_DB7_DATO_MANGO_VACIO;
	if( auxiliar > 0 )									// Se escribe el dato DB7
		LCD_DB7 = 1;
	else
		LCD_DB7 = 0;
}



/****************************************************************/
/* mandarNibbleBajoLCD_Vacio									*/
/*  															*/
/*  Envia la parte baja del dato utilizando la configuracion de */
/*  pines que tiene el LCD en el mango de vacio.				*/
/*  															*/
/*  Recibe: El dato a enviar									*/
/*  Devuelve: Nada												*/
/****************************************************************/
void mandarNibbleBajoLCD_Vacio( char dato )
{
	// Variables auxiliares
	char auxiliar;

	// Se escribe la parte baja de los datos sobre el puerto
	auxiliar = dato & MASCARA_DB210_DATO_MANGO_VACIO;	// Se enmascaran los datos de DB2, DB1 y DB0 
	auxiliar <<= ROTAR_DB210_MANGO_VACIO;				// Se lo desplaza cinco unidades para que coincidan con los pines PTED7, PTED6 y PTED5
	LCD_PUERTO &= BORRAR_DBX_PUERTO_MANGO_VACIO;		// Se borran los pines del puerto para escribir los datos
	LCD_PUERTO |= auxiliar;								// Se escriben los datos en el puerto. Resta escribir DB3
	auxiliar = dato & MASCARA_DB3_DATO_MANGO_VACIO;
	if( auxiliar > 0 )									// Se escribe el dato DB7
		LCD_DB7 = 1;
	else
		LCD_DB7 = 0;
}





/****************************************************************/
/* mandarNibbleAltoLCD_Probador									*/
/*  															*/
/*  Envia la parte alta del dato utilizando la configuracion de */
/*  pines que tiene el LCD en el mango de vacio.				*/
/*  															*/
/*  Recibe: El dato a enviar									*/
/*  Devuelve: Nada												*/
/****************************************************************/
void mandarNibbleAltoLCD_Probador( char dato )
{
	// Variables auxiliares
	char auxiliar;

	// Se escribe la parte alta de los datos sobre el puerto
	auxiliar = dato & MASCARA_DB76_DATO_PROBADOR; 
	auxiliar >>= ROTAR_DB76_PROBADOR;
	PTED &= BORRAR_DB76_PUERTO_PROBADOR;
	PTED |= auxiliar;
	
	auxiliar = dato & MASCARA_DB54_DATO_PROBADOR; 
	auxiliar <<= ROTAR_DB54_PROBADOR;
	PTED &= BORRAR_DB54_PUERTO_PROBADOR;
	PTED |= auxiliar;
}



/****************************************************************/
/* mandarNibbleBajoLCD_Probador									*/
/*  															*/
/*  Envia la parte baja del dato utilizando la configuracion de */
/*  pines que tiene el LCD en el mango de vacio.				*/
/*  															*/
/*  Recibe: El dato a enviar									*/
/*  Devuelve: Nada												*/
/****************************************************************/
void mandarNibbleBajoLCD_Probador( char dato )
{
	// Variables auxiliares
	char auxiliar;

	// Se escribe la parte alta de los datos sobre el puerto
	auxiliar = dato & MASCARA_DB32_DATO_PROBADOR; 
	auxiliar <<= ROTAR_DB32_PROBADOR;
	LCD_PUERTO &= BORRAR_DB76_PUERTO_PROBADOR;
	LCD_PUERTO |= auxiliar;
	
	auxiliar = dato & MASCARA_DB10_DATO_PROBADOR; 
	auxiliar <<= ROTAR_DB10_PROBADOR;
	LCD_PUERTO &= BORRAR_DB54_PUERTO_PROBADOR;
	LCD_PUERTO |= auxiliar;
}



/* ***  	FUNCIONES DE INICIALIZACION		  *** */

/************************************************************/
/* inicializarLCD_4bits										*/
/*  														*/
/*  Inicializa el LCD para usar una interfaz de 4 bits.		*/
/*  Se agrega una demora de 120mSeg luego de cada			*/
/*	instruccion para darle tiempo al LCD a procesarlas.		*/
/*  														*/
/*  Recibe: Nada											*/
/*  Devuelve: Nada											*/
/************************************************************/
void inicializarLCD_4bits( void )
{
	// Variable auxiliar para hacer mas legibles los codigos
	char instruccion;
	
	// Demora de 120mSeg para esperar que se inicie el LCD
	demoraInicialLCD();
	
	// Primero se deben iniciar los pines del puerto
	inicializarPuertoLCD();
	  
	// Demora de 120mSeg para esperar que se inicie el LCD
	demoraInicialLCD();
	
	// Se inicializa un bus de 4 bits
	LCD_RS = DESACTIVADO;
	LCD_E = DESACTIVADO;
	instruccion = INSTRUCCION_LCD_INTERFAZ + INSTRUCCION_LCD_INTERFAZ_4BITS;
	mandarNibbleAltoLCD_Probador( instruccion );
	LCD_E = ACTIVADO;
	DEMORA_200US;
	LCD_E = DESACTIVADO;
	demoraInicialLCD();
	
	// Se selecciona un bus de 4 bits, la cantidad de lineas y la fuente del LCD
	instruccion = INSTRUCCION_LCD_INTERFAZ + INSTRUCCION_LCD_INTERFAZ_4BITS;
	if( LCD.filas == LCD_2_FILAS || LCD.filas == LCD_4_FILAS )
		instruccion += INSTRUCCION_LCD_INTERFAZ_2LINEAS;
	else
		instruccion += INSTRUCCION_LCD_INTERFAZ_1LINEA;
	if( LCD.fuente == LCD_FUENTE_CHICA )
		instruccion += INSTRUCCION_LCD_INTERFAZ_5X8;
	else
		instruccion += INSTRUCCION_LCD_INTERFAZ_5X10;
	enviarInstruccion( instruccion );
	demoraInicialLCD();
	
	// Se enciende la pantalla. Por default se usa el cursor apagado y sin parpadeo
	enviarInstruccion( INSTRUCCION_LCD_PANTALLA + INSTRUCCION_LCD_PANTALLA_ON + INSTRUCCION_LCD_PANTALLA_CURSOR_OFF + INSTRUCCION_LCD_PANTALLA_BLINK_OFF );
	demoraInicialLCD();
	
	// Se selecciona un modo incremental
	enviarInstruccion( INSTRUCCION_LCD_MODO + INSTRUCCION_LCD_MODO_INCREMENTO + INSTRUCCION_LCD_MODO_SHIFT_OFF );
	demoraInicialLCD();
	
	// Se borra la pantalla
	enviarInstruccion( INSTRUCCION_LCD_BORRAR_PANTALLA );
	demoraInicialLCD();

}



/************************************************************/
/* inicializarPuertoLCD										*/
/*  														*/
/*  Define los pines hacia el LCD como salidas y los pone a */
/*  0. Ademas, enciende el backlight para el mango de vacio */
/*  														*/
/*  Recibe: Nada											*/
/*  Devuelve: Nada											*/
/************************************************************/
void inicializarPuertoLCD()
{
	// Se inicializan los puertos como salida
	LCD_RS_DD	= 1;
	LCD_E_DD	= 1;
	
	PTEDD = 0xF0;
	
	// Se ponen a 0 todas las salidas, por las dudas.
	LCD_RS	= 0;
	LCD_E	= 0;
	LCD_DB4	= 0;
	LCD_DB5 = 0;
	LCD_DB6	= 0;
	LCD_DB7	= 0;
}



/****************************************************************/
/* demoraInicialLCD												*/
/*  															*/
/*  Demora de 120mSeg para que el LCD procese las ordenes		*/
/*  de inicializacion.											*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void demoraInicialLCD()
{
	DemoraPorInterrupcion( CUARENTA_MILI_SEGUNDOS );
	DemoraPorInterrupcion( CUARENTA_MILI_SEGUNDOS );
	DemoraPorInterrupcion( CUARENTA_MILI_SEGUNDOS );
}



/* ***  	FUNCIONES DE ESCRITURA DE VALORES EN LA PANTALLA		  *** */

/****************************************************************/
/* escribirMediaROM_EnLCD										*/
/*  															*/
/*  Muestra la mitad de la ROM del sensor en el LCD				*/
/*  															*/
/*  Recibe: La ROM a mostrar y que parte						*/
/*  Devuelve: Nada												*/
/****************************************************************/
void escribirMediaROM_EnLCD( unsigned char renglon, unsigned char *ROM, bool parteAlta )
{
	// Variables auxiliares
	unsigned char 	pTexto[8];
	unsigned char	caracter;
	unsigned char	byte;
	unsigned char	indiceCaracter;
	
	// Mediante la variable "parteAlta" se indica si se imprimen los primeros 4 caracteres o los ultimos 4 de la ROM
	if( parteAlta == TRUE )
		byte = 7;
	else
		byte = 3;
	
	for( indiceCaracter = 0; indiceCaracter < 8; indiceCaracter++ )
	{
		// Parte alta del byte
		caracter = ROM[byte];
		caracter >>= 4;
		if( caracter <= 9 )
			pTexto[indiceCaracter] = caracter + '0';
		else
			pTexto[indiceCaracter] = caracter - 10 + 'A';
		
		// Se incrementa el contador de caracteres
		indiceCaracter++;
		
		// Parte baja del byte
		caracter = ROM[byte];
		caracter &= 0x0F;
		if( caracter <= 9 )
			pTexto[indiceCaracter] = caracter + '0';
		else
			pTexto[indiceCaracter] = caracter - 10 + 'A';
		
		// Se incrementa el contador de bytes
		byte--;
	}
	
	// Se imprime el valor
	error = EscribirMensajeLCD( renglon, 0, 8, pTexto );
}



/****************************************************************/
/* escribirMediaROM_EnLCD										*/
/*  															*/
/*  Muestra la mitad de la ROM del sensor en el LCD				*/
/*  															*/
/*  Recibe: La ROM a mostrar y que parte						*/
/*  Devuelve: Nada												*/
/****************************************************************/
void escribirTodaROM_EnLCD( unsigned char renglon, unsigned char *ROM )
{
	// Variables auxiliares
	unsigned char 	pTexto[16];
	unsigned char	caracter;
	unsigned char	byte;
	unsigned char	indiceCaracter;
	
	// Se inicializa el contador de bytes
	byte = 7;
	
	// Se convierten los valores en caracteres ASCII
	for( indiceCaracter = 0; indiceCaracter < 16; indiceCaracter++ )
	{
		// Parte alta del byte
		caracter = ROM[byte];
		caracter >>= 4;
		if( caracter <= 9 )
			pTexto[indiceCaracter] = caracter + '0';
		else
			pTexto[indiceCaracter] = caracter - 10 + 'A';
		
		// Se incrementa el contador de caracteres
		indiceCaracter++;
		
		// Parte baja del byte
		caracter = ROM[byte];
		caracter &= 0x0F;
		if( caracter <= 9 )
			pTexto[indiceCaracter] = caracter + '0';
		else
			pTexto[indiceCaracter] = caracter - 10 + 'A';
		
		// Se incrementa el contador de bytes
		byte--;
	}
	
	// Se imprime el valor
	error = EscribirMensajeLCD( renglon, 4, 16, pTexto );
}



/* ***  	FUNCIONES DE ESCRITURA DE MENSAJES EN LA PANTALLA		  *** */

/****************************************************************/
/* mensajeLCD_Iniciado											*/
/*  															*/
/*  Muestra el mensaje de inicio								*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void mensajeLCD_Iniciado( void )
{
	// Primero se borra la pantalla
	BorrarLCD();
	
	// Luego se imprime el mensaje
	if( LCD.filas == LCD_4_FILAS )
	{
	  error = EscribirMensajeLCD( RENGLON_SUPERIOR, 1, 18, "Iniciado: Probador" );
	  error = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 3, 13, "de manillares" );
	  error = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 2, 15, "semi automatico" );
	  error = EscribirMensajeLCD( RENGLON_INFERIOR, 4, 11, "Body Health" );
	  DemoraEnSegundos( 2 );
	}
	else
	{
		if( LCD.caracteres == LCD_8_CARACTERES )
		{
			error = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 8, "Iniciado" );
			error = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 8, "Probador" );
			DemoraEnSegundos( 1 );
			error = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 8, "   de   " );
			error = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 8, "manillar" );
			DemoraEnSegundos( 1 );
			error = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 8, "  semi  " );
			error = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 8, "automat." );
			DemoraEnSegundos( 1 );
			error = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 8, "  Body  " );
			error = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 8, " Health " );
		}
	}
}



/****************************************************************/
/* mensajeLCD_SinSensores										*/
/*  															*/
/*  Indica en pantalla que no se registraron sensores			*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void mensajeLCD_SinSensores( void )
{
	if( LCD.filas == LCD_4_FILAS )
	{
		error = EscribirMensajeLCD( RENGLON_INFERIOR, 2, 15, "No hay sensores" );
	}
	else
		if( LCD.caracteres == LCD_8_CARACTERES )
		{
			// Primero se borra la pantalla
			BorrarLCD();
			
			// Luego se imprime el mensaje
			error = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 8, " No hay " );
			error = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 8, "sensores" );
		}
}



/****************************************************************/
/* mensajeLCD_BuscandoSensores									*/
/*  															*/
/*  Indica que no se comenzara la busqueda de sensores			*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void mensajeLCD_BuscandoSensores( void )
{
	if( LCD.filas == LCD_4_FILAS )
	{
		error = EscribirMensajeLCD( RENGLON_SUPERIOR, 1, 17, "Buscando sensores" );
		error = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 20, "conectados en el bus" );
		error = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 20, "--------------------" );
		error = EscribirMensajeLCD( RENGLON_INFERIOR, 0, 20, "                    " );
	}
	else
		if( LCD.caracteres == LCD_8_CARACTERES )
		{
			// Primero se borra la pantalla
			BorrarLCD();
			
			// Luego se imprime el mensaje
			error = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 8, "Buscando" );
			error = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 8, "sensores" );
		}
}



/****************************************************************/
/* mensajeLCD_FuncionDelSensor									*/
/*  															*/
/*  Indica que funcion cumple el sensor indicado en pantalla	*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void mensajeLCD_FuncionDelSensor ( char posicion )
{
	if( LCD.caracteres == LCD_8_CARACTERES )
	{
		// Primero se borra la pantalla ya que no se puede imprimir todo junto
		BorrarLCD();
		
		// Luego se escribe el texto de la funcion que cumpla el sensor
		switch( posicion )
		{
			case 0:
				error = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 8, "1-Soldad" );
				break;
			case 1:
				error = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 8, "2-Mangue" );
				break;
			case 2:
				error = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 8, "3-Lado c" );
				break;
			case 3:
				error = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 8, "4-Peltie" );
				break;
			case 4:
				error = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 8, "5- Extra" );
				break;
		}
	}
	else
	{
		if( LCD.filas == LCD_4_FILAS )
		{
			borrarRenglon( RENGLON_MEDIO_INFERIOR );
			switch( posicion )
			{
				case 0:
					error = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 4, 11, "1 - Soldado" );
					break;
				case 1:
					error = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 4, 12, "2 - Manguera" );
					break;
				case 2:
					error = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 1, 17, "3 - Lado caliente" );
					break;
				case 3:
					error = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 4, 11, "4 - Peltier" );
					break;
				case 4:
					error = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 4, 9, "5 - Extra" );
					break;
			}
		}
	}
}



/****************************************************************/
/* mensajeLCD_SensoresEncontrados								*/
/*  															*/
/*  Muestra la cantidad de sensores que se listaron				*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void mensajeLCD_SensoresEncontrados ( char encontrados )
{
	// Se borra la pantalla porque es lo primero que se va a listar
	BorrarLCD();
	
	if( LCD.caracteres == LCD_8_CARACTERES )
	{
		switch( encontrados )
		{
			case 1:
				EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 8, " Hay 1  " );
				error = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 8, " sensor " );
				return;
			case 2:
				EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 8, " Hay 2  " );
				break;
			case 3:
				EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 8, " Hay 3  " );
				break;
			case 4:
				EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 8, " Hay 4  " );
				break;
			case 5:
				EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 8, " Hay 5  " );
				break;
		}
		error = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 8, "sensores" );
	}
	
	else
	{
		if( LCD.filas == LCD_4_FILAS )
		{
			switch( encontrados )
			{
				case 1:
					EscribirMensajeLCD( RENGLON_SUPERIOR, 4, 12, "Hay 1 sensor" );
					break;
				case 2:
					EscribirMensajeLCD( RENGLON_SUPERIOR, 3, 14, "Hay 2 sensores" );
					break;
				case 3:
					EscribirMensajeLCD( RENGLON_SUPERIOR, 3, 14, "Hay 3 sensores" );
					break;
				case 4:
					EscribirMensajeLCD( RENGLON_SUPERIOR, 3, 14, "Hay 4 sensores" );
					break;
				case 5:
					EscribirMensajeLCD( RENGLON_SUPERIOR, 3, 14, "Hay 5 sensores" );
					break;
			}
		}
	}
}



/****************************************************************/
/* borrarRenglon												*/
/*  															*/
/*  Borra un renglon del LCD									*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void borrarRenglon( char renglon )
{
	switch( renglon )
	{
		case RENGLON_SUPERIOR:
			error = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 20, "                    " );
			break;
		case RENGLON_MEDIO_SUPERIOR:
			error = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 20, "                    " );
			break;
		case RENGLON_MEDIO_INFERIOR:
			error = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 20, "                    " );
			break;
		case RENGLON_INFERIOR:
			error = EscribirMensajeLCD( RENGLON_INFERIOR, 0, 20, "                    " );
			break;
	}
}





/****************************************************************/
/* EscribirUbicacion											*/
/*  															*/
/*  Muestra la ubicacion suministrada en el LCD					*/
/*  															*/
/*  Recibe: La temperatura a mostrar							*/
/*  Devuelve: Nada												*/
/****************************************************************/
void EscribirUbicacion( char ubicacion, char reglon, char offset )
{
	// Variables auxiliares
	unsigned char 	digito;
	unsigned char 	pTexto[2];

	// Se toman las unidades
	digito = ubicacion % 10;
	pTexto[1] = digito + '0';
	
	// Se toman las decenas
	ubicacion /= 10;
	digito = ubicacion % 10;
	pTexto[0] = digito + '0';
	
	// Se imprime el valor
	error = EscribirMensajeLCD( reglon, offset, 2, pTexto );
}
