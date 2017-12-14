/*
 * EDUBoardTestProgram.c
 *
 * Created: 22.04.2017 20:01:40
 * Author : mburger
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "NHD0420Driver.h"
#include "LSM9DS1Driver.h"
#include "NumberStringConverter.h"
#include "GPIODebug.h"
#include "ButtonHandler.h"

#define SOFTWARE_VERSION 1.1

uint8_t hundredMsFlag = 0;
uint8_t tenMsFlag = 0;

void initPorts() {
	PORTF.DIRSET = PIN0_bm; //LED1
	PORTF.DIRSET = PIN1_bm; //LED2
	PORTF.DIRSET = PIN2_bm; //LED3
	PORTF.DIRSET = PIN3_bm; //LED4
	
	PORTE.DIRSET = PIN0_bm; //LED5
	PORTE.DIRSET = PIN1_bm; //LED6
	PORTE.DIRSET = PIN2_bm; //LED7
	PORTE.DIRSET = PIN3_bm; //LED8	

	PORTE.DIRSET = PIN7_bm;
	PORTE.DIRCLR = PIN6_bm;

	PORTC.DIRSET = PIN3_bm;
	PORTC.DIRCLR = PIN2_bm;
	
}

void writeCCP(volatile register8_t* address, uint8_t value) {
	volatile uint8_t* tmpAddr = address;
	asm volatile("movw r30, %0"  "\n\t"
				 "LDI r16,0xD8"  "\n\t"				 
				 "out %2,r16"    "\n\t"
				 "st Z, %1"    "\n\t"
				 :
				 : "r" (tmpAddr), "r" (value), "i"(&CCP)
				 : "r16", "r30", "r31"
				 );
}
void clockInit(void) {	
	OSC.XOSCCTRL = OSC_FRQRANGE_2TO9_gc | OSC_XOSCSEL_XTAL_256CLK_gc | OSC_XOSCPWR_bm;
	OSC.CTRL |= OSC_XOSCEN_bm;
	while(!(OSC.STATUS & OSC_XOSCRDY_bm));
	//OSC.CTRL |= OSC_RC32MEN_bm;
	//while(!(OSC.STATUS & OSC_RC32MRDY_bm));
	OSC.PLLCTRL = (OSC_PLLSRC_XOSC_gc | 4); //PLL Multiplication Factor
	OSC.CTRL |= OSC_PLLEN_bm;		
	while(!(OSC.STATUS & OSC_PLLRDY_bm));	
	writeCCP(&CLK.CTRL, CLK_SCLKSEL_PLL_gc);	
	//writeCCP(&CLK.CTRL, CLK_SCLKSEL_RC32M_gc);
	
}

void initTimer(void) {	
	
	//TCC0.CTRLA = TC_TC0_CLKSEL_DIV64_gc;	
	TCC0.CTRLA = TC_CLKSEL_DIV64_gc;
	TCC0.CTRLB = 0x00;
	TCC0.INTCTRLA = 0x03;
	TCC0.PER = 500;	

	//Config OC1A & OC1B as PWM outputs an  PD4 und PD5
	PORTD.DIR |= 0x30;
	TCD1.CTRLA = TC_CLKSEL_DIV8_gc;	
	TCD1.CTRLB = TC_WGMODE_SINGLESLOPE_gc | TC0_CCAEN_bm | TC0_CCBEN_bm;	
	TCD1.CTRLD = 0x00;
	TCD1.PER = 40000;	
	TCD1.CCA = 35500; //Umschaltwert für Servo1
	TCD1.CCB = 35500; //Umschaltwert für Servo2	
}

uint16_t adResult0 = 0;
uint16_t adResult1 = 0;

void initADC(void) {
	
	ADCA.CTRLA = 0x01; //Enable ADC0;
	ADCA.CTRLB = 0x00; 
	ADCA.REFCTRL = 0x12;
	ADCA.PRESCALER = 0x03;	
	ADCA.CH0.CTRL = 0x01; //Startbit at MSB
	ADCA.CH0.MUXCTRL = (8 << 3) | (7 << 0);
	ADCA.CH0.INTCTRL = 0x03;
	ADCB.CTRLA = 0x01;
	ADCB.CTRLB = 0x00;
	ADCB.REFCTRL = 0x12;
	ADCB.PRESCALER = 0x03;
	ADCB.CH0.CTRL = 0x01;
	ADCB.CH0.MUXCTRL = (9<<3) | (7<<0);
	ADCB.CH0.INTCTRL = 0x03;
	
}

void initUART(void) {
	USARTE1.CTRLA = USART_RXCINTLVL_HI_gc | USART_TXCINTLVL_HI_gc | USART_DREINTLVL_LO_gc;
	USARTE1.CTRLB = USART_RXEN_bm | USART_TXEN_bm;
	USARTE1.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
	USARTE1.BAUDCTRLA = 207;
	USARTE1.BAUDCTRLB = 0;

	USARTC0.CTRLA = USART_RXCINTLVL_HI_gc | USART_TXCINTLVL_HI_gc | USART_DREINTLVL_LO_gc;
	USARTC0.CTRLB = USART_RXEN_bm | USART_TXEN_bm;
	USARTC0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
	USARTC0.BAUDCTRLA = 207;
	USARTC0.BAUDCTRLB = 0;	
}

typedef enum mode_tag {
	MODE_INIT,
	MODE_SCREEN_MAIN,
	MODE_SCREEN_TEMP,
	MODE_SCREEN_GYRO,
	MODE_SCREEN_ACC,
	MODE_SCREEN_MAG,
	MODE_SCREEN_LED,
	MODE_SCREEN_BUTTON,
	MODE_SCREEN_AD,
	MODE_SCREEN_UART
} mode_t;

#define POTI1	0
#define POTI2	1
void setPotentiometer(unsigned int Poti, float value) {
	if((value >= 0.0) && (value <= 100.0)) {
		unsigned int captureValue = (unsigned int)32000 + 4000 - ((value/100) * 4000);
		switch(Poti) {
			case 0: 
				TCD1.CCA = captureValue;
			break;
			case 1:
				TCD1.CCB = captureValue;
			break;
		}
	}
}

void modeHandler(void) {
	static mode_t programmode = MODE_INIT;
	static uint8_t waitTimer = 50;
	static uint8_t actualLed = 0;
	static uint8_t buttonDelay = 0;
	char numberString[6] = "      ";
	//setPin(OFF);
	if(buttonDelay == 0) {
		displayBufferClear();
	}
	switch(programmode) {
		case MODE_INIT:
			displayBufferWriteStringAtPos(0,4, "Init\0");
			if(waitTimer == 0) {
				programmode = MODE_SCREEN_MAIN;
			}
		break;
		case MODE_SCREEN_MAIN:
			displayBufferWriteStringAtPos(0,4, "Main-Screen");
			displayBufferWriteStringAtPos(1, 0, "EDUBoardTestProgram");			
			convert_float_string(SOFTWARE_VERSION, numberString, 1);
			displayBufferWriteStringAtPos(2,4, "Version: ");
			displayBufferWriteStringAtPos(2, 13, numberString);
			displayBufferWriteStringAtPos(3,0, ">Press Button1<");
			if(getButtonPress(BUTTON1) == SHORT_PRESSED) {
				programmode = MODE_SCREEN_TEMP;
			}
		break;
		case MODE_SCREEN_TEMP:
			displayBufferWriteStringAtPos(0,4,"Temperature:");
			convert_float_string(getTemperatureData(), numberString, 2);
			displayBufferWriteStringAtPos(2,4,"Temp:       C");
			displayBufferWriteCharAtPos(2,15,0xDF);
			displayBufferWriteStringAtPos(2,10, numberString);
			displayBufferWriteStringAtPos(3,0, ">Press Button1<");
			if(getButtonPress(BUTTON1) == SHORT_PRESSED) {
				programmode = MODE_SCREEN_GYRO;
			}
		break;
		case MODE_SCREEN_GYRO:
			displayBufferWriteStringAtPos(0,4,"Gyroscope:");
			displayBufferWriteStringAtPos(1,2, "x     y     z");
			convert_sint_string(getGyroData(X_AXIS), numberString);
			displayBufferWriteStringAtPos(2, 0, numberString);
			convert_sint_string(getGyroData(Y_AXIS), numberString);
			displayBufferWriteStringAtPos(2, 7, numberString);
			convert_sint_string(getGyroData(Z_AXIS), numberString);
			displayBufferWriteStringAtPos(2, 14, numberString);
			displayBufferWriteStringAtPos(3,0, ">Press Button1<");
			if(getButtonPress(BUTTON1) == SHORT_PRESSED) {
				programmode = MODE_SCREEN_ACC;
			}
		break;
		case MODE_SCREEN_ACC:

			setPotentiometer(POTI1, (50+((getACCData(X_AXIS)/80))));
			setPotentiometer(POTI2, (50+((getACCData(Y_AXIS)/80))));

			displayBufferWriteStringAtPos(0,4,"Accelerometer:");
			displayBufferWriteStringAtPos(1,2, "x     y     z");
			convert_sint_string(getACCData(X_AXIS), numberString);
			displayBufferWriteStringAtPos(2, 0, numberString);
			convert_sint_string(getACCData(Y_AXIS), numberString);
			displayBufferWriteStringAtPos(2, 7, numberString);
			convert_sint_string(getACCData(Z_AXIS), numberString);
			displayBufferWriteStringAtPos(2, 14, numberString);
			displayBufferWriteStringAtPos(3,0, ">Press Button1<");
			if(getButtonPress(BUTTON1) == SHORT_PRESSED) {
				programmode = MODE_SCREEN_MAG;
			}
		break;
		case MODE_SCREEN_MAG:
			displayBufferWriteStringAtPos(0,4,"Magnetometer:");
			displayBufferWriteStringAtPos(1,2, "x     y     z");
			convert_sint_string(getMagData(X_AXIS), numberString);
			displayBufferWriteStringAtPos(2, 0, numberString);
			convert_sint_string(getMagData(Y_AXIS), numberString);
			displayBufferWriteStringAtPos(2, 7, numberString);
			convert_sint_string(getMagData(Z_AXIS), numberString);
			displayBufferWriteStringAtPos(2, 14, numberString);
			displayBufferWriteStringAtPos(3,0, ">Press Button1<");
			if(getButtonPress(BUTTON1) == SHORT_PRESSED) {
				programmode = MODE_SCREEN_LED;
			}
		break;
		case MODE_SCREEN_LED:			
			displayBufferWriteStringAtPos(0,4,"LED-Test:");
			displayBufferWriteStringAtPos(3,0, ">Press Button1<");
			if(waitTimer == 0) {
				waitTimer = 10;
				PORTF.OUT |= 0x0F;
				PORTE.OUT |= 0x0F;
				switch(actualLed) {
					case 0: PORTF.OUT &= ~(0x01 << actualLed); break;
					case 1: PORTF.OUT &= ~(0x01 << actualLed); break;
					case 2: PORTF.OUT &= ~(0x01 << actualLed); break;
					case 3: PORTF.OUT &= ~(0x01 << actualLed); break;
					case 4: PORTE.OUT &= ~(0x01 << (actualLed-4)); break;
					case 5: PORTE.OUT &= ~(0x01 << (actualLed-4)); break;
					case 6: PORTE.OUT &= ~(0x01 << (actualLed-4)); break;
					case 7: PORTE.OUT &= ~(0x01 << (actualLed-4)); break;
				}
				actualLed++;
				if(actualLed > 7) {
					actualLed = 0;
				}
			}
			if(getButtonPress(BUTTON1) == SHORT_PRESSED) {
				waitTimer = 0;
				PORTF.OUT |= 0x0F;
				PORTE.OUT |= 0x0F;
				programmode = MODE_SCREEN_BUTTON;
			}
		break;
		case MODE_SCREEN_BUTTON:
			displayBufferWriteStringAtPos(0,4,"Button-Test:");
			displayBufferWriteStringAtPos(1,0,"Button: ");
			displayBufferWriteStringAtPos(2,0,"Type:   ");
			if(getButtonPress(BUTTON2) == SHORT_PRESSED) {
				displayBufferWriteStringAtPos(1,8,"B2");
				displayBufferWriteStringAtPos(2,8,"Short");
				buttonDelay = 50;
			}
			if(getButtonPress(BUTTON2) == LONG_PRESSED) {
				displayBufferWriteStringAtPos(1,8,"B2");
				displayBufferWriteStringAtPos(2,8,"Long");
				buttonDelay = 50;
			}
			if(getButtonPress(BUTTON3) == SHORT_PRESSED) {
				displayBufferWriteStringAtPos(1,8,"B3");
				displayBufferWriteStringAtPos(2,8,"Short");
				buttonDelay = 50;
			}
			if(getButtonPress(BUTTON3) == LONG_PRESSED) {
				displayBufferWriteStringAtPos(1,8,"B3");
				displayBufferWriteStringAtPos(2,8,"Long");
				buttonDelay = 50;
			}
			if(getButtonPress(BUTTON4) == SHORT_PRESSED) {
				displayBufferWriteStringAtPos(1,8,"B4");
				displayBufferWriteStringAtPos(2,8,"Short");
				buttonDelay = 50;
			}
			if(getButtonPress(BUTTON4) == LONG_PRESSED) {
				displayBufferWriteStringAtPos(1,8,"B4");
				displayBufferWriteStringAtPos(2,8,"Long");
				buttonDelay = 50;
			}
			displayBufferWriteStringAtPos(3,0, ">Press Button1<");
			if(getButtonPress(BUTTON1) == SHORT_PRESSED) {
				buttonDelay = 0;
				programmode = MODE_SCREEN_AD;
			}
		break;
		case MODE_SCREEN_AD:
			displayBufferWriteStringAtPos(0,4,"AD-Values:");
			displayBufferWriteStringAtPos(1,0,"Pot1: ");
			convert_uint_string(adResult0, numberString);
			displayBufferWriteStringAtPos(1, 6, numberString);
			displayBufferWriteStringAtPos(2,0,"Pot2: ");
			convert_uint_string(adResult1, numberString);
			displayBufferWriteStringAtPos(2,6,numberString);
			displayBufferWriteStringAtPos(3,0, ">Press Button1<");
			if(getButtonPress(BUTTON1) == SHORT_PRESSED) {
				programmode = MODE_SCREEN_UART;
			}
		break;
		case MODE_SCREEN_UART:
			displayBufferWriteStringAtPos(0,4,"UART-Monitor:");
			displayBufferWriteStringAtPos(3,0, ">Press Button1<");
			if(getButtonPress(BUTTON1) == SHORT_PRESSED) {
				programmode = MODE_SCREEN_MAIN;
			}
		break;
	}
	if(waitTimer > 0) {
		waitTimer--;
	}
	if(buttonDelay > 0) {
		buttonDelay--;
	}
	//setPin(ON);
}



int main(void)
{
    clockInit();
	initPorts();
	initButtons();
	setupPin();
	
	displayInit();
	LSM9DS1Init();
	initTimer();
	initADC();
	initUART();

	setPotentiometer(POTI1, 50);
	setPotentiometer(POTI2, 50);

	PMIC.CTRL |= PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;	
	sei();		
	
	displayBufferClear();
	
	
	PORTF.OUT |= 0x0F;
	PORTE.OUT |= 0x0F;
		
    while (1) 
    {
		//setPin(OFF);
		displayUpdateWorker();
		//setPin(ON);
		if(tenMsFlag != 0) {
			tenMsFlag = 0;
			updateButtons();
			modeHandler();
		}

		if(hundredMsFlag != 0) {			
			hundredMsFlag = 0;
			readTempData();
			readGyroData();
			readACCData();
			readMagData();			
		}		
    }
}

ISR(TCC0_OVF_vect) 
{	
	static uint8_t count = 1;
	
	ADCA.CH0.CTRL |= 0x80; //Startbit at MSB
	ADCB.CH0.CTRL |= 0x80;
	if(count % 10 == 0) {		
		tenMsFlag = 1;		
	}
	if(count >= 100) {
		count = 1;
		hundredMsFlag = 1;
	} else {
		count++;
	}	
}
ISR(ADCA_CH0_vect) {
	adResult0 = ADCA.CH0.RES;
	//ADCA.CH0.INTFLAGS = 0x01;
}
ISR(ADCB_CH0_vect) {
	adResult1 = ADCB.CH0.RES;
}

ISR(USARTE1_RXC_vect) {
	//setPin(OFF);
	char c = USARTE1.DATA;
	USARTE1.DATA = c;	
}

ISR(USARTE1_TXC_vect) {
	//setPin(ON);
}

ISR(USARTC0_RXC_vect) {
	char c = USARTC0.DATA;
	USARTC0.DATA = c;
}

ISR(USARTC0_TXC_vect) {

}
