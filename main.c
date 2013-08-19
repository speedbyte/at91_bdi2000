#ifndef main_c
#define main_c

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
AT91S_MciDeviceFeatures			MCI_Device_Features;
AT91S_MciDeviceDesc				MCI_Device_Desc;
AT91S_MciDevice					MCI_Device;
char					Buffer[BUFFER_SIZE_MCI_DEVICE*6];
AT91PS_USART pUSART_vikas; 

static const char POLLING_header[]=
{
"\n\r  *** ATMEL POLLING ***\n\r"
};

static const char PDC_header[]=
{
"\n\r  *** ATMEL PDC ***\n\r"
};
static const char PDC1_header[]=
{
"\n\r  *** ATMEL PDC 1 ***\n\r"
};
static const char PDC2_header[]=
{
"\n\r  *** ATMEL PDC 2 ***\n\r"
};
volatile char message[80];

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
	AT91F_DBGU_Printk("\n\rI am in MCI Interrupt handler\n\r");
}


//*----------------------------------------------------------------------------
//* \fn    main
//* \brief main function
//*----------------------------------------------------------------------------
int main()
{
	unsigned char	caractere;
	unsigned int Max_Read_DataBlock_Length;
	unsigned int i,j;
	int timeout;
	char *pt_char;
	if(Mci_init() != TRUE) {
		AT91F_DBGU_Printk("\n\rMCI Initialisation failed\n\r");
		return FALSE;}
	if(Usart_init() != TRUE) {
		AT91F_DBGU_Printk("\n\rUSART Initialisation failed\n\r");
		return FALSE;}	
	if(AT91F_InitDeviceStructure() != AT91C_INIT_OK) {
		AT91F_DBGU_Printk("\n\rSDcARD Initialisation failed\n\r");
		return FALSE;}
	Led_init();
	Led_glow();
	Max_Read_DataBlock_Length = MCI_Device.pMCI_DeviceFeatures->Max_Read_DataBlock_Length;
	AT91F_DBGU_Printk("\n\r0) SDCARD \n\r1) info\n\r2) Write US1 \n\r3) Send 1 frame\n\r4) 2 littel Frame\n\r5) 10 K Bytes\n\r6) 120s K Bytes\n\r");
	while(1)
	{
		caractere = AT91F_DBGU_getc();
		switch(caractere)
		{
			case '0':	
				
				//* Write Page 1
				//sprintf((char *)Buffer,"\n\rHell\n\r");
				/*for(i=0,j=0;i<BUFFER_SIZE_MCI_DEVICE*5;i++) 	{ 
				Buffer[i] = 'a'+j; 
				if((i%512)==1) j=1; else if((i%512)==2) j=2;else if((i%512)==3) j=3;else if((i%512)==4) j=4;}
				/*if ((AT91F_MCI_WriteBlock(&MCI_Device,(1*Max_Read_DataBlock_Length), (unsigned int *)Buffer,Max_Read_DataBlock_Length)) != AT91C_WRITE_OK)
				AT91F_DBGU_Printk("\n\rWrite not OK\n\r");
				//* Wait end of Write
				AT91F_MCI_DeviceWaitReady(AT91C_MCI_TIMEOUT);*/
				/*for(i=0;i<5;i++){
				if ((AT91F_MCI_WriteBlock(&MCI_Device,(i*Max_Read_DataBlock_Length),(unsigned int *)(Buffer + 512*i),Max_Read_DataBlock_Length)) != AT91C_WRITE_OK)
				AT91F_DBGU_Printk("\n\rRead not OK\n\r");
				//* Wait end of Read
				AT91F_DBGU_Printk("\n\rWriting\n\r");
				AT91F_MCI_DeviceWaitReady(AT91C_MCI_TIMEOUT);
				}*/
				//* Read Block 1
				for(i=0;i<BUFFER_SIZE_MCI_DEVICE*6;i++) 	Buffer[i] = 'z';	
				for(i=0;i<5;i=i+2){
				if ((AT91F_MCI_ReadBlock(&MCI_Device,((i)*Max_Read_DataBlock_Length),(unsigned int *)(Buffer + 512*i),Max_Read_DataBlock_Length)) != AT91C_READ_OK)
 				AT91F_DBGU_Printk("\n\rRead not OK\n\r");
				//* Wait end of Read
				AT91F_DBGU_Printk("\n\rREading\n\r");
				AT91F_MCI_DeviceWaitReady(AT91C_MCI_TIMEOUT);
				//}
				//for(i=0;i<5;i++){
				AT91F_US_EnableIt((AT91PS_USART)AT91C_BASE_US1, AT91C_US_TIMEOUT | AT91C_US_FRAME | AT91C_US_OVRE |AT91C_US_ENDRX | AT91C_US_ENDTX | AT91C_US_ENDTX |AT91C_US_TXBUFE);
				AT91C_BASE_PDC_US1->PDC_PTCR = AT91C_PDC_TXTDIS;
				AT91F_US_SendFrame((AT91PS_USART)AT91C_BASE_US1,(char *)(Buffer + 512*i),512,(char *)(Buffer + 512*(i+1)),512);
				AT91C_BASE_PDC_US1->PDC_PTCR = AT91C_PDC_TXTEN;
				timeout = 100000;
				if ( j == 0 ) AT91F_DBGU_Printk("All buffer full"); else if(j==1) AT91F_DBGU_Printk("NXtbufferavl"); else AT91F_DBGU_Printk("1st pointer avl");
				
				//while(!((AT91C_BASE_US1->US_CSR) & AT91C_US_ENDTX));
				//AT91C_BASE_PDC_US1->PDC_PTCR = AT91C_PDC_TXTDIS;
				AT91F_US_Printk("\n\r");
				AT91C_BASE_PDC_US1->PDC_PTCR = AT91C_PDC_TXTEN;
				while(!((AT91C_BASE_US1->US_CSR) & AT91C_US_TXBUFE));
				AT91F_US_Printk("\n\r");
				}
				AT91F_PDC_IsNextTxEmpty((AT91PS_PDC)AT91C_BASE_PDC_US1);
				
				//for(i=0;i<BUFFER_SIZE_MCI_DEVICE;i++) 	Buffer[i] = 0x00;	

				AT91F_DBGU_Printk("\n\rTests Completed: !!!\n\r");
	    		//* PDC 1 Frame	
				//*AT91F_DBGU_Printk("Send Frame to US1\n\r"); 

				break;

			case '1': //* info
				//sprintf((char*) message,"AT91C_US1_CSR  0x%08X \n\r",(unsigned int) *AT91C_US1_CSR);
				sprintf((char *)message, "the following will send thru USART1");
				AT91F_DBGU_Printk((char*)message);
				break;
			case '2': //* Print in pooling
				//* PDC 1 Frame
				AT91F_US_SendFrame((AT91PS_USART) AT91C_BASE_DBGU, 
				"Write in US1\n\r",sizeof("Write in US1\n\r"),0,0);
				AT91F_US_Printk("Test\n\r");
				break;
			case '3': //* Print in PDC
				//* PDC 1 Frame
				AT91F_US_SendFrame((AT91PS_USART) AT91C_BASE_DBGU, 
				"Send Frame US1\n\r",sizeof("Send Frame US1\n\r"),0,0);
				AT91F_US_Print_frame("Send Frame US1\n\r",sizeof("Send Frame US1\n\r"));
				break;
			case '4': //* Print in PDC
				//* PDC 1 Frame
				AT91F_US_SendFrame((AT91PS_USART) AT91C_BASE_DBGU, 
				"Send Frame US1\n\r",sizeof("Send Frame US1\n\r"),0,0);
				//* PDC  2 Frame
				AT91F_US_Print_2_frame((char *)PDC1_header,sizeof(PDC1_header),(char *)PDC2_header,sizeof(PDC2_header));
				break;
			case '5': //* Send Block 
				pt_char =  AT91DKC_SDRAM_BASE_ADDRESS;
				//sprintf((char*) message,"init Block :0X%08X\n\r",pt_char);
    	        AT91F_DBGU_Printk((char*)message);
				for (i=0; i < BLOCK_SIZE;i++)    // 10240 bytes - 10 kilobytes
				{
				*pt_char='0';
				pt_char++;
				}
				pt_char = AT91DKC_SDRAM_BASE_ADDRESS;
    	        AT91F_DBGU_Printk("send \n\r");
				//sprintf((char*) message,"send  from 0X%08X size %d bytes\n\r",pt_char,BLOCK_SIZE);
    	        AT91F_DBGU_Printk((char*)message);
				AT91F_US_Print_frame(pt_char,BLOCK_SIZE);	
				break;
			case '6': //* Send Block 
				pt_char = AT91DKC_SDRAM_BASE_ADDRESS;
				//sprintf((char*) message,"init Block :0X%08X\n\r",pt_char);
    	        AT91F_DBGU_Printk((char*)message);
				for (i=0; i < (124*1024)/16;i++)
				{
					for (j=0; j <16;j++)
						{
						*pt_char='0'+j; pt_char++;
						}	
				}
				pt_char = AT91DKC_SDRAM_BASE_ADDRESS;
    	        AT91F_DBGU_Printk("send 120K bytes on USART 1 \n\r");
    	        //* for transfert 124 K byte In first transfert (64Kbyte - 1 ) PDC max size end next at: 60 Kbyte +1 
				AT91F_US_Print_2_frame( AT91DKC_SDRAM_BASE_ADDRESS,(64*1024)-1,AT91DKC_SDRAM_BASE_ADDRESS+(64*1024)-1,(60*1024)+1);
				break;
			default:
				AT91F_DBGU_Printk("\n\rBad choice, Retry please\n\r");
				break;		
		}
	}
return 0;
}

#endif
