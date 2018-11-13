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

#define LED PB5
#define AVG_SIZE 10

volatile int dc2;
volatile UCHAR xbyte;
static int avg1[AVG_SIZE];
static int avg2[AVG_SIZE];
static int avg3[AVG_SIZE];
static int avg4[AVG_SIZE];

ISR(TIMER1_OVF_vect) 
{ 
	TCNT1 = 0xBFFF;	// this counts up so the lower, the slower (0xFFFF is the fastest)
	dc2++;
	if(dc2 % 2 == 0)
		PORTB |= (1 << LED);
	else
		PORTB &= ~(1 << LED);
}

//******************************************************************************************//
int main(void)
{
	UINT temp;
	int raw_data1, raw_data2, raw_data3, raw_data4;
	// hex values 0x2b = 70.7F
	// and 0x31 = 76.1F
	int dc3;
	int i;
	UCHAR main_loop_delay = 10;
	UCHAR xbyte;

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
	for(i = 0;i < 30;i++)
	{
		PORTB &= ~(1 << LED);
		_delay_ms(50);
		PORTB |= (1 << LED);
		_delay_ms(50);
	}		
	_delay_ms(1000);
	PORTB |= (1 << LED);
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
	xbyte = 0x21;
	while(1)
	{
		transmitByte(xbyte);

		if(++xbyte > 0x7d)
			xbyte = 0x21;
		_delay_ms(1);
	}
*/
	raw_data1 = readTempFrom1620_int();
	raw_data2 = readTempFrom1620_int_2();
	raw_data3 = readTempFrom1620_int_3();
	raw_data4 = readTempFrom1620_int_4();

	for(i = 0;i < AVG_SIZE;i++)
		avg1[i] = raw_data1;

	for(i = 0;i < AVG_SIZE;i++)
		avg2[i] = raw_data2;

	for(i = 0;i < AVG_SIZE;i++)
		avg3[i] = raw_data3;

	for(i = 0;i < AVG_SIZE;i++)
		avg4[i] = raw_data4;

	sei(); // Enable global interrupts by setting global interrupt enable bit in SREG

	while(1)
	{	
		if(dc2 % main_loop_delay == 0)
		{
			_delay_ms(50);

			raw_data1 = readTempFrom1620_int();
			raw_data1 = do_avg(avg1,raw_data1);
			dc3 = dc2;

			if(dc3 % (main_loop_delay) == 0)
			{
				temp = (UINT)raw_data1;
				temp >>= 8;
				xbyte = (UCHAR)temp;
				transmitByte(xbyte);
				_delay_ms(1);
				temp = (UINT)raw_data1;
				xbyte = (UCHAR)temp;
				transmitByte(xbyte);
			}
			_delay_ms(500);

			raw_data2 = readTempFrom1620_int_2();
			raw_data2 = do_avg(avg2,raw_data2);

			if(dc3 % (main_loop_delay) == 0)
			{
				temp = (UINT)raw_data2;
				temp >>= 8;
				xbyte = (UCHAR)temp;
				transmitByte(xbyte);
				_delay_ms(1);
				temp = (UINT)raw_data2;
				xbyte = (UCHAR)temp;
				transmitByte(xbyte);
			}
			_delay_ms(500);

			raw_data3 = readTempFrom1620_int_3();
			raw_data3 = do_avg(avg3,raw_data3);

			if(dc3 % (main_loop_delay) == 0)
			{
				temp = (UINT)raw_data3;
				temp >>= 8;
				xbyte = (UCHAR)temp;
				transmitByte(xbyte);
				_delay_ms(1);
				temp = (UINT)raw_data3;
				xbyte = (UCHAR)temp;
				transmitByte(xbyte);
			}
			_delay_ms(500);

			raw_data4 = readTempFrom1620_int_4();
			raw_data4 = do_avg(avg2,raw_data4);

			if(dc3 % (main_loop_delay) == 0)
			{
				temp = (UINT)raw_data4;
				temp >>= 8;
				xbyte = (UCHAR)temp;
				transmitByte(xbyte);
				_delay_ms(1);
				temp = (UINT)raw_data4;
				xbyte = (UCHAR)temp;
				transmitByte(xbyte);
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

