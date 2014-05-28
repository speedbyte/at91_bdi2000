#ifndef main_h
#define main_h

extern void 	Interrupt_Handler_MCI_Highlevel(void) ; // to isr.S
extern int 	main(void);     // to cstartup.S
extern int 	readytowriteonSD;
extern char 	usartBuffer1[1024];
extern char 	*Bufferwechsler;
extern char 	*printBuffer;
extern char 	SDBuffer1[512];
extern char 	SDBuffer2[512];
extern int 	globalj, RCR_recirculated;
extern int 	reader, writer;

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
unsigned int hour   : 6;
unsigned int merid  : 1;
unsigned int        : 1;  // this is not required but for the sake of uniformity.
unsigned int        : 8;
} time_bits;
};

typedef union _time TIME;

extern CALENDAR rtc_cal;
extern TIME     rtc_time;

#define NOT_ACTIVE 0
#define WRITE_IN_NEXT_CYCLE 1
#define WRITE_NOW 2

#define ACTIVE 1
#define NOT_ACTIVE 0
/*-----------------------------------------------*/
/* SDCard Device Descriptor Structure Definition */
/*-----------------------------------------------*/
typedef struct	_SDCARD_DESC
{
    volatile unsigned char	state;
	unsigned char			SDCard_bus_width;

} SDCARD_DESC, *SDCARD_DESC_PTR;

/*---------------------------------------------*/
/* MMC & SDCard Structure Device Features	   */
/*---------------------------------------------*/
typedef struct	_SDCARD_INFO
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
	
}	SDCARD_INFO, *SDCARD_INFO_PTR ;

/*---------------------------------------------*/
/* MCI Device Structure Definition 			   */
/*---------------------------------------------*/
typedef struct _SDCARD
{
	SDCARD_DESC_PTR		 	ptr_sdcard_desc;	// MCI device descriptor
	SDCARD_INFO_PTR			ptr_sdcard_info;// Pointer on a MCI device features array  
}SDCARD, *SDCARD_PTR;

/*-----------------------------------------------
-------------------------------------------------*/

#define AT91C_MCI_TIMEOUT			1000000   /* For AT91F_MCIDeviceWaitReady */
#define BUFFER_SIZE_MCI_DEVICE		512
#define 	AT91DKC_SDRAM_BASE_ADDRESS	(( char *)(0x20100000))
#define 	BLOCK_SIZE	(1024 * 10)



#define MY_INT_PIN AT91C_PIO_PB15
#define TIMER0_INTERRUPT_LEVEL		7
#define 	AT91C_AIC_SRCTYPE_INT_POSITIVE_EDGE    ((unsigned int) 0x1 <<  5) // (AIC) Internal Sources Code Label Positive Edge triggered

	#define TIMER_CLOCK1 		0x00     	//MCK/2
	#define TIMER_CLOCK2 		0x01		//MCK/8
	#define TIMER_CLOCK3 		0x02		//MCK/32
	#define TIMER_CLOCK4 		0x03		//MCK/128
	#define TIMER_CLOCK5 		0x04		//SLCK (32768HZ)
	#define INT_COMP_OFF 		0x00		
	#define INT_COMP_ON 		0x01		
#endif
