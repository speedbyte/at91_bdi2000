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
//* Creation            : FB   21/11/2002
//*
//*----------------------------------------------------------------------------
#include <stdio.h>

#include "AT91C_MCI_Device.h"
#include "part.h"
#include <stdlib.h>
#include <string.h>
#define debug(fmt,args...) do {} while(0)
//#define debug(fmt,args...)	printf (fmt ,##args) 
#define uart 1
extern int fat_register_device(block_dev_desc_t *dev_desc, int part_no);
/* Local variables */
static block_dev_desc_t mmc_dev; 
//static	ulong	base_address = 0; 
static int mmc_ready = 0;
block_dev_desc_t * mmc_get_dev(int dev)
{
	return ((block_dev_desc_t *)&mmc_dev);
}


#define AT91C_MCI_TIMEOUT			1000000   /* For AT91F_MCIDeviceWaitReady */
#define BUFFER_SIZE_MCI_DEVICE		512
#define CFG_MMC_BLOCKSIZE			512
#define CFG_MMC_BASE				0xFFFB4000
#define MMC_BLOCK_SIZE				512
#define MASTER_CLOCK				60000000
#define FALSE						-1
#define TRUE						1
AT91PS_USART pUSART_vikas; 
//* External Functions
extern void 					AT91F_DBGU_Printk(char *);
extern void 					AT91F_ASM_MCI_Handler(void);
extern void 					AT91F_MCI_Device_Handler(AT91PS_MciDevice,unsigned int);
extern AT91S_MCIDeviceStatus 	AT91F_MCI_SDCard_Init (AT91PS_MciDevice);
extern AT91S_MCIDeviceStatus	AT91F_MCI_MMC_Init(AT91PS_MciDevice);
extern AT91S_MCIDeviceStatus 	AT91F_MCI_SetBlocklength(AT91PS_MciDevice,unsigned int);
extern AT91S_MCIDeviceStatus 	AT91F_MCI_MMC_SelectCard(AT91PS_MciDevice,unsigned int);
extern AT91S_MCIDeviceStatus 	AT91F_MCI_ReadBlock(AT91PS_MciDevice,int,unsigned int *,int);
extern AT91S_MCIDeviceStatus 	AT91F_MCI_WriteBlock(AT91PS_MciDevice,int,unsigned int *,int);

#ifdef uart
extern void Usart_init (void);
extern void AT91F_US_Print_frame(char *buffer, unsigned short counter); // \arg pointer to a string ending by \0
#endif

/* Local Functions */
void AT91F_MCIDeviceWaitReady	(unsigned int timeout);
void AT91F_MCI_CorrectByteOrder(uchar *pBuffer, int len);


//* Global Variables
AT91S_MciDeviceFeatures			MCI_Device_Features;
AT91S_MciDeviceDesc				MCI_Device_Desc;
AT91S_MciDevice					MCI_Device;
unsigned char					mmc_buf[BUFFER_SIZE_MCI_DEVICE];

static ulong mmc_bread(int dev_num, ulong blknr, ulong blkcnt, ulong *dst);

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCIDeviceWaitReady
//* \brief Wait for MCI Device ready
//*----------------------------------------------------------------------------
void AT91F_MCIDeviceWaitReady(unsigned int timeout)
{
	volatile unsigned int status;
	
	do
	{
		status = AT91C_BASE_MCI->MCI_SR;
		timeout--;
	}
	while( !(status & AT91C_MCI_NOTBUSY)  && (timeout>0) );	
}


//*----------------------------------------------------------------------------
//* \fn    AT91F_Test
//* \brief Test Functions
//*----------------------------------------------------------------------------
/*int AT91F_Test(void)
{
	int i;
	unsigned int Max_Read_DataBlock_Length;
		
	Max_Read_DataBlock_Length = MCI_Device.pMCI_DeviceFeatures->Max_Read_DataBlock_Length;
	
	//* ReadBlock & WriteBlock Test -> Entire Block

	//* Wait MCI Device Ready
	AT91F_MCIDeviceWaitReady(AT91C_MCI_TIMEOUT);

	//* Read Block 1
	for(i=0;i<BUFFER_SIZE_MCI_DEVICE;i++) 	mmc_buf[i] = 0x00;	
	AT91F_MCI_ReadBlock(&MCI_Device,(1*Max_Read_DataBlock_Length),(unsigned int*) mmc_buf,Max_Read_DataBlock_Length);
	
	//* Wait end of Read
	AT91F_MCIDeviceWaitReady(AT91C_MCI_TIMEOUT);

	//* Write Page 1
    sprintf(mmc_buf,"\n\rThis sentence is written in your device... Congratulations Vikas check Lock\n\r");
	AT91F_MCI_WriteBlock(&MCI_Device,(1*Max_Read_DataBlock_Length),(unsigned int*) mmc_buf,Max_Read_DataBlock_Length);

	//* Wait end of Write
	AT91F_MCIDeviceWaitReady(AT91C_MCI_TIMEOUT);

	//* Read Block 1
	for(i=0;i<BUFFER_SIZE_MCI_DEVICE;i++) 	mmc_buf[i] = 0x00;	
	AT91F_MCI_ReadBlock(&MCI_Device,(1*Max_Read_DataBlock_Length),(unsigned int*) mmc_buf,Max_Read_DataBlock_Length);

	//* Wait end of Read
	AT91F_MCIDeviceWaitReady(AT91C_MCI_TIMEOUT);

	//* End Of Test
	AT91F_DBGU_Printk("\n\rTests Completed: !!!\n\r");
	AT91F_DBGU_Printk(mmc_buf);

	return TRUE;
}*/


//*----------------------------------------------------------------------------
//* \fn    AT91F_CfgDevice
//* \brief This function is used to initialise MMC or SDCard Features
//*----------------------------------------------------------------------------
void AT91F_CfgDevice(void)
{
	// Init Device Structure

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

}

//*----------------------------------------------------------------------------
//* \fn    AT91F_Test_MMC
//* \brief Configure MCI for MMC and complete MMC init, then jump to Test Functions
//*----------------------------------------------------------------------------
int AT91F_Test_MMC(void)
{
	//////////////////////////////////////////////////////////
	//* For MMC Init
	//////////////////////////////////////////////////////////
	
	AT91F_MCI_Configure(AT91C_BASE_MCI,
						AT91C_MCI_DTOR_1MEGA_CYCLES,
						0x834A,						// 400kHz for MCK = 60MHz
						AT91C_MCI_MMC_SLOTA);

	//* Wait MCI Device Ready
	AT91F_MCIDeviceWaitReady(AT91C_MCI_TIMEOUT);

	if(AT91F_MCI_MMC_Init(&MCI_Device) != AT91C_INIT_OK)
		return FALSE;

	// Select MMC Card n°1
	if(AT91F_MCI_MMC_SelectCard(&MCI_Device,AT91C_FIRST_RCA) != AT91C_CARD_SELECTED_OK)
		return FALSE;

	// Set Block Length
	if (AT91F_MCI_SetBlocklength(&MCI_Device,MCI_Device.pMCI_DeviceFeatures[0].Max_Read_DataBlock_Length) != AT91C_CMD_SEND_OK)
		return FALSE;

	AT91F_DBGU_Printk("\n\rMMC Initialisation Successful:\n\r");

	// Enter Main Tests	
	return 0; //return(AT91F_Test());
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_Init_SDCard
//* \brief Configure MCI for SDCard and complete SDCard init, then jump to Test Functions
//*----------------------------------------------------------------------------
int AT91F_Init_SDCard(void)
{
	//////////////////////////////////////////////////////////
	//* For SDCard Init
	//////////////////////////////////////////////////////////

	AT91F_MCI_Configure(AT91C_BASE_MCI,
						AT91C_MCI_DTOR_1MEGA_CYCLES,
						AT91C_MCI_MR_PDCMODE,			// 15MHz for MCK = 60MHz (CLKDIV = 1)
						AT91C_MCI_SDCARD_4BITS_SLOTA);
	
	if(AT91F_MCI_SDCard_Init(&MCI_Device) != AT91C_INIT_OK)
		return FALSE;

	// Enter Main Tests	
	//return(AT91F_Test());
	return(TRUE);
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
//* \fn    mmc_block_read
//* \brief calls the AT91 APIs for read
//*----------------------------------------------------------------------------
int mmc_block_read(uchar *dst, ulong src, ulong len)
{
	unsigned int Max_Read_DataBlock_Length;
	Max_Read_DataBlock_Length = MCI_Device.pMCI_DeviceFeatures->Max_Read_DataBlock_Length;

	AT91S_MCIDeviceStatus status;
	if (len > CFG_MMC_BLOCKSIZE) {
		printf ("mmc_block_read: Error len (%lu) > Blocksize \n",len);
		return -1;
	}
	
	//* Read Block 1

	status = AT91F_MCI_ReadBlock(&MCI_Device, src , (unsigned int *)dst,len);
	if (status != AT91C_READ_OK) {
		printf ("mmc_block_read: Error read %d to dst\n", status);
		return -1;
	}
	//* Wait end of Read
	AT91F_MCIDeviceWaitReady(AT91C_MCI_TIMEOUT);


/*	if (dst != mmc_buf) {
		memcpy(dst,mmc_buf,len);
	}*/
	
	//AT91F_MCI_CorrectByteOrder(dst,len);  later
	return 0;
}

//*----------------------------------------------------------------------------
//* \fn    mmc_read
//* \brief called by mmc_bread and do_mem_cp .- copy from source for 512 bytes till destination
//*----------------------------------------------------------------------------
int mmc_read(ulong src, uchar *dst, int size)
{
	ulong end, part_start, part_end, part_len, aligned_start, aligned_end;
	ulong mmc_block_size, mmc_block_address;

	if (size == 0) {
		return 0;
	}

	if (!MCI_Device.pMCI_DeviceFeatures->Max_Read_DataBlock_Length) {
		printf("Please initial the MMC first\n");
		return -1;
	}

	mmc_block_size = CFG_MMC_BLOCKSIZE;
	mmc_block_address = ~(mmc_block_size - 1);

	src -= CFG_MMC_BASE;
	end = src + size;
	part_start = ~mmc_block_address & src;
	part_end = ~mmc_block_address & end;
	aligned_start = mmc_block_address & src;
	aligned_end = mmc_block_address & end;

	/* all block aligned accesses */
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	if (part_start) {
		part_len = mmc_block_size - part_start;
		debug("ps src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read(mmc_buf, aligned_start, mmc_block_size)) < 0) {
			return -1;
		}
		memcpy(dst, mmc_buf+part_start, part_len);
		dst += part_len;
		src += part_len;
	}
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	for (; src < aligned_end; src += mmc_block_size, dst += mmc_block_size) {
		debug("al src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end); 
		if ((mmc_block_read((uchar *)(dst), src, mmc_block_size)) < 0) {
			return -1;
		}
	}
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	if (part_end && src < end) {
		debug("pe src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read(mmc_buf, aligned_end, mmc_block_size)) < 0) {
			return -1;
		}
		memcpy(dst, mmc_buf, part_end);
	}
	return 0;
}

int
/****************************************************/
mmc_block_write(ulong dst, uchar *src, int len)
/****************************************************/
{
	
	if (len > CFG_MMC_BLOCKSIZE) {
		printf ("mmc_block_write: Error len (%d) > Blocksize \n",len);
		return -1;
	}

	AT91F_MCIDeviceWaitReady(AT91C_MCI_TIMEOUT);
	//* Write Page 1
	AT91F_MCI_WriteBlock(&MCI_Device, dst, (unsigned int*) src ,len);
	//* Wait end of Write
	AT91F_MCIDeviceWaitReady(AT91C_MCI_TIMEOUT);

/*	if (dst != mmc_buf) {
		memcpy(dst,mmc_buf,len);
	}*/
	//AT91F_MCI_CorrectByteOrder(dst,len);  later
	return 0;
} 

//*----------------------------------------------------------------------------
//* \fn    mmc_init
//* \brief Device properties for use in migration of Filesystem
//*----------------------------------------------------------------------------
int mmc_init(int verbose)

{
	char buffertemp[100];//at91sdcard_init();
	sprintf(buffertemp,"Card found with %u MB size\r\n",MCI_Device.pMCI_DeviceFeatures[0].Memory_Capacity>>20);
	AT91F_DBGU_Printk(buffertemp);
	/* fill in device description */
	mmc_dev.if_type = IF_TYPE_MMC;
	mmc_dev.part_type = PART_TYPE_DOS;
	mmc_dev.dev = 0;
	mmc_dev.lun = 0;
	mmc_dev.type = 0;
	mmc_dev.blksz = MCI_Device.pMCI_DeviceFeatures[0].Max_Read_DataBlock_Length;
	mmc_dev.lba = MCI_Device.pMCI_DeviceFeatures[0].Memory_Capacity >> 9;
	sprintf((char *)mmc_dev.vendor,"Manu %2x%2x%2x SNr %2x%2x%2x\n\r",0,0,0,0,0,0);
	AT91F_DBGU_Printk((char *)mmc_dev.vendor);

	sprintf((char *)mmc_dev.product,"Prod %s\n","unknown");
	AT91F_DBGU_Printk((char *)mmc_dev.product);

	sprintf((char *)mmc_dev.revision,"\rRev %x %x",0, 0);
	AT91F_DBGU_Printk((char *)mmc_dev.revision);

	mmc_dev.removable = 0;
	mmc_dev.block_read = mmc_bread;   //mmc_bread;  // this information will be used for FAT

	//fat_register_device(&mmc_dev,1);
	return 0;
}

int
/****************************************************/
mmc_write(uchar *src, ulong dst, int size)
/****************************************************/
{
	ulong end, part_start, part_end, part_len, aligned_start, aligned_end;
	ulong mmc_block_size, mmc_block_address;

	if (size == 0) {
		return 0;
	}

	if (!mmc_ready) {
		printf("Please initial the MMC first\n");
		return -1;
	}

	mmc_block_size = MMC_BLOCK_SIZE;
	mmc_block_address = ~(mmc_block_size - 1);

	dst -= CFG_MMC_BASE;
	end = dst + size;
	part_start = ~mmc_block_address & dst;
	part_end = ~mmc_block_address & end;
	aligned_start = mmc_block_address & dst;
	aligned_end = mmc_block_address & end;

	/* all block aligned accesses */
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	if (part_start) {
		part_len = mmc_block_size - part_start;
		debug("ps src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		(ulong)src, dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read(mmc_buf, aligned_start, mmc_block_size)) < 0) {
			return -1;
		}
		memcpy(mmc_buf+part_start, src, part_len);
		if ((mmc_block_write(aligned_start, mmc_buf, mmc_block_size)) < 0) {
			return -1;
		}
		dst += part_len;
		src += part_len;
	}
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	for (; dst < aligned_end; src += mmc_block_size, dst += mmc_block_size) {
		debug("al src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_write(dst, (uchar *)src, mmc_block_size)) < 0) {
			return -1;
		}
	}
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	if (part_end && dst < end) {
		debug("pe src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read(mmc_buf, aligned_end, mmc_block_size)) < 0) {
			return -1;
		}
		memcpy(mmc_buf, src, part_end);
		if ((mmc_block_write(aligned_end, mmc_buf, mmc_block_size)) < 0) {
			return -1;
		}
	}
	return 0;
}

//*----------------------------------------------------------------------------
//* \fn    mmc_bread
//* \brief read a block number from a device dev_num
//*----------------------------------------------------------------------------
ulong mmc_bread(int dev_num, ulong blknr, ulong blkcnt, ulong *dst)
{
	int mmc_block_size = CFG_MMC_BLOCKSIZE;
	ulong src = blknr * mmc_block_size + CFG_MMC_BASE;

	mmc_read(src, (uchar *)dst, blkcnt*mmc_block_size);
	return blkcnt;
}

//*----------------------------------------------------------------------------
//* \fn    mmc2info
//* \brief check for the validity of the address range
//*----------------------------------------------------------------------------

int mmc2info(ulong addr)
{
	ulong capacity = MCI_Device.pMCI_DeviceFeatures[0].Memory_Capacity;
	long laddr;

	laddr = addr - (long)AT91C_BASE_MCI;
	if (laddr < 0) {
		printf (" addr lower than AT91C_BASE_MCI\n");
		return 0;
	}
	if (laddr > capacity) {
		printf (" addr higher than AT91C_BASE_MCI + capacity\n");
		return 0;
	}
	printf ("Ok\n");
	return 1;
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

//*----------------------------------------------------------------------------
//* \fn    do_mem_cp 
//* \brief This function copies data starting from source to destination and with count.
//*----------------------------------------------------------------------------
int do_mem_cp ( ulong dst, ulong src, ulong cnt)
{

return 0;
}

//*----------------------------------------------------------------------------
//* Function Name       : MCI_init
//* Object              : MCI initialization 
//* Input Parameters    : none
//* Output Parameters   : TRUE
//*----------------------------------------------------------------------------
void Mci_init(void)
{
    // Set up PIO SDC_TYPE to switch on MMC/SDCard and not DataFlash Card
	AT91F_PIO_CfgOutput(AT91C_BASE_PIOB,AT91C_PIO_PB7);
	AT91F_PIO_SetOutput(AT91C_BASE_PIOB,AT91C_PIO_PB7);
	
	// Init MCI for MMC and SDCard interface
	AT91F_MCI_CfgPIO();	
	AT91F_MCI_CfgPMC();
	AT91F_PDC_Open(AT91C_BASE_PDC_MCI);

    // Disable all the interrupts
    AT91C_BASE_MCI->MCI_IDR = 0xFFFFFFFF;

	// Init MCI Device Structures
	AT91F_CfgDevice();

	// Configure MCI interrupt 
	AT91F_AIC_ConfigureIt(AT91C_BASE_AIC,
						 AT91C_ID_MCI,
						 AT91C_AIC_PRIOR_HIGHEST,
						 AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE,
						 AT91F_ASM_MCI_Handler);

	// Enable MCI interrupt
	AT91F_AIC_EnableIt(AT91C_BASE_AIC,AT91C_ID_MCI);
}

//tutu
void AT91F_US3_CfgPIO_useB (void)
{
	// Configure PIO controllers to periph mode
	AT91F_PIO_CfgPeriph(
		AT91C_BASE_PIOB, // PIO controller base address
		0,
		((unsigned int) AT91C_PIO_PB0)
		); // Peripheral B
}

// tutu end
//*----------------------------------------------------------------------------
//* \fn    main
//* \brief main function
//*----------------------------------------------------------------------------
int main()
{
	unsigned char	caractere;
	// tutu
	AT91F_US3_CfgPIO_useB ();   	
	pUSART_vikas = AT91C_BASE_US3; 	
	pUSART_vikas->US_CR = AT91C_US_RTSEN;  // to disable RTSDIS
	//tutu
	
	// Init MCi
	Mci_init(); 
#ifdef uart   
	//* Init Usart
    Usart_init();
#endif

	while(1)
	{
		AT91F_DBGU_Printk("\n\rTest MCI Device\n\r1:Test MMC\n\r2:Test SDCard\n\r");
		// Wait for User Choice : 1 <=> Enter Test MMC , 2 <=> Enter Test SDCard
		caractere = AT91F_DBGU_getc();

		switch(caractere)
		{
	
			case '1':	
				if(AT91F_Init_SDCard() == TRUE)
					{
					AT91F_DBGU_Printk("Init SD card successful !!!\n\r");
					mmc_ready = 1;
					//mmc_init(1);				// 1 = verbose
					//mmc_bread(0,0,1,(ulong *)mmc_buf);     //  devnr = dummy, blknr = dummy, blkcnt=1 converts to 512 later, dst 
					AT91F_DBGU_Printk("\n\n\rWriting .................\n\r");
					sprintf((char *)mmc_buf,"from mci to usart with love\n\r");
				
					mmc_write(mmc_buf, CFG_MMC_BASE, CFG_MMC_BLOCKSIZE);
					
					sprintf((char *)mmc_buf,"\n\rOverwriting buffer before reading back\n\r");			
					AT91F_DBGU_Printk((char *)mmc_buf);
					AT91F_DBGU_Printk("\n\rReading .................\n\r");
					
					
					mmc_read(CFG_MMC_BASE, mmc_buf, CFG_MMC_BLOCKSIZE); 					

					//* PDC 1 Frame
					//AT91F_US_SendFrame((AT91PS_USART) AT91C_BASE_DBGU,"Send Frame US1\n\r",sizeof("Send Frame US1\n\r"),0,0);
					
					AT91F_US_Print_frame((char *)mmc_buf,sizeof(mmc_buf));  // sizeof("Send Frame US1\n\r")
					//end PDC frame 
					
					//* End Of Test
					AT91F_DBGU_Printk((char *)mmc_buf);
					AT91F_DBGU_Printk("\n\rTests Completed: !!!\n\r");
					}
				else
					AT91F_DBGU_Printk("\n\rTests MCI SDCard Failed !!!\n\r");
				break;
			
			default:
				AT91F_DBGU_Printk("\n\rBad choice, Retry please\n\r");
				break;
		}

	}
}
