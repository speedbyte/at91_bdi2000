
#ifndef main_c
#define main_c

/* 
REv 0.9 Test of the Interrupt Measurment Service
*/

#include <stdio.h>
#include <unistd.h>

#include "main.h"
#include "isr.h"
#include "InterruptMeasurmentService.h"
#include "Hal.h"



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
	// Initialisation of the board hardware (LED,UART,...)
	InitHardware();
	
	// Welcome Message
	char myBuffer[]="Hi this is AT91RM9200-EK booting up! ;)\n";
	AT91F_US_SendFrame((AT91PS_USART)AT91C_BASE_US1,(char *) &myBuffer,(sizeof(myBuffer)-1),0,0); //Including \0 at the end (sizeof(Buffer)-1) will not send the string delimiter
	
	// Resetting of all LED's
	resetLed(GREEN | RED | YELLOW );
	
	// Configure the demo interrupt PortB as rising
	InitDemoInterrupt(AT91C_BASE_PIOB,AT91C_ID_PIOB, MY_INT_PIN, AT91C_AIC_PRIOR_LOWEST, AT91C_AIC_SRCTYPE_EXT_POSITIVE_EDGE);
	
	// Initialize the Latency Measurment Service
	Init_Latency_Measurement(TIMER_CLOCK1,INT_COMP_ON);
	
	// Turn on the the RED LED
	setLed(RED);

	// Demo application to turn on the RED LED if a 'R' is received
	while(1)
	{
		if(AT91F_US_GetChar((AT91PS_USART)AT91C_BASE_US1)=='R')
		setLed(RED);
	}	
	
return 0;
}
#endif


