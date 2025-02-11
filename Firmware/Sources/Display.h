/****************************************************************************************************/
/****************************************************************************************************/
/* 								DEFINICIONES PARA MANEJAR LOS LCD 									*/
/****************************************************************************************************/
/****************************************************************************************************/

// Includes
#include "IO_Map.h"
#include "PE_Types.h"


// Las demoras de menos de 1mSeg se hacen bloqueantes
#define 	DEMORA_200US			TPM1MOD = 0x00C8;	TPM1CNT = 0;	TPM1SC |= 0x08;		while( TPM1CNT < 0x00C8 )	{};

// Las demas se realizan mediante la interrupcion del timer
#define		CUARENTA_MILI_SEGUNDOS		40000



/* Defines generales para hacer mas legible el codigo */

#define		ACTIVADO					1
#define		DESACTIVADO					0
#define		PARTE_ALTA					TRUE
#define		PARTE_BAJA					FALSE
#define 	RENGLON_SUPERIOR			0
#define 	RENGLON_MEDIO_SUPERIOR		1
#define 	RENGLON_MEDIO_INFERIOR		2
#define 	RENGLON_INFERIOR			3
#define		ROM_PARTE_ALTA				TRUE
#define		ROM_PARTE_BAJA				FALSE
#define		PARAMETRO_NO_UTILIZADO		0



/* Defines para las instrucciones de los LCD */

// Instrucciones para borrar la pantalla
#define 	INSTRUCCION_LCD_BORRAR_PANTALLA		0x01

// Instrucciones para regresar el cursor al inicio
#define 	INSTRUCCION_LCD_REGRESAR_CURSOR		0x20

// Instrucciones para seleccionar el modo
#define 	INSTRUCCION_LCD_MODO				0x04
#define 	INSTRUCCION_LCD_MODO_INCREMENTO		0X02
#define 	INSTRUCCION_LCD_MODO_DECREMENTO		0X00
#define 	INSTRUCCION_LCD_MODO_SHIFT_ON		0X01
#define 	INSTRUCCION_LCD_MODO_SHIFT_OFF		0X00

// Instrucciones para encender o apagar la pantalla
#define 	INSTRUCCION_LCD_PANTALLA			0x08
#define 	INSTRUCCION_LCD_PANTALLA_ON			0x04
#define 	INSTRUCCION_LCD_PANTALLA_OFF		0x00
#define 	INSTRUCCION_LCD_PANTALLA_CURSOR_ON	0x02
#define 	INSTRUCCION_LCD_PANTALLA_CURSOR_OFF	0x00
#define 	INSTRUCCION_LCD_PANTALLA_BLINK_ON	0x01
#define 	INSTRUCCION_LCD_PANTALLA_BLINK_OFF	0x00

// Instrucciones para desplazar el cursor o la pantalla
#define 	INSTRUCCION_LCD_DESPLAZAR			0x10
#define 	INSTRUCCION_LCD_DESPLAZAR_PANTALLA	0x08
#define 	INSTRUCCION_LCD_DESPLAZAR_CURSOR	0x00
#define 	INSTRUCCION_LCD_DESPLAZAR_DERECHA	0x04
#define 	INSTRUCCION_LCD_DESPLAZAR_IZQUIERDA	0x00

// Instrucciones para definir la interfaz
#define 	INSTRUCCION_LCD_INTERFAZ			0x20
#define 	INSTRUCCION_LCD_INTERFAZ_8BITS		0x10
#define 	INSTRUCCION_LCD_INTERFAZ_4BITS		0x00
#define 	INSTRUCCION_LCD_INTERFAZ_2LINEAS	0x08
#define 	INSTRUCCION_LCD_INTERFAZ_1LINEA		0x00
#define 	INSTRUCCION_LCD_INTERFAZ_5X10		0x04
#define 	INSTRUCCION_LCD_INTERFAZ_5X8		0x00

// Instrucciones para definir el renglon sobre el que se escribe
#define 	INSTRUCCION_LCD_RENGLON_SUPERIOR			0x80
#define 	INSTRUCCION_LCD_RENGLON_MEDIO_SUPERIOR		INSTRUCCION_LCD_RENGLON_SUPERIOR + 0x40
#define 	INSTRUCCION_LCD_RENGLON_MEDIO_INFERIOR		INSTRUCCION_LCD_RENGLON_SUPERIOR + 20
#define 	INSTRUCCION_LCD_RENGLON_INFERIOR			INSTRUCCION_LCD_RENGLON_MEDIO_SUPERIOR + 20



/* Defines para Display MangoVacio */

/*
// Data direction - Para elegir si son entradas o salidas
#define 	LCD_BACK_LIGHT_DD	PTGDD_PTGDD1 //backlight
#define  	LCD_RS_DD			PTEDD_PTEDD3 //rs
#define 	LCD_E_DD			PTEDD_PTEDD4 //e
#define 	LCD_DB4_DD 			PTEDD_PTEDD5 //db4
#define 	LCD_DB5_DD  		PTEDD_PTEDD6 //db5
#define		LCD_DB6_DD			PTEDD_PTEDD7 //db6
#define 	LCD_DB7_DD			PTGDD_PTGDD0 //db7

// Data - Para escribir o leer los bits de datos
#define 	LCD_BACK_LIGHT		PTGD_PTGD1 //backlight
#define  	LCD_RS				PTED_PTED3		//rs
#define 	LCD_E				PTED_PTED4		//e
#define 	LCD_DB4 			PTED_PTED5		//db4
#define 	LCD_DB5	    		PTED_PTED6		//db5
#define		LCD_DB6				PTED_PTED7		//db6
#define 	LCD_PUERTO			PTED			//es el puerto donde van DB6, DB5 y DB4
#define 	LCD_DB7				PTGD_PTGD0		//db7
*/

// Mascaras para acceder a los bits de datos
#define		BORRAR_DBX_PUERTO_MANGO_VACIO		0x1F		// Para dejar xxx00000 en el PTE que tiene la configuracion DB6-DB5-DB4-xxxxx
#define		MASCARA_DB654_DATO_MANGO_VACIO		0x70		// Para quedarse con 0xxx0000
#define		MASCARA_DB210_DATO_MANGO_VACIO		0x07		// Para quedarse con 00000xxx
#define		MASCARA_DB7_DATO_MANGO_VACIO		0x80		// Para quedarse con x0000000
#define		MASCARA_DB3_DATO_MANGO_VACIO		0x08		// Para quedarse con 0000x000
#define		ROTAR_DB654_MANGO_VACIO				1			// Desaplazar 0xxx0000 a xxx00000
#define		ROTAR_DB210_MANGO_VACIO				5			// Desaplazar 00000xxx a xxx00000


// Mascaras para acceder a los bits de datos
#define		BORRAR_DB76_PUERTO_PROBADOR		0xCF		// Para dejar xx00xxxx en el PTE que tiene la configuracion DB5-DB4-DB6-DB7-xxxxx
#define		BORRAR_DB54_PUERTO_PROBADOR		0x3F		// Para dejar 00xxxxxx en el PTE que tiene la configuracion DB5-DB4-DB6-DB7-xxxxx

#define		MASCARA_DB76_DATO_PROBADOR		0xC0		// Para quedarse con xx000000
#define		MASCARA_DB54_DATO_PROBADOR		0x30		// Para quedarse con 00xx0000
#define		MASCARA_DB32_DATO_PROBADOR		0x0C		// Para quedarse con 0000xx00
#define		MASCARA_DB10_DATO_PROBADOR		0x03		// Para quedarse con 000000xx

#define		ROTAR_DB76_PROBADOR				2			// Desaplazar xx000000 a 00xx0000
#define		ROTAR_DB54_PROBADOR				2			// Desaplazar 00xx0000 a xx000000
#define		ROTAR_DB32_PROBADOR				2			// Desaplazar 0000xx00 a 00xx0000
#define		ROTAR_DB10_PROBADOR				6			// Desaplazar 000000xx a xx000000


// Data direction - Para elegir si son entradas o salidas
#define  	LCD_RS_DD							PTGDD_PTGDD0 	//rs
#define 	LCD_E_DD							PTGDD_PTGDD1 	//e
#define 	LCD_DB4_DD 							PTEDD_PTEDD6 	//db4
#define 	LCD_DB5_DD  						PTEDD_PTEDD7 	//db5
#define		LCD_DB6_DD							PTEDD_PTEDD4 	//db6
#define 	LCD_DB7_DD							PTEDD_PTEDD5 	//db7

// Data - Para escribir o leer los bits de datos
#define  	LCD_RS								PTGD_PTGD0		//rs
#define 	LCD_E								PTGD_PTGD1		//e
#define 	LCD_DB4 							PTED_PTED6		//db4
#define 	LCD_DB5	    						PTED_PTED7		//db5
#define		LCD_DB6								PTED_PTED4		//db6
#define 	LCD_DB7								PTED_PTED5		//db7
#define 	LCD_PUERTO							PTED			//es el puerto donde van DB6, DB5 y DB4


// Defines para seleccionar que tipo de LCD se va a usar
#define		LCD_20_CARACTERES		20
#define		LCD_16_CARACTERES		16
#define		LCD_8_CARACTERES		8
#define		LCD_2_FILAS				2
#define		LCD_4_FILAS				4
#define		LCD_4_BITS				TRUE
#define		LCD_8_BITS				FALSE
#define		LCD_PUERTO_EXTERNO		TRUE
#define		LCD_PUERTO_INTERNO		FALSE
#define		LCD_FUENTE_CHICA		TRUE
#define		LCD_FUENTE_GRANDE		FALSE
#define		LCD_MANGO_VACIO			FALSE
#define		LCD_PROBADOR			TRUE


// Definicion de una estructura para manejar mas facilmente al LCD, como si fuesen un objeto
typedef struct
{
	char	filas;
	char	caracteres;
	bool	fuente;
	bool	interfaz;
	bool	puerto;
	bool	probador;
}LCD_Objeto;

// Mensajes predefinidos
typedef enum mensajesLCD { INICIADO = 0, SIN_SENSORES, BUSCANDO, SENSORES_ENCONTRADOS, TEXTO_FUNCION };







/* Prototipos de las funciones */

// De uso externo
bool SeleccionarLCD( char totalDeFilas, char totalDeCaracteres, bool fuente, bool interfaz_4bits, bool puertoExterno, bool probador );
void InicializarLCD( void );
void MostrarMensajeLCD( char tipoDeMensaje, char parametroExtra );
bool EscribirMensajeLCD( unsigned char renglon, unsigned char inicioTexto, unsigned char totalDeCaracteres, unsigned char *pTexto);
void EscribirTemperatura( int16_t temperatura, char reglon, char offset );
void EscribirVariacionesDeTemperatura( int16_t variacionDeTemperatura, char reglon, char offset );
void EscribirCorriente( int corriente_mA, char reglon, char offset );
void EscribirROM_EnLCD( unsigned char *ROM );
void BorrarLCD( void );

// De uso interno
void enviarInstruccion( char dato );
void enviarDato( char dato );
void mandarNibbleAltoLCD_Vacio( char dato );
void mandarNibbleBajoLCD_Vacio( char dato );
void mandarNibbleAltoLCD_Probador( char dato );
void mandarNibbleBajoLCD_Probador( char dato );
void inicializarLCD_4bits( void );
void inicializarPuertoLCD( void );

void escribirMediaROM_EnLCD( unsigned char renglon, unsigned char *ROM, bool parteAlta );
void escribirTodaROM_EnLCD( unsigned char renglon, unsigned char *ROM );


void demoraInicialLCD( void );


void mensajeLCD_Iniciado ( void );
void mensajeLCD_SinSensores( void );
void mensajeLCD_BuscandoSensores( void );
void mensajeLCD_FuncionDelSensor ( char posicion );
void mensajeLCD_SensoresEncontrados ( char encontrados );

void borrarRenglon( char renglon );


void ImprimirMensajeErrorPruebaReducida ( char sensor );
void MostrarTemperaturasPruebaReducida ( int16_t sensorSoldado, int16_t sensorLadoCaliente );

void EscribirUbicacion( char ubicacion, char reglon, char offset );
