/*************************************************************************
|  COPYRIGHT (c) 2000 BY ABATRON AG
|*************************************************************************
|
|  PROJECT NAME: BDI Configuration Utility
|  FILENAME    : bdicnf.c
|
|  COMPILER    : GCC
|
|  TARGET OS   : LINUX / UNIX
|  TARGET HW   : PC
|
|  PROGRAMMER  : Abatron / RD
|  CREATION    : 19.12.00
|
|*************************************************************************
|
|  DESCRIPTION :
|  This module builds the configuration stored into the BDI flash memory.
|  The BDI configuration is only stored for the standard firmware.
|  GDB, Tornado and ADA firmware get the configuration via TFTP.
|
|*************************************************************************/

/*************************************************************************
|  INCLUDES
|*************************************************************************/

#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include "bdidll.h"
#include "bdicnf.h"

/*************************************************************************
|  DEFINES
|*************************************************************************/

#define MAX_LINE_LEN    256
#define BDI_FW_COUNT    40


/* ASCII codes */
#define HT              9
#define CR              13
#define LF              10


/*************************************************************************
|  MACROS
|*************************************************************************/

/*************************************************************************
|  TYPEDEFS
|*************************************************************************/

typedef struct {
  char*   name;
  BOOL    hasAddr;
} CNF_InitInfoT;

typedef struct {
  char*   name;
  char**  pEnum;
  WORD    size;
  DWORD   init;
} CNF_ModeInfoT;


/*************************************************************************
|  TABLES
|*************************************************************************/

static char* StartMode[] = {"RESET","STOP","RUN",NULL};
static char* BreakMode[] = {"SOFT","HARD",NULL};
static char* CatchVector[] = {"dummy","CATCH",NULL};
static char* EndianMode[] = {"LITTLE","BIG",NULL};

/********* ARM **********/

static const CNF_InitInfoT InitInfoARM[] =
{
{ "NOP",        FALSE },
{ "DELAY",      FALSE },
{ "WM8",        TRUE  },
{ "WM16",       TRUE  },
{ "WM32",       TRUE  },
{ "WGPR",       TRUE  },
{ "WCSPR",      FALSE },
{ "WCP15",      TRUE  },
{ "MMAP",       TRUE  },
{ "RM8",        TRUE  },
{ "RM16",       TRUE  },
{ "RM32",       TRUE  },
{  NULL,        TRUE  }
};

static char* CpuTypeARM[] = {
"ARM7TDMI",
"ARM7DI",
"ARM710T",
"ARM720T",
"ARM740T",
"ARM9TDMI",
"ARM920T",
"ARM940T",
"TMS470",
"ARM9E",
"ARM946E",
"ARM966E",
"TI925T",
"MAC7100",
"ARM926E",
NULL};

static const CNF_ModeInfoT ModeInfoARM[] =
{
{ "trigger",    NULL,           4, 0xCFA001CF   },
{ "timestamp",  NULL,           4, 0            },
{ "baudrate",   NULL,           4, 9600         },
{ "WORKSPACE",  NULL,           4, 0xFFFFFFFF   },
{ "RUNTIME",    NULL,           2, 2000         },
{ "CPUTYPE",    CpuTypeARM,     1, 0            },
{ "STARTUP",    StartMode,      1, 0            },
{ "BREAKMODE",  BreakMode,      1, 0            },
{ "TRAPMASK",   NULL,           1, 0            },
{ "JTAGCLOCK",  NULL,           1, 3            },
{ "ENDIAN",     EndianMode,     1, 0            },
{ "PREDCOUNT",  NULL,           1, 0            },
{ "PREDSIZE",   NULL,           1, 0            },
{ "SUCCCOUNT",  NULL,           1, 0            },
{ "SUCCSIZE",   NULL,           1, 0            },
{  NULL,        NULL,           0, 0            }
};

/********* ColdFire **********/

static const CNF_InitInfoT InitInfoMCF[] =
{
{ "NOP",        FALSE },
{ "DELAY",      FALSE },
{ "WM8",        TRUE  },
{ "WM16",       TRUE  },
{ "WM32",       TRUE  },
{ "WDREG",      TRUE  },
{ "WAREG",      TRUE  },
{ "WCREG",      TRUE  },
{ "MMAP",       TRUE  },
{ "WTLB",       TRUE  },
{  NULL,        TRUE  }
};

static char* CpuTypeMCF[] = {
"MCF5202",
"MCF5203",
"MCF5204",
"MCF5206",
"MCF5307",
"MCF5407",
"MCF5272",
"MCF5249",
"MCF5280/70/30",
"MCF5480/70",
"MCF5213/12/11",
NULL};

static char* BreakPointMCF[]  = {"FREEZE","LOOP",NULL};
static char* BreakModeMCF[]   = {"SOFT","HARD",NULL};
static char* LoopLevelMCF[]   = {"0","1","2","3","4","5","6","7","CURRENT",NULL};
static char* VectorMCF[]      = {"CATCH","dummy",NULL};

static const CNF_ModeInfoT ModeInfoMCF[] =
{
{ "trigger",    NULL,           4, 0xCF4001CF   },
{ "timestamp",  NULL,           4, 0            },
{ "baudrate",   NULL,           4, 9600         },
{ "CPUCLOCK",   NULL,           4, 16000000     },
{ "WORKSPACE",  NULL,           4, 0x00000400   },
{ "VECTORBASE", NULL,           4, 0x00000000   },
{ "CPUTYPE",    CpuTypeMCF,     1, 8            },
{ "STARTUP",    StartMode,      1, 0            },
{ "BREAKPOINT", BreakPointMCF,  1, 0            },
{ "LOOPLEVEL",  LoopLevelMCF,   1, 8            },
{ "VECTOR",     VectorMCF,      1, 1            },
{ "BREAKMODE",  BreakModeMCF,   1, 0            },
{  NULL,        NULL,           0, 0            }
};

/********* MPC8xx/5xx **********/

static const CNF_InitInfoT InitInfoPPC[] =
{
{ "NOP",        FALSE },
{ "DELAY",      FALSE },
{ "WM8",        TRUE  },
{ "WM16",       TRUE  },
{ "WM32",       TRUE  },
{ "WGPR",       TRUE  },
{ "WSPR",       TRUE  },
{ "WMSR",       FALSE },
{ "WCR",        FALSE },
{ "SUPM",       TRUE  },
{ "WUPM",       TRUE  },
{  NULL,        TRUE  }
};

static char* CpuTypePPC[]   = {"MPC800","MPC500",NULL};

static const CNF_ModeInfoT ModeInfoPPC[] =
{
{ "trigger",    NULL,           4, 0xCF3001CF   },
{ "timestamp",  NULL,           4, 0            },
{ "baudrate",   NULL,           4, 9600         },
{ "CPUCLOCK",   NULL,           4, 16000000     },
{ "CPUTYPE",    CpuTypePPC,     1, 0            },
{ "STARTUP",    StartMode,      1, 0            },
{ "BREAKMODE",  BreakMode,      1, 0            },
{ "dummy",      NULL,           1, 0xFF         },
{ "WORKSPACE",  NULL,           4, 0xFFFFFFFF   },
{  NULL,        NULL,           0, 0            }
};

/********* MPC4xx **********/

static const CNF_InitInfoT InitInfoPP4[] =
{
{ "NOP",        FALSE },
{ "DELAY",      FALSE },
{ "WM8",        TRUE  },
{ "WM16",       TRUE  },
{ "WM32",       TRUE  },
{ "WGPR",       TRUE  },
{ "WSPR",       TRUE  },
{ "WMSR",       FALSE },
{ "WCR",        FALSE },
{ "WDCR",       TRUE  },
{  NULL,        TRUE  }
};

static char* CpuTypePP4[] = {"401","403","405","440",NULL};

static const CNF_ModeInfoT ModeInfoPP4[] =
{
{ "trigger",    NULL,           4, 0xCFD401CF   },
{ "timestamp",  NULL,           4, 0            },
{ "baudrate",   NULL,           4, 9600         },
{ "RUNTIME",    NULL,           2, 2000         },
{ "CPUTYPE",    CpuTypePP4,     1, 0            },
{ "STARTUP",    StartMode,      1, 0            },
{ "JTAGCLOCK",  NULL,           1, 3            },
{ "VECTOR",     CatchVector,    1, 0            },
{  NULL,        NULL,           0, 0            }
};


/********* MPC6xx/7xx/82xx/83xx **********/

static const CNF_InitInfoT InitInfoCOP[] =
{
{ "NOP",        FALSE },
{ "DELAY",      FALSE },
{ "WM8",        TRUE  },
{ "WM16",       TRUE  },
{ "WM32",       TRUE  },
{ "WGPR",       TRUE  },
{ "WSPR",       TRUE  },
{ "WMSR",       FALSE },
{ "WCR",        FALSE },
{ "SUPM",       TRUE  },
{ "WUPM",       TRUE  },
{ "TSZ1",       TRUE  },
{ "TSZ2",       TRUE  },
{ "TSZ4",       TRUE  },
{ "TSZ8",       TRUE  },
{ "MMAP",       TRUE  },
{ "WM64",       TRUE  },
{  NULL,        TRUE  }
};

static char* CpuTypeCOP[] = {
"PPC603e",
"PPC603ev",
"PPC750",
"MPC8260",
"MPC8240",
"PPC750CX",
"MPC7400",
"MGT5100",
"PPC750FX",
"MPC8280",
"MPC5200",
"MPC8220",
"MPC8300",
NULL};

static char* DataCacheCOP[] = {"NOFLUSH","FLUSH",NULL};

static const CNF_ModeInfoT ModeInfoCOP[] =
{
{ "trigger",    NULL,           4, 0xCFD101CF   },
{ "timestamp",  NULL,           4, 0            },
{ "baudrate",   NULL,           4, 9600         },
{ "RUNTIME",    NULL,           2, 2000         },
{ "CPUTYPE",    CpuTypeCOP,     1, 0            },
{ "STARTUP",    StartMode,      1, 0            },
{ "JTAGCLOCK",  NULL,           1, 2            },
{ "VECTOR",     CatchVector,    1, 0            },
{ "DCACHE",     DataCacheCOP,   1, 0            },
{ "dummy",      NULL,           1, 0xFF         },
{ "WORKSPACE",  NULL,           4, 0xFFFFFFFF   },
{  NULL,        NULL,           0, 0            }
};

/********* Init List Info **********/

static const CNF_InitInfoT* InitInfoTable[BDI_FW_COUNT][BDI_TYPE_LAST+1] =
{     /*   BDI-HS           BDI2000          BDI2000 Rev.C    BDI1000    */
/* 00 */ { NULL,            NULL,            NULL,            NULL            },
/* 01 */ { NULL,            NULL,            NULL,            NULL            },
/* 02 */ { NULL,            NULL,            NULL,            NULL            },
/* 03 */ { NULL,            NULL,            NULL,            NULL            },
/* 04 */ { NULL,            NULL,            NULL,            NULL            },
/* 05 */ { NULL,            InitInfoPPC,     InitInfoPPC,     InitInfoPPC     },
/* 06 */ { NULL,            InitInfoMCF,     InitInfoMCF,     InitInfoMCF     },
/* 07 */ { NULL,            NULL,            NULL,            NULL            },
/* 08 */ { NULL,            NULL,            NULL,            NULL            },
/* 09 */ { NULL,            NULL,            NULL,            NULL            },
/* 10 */ { NULL,            InitInfoARM,     InitInfoARM,     InitInfoARM     },
/* 11 */ { NULL,            NULL,            NULL,            NULL            },
/* 12 */ { NULL,            NULL,            NULL,            NULL            },
/* 13 */ { NULL,            NULL,            NULL,            NULL            },
/* 14 */ { NULL,            NULL,            NULL,            NULL            },
/* 15 */ { NULL,            NULL,            NULL,            NULL            },
/* 16 */ { NULL,            NULL,            NULL,            NULL            },
/* 17 */ { NULL,            InitInfoCOP,     InitInfoCOP,     NULL            },
/* 18 */ { NULL,            NULL,            NULL,            NULL            },
/* 19 */ { NULL,            NULL,            NULL,            NULL            },
/* 20 */ { NULL,            InitInfoPP4,     InitInfoPP4,     InitInfoPP4     },
/* 21 */ { NULL,            NULL,            NULL,            NULL            },
/* 22 */ { NULL,            NULL,            NULL,            NULL            },
/* 23 */ { NULL,            NULL,            NULL,            NULL            },
/* 24 */ { NULL,            NULL,            NULL,            NULL            },
/* 25 */ { NULL,            NULL,            NULL,            NULL            },
/* 26 */ { NULL,            NULL,            NULL,            NULL            },
/* 27 */ { NULL,            NULL,            NULL,            NULL            },
/* 28 */ { NULL,            NULL,            NULL,            NULL            },
/* 29 */ { NULL,            NULL,            NULL,            NULL            },
/* 30 */ { NULL,            NULL,            NULL,            NULL            },
/* 31 */ { NULL,            NULL,            NULL,            NULL            },
/* 32 */ { NULL,            NULL,            NULL,            NULL            },
/* 33 */ { NULL,            NULL,            NULL,            NULL            },
/* 34 */ { NULL,            NULL,            NULL,            NULL            },
/* 35 */ { NULL,            NULL,            NULL,            NULL            },
/* 36 */ { NULL,            NULL,            NULL,            NULL            },
/* 37 */ { NULL,            NULL,            NULL,            NULL            },
/* 38 */ { NULL,            NULL,            NULL,            NULL            },
/* 39 */ { NULL,            NULL,            NULL,            NULL            }
};


/********* Mode List Info **********/

static const CNF_ModeInfoT* ModeInfoTable[BDI_FW_COUNT][BDI_TYPE_LAST+1] =
{     /*   BDI-HS           BDI2000          BDI2000 Rev.C    BDI1000    */
/* 00 */ { NULL,            NULL,            NULL,            NULL            },
/* 01 */ { NULL,            NULL,            NULL,            NULL            },
/* 02 */ { NULL,            NULL,            NULL,            NULL            },
/* 03 */ { NULL,            NULL,            NULL,            NULL            },
/* 04 */ { NULL,            NULL,            NULL,            NULL            },
/* 05 */ { NULL,            ModeInfoPPC,     ModeInfoPPC,     ModeInfoPPC     },
/* 06 */ { NULL,            ModeInfoMCF,     ModeInfoMCF,     ModeInfoMCF     },
/* 07 */ { NULL,            NULL,            NULL,            NULL            },
/* 08 */ { NULL,            NULL,            NULL,            NULL            },
/* 09 */ { NULL,            NULL,            NULL,            NULL            },
/* 10 */ { NULL,            ModeInfoARM,     ModeInfoARM,     ModeInfoARM     },
/* 11 */ { NULL,            NULL,            NULL,            NULL            },
/* 12 */ { NULL,            NULL,            NULL,            NULL            },
/* 13 */ { NULL,            NULL,            NULL,            NULL            },
/* 14 */ { NULL,            NULL,            NULL,            NULL            },
/* 15 */ { NULL,            NULL,            NULL,            NULL            },
/* 16 */ { NULL,            NULL,            NULL,            NULL            },
/* 17 */ { NULL,            ModeInfoCOP,     ModeInfoCOP,     ModeInfoCOP     },
/* 18 */ { NULL,            NULL,            NULL,            NULL            },
/* 19 */ { NULL,            NULL,            NULL,            NULL            },
/* 20 */ { NULL,            ModeInfoPP4,     ModeInfoPP4,     ModeInfoPP4     },
/* 21 */ { NULL,            NULL,            NULL,            NULL            },
/* 22 */ { NULL,            NULL,            NULL,            NULL            },
/* 23 */ { NULL,            NULL,            NULL,            NULL            },
/* 24 */ { NULL,            NULL,            NULL,            NULL            },
/* 25 */ { NULL,            NULL,            NULL,            NULL            },
/* 26 */ { NULL,            NULL,            NULL,            NULL            },
/* 27 */ { NULL,            NULL,            NULL,            NULL            },
/* 28 */ { NULL,            NULL,            NULL,            NULL            },
/* 29 */ { NULL,            NULL,            NULL,            NULL            },
/* 30 */ { NULL,            NULL,            NULL,            NULL            },
/* 31 */ { NULL,            NULL,            NULL,            NULL            },
/* 32 */ { NULL,            NULL,            NULL,            NULL            },
/* 33 */ { NULL,            NULL,            NULL,            NULL            },
/* 34 */ { NULL,            NULL,            NULL,            NULL            },
/* 35 */ { NULL,            NULL,            NULL,            NULL            },
/* 36 */ { NULL,            NULL,            NULL,            NULL            },
/* 37 */ { NULL,            NULL,            NULL,            NULL            },
/* 38 */ { NULL,            NULL,            NULL,            NULL            },
/* 39 */ { NULL,            NULL,            NULL,            NULL            }
};


/*************************************************************************
|  LOCALS
|*************************************************************************/

/****************************************************************************
 ****************************************************************************

    BDI_Append___ :

    Host independent helper function to append a numeric value to a buffer.
    The bytes will be stored in the link format order (Motorola byte order).

     INPUT  : value     value
              buffer    pointer to buffer
     OUTPUT : RETURN    pointer to next byte after the stored value

 ****************************************************************************/

static BYTE* BDI_AppendByte(BYTE  value, BYTE* buffer)
{
  *buffer++ = value;
  return buffer;
} /* BDI_AppendByte */

static BYTE* BDI_AppendWord(WORD value, BYTE* buffer)
{
  *buffer++ = (BYTE)(value>>8);
  *buffer++ = (BYTE)value;
  return buffer;
} /* BDI_AppendWord */

static BYTE* BDI_AppendLong(DWORD value, BYTE* buffer)
{
  *buffer++ = (BYTE)(value>>24);
  *buffer++ = (BYTE)(value>>16);
  *buffer++ = (BYTE)(value>>8);
  *buffer++ = (BYTE)value;
  return buffer;
} /* BDI_AppendLong */


/****************************************************************************
 ****************************************************************************

    UTIL_ExtractNumber

    Extract a number (0xnnn for hex). Conversion stops if illegal character.

    INPUT  : getPtr     pointer to the next character to read
    OUTPUT : value      the extracted integer
             RETURN     pointer to the next unread character

 ****************************************************************************/

static char* ExtractHex (DWORD* value,  char* getPtr)
{
  DWORD num;
  DWORD digit;
  BYTE  c;

  while (*getPtr == ' ') getPtr++;
  num = 0;
  for (;;) {
    c = *getPtr;
    if      ((c >= '0') && (c <= '9')) digit = (DWORD)(c - '0');
    else if ((c >= 'A') && (c <= 'F')) digit = (DWORD)(c - 'A' + 10);
    else if ((c >= 'a') && (c <= 'f')) digit = (DWORD)(c - 'a' + 10);
    else break;
    num <<= 4;
    num += digit;
    getPtr++;
  } /* for */
  *value = num;
  return getPtr;
} /* ExtractHex */

static char* ExtractDecimal (DWORD* value,  char* getPtr)
{
  DWORD num;
  DWORD digit;
  BYTE  c;

  while (*getPtr == ' ') getPtr++;
  num = 0;
  for (;;) {
    c = *getPtr;
    if      ((c >= '0') && (c <= '9')) digit = (DWORD)(c - '0');
    else break;
    num *= 10;
    num += digit;
    getPtr++;
  } /* for */
  *value = num;
  return getPtr;
} /* ExtractDecimal */


static char* UTIL_ExtractNumber (DWORD* value,  char* getPtr)
{
  while (*getPtr == ' ') getPtr++;
  if ((*getPtr == '0') && ((*(getPtr+1) == 'x') || (*(getPtr+1) == 'X'))) {
    getPtr +=2;
    return ExtractHex(value, getPtr);
  } /* if */
  else {
    return ExtractDecimal(value, getPtr);
  } /* else */
} /* UTIL_ExtractNumber */


/****************************************************************************
 ****************************************************************************

    UTIL_ExtractString

    Extract a string until the next seperator.

    INPUT  : getPtr       pointer to the next character to read
    OUTPUT : string       a pointer to the first character of the string
             RETURN       pointer to the next unread character after the enumaration

 ****************************************************************************/

static char* UTIL_ExtractString (char* string, char* getPtr)
{

  /* skip spaces */
  while (*getPtr == ' ') getPtr++;

  /* copy string */
  while ((*getPtr != ' ') && (*getPtr != ';') && (*getPtr != 0)) {
    *string++ = *getPtr++;
  } /* while */

  *string = 0;
  return getPtr;
} /* UTIL_ExtractString */


/****************************************************************************
 ****************************************************************************

    ValidEndOfLine:

    Test for a valid enf of line

    INPUT  : linePtr    pointer to the next character to read
    OUTPUT : RETURN     TRUE if valid end of line

 ****************************************************************************/

static BOOL ValidEndOfLine (char* linePtr)
{
  while (*linePtr == ' ') linePtr++;
  return ((*linePtr == 0) || (*linePtr == ';'));
} /* ValidEndOfLine */


/****************************************************************************
 ****************************************************************************

    GetNextLine:

    Read next line from the file

    INPUT  : file       the file pointer
    OUTPUT : line       the line read
             RETURN     number of chars or -1 if end of file

 ****************************************************************************/

static int GetNextLine(FILE* file, char* line)
{
  char* pCh;

  if (fgets(line, MAX_LINE_LEN, file) != NULL) {
    pCh = line;
    while ((*pCh != 0) && (*pCh != CR) && (*pCh != LF))  {
      if (*pCh == HT) {
        *pCh = ' ';
      } /* if */
      else {
        *pCh = toupper(*pCh);
      } /* else */
      pCh++;
    } /* while */
    *pCh = 0;
    return pCh - line;
  } /* if */
  else {
    return -1;
  } /* else */
} /* GetNextLine */


/****************************************************************************
 ****************************************************************************

  CNF_BuildInitList :

  Build the init list based on information in configuration file

  INPUT:  szFile        the configuration file name
          initInfo      table with info about the available commands
  OUTPUT: initList      the init list to store into the BDI flash
          return        number of enties in the init list

 ****************************************************************************/

static int CNF_BuildInitList(const char*           szFile,
                             const CNF_InitInfoT*  initInfo,
                                   CNF_InitEntryT* initList)
{
  FILE*                 file;
  char                  line[MAX_LINE_LEN];
  char                  szCmd[MAX_LINE_LEN];
  char*                 pGet;
  CNF_InitEntryT*       pPut;
  DWORD                 cmd;
  const CNF_InitInfoT*  pInfo;

  /* open configuration file */
  file = fopen(szFile,"r");
  if (file == NULL) {
    printf("Cannot open %s\n", szFile);
    return 0;
  } /* if */

  /* search [INIT] part */
  do {
    if (GetNextLine(file, line) < 0) {
      printf("No [INIT] section found in %s\n", szFile);
      return 0;
    } /* if */
  } while (strcmp(line, "[INIT]") != 0);

  /* process init line */
  pPut = initList;
  while (GetNextLine(file, line) >= 0) {
    pGet = line;
    if (!ValidEndOfLine(pGet)) {
      pGet = UTIL_ExtractString (szCmd, pGet);
      if (szCmd[0] == '[') break;
      pInfo = initInfo;
      cmd   = 0;
      while (pInfo->name != NULL) {
        if (strcmp(pInfo->name, szCmd) == 0) {
          pPut->cmd = cmd;
          if (pInfo->hasAddr) {
            pGet = UTIL_ExtractNumber(&pPut->addr, pGet);
          } /* if */
          pGet = UTIL_ExtractNumber(&pPut->value, pGet);
          pPut++;
          break;
        } /* if */
        cmd++;
        pInfo++;
      } /* while */
      if (pInfo->name == NULL) {
        printf("Invalid line: %s\n", line);
        return 0;
      } /* if */
    } /* if */
    if (!ValidEndOfLine(pGet)) {
      printf("Invalid line: %s\n", line);
      return 0;
    } /* if */
  } /* while */

  return pPut - initList;
} /* CNF_BuildInitList */



/****************************************************************************
 ****************************************************************************

  CNF_BuildModeList :

  Build the mode list based on information in configuration file

  INPUT:  szFile        the configuration file name
          modeInfo      table with info about the available parameters
  OUTPUT: modeList      the mode list to store into the BDI flash
          return        number of enties in the init list

 ****************************************************************************/

static int CNF_BuildModeList(const char*           szFile,
                             const CNF_ModeInfoT*  modeInfo,
                                   BYTE*           modeList)
{
  FILE*                 file;
  char                  line[MAX_LINE_LEN];
  char                  szParam[MAX_LINE_LEN];
  char                  szEnum[MAX_LINE_LEN];
  char*                 pGet;
  BYTE*                 pPut;
  DWORD                 value;
  const CNF_ModeInfoT*  pInfo;
  char**                pEnum;

  /* open configuration file */
  file = fopen(szFile,"r");
  if (file == NULL) {
    printf("Cannot open %s\n", szFile);
    return 0;
  } /* if */

  /* fill default values */
  memset(modeList, 0xFF, CNF_MAX_MODE_SIZE);
  pPut  = modeList;
  pInfo = modeInfo;
  while (pInfo->name != NULL) {
    switch (pInfo->size) {
    case 1:
      pPut = BDI_AppendByte((BYTE)pInfo->init, pPut);
      break;
    case 2:
      pPut = BDI_AppendWord((WORD)pInfo->init, pPut);
      break;
    case 4:
      pPut = BDI_AppendLong(pInfo->init, pPut);
      break;
    } /* switch */
    pInfo++;
  } /* while */
  strcpy(modeList + (CNF_MAX_MODE_SIZE - 40), "<No setup ID>");

  /* search [MODE] part */
  do {
    if (GetNextLine(file, line) < 0) {
      printf("No [MODE] section found in %s\n", szFile);
      return 0;
    } /* if */
  } while (strcmp(line, "[MODE]") != 0);

  /* process init line */
  while (GetNextLine(file, line) >= 0) {
    pGet = line;
    if (!ValidEndOfLine(pGet)) {
      pGet = UTIL_ExtractString (szParam, pGet);
      if (szParam[0] == '[') break;
      pPut  = modeList;
      pInfo = modeInfo;
      while (pInfo->name != NULL) {
        if (strcmp(pInfo->name, szParam) == 0) {
          if (pInfo->pEnum) {
            pGet = UTIL_ExtractString(szEnum, pGet);
            value = 0;
            pEnum = pInfo->pEnum;
            while (*pEnum != NULL) {
              if (strcmp(*pEnum, szEnum) == 0) {
                pPut = BDI_AppendByte((BYTE)value, pPut);
                break;
              } /* if */
              pEnum++;
              value++;
            } /* while */
            if (*pEnum == NULL) {
              printf("Invalid line: %s\n", line);
              return 0;
            } /* if */
          } /* if */
          else {
            pGet = UTIL_ExtractNumber(&value, pGet);
            switch (pInfo->size) {
            case 1:
              pPut = BDI_AppendByte((BYTE)value, pPut);
              break;
            case 2:
              pPut = BDI_AppendWord((WORD)value, pPut);
              break;
            case 4:
              pPut = BDI_AppendLong(value, pPut);
              break;
            } /* switch */
          } /* else */
          break;
        } /* if */
        else {
          pPut += pInfo->size;
        } /* else */
        pInfo++;
      } /* while */
      if (pInfo->name == NULL) {
        printf("Invalid line: %s\n", line);
        return 0;
      } /* if */
    } /* if */
    if (!ValidEndOfLine(pGet)) {
      printf("Invalid line: %s\n", line);
      return 0;
    } /* if */
  } /* while */

  return CNF_MAX_MODE_SIZE;
} /* CNF_BuildModeList */


/****************************************************************************
 ****************************************************************************

  CNF_GetInitList :

  Build the init list based on information in configuration file

  INPUT:  szFile        the configuration file name
          bdiType       the BDI type
          bdiFw         the BDI firmware
  OUTPUT: initList      the init list to store into the BDI flash
          return        number of enties in the init list

 ****************************************************************************/

int CNF_GetInitList(const char*                 szFile,
                          WORD                  bdiType,
                          WORD                  fwType,
                          CNF_InitEntryT*       initList)
{
  const CNF_InitInfoT*  initInfo;

  /* check parameters */
  if ((fwType >= BDI_FW_COUNT) || (bdiType > BDI_TYPE_LAST)) return 0;

  /* check if init info available */
  initInfo = InitInfoTable[fwType][bdiType];
  if (initInfo == NULL) return 0;

  /* build init list */
  return CNF_BuildInitList(szFile, initInfo, initList);

} /* CNF_GetInitList */


/****************************************************************************
 ****************************************************************************

  CNF_GetModeList :

  Build the mode list based on information in configuration file

  INPUT:  szFile        the configuration file name
          bdiType       the BDI type
          bdiFw         the BDI firmware
  OUTPUT: modeList      the mode list to store into the BDI flash
          return        number of enties in the init list

 ****************************************************************************/

int CNF_GetModeList(const char* szFile,
                          WORD  bdiType,
                          WORD  fwType,
                          BYTE* modeList)
{
  const CNF_ModeInfoT*  modeInfo;

  /* check parameters */
  if ((fwType >= BDI_FW_COUNT) || (bdiType > BDI_TYPE_LAST)) return 0;

  /* check if mode info available */
  modeInfo = ModeInfoTable[fwType][bdiType];
  if (modeInfo == NULL) return 0;

  /* build init list */
  return CNF_BuildModeList(szFile, modeInfo, modeList);

} /* CNF_GetModeList */



#if 0
/****************************************************************************
 ****************************************************************************

   main (for test only)

 ****************************************************************************/

int main(int argc, char *argv[ ])
{
  CNF_InitEntryT  initList[1024];
  BYTE            modeList[1024];
  int             count;
  int             i;

  count = CNF_GetInitList("eval7t.cfg", 2, 10, initList);
  printf("Init Count = %i\n", count);
  for (i = 0; i < count; i++) {
    printf("%2i 0x%08lx 0x%08lx\n", initList[i].cmd, initList[i].addr, initList[i].value);
  }

  count = CNF_GetModeList("eval7t.cfg", 2, 10, modeList);
  printf("Mode Count = %i\n", count);
  for (i = 0; i < count; i+=8) {
    printf("%02x %02x %02x %02x %02x %02x %02x %02x\n",
           modeList[i+0], modeList[i+1], modeList[i+2], modeList[i+3],
           modeList[i+4], modeList[i+5], modeList[i+6], modeList[i+7]);
  }

} /* main */
#endif
