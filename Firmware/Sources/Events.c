/* ###################################################################
**     Filename    : Events.c
**     Project     : ProbadorManillares
**     Processor   : MC9S08AC16CFD
**     Component   : Events
**     Version     : Driver 01.02
**     Compiler    : CodeWarrior HCS08 C Compiler
**     Date/Time   : 2018-11-26, 12:56, # CodeGen: 0
**     Abstract    :
**         This is user's event module.
**         Put your event handler code here.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file Events.c
** @version 01.02
** @brief
**         This is user's event module.
**         Put your event handler code here.
*/         
/*!
**  @addtogroup Events_module Events module documentation
**  @{
*/         
/* MODULE Events */


#include "Cpu.h"
#include "Events.h"

/* User includes (#include below this line is not maintained by Processor Expert) */

extern bool flagTerminoElTimer;
extern bool	semaforoADC;

ISR(Interrupcion_TIM1_overflow)
{
	// Se borra el flag de la interrupcion
	clrReg8Bits(TPM1SC, 0x80U);
	
	// Se indica que ingreso a la interrupcion
	flagTerminoElTimer = TRUE;
}


// Interrupcion del ADC
ISR(Interrupcion_ADC)					{	borrarFlagADC();	semaforoADC = TRUE;	}


/* END Events */

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
