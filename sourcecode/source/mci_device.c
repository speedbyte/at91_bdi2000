#ifndef mci_device_c
#define mci_device_c

#include "mci_type.h"
#include "AT91RM9200.h"
#include "lib_AT91RM9200.h"
#include "main.h"               //  structure typedef
#include "isr.h"    		// Interrupt_Handler_MCI_Lowlevel
#include "mci_device.h"         // self include
#include "init.h"				// DBGU_Printk()
#include "errm.h"

//-----------------------------

/* General command API */
uint32 SendCommandtoSD (
	SDCARD_PTR ptr_sdcard,
	unsigned int Cmd,
	unsigned int Arg)
{
	unsigned int	error,status;
    // Send the command
    AT91C_BASE_MCI->MCI_ARGR = Arg;
    AT91C_BASE_MCI->MCI_CMDR = Cmd;
	// The CMDRDY flag in the status register means that the command is still being tested
	do
	{
		status = AT91C_BASE_MCI->MCI_SR;
	}
	while( !(status & AT91C_MCI_CMDRDY) ); // if Status of CMDRDY is not 1, then the pc would rotate here.
    // All responses except for the type R3 are protected by a CRC. CMD1 and ACMD41 come in the picture.
    error = (AT91C_BASE_MCI->MCI_SR) & ERROR_ALL_SR;
	if(error != 0 )
	{
		// if the command is to get OperatingCondition Status the CRC error flag is always present (cf : R3 response)
		if  ((Cmd != ACMD41_SEND_APP_OP_COND) && ( Cmd != CMD1_SEND_OP_COND )) 
			return ((AT91C_BASE_MCI->MCI_SR) & ERROR_ALL_SR);
		else
		{
			if (error != AT91C_MCI_RCRCE)
				return ((AT91C_BASE_MCI->MCI_SR) & ERROR_ALL_SR);
		}
	}
    return SD_CMD_SENDNO_ERROR;
}

/* Specific Command API */

uint32 SendAppCommandtoSD (
	SDCARD_PTR ptr_sdcard,
	unsigned int Cmd_App,
	unsigned int Arg	)
{
	unsigned int status;
	// Send the CMD55 for application specific command
    AT91C_BASE_MCI->MCI_ARGR = (ptr_sdcard->ptr_sdcard_info->Relative_Card_Address << 16 );
    AT91C_BASE_MCI->MCI_CMDR = ACMD55_NEXTCMD_APP_CMD;

	// wait for CMDRDY Status flag to read the response
	do
	{
		status = AT91C_BASE_MCI->MCI_SR;
		//tick++;
	}
	while( !(status & AT91C_MCI_CMDRDY) );//&& (tick<100) );	

    // if an error occurs
    if (((AT91C_BASE_MCI->MCI_SR) & ERROR_ALL_SR) != 0 )
		return ((AT91C_BASE_MCI->MCI_SR) & ERROR_ALL_SR);

    // check if it is a specific command and then send the command
	if ( (Cmd_App && ACMD_ALL) == 0)
		return SD_CMD_SEND_ERROR;

   return( SendCommandtoSD(ptr_sdcard,Cmd_App,Arg) );
}


//* \fn    SDcard_GetStatus_CMD13_R2

uint32 SDcard_GetStatus_CMD13_R2(SDCARD_PTR ptr_sdcard,unsigned int relative_card_address)
{
	if (SendCommandtoSD(ptr_sdcard,
								CMD13_SEND_STATUS,
								relative_card_address <<16) == SD_CMD_SENDNO_ERROR)
    	return (AT91C_BASE_MCI->MCI_RSPR[0]);

    return SD_CMD_SEND_ERROR;
}

//*----------------------------------------------------------------------------
//* \fn    Interrupt_Handler_SDcard_Highlevel
//* \brief MCI C interrupt handler
//*----------------------------------------------------------------------------
void Interrupt_Handler_SDcard_Highlevel(
	SDCARD_PTR ptr_sdcard,
	unsigned int status)
{
	// If End of Tx Buffer Empty interrupt occurred
	if ( status & AT91C_MCI_ENDTX )
    {
		AT91C_BASE_MCI->MCI_IDR = AT91C_MCI_ENDTX;
 		AT91C_BASE_PDC_MCI->PDC_PTCR = AT91C_PDC_TXTDIS;
        DBGU_Printk( "\n\rTM\n\r");	
		ptr_sdcard->ptr_sdcard_desc->state = STATE_SDCARD_IDLE;
	}	// End of if AT91C_MCI_TXBUFF		

    // If End of Rx Buffer Full interrupt occurred
    if ( (status & AT91C_MCI_ENDRX ) && reader)
    {        
       	AT91C_BASE_MCI->MCI_IDR = AT91C_MCI_ENDRX;
 		AT91C_BASE_PDC_MCI->PDC_PTCR = AT91C_PDC_RXTDIS;
		DBGU_Printk( "\n\rRMR\n\r");
		ptr_sdcard->ptr_sdcard_desc->state = STATE_SDCARD_IDLE;
	}	// End of if AT91C_MCI_RXBUFF 

	
    // If End of Rx Buffer Full interrupt occurred
    if (  (status & AT91C_MCI_ENDRX) && writer)
    {        
		AT91C_BASE_MCI->MCI_IDR = AT91C_MCI_ENDRX;
		readytowriteonSD = WRITE_NOW;
		printBuffer = Bufferwechsler;
		AT91C_BASE_PDC_MCI->PDC_RCR = 512;		
		DBGU_Printk( "\n\rRMW\n\r");
		globalj = 0;
		(Bufferwechsler == SDBuffer1) ? (Bufferwechsler = SDBuffer2 ): (Bufferwechsler = SDBuffer1) ; 
		AT91C_BASE_MCI->MCI_IER = AT91C_MCI_ENDRX;
		ptr_sdcard->ptr_sdcard_desc->state = STATE_SDCARD_IDLE;
	}// End of if AT91C_MCI_RXBUFF


}//handler


//*----------------------------------------------------------------------------
//* \fn    SDcard_ReadBlock_CMD17_R1
//* \brief Read an ENTIRE block or PARTIAL block
//*----------------------------------------------------------------------------
uint32 SDcard_ReadBlock_CMD17_R1(
	SDCARD_PTR ptr_sdcard,
	int src,
	unsigned int *dataBuffer,
	int sizeToRead )
{
    ////////////////////////////////////////////////////////////////////////////////////////////
    if(ptr_sdcard->ptr_sdcard_desc->state != STATE_SDCARD_IDLE)
    	return SD_READ_FOUND_ERROR;
    
    if( (SDcard_GetStatus_CMD13_R2(ptr_sdcard,ptr_sdcard->ptr_sdcard_info->Relative_Card_Address) & SDCARD_STATE_SR_READY_FOR_DATA) != SDCARD_STATE_SR_READY_FOR_DATA)
    	return SD_READ_FOUND_ERROR;
    	
    if ( (src + sizeToRead) > ptr_sdcard->ptr_sdcard_info->Memory_Capacity )
		return SD_READ_FOUND_ERROR;

    // If source does not fit a begin of a block
	if ( (src % ptr_sdcard->ptr_sdcard_info->Max_Read_DataBlock_Length) != 0 )
		return SD_READ_FOUND_ERROR;
   
     // Test if the MMC supports Partial Read Block
     // ALWAYS SUPPORTED IN SD Memory Card
     if( (sizeToRead < ptr_sdcard->ptr_sdcard_info->Max_Read_DataBlock_Length) 
    	&& (ptr_sdcard->ptr_sdcard_info->Read_Partial == 0x00) )
   		return SD_READ_FOUND_ERROR;
   		
    if( sizeToRead > ptr_sdcard->ptr_sdcard_info->Max_Read_DataBlock_Length)
   		return SD_READ_FOUND_ERROR;
    ////////////////////////////////////////////////////////////////////////////////////////////
      
    // Init Mode Register
	AT91C_BASE_MCI->MCI_MR |= ((ptr_sdcard->ptr_sdcard_info->Max_Read_DataBlock_Length << 16) | AT91C_MCI_PDCMODE);
	 
    if (sizeToRead %4)
		sizeToRead = (sizeToRead /4)+1;
	else
		sizeToRead = sizeToRead/4; 

	AT91C_BASE_PDC_MCI->PDC_PTCR = (AT91C_PDC_TXTDIS | AT91C_PDC_RXTDIS);
    AT91C_BASE_PDC_MCI->PDC_RPR  = (unsigned int) dataBuffer;
    AT91C_BASE_PDC_MCI->PDC_RCR  = sizeToRead;

	// Send the Read single block command
    if ( SendCommandtoSD(ptr_sdcard, CMD17_READ_SINGLE_BLOCK, src) != SD_CMD_SENDNO_ERROR )
    	return SD_READ_FOUND_ERROR;
//(17 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48 	| AT91C_MCI_TRCMD_START	| AT91C_MCI_TRTYP_BLOCK	| AT91C_MCI_TRDIR	| AT91C_MCI_MAXLAT)
	ptr_sdcard->ptr_sdcard_desc->state = STATE_SDCARD_READ_SINGLE_BLOCK;

	// Enable AT91C_MCI_RXBUFF Interrupt
    AT91C_BASE_MCI->MCI_IER = AT91C_MCI_ENDRX;

	// (PDC) Receiver Transfer Enable
	AT91C_BASE_PDC_MCI->PDC_PTCR = AT91C_PDC_RXTEN;  // here exactly is the data recieved.

	return SD_READ_NO_ERROR;
}

//*----------------------------------------------------------------------------
//* \fn    SDcard_WriteBlock_CMD24_R1
//* \brief  Write an ENTIRE block but not always PARTIAL block !!!
//*----------------------------------------------------------------------------
uint32 SDcard_WriteBlock_CMD24_R1(
	SDCARD_PTR ptr_sdcard,
	int dest,
	unsigned int *dataBuffer,
	int sizeToWrite )
{
	////////////////////////////////////////////////////////////////////////////////////////////
	if( ptr_sdcard->ptr_sdcard_desc->state != STATE_SDCARD_IDLE)
    	return ( SD_WRITE_FOUND_ERROR );
    
    if( (SDcard_GetStatus_CMD13_R2(ptr_sdcard,ptr_sdcard->ptr_sdcard_info->Relative_Card_Address) & SDCARD_STATE_SR_READY_FOR_DATA) != SDCARD_STATE_SR_READY_FOR_DATA)
    	return (  SD_WRITE_FOUND_ERROR);
    	
    if ( (dest + sizeToWrite) > ptr_sdcard->ptr_sdcard_info->Memory_Capacity )
		return (  SD_WRITE_FOUND_ERROR);

    // If source does not fit a begin of a block
	if ( (dest % ptr_sdcard->ptr_sdcard_info->Max_Read_DataBlock_Length) != 0 )
		return (SD_WRITE_FOUND_ERROR);
   
    // Test if the MMC supports Partial Write Block 
    if( (sizeToWrite < ptr_sdcard->ptr_sdcard_info->Max_Write_DataBlock_Length) 
    	&& (ptr_sdcard->ptr_sdcard_info->Write_Partial == 0x00) )
   		return (SD_WRITE_FOUND_ERROR);
   		
   	if( sizeToWrite > ptr_sdcard->ptr_sdcard_info->Max_Write_DataBlock_Length )
   		return (SD_WRITE_FOUND_ERROR);
    ////////////////////////////////////////////////////////////////////////////////////////////
  
    // Init Mode Register
	AT91C_BASE_MCI->MCI_MR |= ((ptr_sdcard->ptr_sdcard_info->Max_Write_DataBlock_Length << 16) | AT91C_MCI_PDCMODE | AT91C_MCI_PDCPADV);
	
	if (sizeToWrite %4)
		sizeToWrite = (sizeToWrite /4)+1;
	else
		sizeToWrite = sizeToWrite/4;
	
	// Init PDC for write sequence
    AT91C_BASE_PDC_MCI->PDC_PTCR = (AT91C_PDC_TXTDIS );  
    AT91C_BASE_PDC_MCI->PDC_TPR = (unsigned int) dataBuffer;
    AT91C_BASE_PDC_MCI->PDC_TCR = sizeToWrite;

	// Send the write single block command
    if ( SendCommandtoSD(ptr_sdcard, CMD24_WRITE_BLOCK, dest) != SD_CMD_SENDNO_ERROR)
    	return (SD_WRITE_FOUND_ERROR  ) ;
//24 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_START	| (AT91CAT91C_MCI_TRTYP_BLOCK 	&  ~(AT91C_MCI_TRDIR))	| AT91C_MCI_MAXLAT
	ptr_sdcard->ptr_sdcard_desc->state = STATE_SDCARD_WRITE_SINGLE_BLOCK;

	// Enable AT91C_MCI_TXBUFE Interrupt
    AT91C_BASE_MCI->MCI_IER = AT91C_MCI_ENDTX;
  
  	// Enables TX for PDC transfert requests
    AT91C_BASE_PDC_MCI->PDC_PTCR = AT91C_PDC_TXTEN; //  here exactly is the data sent !

	return SD_WRITE_NO_ERROR;
}


//*----------------------------------------------------------------------------
//* \fn    SDcard_GetCSD_CMD9_R2
//* \brief Asks to the specified card to send its CSD
//*----------------------------------------------------------------------------
uint32 SDcard_GetCSD_CMD9_R2 (SDCARD_PTR ptr_sdcard, unsigned int relative_card_address , unsigned int * response)
{
 	if(SendCommandtoSD(ptr_sdcard,
								  CMD9_SEND_CSD,
								  (relative_card_address << 16)) != SD_CMD_SENDNO_ERROR)
		return SD_CMD_SEND_ERROR;
	
    response[0] = AT91C_BASE_MCI->MCI_RSPR[0];
   	response[1] = AT91C_BASE_MCI->MCI_RSPR[1];
    response[2] = AT91C_BASE_MCI->MCI_RSPR[2];
    response[3] = AT91C_BASE_MCI->MCI_RSPR[3];
    return SD_CMD_SENDNO_ERROR;
}

//*----------------------------------------------------------------------------
//* \fn    SDcard_SetBlockLength_CMD16_R1
//* \brief Select a block length for all following block commands (R/W)
//*----------------------------------------------------------------------------
uint32 SDcard_SetBlockLength_CMD16_R1(SDCARD_PTR ptr_sdcard,unsigned int length)
{
    return( SendCommandtoSD(ptr_sdcard, CMD16_SET_BLOCKLEN, length) );
}


//*----------------------------------------------------------------------------
//* \fn    SDcard_GetOCR_ACMD41_R3
//* \brief Asks to all cards to send their operations conditions
//*----------------------------------------------------------------------------
uint32 SDcard_GetOCR_ACMD41_R3 (SDCARD_PTR ptr_sdcard)
{
	unsigned int	response =0x0;

	// The RCA to be used for CMD55 in Idle state shall be the card's default RCA=0x0000.
	ptr_sdcard->ptr_sdcard_info->Relative_Card_Address = 0x0;
 	
 	while( (response & SDCARD_POWER_UP_BUSY) != SDCARD_POWER_UP_BUSY )
    {
    	response = SendAppCommandtoSD(ptr_sdcard,
  										ACMD41_SEND_APP_OP_COND,
  										ATMEL_AT91_SDSLOT_VOLTAGE_RANGE);
		if (response != SD_CMD_SENDNO_ERROR)
			return SD_INIT_FOUND_ERROR;
		
		response = AT91C_BASE_MCI->MCI_RSPR[0];
	}
	
	return(AT91C_BASE_MCI->MCI_RSPR[0]);
}

//*----------------------------------------------------------------------------
//* \fn    SDcard_GetCID_CMD2_R2
//* \brief Asks to the SDCard on the chosen slot to send its CID
//*----------------------------------------------------------------------------
uint32 SDcard_GetCID_CMD2_R2 (SDCARD_PTR ptr_sdcard, unsigned int *response)
{
 	if(SendCommandtoSD(ptr_sdcard,
							CMD2_ALL_SEND_CID,
							NO_ARGUMENT_REQUIRED) != SD_CMD_SENDNO_ERROR)
		return SD_CMD_SEND_ERROR;
	
    response[0] = AT91C_BASE_MCI->MCI_RSPR[0];
   	response[1] = AT91C_BASE_MCI->MCI_RSPR[1];
    response[2] = AT91C_BASE_MCI->MCI_RSPR[2];
    response[3] = AT91C_BASE_MCI->MCI_RSPR[3];
    
    return SD_CMD_SENDNO_ERROR;
}

//*----------------------------------------------------------------------------
//* \fn    SDcard_SetBusWidth_CMD7_R1b
//* \brief  Set bus width for SDCard
//*----------------------------------------------------------------------------
uint32 SDcard_SetBusWidth_CMD7_R1b(SDCARD_PTR ptr_sdcard)
{
	volatile int	ret_value;
	char			bus_width;

	do
	{
		ret_value =SDcard_GetStatus_CMD13_R2(ptr_sdcard,ptr_sdcard->ptr_sdcard_info->Relative_Card_Address);
	}
	while((ret_value > 0) && ((ret_value & SDCARD_STATE_SR_READY_FOR_DATA) == 0));

	// Select Card
    SendCommandtoSD(ptr_sdcard,
    						CMD7_SEL_DESEL_CARD,
    						(ptr_sdcard->ptr_sdcard_info->Relative_Card_Address)<<16);

/* The extended DAT lines (DAT1-DAT3) are input on power up. They start to operate as DAT lines after the
SET_BUS_WIDTH command. It is the responsibility of the host designer to connect external pullup resistors to
all data lines even if only DAT0 is to be used. Otherwise, non-expected high current consumption may occur due
to the floating inputs of DAT1 & DAT2 (in case they are not used). 

The extended DAT lines (DAT1-DAT3) are input on power up. They start to operate as DAT lines after the
SET_BUS_WIDTH command. It is the responsibility of the host designer to connect external pullup resistors to
all data lines even if only DAT0 is to be used. Otherwise, non-expected high current consumption may occur due
to the floating inputs of DAT1 & DAT2 (in case they are not used).*/

	// Set bus width for Sdcard
	if(ptr_sdcard->ptr_sdcard_desc->SDCard_bus_width == AT91C_MCI_SCDBUS) // if buswidth == 4 bits
		 	bus_width = ACMD6_BUS_WIDTH_4BIT;
	else	bus_width = ACMD6_BUS_WIDTH_1BIT;

	if (SendAppCommandtoSD(ptr_sdcard,ACMD6_SET_BUS_WIDTH,bus_width) != SD_CMD_SENDNO_ERROR)
		return SD_CMD_SEND_ERROR;

	return SD_CMD_SENDNO_ERROR;
}

//*----------------------------------------------------------------------------
//* \fn    SDDevice_Init_and_Cfg_Registers
//* \brief Return the SDCard initialisation status
//*----------------------------------------------------------------------------
uint32 SDDevice_Init_and_Cfg_Registers (SDCARD_PTR ptr_sdcard)
{
    unsigned int	tab_response[4];
	unsigned int	mult,blocknr;
//-----------------inactive state ------------------
	SendCommandtoSD(ptr_sdcard, CMD0_GO_IDLE_STATE, NO_ARGUMENT_REQUIRED);
// ----------- idle state ------------------
    if(SDcard_GetOCR_ACMD41_R3(ptr_sdcard) == SD_INIT_FOUND_ERROR)
    	return SD_INIT_FOUND_ERROR;
/* Register OCR , 32 bit Register, Operation Condition Register. */
	if (SDcard_GetCID_CMD2_R2(ptr_sdcard,tab_response) == SD_CMD_SENDNO_ERROR) 
/* Register CID width 128 .. Card identification number: individual card number for identification. */
	{
	    ptr_sdcard->ptr_sdcard_info->Card_Inserted = STATE_SDCARD_INSERTED;
		// ----------READY state-------------.
	    if (SendCommandtoSD(ptr_sdcard, CMD3_SET_RELATIVE_ADDR, 0) == SD_CMD_SENDNO_ERROR)
		{
		//------------Standby  state -----------------
/* Register RCA width 16 .. Relative card address: local system address of a card, dynamically
suggested by the card and approved by the host during initialization.  */
			ptr_sdcard->ptr_sdcard_info->Relative_Card_Address = (AT91C_BASE_MCI->MCI_RSPR[0] >> 16);
			if (SDcard_GetCSD_CMD9_R2(ptr_sdcard,ptr_sdcard->ptr_sdcard_info->Relative_Card_Address,tab_response) == SD_CMD_SENDNO_ERROR)
			{
/* Register CSD width 128 .. Card specific data: information about the card operation condition  */
		  		ptr_sdcard->ptr_sdcard_info->Max_Read_DataBlock_Length = 1 << ((tab_response[1] >> R2_CSD_MAXREADBLKLEN_S) & R2_CSD_MAXREADBLKLEN_M );
	 			ptr_sdcard->ptr_sdcard_info->Max_Write_DataBlock_Length =	1 << ((tab_response[3] >> R2_CSD_MAXWRITEBLKLEN_S) & R2_CSD_MAXWRITEBLKLEN_M );
				ptr_sdcard->ptr_sdcard_info->Sector_Size = 1 + ((tab_response[2] >> R2_CSD_v21_SECTSIZE_S) & R2_CSD_v21_SECTSIZE_M );
		  		ptr_sdcard->ptr_sdcard_info->Read_Partial = (tab_response[1] >> R2_CSD_READPARBLK_S) & R2_CSD_READPARBLK_M;
				ptr_sdcard->ptr_sdcard_info->Write_Partial = (tab_response[3] >> R2_CSD_WRITEPARBLK_S) & R2_CSD_WRITEPARBLK_M;
				ptr_sdcard->ptr_sdcard_info->Erase_Block_Enable = (tab_response[3] >> R2_CSD_v21_ERASEBLKEN_S) & R2_CSD_v21_ERASEBLKEN_M;
				ptr_sdcard->ptr_sdcard_info->Read_Block_Misalignment = (tab_response[1] >> R2_CSD_READBLKMIS_S) & R2_CSD_READBLKMIS_M;
				ptr_sdcard->ptr_sdcard_info->Write_Block_Misalignment = (tab_response[1] >> R2_CSD_WRITEBLKMIS_S) & R2_CSD_WRITEBLKMIS_M;

				//// Compute Memory Capacity
					// compute MULT          
/* There are two fields within this CSD that indicate the number of blocks available: C_SIZE and C_SIZE_MULT. You can interpret these two values as mantissa and exponent, where C_SIZE is a 12bit value with an offset of 1, and C_SIZE_MULT is a 3bit value with an offset of 2: 

C_SIZE (Device Size)—computes the card capacity. The memory capacity of the card is computed from the entries C_SIZE, C_SIZE_MULT and READ_BL_LEN as follows:
memory capacity = BLOCKNR * BLOCK_LEN
Where:
BLOCKNR = (C_SIZE+1) * MULT
MULT = 2^(C_SIZE_MULT+2)         -- (C_SIZE_MULT < 8)
BLOCK_LEN = 2^READ_BL_LEN        -- (READ_BL_LEN < 12)
Therefore, the maximum capacity that can be coded is 4096*512*2048=4 GB. For
example, a 4-MB card with BLOCK_LEN = 512 can be coded with C_SIZE_MULT =
0 and C_SIZE = 2047.*/
					mult = 1 << ( ((tab_response[2] >> R2_CSD_CSIZE_MULT_S) & R2_CSD_CSIZE_MULT_M) + 2 );   //2^(C_SIZE_MULT+2)
					// compute MSB of C_SIZE
					blocknr = ((tab_response[1] >> R2_CSD_CSIZE_H_S) & R2_CSD_CSIZE_H_M) << 2; 
					// compute MULT * (LSB of C-SIZE + MSB already computed + 1) = BLOCKNR
					blocknr = mult * ( ( blocknr + ( (tab_response[2] >> R2_CSD_CSIZE_L_S) & R2_CSD_CSIZE_L_M) ) + 1 ); 
																				// 2^(C_SIZE_MULT+2)*(C_SIZE+1)

					ptr_sdcard->ptr_sdcard_info->Memory_Capacity =  ptr_sdcard->ptr_sdcard_info->Max_Read_DataBlock_Length * blocknr;
			  	//// End of Compute Memory Capacity

		  		if( SDcard_SetBusWidth_CMD7_R1b(ptr_sdcard) == SD_CMD_SENDNO_ERROR )
				{	
					 if (SDcard_SetBlockLength_CMD16_R1(ptr_sdcard,ptr_sdcard->ptr_sdcard_info->Max_Read_DataBlock_Length) == SD_CMD_SENDNO_ERROR)
					return SD_INIT_NO_ERROR;
				}
			}
		}
	}
    return SD_INIT_FOUND_ERROR;
}


//porting starts

//*----------------------------------------------------------------------------
//* \fn    SDcard_Poll_AT91CMCINOTBUSY_flag
//* \brief Wait for MCI Device ready
//*----------------------------------------------------------------------------
void SDcard_Poll_AT91CMCINOTBUSY_flag(unsigned int timeout)
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
	AT91F_PIO_SetOutput(AT91C_BASE_PIOB,AT91C_PIO_PB7);     // This register is used to set PIO output data. It affects the pin only if the corresponding PIO output line is enabled and if the pin is controlled by the PIO. Otherwise, the information is stored.
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
						 Interrupt_Handler_MCI_Lowlevel);

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
