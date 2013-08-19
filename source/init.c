#ifndef init_c
#define init_c
//*----------------------------------------------------------------------------
//*         ATMEL Microcontroller Software Support  -  ROUSSET  -
//*----------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*----------------------------------------------------------------------------
//* File Name           : init.c
//* Object              : Low level initialisations written in C
//* Creation            : FB   21/11/2002
//*
//*----------------------------------------------------------------------------
#include "mci_type.h"
#include "AT91RM9200.h"
#include "lib_AT91RM9200.h"
#include "main.h"
#include "mci_device.h"

#ifndef init_c
extern void DBGU_Printk(char *buffer);
void AT91F_SpuriousHandler() 
void AT91F_DataAbort() 
void AT91F_FetchAbort()
void AT91F_Undef() 
void AT91F_UndefHandler() 
void AT91F_LowLevelInit()

#endif

#define MASTER_CLOCK	60000000
#define BAUDRATE		115200

//*----------------------------------------------------------------------------
//* \fn    DBGU_Printk
//* \brief This function is used to send a string through the DBGU channel (Very low level debugging)
//*----------------------------------------------------------------------------
void DBGU_Printk(char *buffer) // \arg pointer to a string ending by \0
{
	while(*buffer != '\0') {
		while (!AT91F_US_TxReady((AT91PS_USART)AT91C_BASE_DBGU));
		AT91F_US_PutChar((AT91PS_USART)AT91C_BASE_DBGU, *buffer++);
	}
}

void USART_Printk(char *buffer) // \arg pointer to a string ending by \0
{
	while(*buffer != '\0') {
		while (!AT91F_US_TxReady((AT91PS_USART)AT91C_BASE_US1));
		AT91F_US_PutChar((AT91PS_USART)AT91C_BASE_US1, *buffer++);
	}
}
//*----------------------------------------------------------------------------
//* \fn    AT91F_DataAbort
//* \brief This function reports an Abort
//*----------------------------------------------------------------------------
void AT91F_SpuriousHandler() 
{
	DBGU_Printk("\n\rSpurious Interrupt detected\n\r");
	while (1);
}


//*----------------------------------------------------------------------------
//* \fn    AT91F_DataAbort
//* \brief This function reports an Abort
//*----------------------------------------------------------------------------
void AT91F_DataAbort() 
{
	DBGU_Printk("\n\rData Abort detected\n\r");
	while (1);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_FetchAbort
//* \brief This function reports an Abort
//*----------------------------------------------------------------------------
void AT91F_FetchAbort()
{
	DBGU_Printk("\n\rPrefetch Abort detected\n\r");
	while (1);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_Undef
//* \brief This function reports an Abort
//*----------------------------------------------------------------------------
void AT91F_Undef() 
{
	DBGU_Printk("\n\rUndef detected\n\r");
	while (1);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_UndefHandler
//* \brief This function reports that no handler have been set for current IT
//*----------------------------------------------------------------------------
void AT91F_UndefHandler() 
{
	DBGU_Printk("\n\rUndef detected\n\r");
	while (1);
}


//*----------------------------------------------------------------------------
//* \fn    AT91F_LowLevelInit
//* \brief This function performs very low level HW initialization
//*----------------------------------------------------------------------------
void AT91F_LowLevelInit()
{
	// Init Interrupt Controller
	AT91F_AIC_Open(
		AT91C_BASE_AIC,          // pointer to the AIC registers
		AT91C_AIC_BRANCH_OPCODE, // IRQ exception vector
		AT91F_UndefHandler,      // FIQ exception vector
		AT91F_UndefHandler,      // AIC default handler
		AT91F_SpuriousHandler,   // AIC spurious handler
		0);                      // Protect mode
/*#define AT91C_AIC_BRANCH_OPCODE ((void (*) ()) 0xE51FFF20) // ldr, pc, [pc, #-&F20]

//*----------------------------------------------------------------------------
//* \fn    AT91F_AIC_SetExceptionVector
//* \brief Configure vector handler
//*----------------------------------------------------------------------------
__inline static unsigned int  AT91F_AIC_SetExceptionVector (
	unsigned int *pVector, // \arg pointer to the AIC registers
	void (*Handler) () )   // \arg Interrupt Handler
{
	unsigned int oldVector = *pVector;

	if ((unsigned int) Handler == (unsigned int) AT91C_AIC_BRANCH_OPCODE)
		*pVector = (unsigned int) AT91C_AIC_BRANCH_OPCODE;
	else
		*pVector = (((((unsigned int) Handler) - ((unsigned int) pVector) - 0x8) >> 2) & 0x00FFFFFF) | 0xEA000000;

	return oldVector;
}
	// Set the IRQ exception vector
	AT91F_AIC_SetExceptionVector((unsigned int *) 0x18, IrqHandler);
	// Set the Fast Interrupt exception vector
	AT91F_AIC_SetExceptionVector((unsigned int *) 0x1C, FiqHandler);
*/
	// Perform 8 End Of Interrupt Command to make sýre AIC will not Lock out nIRQ 
	AT91F_AIC_AcknowledgeIt(AT91C_BASE_AIC);
	AT91F_AIC_AcknowledgeIt(AT91C_BASE_AIC);
	AT91F_AIC_AcknowledgeIt(AT91C_BASE_AIC);
	AT91F_AIC_AcknowledgeIt(AT91C_BASE_AIC);
	AT91F_AIC_AcknowledgeIt(AT91C_BASE_AIC);
	AT91F_AIC_AcknowledgeIt(AT91C_BASE_AIC);
	AT91F_AIC_AcknowledgeIt(AT91C_BASE_AIC);
	AT91F_AIC_AcknowledgeIt(AT91C_BASE_AIC);

	AT91F_AIC_SetExceptionVector((unsigned int *)0x0C, AT91F_FetchAbort);
	AT91F_AIC_SetExceptionVector((unsigned int *)0x10, AT91F_DataAbort);
	AT91F_AIC_SetExceptionVector((unsigned int *)0x4, AT91F_Undef);

	// Open PIO for DBGU
	AT91F_DBGU_CfgPIO();

	// Configure DBGU
	AT91F_US_Configure (
		(AT91PS_USART) AT91C_BASE_DBGU,          // DBGU base address
		MASTER_CLOCK,         // 60 MHz
		AT91C_US_ASYNC_MODE,  // mode Register to be programmed
		BAUDRATE ,            // baudrate to be programmed
		0);                   // timeguard to be programmed

	// Enable Transmitter
	AT91F_US_EnableTx((AT91PS_USART) AT91C_BASE_DBGU);
	// Enable Receiver
	AT91F_US_EnableRx((AT91PS_USART) AT91C_BASE_DBGU);
	
	DBGU_Printk("\n\r\nAT91F_LowLevelInit() done\n\r");
	
}

#endif
