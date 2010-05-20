#ifndef _SPRITE_H_
#define _SPRITE_H_

#include "video.h"
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
  int xspeed, yspeed;  ///< sprite current speed in the x and y direction
  int count;
  char *map;           ///< the sprite pixmap (use read_xpm())
} Sprite;

#define MAX_SPEED 5    /**< each sprite maximum speed in pixels/frame */
#define RIGHT_HIT 2    /**< collision with right block (WHITE)  */
#define LEFT_HIT 3     /**< collision with left block (WHITE)   */
#define MOUSE_HIT 4    /**< collision with mouse (LIGHTMAGENTA) */
#define CHR_H 16
#define BLOCK_HIT 2    /**< indica uma colisao com o bloco do jogador*/
#define H_HIT 3 /**< indica uma colisao com uma parede horizontal do ecra*/
#define V_HIT 4 /**< indica uma colisao com uma parede vertical do ecra*/

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

/** Creates with random speeds (not zero) and position
 * (within the screen limits), a new sprite with pixmap "pic", in
 * memory whose address is "base".
 * Returns NULL on invalid pixmap.
 */
Sprite * create_sprite(char *pic[], char *base);

/** Animate the sprite "fig" according to its attributes in memory, 
 * whose address is "base".
 * The animation detects the screen borders
 * and change the speed according; it also detects collision with
 * other objects, including the player pad. Collisions with the screen
 * border generates a perfect reflection, while collision with other
 * objects generates a random perturbation in the object speed. Other
 * strategies can be devised: use quasi-elastic collision based on the
 * objects "mass" and speed, generate spin effect based on amount of
 * tangential speed direction relative to the object center of
 * "mass"...  Returns the kind of collision detected, RIGHT_HIT or
 * LEFT_HIT, if a collision with the players pad (WHITE colored!) is
 * detected.
 */
int animate_sprite(Sprite *fig, char *base);

/** The "fig" sprite is erased from memory whose address is "base"
 * and used resources released.
 */
void destroy_sprite(Sprite *fig, char *base);

/** Move in memory whose address is 'base', the 'fig' cursor, a
 * standard sprite, from its current position to a new position
 * 'xstep' and 'ystep' away.  
 * The cursor is erased from its present position in xor-mode and draw
 * in the new position also oin xor-mode.
 * The first time this function is called it only draws the cursor at the
 * current position.
 */ 
void move_cursor(Sprite *fig, int xstep, int ystep, char *base);

void draw_sprite(Sprite *spr, char *base);

/** Apaga a sprite "fig" do ecra.
 */
void erase_sprite(Sprite *fig, char *base);

/** Detecta as colisoes da sprite "fig" com os varios elementos graficos presentes na interface
 */
int check_colision(Sprite *fig, char *base);

int auxiliar(int num, int x, int y, int fore_color, int back_color, int char_scale, char *video_base);
void put_int_at(int num, int x, int y, int fore_color, int back_color, int char_scale, char *video_base);
void draw_char_at(char c, int x, int y, int fore_color, int back_color, int char_scale, char *video_base);
void draw_string(char *text, int x, int y, int fore_color, int back_color, int char_scale, char *video_base);
void draw_time(int num, int x, int y, int fore_color, int back_color, int char_scale, char *video_base);

/** @} end of sprite */

#endif
