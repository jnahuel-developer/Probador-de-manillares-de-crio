/* Including needed modules to compile this module/procedure */
#include "Cpu.h"
#include "Events.h"
#include "Vtpm1ovf.h"
/* Include shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"

/* User includes (#include below this line is not maintained by Processor Expert) */
#include "Display.h"
#include "DS18S20.h"
#include "Timer.h"







		/****************************************************************************************************************/
		/*					  						DEFINICIONES GENERALES												*/
		/****************************************************************************************************************/

	/* Continuidad de los pernos */

#define		MINIMAS_CUENTRAS_PARA_CONTINUIDAD		100
#define		HAY_CONTINUIDAD							TRUE
#define		NO_HAY_CONTINUIDAD						FALSE
#define		TOTAL_DE_CONTACTOS_MULTIPLEXORES		16


// Cantidad de pernos para cada target
#define		TOTAL_DE_PERNOS_MANILLAR_CRIO			7
#define		TOTAL_DE_PERNOS_MANILLAR_ENYGMA			4
#define		TOTAL_DE_PERNOS_CABLE_MAZO_66A			14


// Defines para revisar las posiciones de los pernos segun la rotacion
#define		ROTADO_0_POSICIONES						2
#define		VALOR_J2_ROTADO_0_POSICIONES			2
#define		VALOR_J3_ROTADO_0_POSICIONES			3
#define		VALOR_J4_ROTADO_0_POSICIONES			4
#define		VALOR_J5_ROTADO_0_POSICIONES			5
#define		VALOR_J6_ROTADO_0_POSICIONES			6
#define		VALOR_J7_ROTADO_0_POSICIONES			7

#define		ROTADO_1_POSICIONES						3
#define		VALOR_J2_ROTADO_1_POSICIONES			3
#define		VALOR_J3_ROTADO_1_POSICIONES			4
#define		VALOR_J4_ROTADO_1_POSICIONES			5
#define		VALOR_J5_ROTADO_1_POSICIONES			6
#define		VALOR_J6_ROTADO_1_POSICIONES			7
#define		VALOR_J7_ROTADO_1_POSICIONES			2

#define		ROTADO_2_POSICIONES						4
#define		VALOR_J2_ROTADO_2_POSICIONES			4
#define		VALOR_J3_ROTADO_2_POSICIONES			5
#define		VALOR_J4_ROTADO_2_POSICIONES			6
#define		VALOR_J5_ROTADO_2_POSICIONES			7
#define		VALOR_J6_ROTADO_2_POSICIONES			2
#define		VALOR_J7_ROTADO_2_POSICIONES			3

#define		ROTADO_3_POSICIONES						5
#define		VALOR_J2_ROTADO_3_POSICIONES			5
#define		VALOR_J3_ROTADO_3_POSICIONES			6
#define		VALOR_J4_ROTADO_3_POSICIONES			7
#define		VALOR_J5_ROTADO_3_POSICIONES			2
#define		VALOR_J6_ROTADO_3_POSICIONES			3
#define		VALOR_J7_ROTADO_3_POSICIONES			4

#define		ROTADO_4_POSICIONES						6
#define		VALOR_J2_ROTADO_4_POSICIONES			6
#define		VALOR_J3_ROTADO_4_POSICIONES			7
#define		VALOR_J4_ROTADO_4_POSICIONES			2
#define		VALOR_J5_ROTADO_4_POSICIONES			3
#define		VALOR_J6_ROTADO_4_POSICIONES			4
#define		VALOR_J7_ROTADO_4_POSICIONES			5

#define		ROTADO_5_POSICIONES						7
#define		VALOR_J2_ROTADO_5_POSICIONES			7
#define		VALOR_J3_ROTADO_5_POSICIONES			2
#define		VALOR_J4_ROTADO_5_POSICIONES			3
#define		VALOR_J5_ROTADO_5_POSICIONES			4
#define		VALOR_J6_ROTADO_5_POSICIONES			5
#define		VALOR_J7_ROTADO_5_POSICIONES			6

#define		INVERTIDO_J1_J2							1
#define		VALOR_J1_ROTADO_X_POSICIONES			1

#define		UBICACION_CONTACTO_CARCAZA				7


	/* Peltier */

#define		MAXIMAS_CUENTRAS_PARA_PELTIER			205			// Minima tension que se puede leer y representa un sobre consumo 
#define		MINIMAS_CUENTAS_PELTIER_ENCENDIDA		10
#define		MEDICIONES_PELTIER						65
#define		MEDICIONES_PELTIER_CON_RECIRCULACION	31
#define		MINUTOS_PELTIER_CON_RECIRCULACION		32
#define		TEMPERATURA_MINIMA_PELTIER_INVERTIDA	-20
#define		TEMPERATURA_MINIMA_PELTIER_APAGADA		10
#define		RESOLUCION_MA_PELTIER					33


	/* LEDs */

#define		MINIMAS_CUENTRAS_LEDS_2_RAMAS			36
#define		MINIMAS_CUENTRAS_LEDS_1_RAMA			MINIMAS_CUENTRAS_LEDS_2_RAMAS / 2


	/* Teclas */

#define		MINIMO_DE_OPRESIONES_CUALQUIER_TECLA	5
#define		MINIMO_DE_VECES_SIN_REVISAR_OPRESION	20


	/* Comunicacion serie */

#define		COMUNICACION_HEADER_INICIAL_PRUEBA_REDUCIDA		0xAA
#define		COMUNICACION_HEADER_INICIAL_PRUEBA_COMPLETA		0xEE
#define		COMUNICACION_HEADER_FINAL						0x55
#define		BYTES_TRAMA_PRUEBA_COMPLETA						12
#define		BYTES_TRAMA_PRUEBA_REDUCIDA						10





		/****************************************************************************************************************/
		/*			  						DEFINICIONES DE NUEVOS OBJETOS Y DATOS										*/
		/****************************************************************************************************************/

// Definicion de los estados de la maquina de menues
typedef enum estadosFunciones {
	SIN_PRUEBAS = 0,
	SENSORES_DE_TEMPERATURA_AGREGAR,
	SENSORES_DE_TEMPERATURA_QUITAR,
	SENSORES_DE_TEMPERATURA_MOSTRAR_EXTRA,
	SENSORES_DE_TEMPERATURA_MOSTRAR_CONECTADOS,
	PRUEBA_CRIO_REDUCIDA,
	PRUEBA_CRIO_COMPLETA
};

// Definicion para interpretar mas facilmente los errores de los pernos
typedef enum erroresPernos {
	PERNO_SIN_ERRORES = 0,
	PERNO_ABIERTO,
	PERNO_CORTO_CIRCUITO,
	PERNO_CORTO_CIRCUITO_CARCAZA,
	PERNO_SIN_ALTERNANCIA,
	PERNO_CORTOCIRCUITO_CON_5V
} resultadoPernos;

// Definicion para interpretar mas facilmente los errores de los LEDs
typedef enum erroresLEDs {
	LEDS_SIN_ERRORES = 0,
	LEDS_UNA_RAMA,
	LEDS_NINGUNA_RAMA
} resultadoLEDs;

// Definicion para interpretar mas facilmente los errores del sensor de temperatura
typedef enum erroresSensor {
	SENSOR_SIN_ERRORES = 0,
	SENSOR_ERROR_ACTUALIZAR_LISTADO,
	SENSOR_ERROR_NO_AGREGADO,
	SENSOR_ERROR_TEMPERATURA
} resultadoSensor;

// Definicion para interpretar mas facilmente los errores de la Peltier
typedef enum erroresPeltier {
	PELTIER_SIN_ERRORES = 0,
	PELTIER_INVERTIDA,
	PELTIER_SOBRE_CONSUMO,
	PELTIER_NO_ENFRIA,
	PELTIER_APAGADA,
	PELTIER_ENFRIA_DEMASIADO,
	PELTIER_SE_DESCONECTO
} resultadoPeltier;

// Definiciones para los canales del ADC
typedef enum {
	ADC_CANAL_PELTIER = 0,
	ADC_CANAL_PERNOS,
	ADC_CANAL_LEDS
};

// Definicion para interpretar mas facilmente los estados de los pulsadores
typedef enum teclaPresionada {
	TECLA_DERECHA_O_ABAJO = 0,
	TECLA_IZQUIERDA_O_ARRIBA,
	TECLA_ACEPTAR,
	TECLA_CANCELAR,
	TECLA_SELECCIONAR_MENU
} estadosTeclas;


// Definicion de la estructura para manejar los pernos como una matriz mas facil de interpretar
typedef struct	{
	bool	contactos[ TOTAL_DE_CONTACTOS_MULTIPLEXORES ];
	char	totalDeContactos;
} pernosObj;


// Definicion de los targets para las direcciones de los multiplexores
typedef enum targetMult {
	MANILLAR_CRIO = 0,
	MANILLAR_ENYGMA,
	MANILLAR_FACIAL,
	CABLE_MAZO_66A
} targetMultiplexores;




		/****************************************************************************************************************/
		/*				  						DEFINICIONES DE LAS CONEXIONES											*/
		/****************************************************************************************************************/

// Defines para los LEDs
#define  	LED_ROJO_DIRECCION						PTCDD_PTCDD3
#define  	LED_ROJO_PIN							PTCD_PTCD3
#define  	LED_VERDE_DIRECCION						PTCDD_PTCDD5
#define  	LED_VERDE_PIN							PTCD_PTCD5

// Defines para la Peltier
#define  	HABILITAR_PELTIER_DIRECCION				PTCDD_PTCDD1
#define  	HABILITAR_PELTIER_PIN					PTCD_PTCD1

// Defines para la Bomba
#define  	HABILITAR_BOMBA_DIRECCION				PTEDD_PTEDD3
#define  	HABILITAR_BOMBA_PIN						PTED_PTED3
#define  	CAUDALIMETRO_BOMBA_DIRECCION			PTEDD_PTEDD1
#define  	CAUDALIMETRO_BOMBA_PIN					PTED_PTED1
#define  	CAUDALIMETRO_EXTERNO_DIRECCION			PTFDD_PTFDD5
#define  	CAUDALIMETRO_EXTERNO_PIN				PTFD_PTFD5

// Defines para la comunicacion RS485
#define  	HABILITAR_PTE_RS485_DIRECCION			PTCDD_PTCDD4
#define  	HABILITAR_PTE_RS485_PIN					PTCD_PTCD4

// Defines para habilitar los LEDs del manillar
#define  	HABILITAR_LEDS_MANILLAR_DIRECCION		PTCDD_PTCDD2
#define  	HABILITAR_LEDS_MANILLAR_PIN				PTCD_PTCD2

// Defines para los pulsadores
#define  	PULSADOR_S1_DIRECCION					PTEDD_PTEDD0
#define  	PULSADOR_S1_PIN							PTED_PTED0
#define  	PULSADOR_S1_PUERTO						PTED
#define		PULSADOR_S1_MASCARA						PTED_PTED0_MASK		// Es para leer solo el pin del pulsador

#define  	PULSADOR_S2_DIRECCION					PTFDD_PTFDD1
#define  	PULSADOR_S2_PIN							PTFD_PTFD1
#define  	PULSADOR_S2_PUERTO						PTFD
#define		PULSADOR_S2_MASCARA						PTFD_PTFD1_MASK		// Es para leer solo el pin del pulsador

#define  	PULSADOR_S3_DIRECCION					PTFDD_PTFDD4
#define  	PULSADOR_S3_PIN							PTFD_PTFD4
#define  	PULSADOR_S3_PUERTO						PTFD
#define		PULSADOR_S3_MASCARA						PTFD_PTFD4_MASK		// Es para leer solo el pin del pulsador

#define  	PULSADOR_S4_DIRECCION					PTEDD_PTEDD1
#define  	PULSADOR_S4_PIN							PTED_PTED1
#define  	PULSADOR_S4_PUERTO						PTED
#define		PULSADOR_S4_MASCARA						PTED_PTED1_MASK		// Es para leer solo el pin del pulsador

#define  	PULSADOR_SEL_DIRECCION					PTEDD_PTEDD2
#define  	PULSADOR_SEL_PIN						PTED_PTED2
#define  	PULSADOR_SEL_PUERTO						PTED
#define		PULSADOR_SEL_MASCARA					PTED_PTED2_MASK		// Es para leer solo el pin del pulsador


// Defines para los pines del multiplexor
#define		S0_MULTIPLEXOR_PLACA_PIN				PTBD_PTBD3			// PTB3
#define		S1_MULTIPLEXOR_PLACA_PIN				PTBD_PTBD2			// PTB2
#define		S2_MULTIPLEXOR_PLACA_PIN				PTAD_PTAD0			// PTA0
#define		S3_MULTIPLEXOR_PLACA_PIN				PTGD_PTGD2			// PTG2
#define		E_MULTIPLEXOR_PLACA_PIN					PTAD_PTAD1			// PTA1
#define		S0_MULTIPLEXOR_PERNOS_PIN				PTGD_PTGD3			// PTG3
#define		S1_MULTIPLEXOR_PERNOS_PIN				PTCD_PTCD0			// PTC0
#define		S2_MULTIPLEXOR_PERNOS_PIN				PTDD_PTDD1			// PTD1
#define		S3_MULTIPLEXOR_PERNOS_PIN				PTDD_PTDD2			// PTD2
#define		E_MULTIPLEXOR_PERNOS_PIN				PTDD_PTDD0			// PTD0

#define		S0_MULTIPLEXOR_PLACA_DIRECCION			PTBDD_PTBDD3		// PTB3
#define		S1_MULTIPLEXOR_PLACA_DIRECCION			PTBDD_PTBDD2		// PTB2
#define		S2_MULTIPLEXOR_PLACA_DIRECCION			PTADD_PTADD0		// PTA0
#define		S3_MULTIPLEXOR_PLACA_DIRECCION			PTGDD_PTGDD2		// PTG2
#define		E_MULTIPLEXOR_PLACA_DIRECCION			PTADD_PTADD1		// PTA1
#define		S0_MULTIPLEXOR_PERNOS_DIRECCION			PTGDD_PTGDD3		// PTG3
#define		S1_MULTIPLEXOR_PERNOS_DIRECCION			PTCDD_PTCDD0		// PTC0
#define		S2_MULTIPLEXOR_PERNOS_DIRECCION			PTDDD_PTDDD1		// PTD1
#define		S3_MULTIPLEXOR_PERNOS_DIRECCION			PTDDD_PTDDD2		// PTD2
#define		E_MULTIPLEXOR_PERNOS_DIRECCION			PTDDD_PTDDD0		// PTD0

#define		PERNOS_SALIDA_CONTINUIDAD_DIRECCION		PTDDD_PTDDD3		// PTD3
#define		PERNOS_SALIDA_CONTINUIDAD				AD1P11





		/****************************************************************************************************************/
		/*					  						PROTOTIPOS DE LAS FUNCIONES											*/
		/****************************************************************************************************************/


	/* FUNCIONES GENERALES */

void 			wait 						( void );
void 			EsperarOpresionDeTecla		( char pulsador );
estadosTeclas 	RevisarOpresionDeTeclas 	( void );
void 			ApagarLedVerde 				( void );
void 			ApagarLedRojo 				( void );
void 			EncenderLedVerde 			( void );
void 			EncenderLedRojo 			( void );
void 			IndicarPruebaBien 			( void );
void 			IndicarPruebaMal 			( void );
void 			ApagarLeds 					( void );
void 			AlternarLedRojo				( void );
void 			AlternarLedVerde			( void );


	/* FUNCIONES PARA LA PRUEBA REDUCIDA DEL MANILLAR */

bool 				PruebaManillarReducido 					( void );
resultadoPernos 	MedicionDeLosPernos_ManillarCrio		( void );
resultadoLEDs 		MedicionDeLosLEDS 						( void );
resultadoSensor 	MedicionDelSensorDeTemperaturaManillar	( void );
resultadoPeltier	MedicionDeLaPeltier 					( void );


	/* FUNCIONES PARA LAS PRUEBAS COMPLETAS DE LOS MANILLARES */

bool				PruebaCompletaManillarDeCrio 				( void );
resultadoPeltier	MedicionDeLaPeltierConRecirculacionActivada	( void );


	/* FIRMWARE PARA LAS PRUEBAS COMPLETAS DE LOS MANILLARES */

void 	HabilitarBombayCooler ( void );
void 	DeshabilitarBombayCooler ( void );


	/* FUNCIONES PARA LA PRUEBA DEL SENSOR EXTRA */

resultadoSensor RegistrarElSensorDeTemperaturaExtra 	( void );


	/* FIRMWARE PARA LA PRUEBA REDUCIDA DEL MANILLAR */

void ColocarDireccionMultiplexorPernos		( char direccion, char target );
void ColocarDireccionMultiplexorPlaca		( char direccion, char target );
void HabilitarLEDs							( void );
void DeshabilitarLEDs						( void );
void HabilitarBombaCooler					( void );
void DeshabilitarBombaCooler				( void );
void HabilitarPeltier						( void );
void DeshabilitarPeltier					( void );
void InicializarPuertosProbador				( void );


	/* FUNCIONES PARA EL ADC */

void 	ConfigurarADC 			( void );
bool 	TomarMuestraSimpleADC	( char canal );
bool 	iniciarADC_Simple 		( uint8_t canal );
uint8_t leerADC 				( void );
void 	borrarFlagADC 			( void );


	/* FUNCIONES PARA INFORMAR LOS ERRORES */

void	InformarPernosAbiertos 		( char target );
void	InformarPernosEnCorto 		( char target );
void 	InformarMalUbicacion 		( char target );
void	InformarCortoConLaCarcaza	( char target );


	/* FUNCIONES PARA MOSTRAR LAS PANTALLAS DE LOS MENUES */

void MostrarPantallaInicialPruebaReducida 	( void );
void MostrarPantallaInicialSensorExtra 		( void );
void MostrarPantallaTemperaturasIniciales 	( void );
void MostrarPantallaFinalPruebaReducida 	( void );
void MostrarPantallaFinalPruebaCompleta 	( void );
void MostrarPantallaMenuesPruebaCompleta 	( void );



resultadoPernos ProbarCables ( char target );
void InformarCablesAbiertos ( void );
void InformarCablesEnCorto ( void );
void InformarCablesConMalUbicacion ( void );



bool PruebaManillarEnygma ( void );
resultadoPernos MedirPernosManillarEnygma ( void );
void ObtenerMatrizDePernos ( char target );


	/* FUNCIONES PARA LA COMUNICACION SERIE */

void 			InicializarComunicacionSerie( void );
void 			EnviarTramaPruebaReducida_ManillarCrio ( void );
void 			EnviarTramaPruebaCompleta_ManillarCrio ( void );
