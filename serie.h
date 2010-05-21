#ifndef _SERIAL_H_
#define _SERIAL_H_

#include "utypes.h"
#include <pc.h>

/** @defgroup Serial Serial
 * @{
 *
 * Serial Port definitions
 */

/** @name Serial port address*/
/*@{*/ 
#define COM1_ADD 0x3f8
#define COM2_ADD 0x2f8
/*@}*/ 

#define BAUD_FREQ 115200

/** @name The UART register addresses when DLAB=0 */
/*@{*/ 

/* DLAB = 0 */
#define SER_DATA    0 ///< DATA register
#define SER_IER     1 ///< Interrupt Enable register
#define SER_IIR     2 ///< Interrupt Identification
#define SER_FCR     2 ///< FIFO Control
#define SER_LCR     3 ///< Line Control
#define SER_MCR     4 ///< Modem Control
#define SER_LSR     5 ///< Line Status
#define SER_MSR     6 ///< Modem Status
/*@}*/ 

/** @name The UART register addresses when DLAB=1 */
/*@{*/ 

/* DLAB = 1 */
#define DIV_LSB 0 ///< Divider Low Byte
#define DIV_MSB 1 ///< Divider High Byte

#define LSB(i) ((i) & 0xFF)    ///< The Least Significative Byte of a Word
#define MSB(i) ((i) >> 8)      ///< The Most Significative Byte of a Word
/*@}*/ 

/** @name LSR bits meaning */
/*@{*/ 
#define RX_RDY 		(1 << 0) ///< Receiver Ready
#define OVERRUN_ERR	(1 << 1) ///< Overrun error
#define PARITY_ERR	(1 << 2) ///< Parity error
#define FRAME_ERR	(1 << 3) ///< Frame error
#define TX_RDY		(1 << 5) ///< Transmiter Ready
/*@}*/ 

/** @name IER  bits meaning */
/*@{*/ 
#define RX_INT_EN (1 << 0) ///< Enable Receive DATA Interrupt
#define TX_INT_EN (1 << 1) ///< Enable Transmiter Empty Interrupt
/*@}*/ 

/** @name IIR Interrupt Status and Interrupt Origin bits meaning */
/*@{*/ 
#define INT_ID  0x07 ///< the 3 bits that identify an interrupt
#define RX_INT  (1 << 2) ///< Receiver interrupt pending
#define TX_INT  (1 << 1) ///< Transmiter interrupt pending
#define INT_ST  (1 << 0) ///< No interrupt pending
/*@}*/ 

/** @name IIR FIFO status bit meaning */
/*@{*/ 
#define NO_FIFO		(0x00 << 6) ///< No FIFO available
#define NO_USE_FIFO	(0x02 << 6) ///< FIFO not usable
#define FIFO_ENABLED	(0x03 << 6) ///< FIFO is enabled
#define HAS_FIFO_64	(1 << 5) ///< UART has a 64 byte FIFO
/*@}*/ 

#define PAR_BITS	0x38		//0011 1000b
#define WORD_BITS	0x03		//0000 0011b
#define STOP_BITS	0x04		//0000 0100b

/** @name LCR bit meaning */
/*@{*/ 
#define DLAB        (1 << 7)    ///< Divisor Latch Access bit
#define PAR_NONE    (0x00 << 3) ///< No parity
#define PAR_ODD     (0x01 << 3) ///< Odd parity, odd number of 1 in the data and parity bits
#define PAR_EVEN    (0x03 << 3) ///< even parity, even number of 1 in the data and parity bits
#define PAR_HIGH    (0x05 << 3) ///< high parity parity bit always 1
#define PAR_LOW     (0x07 << 3) ///< low parity, parity bit always 0

#define WORD_5 0x00 ///< character to send or receive has 5 bits 
#define WORD_6 0x01 ///< character to send or receive has 6 bits 
#define WORD_7 0x02 ///< character to send or receive has 7 bits 
#define WORD_8 0x03 ///< character to send or receive has 8 bits 

#define STOP_1 (0 << 2) ///< one stop bit
#define STOP_2 (1 << 2) ///< two stop bits
/*@}*/ 

/** @name MCR bit meanings */
/*@{*/ 
#define AUX_OUT2 (1 << 3) ///< auxiliar autput bit. Used in some motherboards to enable generation of UART interrupts
/*@}*/ 

/** @name FCR bit meanings */
/*@{*/ 
#define CLR_TX_FIFO	(1 << 2) ///< Clear Transmiter FIFO
#define CRL_RX_FIFO	(1 << 1) ///< Clear Receiver FIFO
#define EN_FIFO		(1 << 0) ///< enable FIFOs
#define EN_FIFO_64	(1 << 5) ///< enable 64 bytes FIFOs (only some UART)
#define FIFO_INT_1	(0 << 6) ///< Interrupt generated when FIFO has only 1 character
#define FIFO_INT_4	(0x01 << 6) ///< Interrupt generated when FIFO has only 4 characters
#define FIFO_INT_8	(0x02 << 6) ///< Interrupt generated when FIFO has only 8 characters
#define FIFO_INT_14	(0x03 << 6) ///< Interrupt generated when FIFO has only 14 characters
/*@}*/ 

/** @} end of serial */

//Inicio das funções da alínea 1

int get_baud(Word base);
void set_baud(Word base, int rate);

Byte get_parity(Word base);
void set_parity(Word base, Byte par);

int get_nbits(Word base);
void set_nbits(Word base, int nbits);

int get_stop_bits(Word base);
void set_stop_bits(Word base, int stop);

//Fim das funções da alínea 1

Byte get_uart_register(Word base, Word reg);
void set_uart_register(Word base, Word reg, Byte b);

void init_uart(Word base, int rate, int nbits, int stop_bits, Byte parity, Bool rx_int_enable, Bool tx_int_enable, Bool fifo_enable);


Bool mensagem_espera(Word base);
char recebe_mensagem(Word base);
void envia_mensagem(Word base, char c);

#endif
