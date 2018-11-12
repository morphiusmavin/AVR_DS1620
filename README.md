<h1>AVR_DS1620</h1>
Program that runs on AVR ProMini to control 3 heat strips in an insulated battery storage box.<br />
The AVR controls (2) DS1620 digital temp sensors and (5) relays. One of the temp sensors is<br />
outside the box and the other is inside. Three of the relays control the 3 heat strips in the<br />
box and the other 4 switch a float charger every ten minutes between 4 batteries.<br />
The program: test_DS1620.cx is compiled on a linux box to read the output from the serial<br />
port of the AVR and spits out the ambient and inside temps plus if anything changes as far<br />
as the relays and also records to a text file.<br />
<h2>AVR Pinout</h2>
PD2 - DQ	ambient DS1620<br />
PD3 - CLK<br />
PD4 - RST<br />
<br />
PD5 - DQ	inside box DS1620<br />
PD6 - CLK<br />
PD7 - RST<br />
<br />
PB0 - heat relay 1<br />
PB1 - heat relay 2<br />
PB2 - heat relay 3<br />
<br />
PC0 - float relay 1<br />
PC1 - float relay 2<br />
PC2 - float relay 3<br />
PC3 - float relay 4<br />
<br />
credit to: Jens Willy Johannsen on code for DS1620<br />
pictures at: https://batterybox.shutterfly.com<br />
