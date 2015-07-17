#ifndef Conversion_H
#define Conversion_H


void WaitTicks(unsigned int ticks);
unsigned int Convert_Ticks_To_us (unsigned int Ticks,unsigned char TimerClockBase);
void Dec2ASCII_Ticks(unsigned int value,unsigned char blanksym);
void PutDateAndTimeStamp(unsigned char datedivider,unsigned char timedivider);
#endif
