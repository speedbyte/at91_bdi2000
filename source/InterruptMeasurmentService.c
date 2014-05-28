#ifndef IMS_C
#define IMS_C

#include "InterruptMeasurmentService.h"
#include "AT91RM9200.h"
#include "lib_AT91RM9200.h"
#include "Timer.h"
#include "led_device.h"
#include "conversion.h"
#include "mci_type.h"
#include "main.h"
#include "isr.h"


unsigned char		ASCII_Tick_Buffer[]="4294967296 us\n";
unsigned char		ASCII_UART_Buffer[]="DD/MM/YYYY HH:MM:SS 4294967296 us\n";
CALENDAR 				Rtc_Date_Start;
TIME     				Rtc_Time_Start;
unsigned char TimerOvlComp;
unsigned int 		InterruptTimeUsed=0;
unsigned char 		TIMER_CLK_USED;
extern unsigned int 		TimerOverflowCnt;


/* Used Timer: "AT91C_BASE_TC0"
#define TIMER_CLOCK1 		0x00     	//MCK/2
#define TIMER_CLOCK2 		0x01		//MCK/8
#define TIMER_CLOCK3 		0x02		//MCK/32
#define TIMER_CLOCK4 		0x03		//MCK/128
#define TIMER_CLOCK5 		0x04		//SLCK (32768HZ)
*/

void Init_Latency_Measurement(unsigned char TimerClockBase,unsigned char TimerInterruptCompensation)
{
	TIMER_CLK_USED=TimerClockBase;
	initTimer(TIMER_CLK_USED);
	TimerOvlComp=TimerInterruptCompensation;
	AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_TC0, TIMER0_INTERRUPT_LEVEL,AT91C_AIC_SRCTYPE_INT_POSITIVE_EDGE, Interrupt_Handler_TC0_Lowlevel);
	AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC0);

}

void Start_Latency_Measurement(void)
{
TimerOverflowCnt=0;
StartTimer();
Rtc_Time_Start.time_data = (uint32)AT91C_BASE_RTC->RTC_TIMR;
Rtc_Date_Start.cal_data = (uint32)AT91C_BASE_RTC->RTC_CALR;
}

void Stop_Latency_Measurement(void)
{	
	StopTimer();
	InterruptTimeUsed=Convert_Ticks_To_us(getTimerValue(),TIMER_CLK_USED);
	if(TimerOvlComp)
	{
	InterruptTimeUsed-=Time_Used_In_Timer_Int();
	}
		
	
	toggleLed(YELLOW);
	
	PutDateAndTimeStamp('/',':');
	Dec2ASCII_Ticks(InterruptTimeUsed,'0');
	AT91F_US_SendFrame((AT91PS_USART)AT91C_BASE_US1, (char *)&ASCII_UART_Buffer,(sizeof(ASCII_UART_Buffer)-1),0,0); //Including \0 at the end (sizeof(Buffer)-1) will not send the string delimiter

}

#endif
