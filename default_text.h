/*constants for the character widths in the sprite map
 *
 *January 24, 2019
 */
#ifndef DEFAULT_TEXT_H
#define DEFAULT_TEXT_H
 
/*default height is 12 pixels*/
#define CHARSET_H 12
#define TAB_WIDTH 48
#define SPACE_WIDTH 4

extern const char CHARSET_W[128];

int pw_graphics_get_offset_x(const char *src, SDL_Rect *offset, int len);
int pw_graphics_get_offset_y(const char *src, SDL_Rect *offset, int len);

/*functions return 0 when successful, -1 when not successful*/
int pw_graphics_init_default_text();
int pw_graphics_draw_str(const char *src, SDL_Surface *dest, SDL_Rect *offset, int len, unsigned int colour);
int pw_graphics_draw_str_wrapped(const char *src, SDL_Surface *dest, SDL_Rect *offset, int len, unsigned int colour);
void pw_graphics_quit_default_text();

#endif