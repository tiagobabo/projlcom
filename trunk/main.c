#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "video.h"
#include "utypes.h"
#include "sprite.h"
#include "queue.h"
#include "kbc.h"
#include "ints.h"
#include "music.h"
#include "timer.h"
#include "rtc.h"
#include "GQueue.h"
#include "song.h"
#include "video-text.h"
#include "queue.h"
#include "serie.h"

int time_sound;
Note* actual = NULL;
int vidas = 3;
int vidas2 = 3;
Queue teclas;
Queue keys_queue;
Byte temp;
//Teclas Default
Byte key_up = 0x11;
Byte key_down = 0x1f;
Byte key_left = 0x1e;
Byte key_right = 0x20;
Byte key_up_default = 0x11;
Byte key_down_default = 0x1f;
Byte key_left_default = 0x1e;
Byte key_right_default = 0x20;
char *video_mem;
_go32_dpmi_seginfo old_rtc_irq;
Song* bgm;
int song_note_ind = 0;

#define HLT __asm__ __volatile__("HLT")
#define STI __asm__ __volatile__("STI")
#define CLI __asm__ __volatile__("CLI")

void get_time(int *hour, int *minute, int *second);
void asm_kbd();

typedef enum {INACTIVE, PLAY, PAUSE, STOPPED} MUSIC_STATE;
volatile MUSIC_STATE state = INACTIVE;
volatile int ms_count = 0;

Queue rcv_char_queue, send_char_queue;
volatile Word base;
_go32_dpmi_seginfo old_serial_irq;

void serial_isr(void)
{
	int ch; Bool not_full;
	Byte st = inportb(base + SER_IIR) & INT_ID;
	switch(st)
	{
		case INT_ST: return;

		case RX_INT:
			// queuePut(&rcv_char_queue, inportb(base + SER_DATA));
			do not_full = queuePut(&rcv_char_queue, inportb(base + SER_DATA));
			while (not_full && (inportb(base + SER_LSR) & RX_RDY));
			
			break;

		case TX_INT:
			// ch = queueGet(&send_char_queue);
			// if(ch != -1) outportb(base + SER_DATA, ch);
			do
			{
				ch = queueGet(&send_char_queue);
				if(ch != -1) outportb(base + SER_DATA, ch);
			}
			while(ch != -1 && (inportb(base + SER_LSR) & TX_RDY));
			
			break;
	}

	outportb(PIC1_CMD, EOI);
}


void init_serie()
{
	int serial_irq = ((base == COM1_ADD) ? COM1_IRQ : COM2_IRQ);
	install_c_irq_handler(serial_irq, serial_isr, &old_serial_irq);
	
	//inicializar IER
	set_uart_register(base, SER_IER, RX_INT_EN | TX_INT_EN);

	unmask_pic(serial_irq);
}

void print_bin(Byte b)
{
	int offset;
	for(offset = 7; offset >= 0; offset--)
		printf("%i", (b >> offset) & 1);
}

volatile Word base;

void finalize_serie()
{
	int serial_irq = ((base == COM1_ADD) ? COM1_IRQ : COM2_IRQ);
	mask_pic(serial_irq);
	set_uart_register(base, SER_IER, 0);
	reinstall_c_irq_handler(serial_irq, &old_serial_irq);
}

volatile int rtc_a = 0;
volatile int rtc_p = 0;
volatile int rtc_u = 0;

GQueue* musica;
Note* nota = NULL;
int dur;
Queue* apontQueue;
int curH=0, curM=0, curS=0;
extern Byte data_mode;
void draw_menu();

typedef struct {
	char* nome;
	int pontua;
} Pontuacoes;
Pontuacoes pontuacao[10];

Queue rcv_queue, send_queue;

volatile Word base;

void return_pontuacoes(){
	char line[100];
	int pont;
	FILE *file;
	file=fopen("pontuacoes.txt", "r");
	if(file == NULL){
		printf("file not found\n");
		exit(0);
	}
	else{
		int i = 0;
		while(!feof(file)){
		fgets(line, 100, file);
		if(line[strlen(line)-1] == '\n')
			line[strlen(line)-1] = '\0';
		pontuacao[i].nome = malloc (sizeof line);
		strcpy(pontuacao[i].nome, line);

		fgets(line, 100, file);
		pont = atoi(line);
		pontuacao[i].pontua = pont;

		i++;
		}
		//if(i != 10){
		//	while(i<10){
		//	pontuacao[i].nome = NULL;
		//	pontuacao[i].pontua = NULL;
		//	i++;
		//	}
		//}
		fclose(file);
	}
}

void read_pontuacoes(){
	clear_screen(BLACK, video_mem);
	draw_string("Quadro de Pontuacoes", HRES/2-200, 100, GREEN, BLACK, 3, video_mem);
	int i = 0;
	
	while(i<10)
	{
	char* nome;
	char* str;
	int pont;
	strcpy(nome,pontuacao[i].nome);
	pont = pontuacao[i].pontua;
	itoa(pont, str,10);
	draw_string(nome, HRES/2-150, 250+(20*i), WHITE, BLACK, 1, video_mem);
	draw_string(str, HRES/2+150, 250+(20*i), WHITE, BLACK, 1, video_mem);
	i++;
	}
	while(1)
	{
		if(!queueEmpty(&teclas))
		{
			Byte tecla = queueGet(&teclas);
			if(tecla  == 0x1)
			{
				break;
			}
		}
	}
	
	draw_menu();
	
}

void change_key(Byte* key)
{
	queueInit(&teclas);
	Byte last_sc = 0;
	do
	{
		if(!queueEmpty(&teclas))
		{
			last_sc = queueGet(&teclas);
			if(last_sc == 0x2a)
				last_sc = queueGet(&teclas);
			if(!(last_sc >> 7)) // Ignorar a Release.
			{
				if(last_sc != 0x1)
				{
					*key = last_sc;
					break;
				}
			}
		}
	} while(last_sc != 0x1);
}
void config_keys()
{
	clear_screen(BLACK, video_mem);
	draw_string("Insira a nova tecla esquerda:", HRES/2-250, 200, WHITE, BLACK, 2, video_mem);
	change_key(&key_left);
	
	clear_screen(BLACK, video_mem);
	draw_string("Insira a nova tecla direita:", HRES/2-250, 200, WHITE, BLACK, 2, video_mem);
	change_key(&key_right);
	
	clear_screen(BLACK, video_mem);
	draw_string("Insira a nova tecla cima:", HRES/2-250, 200, WHITE, BLACK, 2, video_mem);
	change_key(&key_up);
	
	clear_screen(BLACK, video_mem);
	draw_string("Insira a nova tecla baixo:", HRES/2-250, 200, WHITE, BLACK, 2, video_mem);
	change_key(&key_down);

	draw_menu();
}

void rtc_irq_handler(void)
{
	Byte cause = read_rtc(RTC_STAT_C);
	
	if(cause & RTC_PF) // periodic interrupt
	{
		rtc_p++;
		switch(state)
		{
			case INACTIVE:
				break;
				
			case STOPPED:
				if(!isEmptyGQueue(musica))
				{
					Note* nt = (Note*)getGqueue(musica);
					if(nt->freq != PAUSE_FREQ)
					{
						timer_load(TIMER_2, get_divider(notes_frequency[nt->freq]));
						speaker_on();
					}
					ms_count = nt->dur;
					state = PLAY;
				}
				break;
				
			case PLAY:
				if(ms_count-- == 0)
				{
					speaker_off();
					//ms_count = 20;
					ms_count = bgm->pause;
					state = PAUSE;
					if(song_note_ind < bgm->lenght)
						putGQueue(musica, &(bgm->notes[song_note_ind++]));
				}
				break;
				
			case PAUSE:
				if(ms_count-- == 0)
					state = STOPPED;
				break;
		}
	}
	
	outportb(PIC1_CMD, EOI);
	outportb(PIC2_CMD, EOI);
}

void song_pre_load_queue(int n_notes)
{
	for(song_note_ind = 0; song_note_ind < n_notes; song_note_ind++)
		putGQueue(musica, &(bgm->notes[song_note_ind]));
}

void init()
{
	//disable_irq(RTC_IRQ);
	_go32_dpmi_seginfo old_irq;
	install_c_irq_handler(RTC_IRQ, rtc_irq_handler, &old_irq);
	old_rtc_irq = old_irq;
	//enable_irq(RTC_IRQ);
	
	bgm = song_load("song_cat.txt");
	musica = newGQueue(100, sizeof(Note));
	return_pontuacoes();
	//song_pre_load_queue(100);
	
	Byte stat_b = read_rtc(RTC_STAT_B);
	data_mode = RTC_STAT_B & RTC_DM;
	write_rtcv(RTC_STAT_B, stat_b | RTC_UIE | RTC_AIE | RTC_PIE);
}

void finalize()
{
	state = INACTIVE;
	speaker_off();		
	
	disable_irq(RTC_IRQ);
	reinstall_c_irq_handler(RTC_IRQ, &old_rtc_irq);
	enable_irq(RTC_IRQ);
	
	song_delete(bgm);
	deleteGQueue(musica);
}

void desenha_ecra()
{
	//desenha area de jogo
	draw_line(100, 150, 100, 600, WHITE, video_mem); 
	draw_line(100, 150, 600, 150, WHITE, video_mem); 
	draw_line(600, 150, 600, 600, WHITE, video_mem);
	draw_line(100,600, 600, 600, WHITE, video_mem);
	
	//desenha area de info
	draw_line(650, 150, 650, 600, WHITE, video_mem); 
	draw_line(650, 150, 923, 150, WHITE, video_mem); 
	draw_line(650, 600, 923, 600, WHITE, video_mem); 
	draw_line(923, 150, 923, 600, WHITE, video_mem); 
	
	draw_string("VIDAS J1:", 660, 200, WHITE, BLACK, 2, video_mem);
	drawIntAt(vidas, 800, 200, WHITE, BLACK, 2,video_mem);
	draw_string("VIDAS J2:", 660, 250, WHITE, BLACK, 2, video_mem);
	drawIntAt(vidas2, 800, 250, WHITE, BLACK, 2,video_mem);
	draw_string("TEMPO JOGO:", 660, 300, WHITE, BLACK, 2, video_mem);
}
int argc;
void jogar()
{
	Byte tecla;
	rtc_p = 0;
	int flag = 1;
	clear_screen(BLACK, video_mem);
	if(argc == 1)
	{
		disable_irq(COM1_IRQ);
		base = COM1_ADD;
		init_uart(base, 9600, 8, 1, PAR_NONE, true, true, true);
		enable_irq(COM1_IRQ);
	}
	else if(argc != 1)
	{
		disable_irq(COM2_IRQ);
		base = COM2_ADD;
		init_uart(base, 9600, 8, 1, PAR_NONE, true, true, true);
		enable_irq(COM2_IRQ);
	}
	init_serie();
	if(argc != 1)
	{	
		while(rcv_char_queue.cnt == 0)
		{
			draw_string("A ESPERAR PELO JOGADOR 1!", 200, 300, WHITE, BLACK, 3, video_mem);
			if(!queueEmpty(&teclas))
			{
				tecla = queueGet(&teclas);
				if(tecla  == 0x1)
				{
					envia_mensagem(base, tecla);
					flag = 0;
					break;
				}
			}
			queueGet(&rcv_char_queue);
			envia_mensagem(base, key_right);
		}
		envia_mensagem(base, key_right);
	}
	else
	{
		while(rcv_char_queue.cnt == 0)
		{
			draw_string("A ESPERAR PELO JOGADOR 2!", 200, 300, WHITE, BLACK, 3, video_mem);
			if(!queueEmpty(&teclas))
			{
				tecla = queueGet(&teclas);
				if(tecla  == 0x1)
				{
					envia_mensagem(base, tecla);
					flag = 0;
					break;
				}
			}
			queueGet(&rcv_char_queue);
			envia_mensagem(base, key_left);
		}
		envia_mensagem(base, key_left);
	}
	if(flag)
	{
		do
		{
			clear_screen(BLACK, video_mem);
			char tab[500][450];
			int i, j;
			for(i = 0; i < 499; i++)
				for(j = 0; j < 449; j++)
					tab[i][j] = 0;
			int dir_x, dir_y, dir_x2, dir_y2, x, y, x2, y2;		
			if(argc == 1)
			{
				dir_x = 1;
				dir_y = 0;
				dir_x2 = -1;
				dir_y2 = 0;
				x = 150;
				y = 375;
				x2 = 450;
				y2 = 375;
			}
			else
			{
				dir_x2 = 1;
				dir_y2 = 0;
				dir_x = -1;
				dir_y = 0;
				x2 = 150;
				y2 = 375;
				x = 450;
				y = 375;
			}
			desenha_ecra();
			while((x < 599 && x > 100 && y > 150 && y < 599) && (x2 < 599 && x2 > 100 && y2 > 150 && y2 < 599))
			{
				if(!queueEmpty(&teclas))
				{
				tecla = queueGet(&teclas);
					if(tecla  == 0x1)
					{
						envia_mensagem(base, tecla);
						break;
					}
					else if(tecla == key_down && dir_x != 0)
					{
						dir_x = 0;
						dir_y = 1;
						envia_mensagem(base, key_down_default);
					}
					else if(tecla == key_up && dir_x != 0)
					{
						dir_x = 0;
						dir_y = -1;
						envia_mensagem(base, key_up_default);
					}
					else if(tecla == key_left && dir_y != 0)
					{
						dir_x = -1;
						dir_y = 0;
						envia_mensagem(base, key_left_default);
					}
					else if(tecla == key_right && dir_y != 0)
					{
						dir_x = 1;
						dir_y = 0;
						envia_mensagem(base, key_right_default);
					}
				}
				
			while(!queueEmpty(&rcv_char_queue))
			{
				char ch = queueGet(&rcv_char_queue);
				if(ch  == 0x1)
					{
						break;
					}
					else if(ch == key_down_default && dir_x2 != 0)
					{
						dir_x2 = 0;
						dir_y2 = 1;
					}
					else if(ch == key_up_default && dir_x2 != 0)
					{
						dir_x2 = 0;
						dir_y2 = -1;
					}
					else if(ch == key_left_default && dir_y2 != 0)
					{
						dir_x2 = -1;
						dir_y2 = 0;
					}
					else if(ch == key_right_default && dir_y2 != 0)
					{
						dir_x2 = 1;
						dir_y2 = 0;
					}
			}		
				
				drawIntAt(rtc_p/1000, 830, 300, WHITE, BLACK, 2,video_mem);
				int it = 0;
				
				if(argc == 1){
					set_pixel(x+dir_x, y + dir_y, GREEN, video_mem);
					set_pixel(x2+dir_x2, y2 + dir_y2, LIGHT_BLUE, video_mem);
				}
				else
				{			
					set_pixel(x+dir_x, y + dir_y, LIGHT_BLUE, video_mem);
					set_pixel(x2+dir_x2, y2 + dir_y2, GREEN, video_mem);
				}
				
				if((x+dir_x) == (x2+dir_x2) && (y+dir_y) == (y2+dir_y2))
					break;
				
				if(tab[(x+dir_x)-100][(y+dir_y)-100] == 1 || tab[(x+dir_x)-100][(y+dir_y)-100] == 2) //perde
				{
					if(argc == 1)
						vidas--;
					else
						vidas2--;
					break;
				}
					else
						tab[(x+dir_x)-100][(y+dir_y)-100] = 1;
						
				if(tab[(x2+dir_x2)-100][(y2+dir_y2)-100] == 1 || tab[(x2+dir_x2)-100][(y2+dir_y2)-100] == 2) //perde
				{
					if(argc ==1)
						vidas2--;
					else
						vidas--;
					break;
				}
					else
						tab[(x2+dir_x2)-100][(y2+dir_y2)-100] = 2;
				
				it++;
				x = x + dir_x;
				y = y + dir_y;
				x2 = x2 + dir_x2;
				y2 = y2 + dir_y2;
				int a = rtc_p;
				a += 10;
				while(rtc_p < a);
			}
		}
		while((vidas != 0 && vidas2!=0) && tecla != 0x1);
		
		draw_string("PONTUACAO: ", HRES/2-350, 300, PURPLE, BLACK, 3, video_mem);
		if(argc == 1)
		{
			drawIntAt(vidas*(rtc_p/1000), HRES/2-100, 300, PURPLE, BLACK, 3,video_mem);
			if(vidas > 0)
				draw_string("GANHOU O JOGO!", HRES/2-150, 100, PURPLE, BLACK, 3, video_mem);
			else
				draw_string("PERDEU O JOGO!", HRES/2-150, 100, PURPLE, BLACK, 3, video_mem);
		}
		else
		{
			drawIntAt(vidas2*(rtc_p/1000), HRES/2-100, 300, PURPLE, BLACK, 3,video_mem);
			if(vidas2 > 0)
				draw_string("GANHOU O JOGO!", HRES/2-150, 100, PURPLE, BLACK, 3, video_mem);
			else
				draw_string("PERDEU O JOGO!", HRES/2-150, 100, PURPLE, BLACK, 3, video_mem);
		}
		
		rtc_p = 0;
		while(rtc_p < 2000);
		vidas = 3;
		vidas2 = 3;
	}
	set_uart_register(base, SER_IER, 0);
}

void menu_jogar()
{
	clear_screen(BLACK, video_mem);
	draw_string("LIGHT CYCLES", HRES/2-150, 100, WHITE, BLACK, 3, video_mem);
	draw_string("MENU DE JOGO", HRES/2-150, 200, WHITE, BLACK, 2, video_mem);
	draw_string("1 - CRIAR JOGO", HRES/2-150, 250, WHITE, BLACK, 2, video_mem);
	draw_string("2 - JUNTAR-SE A JOGO", HRES/2-150, 300, WHITE, BLACK, 2, video_mem);
	draw_string("ESC - SAIR", HRES/2-150, 400, WHITE, BLACK, 2, video_mem);
	do
	{
		if(!queueEmpty(&teclas))
		{
			temp = queueGet(&teclas);
			if(temp  == 0x2)
			{
				argc = 1;
				jogar();
			}
			if(temp  == 0x3)
			{
				argc = 2;
				jogar();
			}
		}
	}
	while (temp != 1);
	clear_screen(BLACK, video_mem);
}
	
void draw_menu()
{
	clear_screen(BLACK, video_mem);
	draw_string("LIGHT CYCLES", HRES/2-150, 100, WHITE, BLACK, 3, video_mem);
	draw_string("MENU", HRES/2-50, 200, WHITE, BLACK, 2, video_mem);
	draw_string("1 - JOGAR", HRES/2-150, 250, WHITE, BLACK, 2, video_mem);
	draw_string("2 - CONFIGURAR TECLAS", HRES/2-150, 300, WHITE, BLACK, 2, video_mem);
	draw_string("3 - VER PONTUACOES", HRES/2-150, 350, WHITE, BLACK, 2, video_mem);
	draw_string("ESC - SAIR", HRES/2-150, 400, WHITE, BLACK, 2, video_mem);
	do
	{
		
		if(!queueEmpty(&teclas))
		{
			temp = queueGet(&teclas);
			if(temp  == 0x2)
			{
				draw_string("1 - JOGAR", HRES/2-150, 250, WHITE, BLUE, 2, video_mem);
				draw_string("2 - CONFIGURAR TECLAS", HRES/2-150, 300, WHITE, BLACK, 2, video_mem);
				draw_string("3 - VER PONTUACOES", HRES/2-150, 350, WHITE, BLACK, 2, video_mem);
				clear_screen(BLACK, video_mem);
				menu_jogar();
				break;
			}
			if(temp  == 0x3)
			{
				draw_string("1 - JOGAR", HRES/2-150, 250, WHITE, BLACK, 2, video_mem);
				draw_string("2 - CONFIGURAR TECLAS", HRES/2-150, 300, WHITE, BLUE, 2, video_mem);
				draw_string("3 - VER PONTUACOES", HRES/2-150, 350, WHITE, BLACK, 2, video_mem);
				config_keys();
				break;
			}
			if(temp  == 0x4)
			{
				draw_string("1 - JOGAR", HRES/2-150, 250, WHITE, BLACK, 2, video_mem);
				draw_string("2 - CONFIGURAR TECLAS", HRES/2-150, 300, WHITE, BLACK, 2, video_mem);
				draw_string("3 - VER PONTUACOES", HRES/2-150, 350, WHITE, BLUE, 2, video_mem);
				read_pontuacoes();
				break;
			}
		}
	}
	while (temp != 1);
}
void printbitssimple(int n) {
	unsigned int i;
	i = 1<<7;

	while (i > 0) {
		if (n & i)
			printf("1");
		else
			printf("0");
		i >>= 1;
	}
	printf("\n");
}

int main(int a, char* argv[])
{
	queueInit(&teclas);
	disable_irq(RTC_IRQ);
	_go32_dpmi_seginfo old2;
	disable_irq(KBD_IRQ);
	install_asm_irq_handler(KBD_IRQ, asm_kbd, &old2);
	enable_irq(KBD_IRQ);
	
	int mode=0x0100 + 7;
	__dpmi_meminfo map;
	video_mem = enter_graphics(mode, &map);
	
	clear_screen(BLACK, video_mem);
	
	init();
	enable_irq(RTC_IRQ);
	state = STOPPED;
	draw_menu();
	finalize();	
	
	leave_graphics(&map);
	
	disable_irq(KBD_IRQ);
	reinstall_asm_irq_handler(KBD_IRQ, &old2);
	enable_irq(KBD_IRQ);
	finalize_serie();
	return 0;
	
/*	if(argc == 1)
	{
		base = COM1_ADD;
		init_uart(base, 9600, 8, 1, PAR_NONE, true, true, true);
	}
	else if(argc != 6)
	{
		base = COM2_ADD;
		init_uart(base, 9600, 8, 1, PAR_NONE, true, true, true);
		printf("ERRO!\n");
		//return 0;
	}
	else
	{
		base = (atoi(argv[1]) == 1) ? COM1_ADD : COM2_ADD;
		//Byte parity;
		init_uart(base, atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), true, true, true);
	}

	//mostrar informacao
	//printf("UART -> COM%i", (base == COM1_ADD) ? 1 : 2);*/
	
	/*printf("\nDATA = "); print_bin(inportb(base + SER_DATA));
	printf("\nIER = "); print_bin(inportb(base + SER_IER));
	printf("\nIIR = "); print_bin(inportb(base + SER_IIR));
	printf("\nLCR = "); print_bin(inportb(base + SER_LCR));
	printf("\nMCR = "); print_bin(inportb(base + SER_MCR));
	printf("\nLSR = "); print_bin(inportb(base + SER_LSR));
	printf("\nMSR = "); print_bin(inportb(base + SER_MSR));
	
	printf("\nBaudrate - %d", get_baud(base));
	
	printf("\nParity - ");
	Byte b = get_parity(base);
	switch(b)
	{
		case PAR_NONE: printf("none"); break;
		case PAR_ODD: printf("odd"); break;
		case PAR_EVEN: printf("even"); break;
		case PAR_HIGH: printf("high"); break;
		case PAR_LOW: printf("low"); break;
		default: break;
	}
	
	printf("\nStop bits - %i", get_stop_bits(base));
	printf("\nWord length - %i", get_nbits(base)); */
	
	/*char envia = 0;
	char recebe;
	
	do
	{
		if(kbhit())
		{
			envia = getch();
			printf("%c", envia);
			envia_mensagem(base, envia);
		}
		
		if (mensagem_espera(base))
		{
			recebe = recebe_mensagem(base);
			putchar(recebe);
			//printf("%c",recebe);
		}
		
	} while (envia != 27);	
	return 0;
	
	
	//getchar();
	return 0;*/
	/*init_serie();
	
	//enviar caracteres
	fillScreen(RED);
	drawFrame("Enviado", GREEN, 0, 0, WIDTH, HEIGHT / 2);
	drawFrame("Recebido", GREEN, 0, HEIGHT / 2, WIDTH, HEIGHT - HEIGHT / 2);
	int x_env_pos = 1, y_env_pos = 2, x_rec_pos = 1, y_rec_pos = HEIGHT / 2 + 2;

	char ch = 0;
	do
	{	
		if(!queueEmpty(&rcv_char_queue))
		{
			char ch = queueGet(&rcv_char_queue);
			printCharAt(ch, x_rec_pos, y_rec_pos, GREEN);
			
			//actualizar cursor
			x_rec_pos++;
			if(x_rec_pos == WIDTH - 1)
			{
				x_rec_pos = 1;
				y_rec_pos++;
				if(y_rec_pos == HEIGHT - 1) y_rec_pos = HEIGHT / 2 + 2;
			}
		}
		
		if(kbhit())
		{
			ch = getch();
			printCharAt(ch, x_env_pos, y_env_pos, GREEN);
			
			//actualizar cursor
			x_env_pos++;
			if(x_env_pos == WIDTH - 1)
			{
				x_env_pos = 1;
				y_env_pos++;
				if(y_env_pos == HEIGHT / 2 - 1) y_env_pos = 2;
			}
			
			//queuePut(&send_char_queue, ch);
			envia_mensagem(base, ch);
		}
	}
	while(ch != 27);
	
	finalize_serie();
	
	system("CLS");
	return 0;*/
}
