#ifndef Timer_H
#define Timer_H

void initTimer(void);
void startTimerOnetime(int duration_us);
void startTimerPermanent(int duration_us);
void stopTimerActions(void);


extern void init_I_O(void);
char getDigInputState(AT91PS_PIO Port_pt, unsigned int Pin);







#endif