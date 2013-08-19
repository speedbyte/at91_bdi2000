#ifndef led_device_h
#define led_device_h


#define GREEN  ((unsigned char) (1<<0))
#define YELLOW ((unsigned char) (1<<1))
#define RED    ((unsigned char) (1<<2))


extern void Led_init(void);

static inline void setLed(unsigned char flag)
{
       AT91C_BASE_PIOB->PIO_OER |= flag;
}
    
static inline void resetLed(unsigned char flag)
{
       AT91C_BASE_PIOB->PIO_ODR |= flag;
}
    
static inline unsigned char getLed(unsigned char flag) 
{
       return ((AT91C_BASE_PIOB->PIO_OSR) & flag);
}

#endif
