/*September 28, 2019
 *ryan's quick page formatter for flexible file format,
 *must have SDL.h library included before including this file.*/

#ifndef RYAN_QUICK_FORMAT_H
#define RYAN_QUICK_FORMAT_H

/*returns 850 * 1100 image
 *returns NULL if something wrong with format string*/
SDL_Surface *get_page_from_str(const char *str, Uint8 page_R, Uint8 page_G, Uint8 page_B);

#endif