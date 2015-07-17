#ifndef Timer_H
#define Timer_H

void 				initTimer(unsigned char TimerClockBase);
unsigned int 		getTimerValue(void);
void 				StartTimer(void);
void 				StopTimer(void);
unsigned int 		Time_Used_In_Timer_Int(void);
void 				Interrupt_Handler_TC0_Highlevel (void);

#endif
