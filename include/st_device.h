#ifndef st_device_h
#define st_device_h


extern void 			Interrupt_Handler_SysTimer_Highlevel(void); // to isr.S
extern void 			St_init();
extern unsigned int 	GetTickCount_fromST(void);


#endif
