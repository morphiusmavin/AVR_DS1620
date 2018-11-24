<h1>AVR_DS1620</h1>
Program that runs on AVR ProMini to send temp readings from (4) DS1620 sensors to a<br />
Xilinx Spartan-3E over serial port.<br />
-- AVR -> FPGA -> RS-232 -> monitor<br />
-- AVR to FPGA uses 5v->3v3 conv<br />
-- FPGA to RS-232 uses 3v3 conv to RS-232 conv to monitor (linux box)<br />
-- tx_uart is P77<br />
-- rx_ds1620 is P75<br />
<br />
The program: test_DS1620.cx is compiled on a linux box to read the output<br />
from the serial port of the FPGA and spits out the time with the 4 readings<br />
<br />
<h2>AVR Pinout</h2>
1st DS1620<br />
DS1620_PIN_DQ	PD2<br />
DS1620_PIN_CLK	PD3<br />
DS1620_PIN_RST	PD4<br />
<br />
2nd DS1620<br />
DS1620_PIN_DQ	PD5<br />
DS1620_PIN_CLK	PD6<br />
DS1620_PIN_RST	PD7<br />
<br />
3rd DS1620<br />
DS1620_PIN_DQ	PB0<br />
DS1620_PIN_CLK	PB1<br />
DS1620_PIN_RST	PB2<br />
<br />
4th DS1620<br />
DS1620_PIN_DQ	PC0<br />
DS1620_PIN_CLK	PC1<br />
DS1620_PIN_RST	PC2<br />
<br />
credit to: Jens Willy Johannsen on code for DS1620<br />
