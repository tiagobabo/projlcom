#ifndef _VIDEO_TEXT_H_
#define _VIDEO_TEXT_H_

#include <sys/farptr.h>		//onde estao definidas as funcoes _farpokeb() e _farpeekb()
#include <go32.h>			//onde se encontra definido '_dos_ds'
#include "utypes.h"

/** @defgroup VideoText Video-Text
 * @{
 *
 * Video in text mode definitions
 */
 
/** @name Screen and video memory definitions in text mode */
/*@{*/
#define VIDEO_TEXT 0xB8000 ///< Video address in text mode
#define WIDTH 80	   ///< Number of columns
#define HEIGHT 25	   ///< Number of rows
/*@}*/

/** @name Bit meaning in attribute byte
* (see color definitions in system header file conio.h)
*/
/*@{*/
#define BLUE_FOREG  (1 << 0)  ///< Foreground blue bit
#define GREEN_FOREG (1 << 1)  ///< Foreground green bit
#define RED_FOREG   (1 << 2)  ///< Foreground red bit

#define STRONG      (1 << 3)  ///< Foreground intensity bit

#define BLUE_BACK   (1 << 4)  ///< Background blue bit
#define GREEN_BACK  (1 << 5)  ///< Background gren bit
#define RED_BACK    (1 << 6)  ///< Background red bit

#define AQUA_BACK		(BLUE_BACK | GREEN_BACK)
#define PURPLE_BACK		(BLUE_BACK | RED_BACK)
#define YELLOW_BACK		(GREEN_BACK | RED_BACK)
#define LIGHTGRAY_BACK	(BLUE_BACK | GREEN_BACK | RED_BACK)

//#define BLINK       (1 << 7)  ///< already defined in conio.h

#define NORMAL (RED_FOREG | GREEN_FOREG | BLUE_FOREG) ///< handy definition
/*@}*/

/** @name Usefull functions */
/*@{*/


/** Calculate memory offset for writing at position x, y
*/
ulong calcMemPosition(int x, int y);

//get character at position x, y
int getCharAt(int x, int y);

//get attributes at position x, y
int getAttributesAt(int x, int y);

/** Print character ch at position x,y with attributes
* return true if sucessfull
*/
Bool printCharAt(char ch, int x, int y, char attributes);

//print line
void printLineAt(char ch, int xi, int yi, int xf, int yf, char attributes);

/** Print character array str at position x,y with attributes
* return true if sucessfull
*/
Bool printStringAt(char *str, int x, int y, char attributes);

/* print character array str at position x, y with attributes
if n_char >= 0, it will print exactly n_char characters
returns the number of chars printed
*/
int printSizedStringAt(char *str, int n_char, int x, int y, char attributes);

/** Clear screen with background color
*/
void fillScreen(char background);

/** Print integer num at position x,y with attributes.
* No C library function is used.
* return true if sucessfull
*/
Bool printIntAt(int num, int x, int y, char attributes);

/** Draw rectangular frame with left upper corner at position x,y
* with width and height dimensions, decorated with title attributes.
* return true if sucessfull
*/
Bool drawFrame(char *title, char attributes,
	int x, int y, int width, int height);

//draw table with n_lin x n_col cells (1x1 size)
Bool drawTable(char attributes, int x, int y, int n_lin, int n_col);


/*@}*/

/**@} end of VideoText */

#endif
