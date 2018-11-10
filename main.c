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


void heater_relay(int port, int onoff);
void float_relay(UCHAR mask);

// relays for switching float charger are PC0->PC3
#define FLOAT_RELAY_PORT PORTC
#define FLOAT_RELAY_DDR DDRC

// relays for heat strips are PB0->PB2
#define HEAT_RELAY_DDR DDRB
#define HEAT_RELAY_PORT PORTB
#define HEAT_RELAY1 PB0
#define HEAT_RELAY2 PB1
#define HEAT_RELAY3 PB2
#define LED PB5


volatile int dc2;
volatile UCHAR xbyte;

ISR(TIMER1_OVF_vect) 
{ 
	TCNT1 = 0x0002;	// this counts up so the lower, the slower (0xFFFF is the fastest)
	dc2++;
	if(dc2 % 2 == 0)
		HEAT_RELAY_PORT |= (1 << LED);
	else
		HEAT_RELAY_PORT &= ~(1 << LED);
}

//******************************************************************************************//
int main(void)
{
	UINT temp;
	int raw_data;
	// hex values 0x2b = 70.7F
	// and 0x31 = 76.1F
	int high_temp, low_temp;
	UINT mask, utemp, utemp2;
	int dc3;
	int i;
	low_temp = 0x002b;
	high_temp = 0x0031;

	initUSART();

	HEAT_RELAY_DDR = 0x27;
	FLOAT_RELAY_DDR = 0x0F;

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

	dc3 = 0;

	HEAT_RELAY_PORT = 0xFF;
	FLOAT_RELAY_PORT = 0xFF;

	// flicker the LED
/*
	for(i = 0;i < 20;i++)
	{
		HEAT_RELAY_PORT |= (1 << LED);
		_delay_ms(100);
		HEAT_RELAY_PORT &= ~(1 << LED);
		_delay_ms(100);
	}		
	for(i = 0;i < 3;i++)
	{
		heater_relay(0,1);
		_delay_ms(500);
	}
	for(i = 0;i < 3;i++)
	{
		heater_relay(0,1);
		_delay_ms(500);
	}
*/
	_delay_ms(1);
	init1620();

	raw_data = 0x002c;

	while(1)
	{	
		if(dc2 > 115)	// about a minute
		{
			dc2 = 0;

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

			// format 3rd byte sent to serial port 
			// as XHHHFFFF where H = heat strips and F = float relays
			utemp = FLOAT_RELAY_PORT;
			utemp &= 0x0F;
			utemp2 = HEAT_RELAY_PORT;
			utemp2 <<= 4;
			utemp2 &= 0xF0;
			utemp |= utemp2;
			utemp = ~utemp;
			transmitByte(utemp);

			if(raw_data > high_temp)
			{
				heater_relay(0,0);
//				HEAT_RELAY_PORT &= ~(1 << LED);		// LED only shows if heat strip 1 is on
			}
			else if(raw_data < low_temp)
			{
				heater_relay(0,1);
//				HEAT_RELAY_PORT |= (1 << LED);
			}

			if(raw_data < 0x0025)
				heater_relay(1,1);					// if temp < ~65F then turn on
			else									// 2nd heat strip
				heater_relay(1,0);

			if(raw_data < 0x001A)
				heater_relay(2,1);					// if temp < ~55F then turn on
			else									// 3rd heat strip
				heater_relay(2,0);
			// switch float charge relays every 10 min.
			if(++dc3 > 10)
			{
				dc3 = 0;
				float_relay((UCHAR)mask);
				mask <<= 1;
				// only do the 1st 4 of PORTB
				if(mask == 0x0010)
					mask = 1;
			}				
		}
	}
	return (0);		// this should never happen
}

//******************************************************************************************//
// relay board is active low inputs so to turn on relay set bit low
void heater_relay(int port, int onoff)
{
	switch(port)
	{
		case 0:
			if(onoff > 0)
				HEAT_RELAY_PORT &= ~(1 << HEAT_RELAY1);
			else	
				HEAT_RELAY_PORT |= (1 << HEAT_RELAY1);
		break;
		case 1:
			if(onoff > 0)
				HEAT_RELAY_PORT &= ~(1 << HEAT_RELAY2);
			else	
				HEAT_RELAY_PORT |= (1 << HEAT_RELAY2);
		break;
		case 2:
			if(onoff > 0)
				HEAT_RELAY_PORT &= ~(1 << HEAT_RELAY3);
			else	
				HEAT_RELAY_PORT |= (1 << HEAT_RELAY3);
		break;
		default:
			HEAT_RELAY_PORT |= (1 << HEAT_RELAY1);
			HEAT_RELAY_PORT |= (1 << HEAT_RELAY2);
			HEAT_RELAY_PORT |= (1 << HEAT_RELAY3);
		break;
	}
}

//******************************************************************************************//
void float_relay(UCHAR mask)
{
	FLOAT_RELAY_PORT |= 0x0F;
	switch(mask)
	{
		case 0x01:
			FLOAT_RELAY_PORT &= ~(1 << PC0);
		break;
		case 0x02:
			FLOAT_RELAY_PORT &= ~(1 << PC1);
		break;
		case 0x04:
			FLOAT_RELAY_PORT &= ~(1 << PC2);
		break;
		case 0x08:
			FLOAT_RELAY_PORT &= ~(1 << PC3);
		break;
		default:
			FLOAT_RELAY_PORT |= 0x0F;
		break;
	}
}
