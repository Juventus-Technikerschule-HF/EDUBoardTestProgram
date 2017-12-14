/*
 * twiMaster.h
 *
 * Created: 04.12.2017 23:32:36
 *  Author: Martin Burger
 */ 


#ifndef TWIMASTER_H_
#define TWIMASTER_H_

typedef enum i2cCmd_Tag{
	NOACT = 0,
	START = 1,
	BYTEREC = 2,
	STOP = 3
}i2cCmd_t;

void initI2C(void);
void i2cRead(uint8_t add, uint8_t reg, uint8_t n, uint8_t* data);
uint8_t i2cReadByte(uint8_t add, uint8_t reg);
void i2cWrite(uint8_t add, uint8_t reg, uint8_t n, uint8_t* data);
void i2cWriteByte(uint8_t add, uint8_t reg, uint8_t data);

#endif /* TWIMASTER_H_ */