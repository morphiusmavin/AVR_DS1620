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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define LEN 200

#define CLK PORTD2
#define DATA PORTD3
#define RST PORTD4

#define SET_CLK()	_SB(PORTD,CLK)
#define CLR_CLK()	_CB(PORTD,CLK)

#define SET_DATA()	_SB(PORTD,DATA)
#define CLR_DATA()	_CB(PORTD,DATA)

#define SET_RST()	_SB(PORTD,RST)
#define CLR_RST()	_CB(PORTD,RST)

/*  DS1620 Commands */
#define CONF_REG 0x0c
#define READ_CONF 0xAC
#define READ_TEMP 0xaa
#define START_CONVERT 0xee
#define STOP_CONVERT 0x22

void write_command(int command);
void out_bit(int bit);
int read_raw_data(void);
float convert_to_T_F(int raw_data);

int data=0x00;  /**Global variable**/

volatile UCHAR xbyte;

int main(void)
{
	int raw_data, n;
	int configure=0x02;
	float T_F;
	UCHAR xbyte;
	UINT temp;

	initUSART();

	DDRD &= 0xFF;
	_delay_us(2);

	CLR_RST();
	SET_CLK();
	SET_RST();

	write_command(CONF_REG); /* configure DS1620 in CPU Mode */      
	write_command(configure);
	CLR_RST();
	_delay_us(200);  /* wait for programming of configuration status register */
	SET_CLK();
	SET_RST();
	write_command(READ_CONF);
	raw_data = 0;
	raw_data=read_raw_data();        /*read raw temp data */

	CLR_RST();

	temp = (UINT)raw_data;
	temp >>= 8;
	xbyte = (UCHAR)temp;
	transmitByte(xbyte);
	_delay_ms(1);
	temp = (UINT)raw_data;
	xbyte = (UCHAR)temp;
	transmitByte(xbyte);

	_delay_us(5000);  /* wait for programming of configuration status register */
	SET_CLK();
	SET_RST();
	write_command(START_CONVERT);   /* start converting temp*/    
	CLR_RST();
	_delay_ms(200);

	while(1)
	{	
		SET_CLK();
		SET_RST();
		write_command(READ_TEMP);    /*command to 1620 for reading temp*/      
		raw_data=read_raw_data();        /*read raw temp data */

		CLR_RST();

//			T_F=convert_to_T_F(raw_data);

		temp = (UINT)raw_data;
		temp >>= 8;
		xbyte = (UCHAR)temp;
		transmitByte(xbyte);
		_delay_ms(1);
		temp = (UINT)raw_data;
		xbyte = (UCHAR)temp;
		transmitByte(xbyte);

//			printf("The temperature is %f degrees fahrenheit.\n\n", T_F);    
//			sleep(10);
		_delay_ms(1000);
	}
	SET_CLK();
	SET_RST();
	write_command(STOP_CONVERT); /* stop conversion to save power */    
	CLR_RST();
	_delay_ms(5000);
	return (0);		// this should never happen
}

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

void write_command(int command)
/* sends 8 bit command on /STROBE output, least sig bit first */ 
{
	int n, bit;

	for(n=0;n<8;n++)
	{
		bit = ((command >> n) & (0x01));
		out_bit(bit);
	}
}

int read_raw_data(void)
{
	int bit,n;
	int raw_data=0;
	bit = 0;
//	UCHAR test;

	DDRD &= 0xF7;

	_delay_us(1);
	
	for(n=0;n<9;n++)
	{
		CLR_CLK();
//		test = PIND & _BV(DATA);
//		transmitByte(test);
		if((PIND & _BV(DATA)) == 0x08)
			bit = 1;
		else
			bit = 0;

		SET_CLK();
		raw_data = raw_data | (bit << n);
	}
	DDRD |= 0x08;
	return(raw_data);
}


void out_bit(int bit)
/* state of /STROBE set to value of bit, followed by clock pulse */ 
{
	CLR_CLK();
	if(bit & 0x01)
	{
		SET_DATA();
	}
	else CLR_DATA();
	SET_CLK();
}

