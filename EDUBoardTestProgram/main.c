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

void initPorts() {
	PORTF.DIRSET = PIN0_bm; //LED1
	PORTF.DIRSET = PIN1_bm; //LED2
	PORTF.DIRSET = PIN2_bm; //LED3
	PORTF.DIRSET = PIN3_bm; //LED4
	PORTF.DIRCLR = PIN4_bm; //SW1
	PORTF.DIRCLR = PIN5_bm; //SW2
	PORTF.DIRCLR = PIN6_bm; //SW3
	PORTF.DIRCLR = PIN7_bm; //SW4
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

uint8_t hundredMsFlag = 0;

int main(void)
{
    clockInit();
	initPorts();
	setupPin();
	
	displayInit();
	LSM9DS1Init();
	initTimer();
	initADC();
	initUART();
	PMIC.CTRL |= PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;	
	sei();		
	
	displayBufferClear();
	
	
	PORTF.OUT |= 0x0F;
	PORTE.OUT |= 0x0F;


	
	char line1String[] = "Temp:      AD:";
	char line2String[] = "Gyro:";
	char line3String[] = "ACC :";
	char line4String[] = "MAG :";
	char numberString[6];
	
    while (1) 
    {
		displayUpdateWorker();
		if(hundredMsFlag != 0) {			
			hundredMsFlag = 0;
			//setPin(OFF);
			//USARTE1.DATA = 'M';
			readTempData();
			readGyroData();
			readACCData();
			readMagData();
			displayBufferClear();
			displayBufferWriteStringAtPos(LINE_1, 0, line1String);
			displayBufferWriteStringAtPos(LINE_2, 0, line2String);
			displayBufferWriteStringAtPos(LINE_3, 0, line3String);
			displayBufferWriteStringAtPos(LINE_4, 0, line4String);
			convert_float_string(getTemperatureData(), numberString, 2);
			displayBufferWriteStringAtPos(LINE_1, 5, numberString);
			convert_uint_string(adResult0, numberString);
			displayBufferWriteStringAtPos(LINE_1, 14, numberString);

			convert_sint_string(getGyroData(X_AXIS), numberString);
			displayBufferWriteStringAtPos(LINE_2, 5, numberString);
			convert_sint_string(getGyroData(Y_AXIS), numberString);
			displayBufferWriteStringAtPos(LINE_2, 10, numberString);
			convert_sint_string(getGyroData(Z_AXIS), numberString);
			displayBufferWriteStringAtPos(LINE_2, 15, numberString);

			convert_sint_string(getACCData(X_AXIS), numberString);
			displayBufferWriteStringAtPos(LINE_3, 5, numberString);
			convert_sint_string(getACCData(Y_AXIS), numberString);
			displayBufferWriteStringAtPos(LINE_3, 10, numberString);
			convert_sint_string(getACCData(Z_AXIS), numberString);
			displayBufferWriteStringAtPos(LINE_3, 15, numberString);

			convert_sint_string(getMagData(X_AXIS), numberString);
			displayBufferWriteStringAtPos(LINE_4, 5, numberString);
			convert_sint_string(getMagData(Y_AXIS), numberString);
			displayBufferWriteStringAtPos(LINE_4, 10, numberString);
			convert_sint_string(getMagData(Z_AXIS), numberString);
			displayBufferWriteStringAtPos(LINE_4, 15, numberString);
			//setPin(ON);
		}		
    }
}

ISR(TCC0_OVF_vect) 
{	
	static uint8_t count = 0;
	/*if(PORTF.IN & 0x01 == 0x01) {
		PORTF.OUT &= 0xFE;
	} else {
		PORTF.OUT |= 0x01;
	}*/
	ADCA.CH0.CTRL |= 0x80; //Startbit at MSB
	if(count >= 100) {
		count = 0;
		hundredMsFlag = 1;
	} else {
		count++;
	}
}
ISR(ADCA_CH0_vect) {
	adResult0 = ADCA.CH0.RES;
	//ADCA.CH0.INTFLAGS = 0x01;
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

