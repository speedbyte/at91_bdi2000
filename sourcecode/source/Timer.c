// Timer file for generating periodic and Pulse Tasks / Interrups

#ifndef Timer_C
#define Timer_C


#include "mci_type.h"
#include "AT91RM9200.h"
#include "lib_AT91RM9200.h"
#include "Timer.h"
#include "led_device.h"
#include "conversion.h"
#include "mci_type.h"
#include "main.h"


#define CPCTRG				(1<<14)		//RC Compare Trigger Enable


unsigned int 		TimerOverflowCnt=0;



void initTimer(unsigned char TimerClockBase)
{

AT91C_BASE_PMC->PMC_PCER=1 << AT91C_ID_TC0;  /* enable clock */

AT91C_BASE_TCB0->TCB_BCR = 0;	// TC Block Control Register, No SYNC Mode

AT91C_BASE_TCB0->TCB_BMR = AT91C_TCB_TC0XC0S_NONE | AT91C_TCB_TC1XC1S_NONE | AT91C_TCB_TC2XC2S_NONE; //No External Clock Selection

AT91C_BASE_TC0->TC_CCR=AT91C_TC_CLKDIS; //Disable Timer Clock

AT91C_BASE_TC0->TC_CMR = TimerClockBase | CPCTRG;  //Set Timer To SLCK And ENABLE CPCTRG

AT91C_BASE_TC0->TC_IDR = 0xFFFFFFFF;  //Disable all Timer Interrupts

}




unsigned int getTimerValue(void)
{
		return ((TimerOverflowCnt<<16)+(AT91C_BASE_TC0->TC_CV&0x0000FFFF)); 
}

void StartTimer(void)
{
	AT91C_BASE_TC0->TC_IER = AT91C_TC_COVFS;  //Enable TC0 Counter Overflow
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_SWTRG | AT91C_TC_CLKEN; // Timer Clock enable,reset and (re)start the Timer
	return;
}

void StopTimer(void)
{
	AT91C_BASE_TC0->TC_IDR = AT91C_TC_COVFS;  //Disable TC0 Counter Overflow
	AT91C_BASE_TC0->TC_CCR=AT91C_TC_CLKDIS; //Disable Timer Clock
	return;
}

unsigned int Time_Used_In_Timer_Int(void)
{
	return (unsigned int) TimerOverflowCnt>>0; // MCK= 60MHz ,Approx 60 clockcycles per Timer overflow =1us per switch
}




void  Interrupt_Handler_TC0_Highlevel (void)
{
	// AT91PS_TC TC_pt = AT91C_BASE_TC0;
     unsigned int dummy;
    //* Acknowledge interrupt status
    dummy = AT91C_BASE_TC0->TC_SR;
    //* Suppress warning variable "dummy" was set but never used
    dummy = dummy;
	resetLed(RED);

	TimerOverflowCnt++;
}






#endif
