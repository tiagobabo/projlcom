#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
#include "queue.h"
#include "serie.h"
#include "pixmap.h"

//Variaveis globais
int tocamusica = 1;
char *video_mem;
int time_sound;
Song* bgm;
int song_note_ind = 0;
Note* actual = NULL;
Note* nota = NULL;
int dur;

typedef enum {INACTIVE, PLAY, PAUSE, STOPPED} MUSIC_STATE;
volatile MUSIC_STATE state = INACTIVE;
volatile int ms_count = 0;
volatile int rtc_a = 0;
volatile int rtc_p = 0;
volatile int rtc_u = 0;
volatile Word base;
void get_time(int *hour, int *minute, int *second);
void asm_kbd();
int curH=0, curM=0, curS=0;
extern Byte data_mode;

int vidas = 3;
int vidas2 = 3;
int IA = 1; //Tipo de Inteligencia Artificial
int argc;

//Queues a serem utilizadas
Queue teclas;
Queue rcv_char_queue, send_char_queue;
GQueue* musica;

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


_go32_dpmi_seginfo old_rtc_irq;
_go32_dpmi_seginfo old_serial_irq;

#define HLT __asm__ __volatile__("HLT")
#define STI __asm__ __volatile__("STI")
#define CLI __asm__ __volatile__("CLI")

//Estrutura do array de Pontuacoes
typedef struct {
	char* nome;
	int pontua;
} Pontuacoes;
Pontuacoes pontuacao[10];

volatile Word base;

//handler RTC
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

void serial_isr(void)
{
	int ch; Bool not_full;
	Byte st = inportb(base + SER_IIR) & INT_ID;
	switch(st)
	{
		case INT_ST: return;

		case RX_INT:
			do not_full = queuePut(&rcv_char_queue, inportb(base + SER_DATA));
			while (not_full && (inportb(base + SER_LSR) & RX_RDY));
			
			break;

		case TX_INT:
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

//Inicializacao do porto de serie
void init_serie()
{
	int serial_irq = ((base == COM1_ADD) ? COM1_IRQ : COM2_IRQ);
	install_c_irq_handler(serial_irq, serial_isr, &old_serial_irq);
	
	//inicializar IER
	set_uart_register(base, SER_IER, RX_INT_EN | TX_INT_EN);

	unmask_pic(serial_irq);
}

//Finalizacao do porto de serie
void finalize_serie()
{
	int serial_irq = ((base == COM1_ADD) ? COM1_IRQ : COM2_IRQ);
	mask_pic(serial_irq);
	set_uart_register(base, SER_IER, 0);
	reinstall_c_irq_handler(serial_irq, &old_serial_irq);
}
//Escre as Pontuacoes num ficheiro
void escreve_ficheiro_pontuacoes(){
	
	FILE *fp;
    int i;
	    fp = fopen("pont.txt", "w");
    if(fp == NULL)
	{
        perror("failed to open pont.txt");
		return;
    }
	for(i=0; i< 10; i++)
	{
		char* str = malloc((strlen(pontuacao[i].nome)*sizeof(char)));
		int pontos;
		strcpy(str ,pontuacao[i].nome);
		pontos = pontuacao[i].pontua;
		if(!strcmp(str, "")) break;
		if(i>0) 							//Todas as linhas excepto a primeira.
			fwrite("\n", 1, 1, fp);
		fwrite(str, 1, strlen(str), fp);
		fwrite("\n", 1, 1, fp);
		fprintf(fp, "%i", pontos);
	}
	fclose(fp);
	
}



//Le apartir de um ficheiro as Pontuacoes guardadas anteriormente
void le_ficheiro_pontuacoes(){
	char line[100];
	int pont;
	FILE *file;
	file=fopen("pont.txt", "r");
	if(file == NULL){
		int j;
		for(j=0; j<10; j++)
		{
			pontuacao[j].nome = "";
			pontuacao[j].pontua = 0;
		}
		return;
	}
	else{
		int i = 0;
		while(!feof(file)){
		fgets(line, 100, file);
		if(line[strlen(line)-1] == '\n')
			line[strlen(line)-1] = '\0';
		pontuacao[i].nome = malloc (strlen(line)*sizeof(char));
		strcpy(pontuacao[i].nome, line);

		fgets(line, 100, file);
		pont = atoi(line);
		pontuacao[i].pontua = pont;
		i++;
		}
		if(i != 10){
			while(i<10){
			pontuacao[i].nome = "";		//Caso nao haja jogadores na posicao i o nome ficara por default "".
			pontuacao[i].pontua = 0;
			i++;
			}
		}
		fclose(file);
	}
}

//Mostra o quadro de pontuacoes no ecra.
void mostra_pontuacoes(){
	clear_screen(BLACK, video_mem);
	draw_string("Quadro de Pontuacoes", HRES/2-200, 100, GREEN, BLACK, 3, video_mem);
	int i = 0;
	int j = 0;
	
	while(i<10)
	{
		if(strcmp(pontuacao[i].nome, ""))
		{
			drawIntAt(i+1, HRES/2-190, 250+(20*i), WHITE, BLACK, 1 , video_mem);
			draw_string("-", HRES/2-165, 250+(20*i), WHITE, BLACK, 1, video_mem);
			draw_string(pontuacao[i].nome, HRES/2-150, 250+(20*i), WHITE, BLACK, 1, video_mem);
			drawIntAt(pontuacao[i].pontua, HRES/2+150, 250+(20*i), WHITE, BLACK, 1, video_mem);
		}
		else j++;
		i++;
	}
	if(j==10)
	{
		draw_string("Ainda nao existem pontuacoes!", HRES/2-200, 400, WHITE, BLACK, 2, video_mem);
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
}

//Verifica se a pontuacao obtida e suficiente para entrar no quadro das pontuacoes e se sim insere-o.
void actualiza_pontuacao(int p)
{
	int ind, b;
	int i = 0;
	int a = 0;
	char name[3];
	for(ind = 0; ind < 10 ; ind++)
	{ 
		if(pontuacao[ind].pontua < p)
			{
				b = ind;
				draw_string("PARABENS, NOVO RECORDE!", HRES/2-350, VRES/2 - 40, PURPLE, BLACK, 2, video_mem);
				drawCharAt( 65, HRES/2 - 60, VRES/2, 15, 0, 2, video_mem);
				drawCharAt( 65, HRES/2 - 60 + 0*25, VRES/2, 15, 0, 2, video_mem);
				while(1)
				{
					
					if( !queueEmpty(&teclas))
					{
						char kbd_char = queueGet(&teclas);
						if( kbd_char == 1 ) 
						{ 
							break; 
						}
						if( kbd_char  == 28 )
						{
							 name[a] = 65+abs(i)%25;
							 i=0;
							 a++;
							 if( a >= 3 ) break;
							drawCharAt( 65, HRES/2 - 60 + a*25, VRES/2, 15, 0, 2, video_mem);
						}

						if(kbd_char == key_up)
						{
							i++;
							drawCharAt( 65+abs(i)%25, HRES/2 - 60 + a*25, VRES/2, 15, 0, 2, video_mem);
						}
						else if( kbd_char == key_down)
						{
							i--;
							drawCharAt( 65+abs(i)%25, HRES/2 - 60 + a*25, VRES/2, 15, 0, 2, video_mem);
						}
					}		
				}
				name[3] = '\0';
				int w;
				Pontuacoes temp_pont;
				for(w = 9; w > b; w--)
				{
					pontuacao[w] = pontuacao[w-1];
				}
				temp_pont.pontua = p;
				temp_pont.nome = name;
				pontuacao[b] = temp_pont;				
				break;
			}
	}
	escreve_ficheiro_pontuacoes();
	le_ficheiro_pontuacoes();
}

//Funcao para modificar as teclas de jogo
void change_key(Byte* key)
{
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

//menu de configuracao das teclas de jogo
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
}

//insere as notas na queue musica.
void song_pre_load_queue(int n_notes)
{
	for(song_note_ind = 0; song_note_ind < n_notes; song_note_ind++)
		putGQueue(musica, &(bgm->notes[song_note_ind]));
}

//Inicializacao do programa
void init()
{
	_go32_dpmi_seginfo old_irq;
	install_c_irq_handler(RTC_IRQ, rtc_irq_handler, &old_irq);
	old_rtc_irq = old_irq;
	
	bgm = song_load("song_cat.txt");
	musica = newGQueue(100, sizeof(Note));
	le_ficheiro_pontuacoes();
	song_pre_load_queue(100);
	
	Byte stat_b = read_rtc(RTC_STAT_B);
	data_mode = RTC_STAT_B & RTC_DM;
	write_rtcv(RTC_STAT_B, stat_b | RTC_UIE | RTC_AIE | RTC_PIE);
}

//Finalizacao do programa
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

//desenha o ecra de jogo
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
	
	//desenha logotipo
	create_sprite(light_cycles_logo_xpm, video_mem, HRES/2, 100);
	
	//desenha sprite do tabuleiro de jogo
	create_sprite(board_xpm, video_mem,350,375);
	create_sprite(board_aux_xpm, video_mem,787,376);
	
	//desenha info
	draw_string("VIDAS J1:", 660, 200, WHITE, BLACK, 2, video_mem);
	drawIntAt(vidas, 800, 200, WHITE, BLACK, 2,video_mem);
	draw_string("VIDAS J2:", 660, 250, WHITE, BLACK, 2, video_mem);
	drawIntAt(vidas2, 800, 250, WHITE, BLACK, 2,video_mem);
	draw_string("TEMPO JOGO:", 660, 300, WHITE, BLACK, 2, video_mem);
}

//Modo de jogo multiplayer (2 jogadores) atraves do porto de serie
void jogar_multiplayer()
{
	queueClear(&rcv_char_queue);
	Byte tecla;
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
		rtc_p = 0;
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
				x2 = 550;
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
				x = 550;
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
				
				if(tab[(x+dir_x)-100][(y+dir_y)-150] == 1) //perde
				{
					if(argc == 1)
					{
						vidas--;
						create_sprite(explosion_xpm, video_mem,(x+dir_x),(y+dir_y));
						x=200; y=200;
					}
					else
					{
						vidas2--;
						create_sprite(explosion_xpm, video_mem,(x+dir_x),(y+dir_y));
						x2=200; y2=200;
					}
					break;
				}
					else
						tab[(x+dir_x)-100][(y+dir_y)-150] = 1;
						
				if(tab[(x2+dir_x2)-100][(y2+dir_y2)-150] == 1) //perde
				{
					if(argc == 1)
					{
						vidas2--;
						create_sprite(explosion_xpm, video_mem,(x2+dir_x2),(y2+dir_y2));
						x2=200; y2=200;
					}
					else
					{
						vidas--;
						create_sprite(explosion_xpm, video_mem,(x2+dir_x2),(y2+dir_y2));
						x=200; y=200;
					}
					break;
				}
					else
						tab[(x2+dir_x2)-100][(y2+dir_y2)-150] = 1;
				
				it++;
				x = x + dir_x;
				y = y + dir_y;
				x2 = x2 + dir_x2;
				y2 = y2 + dir_y2;
				int a = rtc_p;
				a += 10;
				while(rtc_p < a);
			}
			
			if(!(x < 599 && x > 100 && y > 150 && y < 599))
			{
				create_sprite(explosion_xpm, video_mem,(x+dir_x),(y+dir_y));
				vidas--;
			}
			else if(!(x2 < 599 && x2 > 100 && y2 > 150 && y2 < 599))
			{
				create_sprite(explosion_xpm, video_mem,(x2+dir_x2),(y2+dir_y2));
				vidas2--;
			}
		}
		while((vidas != 0 && vidas2!=0) && tecla != 0x1);
		if(tecla != 0x1)
		{
			draw_string("PONTUACAO: ", HRES/2-350, 300, PURPLE, BLACK, 3, video_mem);
			if(argc == 1)
			{
				drawIntAt(vidas*(rtc_p/1000), HRES/2-100, 300, PURPLE, BLACK, 3,video_mem);
				if(vidas > 0)
				{
					draw_string("GANHOU O JOGO!", HRES/2-150, 100, PURPLE, BLACK, 3, video_mem);
					actualiza_pontuacao(vidas*(rtc_p/1000));
				}
				else
					draw_string("PERDEU O JOGO!", HRES/2-150, 100, PURPLE, BLACK, 3, video_mem);
			}
			else
			{
				drawIntAt(vidas2*(rtc_p/1000), HRES/2-100, 300, PURPLE, BLACK, 3,video_mem);
				if(vidas2 > 0)
				{
					draw_string("GANHOU O JOGO!", HRES/2-150, 100, PURPLE, BLACK, 3, video_mem);
					actualiza_pontuacao(vidas2*(rtc_p/1000));
				}
				else
					draw_string("PERDEU O JOGO!", HRES/2-150, 100, PURPLE, BLACK, 3, video_mem);
			}
			
			rtc_p = 0;
			while(rtc_p < 2000);
		}
		vidas = 3;
		vidas2 = 3;
	}
	set_uart_register(base, SER_IER, 0);
	finalize_serie();
}
char tab[500][450];

int check_best_way(int x, int y)
{
	int i= 0;
	int j= 0;
	int g= 0;
	int h= 0;
	int x_ori = x-100;
	int y_ori = y-150;
	x = x_ori;
	y = y_ori;
	while(x< 499 && x > 0)
	{
		if(tab[x][y] == 1) break;
		x++;
		i++;
	}
	x = x_ori;
	y = y_ori;
	while(x< 499 && x > 0)
	{
		if(tab[x][y] == 1) break;
		x--;
		j++;
	}
	x = x_ori;
	y = y_ori;
	while(y< 449 && y > 0)
	{
		if(tab[x][y] == 1) break;
		y++;
		g++;
	}
	x = x_ori;
	y = y_ori;
	while(y< 449 && y > 0)
	{
		if(tab[x][y] == 1) break;
		y--;
		h++;
	}
		
	if(i > j)
	{
		if(i > g)
		{
			if(i > h)
			{
				return 1;
			}
			else
				return 4;
		}
		else if(g > h)
		{
			return 3;
		}
		else
			return 4;
	}
	else if(j > g)
	{
		if(j > h)
		{
			return 2;
		}
		else
			return 4;
	}
	else if(g > h)
	{
		return 3;
	}
	else
		return 4;
}

//Modo de jogo singleplayer (1 jogador) contra Inteligencia Artificial programada.
void jogar_singleplayer()
{
	int check_int = 300;
	int check_flag= 0;
	queueClear(&rcv_char_queue);
	Byte tecla;
	clear_screen(BLACK, video_mem);
	{
		rtc_p = 0;
		do
		{
			clear_screen(BLACK, video_mem);
			
			int i, j;
			for(i = 1; i < 499; i++)
				for(j = 1; j < 449; j++)
					{
						tab[i][j] = 0;
					}			
			
			int dir_x, dir_y, dir_x2, dir_y2, x, y, x2, y2;		
			dir_x = 1;
			dir_y = 0;
			dir_x2 = -1;
			dir_y2 = 0;
			x = 150;
			y = 375;
			x2 = 550;
			y2 = 375;
			desenha_ecra();
			while((x < 599 && x > 100 && y > 150 && y < 599) && (x2 < 599 && x2 > 100 && y2 > 150 && y2 < 599))
			{
				if(!queueEmpty(&teclas))
				{
				tecla = queueGet(&teclas);
					if(tecla  == 0x1)
					{
						break;
					}
					else if(tecla == key_down && dir_x != 0)
					{
						dir_x = 0;
						dir_y = 1;
					}
					else if(tecla == key_up && dir_x != 0)
					{
						dir_x = 0;
						dir_y = -1;
					}
					else if(tecla == key_left && dir_y != 0)
					{
						dir_x = -1;
						dir_y = 0;
					}
					else if(tecla == key_right && dir_y != 0)
					{
						dir_x = 1;
						dir_y = 0;
					}
				}
								
				drawIntAt(rtc_p/1000, 830, 300, WHITE, BLACK, 2,video_mem);
				int it = 0;
				int check = 0;
				int tmp_dir_x, tmp_dir_y;
				tmp_dir_x = dir_x2;
				tmp_dir_y = dir_y2;
				
				if(IA == 1 || IA == 3)
				{
					if(check_int == 0 && IA == 1)
					{
						check_flag = 1;
						srand (time(NULL));
						check = rand() % 4;
						check_int = 300;
					}
					while(tab[(x2+dir_x2)-100][(y2+dir_y2)-150] == 1 || (x2+dir_x2) > 598 || (x2+dir_x2) < 101 || (y2+dir_y2) < 151 || (y2+dir_y2) > 598 || check_flag )
					{
							if(check == 0)
							{
								if(tmp_dir_x != 0)
								{
									dir_x2 = 0;
									dir_y2 = -1;
								}
								if(check_flag)
								{
									check = 0;
									check_flag = 0;
								}
							}
							else if(check == 1)
							{
								if(tmp_dir_x != 0)
								{
									dir_x2 = 0;
									dir_y2 = 1;
								}
								if(check_flag)
								{
									check = 0;
									check_flag = 0;
								}

							}					
							else if(check == 2)
							{
								if(tmp_dir_y != 0)
								{
									dir_x2 = 1;
									dir_y2 = 0;
								}
								if(check_flag)
								{
									check = 0;
									check_flag = 0;
								}
								
							}
							else if(check == 3)
							{
								if(tmp_dir_y != 0)
								{
									dir_x2 = -1;
									dir_y2 = 0;
								}
								if(check_flag)
								{
									check = 0;
									check_flag = 0;
								}
								
							}
							else if(check == 4)
							{
								if(check_flag)
								{
									check = 0;
									check_flag = 0;
								}
								break;
							}
							check++;
					}
				}
				else
				{
					int check = check_best_way((x2+dir_x2),(y2+dir_y2));
					
					switch(check)
					{
						case 1:
							dir_x2 = 1;
							dir_y2 = 0;
							break;
						case 2:
							dir_x2 = -1;
							dir_y2 = 0;
							break;
						case 3:
							dir_x2 = 0;
							dir_y2 = 1;
							break;
						case 4:
							dir_x2 = 0;
							dir_y2 = -1;
						default:
							break;				
					}
				}
				
					set_pixel(x+dir_x, y + dir_y, GREEN, video_mem);
					set_pixel(x2+dir_x2, y2 + dir_y2, LIGHT_BLUE , video_mem);
				
				if((x+dir_x) == (x2+dir_x2) && (y+dir_y) == (y2+dir_y2))
					break;
				
				if(tab[(x+dir_x)-100][(y+dir_y)-150] == 1) //perde
				{
					vidas--;
					create_sprite(explosion_xpm, video_mem,(x+dir_x),(y+dir_y));
					x=200; y=200;
					break;
				}
					else
						tab[(x+dir_x)-100][(y+dir_y)-150] = 1;
						
				if(tab[(x2+dir_x2)-100][(y2+dir_y2)-150] == 1) //perde
				{
					vidas2--;
					create_sprite(explosion_xpm, video_mem,(x2+dir_x2),(y2+dir_y2));
					x2=200; y2=200;
					break;
				}
					else
						tab[(x2+dir_x2)-100][(y2+dir_y2)-150] = 1;
				
				it++;
				x = x + dir_x;
				y = y + dir_y;
				x2 = x2 + dir_x2;
				y2 = y2 + dir_y2;
				int a = rtc_p;
				a += 10;
				while(rtc_p < a);
				check_int--;
			}
			if(!(x < 599 && x > 100 && y > 150 && y < 599))
			{
				create_sprite(explosion_xpm, video_mem,(x+dir_x),(y+dir_y));
				vidas--;
			}
			else if(!(x2 < 599 && x2 > 100 && y2 > 150 && y2 < 599))
			{
				create_sprite(explosion_xpm, video_mem,(x2+dir_x2),(y2+dir_y2));
				vidas2--;
			}
		}
		while((vidas != 0 && vidas2!=0) && tecla != 0x1);
		if(tecla != 0x1)
		{
			draw_string("PONTUACAO: ", HRES/2-350, 300, PURPLE, BLACK, 3, video_mem);
			drawIntAt(vidas*(rtc_p/1000), HRES/2-100, 300, PURPLE, BLACK, 3,video_mem);
			if(vidas > 0)
			{
				draw_string("GANHOU O JOGO!", HRES/2-150, 200, PURPLE, BLACK, 3, video_mem);
				actualiza_pontuacao(vidas*(rtc_p/1000));
			}
			else
				draw_string("PERDEU O JOGO!", HRES/2-150, 200, PURPLE, BLACK, 3, video_mem);
			rtc_p = 0;
			while(rtc_p < 2000);
		}
		vidas = 3;
		vidas2 = 3;
	}
}

void jogar_menu_singleplayer()
{
	Byte temp;
	clear_screen(BLACK, video_mem);
	create_sprite(light_cycles_logo_xpm, video_mem, HRES/2, 100);
	create_sprite(menu_xpm, video_mem,(HRES/2)+15,VRES/2);
	draw_string("MENU DE JOGO VS PC", HRES/2-80, 200, WHITE, BLACK, 2, video_mem);
	draw_string("1-AI 1(Around+Random)", HRES/2-150, 300, WHITE, BLACK, 2, video_mem);
	draw_string("2-AI 2(Stay Away)", HRES/2-150, 350, WHITE, BLACK, 2, video_mem);
	draw_string("3-AI 3(WallHugger)", HRES/2-150, 400, WHITE, BLACK, 2, video_mem);
	draw_string("ESC - SAIR", HRES/2-150, 450, WHITE, BLACK, 2, video_mem);
	do
	{
		if(!queueEmpty(&teclas))
		{
			temp = queueGet(&teclas);
			if(temp  == 0x2)
			{
				IA = 1;
				jogar_singleplayer();
				break;
			}
			if(temp  == 0x3)
			{
				IA = 2;
				jogar_singleplayer();
				break;
			}
			if(temp  == 0x4)
			{
				IA = 3;
				jogar_singleplayer();
				break;
			}
		}
	}
	while (temp != 1);
}

//Menu de escolha do modo de jogo.
void menu_jogar()
{
	Byte temp;
	clear_screen(BLACK, video_mem);
	create_sprite(light_cycles_logo_xpm, video_mem, HRES/2, 100);
	create_sprite(menu_xpm, video_mem,(HRES/2)+15,VRES/2);
	draw_string("MENU DE JOGO", HRES/2-100, 200, WHITE, BLACK, 2, video_mem);
	draw_string("1 - CRIAR JOGO", HRES/2-150, 300, WHITE, BLACK, 2, video_mem);
	draw_string("2 - JUNTAR-SE A JOGO", HRES/2-150, 350, WHITE, BLACK, 2, video_mem);
	draw_string("3 - JOGAR CONTRA O PC", HRES/2-150, 400, WHITE, BLACK, 2, video_mem);
	draw_string("ESC - SAIR", HRES/2-150, 450, WHITE, BLACK, 2, video_mem);
	do
	{
		if(!queueEmpty(&teclas))
		{
			temp = queueGet(&teclas);
			if(temp  == 0x2)
			{
				argc = 1;
				jogar_multiplayer();
				break;
			}
			if(temp  == 0x3)
			{
				argc = 2;
				jogar_multiplayer();
				break;
			}
			if(temp  == 0x4)
			{
				jogar_menu_singleplayer();
				break;
			}
		}
	}
	while (temp != 1);
}

//Desenha o menu principal
void draw_menu()
{
	clear_screen(BLACK, video_mem);
	create_sprite(light_cycles_logo_xpm, video_mem, HRES/2, 100);
	create_sprite(menu_xpm, video_mem,(HRES/2)+15,VRES/2);
	draw_string("MENU", HRES/2-50, 200, WHITE, BLACK, 2, video_mem);
	draw_string("1 - JOGAR", HRES/2-150, 300, WHITE, BLACK, 2, video_mem);
	draw_string("2 - CONFIGURAR TECLAS", HRES/2-150, 350, WHITE, BLACK, 2, video_mem);
	draw_string("3 - VER PONTUACOES", HRES/2-150, 400, WHITE, BLACK, 2, video_mem);
	draw_string("ESC - SAIR", HRES/2-150, 450, WHITE, BLACK, 2, video_mem);
	do
	{	
		if(!queueEmpty(&teclas))
		{
			temp = queueGet(&teclas);
			if(temp  == 0x2)
			{
				menu_jogar();
				break;
			}
			else if(temp  == 0x3)
			{
				config_keys();
				break;
			}
			else if(temp  == 0x4)
			{
				mostra_pontuacoes();
				break;
			}
		}
	}
	while (temp != 1);
}

int main(int a, char* argv[])
{
	queueInit(&teclas); //Incializacao da queue do teclado
	
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
	if(tocamusica)
		state = STOPPED;
	while(temp != 1) draw_menu();
	finalize();	
	
	leave_graphics(&map);
	
	disable_irq(KBD_IRQ);
	reinstall_asm_irq_handler(KBD_IRQ, &old2);
	enable_irq(KBD_IRQ);
	return 0;
}
