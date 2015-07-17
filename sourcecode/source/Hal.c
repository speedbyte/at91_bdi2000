#ifndef Hal_c
#define Hal_c

#include "Hal.h"


//* Global Variables
CALENDAR 				rtc_cal;
TIME     				rtc_time;
AT91PS_USART 			USART_pt;

AT91PS_MCI 				MCI_pt;
int			 			SDBeginBlock;
SDCARD_INFO				mci_sdcard_info;
SDCARD_DESC				mci_sdcard_desc;
SDCARD					mci_sdcard;
char					usartBuffer1[1024];
int 					readytowriteonSD;
char 					*Bufferwechsler;
char					*printBuffer;
int 					errorstatus;
int 					globalj;
int 					globali;
char 					SDBuffer1[512];
char 					SDBuffer2[512];
int						RCR_recirculated; 
int 					reader, writer;

void Rtc_init(void)
{	
	while (!(AT91C_BASE_RTC->RTC_SR & AT91C_RTC_SECEV) );  // wait for SEC-Flag to allow change in 
	AT91C_BASE_RTC->RTC_CR = (AT91C_RTC_UPDTIM | AT91C_RTC_UPDCAL);         // step RTC
	while (!(AT91C_BASE_RTC->RTC_SR & AT91C_RTC_ACKUPD) );  // wait for stop acknowledge
	AT91C_BASE_RTC->RTC_MR 		= 0;         		// 24 hour mode
	rtc_time.time_bits.second 	= 0x00; 
	rtc_time.time_bits.minute 	= 0x35; 
	rtc_time.time_bits.hour 	= 0x11; 
	rtc_time.time_bits.merid 	= 0x00;
	
	rtc_cal.cal_bits.century 	= 0x20; 
	rtc_cal.cal_bits.year 		= 0x14; 
	rtc_cal.cal_bits.month 		= 0x06; 
	rtc_cal.cal_bits.day 		= 0x05; 
	rtc_cal.cal_bits.date 		= 0x27;
	
	AT91C_BASE_RTC->RTC_TIMR = (uint32)rtc_time.time_data;	
	AT91C_BASE_RTC->RTC_CALR = (uint32)rtc_cal.cal_data;
	
	AT91C_BASE_RTC->RTC_CR 		= 0; 			// start timer.		
	AT91C_BASE_RTC->RTC_SCCR 	=AT91C_RTC_SECEV; //Clear SEC-Flag
}

void InitHardware(void)
{
	USART_pt = AT91C_BASE_US1;
	Led_init();
	Usart_init();
	Rtc_init();	
}

void InitDemoInterrupt(AT91PS_PIO PIOptr, unsigned int ParallelID, unsigned int MyIOpin, unsigned int priority, unsigned int intType)
{
			AT91F_PMC_EnablePeriphClock (AT91C_BASE_PMC, ((unsigned int) 1 << ParallelID)); // first controller clock can PIOB
			AT91F_PIO_CfgInput (PIOptr, MyIOpin); // PB15 input configured as input
			AT91F_AIC_ConfigureIt(AT91C_BASE_AIC,ParallelID,priority,intType,Measured_Interrupt_Lowlevel);

			AT91F_PIO_InterruptEnable (PIOptr, MyIOpin); // enable change interrupt
			{volatile unsigned int dummy; dummy = PIOptr -> PIO_ISR;}
			AT91F_AIC_EnableIt (AT91C_BASE_AIC, ParallelID); // Enable PIOB controller interrupt

}

//*----------------------------------------------------------------------------
//* \fn    Interrupt_Handler_MCI_Highlevel
//* \brief MCI Handler
//*----------------------------------------------------------------------------
void Interrupt_Handler_MCI_Highlevel(void)
{
	int status;
	status = ( AT91C_BASE_MCI->MCI_SR & AT91C_BASE_MCI->MCI_IMR );
	Interrupt_Handler_SDcard_Highlevel(&mci_sdcard,status);
}

#endif
