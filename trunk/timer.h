#ifndef _TIMER_H_
#define _TIMER_H_

#include "ints.h"

/** @defgroup ContadorTempo Contador-Tempo
 * @{
 *
 * Definições do Contador-Tempo
 */
#define HLT __asm__ __volatile__("HLT")

/** @name Endereços do Timer */
/*@{*/
#define TIMER_0      0x40 ///< Endereço do Timer0
#define TIMER_1      0x41 ///< Endereço do Timer1
#define TIMER_2      0x42 ///< Endereço do Timer2
#define TIMER_CTRL   0x43 ///< Endereço do registo de controlo do timer
/*@}*/

/** @name Controlo do Speaker e Timer2  */
/*@{*/
#define SPEAKER_CTRL    0x61    ///< Endereço genérico do registo PC, que os bits 0 e 1 controlam o relógio timer2 e a saída do speaker
#define TIMER2_ENABLE  (1 << 0) ///< Activa o relógio do Timer2, primeiro bit a 1
#define SPEAKER_ENABLE (1 << 1) ///< Activa a saída do Timer2, segundo bit a 1
/*@}*/

/** @name Algumas definições usuais */
/*@{*/
#define TIMER_CLK    1193181 	///< Frequência de entrada dos timers
#define LCOM_MODE  0x36    		///< Byte de controlo: LSB seguido de MSB, modo 3, contagem binária
#define LSB(i) ((i) & 0xFF)    	///< O menos significativo Byte de uma Word
#define MSB(i) ((i) >> 8)      	///< O mais significativo Byte de uma Word
/*@}*/

/** Variáveis para comunicar com a interrupção em assembly */
extern int segundos;
extern int milis;

void t0_isr(void);

void timer_enable();

void timer_disable();

/** Programar o timer 0, 1, 2 com o um modo
 */
void timer_init(int timer, int mode);

/** Carrega o timer 0, 1, 2 com o um valor
 */
void timer_load(int timer, int value);

int get_divider(float freq);

/** Espera de mili milisegundo
*/
void mili_sleep(int mili);

/** Espera de secs segundo
 */
void sec_sleep(int secs);

/** @} fim do ContadorTempo */

#endif
