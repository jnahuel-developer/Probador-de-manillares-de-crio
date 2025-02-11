// Includes
#include "IO_Map.h"
#include "PE_Types.h"

// 
#define		HABILITAR_INTERRUPCION_Y_ARRANCAR	0x48
#define		DESHABILITAR_INTERRUPCION_Y_FRENAR	0x00

#define		MILISEGUNDOS_50						50000
#define		MILISEGUNDOS_10						10000
#define		MILISEGUNDOS_1						1000

void InicializarTimer1( void );
void DemoraPorInterrupcion( uint16_t uSeg );
void DemoraEnSegundos( char segundos );
void DemoraEnMiliSegundos( char miliSegundos );
void DemoraParaConversionDS18S20( void );
void DemoraParaIniciarPeltier( void );
