//******************************************************************************************//
//*************************************** main.c  ******************************************//
//******************************************************************************************//
/*
bottom view w/ reset button on right

TX		RX		RESET*		gnd		PD2		PD3		PD4		PD5		PD6		PD7		PB0		PB1

RAW		gnd		RESET*		VCC		PC3		PC2		PC1		PC0		PB5		PB4		PB3		PB2
																	SCLK	MISO	MOSI	SS
*either one goes to CS signal on programmer

1)  gnd
2)  +5V
3)  VPU
4)  CLK
5)  CS
6)  3.3V
7)  ADC
8)  AUX
9)  MOSI
10) MISO
(all I'm using, currently are 1,4,5,9 & 10)

ICD cable from programmer looking at hole side with cable up:

1	2	3	4	5
6	7	8	9	10

DS1620 pinout:

+
8 7 6 5
1 2 3 4
	  -

1 - DQ	
2 - CLK
3 - RST
4 - GND
8 - VDD

1st DS1620
#define DS1620_PIN_DQ	PD2
#define DS1620_PIN_CLK	PD3
#define DS1620_PIN_RST	PD4

2nd DS1620
#define DS1620_PIN_DQ	PD5
#define DS1620_PIN_CLK	PD6
#define DS1620_PIN_RST	PD7

3rd DS1620
#define DS1620_PIN_DQ	PB0
#define DS1620_PIN_CLK	PB1
#define DS1620_PIN_RST	PB2

4th DS1620
#define DS1620_PIN_DQ	PC0
#define DS1620_PIN_CLK	PC1
#define DS1620_PIN_RST	PC2

pinouts on board: (starting from left looking at bottom)
DQ CLK RST GND VCC (repeat 4x)

(still have 3 pins avail - PC3, and the 2 behind 12,13 & A0 - PC4 & 5)
red 5v
org gnd
org/wh rst
blu clk
blu/wh data
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

int do_avg(int *avg_array, int cur);
void transmit(int raw_data);
int conv1(void);
int conv2(void);
int conv3(void);
int conv4(void);
void transmitPreamble(void);

#define LED PB5
#define AVG_SIZE 6

volatile int dc2;
static int avg1[AVG_SIZE];
static int avg2[AVG_SIZE];
static int avg3[AVG_SIZE];
static int avg4[AVG_SIZE];

ISR(TIMER1_OVF_vect) 
{ 
	TCNT1 = 0xBFFF;	// this counts up so the lower, the slower (0xFFFF is the fastest)
	dc2++;
/*
	if(dc2 % 2 == 0)
		PORTB |= (1 << LED);
	else
		PORTB &= ~(1 << LED);
*/
}

//******************************************************************************************//
int main(void)
{
//	UINT temp;
	int raw_data1, raw_data2, raw_data3, raw_data4;
	UCHAR xbyte;
	// hex values 0x2b = 70.7F
	// and 0x31 = 76.1F
	int dc3;
	int i;
	UCHAR main_loop_delay = 5;

	initUSART();

	TCNT1 = 0xFFF0;
	TCCR1A = 0x00;
	TCCR1B = (1<<CS10) | (1<<CS12);  // Timer mode with 1024 prescler	(very slow)
//	TCCR1B = (1<<CS10) | (1<<CS11);	// clk/64							(faster)
//	TCCR1B = (1<<CS11);	// clk/8	(see page 144 in datasheet)			(much faster)
//	TCCR1B = (1<<CS10);	// no prescaling								(very fast)
	
	TIMSK1 = (1 << TOIE1) ;   // Enable timer1 overflow interrupt(TOIE1)
	dc2 = 0;
	dc3 = 0;

	// flicker the LED

	DDRB |= 0x20;
	PORTB |= (1 << LED);
	_delay_ms(1000);
	for(i = 0;i < 10;i++)
	{
		PORTB &= ~(1 << LED);
		_delay_ms(5);
		PORTB |= (1 << LED);
		_delay_ms(5);
	}		
	_delay_ms(1000);
	PORTB &= ~(1 << LED);

#if 0
	xbyte = 0x4b;
	while(1)
	{
		transmitByte(0);
		_delay_ms(10);
		transmitByte(xbyte--);
		if(xbyte < 2)
			xbyte = 0x4b;
//		_delay_ms(2);
		_delay_ms(100);
		PORTB |= (1 << LED);
		_delay_ms(100);
		PORTB &= ~(1 << LED);
	}
//	}while(xbyte > 1);
#endif

	sei(); // Enable global interrupts by setting global interrupt enable bit in SREG

	_delay_ms(1);
	init1620();
	_delay_ms(1);
	init1620_2();
	_delay_ms(1);
	init1620_3();
	_delay_ms(1);
	init1620_4();

	_delay_ms(10);
/*
	xbyte = 0x4b;
	do
	{
		transmitByte(0);
		transmitByte(xbyte--);

		_delay_ms(20);
	}while(xbyte > 1);
*/
	writeByteTo1620( DS1620_CMD_STARTCONV );
	raw_data1 = readTempFrom1620_int();
	writeByteTo1620(DS1620_CMD_STOPCONV);

	_delay_ms(10);

	writeByteTo1620_2( DS1620_CMD_STARTCONV );
	raw_data2 = readTempFrom1620_int_2();
	writeByteTo1620_2(DS1620_CMD_STOPCONV);

	_delay_ms(10);

	writeByteTo1620_3( DS1620_CMD_STARTCONV );
	raw_data3 = readTempFrom1620_int_3();
	writeByteTo1620_3(DS1620_CMD_STOPCONV);

	_delay_ms(10);

	writeByteTo1620_4( DS1620_CMD_STARTCONV );
	raw_data4 = readTempFrom1620_int_4();
	writeByteTo1620_4(DS1620_CMD_STOPCONV);

	for(i = 0;i < AVG_SIZE;i++)
		avg1[i] = raw_data1;

	for(i = 0;i < AVG_SIZE;i++)
		avg2[i] = raw_data2;

	for(i = 0;i < AVG_SIZE;i++)
		avg3[i] = raw_data3;

	for(i = 0;i < AVG_SIZE;i++)
		avg4[i] = raw_data4;

	sei(); // Enable global interrupts by setting global interrupt enable bit in SREG


	transmitPreamble();

	transmit(raw_data1);
	_delay_ms(10);

	transmit(raw_data2);
	_delay_ms(10);

	transmit(raw_data3);
	_delay_ms(10);

	transmit(raw_data4);
	_delay_ms(10);

	while(1)
	{	
		if(dc2 % main_loop_delay == 2)
		{
			_delay_ms(200);
			dc3 = dc2;


			transmitPreamble();

			if(dc3 % (main_loop_delay) == 2)
			{
				raw_data1 = conv1();
				transmit(raw_data1);
			}
			_delay_ms(200);


			if(dc3 % (main_loop_delay) == 2)
			{
				raw_data2 = conv2();
				transmit(raw_data2);
			}
			_delay_ms(200);


			if(dc3 % (main_loop_delay) == 2)
			{
				raw_data3 = conv3();
				transmit(raw_data3);
			}
			_delay_ms(200);


			if(dc3 % (main_loop_delay) == 2)
			{
				raw_data4 = conv4();
				transmit(raw_data4);
			}
		}
	}
	return 0;
}

//******************************************************************************************//
int do_avg(int *avg_array, int cur)
{
	int i;
	int avg;
	avg = 0;

return cur;
	
	for(i = 0;i < AVG_SIZE;i++)
		printf("%02d ",avg_array[i]);

	printf("\n");

	for(i = 0;i < AVG_SIZE-1;i++)
		avg_array[i] = avg_array[i+1];

	avg_array[AVG_SIZE-1] = cur;	

	for(i = 0;i < AVG_SIZE;i++)
		printf("%02d ",avg_array[i]);

	printf("\n");

	for(i = 0;i < AVG_SIZE;i++)
		avg += avg_array[i];

	return avg/AVG_SIZE;
}

//******************************************************************************************//
void transmit(int raw_data)
{
	int temp;
	UCHAR xbyte;
	temp = (UINT)raw_data;
	temp >>= 8;
	xbyte = (UCHAR)temp;
	transmitByte(xbyte);
	_delay_ms(1);
	temp = (UINT)raw_data;
	xbyte = (UCHAR)temp;
	transmitByte(xbyte);
}

//******************************************************************************************//
int conv1(void)
{
	int raw_data;
	writeByteTo1620( DS1620_CMD_STARTCONV );
	PORTB |= (1 << LED);
	raw_data = readTempFrom1620_int();
	PORTB &= ~(1 << LED);
	writeByteTo1620(DS1620_CMD_STOPCONV);
	raw_data = do_avg(avg1,raw_data);
	return raw_data;
}

//******************************************************************************************//
int conv2(void)
{
	int raw_data;
	writeByteTo1620_2( DS1620_CMD_STARTCONV );
	PORTB |= (1 << LED);
	raw_data = readTempFrom1620_int_2();
	PORTB &= ~(1 << LED);
	writeByteTo1620_2(DS1620_CMD_STOPCONV);
	raw_data = do_avg(avg2,raw_data);
	return raw_data;
}

//******************************************************************************************//
int conv3(void)
{
	int raw_data;
	writeByteTo1620_3( DS1620_CMD_STARTCONV );
	PORTB |= (1 << LED);
	raw_data = readTempFrom1620_int_3();
	PORTB &= ~(1 << LED);
	writeByteTo1620_3(DS1620_CMD_STOPCONV);
	raw_data = do_avg(avg3,raw_data);
	return raw_data;
}

//******************************************************************************************//
int conv4(void)
{
	int raw_data;
	writeByteTo1620_4( DS1620_CMD_STARTCONV );
	PORTB |= (1 << LED);
	raw_data = readTempFrom1620_int_4();
	PORTB &= ~(1 << LED);
	writeByteTo1620_4(DS1620_CMD_STOPCONV);
	raw_data = do_avg(avg4,raw_data);
	return raw_data;
}
//******************************************************************************************//
void transmitPreamble(void)
{
return;
	transmitByte(0xFF);
	transmitByte(0xFF);
	transmitByte(0);
	transmitByte(0);
}
