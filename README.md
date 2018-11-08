testing DS1620 taking code from:
https://phanderson.com/printer/ds1620/ds1620.html

It's not working because I think he's not using CLK
on each bit shift on write...

use this to read results on monitor connected to
serial port from AVR:

while(1)
{
	read(fd,&ch,1);
	temp = (unsigned int)ch;
	temp <<= 8;
//	printf("%02x ",temp);
	read(fd,&ch,1);
	temp2 = (unsigned int)ch;
//	printf("%02x ",temp2);
	temp |= (unsigned int)temp2;
//	printf("%02x %d\n",temp,temp);
	printf("%02x\n",temp);
	temp = 0;
}


