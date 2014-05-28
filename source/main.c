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
#include <unistd.h>
#include "mci_type.h"
#include "AT91RM9200.h"
#include "lib_AT91RM9200.h"
#include "main.h"
#include "mci_device.h"    // SDDevice_Init_and_Cfg_Registers() , Mci_init() , SDcard_Poll_AT91CMCINOTBUSY_flag(), SDcard_ReadBlock_CMD17_R1(), SDcard_WriteBlock_CMD24_R1()
#include "init.h"          // DBGU_Printk()
#include "usart_device.h"  // AT91F_US_Print_frame(), Usart_init(), AT91F_US_Printk(), AT91F_US_Print_2_frame()
#include "st_device.h"
#include "led_device.h"
#include "errm.h"
#include "Timer.h"
#include "isr.h"




//* Global Variables
CALENDAR 				rtc_cal;
TIME     				rtc_time;
int 					reader, writer;
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



// Local Functions
void Print_LineonSD(char *buffer)
{
	while(*buffer != '\0') 
		Bufferwechsler[globalj++] = *buffer++;
}


void Rtc_init(void)
{	
	while (!(AT91C_BASE_RTC->RTC_SR & AT91C_RTC_SECEV) );  // wait for SEC-Flag to allow change in 
	AT91C_BASE_RTC->RTC_CR = (AT91C_RTC_UPDTIM | AT91C_RTC_UPDCAL);         // step RTC
	while (!(AT91C_BASE_RTC->RTC_SR & AT91C_RTC_ACKUPD) );  // wait for stop acknowledge
	AT91C_BASE_RTC->RTC_MR 		= 0;         		// 24 hour mode
	rtc_time.time_bits.second 	= 0x00; 
	rtc_time.time_bits.minute 	= 0x36; 
	rtc_time.time_bits.hour 	= 0x09; 
	rtc_time.time_bits.merid 	= 0x00;
	
	rtc_cal.cal_bits.century 	= 0x20; 
	rtc_cal.cal_bits.year 		= 0x14; 
	rtc_cal.cal_bits.month 		= 0x05; 
	rtc_cal.cal_bits.day 		= 0x06; 
	rtc_cal.cal_bits.date 		= 0x23;
	
	AT91C_BASE_RTC->RTC_TIMR = (uint32)rtc_time.time_data;	
	AT91C_BASE_RTC->RTC_CALR = (uint32)rtc_cal.cal_data;
	
	AT91C_BASE_RTC->RTC_CR 		= 0; 			// start timer.		
	AT91C_BASE_RTC->RTC_SCCR 	=AT91C_RTC_SECEV; //Clear SEC-Flag
}


inline void __ascii1(unsigned char value)
{
	Bufferwechsler[globalj++] = ((value & 0xF0) >> 4 ) + 48 ; 
	AT91C_BASE_PDC_MCI->PDC_RCR--;	
}

inline void __ascii2(unsigned int value)
{
	Bufferwechsler[globalj++] = ((value & 0xF0) >> 4 ) + 48 ; 
	AT91C_BASE_PDC_MCI->PDC_RCR--;	
	Bufferwechsler[globalj++] = (value & 0xF ) + 48 ; 
	AT91C_BASE_PDC_MCI->PDC_RCR--;							
}

inline void __dec_to_ascii2(unsigned int value)
{
	Bufferwechsler[globalj++] = (value/100) + 48 ; 
	AT91C_BASE_PDC_MCI->PDC_RCR--;	
	Bufferwechsler[globalj++] = ((value/10)%10) + 48 ; 
	AT91C_BASE_PDC_MCI->PDC_RCR--;	
	Bufferwechsler[globalj++] = (value%10) + 48 ; 
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
	__dec_to_ascii2(GetTickCount_fromST());
	//Bufferwechsler[globalj++] = (char)(GetTickCount_fromST()) + 48 ; 
	//AT91C_BASE_PDC_MCI->PDC_RCR--;	
	Bufferwechsler[globalj++] = '-';
	AT91C_BASE_PDC_MCI->PDC_RCR--;
}   

void PutCalStamp()
{
	rtc_cal.cal_data = (uint32)AT91C_BASE_RTC->RTC_CALR;
	__ascii2( rtc_cal.cal_bits.date);
	__format( ',');
	__ascii2( rtc_cal.cal_bits.month);
	__format( ',');
	__ascii2( rtc_cal.cal_bits.century);
	__ascii2( rtc_cal.cal_bits.year);
}   

void PutCalTimeHeader(void)
{
	Print_LineonSD("|-------------------");
	PutCalStamp();
	Print_LineonSD("-------------------|");
	Print_LineonSD("|-------------------");
	PutTimeStamp();
	Print_LineonSD("-------------------|");	
	Bufferwechsler[globalj++] = '\r';
	Bufferwechsler[globalj++] = '\n';
	Bufferwechsler[510] = '\r';
	Bufferwechsler[511] = '\n';
}

//*----------------------------------------------------------------------------
//* \fn    DBGU_GetChar
//* \brief This function is used to receive a character to the "DEVICE"
//*----------------------------------------------------------------------------
char DBGU_GetChar(void)
{
	DBGU_Printk("\n\rbefore while");
	while (!AT91F_US_RxReady((AT91PS_USART)AT91C_BASE_DBGU));  // RXRDY goes high
	DBGU_Printk("\n\rafter while");
	return AT91F_US_GetChar((AT91PS_USART)AT91C_BASE_DBGU);  // RXRDY goes low ( by reading the status->RHR reg )
}

//*----------------------------------------------------------------------------
//* \fn    Get_Char_from_Terminal_to_RS232
//* \brief This function is used to receive a character to the "DEVICE"
//*----------------------------------------------------------------------------
char USART_GetChar(void)
{
	DBGU_Printk("\n\rbefore while");
	while (!AT91F_US_RxReady((AT91PS_USART)AT91C_BASE_US1));  // RXRDY goes high
	DBGU_Printk("\n\rafter while");
	return AT91F_US_GetChar((AT91PS_USART) AT91C_BASE_US1);  // RXRDY goes low ( by reading the status->RHR reg )
}


//*----------------------------------------------------------------------------
//* \fn    AT91F_CfgDevice
//* \brief This function is used to initialise MMC or SDCard Features
//*----------------------------------------------------------------------------
int Sdcard_init(void)
{
	int x;
	mci_sdcard_info.Relative_Card_Address 		= 0;
	mci_sdcard_info.Card_Inserted 				= STATE_SDCARD_REMOVED;
	mci_sdcard_info.Max_Read_DataBlock_Length	= 0;
	mci_sdcard_info.Max_Write_DataBlock_Length 	= 0;
	mci_sdcard_info.Read_Partial 				= 0;
	mci_sdcard_info.Write_Partial 				= 0;
	mci_sdcard_info.Erase_Block_Enable 			= 0;
	mci_sdcard_info.Sector_Size 				= 0;
	mci_sdcard_info.Memory_Capacity 			= 0;
	
	mci_sdcard_desc.state							= STATE_SDCARD_IDLE;
	mci_sdcard_desc.SDCard_bus_width				= AT91C_MCI_SCDBUS;
	
	// Init AT91S_DataFlash Global Structure, by default AT45DB choosen !!!
	mci_sdcard.ptr_sdcard_desc 			= &mci_sdcard_desc;
	mci_sdcard.ptr_sdcard_info 			= &mci_sdcard_info;
	x = SDDevice_Init_and_Cfg_Registers(&mci_sdcard);
	return(x);
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



/* Measured Interrupt Service Routine
/ Interrupt duration is measured automatically and transmitted via UART
/ Parameters: None
/ Returnvalue: None 			*/
void  Measured_Interrupt_Highlevel (void)
{		

	// ########################## DEMO ISR CONTENT ########################## 
	
		// Highlevel (rising edge) for PIO
		if (AT91F_PIO_GetInput (AT91C_BASE_PIOB) & MY_INT_PIN) //Highlevel rising edge
		{	
			// Demo interrupt latency
			setLed(GREEN);
			while(AT91F_PIO_GetInput (AT91C_BASE_PIOB) & MY_INT_PIN); // Wait if high!
			resetLed(GREEN);
		}
		else
		{/*AT91F_US_SendFrame((AT91PS_USART)AT91C_BASE_US1, "Int_L\n",6,0,0);*/ }
		
		// Read the PIO Interrupt Status Register to clear pending Interrupt (reenable rising edge triggering))
		{volatile unsigned int dummy = AT91C_BASE_PIOB -> PIO_ISR; dummy=dummy;}
		
	// ###################################################################### 
	
}



//*----------------------------------------------------------------------------
//* \fn    main
//* \brief main function
//*----------------------------------------------------------------------------
int main()
{
	/*USART_pt = AT91C_BASE_US1;
	MCI_pt = AT91C_BASE_MCI;
	unsigned char	character;
	int j, SDCurrentBlock,SDLastBlock, quitflag;
	uint32 Max_Read_DataBlock_Length;
	Mci_init();
	Usart_init();
	St_init(); 
	Led_init();
	Rtc_init();
	globalj=0;
	setLed(RED);
	while(Sdcard_init() != SD_INIT_NO_ERROR);
	resetLed(RED);
	SDBeginBlock = 100;
	SDLastBlock = SDBeginBlock;
	for (j=0;j<1024;j++) usartBuffer1[j] = 'A';
	for (j=0;j<512;j++) SDBuffer1[j] = ' ';
	for (j=0;j<512;j++) SDBuffer2[j] = ' ';
	Max_Read_DataBlock_Length = mci_sdcard.ptr_sdcard_info->Max_Read_DataBlock_Length;
	DBGU_Printk( "\n\r0) Write to SD 1 Read from SD\n\r");
	// MCI
	MCI_pt->MCI_PTCR = AT91C_PDC_RXTEN;
	MCI_pt->MCI_PTCR = AT91C_PDC_TXTEN;	
	AT91C_BASE_MCI->MCI_IDR = AT91C_MCI_RXBUFF;  // We need interrupts every 512 bytes not 1024 bytes.
	AT91C_BASE_MCI->MCI_IDR = AT91C_MCI_TXBUFE; 
	readytowriteonSD = NOT_ACTIVE;
	reader = NOT_ACTIVE; writer = NOT_ACTIVE;*/
	USART_pt = AT91C_BASE_US1;
	Led_init();
	Usart_init();
	initTimer();
	Rtc_init();

	
	
	unsigned char myUsartRxChar='F';
	
	//USART_Printk( "\n\n--- Hi this is AT91RM9200-EK booting up! ;) ---\n\n ");
	//int vsnprintf(char *buffer, size_t bufsize, const  char *format, va_list ap);
	
	char myBuffer[]="Hi this is AT91RM9200-EK booting up! ;) Int\n";
	unsigned int myIOtest;
	
	AT91F_US_SendFrame((AT91PS_USART)AT91C_BASE_US1, &myBuffer,(sizeof(myBuffer)-1),0,0); //Including \0 at the end (sizeof(Buffer)-1) will not send the string delimiter
	/*va_list ap;
	va_start(ap,fmt);
	vsnprintf(&myBuffer, sizeof(myBuffer),0,0);*/
	
	//resetLed(GREEN | RED | YELLOW );
	
	while(1)
	{
	resetLed(GREEN | RED | YELLOW );
				
			AT91F_PMC_EnablePeriphClock (AT91C_BASE_PMC, ((unsigned int) 1 << AT91C_ID_PIOB)); // first controller clock can PIOB
			AT91F_PIO_CfgInput (AT91C_BASE_PIOB, MY_INT_PIN); // PB0 input configured as input
			AT91F_AIC_ConfigureIt(AT91C_BASE_AIC,AT91C_ID_PIOB,AT91C_AIC_PRIOR_LOWEST,AT91C_AIC_SRCTYPE_EXT_POSITIVE_EDGE,Measured_Interrupt_Lowlevel);
			// AT91C_AIC_SRCTYPE_INT_EDGE_TRIGGERED. 
			// AT91C_AIC_SRCTYPE_EXT_HIGH_LEVEL
			// AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE
			//{volatile dummy; dummy = AT91C_BASE_PIOB -> PIO_ISR;}
			AT91F_PIO_InterruptEnable (AT91C_BASE_PIOB, MY_INT_PIN); // enable change interrupt
			{volatile unsigned int dummy; dummy = AT91C_BASE_PIOB -> PIO_ISR;}
			AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_PIOB); // Enable PIOB controller interrupt
			//Timer Oveflow Test
			//AT91F_AIC_ConfigureIt(AT91C_BASE_AIC,AT91C_ID_TC0,AT91C_AIC_PRIOR_HIGHEST,AT91C_AIC_SRCTYPE_EXT_POSITIVE_EDGE,Interrupt_Handler_TC0_Lowlevel);
			
			//short disable timer (later init)
			//StopTimer();
			
			
			AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_TC0, TIMER0_INTERRUPT_LEVEL,AT91C_AIC_SRCTYPE_INT_POSITIVE_EDGE, Interrupt_Handler_TC0_Lowlevel );
			//AT91C_BASE_TC0->TC_IER = AT91C_TC_COVFS;
			AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC0);
			// AT91C_BASE_TC0->TC_CCR = AT91C_TC_SWTRG ;
			
			setLed(RED);
			//Test
			//initTimer();

	while(1)
	{   
	
	if(AT91F_US_GetChar((AT91PS_USART)AT91C_BASE_US1)=='R')
	setLed(RED);
	
		/*if(getDigInputState(AT91C_BASE_PIOB, AT91C_PIO_PB15))
		{
		setLed(RED);
		}
		else
		{
		resetLed(RED);
		}*/
		

		/*myIOtest=(AT91C_BASE_PIOB->PIO_PDSR);
		AT91F_US_SendFrame((AT91PS_USART)AT91C_BASE_US1, &myIOtest,4,0,0);
		AT91F_US_SendFrame((AT91PS_USART)AT91C_BASE_US1, "\n",1,0,0);
		WaitTicks(500000);
		
		setLed(GREEN | RED | YELLOW );
		myIOtest=(AT91C_BASE_PIOB->PIO_PDSR);
		AT91F_US_SendFrame((AT91PS_USART)AT91C_BASE_US1, &myIOtest,4,0,0);
		AT91F_US_SendFrame((AT91PS_USART)AT91C_BASE_US1, "\n",1,0,0);
		WaitTicks(500000);
		resetLed(GREEN | RED | YELLOW );*/
		
	
	}
	/*while(1) //USART Polling RX Test!
	{
	myUsartRxChar=AT91F_US_GetChar((AT91PS_USART)AT91C_BASE_US1);
		switch(myUsartRxChar)
		{
		case 'R':	
					setLed(RED);
					break;
					
		case 'Y':	
					setLed(YELLOW);
					break;
					
		case 'G':	
					setLed(GREEN);
					break;
					
					
		case 'C':	
					resetLed(GREEN | RED | YELLOW );
					break;
		
		default: break;
		}
	}*/

	
	/*while(1)	//Traffic light test:
	{
	
		//RED
		setLed(RED);
		resetLed(GREEN);
		resetLed(YELLOW);
		WaitTicks(1000000);
		
		//RED YELLOW
		setLed(YELLOW);
		WaitTicks(200000);
		
		//GREEN
		setLed(GREEN);
		resetLed(RED);
		resetLed(YELLOW);
		WaitTicks(1000000);
		
		//YELLOW
		setLed(YELLOW);
		resetLed(RED);
		resetLed(GREEN);
		WaitTicks(400000);	
	
	
	}*/
	
	
	
	
	
	
	
	//*************
	/*character = DBGU_GetChar();      //also done by PDC.. this is the place where the user will also start. like a ON button
	AT91F_US_PutChar((AT91PS_USART)AT91C_BASE_DBGU, character);
		switch(character)
		{
			case '0':
				SDCurrentBlock = SDBeginBlock;
				AT91F_US_EnableRx(USART_pt);  // we need the receiver 
				USART_pt->US_PTCR = AT91C_PDC_RXTDIS;
				Bufferwechsler = SDBuffer1;
				writer = ACTIVE; reader = NOT_ACTIVE;
				AT91C_BASE_PDC_MCI->PDC_RCR = 512;
				AT91C_BASE_MCI->MCI_IDR = AT91C_MCI_TXBUFE | AT91C_MCI_ENDTX; 
				AT91C_BASE_MCI->MCI_IER = AT91C_MCI_ENDRX; 
				globalj = 0; globali = 0; RCR_recirculated = 0;
				Print_LineonSD("\r\nWelcome to the RS232 to SDCard Datalogger\r\n");
				PutCalTimeHeader();
				AT91C_BASE_PDC_MCI->PDC_RCR = 0;   //  Here goes the first Block.	
				if ( readytowriteonSD == WRITE_NOW )
				{
					errorstatus = SDcard_WriteBlock_CMD24_R1(&mci_sdcard,(SDCurrentBlock*Max_Read_DataBlock_Length), (uint32 *)printBuffer,Max_Read_DataBlock_Length);		
					//* Wait end of Write
					SDcard_Poll_AT91CMCINOTBUSY_flag(AT91C_MCI_TIMEOUT); 
					if (errorstatus != SD_WRITE_NO_ERROR)	
					{ 
						DBGU_Printk( "\n\rWrite not OK\n\r");
						if (errorstatus == (SD_WRITE_MEMFULL_ERROR | SD_WRITE_FOUND_ERROR) )
						{
							setLed(RED);
							break;
						}
					}
					SDCurrentBlock++;
					readytowriteonSD = NOT_ACTIVE;
				}
				Configure_USART_RX_PDC(USART_pt,(char *)(usartBuffer1),(1024),0,0); 
				AT91F_US_EnableIt(USART_pt,AT91C_US_TIMEOUT | AT91C_US_FRAME | AT91C_US_OVRE | AT91C_US_PARE);
				AT91F_US_EnableIt(USART_pt, AT91C_US_ENDRX ); // interrupt only after 1024 bytes.
				while (!AT91F_US_RxReady(USART_pt));
				//character = (char)USART_pt->US_RHR;
				USART_pt->US_PTCR = AT91C_PDC_RXTEN;
				PutTimeStamp();
				//Bufferwechsler[globalj++] = character;
				//AT91C_BASE_PDC_MCI->PDC_RCR--;	
				//if(character == '\n')
				//{
				//	PutTimeStamp();
				//}
				readytowriteonSD = NOT_ACTIVE;
				do 
				{
				if(AT91C_BASE_DBGU->DBGU_RHR == 'q') break;
					if ( readytowriteonSD == WRITE_NOW )
					{
						errorstatus = SDcard_WriteBlock_CMD24_R1(&mci_sdcard,(SDCurrentBlock*Max_Read_DataBlock_Length), (uint32 *)printBuffer,Max_Read_DataBlock_Length);		
						// Wait end of Write
						SDcard_Poll_AT91CMCINOTBUSY_flag(AT91C_MCI_TIMEOUT); 
						if (errorstatus != SD_WRITE_NO_ERROR)	
						{ 
							DBGU_Printk( "\n\rWrite not OK\n\r");
							if (errorstatus == (SD_WRITE_MEMFULL_ERROR | SD_WRITE_FOUND_ERROR) )
							{
								setLed(RED);
								break;
							}
						}
						SDCurrentBlock++;
						readytowriteonSD = NOT_ACTIVE;
					}
					if( RCR_recirculated || ((USART_pt->US_RCR) < (1024-globali)) )
					{
					setLed(GREEN);  // shows that there are messages on USART
					character = usartBuffer1[globali++];
					Bufferwechsler[globalj++] = character;
					AT91C_BASE_PDC_MCI->PDC_RCR--;	
					if (globali == 1024) { globali = 0; RCR_recirculated = 0; }
					if(character == '\n')
					{
						PutTimeStamp();
					}
					}
					resetLed(GREEN); // shows no messages are currently coming on USART.
				}
				while(1);
				SDLastBlock = SDCurrentBlock; // to be used for the stop transmission button.
				break;

			case '1': //* Print in pooling
				quitflag = 0;
				AT91C_BASE_MCI->MCI_IDR = (AT91C_MCI_RXBUFF | AT91C_MCI_TXBUFE);				
				SDCurrentBlock = SDBeginBlock;
				AT91F_US_EnableTx(USART_pt);  // We need transmitter for making the device act as a reader. 
				USART_pt->US_PTCR = AT91C_PDC_TXTDIS;      
				reader = ACTIVE; writer=NOT_ACTIVE;
				USART_Printk("Welcome to reading form SD card\r\n");
				USART_Printk(" i> to get information about the card\r\n n> to read SD card one by one\r\n a> to read from the starting block to last block saved\r\n q> to quit tests\r\n");
				while (quitflag == 0 )
				{ 
					character = DBGU_GetChar();
					switch (character)
					{
						case 'i':
							USART_Printk("Total SD CARD Size : ");
							break;
						case 'n':
							Configure_USART_TX_PDC(USART_pt,(char *)SDBuffer1,512,0,0);
							if ((SDcard_ReadBlock_CMD17_R1(&mci_sdcard,(SDCurrentBlock*Max_Read_DataBlock_Length), (unsigned int *)SDBuffer1,Max_Read_DataBlock_Length)) != SD_READ_NO_ERROR)		
								DBGU_Printk( "\n\rRead not OK\n\r");
							//* Wait end of Read
							SDcard_Poll_AT91CMCINOTBUSY_flag(AT91C_MCI_TIMEOUT);
							while(!((USART_pt->US_CSR) & AT91C_US_TXRDY));  // wait till the US_THR is clear
							USART_pt->US_PTCR = AT91C_PDC_TXTEN;        // start sending data to USART
							// Glow Green LED ( sending to USART )
							while(!((USART_pt->US_CSR) & AT91C_US_ENDTX));
							USART_pt->US_PTCR = AT91C_PDC_TXTDIS;	
							SDCurrentBlock++;
							break;
						case 'a':
						//USART_pt->US_PTCR = AT91C_PDC_TXTDIS;
						for (SDCurrentBlock=SDBeginBlock;SDCurrentBlock<SDLastBlock;SDCurrentBlock++) 
						{
							Configure_USART_TX_PDC(USART_pt,(char *)SDBuffer1,512,0,0);
							if ((SDcard_ReadBlock_CMD17_R1(&mci_sdcard,(SDCurrentBlock*Max_Read_DataBlock_Length), (unsigned int *)SDBuffer1,Max_Read_DataBlock_Length)) != SD_READ_NO_ERROR)		
							DBGU_Printk("\n\rRead not OK\n\r");
							//* Wait end of Read
							SDcard_Poll_AT91CMCINOTBUSY_flag(AT91C_MCI_TIMEOUT);
							while(!((USART_pt->US_CSR) & AT91C_US_TXRDY));  // wait till the US_THR is clear
							USART_pt->US_PTCR = AT91C_PDC_TXTEN;        // start sending data to USART
							// Glow Green LED ( sending to USART )
							while(!((USART_pt->US_CSR) & AT91C_US_ENDTX));
							USART_pt->US_PTCR = AT91C_PDC_TXTDIS;							
						}
						break;
						case 'q':
							quitflag = 1;
							break;
						default:
							USART_Printk("\r\nBad choice, reading\r\n");
							break;		
					}
				}
			default:
				DBGU_Printk( "\n\rBad choice, writing\n\r");
				break;		
		}*/
	}	
return 0;
}
#endif


