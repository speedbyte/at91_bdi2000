#ifndef Hal_h
#define Hal_h

#include "AT91RM9200.h"
#include "lib_AT91RM9200.h"
#include "Timer.h"
#include "led_device.h"
#include "conversion.h"
#include "mci_type.h"
#include "main.h"
#include "mci_device.h"    // SDDevice_Init_and_Cfg_Registers() , Mci_init() , SDcard_Poll_AT91CMCINOTBUSY_flag(), SDcard_ReadBlock_CMD17_R1(), SDcard_WriteBlock_CMD24_R1()
#include "init.h"          // DBGU_Printk()
#include "usart_device.h"  // AT91F_US_Print_frame(), Usart_init(), AT91F_US_Printk(), AT91F_US_Print_2_frame()
#include "st_device.h"
#include "led_device.h"
#include "errm.h"
#include "isr.h"

void Rtc_init(void);
void InitHardware(void);
void InitDemoInterrupt(AT91PS_PIO PIOptr, unsigned int ParallelID, unsigned int MyIOpin, unsigned int priority, unsigned int intType);



// Handler to use original isr.S
void Interrupt_Handler_MCI_Highlevel(void);

#endif
