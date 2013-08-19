#ifndef st_device_h
#define st_device_h


extern void AT91F_ST_HANDLER(void); // to isr.S
extern void St_init();
extern unsigned int AT91F_GetTickCount(void);
void AT91F_ST_SetPeriodIntervalTimer(	AT91PS_ST pSt,	unsigned int period);

#endif
