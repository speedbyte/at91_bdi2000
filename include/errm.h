#ifndef errm_h
#define errm_h

#define ERRM(x,y) 		SD_##x##_##y##_ERROR


enum _errors {
ERRM(WRITE,NO)				= 0,
ERRM(WRITE,FOUND)			= 1, 			
ERRM(WRITE,MEMFULL)			= 2, 		
ERRM(READ,NO) 				= 0,
ERRM(READ,FOUND)			= 1,
ERRM(CMD,SENDNO) 			= 0,
ERRM(CMD,SEND)				= 1,
ERRM(INIT,NO)				= 0,
ERRM(INIT,FOUND)			= 1
};


/*
// ---------- to be sent on KLine ----------
union _SDerrormanagement {
unsigned int error_values;
struct 
{
unsigned int OK      : 1;
unsigned int MEM_FULL: 2;
unsigned int year    : 8;
unsigned int month   : 5;
unsigned int day     : 3;
unsigned int date    : 6;
unsigned int         : 2;
} sdcard;
};
typedef union _SDerrormanagement SD_ERRORMANAGEMENT;
*/
#endif
