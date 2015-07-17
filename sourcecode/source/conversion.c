#ifndef Conversion_C
#define Conversion_C

#include "conversion.h"
#include "main.h"
#include "mci_type.h"
#include "AT91RM9200.h"
#include "lib_AT91RM9200.h"

#define MEASVALOFF	20 // Measurement value offset in in string

extern unsigned char		ASCII_Tick_Buffer[];
extern unsigned char	    ASCII_UART_Buffer[];
extern CALENDAR 				Rtc_Date_Start;
extern TIME     				Rtc_Time_Start;




void WaitTicks(unsigned int ticks)
{
		for(unsigned int i=0;i<ticks;i++)
		i=i; //asm("nop");
}


void Dec2ASCII_Ticks(unsigned int value,unsigned char blanksym)
{	unsigned char numberoccurred=0;
	unsigned int num;
	unsigned int ValToWork=value;
	unsigned int i;
	unsigned int Divider=1000000000;
	
	for(i=0;i<10;i++)
	{	
		num=ValToWork/Divider;
		if(num|numberoccurred)
		{
		ASCII_UART_Buffer[i+MEASVALOFF]=(unsigned char)num+48;
		numberoccurred=1;
		}
		else
		{
		ASCII_UART_Buffer[i+MEASVALOFF]=blanksym;
		}
		ValToWork%=Divider;
		Divider/=10;
	}
}

void PutDateAndTimeStamp(unsigned char datedivider,unsigned char timedivider)
{
unsigned char idx=0;
const unsigned char Dec2ASCII[]="0123456789";



//Day
ASCII_UART_Buffer[idx]=Dec2ASCII[((Rtc_Date_Start.cal_bits.date& 0xF0)>>4)];
idx++;
ASCII_UART_Buffer[idx]=Dec2ASCII[((Rtc_Date_Start.cal_bits.date& 0x0F)>>0)];
idx++;
ASCII_UART_Buffer[idx]=datedivider;
idx++;
//Month
ASCII_UART_Buffer[idx]=Dec2ASCII[((Rtc_Date_Start.cal_bits.month& 0xF0)>>4)];
idx++;
ASCII_UART_Buffer[idx]=Dec2ASCII[((Rtc_Date_Start.cal_bits.month& 0x0F)>>0)];
idx++;
ASCII_UART_Buffer[idx]=datedivider;
idx++;
//Year
ASCII_UART_Buffer[idx]=Dec2ASCII[((Rtc_Date_Start.cal_bits.century& 0xF0)>>4)];
idx++;
ASCII_UART_Buffer[idx]=Dec2ASCII[((Rtc_Date_Start.cal_bits.century& 0x0F)>>0)];
idx++;
ASCII_UART_Buffer[idx]=Dec2ASCII[((Rtc_Date_Start.cal_bits.year& 0xF0)>>4)];
idx++;
ASCII_UART_Buffer[idx]=Dec2ASCII[((Rtc_Date_Start.cal_bits.year& 0x0F)>>0)];
idx++;
ASCII_UART_Buffer[idx]=' ';
idx++;
//Hour
ASCII_UART_Buffer[idx]=Dec2ASCII[((Rtc_Time_Start.time_bits.hour& 0xF0)>>4)];
idx++;
ASCII_UART_Buffer[idx]=Dec2ASCII[((Rtc_Time_Start.time_bits.hour& 0x0F)>>0)];
idx++;
ASCII_UART_Buffer[idx]=timedivider;
idx++;
//Minute
ASCII_UART_Buffer[idx]=Dec2ASCII[((Rtc_Time_Start.time_bits.minute& 0xF0)>>4)];
idx++;
ASCII_UART_Buffer[idx]=Dec2ASCII[((Rtc_Time_Start.time_bits.minute& 0x0F)>>0)];
idx++;
ASCII_UART_Buffer[idx]=timedivider;
idx++;
//Second
ASCII_UART_Buffer[idx]=Dec2ASCII[((Rtc_Time_Start.time_bits.second& 0xF0)>>4)];
idx++;
ASCII_UART_Buffer[idx]=Dec2ASCII[((Rtc_Time_Start.time_bits.second& 0x0F)>>0)];

}



unsigned int Convert_Ticks_To_us (unsigned int Ticks,unsigned char TimerClockBase)
{ 
	 switch (TimerClockBase)
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
			return (unsigned int)(((unsigned long long)(Ticks*15625))>>9);  //Tested [v] //SLCK (32768HZ)
			// Value is not valid for times >= approx 5s
	 default:
			return Ticks;
	 }

}
#endif
