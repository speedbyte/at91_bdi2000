#ifndef main_h
#define main_h

extern void AT91F_MCI_Handler(void);  // to isr.S
extern int main(void);     // to cstartup.S
extern char AT91F_DBGU_getc(void);
extern int readytowriteonSD;
extern char usartBuffer1[512];
extern char usartBuffer2[512];
extern char *Bufferwechsler;
extern char *printBuffer;
extern char mciBuffer1[512];
extern char mciBuffer2[512];
extern int globalj;
extern int reader, writer;
#define ACTIVE 1
// REal time clock : 
union _calendar {
unsigned int cal_data;
struct 
{
unsigned int century : 6;
unsigned int         : 2;
unsigned int year    : 8;
unsigned int month   : 5;
unsigned int day     : 3;
unsigned int date    : 6;
unsigned int         : 2;
} cal_bits;
} ;

typedef union _calendar CALENDAR;

union _time {
unsigned int time_data;
struct 
{
unsigned int second : 7;
unsigned int        : 1;
unsigned int minute : 7;
unsigned int        : 1;
unsigned int hour   : 5;
unsigned int        : 1;  // this is not required but for the sake of uniformity.
unsigned int merid  : 1;
unsigned int        : 9;
} time_bits;
};

typedef union _time TIME;

/*typedef struct _led{
char green : 1;
char yellow: 1;
char red   : 1;
char pad   : 5;
} LED; */

extern CALENDAR rtc_cal;
extern TIME     rtc_time;

#define NOT_ACTIVE 0
#define WRITE_IN_NEXT_CYCLE 1
#define WRITE_NOW 2



/*enum {
NOT_ACTIVE,
WRITE_IN_NEXT_CYCLE,
WRITE_NOW
};*/

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
#define 	AT91DKC_SDRAM_BASE_ADDRESS	(( char *)(0x20100000))
#define 	BLOCK_SIZE	(1024 * 10)

#endif
