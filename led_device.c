#ifndef led_device_c
#define led_device_c


#include "mci_type.h"
#include "AT91RM9200.h"
#include "lib_AT91RM9200.h"
#include "init.h"          // AT91F_DBGU_Printk()
#include "led_device.h"


void Led_init()
{
	AT91C_BASE_PIOB->PIO_PER = AT91C_PIO_PB0|AT91C_PIO_PB1|AT91C_PIO_PB2;
}



#endif
