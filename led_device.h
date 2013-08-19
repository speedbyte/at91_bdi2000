#ifndef led_device_h
#define led_device_h


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
       return ((AT91C_BASE_PIOB->PIO_OSR) & flag) != 0;
}

#endif
