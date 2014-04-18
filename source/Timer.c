// Timer file for generating periodic and Pulse Tasks / Interrups

#ifndef Timer_C
#define Timer_C


#include "mci_type.h"
#include "AT91RM9200.h"
#include "lib_AT91RM9200.h"
#include "Timer.h"

/*
void initTimer(void);
void startTimerOnetime(int duration_us);
void startTimerPermanent(int duration_us);
void stopTimerActions(void);
*/

void init_I_O(void)
{
	//AT91C_BASE_PIOB->PIO_PER = AT91C_PIO_PB15; //Enable Register! 
	
	//AT91C_BASE_PIOB->PIO_ODR = AT91C_PIO_PB15; //Disable output! 
	//AT91C_BASE_PIOB->PIO_IDR = AT91C_PIO_PB15; //Interrupt disabled!
	AT91C_BASE_PMC->PMC_PCER=0xFFFFFFFF; // Enable peripheral clock, otherwise input is "frozen"

AT91C_BASE_PIOB->PIO_PER= AT91C_PIO_PB15;

AT91C_BASE_PIOB->PIO_ODR= AT91C_PIO_PB15;

AT91C_BASE_PIOB->PIO_IFDR= AT91C_PIO_PB15;

AT91C_BASE_PIOB->PIO_CODR= AT91C_PIO_PB15;

AT91C_BASE_PIOB->PIO_IDR= AT91C_PIO_PB15;

AT91C_BASE_PIOB->PIO_MDDR= AT91C_PIO_PB15;

AT91C_BASE_PIOB->PIO_PPUDR= AT91C_PIO_PB15;

AT91C_BASE_PIOB->PIO_OWDR= AT91C_PIO_PB15;





}

char getDigInputState(AT91PS_PIO Port_pt, unsigned int Pin)
{
	if(Port_pt->PIO_PDSR&(Pin))
	{
	return 1;
	}
	else
	{
	return 0;
	}
}


#endif