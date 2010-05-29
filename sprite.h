#ifndef _SPRITE_H_
#define _SPRITE_H_

#include "utypes.h"

/** @defgroup sprite Sprite
 * @{
 *
 * Sprite related functions
 */

/** A Sprite is an "object" that contains all needed information to
 * create, animate, and destroy a pixmap.  The functions assume that
 * the background is BLACK and they take into account collision with
 * other graphical objects or the screen limits. 
 */
typedef struct {
  int x, y;            ///< current sprite position
  int width, height;   ///< sprite dimensions
  char *map;           ///< the sprite pixmap (use read_xpm())
} Sprite;


#define CHR_H 16

/** Reads a xpm-like sprite defined in "map" (look at pixmap.h for
 * examples). Returns the address of the allocated memory where the
 * sprite was read. Updates "width" and "heigh" with the sprite
 * dimension.  Return NULL on error.
 * Assumes that VRES and HRES (the screen vertical and horizontal resolution)
 * are externaly defined.
 * 
 * Usage example, using the defined sprite in pixmap.h:
 * <pre>
 *   #include "pixmap.h" // defines  pic1, pic2, etc 
 *   int wd, hg;
 *   char *sprite = read_xpm(pic1, &wd, &hg);
 * </pre>
*/
char *read_xpm(char *map[], int *width, int *height);

/** Creates with x and y position
 * (within the screen limits), a new sprite with pixmap "pic", in
 * memory whose address is "base".
 * Returns NULL on invalid pixmap.
 */
Sprite * create_sprite(char *pic[], char *base, int x, int y);


/** The "fig" sprite is erased from memory whose address is "base"
 * and used resources released.
 */
void destroy_sprite(Sprite *fig, char *base);

void draw_sprite(Sprite *spr, char *base);

/** Apaga a sprite "fig" do ecra.
 */
void erase_sprite(Sprite *fig, char *base);


void drawCharAt(char c, int x, int y, int fore_color, int back_color, int char_scale, char *video_base);
void draw_string(char *text, int x, int y, int fore_color, int back_color, int char_scale, char *video_base);
void drawIntAt(int num, int x_ori, int y_ori, int fore_color, int back_color, int char_scale, char* video_base );

/** @} end of sprite */

#endif
