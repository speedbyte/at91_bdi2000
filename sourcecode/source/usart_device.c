#ifndef usart_device_c
#define usart_device_c

// Include Standard LIB  files 
#include "mci_type.h"
#include "AT91RM9200.h"
#include "lib_AT91RM9200.h"
#include "isr.h" 			// Interrupt_Handler_USART_Lowlevel()
#include "init.h" 				// Send_Stream_from_RS232_to_Terminal
#include "usart_device.h"       // selber inc
#include "main.h"         // readytowriteonSD
#include "st_device.h"      // AT91F_GetTickCount

#ifndef usart_device_c
extern void Interrupt_Handler_USART_Highlevel(AT91PS_USART USART_pt);      // to isr_usart.s
extern void Configure_USART_RX_PDC(AT91PS_USART USART_pt, char *buffer, unsigned short counter,char *buffer2,unsigned short counter2);
extern void Usart_init ( void );
extern void Configure_USART_TX_PDC(AT91PS_USART USART_pt, char *buffer, unsigned short counter,char *buffer2,unsigned short counter2); 
#endif


//*----------------------------------------------------------------------------
//* Function Name       : Interrupt_Handler_USART_Highlevel
//* Object              : C handler interrupt function called by the interrupts 
//*                       assembling routine
//*----------------------------------------------------------------------------
void Interrupt_Handler_USART_Highlevel(AT91PS_USART USART_pt)
{
	unsigned int status;
	//* get Usart status register
	status = USART_pt->US_CSR;
	
	if ( (status & AT91C_US_ENDTX) &  (USART_pt->US_IMR & AT91C_US_ENDTX) ){
		 //*  Acknowledge Interrupt by mask for next send
		 AT91F_US_DisableIt(USART_pt, AT91C_US_ENDTX );
   		//* Trace on DBGU
    		AT91F_US_PutChar((AT91PS_USART) AT91C_BASE_DBGU, 'S');
	}
	
	if (( status & AT91C_US_ENDRX) & (USART_pt->US_IMR & AT91C_US_ENDRX)){ //* Acknowledge Interrupt by reading the status register.
   				//* Acknowledge Interrupt
			AT91F_US_DisableIt(USART_pt, AT91C_US_ENDRX );  // Disable it to avoid regeneration.
			Configure_USART_RX_PDC(USART_pt,(char *)(usartBuffer1),(1024),0,0); 
			AT91F_US_EnableIt(USART_pt, AT91C_US_ENDRX);			
    		AT91F_US_PutChar((AT91PS_USART) AT91C_BASE_DBGU, 'R');
			RCR_recirculated = 1;
	}

	if ( status & AT91C_US_OVRE) {
		//* clear US_RXRDY
		 AT91F_US_GetChar(USART_pt);
   		//* Trace on DBGU
    		AT91F_US_PutChar((AT91PS_USART) AT91C_BASE_DBGU, 'O');

	}
	//* Check error
	if ( status & AT91C_US_PARE) {
   		//* Trace on DBGU
    		AT91F_US_PutChar((AT91PS_USART) AT91C_BASE_DBGU, 'P');
	}

	if ( status & AT91C_US_FRAME) {
   		//* Trace on DBGU
    		AT91F_US_PutChar((AT91PS_USART) AT91C_BASE_DBGU, 'F');
	}

	if ( status & AT91C_US_TIMEOUT){
		USART_pt->US_CR = AT91C_US_STTTO;
   		//* Trace on DBGU
    		AT91F_US_PutChar((AT91PS_USART) AT91C_BASE_DBGU, 'T');

	}

	//* Reset the status bit
	USART_pt->US_CR = AT91C_US_RSTSTA;  //Resets the status bits PARE, FRAME, OVRE and RXBRK in the US_CSR.
}


void Configure_USART_TX_PDC(AT91PS_USART USART_pt, char *buffer, unsigned short counter,char *buffer2,unsigned short counter2) // \arg pointer to a string ending by \0
{
    //* Enable USART IT error and AT91C_US_ENDTX
 	AT91F_US_SendFrame(USART_pt,buffer,counter,buffer2,counter2);
 	//* enable IT
 	AT91F_US_EnableIt(USART_pt, AT91C_US_TIMEOUT | AT91C_US_FRAME | AT91C_US_OVRE  | AT91C_US_ENDTX );
}

void Configure_USART_RX_PDC(AT91PS_USART USART_pt, char *buffer, unsigned short counter,char *buffer2,unsigned short counter2) 
{
    //* Enable USART IT error and AT91C_US_ENDRX
 	AT91F_US_ReceiveFrame(USART_pt,buffer,counter,buffer2,counter2);
 	//* enable IT
 	AT91F_US_EnableIt(USART_pt, AT91C_US_TIMEOUT | AT91C_US_FRAME | AT91C_US_OVRE   );
}



//*----------------------------------------------------------------------------
//* Function Name       : Usart_init
//* Object              : USART initialization 
//* Input Parameters    : none
//* Output Parameters   : TRUE
//*----------------------------------------------------------------------------
void Usart_init ( void )
//* Begin
{
	AT91PS_USART USART_pt ;
	USART_pt = AT91C_BASE_US1;
	char dumbchar;
    //* Define RXD and TXD as peripheral
    AT91F_PIO_CfgPeriph(AT91C_BASE_PIOB,AT91C_PB21_RXD1 | AT91C_PB20_TXD1,0);

    // First, enable the clock of the PIOB
    AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1<<AT91C_ID_US1 ) ;

	// Usart Configure
	AT91F_US_Configure (USART_pt, MASTER_CLOCK ,AT91C_US_ASYNC_MODE, 115200, 0); // time guard TTGR =0 , opp of RTOR
/*                        mode = AT91C_US_USMODE_NORMAL + stopbit = {AT91C_US_NBSTOP_1_BIT} + \
                        no parity = {AT91C_US_PAR_NONE} + \
                        characterlength ( apart from par, stop,){AT91C_US_CHRL_8_BITS} + \
                        clock selection - {AT91C_US_CLKS_CLOCK} )
*/	
/*
	unsigned int baud_value = ((main_clock*10)/(baud_rate * 16));
	if ((baud_value % 10) >= 5)
		baud_value = (baud_value / 10) + 1;
	else
		baud_value /= 10;
		*/

	// Enable usart 
	// Enable transmit and recieve ( this has nothing to do with PDC which is PDC_RXTEN ..
    AT91F_US_EnableRx(USART_pt);
	AT91F_US_EnableTx(USART_pt);	//* open Usart 1 interrupt
	AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_US1, USART_INTERRUPT_LEVEL,AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE, Interrupt_Handler_USART_Lowlevel); 
	AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_US1); 

    //* Enable USART IT error
	// if u dont enable it is automatically disabled.
   	//AT91F_US_DisableIt(USART_pt,AT91C_US_TIMEOUT | AT91C_US_FRAME | AT91C_US_OVRE );
   	AT91F_US_DisableIt(USART_pt,AT91C_US_ENDTX | AT91C_US_ENDRX | AT91C_US_TXBUFE | AT91C_US_RXBUFF);
	//* set time out US_RTOR
   //* Arm time out after 255 * 4  bits time 
   //* for 115200 bit time = 1/115200 = 8.6 micro sec time out unuti= 4* 8.6 = 34 micro
   //* 
	USART_pt->US_RTOR = 0x0;   // Reciever TimeOut register . Time out reg is loaded with 65535 when Start Time out command is given or each new data is recieved.
   // *Enable usart SSTO
	USART_pt->US_CR = AT91C_US_STTTO;           // StartTimeOut according to Recieve TimeoutRegister. interrutpt when longer than that.
	// Currently i will only use the human switch rather than these registers. So i piut 0 in the timer counter.
	USART_pt->US_CR = AT91C_US_RSTSTA; 	
	dumbchar = USART_pt->US_RHR;  // this would make RXRDY low but the char would remain the register. it is overwritten by the next byte.
//* End

}
#endif