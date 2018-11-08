//******************************************************************************************//
//*************************************** main.c  ******************************************//
//******************************************************************************************//
/*
bottom view w/ reset button on right

TX		RX		RESET*		gnd		PD2		PD3		PD4		PD5		PD6		PD7		PB0		PB1

RAW		gnd		RESET*		VCC		PC3		PC2		PC1		PC0		PB5		PB4		PB3		PB2
																	SCLK	MISO	MOSI	SS
*either one goes to CS signal on programmer

DS1620 pinout:

8 7 6 5
1 2 3 4

1 - DQ
2 - CLK
3 - RST
4 - GND
8 - VDD

*/
#include <avr/io.h>
#include<avr/interrupt.h>
//#include "../avr8-gnu-toolchain-linux_x86/avr/include/util/delay.h"
#include "../../Atmel_other/avr8-gnu-toolchain-linux_x86/avr/include/util/delay.h"
#include "sfr_helper.h"
#include <avr/eeprom.h>
#include <stdlib.h>
#include "USART.h"
#include "macros.h"
#include "pinDefines.h"
#include "DS1620.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define LEN 200

/*
#define CLK PORTD2
#define DATA PORTD3
#define RST PORTD4
*/

/***************************************************************/
int main(void)
{
	UCHAR xbyte;
	UINT temp;
	int raw_data;

	initUSART();
	_delay_ms(1);
	init1620();

	while(1)
	{	
		_delay_ms(5);
		raw_data = readTempFrom1620_int();
		_delay_ms(1);
		temp = (UINT)raw_data;
		temp >>= 8;
		xbyte = (UCHAR)temp;
		transmitByte(xbyte);
		_delay_ms(1);
		temp = (UINT)raw_data;
		xbyte = (UCHAR)temp;
		transmitByte(xbyte);
	}
	return (0);		// this should never happen
}

/***************************************************************/
float convert_to_T_F(int raw_data)
{
	float T_F, T_celcius;
	if ((raw_data & 0x100) != 0)
	{
		raw_data = - (((~raw_data)+1) & 0xff); /* take 2's comp */   
	}
	T_celcius = raw_data * 0.5;
	T_F = (T_celcius * 1.8) + 32;
	return(T_F);
}


