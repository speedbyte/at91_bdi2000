#ifndef usart_device_h
#define usart_device_h

extern void Usart_c_irq_handler(AT91PS_USART USART_pt); // to isr_usart.s
extern void AT91F_US_Printk( char *buffer) ;
extern void AT91F_US_Print_frame(char *buffer, unsigned short counter);
extern void AT91F_US_Print_2_frame(AT91PS_USART USART_pt, char *buffer, unsigned short counter,char *buffer2,unsigned short counter2);
extern int Usart_init ( void );



#endif
