/*
 * LSM9DS1Driver.c
 *
 * Created: 15.05.2017 10:33:07
 *  Author: mburger
 */ 
 #include <avr/io.h>
 #include "LSM9DS1Driver.h" 
 #include "LSM9DS1Defines.h"
 #include "twiMaster.h"
 
 /* 
 #define CPU_SPEED 32000000
 #define BAUDRATE    400000
 #define TWI_BAUD(F_SYS, F_TWI) ((F_SYS / (2 * F_TWI)) - 5)
 #define TWI_BAUDSETTING TWI_BAUD(CPU_SPEED, BAUDRATE)
 */

 int16_t gx = 0;
 int16_t gy = 0;
 int16_t gz = 0;
 int16_t ax = 0;
 int16_t ay = 0;
 int16_t az = 0;
 int16_t mx = 0;
 int16_t my = 0;
 int16_t mz = 0;
 float temperature = 0;
 
//  typedef enum i2cCmd_Tag{
// 	NOACT = 0, 
// 	START = 1, 
// 	BYTEREC = 2, 
// 	STOP = 3
// }i2cCmd_t;
// 
//  uint8_t i2cControl(uint8_t ack, i2cCmd_t cmd) {
// 	ack = (ack > 0?1:0);
// 	ack<<=2;
// 	return ack | (cmd & 0x03);
//  }
//  void initI2C(void) {
// 	PORTC.DIRSET = 0x03;
// 	PORTC.OUT = 0x03;		
// 	TWIC.MASTER.CTRLC = i2cControl(1, NOACT);
// 	//TWIC.MASTER.CTRLB = TWI_MASTER_SMEN_bm;
// 	TWIC.MASTER.BAUD = TWI_BAUDSETTING;
// 	TWIC.MASTER.CTRLA = TWI_MASTER_ENABLE_bm;
// 	TWIC.MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;	
//  }
//  void i2cRead(uint8_t add, uint8_t reg, uint8_t n, uint8_t* data) {
// 	TWIC.MASTER.ADDR = add; //WriteAddress
// 	while(!(TWIC.MASTER.STATUS&TWI_MASTER_WIF_bm));
// 	TWIC.MASTER.DATA = (reg /*| (n > 1 ? 0x80 : 0x00)*/); //AutoIncrement
// 	TWIC.MASTER.ADDR = (add | 0x01); //ReadAddress
// 	TWIC.MASTER.CTRLC = i2cControl((n>1?1:0), START);
// 	for(int i = 0; i < n; i++) {
// 		while(!(TWIC.MASTER.STATUS&TWI_MASTER_RIF_bm));
// 		data[i] = TWIC.MASTER.DATA;
// 		TWIC.MASTER.CTRLC = i2cControl((i>=n-1?1:0), (i>=n-1?STOP:BYTEREC));
// 	}	
// 	//TWIC.MASTER.CTRLC = i2cControl(0, STOP);
//  }
//  uint8_t i2cReadByte(uint8_t add, uint8_t reg) {
// 	TWIC.MASTER.ADDR = add; //WriteAddress
// 	while(!(TWIC.MASTER.STATUS&TWI_MASTER_WIF_bm));
// 	TWIC.MASTER.DATA = (reg); //AutoIncrement
// 	TWIC.MASTER.ADDR = (add | 0x01); //ReadAddress
// 	while(!(TWIC.MASTER.STATUS&TWI_MASTER_RIF_bm));
// 	uint8_t returndata = TWIC.MASTER.DATA;
// 	TWIC.MASTER.CTRLC = i2cControl(0, STOP);
// 	return returndata;
//  }
//  void i2cWrite(uint8_t add, uint8_t reg, uint8_t n, uint8_t* data) {
// 	TWIC.MASTER.ADDR = add; //WriteAddress
// 	while(!(TWIC.MASTER.STATUS&TWI_MASTER_WIF_bm));
// 	TWIC.MASTER.DATA = (reg /*| (n > 1 ? 0x80 : 0x00)*/); //AutoIncrement
// 	while(!(TWIC.MASTER.STATUS&TWI_MASTER_WIF_bm));
// 	for(int i = 0; i < n; i++) {
// 		TWIC.MASTER.DATA = data[i];
// 		while(!(TWIC.MASTER.STATUS&TWI_MASTER_WIF_bm));
// 	}
// 	TWIC.MASTER.CTRLC = i2cControl(0, STOP);
//  }
//  void i2cWriteByte(uint8_t add, uint8_t reg, uint8_t data) {
// 	 TWIC.MASTER.ADDR = add; //WriteAddress
// 	 while(!(TWIC.MASTER.STATUS&TWI_MASTER_WIF_bm));
// 	 TWIC.MASTER.DATA = (reg); //AutoIncrement
// 	 while(!(TWIC.MASTER.STATUS&TWI_MASTER_WIF_bm));	 
// 	 TWIC.MASTER.DATA = data;
// 	 while(!(TWIC.MASTER.STATUS&TWI_MASTER_WIF_bm));	
// 	 TWIC.MASTER.CTRLC = i2cControl(0, STOP);
//  }

 void LSM9DS1Init(void) {	
	initI2C();	
	//InitGyro
	i2cWriteByte(GYROACCADDRESS, CTRL_REG1_G, 0xC8); //Add Datarate & Scale - Full Scale = 500dps
	i2cWriteByte(GYROACCADDRESS, CTRL_REG2_G, 0x00); 
	i2cWriteByte(GYROACCADDRESS, CTRL_REG3_G, 0x00);
	
	//InitACC	
	i2cWriteByte(GYROACCADDRESS, CTRL_REG6_XL, 0x98); //Add Datarate & Scale
	i2cWriteByte(GYROACCADDRESS, CTRL_REG7_XL, 0x00);
		
	//General Gyro & ACC Config
	i2cWriteByte(GYROACCADDRESS, CTRL_REG8, 0x04);
	i2cWriteByte(GYROACCADDRESS, CTRL_REG9, 0x10);
	i2cWriteByte(GYROACCADDRESS, FIFO_CTRL, 0xC0);

	//InitMag
	i2cWriteByte(MAGADDRESS, CTRL_REG1_M, 0x1C); //Add Datarate
	i2cWriteByte(MAGADDRESS, CTRL_REG2_M, 0x00); //Set Scale
	i2cWriteByte(MAGADDRESS, CTRL_REG3_M, 0x00);
	i2cWriteByte(MAGADDRESS, CTRL_REG4_M, 0x00);
	i2cWriteByte(MAGADDRESS, CTRL_REG5_M, 0x00);

	//Enable Gyro and ACC
	i2cWriteByte(GYROACCADDRESS, CTRL_REG4, 0x38);
	i2cWriteByte(GYROACCADDRESS, CTRL_REG5_XL, 0x38);


 }
 void readACCData(void) {
	uint8_t temp[6];
	i2cRead(GYROACCADDRESS, OUT_X_L_XL, 6, temp);
	ax = (temp[1] << 8)| temp[0];
	ay = (temp[3] << 8)| temp[2];
	az = (temp[5] << 8)| temp[4];
 }
 void readGyroData(void) {
	uint8_t temp[6];
	i2cRead(GYROACCADDRESS, OUT_X_L_G, 6, temp);
	gx = (temp[1] << 8)| temp[0];
	gy = (temp[3] << 8)| temp[2];
	gz = (temp[5] << 8)| temp[4];
 }
 void readMagData(void) {
	uint8_t temp[6];
	i2cRead(MAGADDRESS, OUT_X_L_M, 6, temp);
	mx = (temp[1] << 8)| temp[0];
	my = (temp[3] << 8)| temp[2];
	mz = (temp[5] << 8)| temp[4];
 }
 void readTempData(void) {
	uint8_t temp[2];
	int16_t tempValue = 0;
	i2cRead(GYROACCADDRESS, OUT_TEMP_L, 2, temp);
	tempValue = (((int16_t) temp[1] << 12) | temp[0] << 4 ) >> 4;
	temperature *= 0.9;
	temperature += ((float)tempValue / 16)+25;
	
	
 }
 int16_t getGyroData(int axis) {
	switch(axis) {
		case X_AXIS:
			return gx;
			break;
		case Y_AXIS:
			return gy;
			break;
		case Z_AXIS:
			return gz;
			break;
	}
	return 0;
 }
 int16_t getACCData(int axis) {
	switch(axis) {
		case X_AXIS:
			return ax;
			break;
		case Y_AXIS:
			return ay;
			break;
		case Z_AXIS:
			return az;
			break;
	}
	return 0;
 }
 int16_t getMagData(int axis) {
	switch(axis) {
		case X_AXIS: 
			return mx;
			break;
		case Y_AXIS:
			return my;
			break;
		case Z_AXIS: 
			return mz;
			break;
	}
	return 0;
 }
 float getTemperatureData() {
	return temperature / 10;
 }
