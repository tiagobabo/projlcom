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
#include "serial.h"
#include "video-text.h"
#include "queue.h"

int time_sound;
Note* actual = NULL;
int vidas = 3;
Queue teclas;
Queue keys_queue;
Byte temp;
//Teclas Default
Byte key_up = 0x11;
Byte key_down = 0x1f;
Byte key_left = 0x1e;
Byte key_right = 0x20;
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

void serial_isr(void)
{
	int ch = 0; 
	Bool not_full = true;
	Byte st = inportb(base + SER_IIR) & INT_ID; /* Origem de interrupção */
	
	switch(st)
	{
		case INT_ST: 
			return;

		case RX_INT:
			// queuePut(&rcv_char_queue, inportb(base + SER_DATA));
			do not_full = queuePut(&rcv_queue, inportb(base + SER_DATA));
			while (not_full && (inportb(base + SER_LSR) & RX_RDY));
			break;

		case TX_INT:
			// ch = queueGet(&send_char_queue);
			// if(ch != -1) outportb(base + SER_DATA, ch);
			do
			{
				ch = queueGet(&send_queue);
				if(ch != -1) 
				    outportb(base + SER_DATA, ch);
			}
			while(ch != -1 && (inportb(base + SER_LSR) & TX_RDY));
			
			break;
	}

	outportb(PIC1_CMD, EOI);
}


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
	bgm = song_load("song_cat.txt");
	musica = newGQueue(100, sizeof(Note));
	return_pontuacoes();
	song_pre_load_queue(100);
	
	Byte stat_b = read_rtc(RTC_STAT_B);
	data_mode = RTC_STAT_B & RTC_DM;
	write_rtcv(RTC_STAT_B, stat_b | RTC_UIE | RTC_AIE | RTC_PIE);
	
	disable_irq(RTC_IRQ);
	_go32_dpmi_seginfo old_irq;
	install_c_irq_handler(RTC_IRQ, rtc_irq_handler, &old_irq);
	old_rtc_irq = old_irq;
	enable_irq(RTC_IRQ);
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
	
	draw_string("VIDAS:", 660, 200, WHITE, BLACK, 2, video_mem);
	drawIntAt(vidas, 750, 200, WHITE, BLACK, 2,video_mem);
	draw_string("TEMPO JOGO:", 660, 250, WHITE, BLACK, 2, video_mem);
}

void jogar()
{
	Byte tecla;
	rtc_p = 0;
	do
	{
		clear_screen(BLACK, video_mem);
		char tab[500][450];
		int i, j;
		for(i = 0; i < 499; i++)
			for(j = 0; j < 449; j++)
				tab[i][j] = 0;
		int dir_x = 1;
		int dir_y = 0;
		desenha_ecra();
		int x = 150;
		int y = 375;
		while(x < 599 && x > 100 && y > 150 && y < 599)
		{
			if(!queueEmpty(&teclas))
			{
			tecla = queueGet(&teclas);
				if(tecla  == 0x1)
				{
					break;
				}
				if(tecla == key_down && dir_y != -1)
				{
					dir_x = 0;
					dir_y = 1;
				}
				if(tecla == key_up && dir_y != 1)
				{
					dir_x = 0;
					dir_y = -1;
				}
				if(tecla == key_left && dir_x != 1)
				{
					dir_x = -1;
					dir_y = 0;
				}
				if(tecla == key_right && dir_x != -1)
				{
					dir_x = 1;
					dir_y = 0;
				}
			}
			drawIntAt(rtc_p/1000, 830, 250, WHITE, BLACK, 2,video_mem);
			
			int it = 0;
			
			set_pixel(x+dir_x, y + dir_y, GREEN, video_mem);
			if(tab[(x+dir_x)-100][(y+dir_y)-100] == 1) //perde
			{
				break;
			}
				else
					tab[(x+dir_x)-100][(y+dir_y)-100] = 1;
			
			it++;
			x = x + dir_x;
			y = y + dir_y;
			int a = rtc_p;
			a += 10;
			while(rtc_p < a);
		}
		vidas--;
	}
	while(vidas > 0 && tecla != 0x1);
	draw_string("PERDEU O JOGO!", HRES/2-150, 100, PURPLE, BLACK, 3, video_mem);
	draw_string("PONTUACAO: ", HRES/2-350, 300, PURPLE, BLACK, 3, video_mem);
	drawIntAt(vidas*(rtc_p/1000), HRES/2-100, 300, PURPLE, BLACK, 3,video_mem);
	delay(2000);
	vidas = 3;
	draw_menu();
}
	
void draw_menu()
{
	clear_screen(BLACK, video_mem);
	queueInit(&teclas);
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
				jogar();
			}
			if(temp  == 0x3)
			{
				draw_string("1 - JOGAR", HRES/2-150, 250, WHITE, BLACK, 2, video_mem);
				draw_string("2 - CONFIGURAR TECLAS", HRES/2-150, 300, WHITE, BLUE, 2, video_mem);
				draw_string("3 - VER PONTUACOES", HRES/2-150, 350, WHITE, BLACK, 2, video_mem);
				config_keys();
			}
			if(temp  == 0x4)
			{
				draw_string("1 - JOGAR", HRES/2-150, 250, WHITE, BLACK, 2, video_mem);
				draw_string("2 - CONFIGURAR TECLAS", HRES/2-150, 300, WHITE, BLACK, 2, video_mem);
				draw_string("3 - VER PONTUACOES", HRES/2-150, 350, WHITE, BLUE, 2, video_mem);
				read_pontuacoes();
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

int main(int argc, char* argv[])
{
	_go32_dpmi_seginfo old2;
	disable_irq(KBD_IRQ);
	install_asm_irq_handler(KBD_IRQ, asm_kbd, &old2);
	enable_irq(KBD_IRQ);
	
	int mode=0x0100 + 7;
	__dpmi_meminfo map;
	video_mem = enter_graphics(mode, &map);
	
	clear_screen(BLACK, video_mem);
	
	init();
	state = STOPPED;
	draw_menu();
	
	finalize();	
	
	leave_graphics(&map);
	
	disable_irq(KBD_IRQ);
	reinstall_asm_irq_handler(KBD_IRQ, &old2);
	enable_irq(KBD_IRQ);
	
	return 0;
}
