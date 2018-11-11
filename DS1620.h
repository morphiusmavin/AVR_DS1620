/*
 *  DS1620.h
 *  DS1620_VUSB
 *
 *  Created by Jens Willy Johannsen on 12-10-10.
 *  Copyright 2010 «YOURCOMPANY». All rights reserved.
 *
 */

#include <avr/io.h>

#define DS1620_CMD_READTEMP		0xAA
#define DS1620_CMD_WRITETH		0x01
#define DS1620_CMD_WRITETL		0x02
#define DS1620_CMD_READTH		0xA1
#define DS1620_CMD_READTL		0xA2
#define DS1620_CMD_READCNTR		0xA0
#define DS1620_CMD_READSLOPE	0xA9
#define DS1620_CMD_STARTCONV	0xEE
#define DS1620_CMD_STOPCONV		0x22
#define DS1620_CMD_WRITECONF	0x0C
#define DS1620_CMD_READCONF		0xAC

#define DS1620_PORT		PORTD
#define DS1620_DDR		DDRD
#define DS1620_PIN		PIND

void init1620();
void writeByteTo1620( uint8_t cmd );
void writeCommandTo1620( uint8_t cmd, uint8_t data );
void writeTempTo1620( uint8_t reg, int temp );
double readTempFrom1620();
int readTempFrom1620_int();

void init1620_2();
void writeByteTo1620_2( uint8_t cmd );
void writeCommandTo1620_2( uint8_t cmd, uint8_t data );
void writeTempTo1620_2( uint8_t reg, int temp );
double readTempFrom1620_2();
int readTempFrom1620_int_2();

