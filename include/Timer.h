#ifndef Timer_H
#define Timer_H

void initTimer(void);
unsigned int getTimerValue(void);
void resetTimerValue(void);


extern void init_I_O(void);
char getDigInputState(AT91PS_PIO Port_pt, unsigned int Pin);







#endif
