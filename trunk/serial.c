#include "serial.h"
#include <stdio.h>

Byte get_uart_register(Word base, Word reg)
{
	return inportb(base + reg);
}


void set_uart_register(Word base, Word reg, Byte b)
{
	outportb(base + reg, b);
}


int get_baud(Word base)
{
	//para se poder aceder DLAB=1
	outportb(base + SER_LCR, inportb(base + SER_LCR) | DLAB);
	//a parte mais significativa do baud_rate encontra-se no Div_msb e a menos significativa no dif_lsb
	int rate = (inportb(base + DIV_MSB) << 8) + inportb(base + DIV_LSB);
	//Dlab tem de se voltar a colocar a zero
	outportb(base + SER_LCR, inportb(base + SER_LCR) & ~DLAB);

	return BAUD_FREQ / rate;
}


void set_baud(Word base, int rate)
{
	outportb(base + SER_LCR, inportb(base + SER_LCR) | DLAB); //para se poder aceder DLAB=1
	outportb(base + DIV_MSB, MSB(BAUD_FREQ / rate)); //escreve MSB do divisor 
	outportb(base + DIV_LSB, LSB(BAUD_FREQ / rate)); // escreve LSB do divisor 
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
	Byte b = inportb(base + SER_LCR) & WORD_BITS;
	switch(b)
	{
		case WORD_5: return 5;
		case WORD_6: return 6;
		case WORD_7: return 7;
		case WORD_8: return 8;
		default: return -1;
	}
}


void set_nbits(Word base, int nbits)
{
	Byte temp;
	switch(nbits)
	{
		case 5: temp = WORD_5; break;
		case 6: temp = WORD_6; break;
		case 7: temp = WORD_7; break;
		case 8: temp = WORD_8; break;
		default: return;
	}
	
	outportb(base + SER_LCR, (inportb(base + SER_LCR) & ~WORD_BITS) | temp);
}


int get_stop_bits(Word base)
{
	Byte b = inportb(base + SER_LCR) & STOP_BITS;
	switch(b)
	{
		case STOP_1: return 1;
		case STOP_2: return 2;
		default: return -1;
	}
}


void set_stop_bits(Word base, int stop)
{
	Byte temp;
	switch(stop)
	{
		case 1: temp = STOP_1; break;
		case 2: temp = STOP_2; break;
		default: return;
	}
	
	outportb(base + SER_LCR, (inportb(base + SER_LCR) & ~STOP_BITS) | temp);
}



void init_uart(Word base, int rate, int nbits, int stop_bits, Byte parity, Bool rx_int_enable, Bool tx_int_enable, Bool fifo_enable)
{
	//inicializar LCR
	set_uart_register(base, SER_LCR, 0);
	set_baud(base, rate);
	set_nbits(base, nbits);
	set_stop_bits(base, stop_bits);
	set_parity(base, parity);
		
	//inicializar MCR
	set_uart_register(base, SER_MCR, AUX_OUT2);
	
	//inicializar IER
	Byte ier_byte = 0;
	if(rx_int_enable) ier_byte |= RX_INT_EN;
	if(tx_int_enable) ier_byte |= TX_INT_EN;
	set_uart_register(base, SER_IER, ier_byte);
	
	//inicializar FCR
	if(fifo_enable)
	{
		/* habilita	FIFOS */
		set_uart_register(base, SER_FCR, FIFO_INT_14 | CLR_TX_FIFO | CRL_RX_FIFO | EN_FIFO);
		/* Verifica estado das FIFO */
		if((get_uart_register(base, SER_IIR) & FIFO_ENABLED) != FIFO_ENABLED)/* Se nao conseguir actvar */
			set_uart_register(base, SER_FCR, 0);
	}
	else set_uart_register(base, SER_FCR, 0); /* Desactiva */
}


void show_uart(Word base)
{
    printf("UART - COM%i", (base == COM1_ADD) ? 1 : 2);
	printf("\nDATA - "); 
    print_binary(get_uart_register(base, SER_DATA));
	printf("\nIER - "); 
    print_binary(get_uart_register(base, SER_IER));
	printf("\nIIR - "); 
    print_binary(get_uart_register(base, SER_IIR));
	printf("\nLCR - "); 
    print_binary(get_uart_register(base, SER_LCR));
	printf("\nMCR - "); 
    print_binary(get_uart_register(base, SER_MCR));
	printf("\nLSR - "); 
	print_binary(get_uart_register(base, SER_LSR));
	printf("\nMSR - "); 
	print_binary(inportb(base +  SER_MSR));
	printf("\nBaudrate - %d", get_baud(base)); 
	
	printf("\nParity - ");
	Byte par = get_parity(base);
	switch(par){
		case PAR_NONE: 
			printf("NONE");
			break;
		case PAR_ODD:
			printf("ODD");
			break;
		case PAR_EVEN:
			printf("EVEN");
			break;
		case PAR_HIGH:
			printf("HIGH");
			break;
		case PAR_LOW:
			printf("LOW");
			break;
		default: break; 
	}
	
	printf("\nStop bits - %i", get_stop_bits(base));
	printf("\nNumero de bits - %i", get_nbits(base));

}

void send_char(Word base, char c)
{
	while((inportb(base + SER_LSR) & TX_RDY) == 0); /* busy wait for trasmiter ready */
	outportb(base + SER_DATA, c); /* OK, escreve caracter c a enviar */
}


char rcv_char(Word base)
{
	while(!char_available(base));
	return inportb(base + SER_DATA);
}


Bool char_available(Word base)
{
	return (inportb(base + SER_LSR) & RX_RDY) == 1;
}


//print a byte in binary
void print_binary(Byte b)
{
	int offset;
	for(offset = 7; offset >= 0; offset--)
		printf("%i", (b >> offset) & 1);
}

