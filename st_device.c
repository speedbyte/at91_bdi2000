#ifndef st_device_c
#define st_device_c


// Include Standard LIB  files 
#include "mci_type.h"
#include "AT91RM9200.h"
#include "lib_AT91RM9200.h"
#include "isr.h" 			// AT91F_ST_ASM_HANDLER()
#include "init.h" 				// AT91F_DBGU_Printk
#include "st_device.h"       // selber inc
//////////////////////////////////////////////////////////////////////////////
// Externals
//////////////////////////////////////////////////////////////////////////////

//* Interrupt Handlers
//extern void AT91F_ST_ASM_HANDLER(void);


//////////////////////////////////////////////////////////////////////////////
// Global variables
//////////////////////////////////////////////////////////////////////////////

//* system timer counter
unsigned int StTick = 0;


//*----------------------------------------------------------------------------
//* \fn    AT91F_GetTickCount
//* \brief This function returns the value of the system timer
//*----------------------------------------------------------------------------
unsigned int AT91F_GetTickCount(void)
{
	return(StTick);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_ST_HANDLER
//* \brief This function is invoked by main
//*----------------------------------------------------------------------------
void AT91F_ST_HANDLER(void)
{
volatile int StStatus;
	// Read the system timer status register 	
	StStatus = *(AT91C_ST_SR);
	StTick++;
	StStatus = ((AT91C_BASE_PIOB->PIO_OSR & 7 )> 0) ? (AT91C_BASE_PIOB->PIO_ODR = AT91C_BASE_PIOB->PIO_ODR | 7) : (AT91C_BASE_PIOB->PIO_OER  = AT91C_BASE_PIOB->PIO_OER | 7);
	
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_ST_SetPeriodIntervalTimer
//* \brief Set Periodic Interval Interrupt (period min <=> 1/32768 s)
//*----------------------------------------------------------------------------
void AT91F_ST_SetPeriodIntervalTimer(
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
							AT91F_ST_ASM_HANDLER );						
	//* Enable ST interrupt
	AT91F_AIC_EnableIt(AT91C_BASE_AIC, AT91C_ID_SYS);


	//* System Timer initialization
	AT91F_ST_SetPeriodIntervalTimer(AT91C_BASE_ST, 0xFFF);	// FFFF gives around 2 seconds
	AT91F_ST_EnableIt(AT91C_BASE_ST, AT91C_ST_PITS);
}
#endif
