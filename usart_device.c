#ifndef usart_device_c
#define usart_device_c

// Include Standard LIB  files 
#include "mci_type.h"
#include "AT91RM9200.h"
#include "lib_AT91RM9200.h"
#include "isr_usart.h" 			// usart_asm_irq_handler()
#include "init.h" 				// AT91F_DBGU_Printk

#define	MCK	60000000
#define USART_INTERRUPT_LEVEL		7

#ifndef usart_device_c
extern void Usart_c_irq_handler(AT91PS_USART USART_pt);      // to isr_usart.s
extern void AT91F_US_Printk( char *buffer) ;
extern void AT91F_US_Print_frame(char *buffer, unsigned short counter);
extern void AT91F_US_Print_2_frame(char *buffer, unsigned short counter,char *buffer2,unsigned short counter2);
extern void Usart_init ( void );
#endif
static const char atmel_header[]=
{
"\n\r  *** ATMEL Usart IRQ ***\n\r"
"Copyright (C) 2003 ATMEL Corporations Version: 1.0\n\r"
};

char message[80];
AT91PS_USART COM0;

//*------------------------- Internal Function --------------------------------
//*------------------------- Interrupt Function -------------------------------

//*----------------------------------------------------------------------------
//* Function Name       : Usart_c_irq_handler
//* Object              : C handler interrupt function called by the interrupts 
//*                       assembling routine
//* Input Parameters    : <RTC_pt> time rtc descriptor
//* Output Parameters   : increment count_timer0_interrupt
//*----------------------------------------------------------------------------
void Usart_c_irq_handler(AT91PS_USART USART_pt)
{
	AT91F_DBGU_Printk("\n\rI am in the beginning of usart interrupt\n\r");
	unsigned int status;
	//* get Usart status register
	status = USART_pt->US_CSR;
	if ( status & AT91C_US_ENDRX){
		//* Acknowledge Interrupt
		 	AT91F_US_ReceiveFrame(USART_pt,(char *)message,10,0,0);         
		//* Get byte and send	
   		//* Trace on DBGU
		
    		AT91F_US_SendFrame((AT91PS_USART) AT91C_BASE_DBGU, "ENDRX\n\r",7,0,0);
	}
	// check if interrupt is present and available 
	if ( (status & AT91C_US_ENDTX) &  (USART_pt->US_IMR & AT91C_US_ENDTX) ){
		 //*  Acknowledge Interrupt by mask for next send
		 AT91F_US_DisableIt(USART_pt, AT91C_US_ENDTX );
   		//* Trace on DBGU
    	AT91C_BASE_PDC_US1->PDC_PTCR = AT91C_PDC_TXTDIS;
		AT91F_US_SendFrame((AT91PS_USART) AT91C_BASE_DBGU, "ENDTX\n\r",7,0,0);
	}
	// check if interrupt is present and available 
	if ( (status & AT91C_US_TXBUFE) &  (USART_pt->US_IMR & AT91C_US_TXBUFE) ){
		 //*  Acknowledge Interrupt by mask for next send
		 AT91F_US_DisableIt(USART_pt, AT91C_US_TXBUFE );
   		//* Trace on DBGU
    		AT91F_US_SendFrame((AT91PS_USART) AT91C_BASE_DBGU, "TXBUFE\n\r",8,0,0);
	}
	
	if ( status & AT91C_US_OVRE) {
		//* clear US_RXRDY
		 AT91F_US_GetChar(USART_pt);
   		//* Trace on DBGU
    		AT91F_US_SendFrame((AT91PS_USART) AT91C_BASE_DBGU, "O",1,0,0);
	}

	//* Check error
	if ( status & AT91C_US_PARE) {
   		//* Trace on DBGU
    		AT91F_US_SendFrame((AT91PS_USART) AT91C_BASE_DBGU, "P",1,0,0);
	}

	if ( status & AT91C_US_FRAME) {
   		//* Trace on DBGU
    		AT91F_US_SendFrame((AT91PS_USART) AT91C_BASE_DBGU, "F",1,0,0);
	}

	if ( status & AT91C_US_TIMEOUT){
		USART_pt->US_CR = AT91C_US_STTTO;
   		//* Trace on DBGU
    		AT91F_US_SendFrame((AT91PS_USART) AT91C_BASE_DBGU, "T",1,0,0);
	}
	AT91F_DBGU_Printk("\n\rI am in the end of usart interrupt\n\r" );

	//* Reset the satus bit
	 USART_pt->US_CR = AT91C_US_RSTSTA;  //Resets the status bits PARE, FRAME, OVRE and RXBRK in the US_CSR.
}
//*-------------------------- External Function -------------------------------

//*----------------------------------------------------------------------------
//* \fn    AT91F_DBGU_Printk
//* \brief This function is used to send a string through the US1 channel 
//*----------------------------------------------------------------------------
void AT91F_US_Printk( char *buffer) // \arg pointer to a string ending by \0
{
	while(*buffer != '\0') {
		while (!AT91F_US_TxReady(COM0));  // Return 1 if a character can be written in US_THR
		AT91F_US_PutChar(COM0, *buffer++);   //pUSART->US_THR = (character & 0x1FF); no Ready to Send check
	}
}
//*----------------------------------------------------------------------------
//* \fn    AT91F_US_Print_frame
//* \brief This function is used to send a Frame through the US1 channel 
//*----------------------------------------------------------------------------
void AT91F_US_Print_frame(char *buffer, unsigned short counter) // \arg pointer to a string ending by \0
{
    //* Enable USART IT error and AT91C_US_ENDRX
 	AT91F_US_SendFrame(COM0,buffer,counter,0,0);
 	//* enable IT
 	AT91F_US_EnableIt(COM0, AT91C_US_ENDTX );
}
//*----------------------------------------------------------------------------
//* \fn    AT91F_US_Print_2_frame
//* \brief This function is used to send a Frame through the US1 channel 
//* (Very low level debugging)
//*----------------------------------------------------------------------------
void AT91F_US_Print_2_frame(char *buffer, unsigned short counter,char *buffer2,unsigned short counter2) // \arg pointer to a string ending by \0
{
    //* Enable USART IT error and AT91C_US_ENDRX
 	AT91F_US_SendFrame(COM0,buffer,counter,buffer2,counter2);
 	//* enable IT
 	AT91F_US_EnableIt(COM0, AT91C_US_TIMEOUT | AT91C_US_FRAME | AT91C_US_OVRE |AT91C_US_ENDRX | AT91C_US_ENDTX | AT91C_US_ENDTX |AT91C_US_TXBUFE);
}
//*----------------------------------------------------------------------------
//* Function Name       : Usart_init
//* Object              : USART initialization 
//* Input Parameters    : none
//* Output Parameters   : TRUE
//*----------------------------------------------------------------------------
int Usart_init ( void )
//* Begin
{
	
	COM0= AT91C_BASE_US1;

        //* Define RXD and TXD as peripheral
        AT91F_PIO_CfgPeriph(AT91C_BASE_PIOB,AT91C_PB21_RXD1 | AT91C_PB20_TXD1,0);

    	// First, enable the clock of the PIOB
    	AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1<<AT91C_ID_US1 ) ;
	
	// Usart Configure
	AT91F_US_Configure (COM0, MCK,AT91C_US_ASYNC_MODE, 115200, 0);
	
	// Enable usart 
	COM0->US_CR = AT91C_US_RXEN | AT91C_US_TXEN;
    	
    //* Enable USART IT error and AT91C_US_ENDRX
    	//AT91F_US_EnableIt(COM0,AT91C_US_TIMEOUT | AT91C_US_FRAME | AT91C_US_OVRE |AT91C_US_ENDRX | AT91C_US_ENDTX);

    	//* open Usart 1 interrupt
	AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_US1, USART_INTERRUPT_LEVEL,AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE, usart_asm_irq_handler); 
	AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_US1); 

	//* set time out US_RTOR
   //* Arm time out after 255 * 4  bits time 
   //* for 115200 bit time = 1/115200 = 8.6 micro sec time out unuti= 4* 8.6 = 34 micro
   //* 
	COM0->US_RTOR = 0xFFFF;   // Reciever TimeOut register . Time out reg is loaded with 65535 when Start Time out command is given or each new data is recieved.
   // *Enable usart SSTO
	COM0->US_CR = AT91C_US_STTTO;

	AT91F_US_PutChar (COM0,'X');
    //* Enable USART IT error and AT91C_US_ENDRX
 	AT91F_US_ReceiveFrame(COM0,(char *)message,10,0,0);
    //* first
 	AT91F_US_SendFrame(COM0,(char *)atmel_header,sizeof(atmel_header),0,0);
	while(!((COM0->US_CSR) & AT91C_US_ENDTX)) ;
	return TRUE;
//* End
}
#endif
