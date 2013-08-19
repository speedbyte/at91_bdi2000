#ifndef mci_device_h
#define mci_device_h


extern uint32 	SDcard_ReadBlock_CMD17_R1(SDCARD_PTR ,int , uint32 * ,int );
extern uint32 	SDcard_WriteBlock_CMD24_R1(SDCARD_PTR,int,uint32 *,int);
extern void 	SDcard_Poll_AT91CMCINOTBUSY_flag	(uint32 timeout);
extern void 	Interrupt_Handler_SDcard_Highlevel(SDCARD_PTR,uint32);
extern uint32 	SDDevice_Init_and_Cfg_Registers (SDCARD_PTR);
extern void 	Mci_init(void);


/////////////////////////////////////////////////////////////////////////////////////////////////////
#define NO_ARGUMENT_REQUIRED			0x0

#define AT91C_FIRST_RCA				0xCAFE
#define AT91C_MAX_MCI_CARDS			10




/* Driver State */
#define STATE_SDCARD_REMOVED			0
#define STATE_SDCARD_INSERTED			2
#define STATE_SDCARD_IDLE      			0x0
#define STATE_MCI_TIMEOUT_ERROR			0x1
#define STATE_SDCARD_READ_SINGLE_BLOCK	0x2
#define STATE_SDCARD_WRITE_SINGLE_BLOCK	0x5

/* TimeOut */
#define TIMEOUT_CMDRDY					30


/////////////////////////////////////////////////////////////////	
// Class 0 & 1 commands: Basic commands and Read Stream commands
/////////////////////////////////////////////////////////////////

#define CMD0_GO_IDLE_STATE				(0 	| AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_NONE )
#define CMD0_MMC_GO_IDLE_STATE			(0 	| AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_NONE  | AT91C_MCI_OPDCMD)
#define CMD1_SEND_OP_COND				(1	| AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48 | AT91C_MCI_OPDCMD)
#define CMD2_ALL_SEND_CID				(2	| AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_136 )
#define CMD2_MMC_ALL_SEND_CID			(2	| AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_136 | AT91C_MCI_OPDCMD)
#define CMD3_SET_RELATIVE_ADDR			(3	| AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48  | AT91C_MCI_MAXLAT )
#define CMD3_MMC_SET_RELATIVE_ADDR		(3	| AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48  | AT91C_MCI_MAXLAT | AT91C_MCI_OPDCMD)

#define CMD4_SET_DSR					(4	| AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_NO		| AT91C_MCI_MAXLAT )	// no tested

#define CMD7_SEL_DESEL_CARD				(7	| AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48  		| AT91C_MCI_MAXLAT )
#define CMD9_SEND_CSD					(9	| AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_136 		| AT91C_MCI_MAXLAT )
#define CMD10_SEND_CID					(10	| AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_136 		| AT91C_MCI_MAXLAT )
#define CMD11_MMC_READ_DAT_UNTIL_STOP	(11	| AT91C_MCI_TRTYP_STREAM| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRDIR	| AT91C_MCI_TRCMD_START | AT91C_MCI_MAXLAT )

#define CMD12_STOP_TRANSMISSION			(12	| AT91C_MCI_TRCMD_STOP 	| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48 		| AT91C_MCI_MAXLAT )
#define CMD12_STOP_TRANSMISSION_SYNC	(12	| AT91C_MCI_TRCMD_STOP 	| AT91C_MCI_SPCMD_SYNC	| AT91C_MCI_RSPTYP_48 		| AT91C_MCI_MAXLAT )
#define CMD13_SEND_STATUS				(13	| AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48 		| AT91C_MCI_MAXLAT )
#define CMD15_GO_INACTIVE_STATE			(15	| AT91C_MCI_RSPTYP_NO )

//*------------------------------------------------
//* Class 2 commands: Block oriented Read commands
//*------------------------------------------------

#define CMD16_SET_BLOCKLEN				(16 | AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48		| AT91C_MCI_MAXLAT )
#define CMD17_READ_SINGLE_BLOCK			(17 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48 	| AT91C_MCI_TRCMD_START	| AT91C_MCI_TRTYP_BLOCK	| AT91C_MCI_TRDIR	| AT91C_MCI_MAXLAT)
#define CMD18_READ_MULTIPLE_BLOCK		(18 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48 	| AT91C_MCI_TRCMD_START	| AT91C_MCI_TRTYP_MULTIPLE	| AT91C_MCI_TRDIR	| AT91C_MCI_MAXLAT)

//*--------------------------------------------
//* Class 3 commands: Sequential write commands
//*--------------------------------------------

#define CMD_MMC_WRITE_DAT_UNTIL_STOP	(20 | AT91C_MCI_TRTYP_STREAM| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48 & ~(AT91C_MCI_TRDIR) | AT91C_MCI_TRCMD_START | AT91C_MCI_MAXLAT )	// MMC

//*------------------------------------------------
//* Class 4 commands: Block oriented write commands
//*------------------------------------------------
	
#define CMD24_WRITE_BLOCK				(24 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_START	| (AT91C_MCI_TRTYP_BLOCK 	&  ~(AT91C_MCI_TRDIR))	| AT91C_MCI_MAXLAT)
#define CMD25_WRITE_MULTIPLE_BLOCK		(25 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_START	| (AT91C_MCI_TRTYP_MULTIPLE	&  ~(AT91C_MCI_TRDIR)) 	| AT91C_MCI_MAXLAT)
#define CMD27_PROGRAM_CSD				(27 | AT91C_MCI_RSPTYP_48 )


//*----------------------------------------
//* Class 6 commands: Group Write protect
//*----------------------------------------

#define CMD28_SET_WRITE_PROT			(28	| AT91C_MCI_RSPTYP_48 )
#define CMD29_CLR_WRITE_PROT			(29	| AT91C_MCI_RSPTYP_48 )
#define CMD30_SEND_WRITE_PROT			(30	| AT91C_MCI_RSPTYP_48 )


//*----------------------------------------
//* Class 5 commands: Erase commands
//*----------------------------------------

#define CMD32_TAG_SECTOR_START			(32 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_NO	| AT91C_MCI_MAXLAT)
#define CMD33_TAG_SECTOR_END 			(33 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_NO	| AT91C_MCI_MAXLAT)
#define CMD34_MMC_UNTAG_SECTOR			(34 | AT91C_MCI_RSPTYP_48 )
#define CMD35_MMC_TAG_ERASE_GROUP_START	(35 | AT91C_MCI_RSPTYP_48 )
#define CMD36_MMC_TAG_ERASE_GROUP_END	(36 | AT91C_MCI_RSPTYP_48 )
#define CMD37_MMC_UNTAG_ERASE_GROUP		(37 | AT91C_MCI_RSPTYP_48 )
#define CMD38_ERASE						(38 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_NO	| AT91C_MCI_MAXLAT )

//*----------------------------------------
//* Class 7 commands: Lock commands
//*----------------------------------------

#define CMD42_LOCK_UNLOCK				(42 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_NO	| AT91C_MCI_MAXLAT)	// no tested

//*-----------------------------------------------
// Class 8 commands: Application specific commands
//*-----------------------------------------------

#define ACMD55_NEXTCMD_APP_CMD			(55 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_NO | AT91C_MCI_MAXLAT)
#define ACMD56_GEN						(56 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_NO | AT91C_MCI_MAXLAT)	// no tested

/*ACMD6 Defines the data bus width (’00’=1bit or’10’=4 bits bus) to be used for data transfer. The allowed data bus widths are given in SCR register.*/
#define ACMD6_SET_BUS_WIDTH				(6 	| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_NO	| AT91C_MCI_MAXLAT)
#define ACMD6_BUS_WIDTH_1BIT			0x00 // bits 0..1
#define ACMD6_BUS_WIDTH_4BIT			0x02 // bits 0..1

#define ACMD13_SDCARD_STATUS			(13 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_NO	| AT91C_MCI_MAXLAT)
#define ACMD22_SEND_NUM_WR_BLOCKS		(22 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_NO	| AT91C_MCI_MAXLAT)
#define ACMD23_SET_WR_BLK_ERASE_COUNT	(23 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_NO	| AT91C_MCI_MAXLAT)
#define ACMD41_SEND_APP_OP_COND			(41 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_NO )
#define ACMD42_SET_CLR_CARD_DETECT		(42 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_NO	| AT91C_MCI_MAXLAT)
#define ACMD51_SEND_SCR					(51 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_NO	| AT91C_MCI_MAXLAT)


#define ACMD_ALL							(ACMD6_SET_BUS_WIDTH +\
											ACMD13_SDCARD_STATUS +\
											ACMD22_SEND_NUM_WR_BLOCKS +\
											ACMD23_SET_WR_BLK_ERASE_COUNT +\
											ACMD41_SEND_APP_OP_COND +\
											ACMD42_SET_CLR_CARD_DETECT +\
											ACMD51_SEND_SCR)

//*----------------------------------------
//* Class 9 commands: IO Mode commands
//*----------------------------------------

#define AT91C_MMC_FAST_IO					(39 | AT91C_MCI_SPCMD_NONE | AT91C_MCI_RSPTYP_48 | AT91C_MCI_MAXLAT)
#define AT91C_MMC_GO_IRQ_STATE				(40 | AT91C_MCI_SPCMD_NONE | AT91C_MCI_RSPTYP_48 | AT91C_MCI_TRCMD_NO	| AT91C_MCI_MAXLAT)



/////////////////////////////////////////////////////////////////////////////////////////////////////
// MCI_SR Errors
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define 	ERROR_ALL_SR				(AT91C_MCI_UNRE |\
										AT91C_MCI_OVRE |\
										AT91C_MCI_DTOE |\
										AT91C_MCI_DCRCE |\
										AT91C_MCI_RTOE |\
										AT91C_MCI_RENDE |\
										AT91C_MCI_RCRCE |\
										AT91C_MCI_RDIRE |\
										AT91C_MCI_RINDE)

////////////////////////////////////////////////////////////////////////////////////////////////////
// OCR Register Resonse Type R3 .. This register stores the Vdd voltage profile of the Sandisk card.
// 
//R3 (OCR Register): response length 48 bits.
//The contents of the OCR Register are sent as a response to CMD1.
////////////////////////////////////////////////////////////////////////////////////////////////////
#define R3_CMD1_OCR_RESERVED_LOWER				(0xF)    // 0..3
#define R3_CMD1_OCR_VDD_16_17					(1 << 4)
#define R3_CMD1_OCR_VDD_17_18					(1 << 5)
#define R3_CMD1_OCR_VDD_18_19					(1 << 6)
#define R3_CMD1_OCR_VDD_19_20					(1 << 7)
#define R3_CMD1_OCR_VDD_20_21					(1 << 8)
#define R3_CMD1_OCR_VDD_21_22					(1 << 9)
#define R3_CMD1_OCR_VDD_22_23					(1 << 10)
#define R3_CMD1_OCR_VDD_23_24					(1 << 11)
#define R3_CMD1_OCR_VDD_24_25					(1 << 12)
#define R3_CMD1_OCR_VDD_25_26					(1 << 13)
#define R3_CMD1_OCR_VDD_26_27					(1 << 14)
#define R3_CMD1_OCR_VDD_27_28					(1 << 15)
#define R3_CMD1_OCR_VDD_28_29					(1 << 16)
#define R3_CMD1_OCR_VDD_29_30					(1 << 17)
#define R3_CMD1_OCR_VDD_30_31					(1 << 18)
#define R3_CMD1_OCR_VDD_31_32					(1 << 19)
#define R3_CMD1_OCR_VDD_32_33					(1 << 20)
#define R3_CMD1_OCR_VDD_33_34					(1 << 21)
#define R3_CMD1_OCR_VDD_34_35					(1 << 22)
#define R3_CMD1_OCR_VDD_35_36					(1 << 23)
//#define R3_CMD1_OCR_RESERVED_HIGHER				(0x7F000000)    // 24..30
#define SDCARD_POWER_UP_BUSY					(1 << 31)

#define ATMEL_AT91_SDSLOT_VOLTAGE_RANGE			(R3_CMD1_OCR_VDD_27_28 +\
												R3_CMD1_OCR_VDD_28_29 +\
												R3_CMD1_OCR_VDD_29_30 +\
												R3_CMD1_OCR_VDD_30_31 +\
												R3_CMD1_OCR_VDD_31_32 +\
												R3_CMD1_OCR_VDD_32_33)

////////////////////////////////////////////////////////////////////////////////////////////////////
// CURRENT_STATE & READY_FOR_DATA in SDCard Status Register definition (response type R1)
/* Table 4-18 defines the SD card state-transition (dependent on the received command). The
SD Card application-specific command state-transitions can be found in Class 8.*/
////////////////////////////////////////////////////////////////////////////////////////////////////
// R2 response format MSB
#define R2_SENDSTATUS_IDLESTATE         		(0X1  <<8 )
#define R2_SENDSTATUS_ERASERESET         		(0X1  <<9 )
#define R2_SENDSTATUS_ILLCOMMAND        		(0X1  <<10 )
#define R2_SENDSTATUS_CRCERROR        			(0X1  <<11 )
#define R2_SENDSTATUS_ERASESEQERROR         	(0X1  <<12 )
#define R2_SENDSTATUS_ADDERROR         			(0X1  <<13 )
#define R2_SENDSTATUS_PARAMERROR         		(0X1  <<14 )

#define SDCARD_STATE_SR_READY_FOR_DATA			(1 << 8)	// corresponds to buffer empty signalling on the bus
#define AT91C_SR_CARD_SELECTED					(AT91C_SR_READY_FOR_DATA + AT91C_SR_TRAN)


// operation mode 2
// caRD IDENtification mode
#define SDCARD_STATE_SR_IDLE					(0 << 9)
#define SDCARD_STATE_SR_READY					(1 << 9)
#define SDCARD_STATE_SR_IDENTIFICATION			(2 << 9)
// Data transfer mode
#define SDCARD_STATE_SR_STBY					(3 << 9)
#define SDCARD_STATE_SR_TRANSFER				(4 << 9)
#define SDCARD_STATE_SR_SENDDATA				(5 << 9)
#define SDCARD_STATE_SR_RECEIVEDATA				(6 << 9)
#define SDCARD_STATE_SR_PROGRAMMING				(7 << 9)
#define SDCARD_STATE_SR_DISCONNECT				(8 << 9)
#define SDCARD_STATE_SR_INA						(9 << 9)

/////////////////////////////////////////////////////////////////////////////////////////////////////
// REsponse TYPE R2  -The content of the CID Register is sent as a response to CMD2 and CMD10. The content 
// of the CSD Register is sent as a response to CMD9.
// MMC CSD register header File					
// _CSD_xxx_S	for shift value
// _CSD_xxx_M	for mask  value
/////////////////////////////////////////////////////////////////////////////////////////////////////




// First Response INT <=> CSD[3] : bits 0 to 31
#define	R2_CSD_BIT0_S					0		x// [0:0]			
#define	R2_CSD_BIT0_M					0x01				
#define	R2_CSD_CRC_S					1		// [7:1]
#define	R2_CSD_CRC_M					0x7F
#define	R2_CSD_MMC_ECC_S				8		// [9:8]		reserved for MMC compatibility
#define	R2_CSD_MMC_ECC_M				0x03
#define	R2_CSD_FILE_FMT_S				10		// [11:10]
#define	R2_CSD_FILE_FMT_M				0x03
#define	R2_CSD_TMP_WRITEPROT_S			12		// [12:12]
#define	R2_CSD_TMP_WRITEPROT_M			0x01
#define	R2_CSD_PERM_WRITEPROT_S 		13		// [13:13]
#define	R2_CSD_PERM_WRITEPROT_M 		0x01
#define	R2_CSD_COPY_S	 				14		// [14:14]
#define	R2_CSD_COPY_M 					0x01
#define	R2_CSD_FILE_FMT_GRP_S			15		// [15:15]
#define	R2_CSD_FILE_FMT_GRP_M			0x01
//	reserved							16		// [20:16]
//	reserved							0x1F
#define	R2_CSD_WRITEPARBLK_S 			21		// [21:21]
#define	R2_CSD_WRITEPARBLK_M 			0x01
#define	R2_CSD_MAXWRITEBLKLEN_S 		22		// [25:22]
#define	R2_CSD_MAXWRITEBLKLEN_M			0x0F
#define	R2_CSD_WRITESPEEDFAC_S 			26		// [28:26]
#define	R2_CSD_WRITESPEEDFAC_M 			0x07
#define	R2_CSD_MMC_DEF_ECC_S			29		// [30:29]		reserved for MMC compatibility
#define	R2_CSD_MMC_DEF_ECC_M			0x03
#define	R2_CSD_v22_WP_GRP_ENABLE_S		31		// [31:31]				
#define	R2_CSD_v22_WP_GRP_ENABLE_M		0x01				


// Seconde Response INT <=> CSD[2] : bits 32 to 63

#define	R2_CSD_v22_ER_GRP_SIZE_S		5		// [41:37]
#define	R2_CSD_v22_ER_GRP_SIZE_M		0x1F

#define	R2_CSD_v21_WP_GRP_SIZE_S		0		// [38:32]				
#define	R2_CSD_v21_WP_GRP_SIZE_M		0x7F				
#define	R2_CSD_v21_SECTSIZE_S			7		// [45:39]
#define	R2_CSD_v21_SECTSIZE_M			0x7F
#define R2_CSD_v21_ERASEBLKEN_S    		14      // [46:46]
#define R2_CSD_v21_ERASEBLKEN_M   		0x1

#define	R2_CSD_CSIZE_MULT_S				15		// [49:47]
#define	R2_CSD_CSIZE_MULT_M				0x07

#define	R2_CSD_MAXCURRWRITE_S 			18		// [52:50]
#define	R2_CSD_MAXCURRWRITE_M 			0x07

#define	R2_CSD_MINCURRWRITE_S	 		21		// [55:53]
#define	R2_CSD_MINCURRWRITE_M 			0x07

#define	R2_CSD_MAXCURRREAD_S 			24		// [58:56]
#define	R2_CSD_MAXCURRREAD_M 			0x07
#define	R2_CSD_MINCURRREAD_S 			27		// [61:59]
#define	R2_CSD_MINCURRREAD_M 			0x07
#define	R2_CSD_CSIZE_L_S 				30		// [63:62] <=> 2 LSB of CSIZE
#define	R2_CSD_CSIZE_L_M 				0x03

// Third Response INT <=> CSD[1] : bits 64 to 95
#define	R2_CSD_CSIZE_H_S 				0		// [73:64]	<=> 10 MSB of CSIZE
#define	R2_CSD_CSIZE_H_M 				0x03FF
// reserved								10		// [75:74]
// reserved								0x03		
#define	R2_CSD_DSRIMP_S 				12		// [76:76]
#define	R2_CSD_DSRIMP_M 				0x01
#define	R2_CSD_READBLKMIS_S 			13		// [77:77]
#define	R2_CSD_READBLKMIS_M 			0x01
#define	R2_CSD_WRITEBLKMIS_S 			14		// [78:78]
#define	R2_CSD_WRITEBLKMIS_M 			0x01
#define	R2_CSD_READPARBLK_S 			15		// [79:79]
#define	R2_CSD_READPARBLK_M 			0x01
#define	R2_CSD_MAXREADBLKLEN_S 			16		// [83:80]
#define	R2_CSD_MAXREADBLKLEN_M 			0x0F
#define	R2_CSD_CARDCMDCLASSES_S	 		20		// [95:84]
#define	R2_CSD_CARDCMDCLASSES_M 		0x0FFF

// Fourth Response INT <=> CSD[0] : bits 96 to 127
#define	R2_CSD_MAXDATATRANSRATE_S 		0		// [103:96]
#define	R2_CSD_MAXDATATRANSRATE_M 		0xFF
#define	R2_CSD_NSAC_S 					8		// [111:104]
#define	R2_CSD_NSAC_M 					0xFF
#define	R2_CSD_DATAREADACTIME_S 		16		// [119:112]
#define	R2_CSD_DATAREADACTIME_M 		0xFF
//	reserved							24		// [121:120]
//	reserved							0x03
#define	R2_CSD_MMC_SPEC_VERS_S			26		// [125:122]	reserved for MMC compatibility
#define	R2_CSD_MMC_SPEC_VERS_M			0x0F
#define	R2_CSD_STRUCT_S					30		// [127:126]
#define	R2_CSD_STRUCT_M 				0x03

/////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
