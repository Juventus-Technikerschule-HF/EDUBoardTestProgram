#ifndef NUMBERSTRINGCONVERTER
#define NUMBERSTRINGCONVERTER

//**********************************************************************************************//
//		Convert unsigned Integer to string 													    //
//																								//
//		number: 		unsigned integer to convert (Restricted to 16 Bit)						//
//		resultString: 	String where the converted Number is written to. (at least 6 Bytes)		//
//		returnvalue: 	Number of Digits in the resultString									//
//**********************************************************************************************//
int convert_uint_string(unsigned int number, char* resultString);

//**********************************************************************************************//
//		Convert signed Integer to string 													    //
//																								//
//		number: 		signed integer to convert (Restricted to 16 Bit)						//
//		resultString: 	String where the converted Number is written to. (at least 7 Bytes)		//
//		returnvalue: 	Number of Digits in the resultString									//
//**********************************************************************************************//
int convert_sint_string(signed int number, char* resultString);

//**********************************************************************************************//
//		Convert floating point Number to string 											    //
//																								//
//		number: 		float to convert 														//
//		resultString: 	String where the converted Number is written to. (at least 8 Bytes)		//
//		decimalPlaces: 	Number of decimal Places you want to convert.							//
//		returnvalue: 	Number of Digits in the resultString									//
//**********************************************************************************************//
int convert_float_string(float number, char* resultString, int decimalPlaces);

void clearString(char* string, int n);

#endif