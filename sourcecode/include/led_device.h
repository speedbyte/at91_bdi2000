#ifndef led_device_h
#define led_device_h


#define GREEN  ((unsigned char) (1<<0))
#define YELLOW ((unsigned char) (1<<1))
#define RED    ((unsigned char) (1<<2))


extern void Led_init(void);
void  toggleLed(unsigned char flag);


static inline void setLed(unsigned char flag)
{
	   AT91C_BASE_PIOB->PIO_CODR =flag;
}
    
static inline void resetLed(unsigned char flag)
{
	   AT91C_BASE_PIOB->PIO_SODR =flag;
}
    
static inline unsigned char getLed(unsigned char flag) 
{
       return (unsigned char)((~(AT91C_BASE_PIOB->PIO_ODSR)) & flag);
}




#endif
