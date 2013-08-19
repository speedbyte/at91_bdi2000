#ifndef usart_device_h
#define usart_device_h


#define	MASTER_CLOCK	60000000
#define USART_INTERRUPT_LEVEL		7

extern void Usart_c_irq_handler(AT91PS_USART USART_pt); // to isr_usart.s
extern void AT91F_US_Print_2_frame(AT91PS_USART USART_pt, char *buffer, unsigned short counter,char *buffer2,unsigned short counter2);
extern void Usart_init ( void );



#endif
