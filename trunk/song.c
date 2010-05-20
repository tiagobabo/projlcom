#include "song.h"


/* Aloca memoria para a musica s e carrega-a com o
conteudo do ficheiro 'filename' formatado adequadamente)
*/
Song* song_load(char* filename)
{
	FILE* pFile;
	pFile = fopen(filename, "r");
	if(!pFile) return NULL;
	
	int beats_measure, n_notes; float bpm;
	fscanf(pFile, "%i %f %i\n", &beats_measure, &bpm, &n_notes);
	
	Note* nt = malloc(n_notes * sizeof(Note));
	if(!nt) return NULL;
	
	Song* s = malloc(sizeof(Song));
	if(!s)
	{
		free(nt);
		return NULL;
	}
	
	s->lenght = n_notes; s->pause = 0; s->notes = nt;
	
	int i;
	for(i = 0; i < n_notes; i++)
		fscanf(pFile, "%i %i\n", &(s->notes[i].freq), &(s->notes[i].dur));
	
	if(beats_measure != -1)
		set_bpm(s, beats_measure, bpm);
	
	fclose(pFile);
	return s;
}


/* Define a velocidade da musica s. Assume um compasso
de 'beats_measure' tempos (dado por 'dur' na estrutura
'Note') e com um numero de batidas por minuto 'bpm'
*/
void set_bpm(Song* s, int beats_measure, float bpm)
{
	//(beats_measure / 4) esta para (60000ms / bpm) assim como 1 esta para multiplier
	float multiplier = 240000.0 / (beats_measure * bpm);
	
	int i;
	for(i = 0; i < s->lenght; i++)
		(s->notes)[i].dur *= multiplier;
}


/* Modifica a velocidade da musica s de um numero de
batidas por minuto 'old_bpm' para 'new_bpm'
*/
void modify_bpm(Song* s, float old_bpm, float new_bpm)
{
	float multiplier = old_bpm / new_bpm;
	int i;
	for(i = 0; i < s->lenght; i++)
		(s->notes)[i].dur *= multiplier;
}


/* Modifica a altura da musica s. Aumenta a altura de todas
as notas 'halfsteps_up' semitons. Um valor negativo torna a
musica mais grave.
*/
void modify_pitch(Song* s, int halfsteps_up)
{
	int i;
	for(i = 0; i < s->lenght; i++)
		if((s->notes)[i].freq != PAUSE_FREQ)
			(s->notes)[i].freq += halfsteps_up;
}


/* Guarda a musica s no ficheiro 'filename', com um formato
especifico
*/
void song_save(Song* s, char* filename)
{
	FILE* pFile;
	pFile = fopen(filename, "w");
	if(!pFile) return;
	
	fprintf(pFile, "-1 0.0 %i\n", s->lenght);
	
	int i;
	for(i = 0; i < s->lenght; i++)
		fprintf(pFile, "%i %i\n", s->notes[i].freq, s->notes[i].dur);
	
	fclose(pFile);
}


/* Liberta o espaco alocado para a musica s
*/
void song_delete(Song* s)
{
	free(s->notes);
	free(s);
}
