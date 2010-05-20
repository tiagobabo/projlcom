#ifndef SONG_MANIP_H_
#define SONG_MANIP_H_

#include <stdio.h>
#include <stdlib.h>

#include "music.h"

#define PAUSE_FREQ -1
/* Aloca memoria para a musica s e carrega-a com o
conteudo do ficheiro 'filename' formatado adequadamente)
*/
Song* song_load(char* filename);


/* Modifica a velocidade da musica s. Assume um compasso
de 'beats_measure' tempos (dado por 'dur' na estrutura
'Note') e com um numero de batidas por minuto 'bpm'
*/
void set_bpm(Song* s, int beats_measure, float bpm);


/* Modifica a velocidade da musica s de um numero de
batidas por minuto 'old_bpm' para 'new_bpm'
*/
void modify_bpm(Song* s, float old_bpm, float new_bpm);


/* Modifica a altura da musica s. Aumenta a altura de todas
as notas 'halfsteps_up' semitons. Um valor negativo torna a
musica mais grave.
*/
void modify_pitch(Song* s, int halfsteps_up);


/* Guarda a musica s no ficheiro 'filename', com um formato
especifico
*/
void song_save(Song* s, char* filename);


/* Liberta o espaco alocado para a musica s
*/
void song_delete(Song* s);
	
#endif
