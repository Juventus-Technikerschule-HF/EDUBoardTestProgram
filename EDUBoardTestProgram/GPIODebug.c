/*
 * GPIODebug.c
 *
 * Created: 19.05.2017 13:25:12
 *  Author: mburger
 */ 

 #include <avr/io.h>
 #include "GPIODebug.h"

 void setupPin() {
	PORTE.DIR |= PIN3_bm;
 }

 void setPin(onOff_t state) {
	if(state == ON) {
		PORTE.OUT |= PIN3_bm;
	} else {
		PORTE.OUT &= ~PIN3_bm;
	}
 }
 void togglePin() {	
	if((PORTE.IN & PIN3_bm) == PIN3_bm) {
		PORTE.OUT &= ~PIN3_bm;
		} else {
		PORTE.OUT |= PIN3_bm;
	}
 }