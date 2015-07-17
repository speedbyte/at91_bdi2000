#ifndef led_device_c
#define led_device_c


#include "mci_type.h"
#include "AT91RM9200.h"
#include "lib_AT91RM9200.h"
#include "init.h"          // Send_Stream_from_RS232_to_Terminal()
#include "led_device.h"


void Led_init()
{
	AT91C_BASE_PIOB->PIO_PER = AT91C_PIO_PB0|AT91C_PIO_PB1|AT91C_PIO_PB2; //Enable Register! 
	AT91C_BASE_PIOB->PIO_OER = AT91C_PIO_PB0|AT91C_PIO_PB1|AT91C_PIO_PB2; //Enable output! 
	AT91C_BASE_PIOB->PIO_CODR = AT91C_PIO_PB0|AT91C_PIO_PB1|AT91C_PIO_PB2; //Assign PB.0-2 to GND
}


void  toggleLed(unsigned char flag) 
{		
 unsigned char LedState=getLed(GREEN | RED | YELLOW);
 
		LedState^=flag;
		
	   AT91C_BASE_PIOB->PIO_CODR =(LedState)&0x07;
	   AT91C_BASE_PIOB->PIO_SODR =(~LedState)&0x07;
}


#endif