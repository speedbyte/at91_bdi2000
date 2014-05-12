#ifndef Conversion_C
#define Conversion_C

#include "conversion.h"

extern unsigned char		ASCII_Tick_Buffer[];

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
		ASCII_Tick_Buffer[i]=(unsigned char)num+48;
		numberoccoured=1;
		}
		else
		{
		ASCII_Tick_Buffer[i]=blanksym;
		}
		ValToWork%=Devider;
		Devider/=10;
	}
}

#endif