/*
To do list :
make a new module named LED
Time stamp before each carriage return.
Reset Button - resets the timer.
Pause button - pauses but the timer is running.
only Green LED show the device is ON and running
only Yellow LED - it is paused.
Green and Yellow one time blink indicates the timer is reset.
only Red - there is an error and cannot continue.
REd and green there was an error but it is still running and saving the data.
Wann gibts kein SpeichernPlatz mehr, blink malRot.
*/

#ifndef main_c
#define main_c

/* 
REv 1.0 : Initial build with MCI Drivers.
REv 1.1 : Improvemnent directory structure, Created header file
REv 1.2 : Integrated USART function calls
*/

#include <stdio.h>
#include <string.h>
#include "mci_type.h"
#include "AT91RM9200.h"
#include "lib_AT91RM9200.h"
#include "main.h"
#include "mci_device.h"    // AT91F_MCI_SDCard_Init() , Mci_init() , AT91F_MCI_DeviceWaitReady(), AT91F_MCI_ReadBlock(), AT91F_MCI_WriteBlock()
#include "init.h"          // AT91F_DBGU_Printk()
#include "usart_device.h"  // AT91F_US_Print_frame(), Usart_init(), AT91F_US_Printk(), AT91F_US_Print_2_frame()
#include "st_device.h"
#include "led_device.h"
#ifndef main_c

//* Functions
void Led_glow();
extern char AT91F_DBGU_getc(void);
int AT91F_InitDeviceStructure(void);
extern void AT91F_MCI_Handler(void);  // to isr.S
extern int main(void);     // to cstartup.S
#endif

CALENDAR rtc_cal;
TIME     rtc_time;
int reader, writer;
//* Global Variables
AT91PS_USART USART_pt = AT91C_BASE_US1;
AT91PS_MCI MCI_pt;
AT91PS_MCI MCI_pt = AT91C_BASE_MCI;
static int MciBeginBlock = 100;
AT91S_MciDeviceFeatures			MCI_Device_Features;
AT91S_MciDeviceDesc				MCI_Device_Desc;
AT91S_MciDevice					MCI_Device;
char							usartBuffer1[BUFFER_SIZE_MCI_DEVICE];
char							usartBuffer2[BUFFER_SIZE_MCI_DEVICE];
int readytowriteonSD;
char *Bufferwechsler;
char *printBuffer;
int errorstatus;
int globalj;
int globali;
char mciBuffer1[512];
char mciBuffer2[512];
// Local Functions


void Rtc_init(void)
{
	
	AT91C_BASE_RTC->RTC_CR = (AT91C_RTC_UPDTIM | AT91C_RTC_UPDCAL);         // step RTC
	while (!(AT91C_BASE_RTC->RTC_SR & AT91C_RTC_ACKUPD) ); // wait for stop acknowledge
	AT91C_BASE_RTC->RTC_MR = 0;         // 24 hour mode
	rtc_time.time_bits.second = 0x0; 
	rtc_time.time_bits.minute = 0x0; 
	rtc_time.time_bits.hour =   0x6; 
	rtc_time.time_bits.merid =  0x0;
	AT91C_BASE_RTC->RTC_TIMR = (uint32)rtc_time.time_data;	
	//while (!(AT91C_BASE_RTC->RTC_SR & AT91C_RTC_ACKUPD) ); // wait for stop acknowledge
	rtc_cal.cal_bits.century = 0x20; 
	rtc_cal.cal_bits.year =    0x07; 
	rtc_cal.cal_bits.month =   0x1; 
	rtc_cal.cal_bits.day =     0x1; 
	rtc_cal.cal_bits.date =    0x1;
	AT91C_BASE_RTC->RTC_CALR = (uint32)rtc_cal.cal_data;
	AT91C_BASE_RTC->RTC_CR =    0; // start timer.		
}
	
inline void __ascii1(unsigned char value)
{
	Bufferwechsler[globalj++] = ((value & 0xF0) >> 4 ) + 48 ; 
	AT91C_BASE_PDC_MCI->PDC_RCR--;	
}

inline void __ascii2(unsigned char value)
{
	Bufferwechsler[globalj++] = ((value & 0xF0) >> 4 ) + 48 ; 
	AT91C_BASE_PDC_MCI->PDC_RCR--;	
	Bufferwechsler[globalj++] = (value & 0xF ) + 48 ; 
	AT91C_BASE_PDC_MCI->PDC_RCR--;							
}

inline void __format(unsigned char value)
{
	Bufferwechsler[globalj++] =  value;
	AT91C_BASE_PDC_MCI->PDC_RCR--;	
}
void PutTimeStamp()
{
	rtc_time.time_data = (uint32)AT91C_BASE_RTC->RTC_TIMR;
	__ascii2(rtc_time.time_bits.hour);
	__format(':');
	__ascii2(rtc_time.time_bits.minute);
	__format(':');
	__ascii2(rtc_time.time_bits.second);
	__format( '.');
	__ascii2(AT91F_GetTickCount() );
	__ascii1(rtc_time.time_bits.merid);
}   

void PutCalStamp()
{
	rtc_cal.cal_data = (uint32)AT91C_BASE_RTC->RTC_CALR;
	__ascii1( rtc_cal.cal_bits.day);
	__format( ',');
	__ascii2( rtc_cal.cal_bits.date);
	__format( ',');
	__ascii2( rtc_cal.cal_bits.month);
	__format( ',');
	__ascii2( rtc_cal.cal_bits.century);
	__ascii2( rtc_cal.cal_bits.year);
}   

void PutCalTimeHeader(void)
{
	int i;
	PutCalStamp();
	for(i=0;i<10;i++) Bufferwechsler[globalj++] = '-';
	PutTimeStamp();
	for(i=0;i<10;i++) Bufferwechsler[globalj++] = '-';
}
//*----------------------------------------------------------------------------
//* \fn    AT91F_DBGU_getc
//* \brief This function is used to receive a character to the DBGU channel
//*----------------------------------------------------------------------------
char AT91F_DBGU_getc(void)
{
	
	AT91F_DBGU_Printk("\n\rbefore while");
	while (!AT91F_US_RxReady((AT91PS_USART)AT91C_BASE_DBGU));  // RXRDY goes high
	AT91F_DBGU_Printk("\n\rafter while");
	return AT91F_US_GetChar((AT91PS_USART) AT91C_BASE_DBGU);  // RXRDY goes low ( by reading the status->RHR reg )
}


//*----------------------------------------------------------------------------
//* \fn    AT91F_CfgDevice
//* \brief This function is used to initialise MMC or SDCard Features
//*----------------------------------------------------------------------------
int AT91F_InitDeviceStructure(void)
{
	int x;
	MCI_Device_Features.Relative_Card_Address 		= 0;
	MCI_Device_Features.Card_Inserted 				= AT91C_CARD_REMOVED;
	MCI_Device_Features.Max_Read_DataBlock_Length	= 0;
	MCI_Device_Features.Max_Write_DataBlock_Length 	= 0;
	MCI_Device_Features.Read_Partial 				= 0;
	MCI_Device_Features.Write_Partial 				= 0;
	MCI_Device_Features.Erase_Block_Enable 			= 0;
	MCI_Device_Features.Sector_Size 				= 0;
	MCI_Device_Features.Memory_Capacity 			= 0;
	
	MCI_Device_Desc.state							= AT91C_MCI_IDLE;
	MCI_Device_Desc.SDCard_bus_width				= AT91C_MCI_SCDBUS;
	
	// Init AT91S_DataFlash Global Structure, by default AT45DB choosen !!!
	MCI_Device.pMCI_DeviceDesc 		= &MCI_Device_Desc;
	MCI_Device.pMCI_DeviceFeatures 	= &MCI_Device_Features;
	x = AT91F_MCI_SDCard_Init(&MCI_Device);
	AT91C_BASE_MCI->MCI_MR |= ((MCI_Device.pMCI_DeviceFeatures->Max_Write_DataBlock_Length << 16) | AT91C_MCI_PDCMODE);	
	
	return(x);
}
	
//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_Handler
//* \brief MCI Handler
//*----------------------------------------------------------------------------
void AT91F_MCI_Handler(void)
{
	int status;
	status = ( AT91C_BASE_MCI->MCI_SR & AT91C_BASE_MCI->MCI_IMR );
	AT91F_MCI_Device_Handler(&MCI_Device,status);
}


//*----------------------------------------------------------------------------
//* \fn    main
//* \brief main function
//*----------------------------------------------------------------------------
int main()
{
	unsigned char	character;
	int j;
	uint32 Max_Read_DataBlock_Length;
	Mci_init();
	Usart_init();
	St_init(); 
	Led_init();
	Rtc_init();
	globalj=0;
	if(AT91F_InitDeviceStructure() != AT91C_INIT_OK) {
		AT91F_DBGU_Printk("\n\rSDcARD Initialisation failed\n\r");
		return FALSE;}

	for (j=0;j<512;j++) usartBuffer1[j] = 'A';
	for (j=0;j<512;j++) usartBuffer2[j] = 'B';
	for (j=0;j<512;j++) mciBuffer1[j] = ' ';
	for (j=0;j<512;j++) mciBuffer2[j] = ' ';
	Max_Read_DataBlock_Length = MCI_Device.pMCI_DeviceFeatures->Max_Read_DataBlock_Length;
	AT91F_DBGU_Printk("\n\r0) Write to SD 1 Read from SD\n\r");

	//USART:
   	AT91F_US_EnableIt(USART_pt,AT91C_US_TIMEOUT | AT91C_US_FRAME | AT91C_US_OVRE | AT91C_US_PARE);
	AT91F_US_EnableIt(USART_pt, AT91C_US_RXBUFF ); // interrupt only after 1024 bytes. no ENDRX
	AT91F_US_EnableTx(USART_pt);  // We need transmitter for making the device act as a reader. 
	AT91F_US_EnableRx(USART_pt);  // we need the receiver 
	USART_pt->US_PTCR = AT91C_PDC_RXTDIS;
	USART_pt->US_PTCR = AT91C_PDC_TXTDIS;      
	
	// MCI
	MCI_pt->MCI_PTCR = AT91C_PDC_RXTEN;
	MCI_pt->MCI_PTCR = AT91C_PDC_TXTEN;	
	AT91C_BASE_MCI->MCI_IDR = AT91C_MCI_RXBUFF;  // We need interrupts every 512 bytes not 1024 bytes.
	AT91C_BASE_MCI->MCI_IDR = AT91C_MCI_TXBUFE; 
	readytowriteonSD = NOT_ACTIVE;
	reader = NOT_ACTIVE; writer = NOT_ACTIVE;
	resetLed(GREEN | RED | YELLOW );
	while(1)
	{
		character = AT91F_DBGU_getc();      //also done by PDC.. this is the place where the user will also start. like a ON button
		switch(character)
		{
			case '0':
				Bufferwechsler = mciBuffer1;
				writer = ACTIVE; reader = NOT_ACTIVE;
				AT91F_US_PutFrame(USART_pt,(char *)(usartBuffer1),(512),(char *)(usartBuffer2),(512)); 
				AT91C_BASE_PDC_MCI->PDC_RCR = 512;
				AT91C_BASE_MCI->MCI_IDR = AT91C_MCI_TXBUFE | AT91C_MCI_ENDTX; 
				AT91C_BASE_MCI->MCI_IER = AT91C_MCI_ENDRX; 
				globalj = 0; globali = 0;
				// Test Switching ON Time  .. this can be done using sprintf.
				// aftersuccess ..*Bufferwechsler = "Test Switch ON date and time\r\n";
				// aftersuccess ..globalj = strlen(Bufferwechsler);
				PutCalTimeHeader();
				AT91C_BASE_PDC_MCI->PDC_RCR = 0;   // 20 bytes		TEst switch on Time	
				do 
				{
					while (!AT91F_US_RxReady(USART_pt));					
					USART_pt->US_PTCR = AT91C_PDC_RXTEN;
					if ( readytowriteonSD == WRITE_NOW )
					{
						setLed(RED);
						errorstatus = AT91F_MCI_WriteBlock(&MCI_Device,(MciBeginBlock*Max_Read_DataBlock_Length), (uint32 *)printBuffer,Max_Read_DataBlock_Length);	
						if (errorstatus != AT91C_WRITE_OK)	AT91F_DBGU_Printk("\n\rWrite not OK\n\r");
						//* Wait end of Write
						//AT91F_MCI_DeviceWaitReady(AT91C_MCI_TIMEOUT); //the interrupt will tell me when it is finished.
						MciBeginBlock++;
						readytowriteonSD = NOT_ACTIVE;
					}
					character = *((char *)((USART_pt->US_RPR)-1));
					Bufferwechsler[globalj++] = character;
					AT91C_BASE_PDC_MCI->PDC_RCR--;		
					if(character == '\n')
					{
						PutTimeStamp();
					}
					USART_pt->US_PTCR = AT91C_PDC_RXTDIS;
				}
				while(1);
				break;

			case '1': //* Print in pooling
				reader = 1;
				AT91C_BASE_MCI->MCI_IDR = (AT91C_MCI_RXBUFF | AT91C_MCI_TXBUFE );				
				for (MciBeginBlock=100;MciBeginBlock<103;MciBeginBlock++) {
				if ((AT91F_MCI_ReadBlock(&MCI_Device,(MciBeginBlock*Max_Read_DataBlock_Length), (unsigned int *)mciBuffer1,Max_Read_DataBlock_Length)) != AT91C_READ_OK)		
					AT91F_DBGU_Printk("\n\rRead not OK\n\r");
				//* Wait end of Read
				// glow Yellow LED
				AT91F_MCI_DeviceWaitReady(AT91C_MCI_TIMEOUT);
				
				AT91F_US_Print_2_frame(USART_pt,(char *)mciBuffer1,512,0,0);
				while(!((USART_pt->US_CSR) & AT91C_US_TXRDY));  // wait till the US_THR is clear
				USART_pt->US_PTCR = AT91C_PDC_TXTEN;        // start sending data to USART
				// Glow Green LED ( sending to USART )
				while(!((USART_pt->US_CSR) & AT91C_US_ENDTX));
				USART_pt->US_PTCR = AT91C_PDC_TXTDIS;	
				AT91F_DBGU_Printk("\n\rnext\n\r");
				}
//				}
				break;
					
			default:
				AT91F_DBGU_Printk("\n\rBad choice, Retry please\n\r");
				break;		
		}
	}
return 0;
}
#endif


