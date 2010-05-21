#include "serie.h"
#include <pc.h>

int get_baud(Word base)
{
	outportb(base + SER_LCR, inportb(base + SER_LCR) | DLAB);
	int rate = (inportb(base + DIV_MSB) << 8) + inportb(base + DIV_LSB);
	outportb(base + SER_LCR, inportb(base + SER_LCR) & ~DLAB);

	return BAUD_FREQ / rate;
}

void set_baud(Word base, int rate)
{
	outportb(base + SER_LCR, inportb(base + SER_LCR) | DLAB);
	outportb(base + DIV_MSB, MSB(BAUD_FREQ / rate));
	outportb(base + DIV_LSB, LSB(BAUD_FREQ / rate));
	outportb(base + SER_LCR, inportb(base + SER_LCR) & ~DLAB);
}

Byte get_parity(Word base)
{
	return inportb(base + SER_LCR) & PAR_BITS;
}
void set_parity(Word base, Byte par)
{
	outportb(base + SER_LCR, (inportb(base + SER_LCR) & ~PAR_BITS) | par);
}

int get_nbits(Word base)
{
	Byte byte = inportb(base + SER_LCR) & WORD_BITS;
	
	switch(byte)
	{
		case WORD_5: 
			return 5;
		case WORD_6: 
			return 6;
		case WORD_7: 
			return 7;
		case WORD_8: 
			return 8;
			
		default:
			return -1;
	}
}


void set_nbits(Word base, int nbits)
{
	Byte aux;
	
	switch(nbits)
	{
		case 5: 
			aux = WORD_5; 
			break;
		case 6: 
			aux = WORD_6; 
			break;
		case 7: 
			aux = WORD_7; 
			break;
		case 8: 
			aux = WORD_8; 
			break;
			
		default: 
			return;
	}
	outportb(base + SER_LCR, (inportb(base + SER_LCR) & ~WORD_BITS) | aux);
}

int get_stop_bits(Word base)
{
	Byte byte = inportb(base + SER_LCR) & STOP_BITS;
	
	switch(byte)
	{
		case STOP_1: 
			return 1;
		case STOP_2: 
			return 2;
		default: 
			return -1;
	}
}


void set_stop_bits(Word base, int stop)
{
	Byte aux;
	
	switch(stop)
	{
		case 1: 
			aux = STOP_1; 
			break;
		case 2: 
			aux = STOP_2; 
			break;
			
		default: 
			return;
	}
	
	outportb(base + SER_LCR, (inportb(base + SER_LCR) & ~STOP_BITS) | aux);
}

void init_uart(Word base, int rate, int nbits, int stop_bits, Byte parity, Bool rx_int, Bool tx_int, Bool fifo)
{
	//inicializar LCR
	outportb(base + SER_LCR, 0);
	set_baud(base, rate);
	set_nbits(base, nbits);
	set_stop_bits(base, stop_bits);
	set_parity(base, parity);
		
	outportb(base + SER_MCR, AUX_OUT2);//inicializar MCR bit AUX_OUT2
	
	//inicializar IER
	Byte ier_byte = 0;
	
	if(rx_int) 
		ier_byte |= RX_INT_EN;
		
	if(tx_int) 
		ier_byte |= TX_INT_EN;
		
	outportb(base + SER_IER, ier_byte);
	
	
	if(fifo)//inicializar queue reg (FCR)
	{
		outportb(base + SER_FCR, FIFO_INT_14 | CLR_TX_FIFO | CRL_RX_FIFO | EN_FIFO | EN_FIFO_64);

	}
	
	else 
		outportb(base + SER_FCR, 0);
}

void envia_mensagem(Word base, char c)
{
	while (!inportb(base + SER_LSR) & TX_RDY);
	
	outportb(base + SER_DATA, c);
}

char recebe_mensagem(Word base)
{
	while (!mensagem_espera(base));
	return inportb(base + SER_DATA);
}

Bool mensagem_espera(Word base)
{
	if ((inportb(base + SER_LSR) & RX_RDY) != 0)
		return true;
	else
		return false;
}

Byte get_uart_register(Word base, Word reg)
{
	return inportb(base + reg);
}


void set_uart_register(Word base, Word reg, Byte b)
{
	outportb(base + reg, b);
}

