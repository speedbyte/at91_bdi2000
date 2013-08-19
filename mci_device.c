#ifndef mci_device_c
#define mci_device_c

#include "mci_type.h"
#include "main.h"               //  structure typedef
#include "AT91RM9200.h"
#include "lib_AT91RM9200.h"
#include "isr_mci.h"    		// AT91F_ASM_MCI_Handler
#include "mci_device.h"         // self include
#include "init.h"				// AT91F_DBGU_Printk()

#ifndef mci_device_c
/*Functions*/
AT91S_MCIDeviceStatus AT91F_MCI_SendCommand (	AT91PS_MciDevice ,	unsigned int ,	unsigned int );
AT91S_MCIDeviceStatus AT91F_MCI_SDCard_SendAppCommand (AT91PS_MciDevice ,	unsigned int ,	unsigned int );
AT91S_MCIDeviceStatus AT91F_MCI_GetStatus(AT91PS_MciDevice pMCI_Device, unsigned int relative_card_address);
extern void AT91F_MCI_Device_Handler(	AT91PS_MciDevice pMCI_Device,	unsigned int status);
extern AT91S_MCIDeviceStatus AT91F_MCI_ReadBlock(	AT91PS_MciDevice pMCI_Device,	int src,	unsigned int *dataBuffer,	int sizeToRead );
extern AT91S_MCIDeviceStatus AT91F_MCI_WriteBlock(	AT91PS_MciDevice pMCI_Device,	int dest,	unsigned int *dataBuffer,	int sizeToWrite );
AT91S_MCIDeviceStatus AT91F_MCI_MMC_SelectCard(AT91PS_MciDevice pMCI_Device, unsigned int relative_card_address);
AT91S_MCIDeviceStatus AT91F_MCI_GetCSD (AT91PS_MciDevice pMCI_Device, unsigned int relative_card_address , unsigned int * response);
AT91S_MCIDeviceStatus AT91F_MCI_SetBlocklength(AT91PS_MciDevice pMCI_Device,unsigned int length);
AT91S_MCIDeviceStatus AT91F_MCI_MMC_GetAllOCR (AT91PS_MciDevice pMCI_Device);
AT91S_MCIDeviceStatus AT91F_MCI_MMC_GetAllCID (AT91PS_MciDevice pMCI_Device, unsigned int *response);
AT91S_MCIDeviceStatus AT91F_MCI_MMC_Init (AT91PS_MciDevice pMCI_Device);
AT91S_MCIDeviceStatus AT91F_MCI_SDCard_GetOCR (AT91PS_MciDevice pMCI_Device);
AT91S_MCIDeviceStatus AT91F_MCI_SDCard_GetCID (AT91PS_MciDevice pMCI_Device, unsigned int *response);
AT91S_MCIDeviceStatus AT91F_MCI_SDCard_SetBusWidth(AT91PS_MciDevice pMCI_Device);
extern AT91S_MCIDeviceStatus AT91F_MCI_SDCard_Init (AT91PS_MciDevice pMCI_Device);
extern void AT91F_MCI_DeviceWaitReady(unsigned int timeout);
extern int Mci_init(void);
#endif
//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_SendCommand
//* \brief Generic function to send a command to the MMC or SDCard
//*----------------------------------------------------------------------------
AT91S_MCIDeviceStatus AT91F_MCI_SendCommand (
	AT91PS_MciDevice pMCI_Device,
	unsigned int Cmd,
	unsigned int Arg)
{
	unsigned int	error,status;
	//unsigned int	tick=0;

    // Send the command
    AT91C_BASE_MCI->MCI_ARGR = Arg;
    AT91C_BASE_MCI->MCI_CMDR = Cmd;

	// wait for CMDRDY Status flag to read the response
	do
	{
		status = AT91C_BASE_MCI->MCI_SR;
		//tick++;
	}
	while( !(status & AT91C_MCI_CMDRDY) );//&& (tick<100) );

    // Test error  ==> if crc error and response R3 ==> don't check error
    error = (AT91C_BASE_MCI->MCI_SR) & AT91C_MCI_SR_ERROR;
	if(error != 0 )
	{
		// if the command is SEND_OP_COND the CRC error flag is always present (cf : R3 response)
		if ( (Cmd != AT91C_SDCARD_APP_OP_COND_CMD) && (Cmd != AT91C_MMC_SEND_OP_COND_CMD) )
			return ((AT91C_BASE_MCI->MCI_SR) & AT91C_MCI_SR_ERROR);
		else
		{
			if (error != AT91C_MCI_RCRCE)
				return ((AT91C_BASE_MCI->MCI_SR) & AT91C_MCI_SR_ERROR);
		}
	}
    return AT91C_CMD_SEND_OK;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_SDCard_SendAppCommand
//* \brief Specific function to send a specific command to the SDCard
//*----------------------------------------------------------------------------
AT91S_MCIDeviceStatus AT91F_MCI_SDCard_SendAppCommand (
	AT91PS_MciDevice pMCI_Device,
	unsigned int Cmd_App,
	unsigned int Arg	)
{
	unsigned int status;
	//unsigned int	tick=0;

	// Send the CMD55 for application specific command
    AT91C_BASE_MCI->MCI_ARGR = (pMCI_Device->pMCI_DeviceFeatures->Relative_Card_Address << 16 );
    AT91C_BASE_MCI->MCI_CMDR = AT91C_APP_CMD;

	// wait for CMDRDY Status flag to read the response
	do
	{
		status = AT91C_BASE_MCI->MCI_SR;
		//tick++;
	}
	while( !(status & AT91C_MCI_CMDRDY) );//&& (tick<100) );	

    // if an error occurs
    if (((AT91C_BASE_MCI->MCI_SR) & AT91C_MCI_SR_ERROR) != 0 )
		return ((AT91C_BASE_MCI->MCI_SR) & AT91C_MCI_SR_ERROR);

    // check if it is a specific command and then send the command
	if ( (Cmd_App && AT91C_SDCARD_APP_ALL_CMD) == 0)
		return AT91C_CMD_SEND_ERROR;

   return( AT91F_MCI_SendCommand(pMCI_Device,Cmd_App,Arg) );
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_GetStatus
//* \brief Addressed card sends its status register
//*----------------------------------------------------------------------------
AT91S_MCIDeviceStatus AT91F_MCI_GetStatus(AT91PS_MciDevice pMCI_Device,unsigned int relative_card_address)
{
	if (AT91F_MCI_SendCommand(pMCI_Device,
								AT91C_SEND_STATUS_CMD,
								relative_card_address <<16) == AT91C_CMD_SEND_OK)
    	return (AT91C_BASE_MCI->MCI_RSPR[0]);

    return AT91C_CMD_SEND_ERROR;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_Device_Handler
//* \brief MCI C interrupt handler
//*----------------------------------------------------------------------------
void AT91F_MCI_Device_Handler(
	AT91PS_MciDevice pMCI_Device,
	unsigned int status)
{
	// If End of Tx Buffer Empty interrupt occurred
	if ( status & AT91C_MCI_TXBUFE )
    {
		AT91C_BASE_MCI->MCI_IDR = AT91C_MCI_TXBUFE;
 		AT91C_BASE_PDC_MCI->PDC_PTCR = AT91C_PDC_TXTDIS;
        AT91F_DBGU_Printk("\n\rTXBUFE\n\r");	
		pMCI_Device->pMCI_DeviceDesc->state = AT91C_MCI_IDLE;
	}	// End of if AT91C_MCI_TXBUFF		
	
    // If End of Rx Buffer Full interrupt occurred
    if ( status & AT91C_MCI_RXBUFF )
    {        
       	AT91C_BASE_MCI->MCI_IDR = AT91C_MCI_RXBUFF;
 		AT91C_BASE_PDC_MCI->PDC_PTCR = AT91C_PDC_RXTDIS;
		AT91F_DBGU_Printk("\n\rRXBUFE\n\r");
		pMCI_Device->pMCI_DeviceDesc->state = AT91C_MCI_IDLE;
	}	// End of if AT91C_MCI_RXBUFF

}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_ReadBlock
//* \brief Read an ENTIRE block or PARTIAL block
//*----------------------------------------------------------------------------
AT91S_MCIDeviceStatus AT91F_MCI_ReadBlock(
	AT91PS_MciDevice pMCI_Device,
	int src,
	unsigned int *dataBuffer,
	int sizeToRead )
{
    ////////////////////////////////////////////////////////////////////////////////////////////
    if(pMCI_Device->pMCI_DeviceDesc->state != AT91C_MCI_IDLE)
    	{AT91F_DBGU_Printk("\n\r1\n\r"); return AT91C_READ_ERROR;}
    
    if( (AT91F_MCI_GetStatus(pMCI_Device,pMCI_Device->pMCI_DeviceFeatures->Relative_Card_Address) & AT91C_SR_READY_FOR_DATA) != AT91C_SR_READY_FOR_DATA)
    	{AT91F_DBGU_Printk("\n\r2 CMD Read not OK\n\r"); return AT91C_READ_ERROR;}
    	
    if ( (src + sizeToRead) > pMCI_Device->pMCI_DeviceFeatures->Memory_Capacity )
		{AT91F_DBGU_Printk("\n\r3 CMD Read not OK\n\r"); return AT91C_READ_ERROR;}

    // If source does not fit a begin of a block
	if ( (src % pMCI_Device->pMCI_DeviceFeatures->Max_Read_DataBlock_Length) != 0 )
		{AT91F_DBGU_Printk("\n\r4 CMD Read not OK\n\r"); return AT91C_READ_ERROR;}
   
     // Test if the MMC supports Partial Read Block
     // ALWAYS SUPPORTED IN SD Memory Card
     if( (sizeToRead < pMCI_Device->pMCI_DeviceFeatures->Max_Read_DataBlock_Length) 
    	&& (pMCI_Device->pMCI_DeviceFeatures->Read_Partial == 0x00) )
   		{AT91F_DBGU_Printk("\n\r5 CMD Read not OK\n\r"); return AT91C_READ_ERROR;}
   		
    if( sizeToRead > pMCI_Device->pMCI_DeviceFeatures->Max_Read_DataBlock_Length)
   		{AT91F_DBGU_Printk("\n\r6 CMD Read not OK\n\r"); return AT91C_READ_ERROR;}
    ////////////////////////////////////////////////////////////////////////////////////////////
      
    // Init Mode Register
	AT91C_BASE_MCI->MCI_MR |= ((pMCI_Device->pMCI_DeviceFeatures->Max_Read_DataBlock_Length << 16) | AT91C_MCI_PDCMODE);
	 
    if (sizeToRead %4)
		sizeToRead = (sizeToRead /4)+1;
	else
		sizeToRead = sizeToRead/4; 

	AT91C_BASE_PDC_MCI->PDC_PTCR = (AT91C_PDC_TXTDIS | AT91C_PDC_RXTDIS);
    AT91C_BASE_PDC_MCI->PDC_RPR  = (unsigned int) dataBuffer;
    AT91C_BASE_PDC_MCI->PDC_RCR  = sizeToRead;

	// Send the Read single block command
    if ( AT91F_MCI_SendCommand(pMCI_Device, AT91C_READ_SINGLE_BLOCK_CMD, src) != AT91C_CMD_SEND_OK )
    	{AT91F_DBGU_Printk("\n\r7 CMD Read not OK\n\r"); return AT91C_READ_ERROR;}
//(17 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48 	| AT91C_MCI_TRCMD_START	| AT91C_MCI_TRTYP_BLOCK	| AT91C_MCI_TRDIR	| AT91C_MCI_MAXLAT)
	pMCI_Device->pMCI_DeviceDesc->state = AT91C_MCI_RX_SINGLE_BLOCK;

	// Enable AT91C_MCI_RXBUFF Interrupt
    AT91C_BASE_MCI->MCI_IER = AT91C_MCI_RXBUFF;

	// (PDC) Receiver Transfer Enable
	AT91C_BASE_PDC_MCI->PDC_PTCR = AT91C_PDC_RXTEN;  // here exactly is the data recieved.

	return AT91C_READ_OK;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_WriteBlock
//* \brief  Write an ENTIRE block but not always PARTIAL block !!!
//*----------------------------------------------------------------------------
AT91S_MCIDeviceStatus AT91F_MCI_WriteBlock(
	AT91PS_MciDevice pMCI_Device,
	int dest,
	unsigned int *dataBuffer,
	int sizeToWrite )
{
	////////////////////////////////////////////////////////////////////////////////////////////
	if( pMCI_Device->pMCI_DeviceDesc->state != AT91C_MCI_IDLE)
    	return AT91C_WRITE_ERROR;
    
    if( (AT91F_MCI_GetStatus(pMCI_Device,pMCI_Device->pMCI_DeviceFeatures->Relative_Card_Address) & AT91C_SR_READY_FOR_DATA) != AT91C_SR_READY_FOR_DATA)
    	return AT91C_WRITE_ERROR;
    	
    if ( (dest + sizeToWrite) > pMCI_Device->pMCI_DeviceFeatures->Memory_Capacity )
		return AT91C_WRITE_ERROR;

    // If source does not fit a begin of a block
	if ( (dest % pMCI_Device->pMCI_DeviceFeatures->Max_Read_DataBlock_Length) != 0 )
		return AT91C_WRITE_ERROR;
   
    // Test if the MMC supports Partial Write Block 
    if( (sizeToWrite < pMCI_Device->pMCI_DeviceFeatures->Max_Write_DataBlock_Length) 
    	&& (pMCI_Device->pMCI_DeviceFeatures->Write_Partial == 0x00) )
   		return AT91C_WRITE_ERROR;
   		
   	if( sizeToWrite > pMCI_Device->pMCI_DeviceFeatures->Max_Write_DataBlock_Length )
   		return AT91C_WRITE_ERROR;
    ////////////////////////////////////////////////////////////////////////////////////////////
  
    // Init Mode Register
	AT91C_BASE_MCI->MCI_MR |= ((pMCI_Device->pMCI_DeviceFeatures->Max_Write_DataBlock_Length << 16) | AT91C_MCI_PDCMODE | AT91C_MCI_PDCPADV);
	
	if (sizeToWrite %4)
		sizeToWrite = (sizeToWrite /4)+1;
	else
		sizeToWrite = sizeToWrite/4;
	
	// Init PDC for write sequence
    AT91C_BASE_PDC_MCI->PDC_PTCR = (AT91C_PDC_TXTDIS | AT91C_PDC_RXTDIS);  
    AT91C_BASE_PDC_MCI->PDC_TPR = (unsigned int) dataBuffer;
    AT91C_BASE_PDC_MCI->PDC_TCR = sizeToWrite;

	// Send the write single block command
    if ( AT91F_MCI_SendCommand(pMCI_Device, AT91C_WRITE_BLOCK_CMD, dest) != AT91C_CMD_SEND_OK)
    	return AT91C_WRITE_ERROR;
//24 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_START	| (AT91C_MCI_TRTYP_BLOCK 	&  ~(AT91C_MCI_TRDIR))	| AT91C_MCI_MAXLAT
	pMCI_Device->pMCI_DeviceDesc->state = AT91C_MCI_TX_SINGLE_BLOCK;

	// Enable AT91C_MCI_TXBUFE Interrupt
    AT91C_BASE_MCI->MCI_IER = AT91C_MCI_TXBUFE;
  
  	// Enables TX for PDC transfert requests
    AT91C_BASE_PDC_MCI->PDC_PTCR = AT91C_PDC_TXTEN; //  here exactly is the data sent !

	return AT91C_WRITE_OK;
}


//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_GetCSD
//* \brief Asks to the specified card to send its CSD
//*----------------------------------------------------------------------------
AT91S_MCIDeviceStatus AT91F_MCI_GetCSD (AT91PS_MciDevice pMCI_Device, unsigned int relative_card_address , unsigned int * response)
{
 	if(AT91F_MCI_SendCommand(pMCI_Device,
								  AT91C_SEND_CSD_CMD,
								  (relative_card_address << 16)) != AT91C_CMD_SEND_OK)
		return AT91C_CMD_SEND_ERROR;
	
    response[0] = AT91C_BASE_MCI->MCI_RSPR[0];
   	response[1] = AT91C_BASE_MCI->MCI_RSPR[1];
    response[2] = AT91C_BASE_MCI->MCI_RSPR[2];
    response[3] = AT91C_BASE_MCI->MCI_RSPR[3];
    return AT91C_CMD_SEND_OK;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_SetBlocklength
//* \brief Select a block length for all following block commands (R/W)
//*----------------------------------------------------------------------------
AT91S_MCIDeviceStatus AT91F_MCI_SetBlocklength(AT91PS_MciDevice pMCI_Device,unsigned int length)
{
    return( AT91F_MCI_SendCommand(pMCI_Device, AT91C_SET_BLOCKLEN_CMD, length) );
}


//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_SDCard_GetOCR
//* \brief Asks to all cards to send their operations conditions
//*----------------------------------------------------------------------------
AT91S_MCIDeviceStatus AT91F_MCI_SDCard_GetOCR (AT91PS_MciDevice pMCI_Device)
{
	unsigned int	response =0x0;

	// The RCA to be used for CMD55 in Idle state shall be the card's default RCA=0x0000.
	pMCI_Device->pMCI_DeviceFeatures->Relative_Card_Address = 0x0;
 	
 	while( (response & AT91C_CARD_POWER_UP_BUSY) != AT91C_CARD_POWER_UP_BUSY )
    {
    	response = AT91F_MCI_SDCard_SendAppCommand(pMCI_Device,
  										AT91C_SDCARD_APP_OP_COND_CMD,
  										AT91C_MMC_HOST_VOLTAGE_RANGE);
		if (response != AT91C_CMD_SEND_OK)
			return AT91C_INIT_ERROR;
		
		response = AT91C_BASE_MCI->MCI_RSPR[0];
	}
	
	return(AT91C_BASE_MCI->MCI_RSPR[0]);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_SDCard_GetCID
//* \brief Asks to the SDCard on the chosen slot to send its CID
//*----------------------------------------------------------------------------
AT91S_MCIDeviceStatus AT91F_MCI_SDCard_GetCID (AT91PS_MciDevice pMCI_Device, unsigned int *response)
{
 	if(AT91F_MCI_SendCommand(pMCI_Device,
							AT91C_ALL_SEND_CID_CMD,
							AT91C_NO_ARGUMENT) != AT91C_CMD_SEND_OK)
		return AT91C_CMD_SEND_ERROR;
	
    response[0] = AT91C_BASE_MCI->MCI_RSPR[0];
   	response[1] = AT91C_BASE_MCI->MCI_RSPR[1];
    response[2] = AT91C_BASE_MCI->MCI_RSPR[2];
    response[3] = AT91C_BASE_MCI->MCI_RSPR[3];
    
    return AT91C_CMD_SEND_OK;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_SDCard_SetBusWidth
//* \brief  Set bus width for SDCard
//*----------------------------------------------------------------------------
AT91S_MCIDeviceStatus AT91F_MCI_SDCard_SetBusWidth(AT91PS_MciDevice pMCI_Device)
{
	volatile int	ret_value;
	char			bus_width;

	do
	{
		ret_value =AT91F_MCI_GetStatus(pMCI_Device,pMCI_Device->pMCI_DeviceFeatures->Relative_Card_Address);
	}
	while((ret_value > 0) && ((ret_value & AT91C_SR_READY_FOR_DATA) == 0));

	// Select Card
    AT91F_MCI_SendCommand(pMCI_Device,
    						AT91C_SEL_DESEL_CARD_CMD,
    						(pMCI_Device->pMCI_DeviceFeatures->Relative_Card_Address)<<16);

	// Set bus width for Sdcard
	if(pMCI_Device->pMCI_DeviceDesc->SDCard_bus_width == AT91C_MCI_SCDBUS)
		 	bus_width = AT91C_BUS_WIDTH_4BITS;
	else	bus_width = AT91C_BUS_WIDTH_1BIT;

	if (AT91F_MCI_SDCard_SendAppCommand(pMCI_Device,AT91C_SDCARD_SET_BUS_WIDTH_CMD,bus_width) != AT91C_CMD_SEND_OK)
		return AT91C_CMD_SEND_ERROR;

	return AT91C_CMD_SEND_OK;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_SDCard_Init
//* \brief Return the SDCard initialisation status
//*----------------------------------------------------------------------------
AT91S_MCIDeviceStatus AT91F_MCI_SDCard_Init (AT91PS_MciDevice pMCI_Device)
{
    unsigned int	tab_response[4];
	unsigned int	mult,blocknr;

	AT91F_MCI_SendCommand(pMCI_Device, AT91C_GO_IDLE_STATE_CMD, AT91C_NO_ARGUMENT);

    if(AT91F_MCI_SDCard_GetOCR(pMCI_Device) == AT91C_INIT_ERROR)
    	return AT91C_INIT_ERROR;

	if (AT91F_MCI_SDCard_GetCID(pMCI_Device,tab_response) == AT91C_CMD_SEND_OK)
	{
	    pMCI_Device->pMCI_DeviceFeatures->Card_Inserted = AT91C_SD_CARD_INSERTED;

	    if (AT91F_MCI_SendCommand(pMCI_Device, AT91C_SET_RELATIVE_ADDR_CMD, 0) == AT91C_CMD_SEND_OK)
		{
			pMCI_Device->pMCI_DeviceFeatures->Relative_Card_Address = (AT91C_BASE_MCI->MCI_RSPR[0] >> 16);
			if (AT91F_MCI_GetCSD(pMCI_Device,pMCI_Device->pMCI_DeviceFeatures->Relative_Card_Address,tab_response) == AT91C_CMD_SEND_OK)
			{
		  		pMCI_Device->pMCI_DeviceFeatures->Max_Read_DataBlock_Length = 1 << ((tab_response[1] >> AT91C_CSD_RD_B_LEN_S) & AT91C_CSD_RD_B_LEN_M );
	 			pMCI_Device->pMCI_DeviceFeatures->Max_Write_DataBlock_Length =	1 << ((tab_response[3] >> AT91C_CSD_WBLEN_S) & AT91C_CSD_WBLEN_M );
				pMCI_Device->pMCI_DeviceFeatures->Sector_Size = 1 + ((tab_response[2] >> AT91C_CSD_v21_SECT_SIZE_S) & AT91C_CSD_v21_SECT_SIZE_M );
		  		pMCI_Device->pMCI_DeviceFeatures->Read_Partial = (tab_response[1] >> AT91C_CSD_RD_B_PAR_S) & AT91C_CSD_RD_B_PAR_M;
				pMCI_Device->pMCI_DeviceFeatures->Write_Partial = (tab_response[3] >> AT91C_CSD_WBLOCK_P_S) & AT91C_CSD_WBLOCK_P_M;
				pMCI_Device->pMCI_DeviceFeatures->Erase_Block_Enable = (tab_response[3] >> AT91C_CSD_v21_ER_BLEN_EN_S) & AT91C_CSD_v21_ER_BLEN_EN_M;
				pMCI_Device->pMCI_DeviceFeatures->Read_Block_Misalignment = (tab_response[1] >> AT91C_CSD_RD_B_MIS_S) & AT91C_CSD_RD_B_MIS_M;
				pMCI_Device->pMCI_DeviceFeatures->Write_Block_Misalignment = (tab_response[1] >> AT91C_CSD_WR_B_MIS_S) & AT91C_CSD_WR_B_MIS_M;

				//// Compute Memory Capacity
					// compute MULT
					mult = 1 << ( ((tab_response[2] >> AT91C_CSD_C_SIZE_M_S) & AT91C_CSD_C_SIZE_M_M) + 2 );
					// compute MSB of C_SIZE
					blocknr = ((tab_response[1] >> AT91C_CSD_CSIZE_H_S) & AT91C_CSD_CSIZE_H_M) << 2;
					// compute MULT * (LSB of C-SIZE + MSB already computed + 1) = BLOCKNR
					blocknr = mult * ( ( blocknr + ( (tab_response[2] >> AT91C_CSD_CSIZE_L_S) & AT91C_CSD_CSIZE_L_M) ) + 1 );

					pMCI_Device->pMCI_DeviceFeatures->Memory_Capacity =  pMCI_Device->pMCI_DeviceFeatures->Max_Read_DataBlock_Length * blocknr;
			  	//// End of Compute Memory Capacity

		  		if( AT91F_MCI_SDCard_SetBusWidth(pMCI_Device) == AT91C_CMD_SEND_OK )
				{	
					 if (AT91F_MCI_SetBlocklength(pMCI_Device,pMCI_Device->pMCI_DeviceFeatures->Max_Read_DataBlock_Length) == AT91C_CMD_SEND_OK)
					return AT91C_INIT_OK;
				}
			}
		}
	}
    return AT91C_INIT_ERROR;
}


//porting starts

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_DeviceWaitReady
//* \brief Wait for MCI Device ready
//*----------------------------------------------------------------------------
void AT91F_MCI_DeviceWaitReady(unsigned int timeout)
{
	volatile unsigned int status;
	
	do
	{
		status = AT91C_BASE_MCI->MCI_SR;
		timeout--;
	}
	while( !(status & AT91C_MCI_NOTBUSY)  && (timeout>0) );	 // wait while reading and writing ; the DAT lines are busy
}


//*----------------------------------------------------------------------------
//* Function Name       : MCI_init ( also for SD CArd )
//* Object              : MCI initialization ( also for SD card )
//* Input Parameters    : none
//* Output Parameters   : TRUE
//*----------------------------------------------------------------------------

void Mci_init(void)
{
    // Set up PIO SDC_TYPE to switch on MMC/SDCard and not DataFlash Card
	AT91F_PIO_CfgOutput(AT91C_BASE_PIOB,AT91C_PIO_PB7);     // 	Enable PIO in output mode - Timer Counter 3 Multipurpose Timer I/O Pin B
	AT91F_PIO_SetOutput(AT91C_BASE_PIOB,AT91C_PIO_PB7);     // Set to 1 Output AT91C_PB7_TIOB3 ( both are same ) 1<<7
	
	// Init MCI for MMC and SDCard interface
	AT91F_MCI_CfgPIO();	//Configure PIO to peripheral mode for MCI - both PIOA PA27,PA28,PA29 and PIOB PB3,PB4,PB5
/*Signal	Symbol	PIO controller	Description
MCDA3	(AT91C_PB5_MCDA3 )	PIOB Periph: B Bit: 5	Multimedia Card A Data 3
MCCDA	(AT91C_PA28_MCCDA )	PIOA Periph: A Bit: 28	Multimedia Card A Command
MCDA0	(AT91C_PA29_MCDA0 )	PIOA Periph: A Bit: 29	Multimedia Card A Data 0
MCDA1	(AT91C_PB3_MCDA1 )	PIOB Periph: B Bit: 3	Multimedia Card A Data 1
MCCK	(AT91C_PA27_MCCK )	PIOA Periph: A Bit: 27	Multimedia Card Clock
MCDA2	(AT91C_PB4_MCDA2 )	PIOB Periph: B Bit: 4	Multimedia Card A Data 2     */

	AT91F_MCI_CfgPMC();       // Enable Peripheral Clock for MCI ( ID = 10 )
    // Disable all the interrupts
    AT91C_BASE_MCI->MCI_IDR = 0xFFFFFFFF;       // interrupt disable register; reduntant .. also done in MCI_Configure() . see below
	
	// Configure MCI interrupt 
	AT91F_AIC_ConfigureIt(AT91C_BASE_AIC,
						 AT91C_ID_MCI,
						 AT91C_AIC_PRIOR_HIGHEST,
						 AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE,
						 AT91F_ASM_MCI_Handler);

	// Enable MCI interrupt
	AT91F_AIC_EnableIt(AT91C_BASE_AIC,AT91C_ID_MCI);
	//////////////////////////////////////////////////////////
	//* 	Atmel Fn to Configure the MCI For SDCard Init
	//////////////////////////////////////////////////////////

	AT91F_MCI_Configure(AT91C_BASE_MCI,
						AT91C_MCI_DTOR_1MEGA_CYCLES,
						AT91C_MCI_MR_PDCMODE,			// 15MHz for MCK = 60MHz (CLKDIV = 1)
														/*#define AT91C_MCI_MR_PDCMODE	(	AT91C_MCI_CLKDIV |\
																							AT91C_MCI_PWSDIV |\
																							AT91C_MCI_PWSDIV<<1 |\
																							AT91C_MCI_PDCMODE)  1<<15 bit in MR*/ 
						AT91C_MCI_SDCARD_4BITS_SLOTA);
/*
----------------------------------------------------------------------------
\fn    AT91F_MCI_Configure
\brief Configure the MCI
----------------------------------------------------------------------------
__inline void AT91F_MCI_Configure (
        AT91PS_MCI pMCI,  			 // \arg pointer to a MCI controller
        unsigned int DTOR_register,  // \arg Data Timeout Register to be programmed
        unsigned int MR_register,  	 // \arg Mode Register to be programmed
        unsigned int SDCR_register)  // \arg SDCard Register to be programmed
{
    * Reset the MCI
    pMCI->MCI_CR = AT91C_MCI_MCIEN | AT91C_MCI_PWSEN;

    * Disable all the interrupts
    pMCI->MCI_IDR = 0xFFFFFFFF;

    * Set the Data Timeout Register
    pMCI->MCI_DTOR = DTOR_register;

    * Set the Mode Register
    pMCI->MCI_MR = MR_register;

    * Set the SDCard Register
    pMCI->MCI_SDCR = SDCR_register;
}
*/
}
#endif
