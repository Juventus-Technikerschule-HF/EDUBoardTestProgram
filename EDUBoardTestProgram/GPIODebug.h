/*
 * GPIODebug.h
 *
 * Created: 19.05.2017 13:25:25
 *  Author: mburger
 */ 


#ifndef GPIODEBUG_H_
#define GPIODEBUG_H_

typedef enum {
	OFF = 0,
	ON  = 1
} onOff_t;

void setupPin();
void setPin(onOff_t state);
void togglePin();

#endif /* GPIODEBUG_H_ */