#ifndef _UTYPES_H_
#define _UTYPES_H_

/** @defgroup UserTypes UserTypes
 * @{
 *
 * Typedefs for lazzy programmers
 */

typedef unsigned char uchar;   ///< uchar is shorter to type than unsigned char
typedef unsigned short ushort; ///< ushort is an unsigned short
typedef unsigned int uint;     ///< uint is an unsigned int
typedef unsigned long ulong;   ///< ulong is an unsigned long

typedef unsigned char Byte;    ///< 8 bits, only on i386
typedef unsigned short Word;   ///< 16 bits, only on i386 

/** User defined boolean type
* <pre>
* usage example:
* Bool done = false;
* while (done == false) {...}
* </pre>
*/
typedef enum {
  false = 0, ///< the false value
  true = 1 ///< the true value
} Bool; ///< user defined boolean type

#define BLACK 0
#define WHITE 0xF
#define GREEN 2
#define RED 4
#define BLUE 1
#define PURPLE 5
#define ORANGE 6
#define LIGHT_GREY 7
#define DARK_GREY 8
#define LIGHT_GREEN 10
#define LIGHT_BLUE 11
#define SALMON 12

/**@} end of user types */

#endif
