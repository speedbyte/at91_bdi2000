#ifndef Conversion_C
#define Conversion_C

#include "conversion.h"
#include "main.h"
#include "mci_type.h"
#include "AT91RM9200.h"
#include "lib_AT91RM9200.h"

#define MEASVALOFF	20

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
{	unsigned char numberoccoured=0;
	unsigned int num;
	unsigned int ValToWork=value;
	unsigned int i;
	unsigned int Devider=1000000000;
	
	for(i=0;i<10;i++)
	{	
		num=ValToWork/Devider;
		if(num|numberoccoured)
		{
		ASCII_UART_Buffer[i+MEASVALOFF]=(unsigned char)num+48;
		numberoccoured=1;
		}
		else
		{
		ASCII_UART_Buffer[i+MEASVALOFF]=blanksym;
		}
		ValToWork%=Devider;
		Devider/=10;
	}
}

void PutDateAndTimeStamp(unsigned char datedivider,unsigned char timedivider)
{
char idx=0;
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

#endif