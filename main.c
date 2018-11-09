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

#define RELAY_PORT PORTB
#define RELAY_DDR DDRB

// relays for heat strips
#define RELAY1 PB4
#define RELAY2 PB6
#define RELAY3 PB7
// relays for switching float charger are PB0->PB3

volatile int dc2;
volatile UCHAR xbyte;

ISR(TIMER1_OVF_vect) 
{ 
	TCNT1 = 0x0002;	// this counts up so the lower, the slower (0xFFFF is the fastest)
	dc2++;
}

/***************************************************************/
int main(void)
{
	UINT temp;
	int raw_data;
	// hex values 0x2b = 70.7F
	// and 0x31 = 76.1F
	int high_temp, low_temp;
	UINT mask, utemp;
	int dc3;
	int i;
	low_temp = 0x002b;
	high_temp = 0x0031;

	initUSART();

	RELAY_DDR = 0xFF;		// all outputs

	TCNT1 = 0xFFF0;
	TCCR1A = 0x00;
//	TCCR1B = (1<<CS10) | (1<<CS12);  // Timer mode with 1024 prescler
	TCCR1B = (1<<CS10) | (1<<CS11);	// clk/64
//	TCCR1B = (1<<CS11);	// clk/8	(see page 144 in datasheet)
//	TCCR1B = (1<<CS10);	// no prescaling
	
	TIMSK1 = (1 << TOIE1) ;   // Enable timer1 overflow interrupt(TOIE1)
	dc2 = 0;
	dc3 = 0;
	mask = 1;
	sei(); // Enable global interrupts by setting global interrupt enable bit in SREG

	RELAY_DDR = 0xFF;		// all outputs

	dc3 = 0;

	_delay_ms(1);
	init1620();

	RELAY_PORT = 1;

	raw_data = 0x002c;
	while(1)
	{	
		if(dc2 > 115)	// about a minute
		{
			dc2 = 0;

			if(raw_data > high_temp)
			{
				RELAY_PORT &= ~(1 << PB4);
				RELAY_PORT &= ~(1 << PB5);
			}
			else if(raw_data < low_temp)
			{
				RELAY_PORT |= (1 << PB4);
				RELAY_PORT |= (1 << PB5);
			}
			if(raw_data < 0x0025)
				RELAY_PORT |= (1 << PB6);
			else
				RELAY_PORT &= ~(1 << PB6);	

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
			utemp = PORTB;
			transmitByte(utemp);

			if(++dc3 > 10)
			{
				dc3 = 0;
				RELAY_PORT &= 0xF0;
				RELAY_PORT |= (UCHAR)mask;
				mask <<= 1;
				// only do the 1st 4 of PORTB
				if(mask == 0x0010)
					mask = 1;
			}				
		}
	}
	return (0);		// this should never happen
}

