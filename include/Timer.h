#ifndef Timer_H
#define Timer_H

void initTimer(void);
unsigned int getTimerValue(void);
void StartTimer(void);
void StopTimer(void);
void Interrupt_Handler_TC0_Highlevel (void);

void Start_Latency_Measurement(void);
void Stop_Latency_Mesurement(void);



#endif
