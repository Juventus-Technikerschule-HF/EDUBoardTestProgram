/*
 * NHD0420Driver.c
 *
 * Created: 15.05.2017 08:32:12
 *  Author: mburger
 */ 
 #include <avr/io.h>
 #include <avr/interrupt.h>
 #include "NHD0420Driver.h"
 

 uint32_t cpuFrequency = 32000000;

 uint8_t displayLine = 0;
 uint8_t displayPos = 0;
 char displayBuffer[4][20];

 void delayUS(uint32_t us) {
	uint32_t n = cpuFrequency/1000000;
	n*=us;
	if(n > 1) {
		for(int i = 0; i < n; i++) {
			asm volatile("nop");
		}
	} else {
		asm volatile("nop");
	}
 }
 void setPort(uint8_t data) {
	data &= 0x0F;
	data <<= 4;
	PORTA.OUT &= (data | 0x0F);
	PORTA.OUT |= data;
 }
 void setRS(char value) {
	if(value > 0) {
		PORTD.OUTSET = PIN0_bm;
	} else {
		PORTD.OUTCLR = PIN0_bm;
	}
 }
 void setRW(char value) {
	if(value > 0) {
		PORTD.OUTSET = PIN1_bm;
		} else {
		PORTD.OUTCLR = PIN1_bm;
	}
 }
 void setE(char value) {
	if(value > 0) {
		PORTD.OUTSET = PIN2_bm;
		} else {
		PORTD.OUTCLR = PIN2_bm;
	}
 }
 void Nybble() {
	setE(1);
	delayUS(1);
	setE(0);
 }
 void command(char i) {
	setPort((i>>4)&0x0F);
	setRS(0);
	setRW(0);
	Nybble();
	setPort(i & 0x0F);
	Nybble();
 }
 void write(char i) {
	setPort((i>>4)&0x0F);
	setRS(1);
	setRW(0);
	Nybble();
	setPort(i & 0x0F);
	Nybble();
 }

 void displayCPUClockConfig(uint32_t frequency) {
	cpuFrequency = frequency;
 }

 void initDisplayTimer(uint8_t updateRateHz) {
	//TCF0.CTRLA = TC_TC0_CLKSEL_DIV64_gc;
	TCF0.CTRLA = TC_CLKSEL_DIV64_gc;
	TCF0.CTRLB = 0x00;
	TCF0.INTCTRLA = 0x02;
	TCF0.PER = (uint16_t) (500000 / updateRateHz);
 }

 void displayInit() {
	PORTA.DIRSET = PIN4_bm;
	PORTA.DIRSET = PIN5_bm;
	PORTA.DIRSET = PIN6_bm;
	PORTA.DIRSET = PIN7_bm;
	PORTD.DIRSET = PIN0_bm;
	PORTD.DIRSET = PIN1_bm;
	PORTD.DIRSET = PIN2_bm;
	PORTA.OUT &= 0x0F;
	PORTD.OUT &= 0xF8;

	delayUS(40000);
	setPort(0x03);
	delayUS(5000);
	Nybble();
	delayUS(160);
	Nybble();
	delayUS(160);
	Nybble();
	delayUS(160);
	setPort(0x02);
	Nybble();
	command(0x28);
	command(0x10);
	command(0x0F);
	command(0x06);

	displayBufferClear();
	initDisplayTimer(10);
	
 }
 void displaySetPos(int line, int pos) {
	switch(line) {
		case 0: 
			command(0x80 + 0x00 + pos);
		break;
		case 1: 
			command(0x80 + 0x40 + pos);
		break;
		case 2:
			command(0x80 + 0x14 + pos);
		break;
		case 3:
			command(0x80 + 0x54 + pos);
		break;
	}
	delayUS(39);
 }
 void displayHome() {
	command(0x02);
 }
 void displayClear() {
	command(0x01);
	delayUS(1530);
 }
 void displayWriteChar(char c) {
	write(c);
	delayUS(43);
 }
 void displayWriteCharAtPos(int line, int pos, char c) {
	displaySetPos(line, pos);
	displayWriteChar(c);
 }
 void displayWriteString(char* s) {
	for(int i = 0; i < 20; i++) {
		if(s[i] == '\0') {
			break;
		}
		displayWriteChar(s[i]);
	}
 }
 void displayWriteStringAtPos(int line, int pos, char* s) {
	displaySetPos(line, pos);
	displayWriteString(s);
 } 

void displayBufferSetPos(int line, int pos) {
	displayLine = line;
	displayPos = pos;
}
void displayBufferHome() {
	displayLine = 0;
	displayPos = 0;
}
void displayBufferClear() {
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 20; j++) {
			displayBuffer[i][j] = ' ';
		}
	}
}
void displayBufferWriteChar(char c) {
	displayBuffer[displayLine][displayPos] = c;
	displayPos++;
	if(displayPos >= 20) {
		displayPos = 0;
		displayLine++;
	}
	if(displayLine >= 4) {
		displayLine = 0;
	}
}
void displayBufferWriteCharAtPos(int line, int pos, char c) {
	displayLine = line;
	displayPos = pos;
	displayBufferWriteChar(c);
}
void displayBufferWriteString(char* s) {
	for(int i = 0; i < 20; i++) {
		if(s[i] == '\0') {
			break;
		}
		displayBufferWriteChar(s[i]);
	}
}
void displayBufferWriteStringAtPos(int line, int pos, char* s) {
	displayLine = line;
	displayPos = pos;
	displayBufferWriteString(s);
}

void displayForceUpdateBuffer(void) {
	for(int i = 0; i < 4; i++) {
		displayWriteStringAtPos(i,0,&displayBuffer[i][0]);
	}
}
uint8_t displayUpdate = 0;
void displayUpdateWorker(void) {
	if(displayUpdate > 0) {
		displayUpdate = 0;
		for(int i = 0; i < 4; i++) {
			displayWriteStringAtPos(i,0,&displayBuffer[i][0]);
		}	
	}
}
 ISR(TCF0_OVF_vect) {
	//Timer F0 Overflow
	//Update Display
	//Write Buffer to Display
	displayUpdate = 1;
 }
