#ifndef main_h
#define main_h

extern void AT91F_MCI_Handler(void);  // to isr_mci.s
extern int main(void);     // to cstartup.s
extern char AT91F_DBGU_getc(void);
/*-----------------------------------------------*/
/* SDCard Device Descriptor Structure Definition */
/*-----------------------------------------------*/
typedef struct	_AT91S_MciDeviceDesc
{
    volatile unsigned char	state;
	unsigned char			SDCard_bus_width;

} AT91S_MciDeviceDesc, *AT91PS_MciDeviceDesc;

/*---------------------------------------------*/
/* MMC & SDCard Structure Device Features	   */
/*---------------------------------------------*/
typedef struct	_AT91S_MciDeviceFeatures
{
    unsigned char	Card_Inserted;				// (0=AT91C_CARD_REMOVED) (1=AT91C_MMC_CARD_INSERTED) (2=AT91C_SD_CARD_INSERTED)
    unsigned int 	Relative_Card_Address;		// RCA
	unsigned int 	Max_Read_DataBlock_Length;	// 2^(READ_BL_LEN) in CSD 
	unsigned int 	Max_Write_DataBlock_Length;	// 2^(WRITE_BL_LEN) in CSD
	unsigned char	Read_Partial;				// READ_BL_PARTIAL
	unsigned char	Write_Partial;				// WRITE_BL_PARTIAL
	unsigned char	Erase_Block_Enable;			// ERASE_BLK_EN
	unsigned char	Read_Block_Misalignment;	// READ_BLK_MISALIGN
	unsigned char	Write_Block_Misalignment;	// WRITE_BLK_MISALIGN
	unsigned char	Sector_Size;				// SECTOR_SIZE
	unsigned int	Memory_Capacity;			// Size in bits of the device
	
}	AT91S_MciDeviceFeatures, *AT91PS_MciDeviceFeatures ;

/*---------------------------------------------*/
/* MCI Device Structure Definition 			   */
/*---------------------------------------------*/
typedef struct _AT91S_MciDevice
{
	AT91PS_MciDeviceDesc		 	pMCI_DeviceDesc;	// MCI device descriptor
	AT91PS_MciDeviceFeatures		pMCI_DeviceFeatures;// Pointer on a MCI device features array  
}AT91S_MciDevice, *AT91PS_MciDevice;

/*-----------------------------------------------
-------------------------------------------------*/

#define AT91C_MCI_TIMEOUT			1000000   /* For AT91F_MCIDeviceWaitReady */
#define BUFFER_SIZE_MCI_DEVICE		512
#define MASTER_CLOCK				60000000
#define 	AT91DKC_SDRAM_BASE_ADDRESS	(( char *)(0x20100000))
#define 	BLOCK_SIZE	(1024 * 10)

#endif
