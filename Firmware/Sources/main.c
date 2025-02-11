/* ###################################################################
**     Filename    : main.c
**     Project     : ProbadorManillares
**     Processor   : MC9S08AC16CFD
**     Version     : Driver 01.12
**     Compiler    : CodeWarrior HCS08 C Compiler
**     Date/Time   : 2018-11-26, 12:56, # CodeGen: 0
**     Abstract    :
**         Main module.
**         This module contains user's application code.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file main.c
** @version 01.12
** @brief
**         Main module.
**         This module contains user's application code.
*/         
/*!
**  @addtogroup main_module main module documentation
**  @{
*/         
/* MODULE main */


/* Including needed modules to compile this module/procedure */
#include "Cpu.h"
#include "Events.h"
#include "Vtpm1ovf.h"
#include "Vadc1.h"
/* Include shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"

#include "main.h"

/* User includes (#include below this line is not maintained by Processor Expert) */


	/****************************************************************************************************************/
	/*  												VARIABLES													*/
	/****************************************************************************************************************/

// Variables para controlar los sensores de temperatura
extern sensorDS18S20	sensores[ TOTAL_DE_SENSORES ];
erroresDS18S20 			errorSensorDS18S20;


// Variables para controlar los pulsadores
estadosTeclas 			estadoTeclaPresionada;
uint16_t				vecesSinRevisarTeclaAceptar;
uint16_t				vecesSinRevisarTeclaCancelar;
uint16_t				vecesSinRevisarTeclaDerecha;
uint16_t				vecesSinRevisarTeclaIzquierda;
char					pulsador;


// Variables para manejar los pernos
pernosObj				pernos[ TOTAL_DE_CONTACTOS_MULTIPLEXORES ];
resultadoPernos			estadoPernos;
resultadoPernos			condicionDelPerno[ TOTAL_DE_CONTACTOS_MULTIPLEXORES ];
char					pernosUbicacion[ TOTAL_DE_CONTACTOS_MULTIPLEXORES ];
bool 					hayPernoAbierto;
bool 					hayPernoEnCorto;
bool 					hayPernoEnCortoCon5V;
bool 					hayPernoEnCortoElMazo;
bool 					hayPernoAlternado;


// Variable para manejar el estado de los LEDs
resultadoLEDs			estadoLEDs;


// Variable para manejar el estado del sensor de temperatura de la Peltier
resultadoSensor			estadoSensor;


// Variable para manejar el estado de la celda Peltier
resultadoPeltier 		estadoPeltier;
int						medicionesCorrientePeltier[ 2 ];


// Variables auxiliares para almacenar las mediciones durante los segundos de prueba de la celda Peltier
char 					indiceMedicion;
int16_t					medicionesTemperaturaAmbiente[ 2 ];
int16_t					medicionesTemperaturaLadoFrio[ 2 ];
int16_t					medicionesTemperaturaLadoCaliente[ 2 ];
int16_t					medicionesTemperaturaManguera[ 2 ];
int16_t					variacionDeTemperaturaLadoFrio;
int16_t					variacionDeTemperaturaLadoCaliente;
int16_t					variacionInicialDeTemperaturaEntreLados;
int16_t					variacionFinalDeTemperaturaEntreLados;
int16_t					acumulado_ladoFrio;


// Variables para controlar el ADC
uint8_t					cuentasADC;
bool					semaforoADC;


// Variable generica para cualquier cosa
char 					auxiliar;



char					bufferTransmicion[ BYTES_TRAMA_PRUEBA_REDUCIDA ];


// Variable para indicar la finalizacion de los loops
bool					finalizarLoop;


// Variable para contemplar la devolucion de las funciones que indican algun error en su ejecucion
bool 					errorGeneral;



	/****************************************************************************************************************/
	/*  												PROGRAMA													*/
	/****************************************************************************************************************/

void main(void)
{
  /* Write your local variable definition here */ 
	bool	reimprimirPantallaInicial = TRUE;
	bool	iniciarPrueba = FALSE;
	bool	pruebaReducida = TRUE;
	char	indiceByte;
	char	opcionIncicadaEnElMenu = 1;
	
  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/

  /* Write your code here */

  
  
/* ******************** ******************** CODIGO PROPIO ******************** ******************** */

    // Se inicializan los puertos del micro
    InicializarPuertosProbador();
  
	// Se inicializa el timer
	InicializarTimer1();
	
	// Se inicializa la comunicacion serie, independientemente de si se use o no
	InicializarComunicacionSerie();
	
	// Demora para dejar que se estabilice la fuente de alimentacion
	DemoraEnSegundos( 1 );
	
	// Se configura el tipo de LCD a utilizar
	errorGeneral = SeleccionarLCD( LCD_4_FILAS, LCD_20_CARACTERES, LCD_FUENTE_CHICA, LCD_4_BITS, LCD_PUERTO_EXTERNO, LCD_PROBADOR );
	
	// Se inicializa el LCD
	InicializarLCD();
	
	// Se configura el ADC
	ConfigurarADC();
	
	
	// Se inician las cosas que se toman por default para la prueba reducida
	InicializarSensoresProbador();
	
	// Bucle perpetuo para ejecutar las pruebas
	while( TRUE )
	{
		// Se revisa el selector del tipo de pruebas
		pulsador = getRegBits( PULSADOR_SEL_PUERTO, PULSADOR_SEL_MASCARA );
		if( pulsador == 0 )
		{
			/* ******************** PROBADOR DE CABLES ******************** */
			EncenderLedVerde();
			
			if( reimprimirPantallaInicial == TRUE )
			{
				reimprimirPantallaInicial = FALSE;
				BorrarLCD();
				errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR,       0, 20, " Probador de cables " );
				errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 20, "     automatico     " );
				errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 20, " ACEPTAR -> Probar  " );
			}
			
			// Se espera la opresion de alguna tecla
			estadoTeclaPresionada = RevisarOpresionDeTeclas();
			
			// Se atiende la opresion de la tecla
			switch( estadoTeclaPresionada )
			{
				// Se quiere verificar el cable conectado
				case TECLA_ACEPTAR:
					
					// Se indica que luego se debe reimprimir la pantalla, ya que se informara el estado de las pruebas en la misma
					reimprimirPantallaInicial = TRUE;
					
					// Se realiza la prueba de los cables
					estadoPernos = ProbarCables( CABLE_MAZO_66A );
					
					// Se informa en pantalla el resultado de la prueba
					switch( estadoPernos )
					{
						case PERNO_SIN_ERRORES:
							errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR,       0, 20, "      Todo ok       " );
							errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 20, "   Cable aprobado   " );
							break;
						case PERNO_ABIERTO:
							InformarCablesAbiertos();
							break;
						case PERNO_CORTO_CIRCUITO:
							InformarCablesEnCorto();
							break;
						case PERNO_SIN_ALTERNANCIA:
							InformarCablesConMalUbicacion();
							break;
					}
					
					// Se demora un tiempo para que el usuario pueda leer las indicaciones
					DemoraEnSegundos( 10 );
					break;
					
					
				// Se debe pasar al menu de la prueba reducida, si es que se saca el jumper
				case TECLA_CANCELAR:
					
					// Se setean las variables de control necesarias
					reimprimirPantallaInicial = TRUE;
					pruebaReducida = TRUE;
					break;
			}
			
			/* ******************** FIN DEL PROBADOR DE CABLES ******************** */
		}
		
		
		else
		{
			ApagarLedVerde();
			if( pruebaReducida == TRUE )
			{
				/* ******************** PRUEBA REDUCIDA ******************** */
				
				/********************************************************************/
				/********************************************************************/
				/* 																 	*/
				/* 				SECCION PARA REALIZAR LA PRUEBA REDUCIDA		 	*/
				/* 																 	*/
				/********************************************************************/
				/*	La secuencia que se ejecuta es la siguiente:					*/
				/* 	Medicion de continuidad de los pernos:							*/
				/* 		- Se realiza el barrido de ambos multiplexores y se			*/
				/* 		  registra la continuidad entre ambos extremos para todas	*/
				/* 		  las combinaciones. Para un correcto funcionamiento, solo	*/
				/* 		  se deberia registrar continuidad cuando ambos				*/
				/* 		  multiplexores tengan la misma direccion. Esto significa	*/
				/* 		  que solo la diagonal principal de la matriz de resultados	*/
				/* 		  deberia ser no nula.										*/
				/* 	Medicion de las ramas de los LEDs:								*/
				/* 		- Se alimentan los LEDs y se registra la corriente que		*/
				/* 		  consumen. Se definen 2 umbrales para inferir el estado de */
				/* 		  las 2 ramas.												*/
				/* 	Registrar el sensor de temperatura:								*/
				/* 		- Se listan los sensores de temperatura, teniendo fijas las */
				/* 		  direcciones de los sensores del sistema de circulacion.	*/
				/* 		  La direccion no registrada se corresponde con el sensor	*/
				/* 		  propio del manillar. Se registra la temperatura, que no 	*/
				/* 		  deberia diferir mucho de la ambiente, salvo que se haya	*/
				/* 		  utilizado el manillar recientemente.						*/
				/* 	Medir el correcto enfriamiento de la Peltier:					*/
				/* 		- Se enciende la celda Peltier por 10 segundos y se			*/
				/* 		  registra que la temperatura descienda correctamente.		*/
				/********************************************************************/
				/********************************************************************/
				
				if( reimprimirPantallaInicial == TRUE )
				{
					reimprimirPantallaInicial = FALSE;
					
					// Se da un mensaje para indicar que se espera la orden de inicio
					MostrarPantallaInicialPruebaReducida();
				}
				
				// Se espera la opresion de alguna tecla
				estadoTeclaPresionada = RevisarOpresionDeTeclas();
				
				// Se atiende la opresion de la tecla
				switch( estadoTeclaPresionada )
				{
					// Se quiere registrar el sensor extra
					case TECLA_IZQUIERDA_O_ARRIBA:
		
						// Se indica que se deberia reimprimir la pantalla inicial si se sale de este menu
						reimprimirPantallaInicial = TRUE;
						
						// Se borra el flag
						finalizarLoop = FALSE;
						
						// Se da un mensaje para indicar que se espera la confirmacion de la colocacion del sensor
						MostrarPantallaInicialSensorExtra();
						
						while( finalizarLoop == FALSE )
						{
							// Se espera la opresion de alguna tecla
							estadoTeclaPresionada = RevisarOpresionDeTeclas();
							
							// Se atiende la opresion de la tecla
							switch( estadoTeclaPresionada )
							{
								// Se deben tomar los datos del sensor
								case TECLA_ACEPTAR:
									
									if( RegistrarElSensorDeTemperaturaExtra() == TRUE )
									{
										escribirTodaROM_EnLCD( RENGLON_INFERIOR, sensores[ SENSOR_MANILLAR_O_EXTRA ].ROM );
										DemoraEnSegundos( 2 );
										errorSensorDS18S20 = LeerTemperaturaDS18S20( SENSOR_MANILLAR_O_EXTRA );
										errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 0, 20, "Temperatura:        " );
										EscribirTemperatura( sensores[ SENSOR_MANILLAR_O_EXTRA ].temperatura, RENGLON_INFERIOR, 13 );
										
										// Se borran los datos previos del sensor de temperatura extra
										sensores[ SENSOR_MANILLAR_O_EXTRA ].usado = FALSE;
										sensores[ SENSOR_MANILLAR_O_EXTRA ].temperatura = 0;
										for( indiceByte = 0; indiceByte < LONGITUD_ROM_BYTES; indiceByte++ )
											sensores[ SENSOR_MANILLAR_O_EXTRA ].ROM[ indiceByte ] = 0;
									}
									else
										errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 0, 20, "*SENSOR NO CONECTADO" );
									DemoraEnSegundos( 2 );
									errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 0, 20, "ROM:                " );
									
									// Cuando se termina la prueba, se borran los datos del sensor de temperatura extra
									sensores[ SENSOR_MANILLAR_O_EXTRA ].usado = FALSE;
									sensores[ SENSOR_MANILLAR_O_EXTRA ].temperatura = 0;
									for( indiceByte = 0; indiceByte < LONGITUD_ROM_BYTES; indiceByte++ )
										sensores[ SENSOR_MANILLAR_O_EXTRA ].ROM[ indiceByte ] = 0;
									break;
									
									
								// Se quiere volver al menu anterior
								case TECLA_DERECHA_O_ABAJO:
									
									// Se setea el indicador para no continuar a la espera de sensores extra
									finalizarLoop = TRUE;
									break;
							}
						}
						
						break;
						
					// Se quiere realizar la prueba reducida
					case TECLA_ACEPTAR:
						
						// Se indica que se debe reimprimir la pantalla inicial con las indicaciones para controlar la prueba
						reimprimirPantallaInicial = TRUE;
						
						// Se muestra un mensaje indicando que se mediran las temperaturas
						MostrarPantallaTemperaturasIniciales();
						
						// Antes de iniciar una prueba, se revisa que los sensores de temperatura del equipo esten correctos
						errorSensorDS18S20 = LeerTodasLasTemperaturasDS18S20();
						if( errorSensorDS18S20 != SIN_ERROR )
						{
							if( errorSensorDS18S20 == TEMPERATURA_PROHIBIDA )
							{
								// Si falla alguno de los sensores propios del manillar, se informa cual es
								if( sensores[ SENSOR_SOLDADO_EN_PLACA ].temperatura == -5 )
									ImprimirMensajeErrorPruebaReducida( 1 );
								if( sensores[ SENSOR_PARA_LADO_CALIENTE ].temperatura == -5 )
									ImprimirMensajeErrorPruebaReducida( 2 );
								
								// Ante una falla, se informa y se espera la opresion de la tecla para continuar
								EsperarOpresionDeTecla( TECLA_DERECHA_O_ABAJO );
							}
						}
						
						else
						{
							// Si to-do esta correcto, se realiza la prueba
							MostrarTemperaturasPruebaReducida( sensores[ SENSOR_SOLDADO_EN_PLACA ].temperatura, sensores[ SENSOR_PARA_LADO_CALIENTE ].temperatura );
							DemoraEnSegundos( 3 );
							errorGeneral = PruebaManillarReducido();
							DemoraEnSegundos( 10 );
							ApagarLeds();
							
							// En este punto, se termino de ejecutar la prueba del manillar y se informo al usuario su resultado
							MostrarPantallaFinalPruebaReducida();
							EsperarOpresionDeTecla( TECLA_DERECHA_O_ABAJO );
							DemoraEnSegundos( 1 );
							
							// Cuando se termina la prueba, se borran los datos del sensor de temperatura del manillar
							sensores[ SENSOR_MANILLAR_O_EXTRA ].usado = FALSE;
							sensores[ SENSOR_MANILLAR_O_EXTRA ].temperatura = 0;
							for( indiceByte = 0; indiceByte < LONGITUD_ROM_BYTES; indiceByte++ )
								sensores[ SENSOR_MANILLAR_O_EXTRA ].ROM[indiceByte] = 0;
						}
						
						break;
						
					// Se quiere pasar a la version de pruebas completas
					case TECLA_CANCELAR:
						reimprimirPantallaInicial = TRUE;
						pruebaReducida = FALSE;
						opcionIncicadaEnElMenu = 1;
						break;
				}	// Fin del "switch( estadoTeclaPresionada )"
				
				/* ******************** FIN DE LA PRUEBA REDUCIDA ******************** */
				
			}	// Fin del "if( pruebaReducida == TRUE )"	-> Esto indica que se queria ingresar al menu de prueba reducida
			

			else
			{
				/* ******************** PRUEBAS COMPLETAS ******************** */
				
				if( reimprimirPantallaInicial == TRUE )
				{
					reimprimirPantallaInicial = FALSE;
					
					// Se muestran las distintas opciones 
					MostrarPantallaMenuesPruebaCompleta();
					
				}

				// Se espera la opresion de alguna tecla
				estadoTeclaPresionada = RevisarOpresionDeTeclas();
				
				// Se atiende la opresion de la tecla
				switch( estadoTeclaPresionada )
				{
				
					// Se quiere pasar a indicar la opcion inferior
					case TECLA_DERECHA_O_ABAJO:
						opcionIncicadaEnElMenu++;
						if( opcionIncicadaEnElMenu >= 3 )
							opcionIncicadaEnElMenu = 3;
						
						// Hay que colocar el indicador visual *
						if( opcionIncicadaEnElMenu == 1 )
						{
							errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 1, "*" );
							errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 1, " " );
							errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 0, 1, " " );
						}
						else
							if( opcionIncicadaEnElMenu == 2 )
							{
								errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 1, " " );
								errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 1, "*" );
								errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 0, 1, " " );
							}
							else
							{
								errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 1, " " );
								errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 1, " " );
								errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 0, 1, "*" );
							}
						break;
						
						
					// Se quiere pasar a indicar la opcion superior
					case TECLA_IZQUIERDA_O_ARRIBA:
						opcionIncicadaEnElMenu--;
						if( opcionIncicadaEnElMenu <= 1 )
							opcionIncicadaEnElMenu = 1;
						
						// Hay que colocar el indicador visual *
						if( opcionIncicadaEnElMenu == 1 )
						{
							errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 1, "*" );
							errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 1, " " );
							errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 0, 1, " " );
						}
						else
							if( opcionIncicadaEnElMenu == 2 )
							{
								errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 1, " " );
								errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 1, "*" );
								errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 0, 1, " " );
							}
							else
							{
								errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 1, " " );
								errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 1, " " );
								errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 0, 1, "*" );
							}
						break;
						
						
					// Se quiere realizar la prueba reducida
					case TECLA_ACEPTAR:
						
						reimprimirPantallaInicial = TRUE;
						
						switch( opcionIncicadaEnElMenu )
						{
							case 1:		// Prueba completa del manillar de crio
								
								// Se muestra un mensaje indicando que se mediran las temperaturas
								MostrarPantallaTemperaturasIniciales();
								
								// Antes de iniciar una prueba, se revisa que los sensores de temperatura del equipo esten correctos
								errorSensorDS18S20 = LeerTodasLasTemperaturasDS18S20();

								if( errorSensorDS18S20 != SIN_ERROR )
								{
									if( errorSensorDS18S20 == TEMPERATURA_PROHIBIDA )
									{
										// Si falla alguno de los sensores propios del manillar, se informa cual es
										if( sensores[ SENSOR_SOLDADO_EN_PLACA ].temperatura == -5 )
											ImprimirMensajeErrorPruebaReducida( 1 );
										if( sensores[ SENSOR_PARA_LA_MANGUERA ].temperatura == -5 )
											ImprimirMensajeErrorPruebaReducida( 2 );
										if( sensores[ SENSOR_PARA_LADO_CALIENTE ].temperatura == -5 )
											ImprimirMensajeErrorPruebaReducida( 3 );
										
										// Ante una falla, se informa y se espera la opresion de la tecla para continuar
										EsperarOpresionDeTecla( TECLA_DERECHA_O_ABAJO );
									}
								}
								
								else
								{
									// Si to-do esta correcto, se realiza la prueba
									MostrarTemperaturasPruebaReducida( sensores[0].temperatura, sensores[1].temperatura );
									DemoraEnSegundos( 3 );
									EncenderLedVerde();
									errorGeneral = PruebaCompletaManillarDeCrio();
									DemoraEnSegundos( 2 );
									ApagarLeds();
									
									// En este punto, se termino de ejecutar la prueba del manillar y se informo al usuario su resultado
									MostrarPantallaFinalPruebaCompleta();
									EsperarOpresionDeTecla( TECLA_DERECHA_O_ABAJO );
									DemoraEnSegundos( 1 );
									
									// Cuando se termina la prueba, se borran los datos del sensor de temperatura del manillar
									sensores[ SENSOR_MANILLAR_O_EXTRA ].usado = FALSE;
									sensores[ SENSOR_MANILLAR_O_EXTRA ].temperatura = 0;
									for( indiceByte = 0; indiceByte < LONGITUD_ROM_BYTES; indiceByte++ )
										sensores[ SENSOR_MANILLAR_O_EXTRA ].ROM[ indiceByte ] = 0;
								}
								
								break;
								
								
							case 2:		// Prueba del manillar de facial
								
								break;
								
								
							case 3:		// Prueba del manillar de enygma

								EncenderLedVerde();
								errorGeneral = PruebaManillarEnygma();
								DemoraEnSegundos( 10 );
								ApagarLeds();
								
								// En este punto, se termino de ejecutar la prueba del manillar y se informo al usuario su resultado
								BorrarLCD();
								errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 20, "  Presione DERECHA  " );
								EsperarOpresionDeTecla( TECLA_DERECHA_O_ABAJO );
								DemoraEnSegundos( 1 );
								
								// Cuando se termina la prueba, se borran los datos del sensor de temperatura del manillar
								sensores[ SENSOR_MANILLAR_O_EXTRA ].usado = FALSE;
								sensores[ SENSOR_MANILLAR_O_EXTRA ].temperatura = 0;
								for( indiceByte = 0; indiceByte < LONGITUD_ROM_BYTES; indiceByte++ )
									sensores[ SENSOR_MANILLAR_O_EXTRA ].ROM[ indiceByte ] = 0;
								
								break;
						}
						
						break;
						
						
					// Se quiere pasar a la version de pruebas reducidas
					case TECLA_CANCELAR:
						reimprimirPantallaInicial = TRUE;
						pruebaReducida = TRUE;
						break;
						
				}	// Fin del "switch( estadoTeclaPresionada )"
				
				/* ******************** FIN DE LAS PRUEBAS COMPLETAS ******************** */
				
			}	// Fin del "if( pruebaReducida == FALSE )"	-> Esto indica que se queria ingresar al menu de pruebas completas
			
		}	// Fin del "if( pulsador != 0 )" -> Esto indica que se queria ingresar al menu de pruebas de manillares 
		
	}




  /* ******************** CODIGO PROPIO ******************** */
  
  /* For example: for(;;) { } */

  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/


/* END main */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.3 [05.09]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/





	/****************************************************************************************************************/
	/*												FUNCIONES GENERALES												*/
	/****************************************************************************************************************/


/****************************************************************/
/* wait															*/
/*  															*/
/*  Coloca el micro controlador en estado de espera, del cual	*/
/*  se repone mediante una interrupcion. Sirve para evitar un	*/
/*  consumo innecesario durante las demoras.					*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void wait ( void )
{
	asm(wait);
}



/****************************************************************/
/* EsperarOpresionDeTecla										*/
/*  															*/
/*  Genera una demora bloqueante hasta que se oprima la tecla	*/
/*  solicitada													*/
/*  															*/
/*  Recibe: El pulsador que se quiere esperar					*/
/*  Devuelve: Nada												*/
/****************************************************************/
void EsperarOpresionDeTecla ( char tecla )
{
	switch( tecla )
	{
		case TECLA_DERECHA_O_ABAJO:
			while( TRUE )
			{
				pulsador = getRegBits( PULSADOR_S3_PUERTO, PULSADOR_S3_MASCARA );
				if( pulsador == 0 )
				{
					// Si se detecta una opresion, se revisa que se mantenga durante cierto tiempo para darla por valida
					for( auxiliar = 0; auxiliar < MINIMO_DE_OPRESIONES_CUALQUIER_TECLA; auxiliar++ )
					{
						DemoraPorInterrupcion( MILISEGUNDOS_10 );
						pulsador = getRegBits( PULSADOR_S3_PUERTO, PULSADOR_S3_MASCARA );
						if( pulsador > 0 )
							break;
					}
					
					// Si el contador llego al maximo, la tecla se mantuvo presionada durante un tiempo valido
					if( auxiliar == MINIMO_DE_OPRESIONES_CUALQUIER_TECLA )
						// Se rompe el while
						break;
				}
				
				DemoraPorInterrupcion( MILISEGUNDOS_50 );
			}
			break;
			
		case TECLA_IZQUIERDA_O_ARRIBA:
			while( TRUE )
			{
				pulsador = getRegBits( PULSADOR_S2_PUERTO, PULSADOR_S2_MASCARA );
				if( pulsador == 0 )
				{
					// Si se detecta una opresion, se revisa que se mantenga durante cierto tiempo para darla por valida
					for( auxiliar = 0; auxiliar < MINIMO_DE_OPRESIONES_CUALQUIER_TECLA; auxiliar++ )
					{
						DemoraPorInterrupcion( MILISEGUNDOS_10 );
						pulsador = getRegBits( PULSADOR_S2_PUERTO, PULSADOR_S2_MASCARA );
						if( pulsador > 0 )
							break;
					}
					
					// Si el contador llego al maximo, la tecla se mantuvo presionada durante un tiempo valido
					if( auxiliar == MINIMO_DE_OPRESIONES_CUALQUIER_TECLA )
						// Se rompe el while
						break;
				}
				
				DemoraPorInterrupcion( MILISEGUNDOS_50 );
			}
			break;
			
			
		case TECLA_ACEPTAR:
			while( TRUE )
			{
				pulsador = getRegBits( PULSADOR_S1_PUERTO, PULSADOR_S1_MASCARA );
				if( pulsador == 0 )
				{
					// Si se detecta una opresion, se revisa que se mantenga durante cierto tiempo para darla por valida
					for( auxiliar = 0; auxiliar < MINIMO_DE_OPRESIONES_CUALQUIER_TECLA; auxiliar++ )
					{
						DemoraPorInterrupcion( MILISEGUNDOS_10 );
						pulsador = getRegBits( PULSADOR_S1_PUERTO, PULSADOR_S1_MASCARA );
						if( pulsador > 0 )
							break;
					}
					
					// Si el contador llego al maximo, la tecla se mantuvo presionada durante un tiempo valido
					if( auxiliar == MINIMO_DE_OPRESIONES_CUALQUIER_TECLA )
						// Se rompe el while
						break;
				}
				
				DemoraPorInterrupcion( MILISEGUNDOS_50 );
			}
			break;
	}
}



/****************************************************************/
/* RevisarOpresionDeTeclas										*/
/*  															*/
/*  Revisa el estado de los pulsdaores y devuelve un indicador  */
/*  de cual se oprimio											*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: El pulsador que se oprimio						*/
/****************************************************************/
estadosTeclas RevisarOpresionDeTeclas ( void )
{
	// Bucle perpetuo hasta esperar la opresion de algun pulsador
	while( TRUE )
	{
		
		// Se revisa el pulsador derecho
		pulsador = getRegBits( PULSADOR_S3_PUERTO, PULSADOR_S3_MASCARA );
		if( pulsador == 0 )
		{
			// Se revisa que haya pasado cierto tiempo desde la ultima opresion, antes de tomarlo como una nueva
			if( vecesSinRevisarTeclaDerecha > MINIMO_DE_VECES_SIN_REVISAR_OPRESION )
			{
				// Se borra el indicador
				vecesSinRevisarTeclaDerecha = 0;
				
				// Si se detecta una opresion, se revisa que se mantenga durante cierto tiempo para darla por valida
				for( auxiliar = 0; auxiliar < MINIMO_DE_OPRESIONES_CUALQUIER_TECLA; auxiliar++ )
				{
					DemoraPorInterrupcion( MILISEGUNDOS_10 );
					pulsador = getRegBits( PULSADOR_S3_PUERTO, PULSADOR_S3_MASCARA );
					if( pulsador > 0 )
						break;
				}
				
				// Si el contador llego al maximo, la tecla se mantuvo presionada durante un tiempo valido
				if( auxiliar == MINIMO_DE_OPRESIONES_CUALQUIER_TECLA )
					return( TECLA_DERECHA_O_ABAJO );
			}
		}
		
		
		// Se revisa el pulsador izquierdo
		pulsador = getRegBits( PULSADOR_S2_PUERTO, PULSADOR_S2_MASCARA );
		if( pulsador == 0 )
		{
			// Se revisa que haya pasado cierto tiempo desde la ultima opresion, antes de tomarlo como una nueva
			if( vecesSinRevisarTeclaIzquierda > MINIMO_DE_VECES_SIN_REVISAR_OPRESION )
			{
				// Se borra el indicador
				vecesSinRevisarTeclaIzquierda = 0;
				
				// Si se detecta una opresion, se revisa que se mantenga durante cierto tiempo para darla por valida
				for( auxiliar = 0; auxiliar < MINIMO_DE_OPRESIONES_CUALQUIER_TECLA; auxiliar++ )
				{
					DemoraPorInterrupcion( MILISEGUNDOS_10 );
					pulsador = getRegBits( PULSADOR_S2_PUERTO, PULSADOR_S2_MASCARA );
					if( pulsador > 0 )
						break;
				}
				
				// Si el contador llego al maximo, la tecla se mantuvo presionada durante un tiempo valido
				if( auxiliar == MINIMO_DE_OPRESIONES_CUALQUIER_TECLA )
					return( TECLA_IZQUIERDA_O_ARRIBA );
			}
		}
		
		
		// Se revisa el pulsador de aceptar
		pulsador = getRegBits( PULSADOR_S1_PUERTO, PULSADOR_S1_MASCARA );
		if( pulsador == 0 )
		{
			// Se revisa que haya pasado cierto tiempo desde la ultima opresion, antes de tomarlo como una nueva
			if( vecesSinRevisarTeclaAceptar > MINIMO_DE_VECES_SIN_REVISAR_OPRESION )
			{
				// Se borra el indicador
				vecesSinRevisarTeclaAceptar = 0;
				
				// Si se detecta una opresion, se revisa que se mantenga durante cierto tiempo para darla por valida
				for( auxiliar = 0; auxiliar < MINIMO_DE_OPRESIONES_CUALQUIER_TECLA; auxiliar++ )
				{
					DemoraPorInterrupcion( MILISEGUNDOS_10 );
					pulsador = getRegBits( PULSADOR_S1_PUERTO, PULSADOR_S1_MASCARA );
					if( pulsador > 0 )
						break;
				}
				
				// Si el contador llego al maximo, la tecla se mantuvo presionada durante un tiempo valido
				if( auxiliar == MINIMO_DE_OPRESIONES_CUALQUIER_TECLA )
					return( TECLA_ACEPTAR );
			}
		}
		
		
		// Se revisa el pulsador de cancelar
		pulsador = getRegBits( PULSADOR_S4_PUERTO, PULSADOR_S4_MASCARA );
		if( pulsador == 0 )
		{
			// Se revisa que haya pasado cierto tiempo desde la ultima opresion, antes de tomarlo como una nueva
			if( vecesSinRevisarTeclaCancelar > MINIMO_DE_VECES_SIN_REVISAR_OPRESION )
			{
				// Se borra el indicador
				vecesSinRevisarTeclaCancelar = 0;
				
				// Si se detecta una opresion, se revisa que se mantenga durante cierto tiempo para darla por valida
				for( auxiliar = 0; auxiliar < MINIMO_DE_OPRESIONES_CUALQUIER_TECLA; auxiliar++ )
				{
					DemoraPorInterrupcion( MILISEGUNDOS_10 );
					pulsador = getRegBits( PULSADOR_S4_PUERTO, PULSADOR_S4_MASCARA );
					if( pulsador > 0 )
						break;
				}
				
				// Si el contador llego al maximo, la tecla se mantuvo presionada durante un tiempo valido
				if( auxiliar == MINIMO_DE_OPRESIONES_CUALQUIER_TECLA )
					return( TECLA_CANCELAR );
				}
		}
		
		// Demora hasta revisar nuevamente los pulsadores 
		DemoraPorInterrupcion( MILISEGUNDOS_50 );
		
		// Se incrementan los contadores
		vecesSinRevisarTeclaAceptar++;
		vecesSinRevisarTeclaCancelar++;
		vecesSinRevisarTeclaDerecha++;
		vecesSinRevisarTeclaIzquierda++;
		
	}
}



// Funciones para prender o apagar los leds rojo y verde de la placa, que se usan para indicar el estado de las pruebas
void ApagarLedVerde ( void )	{	LED_VERDE_PIN = 0;	}
void ApagarLedRojo ( void )		{	LED_ROJO_PIN = 0;	}
void EncenderLedVerde ( void )	{	LED_VERDE_PIN = 1;	}
void EncenderLedRojo ( void )	{	LED_ROJO_PIN = 1;	}
void IndicarPruebaBien ( void )	{ EncenderLedVerde(); ApagarLedRojo(); }
void IndicarPruebaMal ( void )	{ ApagarLedVerde(); EncenderLedRojo(); }
void ApagarLeds ( void )		{ ApagarLedVerde(); ApagarLedRojo(); }


void AlternarLedRojo( void )
{
	if( LED_ROJO_PIN == 1 )
		ApagarLedRojo();
	else
		EncenderLedRojo();
}

void AlternarLedVerde( void )
{
	if( LED_VERDE_PIN == 1 )
		ApagarLedVerde();
	else
		EncenderLedVerde();
}





	/****************************************************************************************************************/
	/*  							FUNCIONES PARA LA PRUEBA REDUCIDA DEL MANILLAR DE CRIO							*/
	/****************************************************************************************************************/


/****************************************************************/
/* PruebaManillarReducido										*/
/*  															*/
/*  Ejecuta una secuencia de prueba para verificar el correcto	*/
/*  funcionamiento del manillar de crio. A medida que se va		*/
/*  realizando la prueba, va informando el estado en el LCD.	*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: NFALSE, si hubo algun error						*/
/****************************************************************/
bool PruebaManillarReducido ( void )
{
	// Variable para saber si hubo algun error
	bool errorEnLaPrueba;
	
	// Se borra el flag
	errorEnLaPrueba = FALSE;
	
	// Se borra el LCD para indicar el estado de la prueba
	BorrarLCD();
	
	// Se ejecuta la secuencia de medicion de los pernos
	errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 20, "Pernos:  -----------" );
	estadoPernos = MedicionDeLosPernos_ManillarCrio();
	switch( estadoPernos )
	{
		case PERNO_SIN_ERRORES:
			errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 9, 11, "TODO OK    " );
			break;
		case PERNO_ABIERTO:
			InformarPernosAbiertos( MANILLAR_CRIO );
			errorEnLaPrueba = TRUE;
			break;
		case PERNO_CORTO_CIRCUITO:
			InformarPernosEnCorto( MANILLAR_CRIO );
			errorEnLaPrueba = TRUE;
			break;
		case PERNO_SIN_ALTERNANCIA:
			InformarMalUbicacion( MANILLAR_CRIO );
			errorEnLaPrueba = TRUE;
			break;
		case PERNO_CORTO_CIRCUITO_CARCAZA:
			InformarCortoConLaCarcaza( MANILLAR_CRIO );
			errorEnLaPrueba = TRUE;
			break;
		case PERNO_CORTOCIRCUITO_CON_5V:
			errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 20, "Pernos: Corto con 5V" );
			errorEnLaPrueba = TRUE;
			break;
	}

	
	// Se ejecuta la secuencia de medicion de los LEDS
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 20, "LEDS:    -----------" );
	estadoLEDs = MedicionDeLosLEDS();
	switch( estadoLEDs )
	{
		case LEDS_SIN_ERRORES:
			errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 9, 11, "TODO OK    " );
			break;
		case LEDS_UNA_RAMA:
			errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 9, 11, "*1 RAMA    " );
			errorEnLaPrueba = TRUE;
			break;
		case LEDS_NINGUNA_RAMA:
			errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 9, 11, "*MAL       " );
			errorEnLaPrueba = TRUE;
			break;
	}

	
	// Se ejecuta la secuencia de medicion del sensor de temperatura
	estadoSensor = MedicionDelSensorDeTemperaturaManillar();
	switch( estadoSensor )
	{
		case SENSOR_SIN_ERRORES:
			// Se ejecuta la secuencia de medicion de la Peltier, solo cuando se detecta que el sensor esta bien
			errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 20, "Lado ca:  ----------" );
			errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 	   0, 20, "Pelt:  -------------" );
			estadoPeltier = MedicionDeLaPeltier();
			
			// Luego de realizar la medicion, sea cual fuera el resultado, se apaga la celda Peltier
			DeshabilitarPeltier();
			
			switch( estadoPeltier )
			{
				case PELTIER_INVERTIDA:
					EscribirMensajeLCD( RENGLON_INFERIOR, 9, 11, "*INVERTIDA " );
					errorEnLaPrueba = TRUE;
					break;
				case PELTIER_NO_ENFRIA:
					EscribirMensajeLCD( RENGLON_INFERIOR, 9, 11, "*NO ENFRIA " );
					errorEnLaPrueba = TRUE;
					break;
				case PELTIER_ENFRIA_DEMASIADO:
					EscribirMensajeLCD( RENGLON_INFERIOR, 9, 11, "*ENFRIA DEM" );
					errorEnLaPrueba = TRUE;
					break;
				case PELTIER_SOBRE_CONSUMO:
					EscribirMensajeLCD( RENGLON_INFERIOR, 9, 11, "*SOBRE CONS" );
					errorEnLaPrueba = TRUE;
					break;
				case PELTIER_APAGADA:
					EscribirMensajeLCD( RENGLON_INFERIOR, 9, 11, "*APAGADA   " );
					errorEnLaPrueba = TRUE;
					break;
				case PELTIER_SE_DESCONECTO:
					EscribirMensajeLCD( RENGLON_INFERIOR, 9, 11, "*SE DESCONE" );
					errorEnLaPrueba = TRUE;
					break;
			}
			
			break;
			
		// Cualquier error que haga que no se pueda leer el sensor de la Peltier, obliga a que no se la pueda medir
		case SENSOR_ERROR_ACTUALIZAR_LISTADO:
		case SENSOR_ERROR_NO_AGREGADO:
		case SENSOR_ERROR_TEMPERATURA:
			errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 20, "Sensor:  *NO RESPOND" );
			errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 0, 20, "Peltier: -SIN MEDIR-" );
			break;
	}
	
	// Se indica mediante los LEDs de la placa, el estado de la prueba
	if( errorEnLaPrueba == FALSE )
		IndicarPruebaBien();
	else
		IndicarPruebaMal();
	
	DemoraEnSegundos( 1 );
	
	// Si se registro correctamente el descenso de temperatura de la Peltier, se muestra una pantalla informativa
	if( estadoSensor == SENSOR_SIN_ERRORES && estadoPeltier == PELTIER_SIN_ERRORES )
	{
		// Se borra el LCD
		BorrarLCD();
		
		// Se muestran los datos del lado frio
		errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 20, "Frio       ->       " );
		EscribirTemperatura( medicionesTemperaturaLadoFrio[ 0 ], RENGLON_SUPERIOR, 5 );
		EscribirTemperatura( medicionesTemperaturaLadoFrio[ 1 ], RENGLON_SUPERIOR, 14 );
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 20, "Bajo                " );
		EscribirTemperatura( variacionDeTemperaturaLadoFrio, RENGLON_MEDIO_SUPERIOR, 7 );
		
		// Se muestran los datos del lado caliente
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 20, "Cal.       ->       " );
		EscribirTemperatura( medicionesTemperaturaLadoCaliente[ 0 ], RENGLON_MEDIO_INFERIOR, 5 );
		EscribirTemperatura( medicionesTemperaturaLadoCaliente[ 1 ], RENGLON_MEDIO_INFERIOR, 14 );
		errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 0, 20, "Subio               " );
		EscribirTemperatura( variacionDeTemperaturaLadoCaliente, RENGLON_INFERIOR, 7 );
		
		
	}
	
	return (errorEnLaPrueba);
}



/****************************************************************/
/* MedicionDeLosPernos_ManillarCrio								*/
/*  															*/
/*  Primero se registra la matriz de continuidad de todos los   */
/*  contactos. Se cuentan los pernos que den continuidad con	*/
/*  cada contacto desde la placa. Si una fila da una cuenta		*/
/*  nula, entonces el perno correspondiente esta abierto. Si el */
/*  contador da mas de 1, es porque el perno esta en			*/
/*  cortocircuito con otros.									*/
/*  Luego de tener la certeza que todos los pernos estan bien,	*/
/*  se verifica la alternancia. Para esto, primero se			*/
/*  identifica la posicion del perno 2, exitandolo desde el		*/
/*  conector de la placa y buscando que perno registra			*/
/*  continuidad. Sabiendo las posiciones de to-dos los pernos,	*/
/*  se verifica la alternancia a traves de la matriz de			*/
/*  continuidad.												*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
resultadoPernos MedicionDeLosPernos_ManillarCrio( void )
{
	// Variables auxiliares
	char indiceContactoPernos;
	char indiceContactoPlaca;
	char contactos;
	char totalDePernosAbiertos;
	char totalDeCortocircuitos;
	char totalDeCortocircuitosMazo;
	char paresCorrectamenteAlternados;
	bool cortocircuitoConElMazo;
	bool errorAlternancia;
	
	// Se borran los contadores
	errorAlternancia = FALSE;

	
	// Se levanta la matriz de contactos de los pernos
	ObtenerMatrizDePernos( MANILLAR_CRIO );
	
	// Primero se revisa que ningun perno tenga continuidad de por si con la linea de 5V
	if( hayPernoEnCortoCon5V == TRUE )
		return( PERNO_CORTOCIRCUITO_CON_5V );
	
	// Luego se verifica que no haya ningun perno en contacto con la carcaza
	if( pernosUbicacion[ UBICACION_CONTACTO_CARCAZA ] == HAY_CONTINUIDAD )
		return( PERNO_CORTO_CIRCUITO_CARCAZA );
	
	// Se revisa el estado de los contactos para diagnosticar los que esten abiertos y los que esten en corto
	for( indiceContactoPlaca = 1; indiceContactoPlaca < TOTAL_DE_PERNOS_MANILLAR_CRIO + 1; indiceContactoPlaca++ )
	{
		// Primero se revisa que no este abierto
		if( pernos[ indiceContactoPlaca ].totalDeContactos == 0 )
			{	condicionDelPerno[ indiceContactoPlaca ] = PERNO_ABIERTO;	hayPernoAbierto = TRUE;	}
		
		// Luego se verifica que no este en cortocircuito
		else
			if( pernos[ indiceContactoPlaca ].totalDeContactos > 1 )
				{	condicionDelPerno[ indiceContactoPlaca ] = PERNO_CORTO_CIRCUITO;	hayPernoEnCorto = TRUE;	}
	}
	
	// Si hay algun perno abierto o en cortocircuito, no se prosigue con la verificacion de las ubicaciones
	if( hayPernoAbierto == TRUE )
		return( PERNO_ABIERTO );
	if( hayPernoEnCorto == TRUE )
		return( PERNO_CORTO_CIRCUITO );

	// Luego de verificar que los pernos esten bien, resta ver la alternancia, verificando que cada uno este en la posicion que deberia
	switch( pernosUbicacion[ 2 ] )
	{
		case ROTADO_0_POSICIONES:
			if( pernosUbicacion[ 2 ] != VALOR_J2_ROTADO_0_POSICIONES )
				{	condicionDelPerno[ 2 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 3 ] != VALOR_J3_ROTADO_0_POSICIONES )
				{	condicionDelPerno[ 3 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 4 ] != VALOR_J4_ROTADO_0_POSICIONES )
				{	condicionDelPerno[ 4 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 5 ] != VALOR_J5_ROTADO_0_POSICIONES )
				{	condicionDelPerno[ 5 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 6 ] != VALOR_J6_ROTADO_0_POSICIONES )
				{	condicionDelPerno[ 6 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 7 ] != VALOR_J7_ROTADO_0_POSICIONES )
				{	condicionDelPerno[ 7 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			break;
			
		case ROTADO_1_POSICIONES:
			if( pernosUbicacion[ 2 ] != VALOR_J2_ROTADO_1_POSICIONES )
				{	condicionDelPerno[ 2 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 3 ] != VALOR_J3_ROTADO_1_POSICIONES )
				{	condicionDelPerno[ 3 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 4 ] != VALOR_J4_ROTADO_1_POSICIONES )
				{	condicionDelPerno[ 4 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 5 ] != VALOR_J5_ROTADO_1_POSICIONES )
				{	condicionDelPerno[ 5 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 6 ] != VALOR_J6_ROTADO_1_POSICIONES )
				{	condicionDelPerno[ 6 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 7 ] != VALOR_J7_ROTADO_1_POSICIONES )
				{	condicionDelPerno[ 7 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			break;
			
		case ROTADO_2_POSICIONES:
			if( pernosUbicacion[ 2 ] != VALOR_J2_ROTADO_2_POSICIONES )
				{	condicionDelPerno[ 2 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 3 ] != VALOR_J3_ROTADO_2_POSICIONES )
				{	condicionDelPerno[ 3 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 4 ] != VALOR_J4_ROTADO_2_POSICIONES )
				{	condicionDelPerno[ 4 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 5 ] != VALOR_J5_ROTADO_2_POSICIONES )
				{	condicionDelPerno[ 5 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 6 ] != VALOR_J6_ROTADO_2_POSICIONES )
				{	condicionDelPerno[ 6 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 7 ] != VALOR_J7_ROTADO_2_POSICIONES )
				{	condicionDelPerno[ 7 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			break;
			
		case ROTADO_3_POSICIONES:
			if( pernosUbicacion[ 2 ] != VALOR_J2_ROTADO_3_POSICIONES )
				{	condicionDelPerno[ 2 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 3 ] != VALOR_J3_ROTADO_3_POSICIONES )
				{	condicionDelPerno[ 3 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 4 ] != VALOR_J4_ROTADO_3_POSICIONES )
				{	condicionDelPerno[ 4 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 5 ] != VALOR_J5_ROTADO_3_POSICIONES )
				{	condicionDelPerno[ 5 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 6 ] != VALOR_J6_ROTADO_3_POSICIONES )
				{	condicionDelPerno[ 6 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 7 ] != VALOR_J7_ROTADO_3_POSICIONES )
				{	condicionDelPerno[ 7 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			break;
			
		case ROTADO_4_POSICIONES:
			if( pernosUbicacion[ 2 ] != VALOR_J2_ROTADO_4_POSICIONES )
				{	condicionDelPerno[ 2 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 3 ] != VALOR_J3_ROTADO_4_POSICIONES )
				{	condicionDelPerno[ 3 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 4 ] != VALOR_J4_ROTADO_4_POSICIONES )
				{	condicionDelPerno[ 4 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 5 ] != VALOR_J5_ROTADO_4_POSICIONES )
				{	condicionDelPerno[ 5 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 6 ] != VALOR_J6_ROTADO_4_POSICIONES )
				{	condicionDelPerno[ 6 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 7 ] != VALOR_J7_ROTADO_4_POSICIONES )
				{	condicionDelPerno[ 7 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			break;
			
		case ROTADO_5_POSICIONES:
			if( pernosUbicacion[ 2 ] != VALOR_J2_ROTADO_5_POSICIONES )
				{	condicionDelPerno[ 2 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 3 ] != VALOR_J3_ROTADO_5_POSICIONES )
				{	condicionDelPerno[ 3 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 4 ] != VALOR_J4_ROTADO_5_POSICIONES )
				{	condicionDelPerno[ 4 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 5 ] != VALOR_J5_ROTADO_5_POSICIONES )
				{	condicionDelPerno[ 5 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 6 ] != VALOR_J6_ROTADO_5_POSICIONES )
				{	condicionDelPerno[ 6 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			if( pernosUbicacion[ 7 ] != VALOR_J7_ROTADO_5_POSICIONES )
				{	condicionDelPerno[ 7 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
			break;
			
		// Si el perno central no esta ubicado correctamente, se marca el error
		case INVERTIDO_J1_J2:
			condicionDelPerno[ 1 ] = PERNO_SIN_ALTERNANCIA;
			condicionDelPerno[ 2 ] = PERNO_SIN_ALTERNANCIA;
			return( PERNO_SIN_ALTERNANCIA );
			break;
	}
	
	// Luego de revisar todos los pernos, resta verificar el perno central, que siempre deberia estar en la misma posicion
	if( pernosUbicacion[ 1 ] != VALOR_J1_ROTADO_X_POSICIONES )
		{	condicionDelPerno[ 1 ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
	
	// Si algun perno esta en una mala ubicacion, se lo indica
	if( errorAlternancia == TRUE )
		return( PERNO_SIN_ALTERNANCIA );
	
	// Si llega aqui, es que no hay ningun error con los pernos
	return( PERNO_SIN_ERRORES );

}



/****************************************************************/
/* MedicionDeLosLEDS											*/
/*  															*/
/*  Activa el circuito de alimentacion de los LEDs y mide la	*/
/*  corriente consumida.										*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Un indicador del resultado de la medicion			*/
/****************************************************************/
resultadoLEDs MedicionDeLosLEDS ( void )
{
	// Variable auxiliar para almacenar el posible error en la medicion del ADC
	bool error;
	
	// Primero se debe habilitar la alimentacion de los LEDs
	HabilitarLEDs();
	
	// Se debe revisar el valor de la corriente
	error = TomarMuestraSimpleADC( ADC_CANAL_LEDS );
	
	// Luego de leer la corriente, se desactiva la alimentacion de los LEDs
	DeshabilitarLEDs();
	
	// Si hubo un error en la medicion, se devuelve el indicador de ninguna rama
	if( error == FALSE )
		return( LEDS_NINGUNA_RAMA );
	
	// Se verifica el valor de las cuentas obtenidas
	if( cuentasADC > MINIMAS_CUENTRAS_LEDS_2_RAMAS )
		return( LEDS_SIN_ERRORES );
	else
		if( cuentasADC > MINIMAS_CUENTRAS_LEDS_1_RAMA )
			return( LEDS_UNA_RAMA );
		else
			return( LEDS_NINGUNA_RAMA );

}



/****************************************************************/
/* MedicionDelSensorDeTemperaturaManillar						*/
/*  															*/
/*  Actualiza el listado de sensores para agregar el del		*/
/*  manillar. Revisa que no tenga ningun error.					*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: El cogido de error, si es que lo hay				*/
/****************************************************************/
resultadoSensor MedicionDelSensorDeTemperaturaManillar ( void )
{
	// Variable auxiliar
	char indiceByte;
	
	// Se borran los datos previos del sensor de temperatura del manillar
	sensores[ SENSOR_MANILLAR_O_EXTRA ].temperatura = 0;
	sensores[ SENSOR_MANILLAR_O_EXTRA ].usado = FALSE;
	for( indiceByte = 0; indiceByte < LONGITUD_ROM_BYTES; indiceByte ++ )
		sensores[ SENSOR_MANILLAR_O_EXTRA ].ROM[ indiceByte ] = 0;
	
	// Hay que listar todos los sensores de temperatura que haya conectados
	if( ActualizarListadoDeSensores() == FALSE )
		return( SENSOR_ERROR_ACTUALIZAR_LISTADO );
	
	// Se deberia encontrar uno que no este registrado, que es el del manillar
	if( sensores[ SENSOR_MANILLAR_O_EXTRA ].usado == FALSE )
		return( SENSOR_ERROR_NO_AGREGADO );

	return( SENSOR_SIN_ERRORES );
}



/****************************************************************/
/* MedicionDeLaPeltier											*/
/*  															*/
/*  Se habilita la alimentacion del modulo, que tiene			*/
/*  incorporada una demora para dejar pasar el transitorio y	*/
/*  que la corriente se estabilice.								*/
/*  Se lee el valor en que se establece la corriente para ver	*/
/*  que no sea muy elevado.										*/
/*  Se toma la temperatura inicial de la celda y su valor luego */
/*  de 3 segundos. Si el lado frio ascendio o el lado caliente  */
/*  descendio, la celda esta invertida.							*/
/*  Si la Peltier esta bien colocada, se realiza un loop de		*/
/*  varias mediciones donde se registra la corriente y la 		*/
/*  temperatura a cada segundo. Si la corriente indica un		*/
/*  sobreconsumo o que la celda no esta conectada, se corta el	*/
/*  loop y se informa el error.									*/
/*  Luego de las mediciones, se toman las diferencias de la		*/
/*  temperatura final respecto de la inicial para ver que la	*/
/*  celda haya enfriado correctamente.							*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
resultadoPeltier MedicionDeLaPeltier ( void )
{
	// Variable solo para las pruebas
	int		corriente_mA;
	
	// Se acciona el circuito de habilitacion de la Peltier
	HabilitarPeltier();
	
	errorGeneral = TomarMuestraSimpleADC( ADC_CANAL_PELTIER );
	if( cuentasADC > MAXIMAS_CUENTRAS_PARA_PELTIER )
		return( PELTIER_SOBRE_CONSUMO );
	if( cuentasADC < MINIMAS_CUENTAS_PELTIER_ENCENDIDA )
		return( PELTIER_APAGADA );
	
	// Si el valor de corriente es aceptable, se lo toma como la corriente inicial
	corriente_mA = cuentasADC * RESOLUCION_MA_PELTIER;
	EscribirCorriente( corriente_mA, RENGLON_INFERIOR, 16 );
	medicionesCorrientePeltier[ 0 ] = corriente_mA;
	
	// Se imprimen las temperaturas iniciales
	errorSensorDS18S20 = LeerTodasLasTemperaturasDS18S20();
	
	medicionesTemperaturaLadoFrio[ 0 ] = sensores[ SENSOR_MANILLAR_O_EXTRA ].temperatura;
	EscribirTemperatura( medicionesTemperaturaLadoFrio[ 0 ], RENGLON_INFERIOR, 9 );
	
	medicionesTemperaturaLadoCaliente[ 0 ] = sensores[ SENSOR_PARA_LADO_CALIENTE ].temperatura;
	EscribirTemperatura( medicionesTemperaturaLadoCaliente[0], RENGLON_MEDIO_INFERIOR, 9 );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 15, 6, "      " );
	
	// Se genera una demora inicial para esperar a que la Peltier comienze a enfriar correctamente
	DemoraEnSegundos( 3 );
	
	// Luego de esa demora se verifica que la celda no este conectada de forma inversa
	errorSensorDS18S20 = LeerTodasLasTemperaturasDS18S20();
	medicionesTemperaturaLadoFrio[ 1 ] = sensores[ SENSOR_MANILLAR_O_EXTRA ].temperatura;
	variacionDeTemperaturaLadoFrio = medicionesTemperaturaLadoFrio[ 0 ] - medicionesTemperaturaLadoFrio[ 1 ];
	medicionesTemperaturaLadoCaliente[ 1 ] = sensores[ SENSOR_PARA_LADO_CALIENTE ].temperatura;
	variacionDeTemperaturaLadoCaliente = medicionesTemperaturaLadoCaliente[ 0 ] - medicionesTemperaturaLadoCaliente[ 1 ];
	
	if( variacionDeTemperaturaLadoFrio < 0 || variacionDeTemperaturaLadoCaliente > 0 )
		return( PELTIER_INVERTIDA );
	
	setReg8( TPM2SC, 0x0E);    /* Set prescaler PS = 64; and run counter */
	
	// Bucle para tomar la corriente y la temperatura a cada segundo durante X segundos
	for( indiceMedicion = 0; indiceMedicion < MEDICIONES_PELTIER; indiceMedicion++ )
	{
		// Se resetea el contador del timer 2
		TPM2CNT = 0;
		
		// Se verifica que no sobrepase la corriente
		errorGeneral = TomarMuestraSimpleADC( ADC_CANAL_PELTIER ); 
		if( cuentasADC > MAXIMAS_CUENTRAS_PARA_PELTIER )
			return( PELTIER_SOBRE_CONSUMO );
		if( cuentasADC < MINIMAS_CUENTAS_PELTIER_ENCENDIDA )
			return( PELTIER_SE_DESCONECTO );
		
		corriente_mA = cuentasADC * RESOLUCION_MA_PELTIER;
//		EscribirCorriente( corriente_mA, RENGLON_INFERIOR, 16 );
		medicionesCorrientePeltier[ 1 ] = corriente_mA;
		
		// Luego se verifica la temperatura
		errorSensorDS18S20 = LeerTodasLasTemperaturasDS18S20();
		medicionesTemperaturaLadoFrio[ 1 ] = sensores[ SENSOR_MANILLAR_O_EXTRA ].temperatura;
		medicionesTemperaturaLadoCaliente[ 1 ] = sensores[ SENSOR_PARA_LADO_CALIENTE ].temperatura;
		
		// Se muestran en el LCD
		EscribirTemperatura( medicionesTemperaturaLadoFrio[ 1 ], RENGLON_INFERIOR, 9 );
		EscribirTemperatura( medicionesTemperaturaLadoCaliente[ 1 ], RENGLON_MEDIO_INFERIOR, 7 );

		// Se calcula el descenso de temperatura
		acumulado_ladoFrio = medicionesTemperaturaLadoFrio[ 1 ] - medicionesTemperaturaLadoFrio[ 0 ];
		EscribirTemperatura( acumulado_ladoFrio, RENGLON_INFERIOR, 14 );
		
		// Se envian los datos a la computadora
		EnviarTramaPruebaReducida_ManillarCrio();
		
		// Se demora hasta completar 1 segundo
		while( TPM2CNT < 0xEA60 )
			{ asm(nop); };
	}
	
	// Se toma la diferencia critica como la medicion inicial menos la final del lado frio
	variacionDeTemperaturaLadoFrio = medicionesTemperaturaLadoFrio[ 0 ] - medicionesTemperaturaLadoFrio[ 1 ];
	
	// Si la diferencia es negativa y superior a 2 grados, la celda esta colocada al reves
	if( variacionDeTemperaturaLadoFrio < TEMPERATURA_MINIMA_PELTIER_INVERTIDA )
		return( PELTIER_INVERTIDA );
	
	// Si la diferencia no es mayor a 1 grado, la celda esta apagada
	if( variacionDeTemperaturaLadoFrio < TEMPERATURA_MINIMA_PELTIER_APAGADA )
		return( PELTIER_APAGADA );
	
	// Se registran las variaciones entre ambos lados
	variacionDeTemperaturaLadoCaliente = medicionesTemperaturaLadoCaliente[ 1 ] - medicionesTemperaturaLadoCaliente[ 0 ];
	variacionInicialDeTemperaturaEntreLados = medicionesTemperaturaLadoCaliente[ 0 ] - medicionesTemperaturaLadoFrio[ 0 ];
	variacionFinalDeTemperaturaEntreLados = medicionesTemperaturaLadoCaliente[ 1 ] - medicionesTemperaturaLadoFrio[ 1 ];
	
	// Si llega aca, es porque la celda esta correcta
	return( PELTIER_SIN_ERRORES );
}





	/****************************************************************************************************************/
	/*  								FUNCIONES PARA LA PRUEBA DEL SENSOR EXTRA									*/
	/****************************************************************************************************************/


/****************************************************************/
/* RegistrarElSensorDeTemperaturaExtra							*/
/*  															*/
/*  Realiza la enumeracion de los dispositivos en el bus y		*/
/*  agrega el nuevo sensor en la posicion del sensor extra		*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
resultadoSensor RegistrarElSensorDeTemperaturaExtra ( void )
{
	// Variable auxiliar
	char indiceByte;
	
	// Se borran los datos previos del sensor de temperatura extra
	sensores[ SENSOR_MANILLAR_O_EXTRA ].usado = FALSE;
	sensores[ SENSOR_MANILLAR_O_EXTRA ].temperatura = 0;
	for( indiceByte = 0; indiceByte < LONGITUD_ROM_BYTES; indiceByte++ )
		sensores[ SENSOR_MANILLAR_O_EXTRA ].ROM[ indiceByte ] = 0;
	
	// Hay que listar todos los sensores de temperatura que haya conectados
	if( ActualizarListadoDeSensores() == FALSE )
		return( SENSOR_ERROR_ACTUALIZAR_LISTADO );
	
	// Se deberia encontrar uno que no este registrado, que es el del manillar
	if( sensores[ SENSOR_MANILLAR_O_EXTRA ].usado == FALSE )
		return( SENSOR_ERROR_NO_AGREGADO );
	
	return( SENSOR_SIN_ERRORES );
}





	/****************************************************************************************************************/
	/*  							FIRMWARE PARA CONTOLAR LOS MULTIPLEXORES DE LOS PERNOS							*/
	/****************************************************************************************************************/


/****************************************************************/
/* ObtenerMatrizDePernos										*/
/*  															*/
/*  Levanta la matriz de contactos de los pernos. Solo se marca	*/
/*  si hay continuidad entre las direcciones utilizadas y la	*/
/*  cantidad de contactos para cada direccion.					*/
/*  															*/
/*  Recibe: El target, para saber la cantidad de pernos			*/
/*  Devuelve: Nada												*/
/****************************************************************/
void ObtenerMatrizDePernos ( char target )
{
	// Variables auxiliares
	char indiceContactoPernos;
	char indiceContactoPlaca;
	char totalDeContactos;
	
	// Se selecciona el total de contactos en funcion del target
	switch( target )
	{
		/* *** ******************** Manillares ******************** *** */
		case MANILLAR_CRIO:
			totalDeContactos = TOTAL_DE_PERNOS_MANILLAR_CRIO + 1 + 1;	// Se incrementa en 1 para contemplar el contacto de la carcaza
			break;
			
		case MANILLAR_ENYGMA:
			totalDeContactos = TOTAL_DE_PERNOS_MANILLAR_ENYGMA + 1;	// Se incrementa en 1 para evitar el subindice 0
			break;
			
			
		/* *** ******************** Cables ******************** *** */
		case CABLE_MAZO_66A:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_66A + 1;
			break;
	}
	
	// Se borran los contadores
	hayPernoAbierto = FALSE;
	hayPernoEnCorto = FALSE;
	hayPernoEnCortoCon5V = FALSE;
	hayPernoEnCortoElMazo = FALSE;
	hayPernoAlternado = FALSE;
	
	// Se borran los indicadores
	for( indiceContactoPlaca = 1; indiceContactoPlaca < totalDeContactos; indiceContactoPlaca++ )
	{
		condicionDelPerno[ indiceContactoPlaca ] = PERNO_SIN_ERRORES;
		pernosUbicacion[ indiceContactoPlaca ] = 0;
		pernos[ indiceContactoPlaca ].totalDeContactos = 0;
		for( indiceContactoPernos = 1; indiceContactoPernos < totalDeContactos; indiceContactoPernos++ )
			pernos[ indiceContactoPlaca ].contactos[ indiceContactoPernos ] = NO_HAY_CONTINUIDAD;
	}
	
	
	// Antes de realizar la prueba de continuidad de todos los pernos, se revisa que ninguno mida algo con el multiplexor de la placa deshabilitado
	E_MULTIPLEXOR_PLACA_PIN = 1;			// Se deshabilita el multiplexor de la placa
	E_MULTIPLEXOR_PERNOS_PIN = 0;			// Se habilita el multiplexor de los pernos
	for( indiceContactoPernos = 1; indiceContactoPernos < totalDeContactos; indiceContactoPernos++ )
	{
		ColocarDireccionMultiplexorPernos( indiceContactoPernos, target );
		errorGeneral = TomarMuestraSimpleADC( ADC_CANAL_PERNOS ); 
		if( cuentasADC > MINIMAS_CUENTRAS_PARA_CONTINUIDAD )
		{
			// Se detecto continuidad
			condicionDelPerno[ indiceContactoPernos ] = PERNO_CORTOCIRCUITO_CON_5V;
			hayPernoEnCortoCon5V = TRUE;
		}
	}
	
	// Si alguno de los pernos tiene continuidad de por si con 5V, la prueba de medicion no continua
	if( hayPernoEnCortoCon5V == TRUE )
		return;
	
	
	// Se deben habilitar los 2 Enables de los multiplexores
	E_MULTIPLEXOR_PLACA_PIN = 0;
	E_MULTIPLEXOR_PERNOS_PIN = 0;
	
	// Se deja fija la direccion de uno de los multiplexores y se varian todas las del otro
	for( indiceContactoPlaca = 1; indiceContactoPlaca < totalDeContactos; indiceContactoPlaca++ )
	{
		// Se toman como fijas las posiciones del multiplexor de la placa, ya que el conector que va aca esta correctamente armado
		ColocarDireccionMultiplexorPlaca( indiceContactoPlaca, target );
		
		// Se levanta una columna de la matriz de contactos
		for( indiceContactoPernos = 1; indiceContactoPernos < totalDeContactos; indiceContactoPernos++ )
		{
			// Los contactos desde el lado de los pernos pueden estar en ubicaciones incorrectas
			ColocarDireccionMultiplexorPernos( indiceContactoPernos, target );
			
			// Si hay continuidad, la salida del multiplexor PERNOS_SALIDA_CONTINUIDAD tiene que tener un valor de tension distinto de cero.
			errorGeneral = TomarMuestraSimpleADC( ADC_CANAL_PERNOS ); 
			
			if( cuentasADC > MINIMAS_CUENTRAS_PARA_CONTINUIDAD )									// Se detecto continuidad
			{
				pernos[ indiceContactoPlaca ].contactos[ indiceContactoPernos ] = HAY_CONTINUIDAD;
				pernos[ indiceContactoPlaca ].totalDeContactos++;
				// Ademas, se registra en que ubicacion se encuentra
				pernosUbicacion[ indiceContactoPlaca ] = indiceContactoPernos;
			}
			else																					// No se detecto continuidad
				pernos[ indiceContactoPlaca ].contactos[ indiceContactoPernos ] = NO_HAY_CONTINUIDAD;
		}
	}
	
	// Se deben deshabilitar los 2 Enables de los multiplexores
	E_MULTIPLEXOR_PLACA_PIN = 1;
	E_MULTIPLEXOR_PERNOS_PIN = 1;
	
	return;

}



/****************************************************************/
/* ColocarDireccionMultiplexorPernos							*/
/*  															*/
/*  Se encarga de modificar los pines del multiplexor para los	*/
/*  contactos de los pernos, para poder seleccionar la			*/
/*  direccion suministrada en funcion del target.				*/
/*  															*/
/*  Recibe: La direccion a colocar								*/
/*  Devuelve: Nada												*/
/****************************************************************/
void ColocarDireccionMultiplexorPernos( char direccion, char target )
{
	// Variable auxiliar
	char auxiliar;
	
	/* *** Se traduce la direccion logica solicitada a la direccion fisica necesaria en los pines segun el target *** */
	switch( target )
	{
		/* *** ******************** Manillares ******************** *** */
		case MANILLAR_CRIO:
			switch( direccion )
			{
				case 1: 	direccion = 0; 		break;
				case 2: 	direccion = 1; 		break;
				case 3: 	direccion = 2; 		break;
				case 4: 	direccion = 3; 		break;
				case 5: 	direccion = 4; 		break;
				case 6: 	direccion = 5; 		break;
				case 7: 	direccion = 6; 		break;
			}
			break;
			
		case MANILLAR_ENYGMA:
			switch( direccion )
			{
				case 1: 	direccion = 0; 		break;
				case 2: 	direccion = 1; 		break;
				case 3: 	direccion = 2; 		break;
				case 4: 	direccion = 3; 		break;
			}
			break;
	
			
		/* *** ******************** Cables ******************** *** */
		case CABLE_MAZO_66A:
			switch( direccion )
			{
				case 1: 	direccion = 2; 		break;		// Brown
				case 2: 	direccion = 4; 		break;		// Light blue
				case 3: 	direccion = 5; 		break;		// Pink
				case 4: 	direccion = 3; 		break;		// Orange
				case 5: 	direccion = 1; 		break;		// Yellow
				case 6: 	direccion = 0; 		break;		// Green
				case 7: 	direccion = 6; 		break;		// Blue
				
				case 8: 	direccion = 12; 	break;		// Violet
				case 9: 	direccion = 11; 	break;		// Red
				case 10: 	direccion = 9; 		break;		// Shield
				case 11: 	direccion = 10; 	break;		// Wire
				case 12: 	direccion = 15; 	break;		// Red - Peltier
				case 13: 	direccion = 0; 		break;		// Green
				case 14: 	direccion = 14; 	break;		// Black - Peltier
			}
			break;
			
	}
	
	// Se modifican los pines para colocar la direccion solicitada
	auxiliar = direccion;
	auxiliar &= 0x01;
	if( auxiliar > 0 )
	{
		// Se setea el pin S0 de los pernos
		S0_MULTIPLEXOR_PERNOS_PIN = 1;
	}
	else
	{
		// Se borra el pin S0 de los pernos
		S0_MULTIPLEXOR_PERNOS_PIN = 0;
	}
	
	auxiliar = direccion >> 1;
	auxiliar &= 0x01;
	if( auxiliar > 0 )
	{
		// Se setea el pin S1 de los pernos
		S1_MULTIPLEXOR_PERNOS_PIN = 1;
	}
	else
	{
		// Se borra el pin S1 de los pernos
		S1_MULTIPLEXOR_PERNOS_PIN = 0;
	}
	
	auxiliar = direccion >> 2;
	auxiliar &= 0x01;
	if( auxiliar > 0 )
	{
		// Se setea el pin S2 de los pernos
		S2_MULTIPLEXOR_PERNOS_PIN = 1;
	}
	else
	{
		// Se borra el pin S2 de los pernos
		S2_MULTIPLEXOR_PERNOS_PIN = 0;
	}
	
	auxiliar = direccion >> 3;
	auxiliar &= 0x01;
	if( auxiliar > 0 )
	{
		// Se setea el pin S3 de los pernos
		S3_MULTIPLEXOR_PERNOS_PIN = 1;
	}
	else
	{
		// Se borra el pin S3 de los pernos
		S3_MULTIPLEXOR_PERNOS_PIN = 0;
	}
	
	// Se genera una pequenia demora para asegurarse que los multiplexores propaguen correctamente las seniales
	DEMORA_10US;
}



/****************************************************************/
/* ColocarDireccionMultiplexorPlaca								*/
/*  															*/
/*  Se encarga de modificar los pines del multiplexor para los	*/
/*  contactos de la placa, para poder seleccionar la direccion	*/
/*  suministrada en funcion del target.							*/
/*  															*/
/*  Recibe: La direccion a colocar								*/
/*  Devuelve: Nada												*/
/****************************************************************/
void ColocarDireccionMultiplexorPlaca( char direccion, char target )
{
	// Variable auxiliar
	char auxiliar;
	
	/* *** Se traduce la direccion logica solicitada a la direccion fisica necesaria en los pines segun el target *** */
	switch( target )
	{
		/* *** ******************** Manillares ******************** *** */
		case MANILLAR_CRIO:
			switch( direccion )
			{
				case 1: 	direccion = 0; 		break;
				case 2: 	direccion = 1; 		break;
				case 3: 	direccion = 2; 		break;
				case 4: 	direccion = 3; 		break;
				case 5: 	direccion = 4; 		break;
				case 6: 	direccion = 5; 		break;
				case 7: 	direccion = 6; 		break;
			}
			break;
			
		case MANILLAR_ENYGMA:
			switch( direccion )
			{
				case 1: 	direccion = 0; 		break;
				case 2: 	direccion = 1; 		break;
				case 3: 	direccion = 2; 		break;
				case 4: 	direccion = 3; 		break;
			}
			break;
	
			
		/* *** ******************** Cables ******************** *** */
		case CABLE_MAZO_66A:
			switch( direccion )
			{
				case 1: 	direccion = 2; 		break;		// Brown
				case 2: 	direccion = 4; 		break;		// Light blue
				case 3: 	direccion = 5; 		break;		// Pink
				case 4: 	direccion = 3; 		break;		// Orange
				case 5: 	direccion = 1; 		break;		// Yellow
				case 6: 	direccion = 0; 		break;		// Green
				case 7: 	direccion = 6; 		break;		// Blue
				
				case 8: 	direccion = 12; 	break;		// Violet
				case 9: 	direccion = 11; 	break;		// Red
				case 10: 	direccion = 9; 		break;		// Shield
				case 11: 	direccion = 10; 	break;		// Wire
				case 12: 	direccion = 15; 	break;		// Red - Peltier
				case 13: 	direccion = 0; 		break;		// Green
				case 14: 	direccion = 14; 	break;		// Black - Peltier
			}
			break;
			
	}
	

	// Se modifican los pines para colocar la direccion solicitada
	auxiliar = direccion;
	auxiliar &= 0x01;
	if( auxiliar > 0 )
	{
		// Se setea el pin S0 de la placa
		S0_MULTIPLEXOR_PLACA_PIN = 1;
	}
	else
	{
		// Se borra el pin S0 de la placa
		S0_MULTIPLEXOR_PLACA_PIN = 0;
	}
	
	auxiliar = direccion >> 1;
	auxiliar &= 0x01;
	if( auxiliar > 0 )
	{
		// Se setea el pin S1 de la placa
		S1_MULTIPLEXOR_PLACA_PIN = 1;
	}
	else
	{
		// Se borra el pin S1 de la placa
		S1_MULTIPLEXOR_PLACA_PIN = 0;
	}
	
	auxiliar = direccion >> 2;
	auxiliar &= 0x01;
	if( auxiliar > 0 )
	{
		// Se setea el pin S2 de la placa
		S2_MULTIPLEXOR_PLACA_PIN = 1;
	}
	else
	{
		// Se borra el pin S2 de la placa
		S2_MULTIPLEXOR_PLACA_PIN = 0;
	}
	
	auxiliar = direccion >> 3;
	auxiliar &= 0x01;
	if( auxiliar > 0 )
	{
		// Se setea el pin S3 de la placa
		S3_MULTIPLEXOR_PLACA_PIN = 1;
	}
	else
	{
		// Se borra el pin S3 de la placa
		S3_MULTIPLEXOR_PLACA_PIN = 0;
	}
	
	// Se genera una pequenia demora para asegurarse que los multiplexores propaguen correctamente las seniales
	DEMORA_10US;
}





		/****************************************************************************************************************/
		/*  								FIRMWARE PARA LA PRUEBA REDUCIDA DEL MANILLAR								*/
		/****************************************************************************************************************/


/****************************************************************/
/* HabilitarLEDs												*/
/*  															*/
/*  Se encarga de activar el circuito de alimentacion de los	*/
/*  LEDs														*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void HabilitarLEDs()
{
	// Se debe setear el pin del transistor
	HABILITAR_LEDS_MANILLAR_PIN = 1;
	
	// Se genera una demora de algunos mili segundos para evitar el transitorio
	DemoraPorInterrupcion( CUARENTA_MILI_SEGUNDOS );
}



/****************************************************************/
/* DeshabilitarLEDs												*/
/*  															*/
/*  Se encarga de desactivar el circuito de alimentacion de los	*/
/*  LEDs														*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void DeshabilitarLEDs()
{
	// Se debe borrar el pin del transistor
	HABILITAR_LEDS_MANILLAR_PIN = 0;
}



/****************************************************************/
/* HabilitarBombaCooler											*/
/*  															*/
/*  Se encarga de activar el circuito de alimentacion de la		*/
/*  bomba y el cooler											*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void HabilitarBombaCooler()
{
	// Se debe setear el pin del transistor
	HABILITAR_BOMBA_PIN = 1;
	
	// Se genera una demora de algunos mili segundos para evitar el transitorio
	DemoraPorInterrupcion( CUARENTA_MILI_SEGUNDOS );
}



/****************************************************************/
/* DeshabilitarBombaCooler										*/
/*  															*/
/*  Se encarga de desactivar el circuito de alimentacion de la	*/
/*  bomba y el cooler											*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void DeshabilitarBombaCooler()
{
	// Se debe borrar el pin del transistor
	HABILITAR_BOMBA_PIN = 0;
}



/****************************************************************/
/* HabilitarPeltier												*/
/*  															*/
/*  Se encarga de activar el circuito de alimentacion de la		*/
/*  celda Peltier.												*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void HabilitarPeltier( void )
{
	// Se debe borrar el pin del transistor
	HABILITAR_PELTIER_PIN = 0;
	
	// Se genera una demora de 1 mili segundo para evitar el transitorio
	DemoraParaIniciarPeltier();
}



/****************************************************************/
/* DeshabilitarPeltier											*/
/*  															*/
/*  Se encarga de desactivar el circuito de alimentacion de la	*/
/*  celda Peltier.												*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void DeshabilitarPeltier( void )
{
	// Se debe setear el pin del transistor
	HABILITAR_PELTIER_PIN = 1;
}



/****************************************************************/
/* InicializarPuertosProbador									*/
/*  															*/
/*  Define la funcion que tendra cada pin del microcontrolador	*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void InicializarPuertosProbador( void )
{
	/* PINES PARA LOS MULTIPLEXORES */
	
	// Pines para el multiplexor de los contactos de la placa
	S0_MULTIPLEXOR_PLACA_DIRECCION = 1;						// Salida
	S1_MULTIPLEXOR_PLACA_DIRECCION = 1;						// Salida
	S2_MULTIPLEXOR_PLACA_DIRECCION = 1;						// Salida
	S3_MULTIPLEXOR_PLACA_DIRECCION = 1;						// Salida
	E_MULTIPLEXOR_PLACA_DIRECCION = 1;						// Salida
	S0_MULTIPLEXOR_PLACA_PIN = 0;							// Activo nivel alto
	S1_MULTIPLEXOR_PLACA_PIN = 0;							// Activo nivel alto
	S2_MULTIPLEXOR_PLACA_PIN = 0;							// Activo nivel alto
	S3_MULTIPLEXOR_PLACA_PIN = 0;							// Activo nivel alto
	E_MULTIPLEXOR_PLACA_PIN = 1;							// Activo nivel bajo
	
	// Pines para el multiplexor de los contactos desde los pernos
	S0_MULTIPLEXOR_PERNOS_DIRECCION = 1;					// Salida
	S1_MULTIPLEXOR_PERNOS_DIRECCION = 1;					// Salida
	S2_MULTIPLEXOR_PERNOS_DIRECCION = 1;					// Salida
	S3_MULTIPLEXOR_PERNOS_DIRECCION = 1;					// Salida
	E_MULTIPLEXOR_PERNOS_DIRECCION = 1;						// Salida
	S0_MULTIPLEXOR_PERNOS_PIN = 0;							// Activo nivel alto
	S1_MULTIPLEXOR_PERNOS_PIN = 0;							// Activo nivel alto
	S2_MULTIPLEXOR_PERNOS_PIN = 0;							// Activo nivel alto
	S3_MULTIPLEXOR_PERNOS_PIN = 0;							// Activo nivel alto
	E_MULTIPLEXOR_PERNOS_PIN = 1;							// Activo nivel bajo
	
	// Se define la salida del multiplexor donde se mide la continuidad como entrada. Luego se usaran las funciones analogicas
	PERNOS_SALIDA_CONTINUIDAD_DIRECCION = 0;				// Entrada
	
	
	/* PINES PARA LOS LEDS DEL MANILLAR */
	HABILITAR_LEDS_MANILLAR_DIRECCION = 1;					// Salida
	HABILITAR_LEDS_MANILLAR_PIN = 0;						// Activo nivel alto
	
	
	/* PINES PARA CONTROLAR LA PELTIER */
	HABILITAR_PELTIER_DIRECCION = 1;						// Salida
	HABILITAR_PELTIER_PIN = 1;								// Activo nivel bajo
	
	
	/* PINES PARA CONTROLAR LA BOMBA Y EL COOLER */
	HABILITAR_BOMBA_DIRECCION = 1;							// Salida
	HABILITAR_BOMBA_PIN = 0;								// Activo nivel alto
	
	
	/* PIN PARA CONTROLAR LA COMUNICACION RS485 */
	HABILITAR_PTE_RS485_DIRECCION = 0;						// Entrada
	HABILITAR_PTE_RS485_PIN = 0;
	
	
	/* PIN PARA CONTROLAR LOS CAUDALIMETROS */
	CAUDALIMETRO_BOMBA_DIRECCION = 0;						// Entrada
	CAUDALIMETRO_BOMBA_PIN = 0;
	CAUDALIMETRO_EXTERNO_DIRECCION = 0;						// Entrada
	CAUDALIMETRO_EXTERNO_PIN = 0;
	
	
	/* PIN PARA CONTROLAR LOS SENSORES DE TEMPERATURA */
	DS18S20_DIRECCION = 1;									// Salida
	DS18S20_PIN = 1;										// Stand by con el bus en alto
	
	
	/* PINES PARA CONTROLAR LOS LEDS */
	LED_ROJO_DIRECCION = 1;									// Salida
	LED_ROJO_PIN = 0;										// Activo nivel alto
	LED_VERDE_DIRECCION = 1;								// Salida
	LED_VERDE_PIN = 0;										// Activo nivel alto
	
	
	/* PINES PARA CONTROLAR LOS PULSADORES */
	PULSADOR_S1_DIRECCION = 0;								// Entrada
	PULSADOR_S2_DIRECCION = 0;								// Entrada
	PULSADOR_S3_DIRECCION = 0;								// Entrada
	PULSADOR_S4_DIRECCION = 0;								// Entrada
	PULSADOR_SEL_DIRECCION = 0;								// Entrada
	
}





		/****************************************************************************************************************/
		/*  									FUNCIONES PARA LAS PRUEBAS COMPLETAS									*/
		/****************************************************************************************************************/


/****************************************************************/
/* PruebaCompletaManillarDeCrio									*/
/*  															*/
/*  Ejecuta una secuencia de prueba para verificar el correcto	*/
/*  funcionamiento del manillar de crio. A medida que se va		*/
/*  realizando la prueba, va informando el estado en el LCD.	*/
/*  Esta prueba acciona el sistema de recirculacion.			*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: FALSE, de haber algun inconveniente				*/
/****************************************************************/
bool	PruebaCompletaManillarDeCrio ( void )
{
	// Variable para saber si hubo algun error
	bool errorEnLaPrueba;
	
	// Se borra el flag
	errorEnLaPrueba = FALSE;
	
	// Se borra el LCD para indicar el estado de la prueba
	BorrarLCD();
	
	// Se ejecuta la secuencia de medicion del sensor de temperatura
	estadoSensor = MedicionDelSensorDeTemperaturaManillar();
	switch( estadoSensor )
	{
		// Cualquier error que haga que no se pueda leer el sensor de la Peltier, obliga a que no se la pueda medir
		case SENSOR_ERROR_ACTUALIZAR_LISTADO:
		case SENSOR_ERROR_NO_AGREGADO:
		case SENSOR_ERROR_TEMPERATURA:
			errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 20, "Sensor:  *NO RESPOND" );
			errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 0, 20, "Peltier: -SIN MEDIR-" );
			DemoraEnSegundos( 2 );
			break;
	}
	
	// Se indica mediante los LEDs de la placa, el estado de la prueba
	if( errorEnLaPrueba == FALSE )
		IndicarPruebaBien();
	else
	{
		// Si hay algun fallo en la prueba, no se prosigue con la medicion activando el sistema de recirculacion
		IndicarPruebaMal();
		return (errorEnLaPrueba);
	}
	
	
	// Se indica que se comienza a medir con el sistema de recirculacion activado
	errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 	   0, 20, "Ambi.:              " );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 20, "Cali.:              " );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 20, "Mang.:              " );
	errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 	   0, 20, "Pelt.:              " );
	
	// Luego se realizan las pruebas con el sistema de recirculacion
	estadoPeltier = MedicionDeLaPeltierConRecirculacionActivada();
	
	// Independientemente del resultado de la medicion, se apagan los sistemas de recirculacion, la Peltier y los leds
	DeshabilitarPeltier();
	DeshabilitarBombayCooler();
	ApagarLeds();
	
	return (errorEnLaPrueba);
	
}



/****************************************************************/
/* MedicionDeLaPeltierConRecirculacionActivada					*/
/*  															*/
/*  Se habilita la alimentacion del modulo, que tiene			*/
/*  incorporada una demora para dejar pasar el transitorio y	*/
/*  que la corriente se estabilice.								*/
/*  Se lee el valor en que se establece la corriente para ver	*/
/*  que no sea muy elevado.										*/
/*  Se toma la temperatura inicial de la celda y su valor luego */
/*  de 5 segundos. Si el lado frio ascendio, la celda esta		*/
/*  invertida.													*/
/*  Si la Peltier esta bien colocada, se realiza un loop de		*/
/*  varias mediciones donde se registra la corriente y la 		*/
/*  temperatura a cada segundo. Si la corriente indica un		*/
/*  sobreconsumo o que la celda no esta conectada, se corta el	*/
/*  loop y se informa el error.									*/
/*  Luego de las mediciones, se toman las diferencias de la		*/
/*  temperatura final respecto de la inicial para ver que la	*/
/*  celda haya enfriado correctamente.							*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
resultadoPeltier	MedicionDeLaPeltierConRecirculacionActivada( void )
{
	// Variable solo para las pruebas
	int		corriente_mA;
	
	// Se habilita el sistema de recirculacion
	HabilitarBombayCooler();
	
	// Se acciona el circuito de habilitacion de la Peltier
	HabilitarPeltier();
	
	errorGeneral = TomarMuestraSimpleADC( ADC_CANAL_PELTIER );
	if( cuentasADC > MAXIMAS_CUENTRAS_PARA_PELTIER )
		return( PELTIER_SOBRE_CONSUMO );
	if( cuentasADC < MINIMAS_CUENTAS_PELTIER_ENCENDIDA )
		return( PELTIER_APAGADA );
	
	// Si el valor de corriente es aceptable, se lo toma como la corriente inicial
	medicionesCorrientePeltier[0] = cuentasADC * RESOLUCION_MA_PELTIER;
	EscribirCorriente( medicionesCorrientePeltier[0], RENGLON_INFERIOR, 16 );
	
	// Se imprimen las temperaturas iniciales
	errorSensorDS18S20 = LeerTodasLasTemperaturasDS18S20();
	
	// Ambiente
	medicionesTemperaturaAmbiente[0] = sensores[ SENSOR_SOLDADO_EN_PLACA ].temperatura;
	EscribirTemperatura( medicionesTemperaturaLadoFrio[0], RENGLON_SUPERIOR, 7 );
	
	// Lado Frio
	medicionesTemperaturaLadoFrio[0] = sensores[ SENSOR_MANILLAR_O_EXTRA ].temperatura;
	EscribirTemperatura( medicionesTemperaturaLadoFrio[0], RENGLON_INFERIOR, 7 );
	
	// Lado Caliente
	medicionesTemperaturaLadoCaliente[0] = sensores[ SENSOR_PARA_LADO_CALIENTE ].temperatura;
	EscribirTemperatura( medicionesTemperaturaLadoCaliente[0], RENGLON_MEDIO_SUPERIOR, 7 );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 13, 8, "        " );
	
	// Manguera
	medicionesTemperaturaManguera[0] = sensores[ SENSOR_PARA_LA_MANGUERA ].temperatura;
	EscribirTemperatura( medicionesTemperaturaManguera[0], RENGLON_MEDIO_INFERIOR, 7 );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 13, 8, "        " );
	
	// Se genera una demora inicial para esperar a que la Peltier comienze a enfriar correctamente
	DemoraEnSegundos( 5 );
	
	// Luego de esa demora se verifica que la celda no este conectada de forma inversa
	errorSensorDS18S20 = LeerTodasLasTemperaturasDS18S20();
	medicionesTemperaturaLadoFrio[1] = sensores[ SENSOR_MANILLAR_O_EXTRA ].temperatura;
	variacionDeTemperaturaLadoFrio = medicionesTemperaturaLadoFrio[0] - medicionesTemperaturaLadoFrio[1];
	
	// Si el lado frio elevo su temperatura, es porque la celda esta al reves
	if( variacionDeTemperaturaLadoFrio < 0 )
		return( PELTIER_INVERTIDA );
	
	setReg8( TPM2SC, 0x0E);    /* Set prescaler PS = 64; and run counter */
	
	// Bucle para tomar la corriente y la temperatura a cada segundo durante X segundos
	for( indiceMedicion = 0; indiceMedicion < MINUTOS_PELTIER_CON_RECIRCULACION; indiceMedicion++ )
	{
		for( auxiliar = 0; auxiliar < 60; auxiliar++ )
		{
			// Se resetea el contador del timer 2
			TPM2CNT = 0;
			
			// Se verifica que no sobrepase la corriente
			errorGeneral = TomarMuestraSimpleADC( ADC_CANAL_PELTIER ); 
			if( cuentasADC > MAXIMAS_CUENTRAS_PARA_PELTIER )
				return( PELTIER_SOBRE_CONSUMO );
			if( cuentasADC < MINIMAS_CUENTAS_PELTIER_ENCENDIDA )
				return( PELTIER_SE_DESCONECTO );
			
			// Se almacena el valor de la corriente de la celda
			medicionesCorrientePeltier[ 1 ] = cuentasADC * RESOLUCION_MA_PELTIER;
			
			// Luego se verifica la temperatura
			errorSensorDS18S20 = LeerTodasLasTemperaturasDS18S20();
			medicionesTemperaturaAmbiente[ 1 ] = sensores[ SENSOR_SOLDADO_EN_PLACA ].temperatura;
			medicionesTemperaturaLadoFrio[ 1 ] = sensores[ SENSOR_MANILLAR_O_EXTRA ].temperatura;
			medicionesTemperaturaLadoCaliente[ 1 ] = sensores[ SENSOR_PARA_LADO_CALIENTE ].temperatura;
			medicionesTemperaturaManguera[ 1 ] = sensores[ SENSOR_PARA_LA_MANGUERA ].temperatura;
			
			// Se envian los datos a la computadora cada 10 segundos
			if( ( auxiliar % 10 ) == 0 )
				EnviarTramaPruebaCompleta_ManillarCrio();
			
			while( TPM2CNT < 0xEA60 )
				{ asm(nop); };
		}
		
		// Se muestran en el LCD
		EscribirTemperatura( medicionesTemperaturaAmbiente[ 1 ], RENGLON_SUPERIOR, 7 );
		EscribirTemperatura( medicionesTemperaturaLadoFrio[ 1 ], RENGLON_INFERIOR, 7 );
		EscribirTemperatura( medicionesTemperaturaLadoCaliente[ 1 ], RENGLON_MEDIO_SUPERIOR, 7 );
		EscribirTemperatura( medicionesTemperaturaManguera[ 1 ], RENGLON_MEDIO_INFERIOR, 7 );
		
		// Se calcula el descenso de temperatura
		acumulado_ladoFrio = medicionesTemperaturaLadoFrio[ 1 ] - medicionesTemperaturaLadoFrio[ 0 ];
		EscribirTemperatura( acumulado_ladoFrio, RENGLON_INFERIOR, 14 );
//		EscribirCorriente( medicionesCorrientePeltier[ 1 ], RENGLON_INFERIOR, 16 );
	}
	
	// Se toma la diferencia critica como la medicion inicial menos la final del lado frio
	variacionDeTemperaturaLadoFrio = medicionesTemperaturaLadoFrio[ 0 ] - medicionesTemperaturaLadoFrio[ 1 ];
	
	// Si la diferencia es negativa y superior a 2 grados, la celda esta colocada al reves
	if( variacionDeTemperaturaLadoFrio < TEMPERATURA_MINIMA_PELTIER_INVERTIDA )
		return( PELTIER_INVERTIDA );
	
	// Si la diferencia no es mayor a 1 grado, la celda esta apagada
	if( variacionDeTemperaturaLadoFrio < TEMPERATURA_MINIMA_PELTIER_APAGADA )
		return( PELTIER_APAGADA );
	
	// Si llega aca, es porque la celda esta correcta
	return( PELTIER_SIN_ERRORES );
}





		/****************************************************************************************************************/
		/*  							FIRMWARE PARA LAS PRUEBAS COMPLETAS DE LOS MANILLARES							*/
		/****************************************************************************************************************/



/****************************************************************/
/* HabilitarBombayCooler										*/
/*  															*/
/*  Se encarga de activar el circuito de alimentacion de la		*/
/*  bomba y el cooler											*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void HabilitarBombayCooler()
{
	// Se define el pin como salida, ante una posible falla en la inicializacion
	HABILITAR_BOMBA_DIRECCION = 1;
	
	// Se debe setear el pin del transistor
	HABILITAR_BOMBA_PIN = 1;
	
	// Se genera una demora de algunos mili segundos para evitar el transitorio
	DemoraPorInterrupcion( CUARENTA_MILI_SEGUNDOS );
}



/****************************************************************/
/* DeshabilitarBombayCooler										*/
/*  															*/
/*  Se encarga de desactivar el circuito de alimentacion de la	*/
/*  bomba y el cooler											*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void DeshabilitarBombayCooler()
{
	// Se debe borrar el pin del transistor
	HABILITAR_BOMBA_PIN = 0;
}





		/****************************************************************************************************************/
		/*				  								FUNCIONES PARA EL ADC											*/
		/****************************************************************************************************************/


/****************************************************************/
/* ConfigurarADC												*/
/*  															*/
/*  Setea el modulo para usarlo en baja velocidad, con el bus	*/
/*  principal dividido 16 (8x2), tiempos largos para la toma de	*/
/*	muestras y disparado por software							*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void ConfigurarADC ( void )
{
	ADC1CFG = 0xF1;		// Baja velocidad, dividido 8, tiempos largos, 8 bits y dividido 2
	ADC1SC2 = 0;		// Disparable por software, sin comparacion
	APCTL1 = 0x03;		// Se habilitan los pines de los canales 0 (Leds manillar) y 1 (Peltier)
	APCTL2 = 0x08;		// Se habilita el pin del canal 11	(Salida de los pernos)
}



/****************************************************************/
/* TomarMuestraSimpleADC										*/
/*  															*/
/*  Se inicia una conversion simple en el canal seleccionado y  */
/*  se espera por la interrupcion de la conversion.				*/
/*	El resultado se almacena en la variable global "cuentasADC"	*/
/*  															*/
/*  Recibe: El canal del cual se deba leer el valor				*/
/*  Devuelve: FALSE, si hay algun problema						*/
/****************************************************************/
bool TomarMuestraSimpleADC( char canal )
{
	// Primero se verifica que el canal suministrado sea valido y se inicia la conversion
	if( iniciarADC_Simple( canal ) == FALSE )
		return( FALSE );
	
	// Se borra el flag de la interrupcion
	semaforoADC = FALSE;
	
	// Luego se espera a que termine la conversion
	while( TRUE )
	{
		if( semaforoADC == TRUE )
		{
			semaforoADC = FALSE;
			break;
		}
	}
	
	// Se toma el valor de la conversion
	cuentasADC = leerADC();
	
	return( TRUE );
}



// Iniciar una conversion simple
bool iniciarADC_Simple ( uint8_t canal )
{
	switch(canal)
	{
		case ADC_CANAL_LEDS:
			ADC1SC1 = 0x40;
			break;
		case ADC_CANAL_PELTIER:
			ADC1SC1 = 0x41;
			break;
		case ADC_CANAL_PERNOS:
			ADC1SC1 = 0x4B;
			break;
		default:
			return( FALSE );	
	}
	
	// Si el canal era correcto, se devuelve TRUE 
	return( TRUE );
}

// Se devuelve el registro del ADC donde se almacena el resultado
uint8_t leerADC ( void )				{	return( ADC1RL );	}

// Funcion para borrar el flag de la interrupcion
void borrarFlagADC ( void )				{	clrReg8Bits(ADC1SC1, 0x80U);	}





		/****************************************************************************************************************/
		/*		  								FUNCIONES PARA INFORMAR LOS ERRORES										*/
		/****************************************************************************************************************/


/****************************************************************/
/* InformarPernosAbiertos										*/
/*  															*/
/*  Muestra en pantalla la indicacion de que pernos se			*/
/*  encuentran abiertos											*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void	InformarPernosAbiertos ( char target )
{
	// Variable auxiliar
	char	indicePerno;
	char	offset;
	char	indicePernoCaracter[2];
	char	totalDePernos;
	
	// Se selecciona la cantidad de pernos en funcion del manillar
	switch( target )
	{
		case MANILLAR_CRIO:
			totalDePernos = TOTAL_DE_PERNOS_MANILLAR_CRIO + 1;
			break;
		case MANILLAR_ENYGMA:
			totalDePernos = TOTAL_DE_PERNOS_MANILLAR_ENYGMA + 1;
			break;
	}
	
	// Se informa que hay un error por que algunos pernos estan en corto
	errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 20, "Abier:               " );
	
	// La letra para indicar el perno no cambia en ningun momento
	indicePernoCaracter[0] = 'J';
	
	// Se recorre el estado de todos los pernos para informar cuales estan en falla
	for( indicePerno = 1; indicePerno < totalDePernos; indicePerno++ )
	{
		if( condicionDelPerno[ indicePerno ] == PERNO_ABIERTO )
		{
			offset = 4 + indicePerno * 2;
			indicePernoCaracter[1] = indicePerno + '0';
			errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, offset, 2, indicePernoCaracter );
		}
	}
}



/****************************************************************/
/* InformarPernosEnCorto										*/
/*  															*/
/*  Muestra en pantalla la indicacion de que pernos se			*/
/*  encuentran en corto circuito entre si						*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void	InformarPernosEnCorto ( char target )
{
	// Variable auxiliar
	char	indicePerno;
	char	offset;
	char	indicePernoCaracter[2];
	char	totalDePernos;
	
	// Se selecciona la cantidad de pernos en funcion del manillar
	switch( target )
	{
		case MANILLAR_CRIO:
			totalDePernos = TOTAL_DE_PERNOS_MANILLAR_CRIO + 1;
			break;
		case MANILLAR_ENYGMA:
			totalDePernos = TOTAL_DE_PERNOS_MANILLAR_ENYGMA + 1;
			break;
	}
	
	// Se informa que hay un error por que algunos pernos estan en corto
	errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 20, "Corto:               " );
	
	// Se recorre el estado de todos los pernos para informar cuales estan en falla
	for( indicePerno = 1; indicePerno < totalDePernos; indicePerno++ )
	{
		if( condicionDelPerno[ indicePerno ] == PERNO_CORTO_CIRCUITO )
		{
			offset = 4 + indicePerno * 2;
			indicePernoCaracter[0] = 'J';
			indicePernoCaracter[1] = indicePerno + '0';
			errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, offset, 2, indicePernoCaracter );
		}
	}
}



/****************************************************************/
/* InformarMalUbicacion											*/
/*  															*/
/*  Muestra en pantalla la indicacion de que pernos se			*/
/*  encuentran en una posicion incorrecta						*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void InformarMalUbicacion ( char target )
{
	// Variable auxiliar
	char	indicePerno;
	char	offset;
	char	indicePernoCaracter[2];
	char	totalDePernos;
	
	// Se selecciona la cantidad de pernos en funcion del manillar
	switch( target )
	{
		case MANILLAR_CRIO:
			totalDePernos = TOTAL_DE_PERNOS_MANILLAR_CRIO + 1;
			break;
		case MANILLAR_ENYGMA:
			totalDePernos = TOTAL_DE_PERNOS_MANILLAR_ENYGMA + 1;
			break;
	}
	
	// Se informa que hay un error por que algunos pernos estan en corto
	errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 20, "Alter:               " );
	
	// Se recorre el estado de todos los pernos para informar cuales estan en falla
	for( indicePerno = 1; indicePerno < totalDePernos; indicePerno++ )
	{
		if( condicionDelPerno[ indicePerno ] == PERNO_SIN_ALTERNANCIA )
		{
			offset = 4 + indicePerno * 2;
			indicePernoCaracter[0] = 'J';
			indicePernoCaracter[1] = indicePerno + '0';
			errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, offset, 2, indicePernoCaracter );
		}
	}
}



/****************************************************************/
/* InformarCortoConLaCarcaza									*/
/*  															*/
/*  Muestra en pantalla la indicacion de que pernos se			*/
/*  encuentran en cortocircuito con la carcaza					*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void	InformarCortoConLaCarcaza( char target )
{
	// Variable auxiliar
	char	indicePerno;
	char	offset;
	char	indicePernoCaracter[2];
	char	totalDePernos;
	
	// Se selecciona la cantidad de pernos en funcion del manillar
	switch( target )
	{
		case MANILLAR_CRIO:
			totalDePernos = TOTAL_DE_PERNOS_MANILLAR_CRIO + 1;
			break;
		case MANILLAR_ENYGMA:
			totalDePernos = TOTAL_DE_PERNOS_MANILLAR_ENYGMA + 1;
			break;
	}
	
	// Se informa que hay un error por que algunos pernos estan en corto
	errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 20, "Mazo:                " );
	
	// Se recorre el estado de todos los pernos para informar cuales estan en falla
	for( indicePerno = 1; indicePerno < totalDePernos; indicePerno++ )
	{
		if( condicionDelPerno[ indicePerno ] == PERNO_CORTO_CIRCUITO_CARCAZA )
		{
			offset = 4 + indicePerno * 2;
			indicePernoCaracter[0] = 'J';
			indicePernoCaracter[1] = indicePerno + '0';
			errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, offset, 2, indicePernoCaracter );
		}
	}	
}





		/****************************************************************************************************************/
		/*		  						FUNCIONES PARA MOSTRAR LAS PANTALLAS DE LOS MENUES								*/
		/****************************************************************************************************************/


/****************************************************************/
/* MostrarPantallaInicialPruebaReducida							*/
/*  															*/
/*  Muestra en pantalla el menu para iniciar la prueba reducida */
/*  o para registra el sensor extra								*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void MostrarPantallaInicialPruebaReducida ( void )
{
	BorrarLCD();
	errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 20, "  Prueba reducida:  " );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 20, "ACEPTAR para iniciar" );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 20, "CANCELAR -> completa" );
	errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 0, 20, "IZQ <- Sensor extra " );
}



/****************************************************************/
/* MostrarPantallaInicialSensorExtra							*/
/*  															*/
/*  Muestra en pantalla el menu para registrar el sensor extra	*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void MostrarPantallaInicialSensorExtra ( void )
{
	BorrarLCD();
	errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 20, "    Sensor extra    " );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 20, "ACEPTAR -> Registrar" );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 20, "DERECHA -> Volver   " );
	errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 0, 20, "ROM:                " );
}



/****************************************************************/
/* MostrarPantallaTemperaturasIniciales							*/
/*  															*/
/*  Muestra las temperaturas de los sensores del dispositivo	*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void MostrarPantallaTemperaturasIniciales ( void )
{
	BorrarLCD();
	errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 20, "Temperaturas previas" );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 10, "SOLDADO:  " );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 10, "LADO CAL: " );
	errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 0, 20, "                    " );
}



/****************************************************************/
/* MostrarPantallaFinalPruebaReducida							*/
/*  															*/
/*  Muestra los datos relevantes de la prueba finalizada		*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void MostrarPantallaFinalPruebaReducida ( void )
{
	// Se borra la pantalla
	BorrarLCD();
	
	// Se muestran cuanto descendio el Lado Frio y cuanto aumento el Lado Caliente
	errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 20, "LF:       LC:       " );
	EscribirVariacionesDeTemperatura( variacionDeTemperaturaLadoFrio, RENGLON_SUPERIOR, 3 );
	EscribirVariacionesDeTemperatura( variacionDeTemperaturaLadoCaliente, RENGLON_SUPERIOR, 13 );
	
	// Se muestra la corriente inicial y la corriente final de la Peltier
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 20, "Ii:       If:       " );
	EscribirCorriente( medicionesCorrientePeltier[0], RENGLON_MEDIO_SUPERIOR, 5 );
	EscribirCorriente( medicionesCorrientePeltier[1], RENGLON_MEDIO_SUPERIOR, 15 );
	
	// Se muestra la ROM del sensor
	escribirTodaROM_EnLCD( RENGLON_MEDIO_INFERIOR, sensores[SENSOR_MANILLAR_O_EXTRA].ROM );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 4, "ROM:" );
	
	// Se da la indicacion de como proseguir
	errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 0, 20, "  Presione DERECHA  " );
}



/****************************************************************/
/* MostrarPantallaFinalPruebaCompleta							*/
/*  															*/
/*  Muestra los datos relevantes de la prueba finalizada		*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void MostrarPantallaFinalPruebaCompleta ( void )
{
	// Se borra la pantalla
	BorrarLCD();
	
	// Se muestran cuanto descendio el Lado Frio
	errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 20, "LF:                 " );
	EscribirVariacionesDeTemperatura( variacionDeTemperaturaLadoFrio, RENGLON_SUPERIOR, 3 );
	
	// Se muestra la corriente inicial y la corriente final de la Peltier
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 20, "Ii:       If:       " );
	EscribirCorriente( medicionesCorrientePeltier[0], RENGLON_MEDIO_SUPERIOR, 5 );
	EscribirCorriente( medicionesCorrientePeltier[1], RENGLON_MEDIO_SUPERIOR, 15 );
	
	// Se muestra la ROM del sensor
	escribirTodaROM_EnLCD( RENGLON_MEDIO_INFERIOR, sensores[SENSOR_MANILLAR_O_EXTRA].ROM );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 4, "ROM:" );
	
	// Se da la indicacion de como proseguir
	errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 0, 20, "  Presione DERECHA  " );
}



/****************************************************************/
/* InformarCortoConLaCarcaza									*/
/*  															*/
/*  Muestra en pantalla el menu para iniciar la prueba reducida */
/*  o para registra el sensor extra								*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void MostrarPantallaMenuesPruebaCompleta ( void )
{
	BorrarLCD();
	errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 20, "  Prueba completa:  " );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 20, "*Manillar de crio   " );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 20, " Manillar de facial " );
	errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 0, 20, " Manillar de enygma " );
}





/****************************************************************/
/* ProbarCables													*/
/*  															*/
/*  Ejecuta la secuencia de prueba de continuidad de todos los  */
/*  contactos del cable solicitado								*/
/*  															*/
/*  Recibe: El cable que se quiera probar						*/
/*  Devuelve: El estado de los cables							*/
/****************************************************************/
resultadoPernos ProbarCables ( char target )
{
	// Variables auxiliares
	char indiceContactoPlaca;
	
	// Se obtiene la matriz de contactos
	ObtenerMatrizDePernos ( target );
	
	// Luego se debe revisar que no haya ningun cable en corto con los 5V de por si. De ser asi, la prueba no continua
	if( hayPernoEnCortoCon5V == TRUE )
		return( PERNO_CORTOCIRCUITO_CON_5V );
	
	
	// Se revisa el estado de los contactos, a excepcion del 5, 6 y 13 que son especiales
	for( indiceContactoPlaca = 1; indiceContactoPlaca < TOTAL_DE_PERNOS_CABLE_MAZO_66A + 1; indiceContactoPlaca++ )
	{
		// Se evitan el 5 y el 6
		if( indiceContactoPlaca == 5 || indiceContactoPlaca == 6 )		{	indiceContactoPlaca = 7;	}
		
		// Se evita el contacto 13
		if( indiceContactoPlaca == 13 )			{	indiceContactoPlaca++;	}
		
		// Primero se revisa si el contacto esta abierto
		if( pernos[ indiceContactoPlaca ].totalDeContactos == 0 )
		{
			condicionDelPerno[ indiceContactoPlaca ] = PERNO_ABIERTO;
			hayPernoAbierto = TRUE;
		}
		else
		{
			// Luego se revisa si esta en cortocircuito
			if( pernos[ indiceContactoPlaca ].totalDeContactos > 1 )
			{
				condicionDelPerno[ indiceContactoPlaca ] = PERNO_CORTO_CIRCUITO;
				hayPernoEnCorto = TRUE;
			}
			else
			{
				// De haber un solo contacto, se verifica que sea en la posicion correcta
				if( pernosUbicacion[ indiceContactoPlaca ] == indiceContactoPlaca )
					condicionDelPerno[ indiceContactoPlaca ] = PERNO_SIN_ERRORES;
				else
				{
					condicionDelPerno[ indiceContactoPlaca ] = PERNO_SIN_ALTERNANCIA;
					hayPernoAlternado = TRUE;
				}
			}
		}
	}
	
	
	// Perno numero 5
	if( pernos[ 5 ].totalDeContactos == 0 )
	{
		condicionDelPerno[ 5 ] = PERNO_ABIERTO;
		hayPernoAbierto = TRUE;
	}
	else
	{
		if( hayPernoAlternado == TRUE )
		{
			if( pernos[ 5 ].contactos[ 5 ] == HAY_CONTINUIDAD )
			{
				if( pernos[ 5 ].contactos[ 6 ] == HAY_CONTINUIDAD )
				{
					pernosUbicacion[ 5 ] = 6;
					condicionDelPerno[ 5 ] = PERNO_SIN_ALTERNANCIA;
					hayPernoAlternado = TRUE;
				}
				else
					condicionDelPerno[ 5 ] = PERNO_SIN_ERRORES;
			}
			else
			{
				condicionDelPerno[ 5 ] = PERNO_SIN_ALTERNANCIA;
				hayPernoAlternado = TRUE;
			}
		}
		else
		{
			// Si no estan alternados los demas contactos, no deberia registrar mas de un contacto con continuidad
			if( pernos[ 5 ].totalDeContactos > 1 )
			{
				condicionDelPerno[ 5 ] = PERNO_CORTO_CIRCUITO;
				hayPernoEnCorto = TRUE;
			}
			else
			{
				// De haber un solo contacto, se verifica que sea en la posicion correcta
				if( pernosUbicacion[ 5 ] == 5 )
					condicionDelPerno[ 5 ] = PERNO_SIN_ERRORES;
				else
				{
					condicionDelPerno[ 5 ] = PERNO_SIN_ALTERNANCIA;
					hayPernoAlternado = TRUE;
				}
			}
		}
	}
	

	// Perno numero 6
	if( pernos[ 6 ].totalDeContactos == 0 )
	{
		condicionDelPerno[ 6 ] = PERNO_ABIERTO;
		hayPernoAbierto = TRUE;
	}
	else
	{
		if( pernos[ 6 ].contactos[ 6 ] == HAY_CONTINUIDAD )
			if( pernos[ 6 ].contactos[ 5 ] == HAY_CONTINUIDAD )
			{
				pernosUbicacion[ 6 ] = 5;
				condicionDelPerno[ 6 ] = PERNO_SIN_ALTERNANCIA;
				hayPernoAlternado = TRUE;
			}
			else
				condicionDelPerno[ 6 ] = PERNO_SIN_ERRORES;
		else
		{
			condicionDelPerno[ 6 ] = PERNO_SIN_ALTERNANCIA;
			hayPernoAlternado = TRUE;
		}
	}
	
	// Perno numero 13
	if( pernos[ 13 ].totalDeContactos == 0 )
	{
		condicionDelPerno[ 13 ] = PERNO_ABIERTO;
		hayPernoAbierto = TRUE;
	}
	else
	{
		if( pernos[ 13 ].contactos[ 13 ] == HAY_CONTINUIDAD )
			condicionDelPerno[ 13 ] = PERNO_SIN_ERRORES;
		else
		{
			condicionDelPerno[ 13 ] = PERNO_SIN_ALTERNANCIA;
			hayPernoAlternado = TRUE;
		}
	}
	
	
	if( hayPernoAbierto == TRUE )
		return( PERNO_ABIERTO );
	
	if( hayPernoEnCorto == TRUE )
		return( PERNO_CORTO_CIRCUITO );
	
	if( hayPernoAlternado == TRUE )
		return( PERNO_SIN_ALTERNANCIA );
		
	// Si llega hasta aca, es porque todos los contactos estan correctos
	return( PERNO_SIN_ERRORES );
}



/****************************************************************/
/* InformarCablesAbiertos										*/
/*  															*/
/*  Muestra en pantalla la indicacion de que cables se			*/
/*  encuentran abiertos											*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void InformarCablesAbiertos ( void )
{
	// Se informa que hay un error por que algunos pernos estan abiertos
	BorrarLCD();
	errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR,       0, 20, "Abiert:    xx-xx-xx " );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 20, "         xx-xx-xx-xx" );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 20, "         xx-xx-xx-xx" );
	errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR,       0, 20, "           xx-xx-xx " );
	
	/* *** RENGLON SUPERIOR: CABLES DEL 1 AL 3 *** */
	if( condicionDelPerno[ 1 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 11, 2, "01" );				// Brown
	if( condicionDelPerno[ 2 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 14, 2, "02" );				// Light blue
	if( condicionDelPerno[ 3 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 17, 2, "03" );				// Pink

	/* *** RENGLON MEDIO SUPERIOR: CABLES DEL 4 AL 7 *** */
	if( condicionDelPerno[ 4 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 9, 2, "04" );		// Orange
	if( condicionDelPerno[ 5 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 12, 2, "05" );		// Yellow
	if( condicionDelPerno[ 6 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 15, 2, "06" );		// Green
	if( condicionDelPerno[ 7 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 18, 2, "07" );		// Blue
	
	/* *** RENGLON MEDIO INFERIOR: CABLES DEL 8 AL 11 *** */
	if( condicionDelPerno[ 8 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 9, 2, "08" );		// Violet
	if( condicionDelPerno[ 9 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 12, 2, "09" );		// Red
	if( condicionDelPerno[ 10 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 15, 2, "10" );		// Shield
	if( condicionDelPerno[ 11 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 18, 2, "11" );		// Wire
	
	
	/* *** RENGLON INFERIOR: CABLES DEL 12 AL 14 *** */
	if( condicionDelPerno[ 12 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 11, 2, "12" );				// Red - Peltier
	if( condicionDelPerno[ 13 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 14, 2, "13" );				// Green
	if( condicionDelPerno[ 14 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 17, 2, "14" );				// Black - Peltier
}



/****************************************************************/
/* InformarCablesEnCorto										*/
/*  															*/
/*  Muestra en pantalla la indicacion de que cables se			*/
/*  encuentran en corto circuito entre si						*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void InformarCablesEnCorto ( void )
{
	// Variables auxiliares
	char	indiceContactoPlaca;
	char	renglon;
	char	offset;
	
	// Se informa que hay un error por que algunos pernos estan en corcocircuito
	BorrarLCD();
	errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR,       0, 20, "Corto:     xx-xx-xx " );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 20, "         xx-xx-xx-xx" );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 20, "         xx-xx-xx-xx" );
	errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR,       0, 20, "           xx-xx-xx " );
	
	// Se recorren todos los pernos indicando si estan en corto o no
	for( indiceContactoPlaca = 1; indiceContactoPlaca < TOTAL_DE_PERNOS_CABLE_MAZO_66A + 1; indiceContactoPlaca++ )
	{
		// Solo se indican los que esten en cortocircuito
		if( condicionDelPerno[ indiceContactoPlaca ] == PERNO_CORTO_CIRCUITO )
		{
			// Se calcula el offset y el renglon para la ubicacion de cada indicacion
			switch( indiceContactoPlaca )
			{
				case 1:
					renglon = RENGLON_SUPERIOR;
					offset = 11;
					break;
				case 2:
					renglon = RENGLON_SUPERIOR;
					offset = 14;
					break;
				case 3:
					renglon = RENGLON_SUPERIOR;
					offset = 17;
					break;
					
				case 4:
					renglon = RENGLON_MEDIO_SUPERIOR;
					offset = 9;
					break;
				case 5:
					renglon = RENGLON_MEDIO_SUPERIOR;
					offset = 12;
					break;
				case 6:
					renglon = RENGLON_MEDIO_SUPERIOR;
					offset = 15;
					break;
				case 7:
					renglon = RENGLON_MEDIO_SUPERIOR;
					offset = 18;
					break;
					
				case 8:
					renglon = RENGLON_MEDIO_INFERIOR;
					offset = 9;
					break;
				case 9:
					renglon = RENGLON_MEDIO_INFERIOR;
					offset = 12;
					break;
				case 10:
					renglon = RENGLON_MEDIO_INFERIOR;
					offset = 15;
					break;
				case 11:
					renglon = RENGLON_MEDIO_INFERIOR;
					offset = 18;
					break;
					
				case 12:
					renglon = RENGLON_INFERIOR;
					offset = 11;
					break;
				case 13:
					renglon = RENGLON_INFERIOR;
					offset = 14;
					break;
				case 14:
					renglon = RENGLON_INFERIOR;
					offset = 17;
					break;
			}
			
			// Se imprime la ubucacion que se encuentra en un lugar incorrecto
			EscribirUbicacion( pernosUbicacion[ indiceContactoPlaca ], renglon, offset );
		}
	}
	
}



/****************************************************************/
/* InformarCablesConMalUbicacion								*/
/*  															*/
/*  Muestra en pantalla la indicacion de que cables se			*/
/*  encuentran en una posicion incorrecta						*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void InformarCablesConMalUbicacion ( void )
{
	// Variables auxiliares
	char	indiceContactoPlaca;
	char	renglon;
	char	offset;
	
	// Se informa que hay un error por que algunos pernos estan abiertos
	BorrarLCD();
	errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR,       0, 20, "Alter:     xx-xx-xx " );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 20, "         xx-xx-xx-xx" );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 20, "         xx-xx-xx-xx" );
	errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR,       0, 20, "           xx-xx-xx " );
	
	for( indiceContactoPlaca = 1; indiceContactoPlaca < TOTAL_DE_PERNOS_CABLE_MAZO_66A + 1; indiceContactoPlaca++ )
	{
		// Si el perno esta en su lugar correcto, no se indica nada
		if( condicionDelPerno[ indiceContactoPlaca ] == PERNO_SIN_ALTERNANCIA )
		{
			// Se calcula el offset y el renglon para la ubicacion de cada indicacion
			switch( indiceContactoPlaca )
			{
				case 1:
					renglon = RENGLON_SUPERIOR;
					offset = 11;
					break;
				case 2:
					renglon = RENGLON_SUPERIOR;
					offset = 14;
					break;
				case 3:
					renglon = RENGLON_SUPERIOR;
					offset = 17;
					break;
					
				case 4:
					renglon = RENGLON_MEDIO_SUPERIOR;
					offset = 9;
					break;
				case 5:
					renglon = RENGLON_MEDIO_SUPERIOR;
					offset = 12;
					break;
				case 6:
					renglon = RENGLON_MEDIO_SUPERIOR;
					offset = 15;
					break;
				case 7:
					renglon = RENGLON_MEDIO_SUPERIOR;
					offset = 18;
					break;
					
				case 8:
					renglon = RENGLON_MEDIO_INFERIOR;
					offset = 9;
					break;
				case 9:
					renglon = RENGLON_MEDIO_INFERIOR;
					offset = 12;
					break;
				case 10:
					renglon = RENGLON_MEDIO_INFERIOR;
					offset = 15;
					break;
				case 11:
					renglon = RENGLON_MEDIO_INFERIOR;
					offset = 18;
					break;
					
				case 12:
					renglon = RENGLON_INFERIOR;
					offset = 11;
					break;
				case 13:
					renglon = RENGLON_INFERIOR;
					offset = 14;
					break;
				case 14:
					renglon = RENGLON_INFERIOR;
					offset = 17;
					break;
			}
			
			// Se imprime la ubucacion que se encuentra en un lugar incorrecto
			EscribirUbicacion( pernosUbicacion[ indiceContactoPlaca ], renglon, offset );
		}
	}
}







		/****************************************************************************************************************/
		/*  								FUNCIONES PARA LA PRUEBA DEL MANILLAR ENYGMA								*/
		/****************************************************************************************************************/


/****************************************************************/
/* PruebaManillarEnygma											*/
/*  															*/
/*  Ejecuta una secuencia de prueba para verificar el correcto	*/
/*  funcionamiento del manillar de enygma. A medida que se va	*/
/*  realizando la prueba, va informando el estado en el LCD.	*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
bool PruebaManillarEnygma ( void )
{
	// Variable para saber si hubo algun error
	bool errorEnLaPrueba;
	
	// Se borra el flag
	errorEnLaPrueba = FALSE;
	
	// Se borra el LCD para indicar el estado de la prueba
	BorrarLCD();
	
	// Se ejecuta la secuencia de medicion de los pernos
	errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 20, "Pernos:  -----------" );
	estadoPernos = MedirPernosManillarEnygma();
	switch( estadoPernos )
	{
		case PERNO_SIN_ERRORES:
			errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 9, 11, "TODO OK    " );
			break;
		case PERNO_ABIERTO:
			InformarPernosAbiertos( MANILLAR_ENYGMA );
			errorEnLaPrueba = TRUE;
			break;
		case PERNO_CORTO_CIRCUITO:
			InformarPernosEnCorto( MANILLAR_ENYGMA );
			errorEnLaPrueba = TRUE;
			break;
		case PERNO_SIN_ALTERNANCIA:
			InformarMalUbicacion( MANILLAR_ENYGMA );
			errorEnLaPrueba = TRUE;
			break;
		case PERNO_CORTO_CIRCUITO_CARCAZA:
			InformarCortoConLaCarcaza( MANILLAR_ENYGMA );
			errorEnLaPrueba = TRUE;
			break;
		case PERNO_CORTOCIRCUITO_CON_5V:
			errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 0, 20, "Pernos: Corto con 5V" );
			errorEnLaPrueba = TRUE;
			break;
	}

	// Se ejecuta la secuencia de medicion del sensor de temperatura
	ActualizarListadoDeSensores();
	
	if( sensores[ SENSOR_MANILLAR_O_EXTRA ].usado == FALSE )
	{
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 20, " Falla en el sensor " );
		errorEnLaPrueba = TRUE;
	}
	else
	{
		LeerTemperaturaDS18S20( SENSOR_MANILLAR_O_EXTRA );
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 20, "Temperatura:        " );
		EscribirTemperatura( sensores[ SENSOR_MANILLAR_O_EXTRA ].temperatura, RENGLON_MEDIO_SUPERIOR, 13 );
	}
	
	return( errorEnLaPrueba );
	
}



resultadoPernos MedirPernosManillarEnygma ( void )
{
	// Variables auxiliares
	char indiceContactoPlaca;
	char paresCorrectamenteAlternados;
	bool cortocircuitoCon5V;
	bool cortocircuitoConElMazo;
	bool errorAlternancia;
	
	// Se borran los contadores
	paresCorrectamenteAlternados = 0;
	errorAlternancia = FALSE;
	cortocircuitoCon5V = FALSE;
	
	// Se levanta la matriz de contactos de los pernos
	ObtenerMatrizDePernos( MANILLAR_ENYGMA );
	
	// Primero se revisa que ningun perno tenga continuidad de por si con la linea de 5V
	if( hayPernoEnCortoCon5V == TRUE )
		return( PERNO_CORTOCIRCUITO_CON_5V );
	
	// Luego se verifica que no haya ningun perno en contacto con la carcaza
	if( pernosUbicacion[ UBICACION_CONTACTO_CARCAZA ] == HAY_CONTINUIDAD )
		return( PERNO_CORTO_CIRCUITO_CARCAZA );
	
	// Se revisa el estado de los contactos para diagnosticar los que esten abiertos y los que esten en corto
	for( indiceContactoPlaca = 1; indiceContactoPlaca < TOTAL_DE_PERNOS_MANILLAR_ENYGMA + 1; indiceContactoPlaca++ )
	{
		// Primero se revisa que no este abierto
		if( pernos[ indiceContactoPlaca ].totalDeContactos == 0 )
			{	condicionDelPerno[ indiceContactoPlaca ] = PERNO_ABIERTO;	hayPernoAbierto = TRUE;	}
		
		// Luego se verifica que no este en cortocircuito
		else
			if( pernos[ indiceContactoPlaca ].totalDeContactos > 1 )
				{	condicionDelPerno[ indiceContactoPlaca ] = PERNO_CORTO_CIRCUITO;	hayPernoEnCorto = TRUE;	}
	}
	
	// Si hay algun perno abierto o en cortocircuito, no se prosigue con la verificacion de las ubicaciones
	if( hayPernoAbierto == TRUE )
		return( PERNO_ABIERTO );
	if( hayPernoEnCorto == TRUE )
		return( PERNO_CORTO_CIRCUITO );

	// Por ultimo, se verifica la correcta ubicacion de los contactos
	for( indiceContactoPlaca = 1; indiceContactoPlaca < TOTAL_DE_PERNOS_MANILLAR_ENYGMA + 1; indiceContactoPlaca++ )
	{
		if( pernos[ indiceContactoPlaca ].contactos[ indiceContactoPlaca ] == NO_HAY_CONTINUIDAD )
			{	condicionDelPerno[ indiceContactoPlaca ] = PERNO_SIN_ALTERNANCIA;	errorAlternancia = TRUE;	}
		else
			condicionDelPerno[ indiceContactoPlaca ] = PERNO_SIN_ERRORES;
		
	}
	
	// Se indica si hay algun perno fuera de lugar
	if( errorAlternancia == TRUE )
		return( PERNO_SIN_ALTERNANCIA );
	
	
	return( PERNO_SIN_ERRORES );
}



















		/****************************************************************************************************************/
		/*	  								FUNCIONES PARA LA COMUNICACION SERIE										*/
		/****************************************************************************************************************/


/****************************************************************/
/* InicializarComunicacionSerie									*/
/*  															*/
/*  Configura el modulo para utilizar una comunicacion RS232	*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void InicializarComunicacionSerie( void )
{
	// Se selecciona un baudaje de 9600 (en realidad no da extacto, al ser fBus / ( 16 * SCI2BD ) )
	SCI2BDH = 0;
	SCI2BDL = 25;
	
	// Registros de control
		// SCI2C1 - SCI2 Control Register 1;
		SCI2C1_PT = 0;			// Parity Type = Even parity
		SCI2C1_PE = 0;			// Parity Enable = No hardware generation
		SCI2C1_ILT = 0;			// Idle Line Type Select = Idle character bit count starts after start bit
		SCI2C1_WAKE = 0;		// Receiver Wakeup Method Select = Idle-Line wakeup
		SCI2C1_M = 0;			// 9-Bit or 8-Bit Mode Select = 8 bits
		SCI2C1_RSRC = 0;		// Receiver Source Select = Don't use
		SCI2C1_SCISWAI = 0;		// SCI Stops in Wait Mode = Don't freeze SCI clocks in wait mode
		SCI2C1_LOOPS = 0;		// Loop Mode Select = No loop
	
		// SCI1C2 - SCI1 Control Register 2;
		SCI2C2_SBK = 0;			// Send Break = Normal transmitter operation
		SCI2C2_RWU = 0;			// Receiver Wakeup Control = Normal SCI receiver operation
		SCI2C2_RE = 0;			// Receiver Enable = Receiver off
		SCI2C2_TE = 0;			// Transmitter Enable = Transmitter off 
		SCI2C2_ILIE = 0;		// Idle Line Interrupt Enable (for IDLE) = Hardware interrupts from IDLE disabled
		SCI2C2_RIE = 0;			// Receiver Interrupt Enable (for RDRF) = Hardware interrupts from RDFR disabled
		SCI2C2_TCIE = 0;		// Transmission Complete Interrupt Enable (for TC) = Hardware interrupts requested when TC flag is 1 
		SCI2C2_TIE = 0;			// Transmit Interrupt Enable (for TDRE) = Hardware interrupts from TDRE disabled
		
		// SCI1C3 - SCI1 Control Register 3;
		SCI2C3_PEIE = 0;		// Parity Error Interrupt Enable = PF interrupts disabled 
		SCI2C3_FEIE = 0;		// Framing Error Interrupt Enable = FE interrupts disabled
		SCI2C3_NEIE = 0;		// Noise Error Interrupt Enable = NF interrupts disabled
		SCI2C3_ORIE = 0;		// Overrun Interrupt Enable = OR interrupts disabled
		SCI2C3_TXINV = 0;		// Transmit Data Inversion = Transmit data not inverted
		SCI2C3_TXDIR = 0;		// TxD Pin Direction in Single-Wire Mode = Don't use
		SCI2C3_T8 = 0;			// Ninth Data Bit for Transmitter = Don't use
		SCI2C3_R8 = 0;			// Ninth Data Bit for Receiver = Don't use
}



void EnviarTramaPruebaReducida_ManillarCrio ( void )
{
	// Variables auxiliares
	char	auxiliar;
	
	char	flag;
	
	// Se enciende el modulo para transmitir
	SCI2C2_TE = 1;
	
	// Se coloca el header inicial, para indicar donde arrancan los datos
	bufferTransmicion[ 0 ] = COMUNICACION_HEADER_INICIAL_PRUEBA_REDUCIDA;
	
	// Se escribe la temperatura del sensor soldado en la placa
	bufferTransmicion[ 1 ] = sensores[ SENSOR_SOLDADO_EN_PLACA ].temperatura;
	bufferTransmicion[ 2 ] = sensores[ SENSOR_SOLDADO_EN_PLACA ].temperatura >> 8;
	
	// Se escribe la temperatura del sensor para el lado caliente
	bufferTransmicion[ 3 ] = sensores[ SENSOR_PARA_LADO_CALIENTE ].temperatura;
	bufferTransmicion[ 4 ] = sensores[ SENSOR_PARA_LADO_CALIENTE ].temperatura >> 8;
	
	// Se escribe la temperatura del sensor propio de la Peltier
	bufferTransmicion[ 5 ] = sensores[ SENSOR_MANILLAR_O_EXTRA ].temperatura;
	bufferTransmicion[ 6 ] = sensores[ SENSOR_MANILLAR_O_EXTRA ].temperatura >> 8;
	
	// Se escribe la corriente de la Peltier
	bufferTransmicion[ 7 ] = medicionesCorrientePeltier[1];
	bufferTransmicion[ 8 ] = medicionesCorrientePeltier[1] >> 8;
	
	// Se coloca el header final, para indicar donde acaban los datos
	bufferTransmicion[ 9 ] = COMUNICACION_HEADER_FINAL;
	
	// Se escriben los caracteres en el buffer de pre transmicion
	for( auxiliar = 0; auxiliar < BYTES_TRAMA_PRUEBA_REDUCIDA; auxiliar++ )
	{
		// Se aguarda a que el buffer de pre transmicion este disponible para un nuevo caracter
		flag = 0;
		while( flag == 0 )
		{
			flag = SCI2S1;
			flag &= 0x80;
		}
		
		// Se escribe el dato
		SCI2D = bufferTransmicion[ auxiliar ];
		
	}
	
	// Se espera a que se termine la transmicion para apagar el modulo
	flag = 0;
	while( flag == 0 )
	{
		flag = SCI2S1;
		flag &= 0x40;
	}
	
	// Se apaga el modulo para transmitir
	SCI2C2_TE = 0;
	
	return;
}



void EnviarTramaPruebaCompleta_ManillarCrio ( void )
{
	// Variables auxiliares
	char	auxiliar;
	
	char	flag;
	
	// Se enciende el modulo para transmitir
	SCI2C2_TE = 1;
	
	// Se coloca el header inicial, para indicar donde arrancan los datos
	bufferTransmicion[ 0 ] = COMUNICACION_HEADER_INICIAL_PRUEBA_COMPLETA;
	
	// Se escribe la temperatura del sensor soldado en la placa
	bufferTransmicion[ 1 ] = sensores[ SENSOR_SOLDADO_EN_PLACA ].temperatura;
	bufferTransmicion[ 2 ] = sensores[ SENSOR_SOLDADO_EN_PLACA ].temperatura >> 8;
	
	// Se escribe la temperatura del sensor para el lado caliente
	bufferTransmicion[ 3 ] = sensores[ SENSOR_PARA_LADO_CALIENTE ].temperatura;
	bufferTransmicion[ 4 ] = sensores[ SENSOR_PARA_LADO_CALIENTE ].temperatura >> 8;
	
	// Se escribe la temperatura del sensor propio de la Peltier
	bufferTransmicion[ 5 ] = sensores[ SENSOR_MANILLAR_O_EXTRA ].temperatura;
	bufferTransmicion[ 6 ] = sensores[ SENSOR_MANILLAR_O_EXTRA ].temperatura >> 8;
	
	// Se escribe la corriente de la Peltier
	bufferTransmicion[ 7 ] = medicionesCorrientePeltier[1];
	bufferTransmicion[ 8 ] = medicionesCorrientePeltier[1] >> 8;
	
	// Se escribe la temperatura del sensor en la manguera de agua
	bufferTransmicion[ 9 ] = sensores[ SENSOR_PARA_LA_MANGUERA ].temperatura;
	bufferTransmicion[ 10 ] = sensores[ SENSOR_PARA_LA_MANGUERA ].temperatura >> 8;
	
	// Se coloca el header final, para indicar donde acaban los datos
	bufferTransmicion[ 11 ] = COMUNICACION_HEADER_FINAL;
	
	// Se escriben los caracteres en el buffer de pre transmicion
	for( auxiliar = 0; auxiliar < BYTES_TRAMA_PRUEBA_COMPLETA; auxiliar++ )
	{
		// Se aguarda a que el buffer de pre transmicion este disponible para un nuevo caracter
		flag = 0;
		while( flag == 0 )
		{
			flag = SCI2S1;
			flag &= 0x80;
		}
		
		// Se escribe el dato
		SCI2D = bufferTransmicion[ auxiliar ];
		
	}
	
	// Se espera a que se termine la transmicion para apagar el modulo
	flag = 0;
	while( flag == 0 )
	{
		flag = SCI2S1;
		flag &= 0x40;
	}
	
	// Se apaga el modulo para transmitir
	SCI2C2_TE = 0;
	
	return;
}
