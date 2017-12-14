/*
 * twiMaster.c
 *
 * Created: 04.12.2017 23:32:22
 *  Author: Martin Burger
 */ 

 #include <avr/io.h>
 #include "twiMaster.h"

 #define CPU_SPEED 32000000
 #define BAUDRATE    400000
 #define TWI_BAUD(F_SYS, F_TWI) ((F_SYS / (2 * F_TWI)) - 5)
 #define TWI_BAUDSETTING TWI_BAUD(CPU_SPEED, BAUDRATE)

 uint8_t i2cControl(uint8_t ack, i2cCmd_t cmd) {
	 ack = (ack > 0?1:0);
	 ack<<=2;
	 return ack | (cmd & 0x03);
 }

 void initI2C(void) {
	 PORTC.DIRSET = 0x03;
	 PORTC.OUT = 0x03;	 
	 TWIC.MASTER.CTRLC = i2cControl(1, NOACT);	 
	 TWIC.MASTER.BAUD = TWI_BAUDSETTING;
	 TWIC.MASTER.CTRLA = TWI_MASTER_ENABLE_bm;
	 TWIC.MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
 }

 void i2cRead(uint8_t add, uint8_t reg, uint8_t n, uint8_t* data) {
	 TWIC.MASTER.ADDR = add;
	 while(!(TWIC.MASTER.STATUS&TWI_MASTER_WIF_bm));
	 TWIC.MASTER.DATA = (reg);
	 TWIC.MASTER.ADDR = (add | 0x01);
	 TWIC.MASTER.CTRLC = i2cControl((n>1?1:0), START);
	 for(int i = 0; i < n; i++) {
		 while(!(TWIC.MASTER.STATUS&TWI_MASTER_RIF_bm));
		 data[i] = TWIC.MASTER.DATA;
		 TWIC.MASTER.CTRLC = i2cControl((i>=n-1?1:0), (i>=n-1?STOP:BYTEREC));
	 }	 
 }

 uint8_t i2cReadByte(uint8_t add, uint8_t reg) {
	 TWIC.MASTER.ADDR = add;
	 while(!(TWIC.MASTER.STATUS&TWI_MASTER_WIF_bm));
	 TWIC.MASTER.DATA = (reg);
	 TWIC.MASTER.ADDR = (add | 0x01);
	 while(!(TWIC.MASTER.STATUS&TWI_MASTER_RIF_bm));
	 uint8_t returndata = TWIC.MASTER.DATA;
	 TWIC.MASTER.CTRLC = i2cControl(0, STOP);
	 return returndata;
 }

 void i2cWrite(uint8_t add, uint8_t reg, uint8_t n, uint8_t* data) {
	 TWIC.MASTER.ADDR = add;
	 while(!(TWIC.MASTER.STATUS&TWI_MASTER_WIF_bm));
	 TWIC.MASTER.DATA = (reg);
	 while(!(TWIC.MASTER.STATUS&TWI_MASTER_WIF_bm));
	 for(int i = 0; i < n; i++) {
		 TWIC.MASTER.DATA = data[i];
		 while(!(TWIC.MASTER.STATUS&TWI_MASTER_WIF_bm));
	 }
	 TWIC.MASTER.CTRLC = i2cControl(0, STOP);
 }

 void i2cWriteByte(uint8_t add, uint8_t reg, uint8_t data) {
	 TWIC.MASTER.ADDR = add;
	 while(!(TWIC.MASTER.STATUS&TWI_MASTER_WIF_bm));
	 TWIC.MASTER.DATA = (reg);
	 while(!(TWIC.MASTER.STATUS&TWI_MASTER_WIF_bm));
	 TWIC.MASTER.DATA = data;
	 while(!(TWIC.MASTER.STATUS&TWI_MASTER_WIF_bm));
	 TWIC.MASTER.CTRLC = i2cControl(0, STOP);
 }