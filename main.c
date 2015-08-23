//*----------------------------------------------------------------------------
//* AT91SAM7 UART and GPIO example
//* by Martin Thomas 10/2005
//* 
//* modified and extended as a WinARM-Example
//* same warranty as the original code
//*
//* - simple UART output (mixture of other demos esp. from Keil)
//* - send Text on SW4 (PA14) and switch LED4
//* - Reset Button on Demo-Board enabled
//* - old Demo "Knight-Rider" Function now with just 3 LEDS,
//* - SW1 and SW3 still control speed
//*
//* based on code from:
//*----------------------------------------------------------------------------
//*         ATMEL Microcontroller Software Support  -  ROUSSET  -
//*----------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*----------------------------------------------------------------------------
//* File Name           : main.c
//* Object              : main application written in C
//* Creation            : JPP   16/Jun/2004
//*----------------------------------------------------------------------------

#include "Board.h"
#include "Cstartup_SAM7.h"
#include "serial.h"
#include "rprintf.h"

/* Global variables */
#define SPEED 		(MCKKHz/10)
unsigned int LedSpeed = SPEED *50 ;
const int led_mask[8]= {LED1, LED2, LED3, LED4};

//*--------------------------------------------------------------------------------------
//* Function Name       : change_speed
//* Object              : Adjust "LedSpeed" value depending on SW1 and SW2 are pressed or not
//* Input Parameters    : none
//* Output Parameters   : Update of LedSpeed value.
//*--------------------------------------------------------------------------------------
static void change_speed ( void )
{//* Begin
    if ( (AT91F_PIO_GetInput(AT91C_BASE_PIOA) & SW1_MASK) == 0 )
    {
        if ( LedSpeed > SPEED ) LedSpeed -=SPEED ;
    }
    if ( (AT91F_PIO_GetInput(AT91C_BASE_PIOA) & SW3_MASK) == 0 )
    {
        if ( LedSpeed < MCK ) LedSpeed +=SPEED ;
    }
}//* End

//*--------------------------------------------------------------------------------------
//* Function Name       : wait
//* Object              : Software waiting loop
//* Input Parameters    : none. Waiting time is defined by the global variable LedSpeed.
//* Output Parameters   : none
//*--------------------------------------------------------------------------------------
static void wait ( void )
{//* Begin
    unsigned int waiting_time ;
    change_speed () ;
    for(waiting_time = 0; waiting_time < LedSpeed; waiting_time++) ;
}//* End


static void check_key( void )
{
	int c;
	
	if ( uart0_kbhit() ) {
		c = uart0_getc();
		rprintf("\nYou pressed the \"%c\" key\n", (char)c);
	}
}


//*--------------------------------------------------------------------------------------
//* Function Name       : Main
//* Object              : Software entry point
//* Input Parameters    : none.
//* Output Parameters   : none.
//*--------------------------------------------------------------------------------------
int main(void)
{//* Begin
	volatile int i;
	unsigned long keystate;
	
	i = 0xDEAD;
	
	// call low-level init
	AT91F_LowLevelInit();
	
	// enable the clock of the PIO (mt:) and UART0
	AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, ( 1 << AT91C_ID_PIOA ) | ( 1 << AT91C_ID_US0 ) ) ;
	
	// then, we configure the PIO Lines corresponding to LED1 to LED4
	// to be outputs. No need to set these pins to be driven by the PIO because it is GPIO pins only.
	AT91F_PIO_CfgOutput( AT91C_BASE_PIOA, LED_MASK ) ;
	
	// Clear the LED's. On the EB55 we must apply a "1" to turn off LEDs
	AT91F_PIO_SetOutput( AT91C_BASE_PIOA, LED_MASK ) ;
	
	
	// mt: enable reset-key on demo-board ("say hello on key")
	AT91F_RSTSetMode( AT91C_BASE_RSTC , AT91C_RSTC_URSTEN );
		
	uart0_init();
	uart0_puts("\n\nHello World! (WinARM) (2c/tango/814)\n");
	
	rprintf("rprintf Test %i %i %i\n", 1, 2, 3);
	
	// Loop forever
	for (;;)
	{
		// Once a Shot on each led
		///uart0_puts("ping");
		for ( i=0 ; i < NB_LEB-1 ; i++ )
		{
			AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, led_mask[i]) ;
			wait();
			AT91F_PIO_SetOutput( AT91C_BASE_PIOA, led_mask[i] ) ;
			wait();
		}// End for
		
		check_key();
	
		// Once a Shot on each led
		///uart0_puts("pong");
		for ( i=(NB_LEB-1-1) ; i >= 0 ; i-- )
		{
			AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, led_mask[i]) ;
			wait();
			AT91F_PIO_SetOutput( AT91C_BASE_PIOA, led_mask[i] ) ;
			wait();
		}
		
		check_key();
		
		keystate = AT91F_PIO_GetInput( AT91C_BASE_PIOA ); 
		if ( ( keystate & SW4 ) == 0 ) { // keys low active
			// pressed -> blabla 
			uart0_puts("\nklickiklick\n");
			// and LED4 on
			AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, LED4 ) ;
		}
		else {
			// LED4 off
			AT91F_PIO_SetOutput( AT91C_BASE_PIOA, LED4 ) ;
		}
		

	} // End for

} //* End
