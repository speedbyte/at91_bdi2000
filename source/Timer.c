// Timer file for generating periodic and Pulse Tasks / Interrups

#ifndef Timer_C
#define Timer_C


#include "mci_type.h"
#include "AT91RM9200.h"
#include "lib_AT91RM9200.h"
#include "Timer.h"

/*

unsigned int getTimerValue(void);
void resetTimerValue(void);
*/

void initTimer(void)
{
// Used Timer: "AT91C_BASE_TC0"
#define TIMER_CLOCK1 		0x00     	//MCK/2
#define TIMER_CLOCK2 		0x01		//MCK/8
#define TIMER_CLOCK3 		0x02		//MCK/32
#define TIMER_CLOCK4 		0x03		//MCK/129
#define TIMER_CLOCK5 		0x04		//SLCK (32768HZ)
#define CPCTRG				(1<<14)		//RC Compare Trigger Enable
#define TIMER_START_VAL		0x0000		//RC Compare Trigger Enable


AT91C_BASE_PMC->PMC_PCER=1 << AT91C_ID_TC0;  /* enable clock */

AT91C_BASE_TCB0->TCB_BCR = 0;	// TC Block Control Register, No SYNC Mode

AT91C_BASE_TCB0->TCB_BMR = AT91C_TCB_TC0XC0S_NONE | AT91C_TCB_TC1XC1S_NONE | AT91C_TCB_TC2XC2S_NONE; //No External Clock Selection

AT91C_BASE_TC0->TC_CCR=AT91C_TC_CLKDIS; //Disable Timer Clock

AT91C_BASE_TC0->TC_CMR = TIMER_CLOCK5 | CPCTRG;  //Set Timer To SLCK And ENABLE CPCTRG

AT91C_BASE_TC0->TC_IDR = ~0ul;  //Disable all Timer Interrupts

AT91C_BASE_TC0->TC_IER = AT91C_TC_COVFS;  //Enable TC0 Counter Overflow

//AT91C_BASE_TC0->TC_RC = TIMER_START_VAL;

AT91C_BASE_TC0->TC_CCR = AT91C_TC_SWTRG | AT91C_TC_CLKEN; // Timer Clock enable and start the Timer

}


unsigned int getTimerValue(void)
{
return (AT91C_BASE_TC0->TC_CV&0x0000FFFF); 
}

void resetTimerValue(void)
{
//AT91C_BASE_TC0->TC_RC=val;
AT91C_BASE_TC0->TC_CCR = AT91C_TC_SWTRG | AT91C_TC_CLKEN; // Timer Clock enable,reset and (re)start the Timer
}






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
