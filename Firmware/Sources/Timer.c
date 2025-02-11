#include "Timer.h"

// Se define una variable Flag para saber usar como sincronizacion con la interrupcion
bool flagTerminoElTimer = FALSE;



void InicializarTimer1(void)
{
  setReg8( TPM1SC, 0x00);
  setReg8( TPM1SC, 0x0A);    /* Set prescaler PS = 4; and run counter */
  
  // Se borra el flag de sincronizacion
  flagTerminoElTimer = FALSE;
}





/************************************************************/
/* DemoraPorInterrupcion									*/
/*  														*/
/*  Realiza la demora de los uSeg solicitados mediante la	*/
/*  interrupcion del timer. El tiempo de espera se pasa en	*/
/*  estado de espera para bajar el consumo.					*/
/*  														*/
/*  Recibe: Los uSeg a demorar								*/
/*  Devuelve: Nada											*/
/************************************************************/
void DemoraPorInterrupcion( uint16_t uSeg )
{
	// Se borra el flag de sincronizacion
	flagTerminoElTimer = FALSE;
	
	// Se carga el valor de la demora deseada en los registros del timer
	TPM1MOD = uSeg;
	
	// Se resetean los contadores
	TPM1CNT = 0;
	
	// Se habilita el modulo (con el bus interno) y se habilita la interrupcion
	TPM1SC |= HABILITAR_INTERRUPCION_Y_ARRANCAR;
	
	TPM1SC &= 0x7F;
	
	// Se entra en modo de espera hasta que desborde el timer
	while( flagTerminoElTimer == FALSE )
		asm( wait );
	
	// Se desactiva el modulo y las interrupciones
	clrReg8Bits(TPM1SC, DESHABILITAR_INTERRUPCION_Y_FRENAR);
}



/************************************************************/
/* DemoraEnSegundos											*/
/*  														*/
/*  Realiza la demora de los seg solicitados				*/
/*  														*/
/*  Recibe: Los seg a demorar								*/
/*  Devuelve: Nada											*/
/************************************************************/
void DemoraEnSegundos( char segundos )
{
	// Variable auxiliar
	char veces;
	
	for( ; segundos > 0; segundos-- )
		for( veces = 0; veces < 20; veces++ )
			DemoraPorInterrupcion( MILISEGUNDOS_50 );
}



/************************************************************/
/* DemoraParaConversionDS18S20								*/
/*  														*/
/*  Realiza la demora para que los sensores conviertan sus	*/
/*  temperaturas											*/
/*  														*/
/*  Recibe: Los seg a demorar								*/
/*  Devuelve: Nada											*/
/************************************************************/
void DemoraParaConversionDS18S20( void )
{
	// Variables auxiliares
	int veces;
	
	for( veces = 0; veces < 15; )
	{
		TPM1CNT = 0;
		while( TPM1CNT < 0xC350 )
			{};
		veces = veces + 1;
	}
}



/************************************************************/
/* DemoraParaIniciarPeltier									*/
/*  														*/
/*  Demora inicial para esperar que la Peltier se			*/
/*  estabilice.												*/
/*  														*/
/*  Recibe: Los seg a demorar								*/
/*  Devuelve: Nada											*/
/************************************************************/
void DemoraParaIniciarPeltier( void )
{
	// Se necesitan 150uSeg para que la corriente de la celda se estabilice
	DemoraPorInterrupcion( MILISEGUNDOS_1 );
}



/************************************************************/
/* DemoraEnMiliSegundos										*/
/*  														*/
/*  Realiza la demora de los mSeg solicitados				*/
/*  														*/
/*  Recibe: Los mSeg a demorar								*/
/*  Devuelve: Nada											*/
/************************************************************/
void DemoraEnMiliSegundos( char miliSegundos )
{
	for( ; miliSegundos > 0; miliSegundos-- )
		DemoraPorInterrupcion( MILISEGUNDOS_1 );
}

