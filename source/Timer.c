// Timer file for generating periodic and Pulse Tasks / Interrups

#ifndef Timer_C
#define Timer_C


#include "mci_type.h"
#include "AT91RM9200.h"
#include "lib_AT91RM9200.h"
#include "Timer.h"
#include "led_device.h"
#include "conversion.h"



// Used Timer: "AT91C_BASE_TC0"
#define TIMER_CLOCK1 		0x00     	//MCK/2
#define TIMER_CLOCK2 		0x01		//MCK/8
#define TIMER_CLOCK3 		0x02		//MCK/32
#define TIMER_CLOCK4 		0x03		//MCK/128
#define TIMER_CLOCK5 		0x04		//SLCK (32768HZ)

#define CPCTRG				(1<<14)		//RC Compare Trigger Enable

#define TIMER_CLK_USED TIMER_CLOCK4

unsigned int 		TimerOverflowCnt=0;
unsigned int 		InterruptTimeUsed=0;
unsigned char		ASCII_Tick_Buffer[]="4294967296 us\n";

void initTimer(void)
{
AT91C_BASE_PMC->PMC_PCER=1 << AT91C_ID_TC0;  /* enable clock */

AT91C_BASE_TCB0->TCB_BCR = 0;	// TC Block Control Register, No SYNC Mode

AT91C_BASE_TCB0->TCB_BMR = AT91C_TCB_TC0XC0S_NONE | AT91C_TCB_TC1XC1S_NONE | AT91C_TCB_TC2XC2S_NONE; //No External Clock Selection

AT91C_BASE_TC0->TC_CCR=AT91C_TC_CLKDIS; //Disable Timer Clock

AT91C_BASE_TC0->TC_CMR = TIMER_CLK_USED | CPCTRG;  //Set Timer To SLCK And ENABLE CPCTRG

AT91C_BASE_TC0->TC_IDR = 0xFFFFFFFF;  //Disable all Timer Interrupts

}




unsigned int getTimerValue(void)
{
//((TimerOverflowCnt<<16)+getTimerValue())
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

void  Interrupt_Handler_TC0_Highlevel (void)
{
	AT91PS_TC TC_pt = AT91C_BASE_TC0;
    unsigned int dummy;
    //* Acknowledge interrupt status
    dummy = TC_pt->TC_SR;
    //* Suppress warning variable "dummy" was set but never used
    dummy = dummy;
	resetLed(RED);

	TimerOverflowCnt++;
}

void Start_Latency_Measurement(void)
{
TimerOverflowCnt=0;
StartTimer();
}

void Stop_Latency_Measurement(void)
{
	InterruptTimeUsed=Convert_Ticks_To_us(getTimerValue());
	StopTimer();
	toggleLed(YELLOW);
	Dec2ASCII_Ticks(InterruptTimeUsed,'0');
	
	AT91F_US_SendFrame((AT91PS_USART)AT91C_BASE_US1, &ASCII_Tick_Buffer,(sizeof(ASCII_Tick_Buffer)-1),0,0); //Including \0 at the end (sizeof(Buffer)-1) will not send the string delimiter
}

static inline unsigned int Convert_Ticks_To_us (unsigned int Ticks)
{ 
 switch (TIMER_CLK_USED)
 {
 case TIMER_CLOCK1:
		return Ticks/30; 		//Tested [v]
	
 case TIMER_CLOCK2:
		return (Ticks<<1)/15; 	//Tested [v]
 
 case TIMER_CLOCK3:
		return (Ticks<<3)/15;	//Tested [v]	
 
 case TIMER_CLOCK4:
		return (Ticks<<5)/15;	//Tested [v]
		
 case TIMER_CLOCK5:
		return (unsigned int)(((unsigned long long)(Ticks*15625))>>9);  //SLCK (32768HZ)

 default:
		return Ticks;
 }

}




#endif
