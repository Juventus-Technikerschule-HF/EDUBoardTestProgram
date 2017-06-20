//--------------------------------------------------------------------------------------//
//	Number to String Converter 															//
//	08.05.2015																			//
//  Martin Burger																		//
//--------------------------------------------------------------------------------------//

#include "NumberStringConverter.h"

unsigned long tenToThePowerOf(int power) {
	int i = 0;
	unsigned long result = 1;
	for(i = 0; i < power; i++) {
		result *= 10;
	}
	return result;
}

int convert_uint_string(unsigned int number, char* resultString) {
	int i = 0;
	int j = 0;
	int numberOfDigits = 0;
	char tempString[5];
	if(number > 0) {	
		while(number > 0) {
			tempString[i] = (char)(0x30 + (number %10));
			number /= 10;		
			i++;
		}
		numberOfDigits = i;
		for(j = 0; j < 6; j++) {
			if(i > 0) {
				resultString[j] = tempString[i-1];
				i--;
			} else {
				resultString[j] = '\0';
			}			
		}
	} else {
		resultString[0] = '0';
		resultString[1] = '\0';
		numberOfDigits = 1;
	}
	return numberOfDigits;
}

int convert_sint_string(signed int number, char* resultString) {
	int i = 0;
	int j = 0;
	int numberOfDigits = 0;
	int resultStart = 0;
	char tempString[5];
	int numberNegative = 0;
	if(number < 0) {
		numberNegative = 1;
		number *= -1;
	}
	if(number > 0) {	
		while(number > 0) {
			tempString[i] = (char)(0x30 + (number %10));
			number /= 10;		
			i++;
		}
		numberOfDigits = i;
		if(numberNegative == 1) {
			resultString[0] = '-';
			numberOfDigits++;
			resultStart = 1;
		}
		for(j = resultStart; j < 7; j++) {
			if(i > 0) {
				resultString[j] = tempString[i-1];
				i--;
			} else {
				resultString[j] = '\0';
			}			
		}
	} else {
		resultString[0] = '0';
		resultString[1] = '\0';
		numberOfDigits = 1;
	}
	return numberOfDigits;
}

int convert_float_string(float number, char* resultString, int decimalPlaces) {
	int i = 0;
	int j = 0;
	int numberOfDigits = 0;
	int resultStart = 0;
	char tempString1[8];
	char tempString2[8];
	int numberNegative = 0;
	int decimalCommaPosition = 0;
	signed long multipliedNumber = (signed long)(number * tenToThePowerOf(decimalPlaces));
	if(multipliedNumber < 0) {
		numberNegative = 1;
		multipliedNumber *= -1;
	}
	if(multipliedNumber > 0) {	
		//Convert Number to String
		while(multipliedNumber > 0) {
			tempString1[i] = (char)(0x30 + (multipliedNumber %10));
			multipliedNumber /= 10;		
			i++;
		}
		numberOfDigits = i+1; //Comma is a Digit aswell...
		//Add Comma to NumberString
		i = numberOfDigits;
		for(j=0;j<8;j++) {
			if(j == (numberOfDigits-decimalPlaces-1)) {
				tempString2[j] = ',';
			} else {
				if(i > 0) {
					tempString2[j] = tempString1[i-2];
					i--;
				} else {
					tempString2[j] = '\0';
				}
			}			
		}
		//Add Minus if nescessary
		if(numberNegative == 1) {
			resultString[0] = '-';
			numberOfDigits++;
			resultStart = 1;
		}
		i = 0;
		for(j = resultStart; j<8;j++) {
			resultString[j] = tempString2[i];
			i++;
		}
	} else {
		resultString[0] = '0';
		resultString[1] = '\0';
		numberOfDigits = 1;
	}
	return numberOfDigits;
}
void clearString(char* string, int n) {
	for(int i = 0; i < n; i++) {
		string[i] = ' ';
	}
}