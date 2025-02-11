/****************************************************************************************************/
/****************************************************************************************************/
/* 						DEFINICIONES PARA MANEJAR LOS SENSORES DE TEMPERATURA 						*/
/****************************************************************************************************/
/****************************************************************************************************/

// Includes
#include "IO_Map.h"
#include "PE_Types.h"



// Defines para los sensores que van a ser practicamente fijos en la placa:

	/*********************************************************************/
	/* *** Sensores para la placa de pruebas completas de manillares *** */
	/*********************************************************************/

	// Sensor soldado directamente
	#define	DS18S20_SOLDADO_BYTE_0		0x10
	#define	DS18S20_SOLDADO_BYTE_1		0x0B
	#define	DS18S20_SOLDADO_BYTE_2		0x38
	#define	DS18S20_SOLDADO_BYTE_3		0x5F
	#define	DS18S20_SOLDADO_BYTE_4		0x03
	#define	DS18S20_SOLDADO_BYTE_5		0x08
	#define	DS18S20_SOLDADO_BYTE_6		0x00
	#define	DS18S20_SOLDADO_BYTE_7		0x69

/*
	// Sensor para la manguera
	#define	DS18S20_MANGUERA_BYTE_0		0x10
	#define	DS18S20_MANGUERA_BYTE_1		0x59
	#define	DS18S20_MANGUERA_BYTE_2		0x2D
	#define	DS18S20_MANGUERA_BYTE_3		0x5F
	#define	DS18S20_MANGUERA_BYTE_4		0x03
	#define	DS18S20_MANGUERA_BYTE_5		0x08
	#define	DS18S20_MANGUERA_BYTE_6		0x00
	#define	DS18S20_MANGUERA_BYTE_7		0x87

	// Sensor para el lado caliente del manillar
	#define	DS18S20_LADO_CALIENTE_BYTE_0		0x10
	#define	DS18S20_LADO_CALIENTE_BYTE_1		0x85
	#define	DS18S20_LADO_CALIENTE_BYTE_2		0x4D
	#define	DS18S20_LADO_CALIENTE_BYTE_3		0x5F
	#define	DS18S20_LADO_CALIENTE_BYTE_4		0x03
	#define	DS18S20_LADO_CALIENTE_BYTE_5		0x08
	#define	DS18S20_LADO_CALIENTE_BYTE_6		0x00
	#define	DS18S20_LADO_CALIENTE_BYTE_7		0x2F
*/

	/******************************************************************************************/
	/* *** Sensores para la placa de pruebas reducidas con el display con todos los pines *** */
	/******************************************************************************************/
/*
	// Sensor soldado directamente
	#define	DS18S20_SOLDADO_BYTE_0		0x10
	#define	DS18S20_SOLDADO_BYTE_1		0xBE
	#define	DS18S20_SOLDADO_BYTE_2		0xA2
	#define	DS18S20_SOLDADO_BYTE_3		0x79
	#define	DS18S20_SOLDADO_BYTE_4		0x03
	#define	DS18S20_SOLDADO_BYTE_5		0x08
	#define	DS18S20_SOLDADO_BYTE_6		0x00
	#define	DS18S20_SOLDADO_BYTE_7		0xF6

	// Sensor para la manguera
	#define	DS18S20_MANGUERA_BYTE_0		0x10
	#define	DS18S20_MANGUERA_BYTE_1		0x59
	#define	DS18S20_MANGUERA_BYTE_2		0x2D
	#define	DS18S20_MANGUERA_BYTE_3		0x5F
	#define	DS18S20_MANGUERA_BYTE_4		0x03
	#define	DS18S20_MANGUERA_BYTE_5		0x08
	#define	DS18S20_MANGUERA_BYTE_6		0x00
	#define	DS18S20_MANGUERA_BYTE_7		0x87

	// Sensor para el lado caliente del manillar
	#define	DS18S20_LADO_CALIENTE_BYTE_0		0x10
	#define	DS18S20_LADO_CALIENTE_BYTE_1		0xDD
	#define	DS18S20_LADO_CALIENTE_BYTE_2		0x83
	#define	DS18S20_LADO_CALIENTE_BYTE_3		0x5F
	#define	DS18S20_LADO_CALIENTE_BYTE_4		0x03
	#define	DS18S20_LADO_CALIENTE_BYTE_5		0x08
	#define	DS18S20_LADO_CALIENTE_BYTE_6		0x00
	#define	DS18S20_LADO_CALIENTE_BYTE_7		0x20
*/

	/**************************************************************************************/
	/* *** Sensores para la placa de pruebas reducidas con el display con menos pines *** */
	/**************************************************************************************/
/*
	// Sensor soldado directamente
	#define	DS18S20_SOLDADO_BYTE_0		0x10
	#define	DS18S20_SOLDADO_BYTE_1		0x0E
	#define	DS18S20_SOLDADO_BYTE_2		0xD4
	#define	DS18S20_SOLDADO_BYTE_3		0x7A
	#define	DS18S20_SOLDADO_BYTE_4		0x03
	#define	DS18S20_SOLDADO_BYTE_5		0x08
	#define	DS18S20_SOLDADO_BYTE_6		0x00
	#define	DS18S20_SOLDADO_BYTE_7		0x88
*/
	// Sensor para la manguera
	#define	DS18S20_MANGUERA_BYTE_0		0x10
	#define	DS18S20_MANGUERA_BYTE_1		0x59
	#define	DS18S20_MANGUERA_BYTE_2		0x2D
	#define	DS18S20_MANGUERA_BYTE_3		0x5F
	#define	DS18S20_MANGUERA_BYTE_4		0x03
	#define	DS18S20_MANGUERA_BYTE_5		0x08
	#define	DS18S20_MANGUERA_BYTE_6		0x00
	#define	DS18S20_MANGUERA_BYTE_7		0x87

	// Sensor para el lado caliente del manillar
	#define	DS18S20_LADO_CALIENTE_BYTE_0		0x10
	#define	DS18S20_LADO_CALIENTE_BYTE_1		0x79
	#define	DS18S20_LADO_CALIENTE_BYTE_2		0x80
	#define	DS18S20_LADO_CALIENTE_BYTE_3		0x5F
	#define	DS18S20_LADO_CALIENTE_BYTE_4		0x03
	#define	DS18S20_LADO_CALIENTE_BYTE_5		0x08
	#define	DS18S20_LADO_CALIENTE_BYTE_6		0x00
	#define	DS18S20_LADO_CALIENTE_BYTE_7		0xEE





/* Macros para las demoras */
#define 	DEMORA_1US				asm( nop ); asm( nop ); asm( nop ); asm( nop );
#define 	DEMORA_480US			TPM1CNT = 0;	while( TPM1CNT < 0x01D8 )	{};		// Para el reset
#define 	DEMORA_410US			TPM1CNT = 0;	while( TPM1CNT < 0x0195 )	{};		// Para el reset
#define 	DEMORA_70US				TPM1CNT = 0;	while( TPM1CNT < 0x0041 )	{};		// Para el reset
#define 	DEMORA_10US				TPM1CNT = 0;	while( TPM1CNT < 0x0005 )	{};		// Para escribir un 1
#define 	DEMORA_55US				TPM1CNT = 0;	while( TPM1CNT < 0x0032 )	{};		// Para escribir un 1
#define 	DEMORA_65US				TPM1CNT = 0;	while( TPM1CNT < 0x003C )	{};		// Para escribir un 0
#define 	DEMORA_5US				TPM1CNT = 0;	while( TPM1CNT < 0x0003 )	{};		// Para escribir un 0
#define 	DEMORA_50mS				TPM1CNT = 0;	while( TPM1CNT < 0xC350 )	{};		// Para darle tiempo al sensor a que convierta la temperatura


/* Defines para el sensor */

#define  	DS18S20_DIRECCION		PTFDD_PTFDD0
#define  	DS18S20_PIN				PTFD_PTFD0
#define  	DS18S20_PUERTO			PTFD
#define		DS18S20_MASCARA			PTFD_PTFD0_MASK		// Es para leer solo el pin del sensor
#define		LONGITUD_ROM_BYTES		8
#define		LONGITUD_ROM_BITS		64
#define		LONGITUD_RESPUESTA		9
#define		TEMPERATURA_BYTE_BAJO	0
#define		TEMPERATURA_BYTE_ALTO	1
#define		TOTAL_DE_SENSORES		4
#define		TOTAL_DE_INTENTOS		10
#define		BYTES_TIPO_DE_SENSOR	0

#define		NO_MAS_COLISIONES		0xFF


typedef enum errores {
	SIN_ERROR = 0,
	PULSO_RESET,
	BUS_SIN_SENSORES,
	ORDEN_INCORRECTA,
	SENSOR_INCORRECTO,
	NO_MAS_SENSORES,
	TEMPERATURA_PROHIBIDA
} erroresDS18S20; 

typedef enum ubicacionSensores {
	SENSOR_SOLDADO_EN_PLACA = 0,
	SENSOR_PARA_LADO_CALIENTE,
	SENSOR_PARA_LA_MANGUERA,
	SENSOR_MANILLAR_O_EXTRA
} posicionSensores;

// Definicion de una estructura para manejar mas facilmente a los sensores, como si fuesen un objeto
typedef struct
{
	unsigned char 	ROM[8];
	int16_t			temperatura;	// Se almacena el valor multiplicado por 10, para evitar usar decimales
	bool			usado;
}sensorDS18S20;



/* **** Defines para las instrucciones de comando de los sensores **** */

// Instruccion para iniciar la conversion de temperatura
#define		INSTRUCCION_DS18S20_INICAR_CONVERSION_T			0

// Instruccion para que se identifique un sensor en particular
#define		INSTRUCCION_DS18S20_USAR_ROM_ID					INSTRUCCION_DS18S20_INICAR_CONVERSION_T + 1

// Instruccion para que todos los sensores tomen la orden
#define		INSTRUCCION_DS18S20_NO_USAR_ROM_ID				INSTRUCCION_DS18S20_USAR_ROM_ID + 1

// Instruccion para que el sensor identificado devuelva el dato de su temperatura
#define		INSTRUCCION_DS18S20_LEER_TEMPERATURA			INSTRUCCION_DS18S20_NO_USAR_ROM_ID + 1



/* Defines para los codigos que necesita el sensor DS18S20 para cada instruccion */

// Instruccion para indicar que se enviara un codigo ROM y solo debe tomar los datos el sensor que tenga ese codigo
#define 	CODIGO_DS18S20_IDENTIFICAR_ROM			0x55

// Instruccion para indicar que no se usara el codigo ROM para identificar a un sensor, con lo que todos leeran la siguiente orden
#define 	CODIGO_DS18S20_NO_IDENTIFICAR_ROM		0xCC

// Instruccion para que los sensores seleccionados (o todos) inicien la conversion de temperatura
#define 	CODIGO_DS18S20_INICIAR_CONVERSION_T		0x44

// Instruccion para indicar que se leera el valor de la conversion de temperatura del sensor seleccionado
#define 	CODIGO_DS18S20_LEER_TEMPERATURA			0xBE

// Instruccion para indicar a los sensores que coloquen en el bus sus numeros de ROM para listarlos
#define 	CODIGO_DS18S20_LEER_ROMS				0xF0




/* Prototipos de las funciones */

// De uso externo
bool 			EnviarResetDS18S20( void );
erroresDS18S20 	EnviarInstruccionDS18S20( char instruccion );
bool 			ActualizarListadoDeSensores( void );
erroresDS18S20 	LeerTemperaturaDS18S20( posicionSensores sensor );
erroresDS18S20	LeerTodasLasTemperaturasDS18S20( void );

// De uso interno
void escribirArrayDS18S20 ( char *array, char totalDeBytes );
void escribirByteDS18S20 ( char byte );
void escribirBitDS18S20 ( char bit );
void leerArrayDS18S20 ( void );
char leerByteDS18S20 ( void );
char leerBitDS18S20 ( void );
void interpretarDatosSensor( posicionSensores sensor );
bool enumerarDispositivos( void );
erroresDS18S20 leerROM_DS18S20 ( void );
bool verificarROM_EnUso ( void );
void agregarRom( posicionSensores indiceSensor );
bool leerUnaROM ( void );
bool restanLeerROMS( void );
void intercambiarROMS( char previa, char actual );
void reacomodarROMS( void );

void borrarRegistrosSensores( void );


// Funciones para la prueba reducida solamente
void InicializarSensoresProbador ( void );
char TomarTemperaturaSensoresPruebaReducida( void );


bool reordenarSensores( void );
