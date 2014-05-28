#ifndef isr_h
#define isr_h

extern void 					Interrupt_Handler_USART_Lowlevel(void);
extern void 					Interrupt_Handler_MCI_Lowlevel(void);
extern void					Interrupt_Handler_SysTimer_Lowlevel(void);
extern void					Measured_Interrupt_Lowlevel(void);
extern void					Interrupt_Handler_TC0_Lowlevel(void);

#endif
