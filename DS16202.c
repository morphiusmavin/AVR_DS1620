/*
 *  DS1620.c
 *  DS1620_VUSB
 *
 *  Created by Jens Willy Johannsen on 12-10-10.
 *  Copyright 2010 «YOURCOMPANY». All rights reserved.
 *
 */

#include "DS1620.h"
//#import <util/delay.h>
#include "../../Atmel_other/avr8-gnu-toolchain-linux_x86/avr/include/util/delay.h"

// Define PORTx, DDRx and PINx here
// Define individual pins here

#define DS1620_PORT		PORTD
#define DS1620_DDR		DDRD
#define DS1620_PIN		PIND

#define DS1620_PIN_DQ	PD5
#define DS1620_PIN_CLK	PD6
#define DS1620_PIN_RST	PD7

void init1620_2()
{
	// All pins -> output
	DS1620_DDR |= (1<< DS1620_PIN_DQ) | (1<< DS1620_PIN_RST) | (1<< DS1620_PIN_CLK);
	writeCommandTo1620_2( DS1620_CMD_WRITECONF, 0x02 );			// CPU mode; continous conversion
//	writeByteTo1620_2( DS1620_CMD_STARTCONV );					// Start conversion
}

void shiftOutByte_2( uint8_t val )
{
	int i;
	// Send uint8_t, LSB first
	for( i = 0; i < 8; i++ )
	{
		DS1620_PORT &= ~(1<< DS1620_PIN_CLK );					// tick...
		
		// Set bit
		if( val & (1 << i))
		{
			DS1620_PORT |= (1<< DS1620_PIN_DQ );
		}
		else
		{
			DS1620_PORT &= ~(1<< DS1620_PIN_DQ );
		}
		DS1620_PORT |= (1<< DS1620_PIN_CLK );					// ...tock
	}
}

void writeByteTo1620_2( uint8_t cmd )
{
	DS1620_PORT |= (1<< DS1620_PIN_RST );						// start comm - RST high
	
	shiftOutByte_2( cmd );
	
	DS1620_PORT &= ~(1<< DS1620_PIN_RST );						// end comm
}

void writeCommandTo1620_2( uint8_t cmd, uint8_t data )
{
	DS1620_PORT |= (1<< DS1620_PIN_RST );						// start comm - RST high
	
	shiftOutByte_2( cmd );	// send command
	shiftOutByte_2( data );	// send 8 bit data
	
	DS1620_PORT &= ~(1<< DS1620_PIN_RST );						// end comm
}

void writeTempTo1620_2( uint8_t reg, int temp )
{
	uint8_t lsb = temp;											// truncate to high uint8_t
	uint8_t msb = temp >> 8;									// shift high -> low uint8_t
	
	DS1620_PORT |= _BV( DS1620_PIN_RST );						// start comm - RST high
	
	shiftOutByte_2( reg );	// send register select
	shiftOutByte_2( lsb );	// send LSB 8 bit data
	shiftOutByte_2( msb );	// send MSB 8 bit data (only bit 0 is used)
	
	DS1620_PORT &= ~(1<< DS1620_PIN_RST );						// end comm
}

//double readTempFrom1620()
double readTempFrom1620_2()
{
	int i;
	
	DS1620_PORT |= (1<< DS1620_PIN_RST );						// start comm - RST high
	
	shiftOutByte_2( DS1620_CMD_READTEMP );						// send register select
	
	DS1620_DDR &= ~(1<< DS1620_PIN_DQ );						// configure for input
	
	int raw = 0;
	
	for( i=0; i<9; i++ )										// read 9 bits
	{
		DS1620_PORT &= ~(1<< DS1620_PIN_CLK );					// CLK low
		_delay_us( 2 );											// 1 µsec delay to allow the DS1620 to set the value before we read it
		if( DS1620_PIN & (1<< DS1620_PIN_DQ ))					// read bit
			raw |= (1 << i);									// add value
		DS1620_PORT |= (1<< DS1620_PIN_CLK );					// CLK high
	}
	
	DS1620_PORT &= ~(1<< DS1620_PIN_RST );						// end comm
	
	DS1620_DDR |= (1<< DS1620_PIN_DQ );							// DQ back to output mode
	
	return (double)(raw/(double)2);								// divide by 2 and return
																// as double
}

int readTempFrom1620_int_2()
{
	int i;
	
	DS1620_PORT &= ~(1<< DS1620_PIN_CLK );					// CLK low
	DS1620_PORT |= (1<< DS1620_PIN_RST );						// start comm - RST high
	
	shiftOutByte_2( DS1620_CMD_READTEMP );						// send register select
	
	DS1620_DDR &= ~(1<< DS1620_PIN_DQ );						// configure for input
	
	int raw = 0;
	
	for( i=0; i<9; i++ )										// read 9 bits
	{
		DS1620_PORT &= ~(1<< DS1620_PIN_CLK );					// CLK low
//		_delay_us( 2 );											// 1 µsec delay to allow the
																// DS1620 to set the value 																	// before we read it
		_delay_ms(5);
		if( DS1620_PIN & (1<< DS1620_PIN_DQ ))					// read bit
			raw |= (1 << i);									// add value
		DS1620_PORT |= (1<< DS1620_PIN_CLK );					// CLK high
		_delay_ms(1);
	}
	
	DS1620_PORT &= ~(1<< DS1620_PIN_RST );						// end comm
	
	DS1620_DDR |= (1<< DS1620_PIN_DQ );							// DQ back to output mode
	
	return raw;
}

