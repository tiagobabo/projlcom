#include "kbc.h"
#include <dos.h>

int write_kbc(unsigned adr, unsigned data)
{
	unsigned char stat;
	int i=0;
	while(i < KBC_TIMEOUT) 
	{
		stat = inportb(STAT_REG);
		/* loop until 8042 input buffer empty */
		if((stat & IBF) == 0) 
		{
			outportb(adr, data); /* argumentos da função */
			return 0;
		}
		delay(1);
		i++;
	}
	// terminou o tempo
	return -1;
}

int read_kbc(void)
{
	unsigned char stat, data;
	int i=0;
	while(i < KBC_TIMEOUT) 
	{
		stat = inportb(STAT_REG);
		/* loop until 8042 output buffer full */
		if((stat & OBF) != 0) 
		{
			data = inportb(DATA_REG);
			/* or parity error or receive timeout */
			if((stat & (PAR_ERR | TO_ERR)) == 0)
				return data;
			else
			return -1;
		}
		delay(1);
		i++;
	}
	//acabou o tempo
	return -1;
}

void blink_leds(void)
{
	write_kbc(DATA_REG, WriteLEDS); 
	write_kbc(DATA_REG, (NUM_LOCK | CAPS_LOCK | SCROLL_LOCK)); 
	delay(150);
	write_kbc(DATA_REG, WriteLEDS); 
	write_kbc(DATA_REG,0x0);
}

void blink_led(uchar led)
{
	int aux;
	
	if (led == 1)
		aux = NUM_LOCK;
	else if (led == 2)
		aux = CAPS_LOCK;
	else
		aux = SCROLL_LOCK;
	
	write_kbc(DATA_REG, WriteLEDS); 
	write_kbc(DATA_REG, aux); 
	delay(150);
	write_kbc(DATA_REG, WriteLEDS);
	write_kbc(DATA_REG,0x0);
}
