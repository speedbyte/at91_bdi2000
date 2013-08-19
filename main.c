#ifndef main_c
#define main_c

/* 
REv 1.0 : Initial build with MCI Drivers.
REv 1.1 : Improvemnent directory structure, Created header file
REv 1.2 : Integrated USART function calls
*/

#include <stdio.h>
#include "mci_type.h"
#include "AT91RM9200.h"
#include "lib_AT91RM9200.h"
#include "main.h"
#include "mci_device.h"    // AT91F_MCI_SDCard_Init() , Mci_init() , AT91F_MCI_DeviceWaitReady(), AT91F_MCI_ReadBlock(), AT91F_MCI_WriteBlock()
#include "init.h"          // AT91F_DBGU_Printk()
#include "usart_device.h"  // AT91F_US_Print_frame(), Usart_init(), AT91F_US_Printk(), AT91F_US_Print_2_frame()

#ifndef main_c
//* Functions
void Led_glow();
void Led_init();
extern char AT91F_DBGU_getc(void);
int AT91F_InitDeviceStructure(void);
extern void AT91F_MCI_Handler(void);  // to isr_mci.s
extern int main(void);     // to cstartup.s
#endif

//* Global Variables
AT91PS_USART USART_pt = AT91C_BASE_US1;
AT91PS_MCI MCI_pt;
static int MciBeginBlock = 0;
AT91S_MciDeviceFeatures			MCI_Device_Features;
AT91S_MciDeviceDesc				MCI_Device_Desc;
AT91S_MciDevice					MCI_Device;
char							Buffer[BUFFER_SIZE_MCI_DEVICE];
AT91PS_USART pUSART_vikas; 

// Local Functions
void Led_glow()
{
	pUSART_vikas = AT91C_BASE_US3; 	
	pUSART_vikas->US_CR = AT91C_US_RTSEN;  // to disable use RTSDIS
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_DBGU_getc
//* \brief This function is used to receive a character to the DBGU channel
//*----------------------------------------------------------------------------
char AT91F_DBGU_getc(void)
{
	while (!AT91F_US_RxReady((AT91PS_USART)AT91C_BASE_DBGU));
	return AT91F_US_GetChar((AT91PS_USART) AT91C_BASE_DBGU);
}

void Led_init()
{
	// Configure PIO controllers to periph mode
	AT91F_PIO_CfgPeriph(
		AT91C_BASE_PIOB, // PIO controller base address
		0,
		((unsigned int) AT91C_PIO_PB0)
		); // Peripheral B
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_CfgDevice
//* \brief This function is used to initialise MMC or SDCard Features
//*----------------------------------------------------------------------------
int AT91F_InitDeviceStructure(void)
{

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
	
	return(AT91F_MCI_SDCard_Init(&MCI_Device));
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
	unsigned char	caractere;
	Mci_init();
	Usart_init();
	Led_init();
	int i,x = 33,j;
	unsigned int Max_Read_DataBlock_Length;
	if(AT91F_InitDeviceStructure() != AT91C_INIT_OK) {
		AT91F_DBGU_Printk("\n\rSDcARD Initialisation failed\n\r");
		return FALSE;}
	Led_glow();
	Max_Read_DataBlock_Length = MCI_Device.pMCI_DeviceFeatures->Max_Read_DataBlock_Length;
	AT91F_DBGU_Printk("\n\r0) Write to SD 1) Read from SD\n\r");
	
	//Interrupt Enable USART:
	AT91F_US_EnableIt(USART_pt, AT91C_US_ENDRX | AT91C_US_ENDTX | AT91C_US_ENDTX | AT91C_US_TXBUFE); // All PDC related
													// Error related done in INIT
	// Disable all PDC related
	USART_pt->US_PTCR = AT91C_PDC_RXTDIS;
	USART_pt->US_PTCR = AT91C_PDC_TXTDIS;      
	MCI_pt->MCI_PTCR = AT91C_PDC_RXTDIS;  // this is reduntant as it is disable in the beginning of the sdcard read 
	MCI_pt->MCI_PTCR = AT91C_PDC_TXTDIS;  // this is reduntant as it is disable in the beginning of the sdcard write
	
	while(1)
	{
		USART_pt->US_PTCR = AT91C_PDC_RXTEN;
		caractere = AT91F_DBGU_getc();      // also done by PDC
		switch(caractere)
		{
			case '0':			
//				while(1){
				
				for (MciBeginBlock=0;MciBeginBlock<4;MciBeginBlock++)  
				{
				for (j=0;j<512;j++) {Buffer[j] = 33+x; if ( x==123 ) x=33;} x++;
				/*AT91F_US_ReceiveFrame(USART_pt,(char *)Buffer,512,0,0);  // without meaning ..effective only on PDC enable
				while(!((USART_pt->US_CSR) & AT91C_US_RXRDY));    // wait till the first char arrives.
				USART_pt->US_PTCR = AT91C_PDC_RXTEN;        // start receiving the data from USART
				// Glow Green LED ( reading from USART )
				while(!((USART_pt->US_CSR) & AT91C_US_ENDRX));*/
				USART_pt->US_PTCR = AT91C_PDC_RXTDIS;
				
				if ((AT91F_MCI_WriteBlock(&MCI_Device,(MciBeginBlock*Max_Read_DataBlock_Length), (unsigned int *)Buffer,Max_Read_DataBlock_Length)) != AT91C_WRITE_OK)		
					AT91F_DBGU_Printk("\n\rWrite not OK\n\r");
				// glow yellow LED ( writing to SD card )				
				//* Wait end of Write
				AT91F_MCI_DeviceWaitReady(AT91C_MCI_TIMEOUT);
				}
				
//				}
				break;

			case '1': //* Print in pooling
//				while(1){
				
				for (MciBeginBlock=0;MciBeginBlock<4;MciBeginBlock++) {
				if ((AT91F_MCI_ReadBlock(&MCI_Device,(MciBeginBlock*Max_Read_DataBlock_Length), (unsigned int *)Buffer,Max_Read_DataBlock_Length)) != AT91C_READ_OK)		
					AT91F_DBGU_Printk("\n\rRead not OK\n\r");
				//* Wait end of Read
				// glow Yellow LED
				AT91F_MCI_DeviceWaitReady(AT91C_MCI_TIMEOUT);
				
				AT91F_US_Print_2_frame(USART_pt,(char *)Buffer,512,0,0);
				while(!((USART_pt->US_CSR) & AT91C_US_TXRDY));  // wait till the US_THR is clear
				USART_pt->US_PTCR = AT91C_PDC_TXTEN;        // start sending data to USART
				// Glow Green LED ( sending to USART )
				while(!((USART_pt->US_CSR) & AT91C_US_ENDTX));
				USART_pt->US_PTCR = AT91C_PDC_TXTDIS;	
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
