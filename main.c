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
#include "st_device.h"

#ifndef main_c
//* Functions
void Led_glow();
extern char AT91F_DBGU_getc(void);
int AT91F_InitDeviceStructure(void);
extern void AT91F_MCI_Handler(void);  // to isr.S
extern int main(void);     // to cstartup.S
#endif

//* Global Variables
AT91PS_USART USART_pt = AT91C_BASE_US1;
AT91PS_MCI MCI_pt;
static int MciBeginBlock = 10;
AT91S_MciDeviceFeatures			MCI_Device_Features;
AT91S_MciDeviceDesc				MCI_Device_Desc;
AT91S_MciDevice					MCI_Device;
char							Buffer[BUFFER_SIZE_MCI_DEVICE];
char							Buffer2[BUFFER_SIZE_MCI_DEVICE];
int readytowriteonSD = 0;
char *dummy;
// Local Functions
void Led_glow()
{
	AT91C_BASE_PIOB->PIO_PER = AT91C_PIO_PB0|AT91C_PIO_PB1|AT91C_PIO_PB2;
	AT91C_BASE_PIOB->PIO_OER = AT91C_PIO_PB0|AT91C_PIO_PB1|AT91C_PIO_PB2;   // a 1 at the relevant pin would pull it down
	
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
	int j;
	unsigned int Max_Read_DataBlock_Length;

	Mci_init();
	Usart_init();
	St_init(); 
	dummy = Buffer;
	if(AT91F_InitDeviceStructure() != AT91C_INIT_OK) {
		AT91F_DBGU_Printk("\n\rSDcARD Initialisation failed\n\r");
		return FALSE;}
	Led_glow();
	for (j=0;j<512;j++) Buffer[j] = 'A';
	for (j=0;j<512;j++) Buffer2[j] = 'B';
	Max_Read_DataBlock_Length = MCI_Device.pMCI_DeviceFeatures->Max_Read_DataBlock_Length;
	AT91F_DBGU_Printk("\n\r0) Write to SD 1) Read from SD\n\r");
	
	//Interrupt Enable USART:
	AT91F_US_EnableIt(USART_pt, AT91C_US_ENDRX | AT91C_US_ENDTX ); // All PDC related
													// Error related done in INIT
	AT91F_US_DisableIt(USART_pt , AT91C_US_RXRDY ); // TXRDY and RXRDY are automatically status are set and cleared.
	
	// Disable all PDC related	
	USART_pt->US_PTCR = AT91C_PDC_RXTDIS;
	USART_pt->US_PTCR = AT91C_PDC_TXTDIS;      
	//MCI_pt->MCI_PTCR = AT91C_PDC_RXTDIS;  // this is reduntant as it is disable in the beginning of the sdcard read 
	//MCI_pt->MCI_PTCR = AT91C_PDC_TXTDIS;  // this is reduntant as it is disable in the beginning of the sdcard write
	USART_pt->US_PTCR = AT91C_PDC_RXTEN;  
	readytowriteonSD = 2;
j = 0;  
	while(1)
	{
		
		caractere = AT91F_DBGU_getc();      // also done by PDC
		
		switch(caractere)
		{
			case '0':			
				while(1){
				
				
				//while (!AT91F_US_RxReady(USART_pt));
				AT91F_US_PutFrame(USART_pt,(char *)dummy,512,(char *)Buffer2,512); 
				    // start receiving the data from USART
				// Glow Green LED ( reading from USART )
				//while(1);
				while(!((USART_pt->US_CSR) & AT91C_US_ENDRX));
				((readytowriteonSD % 2 ) != 0) ? (dummy = Buffer) : (dummy = Buffer2);
				//while(USART_pt->USART_RCR);
				//USART_pt->US_PTCR = AT91C_PDC_RXTDIS;
		
				if ((AT91F_MCI_WriteBlock(&MCI_Device,(MciBeginBlock*Max_Read_DataBlock_Length), (unsigned int *)dummy,Max_Read_DataBlock_Length)) != AT91C_WRITE_OK)		
					AT91F_DBGU_Printk("\n\rWrite not OK\n\r");
				// glow yellow LED ( writing to SD card )				
				//* Wait end of Write
				AT91F_MCI_DeviceWaitReady(AT91C_MCI_TIMEOUT);
				USART_pt->US_PTCR = AT91C_PDC_RXTEN;
				USART_pt->US_PTCR = AT91C_PDC_RXTEN;
				MciBeginBlock++;
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
