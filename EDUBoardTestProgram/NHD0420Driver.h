/*
 * NHD0420Driver.h
 *
 * Created: 15.05.2017 08:31:46
 *  Author: mburger
 */ 


#ifndef NHD0420DRIVER_H_
#define NHD0420DRIVER_H_

#define LINE_1		0x00
#define LINE_2		0x01
#define LINE_3		0x02
#define LINE_4		0x03

void displayCPUClockConfig(uint32_t frequency);
void displayInit();
void displayBufferSetPos(int line, int pos);
void displayBufferHome();
void displayBufferClear();
void displayBufferWriteChar(char c);
void displayBufferWriteCharAtPos(int line, int pos, char c);
void displayBufferWriteString(char* s);
void displayBufferWriteStringAtPos(int line, int pos, char* s);


void displayForceUpdateBuffer(void);

/********************************************************/
/*Update-Worker has to be placed in main-loop           */
/********************************************************/
void displayUpdateWorker(void);
#endif /* NHD0420DRIVER_H_ */