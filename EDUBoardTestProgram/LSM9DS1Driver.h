/*
 * LSM9DS1Driver.h
 *
 * Created: 15.05.2017 10:32:45
 *  Author: mburger
 */ 


#ifndef LSM9DS1DRIVER_H_
#define LSM9DS1DRIVER_H_

#define X_AXIS		0x00
#define Y_AXIS		0x01
#define Z_AXIS		0x02

#define GYROACCADDRESS	0xD4
#define MAGADDRESS		0x38

void LSM9DS1Init(void);
void readACCData(void);
void readGyroData(void);
void readMagData(void);
void readTempData(void);
int16_t getGyroData(int axis);
int16_t getACCData(int axis);
int16_t getMagData(int axis);
float getTemperatureData();

#endif /* LSM9DS1DRIVER_H_ */