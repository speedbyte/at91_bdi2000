#ifndef __BDICNF_H__
#define __BDICNF_H__
/*************************************************************************
|  COPYRIGHT (c) 2000 BY ABATRON AG
|*************************************************************************
|
|  PROJECT NAME: BDI Configuration Utility
|  FILENAME    : bdicnf.h
|
|  COMPILER    : GCC
|
|  TARGET OS   : LINUX
|  TARGET HW   : PC
|
|  PROGRAMMER  : Abatron / RD
|  CREATION    : 19.12.00
|
|*************************************************************************
|
|  DESCRIPTION :
|  Helper functions to build the BDI configuration structure
|
|
|*************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
|  DEFINES
|*************************************************************************/

#define CNF_MAX_MODE_SIZE        128
#define CNF_MAX_INIT_SIZE        1024


/* BDI type */
#define BDI_TYPE_HS              0
#define BDI_TYPE_20              1      /* BDI2000       */
#define BDI_TYPE_21              2      /* BDI2000 Rev.C */
#define BDI_TYPE_10              3      /* BDI1000       */
#define BDI_TYPE_LAST            3


/*************************************************************************
|  TYPEDEFS
|*************************************************************************/

typedef struct {
  DWORD   cmd;
  DWORD   addr;
  DWORD   value;
} CNF_InitEntryT;


/*************************************************************************
|  FUNCTIONS
|*************************************************************************/

int CNF_GetInitList(const char*           szFile,
                          WORD            bdiType,
                          WORD            fwType,
                          CNF_InitEntryT* initList);

int CNF_GetModeList(const char* szFile,
                          WORD  bdiType,
                          WORD  fwType,
                          BYTE* modeList);


#ifdef __cplusplus
}
#endif

#endif

