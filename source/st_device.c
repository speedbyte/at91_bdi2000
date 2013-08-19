#ifndef st_device_c
#define st_device_c


// Include Standard LIB  files 
#include "mci_type.h"
#include "AT91RM9200.h"
#include "lib_AT91RM9200.h"
#include "isr.h" 			// Interrupt_Handler_SysTimer_Lowlevel()
#include "init.h" 				// Send_Stream_from_RS232_to_Terminal
#include "st_device.h"       // selber inc
#include "led_device.h"

//////////////////////////////////////////////////////////////////////////////
// Externals
//////////////////////////////////////////////////////////////////////////////

//* Interrupt Handlers
//extern void Interrupt_Handler_SysTimer_Lowlevel(void);


//////////////////////////////////////////////////////////////////////////////
// Global variables
//////////////////////////////////////////////////////////////////////////////

//* system timer counter
unsigned int StTick = 0;


//*----------------------------------------------------------------------------
//* \fn    GetTickCount_fromST
//* \brief This function returns the value of the system timer
//*----------------------------------------------------------------------------
unsigned int GetTickCount_fromST(void)
{
	return(StTick);
}

//*----------------------------------------------------------------------------
//* \fn    Interrupt_Handler_SysTimer_Highlevel
//* \brief This function is invoked by main
//*----------------------------------------------------------------------------
void Interrupt_Handler_SysTimer_Highlevel(void)
{
volatile int StStatus;
	// Read the system timer status register 	
	StStatus = *(AT91C_ST_SR);
	StTick++;
	if (StTick > 999)  StTick = 0; 
	(getLed(YELLOW)) ? (resetLed(YELLOW)): (setLed(YELLOW));
}

//*----------------------------------------------------------------------------
//* \fn    SetPeriodicTimerCounter
//* \brief Set Periodic Interval Interrupt (period min <=> 1/32768 s)
//*----------------------------------------------------------------------------
void SetPeriodicTimerCounter(
	AT91PS_ST pSt,
	unsigned int period)
{
	volatile int status;
	pSt->ST_IDR = AT91C_ST_PITS;	/* Interrupt disable Register */

	status = pSt->ST_SR;
    pSt->ST_PIMR = period;			/* Period Interval Mode Register */
}


//*----------------------------------------------------------------------------
//* \fn    main
//* \brief 
//*----------------------------------------------------------------------------
void St_init()
{
	
	AT91F_AIC_ConfigureIt (	AT91C_BASE_AIC,                        // AIC base address
							AT91C_ID_SYS,                          // System peripheral ID
							AT91C_AIC_PRIOR_HIGHEST,               // Max priority
							AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE, // Level sensitive
							Interrupt_Handler_SysTimer_Lowlevel );						
	//* Enable ST interrupt
	AT91F_AIC_EnableIt(AT91C_BASE_AIC, AT91C_ID_SYS);


	//* System Timer initialization
	SetPeriodicTimerCounter(AT91C_BASE_ST, 32);	// FFFF gives around 2 seconds
	AT91F_ST_EnableIt(AT91C_BASE_ST, AT91C_ST_PITS);
}
#endif
