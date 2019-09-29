#include <SDL/SDL.h>
#include "default_text.h"
#include "load_image.h"
#include "ryan_quick_format.h"

#include <string.h>

/*helper function used by get_page_from_str() that copies a formatted text string literal
 *read from file that looks like 'formatted text\n\r\t', and convert backslash sequences
 *to the appropriate characters. Use single quotes for the string literal.
 *
 *n is the number of bytes to store to dest, includeing null character
 *
 *Returns the length of the formatted string literal, including the quotes.
 */
const char *parse_format_str(const char *src, char *dest, int n);

SDL_Surface *get_page_from_str(const char *str, Uint8 page_R, Uint8 page_G, Uint8 page_B){
	SDL_Surface *page_surface = NULL;
	SDL_Surface *tmp = NULL;
	SDL_Surface *image = NULL;
	SDL_Rect page_rct, tmp_rct;
	Uint32 rmask, gmask, bmask, amask;
	Uint32 page_color;
	
	int page_width = 850;
	int page_height = 1100;
	
	//for string parsing
	size_t str_offset = 0;
	
	//boundaries for page content
	SDL_Rect rect_text = {0, 0, 850, 1100};
	//offset for objects
	SDL_Rect rect_img = {0, 0, 850, 1100};
	
	char image_filename[128];
	int i, j;
	float w_scale, h_scale;
	Uint8 img_R, img_G, img_B, img_A;
	Uint32 image_pixel;
	
	//text prepared after parsing from file
	char ready_text[4096];
	int text_format_bold = 0;
	
	SDL_Rect text_offset = {0, 0, 850, 1100};
	
	int placeholder;
	const char *parse_return_value;
	
	int fail = 0;
	
	//taken directly from SDL 1.2 documentation.
	//RGB masks work differently bitwise depending on the computer
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		//rmask = (Uint32)(page_R) << 24;
		//gmask = (Uint32)(page_G) << 16;
		//bmask = (Uint32)(page_B) << 8;
		rmask = 0xff000000;
		gmask = 0x00ff0000;
		bmask = 0x0000ff00;
		amask = 0x000000ff;
	#else
		//rmask = (Uint32)(page_R);
		//gmask = (Uint32)(page_G) << 8;
		//bmask = (Uint32)(page_B) << 16;
		rmask = 0x000000ff;
		gmask = 0x0000ff00;
		bmask = 0x00ff0000;
		amask = 0xff000000;
	#endif
	
	page_surface = SDL_CreateRGBSurface(
		SDL_SWSURFACE,
		page_width,
		page_height,
		32,
		rmask,
		gmask, 
		bmask,
		amask);
	
	if(page_surface == NULL){
		return NULL;
	}
	//get page color and fill in in the page surface
	SDL_GetClipRect(page_surface, &page_rct);
	page_color = SDL_MapRGB(page_surface->format, page_R, page_G, page_B);
	SDL_FillRect(page_surface, &page_rct, page_color);
	
	while(str[str_offset]){
		//check first word
		str_offset += strspn(str + str_offset, " \n\r\t");
		//case of comment?
		if(str[str_offset] == '#'){
			str_offset += strcspn(str + str_offset, "\n");
		}
		//case of key words?
		else if(strncmp(str + str_offset, "page_width", 10) == 0){
			str_offset += 10;
			
			//go to equality sign, first by skipping space 
			str_offset += strspn(str + str_offset, " \n\r\t");
			if(str[str_offset] != '='){
				fail = 1;
				break;
			}
			str_offset += 1;
			//skip any space after the equality sign
			str_offset += strspn(str + str_offset, " \n\r\t");
			
			if(sscanf(str + str_offset, "%d", &page_width) != 1){
				fail = 1;
				break;
			}
			
			//create a new page image with the new width
			tmp = SDL_CreateRGBSurface(
				SDL_SWSURFACE,
				page_width,
				page_height,
				32,
				rmask,
				gmask, 
				bmask,
				amask);
			
			//copy old image to new image
			SDL_GetClipRect(page_surface, &page_rct);
			SDL_GetClipRect(tmp, &tmp_rct);
			
			//fill new page with background color
			SDL_FillRect(tmp, &tmp_rct, page_color);
			if(SDL_BlitSurface(page_surface, &page_rct, tmp, &tmp_rct) != 0){
				fail = 1;
				break;
			}
			
			SDL_FreeSurface(page_surface);
			page_surface = tmp;
			tmp = NULL;
			//update page_surface's rectangle
			SDL_GetClipRect(page_surface, &page_rct);
			
			//go past the number
			str_offset += strspn(str + str_offset, "0123456789-.");
		}
		else if(strncmp(str + str_offset, "page_height", 11) == 0){
			str_offset += 11;
			
			//go to equality sign, first by skipping space 
			str_offset += strspn(str + str_offset, " \n\r\t");
			if(str[str_offset] != '='){
				fail = 1;
				break;
			}
			str_offset += 1;
			//skip any space after the equality sign
			str_offset += strspn(str + str_offset, " \n\r\t");
			
			if(sscanf(str + str_offset, "%d", &page_height) != 1){
				fail = 1;
				break;
			}
			
			//create a new page image with the new height
			tmp = SDL_CreateRGBSurface(
				SDL_SWSURFACE,
				page_width,
				page_height,
				32,
				rmask,
				gmask, 
				bmask,
				amask);
			
			//copy old image to new image
			SDL_GetClipRect(page_surface, &page_rct);
			SDL_GetClipRect(tmp, &tmp_rct);
			
			//fill new page with background color
			SDL_FillRect(tmp, &tmp_rct, page_color);
			if(SDL_BlitSurface(page_surface, &page_rct, tmp, &tmp_rct) != 0){
				fail = 1;
				break;
			}
			
			SDL_FreeSurface(page_surface);
			page_surface = tmp;
			tmp = NULL;
			//update page_surface's rectangle
			SDL_GetClipRect(page_surface, &page_rct);
			
			//go past the number
			str_offset += strspn(str + str_offset, "0123456789-.");
		}
		else if(strncmp(str + str_offset, "tx", 2) == 0){
			str_offset += 2;
			
			//go to equality sign, first by skipping space 
			str_offset += strspn(str + str_offset, " \n\r\t");
			if(str[str_offset] != '='){
				fail = 1;
				break;
			}
			str_offset += 1;
			//skip any space after the equality sign
			str_offset += strspn(str + str_offset, " \n\r\t");
			
			if(sscanf(str + str_offset, "%d", &placeholder) != 1){
				fail = 1;
				break;
			}
			
			//go past the number
			str_offset += strspn(str + str_offset, "0123456789-.");
			
			rect_text.x = placeholder;
			text_offset.x = placeholder;
			text_offset.y = rect_text.y;
		}
		else if(strncmp(str + str_offset, "ty", 2) == 0){
			str_offset += 2;
			
			//go to equality sign, first by skipping space 
			str_offset += strspn(str + str_offset, " \n\r\t");
			if(str[str_offset] != '='){
				fail = 1;
				break;
			}
			str_offset += 1;
			//skip any space after the equality sign
			str_offset += strspn(str + str_offset, " \n\r\t");
			
			if(sscanf(str + str_offset, "%d", &placeholder) != 1){
				fail = 1;
				break;
			}
			
			//go past the number
			str_offset += strspn(str + str_offset, "0123456789-.");
			
			rect_text.y = placeholder;
			text_offset.y = placeholder;
			text_offset.x = rect_text.x;
		}
		else if(strncmp(str + str_offset, "tw", 2) == 0){
			str_offset += 2;
			
			//go to equality sign, first by skipping space 
			str_offset += strspn(str + str_offset, " \n\r\t");
			if(str[str_offset] != '='){
				fail = 1;
				break;
			}
			str_offset += 1;
			//skip any space after the equality sign
			str_offset += strspn(str + str_offset, " \n\r\t");
			
			if(sscanf(str + str_offset, "%d", &placeholder) != 1){
				fail = 1;
				break;
			}
			
			//go past the number
			str_offset += strspn(str + str_offset, "0123456789-.");
			
			rect_text.w = placeholder;
			text_offset.x = rect_text.x;
			text_offset.y = rect_text.y;
		}
		else if(strncmp(str + str_offset, "th", 2) == 0){
			str_offset += 2;
			
			//go to equality sign, first by skipping space 
			str_offset += strspn(str + str_offset, " \n\r\t");
			if(str[str_offset] != '='){
				fail = 1;
				break;
			}
			str_offset += 1;
			//skip any space after the equality sign
			str_offset += strspn(str + str_offset, " \n\r\t");
			
			if(sscanf(str + str_offset, "%d", &placeholder) != 1){
				fail = 1;
				break;
			}
			
			//go past the number
			str_offset += strspn(str + str_offset, "0123456789-.");
			
			rect_text.h = placeholder;
			text_offset.h = placeholder;
			text_offset.x = rect_text.x;
			text_offset.y = rect_text.y;
		}
		else if(strncmp(str + str_offset, "ix", 2) == 0){
			str_offset += 2;
			
			//go to equality sign, first by skipping space 
			str_offset += strspn(str + str_offset, " \n\r\t");
			if(str[str_offset] != '='){
				fail = 1;
				break;
			}
			str_offset += 1;
			//skip any space after the equality sign
			str_offset += strspn(str + str_offset, " \n\r\t");
			
			if(sscanf(str + str_offset, "%d", &placeholder) != 1){
				fail = 1;
				break;
			}
			
			//go past the number
			str_offset += strspn(str + str_offset, "0123456789-.");
			
			rect_img.x = placeholder;
		}
		else if(strncmp(str + str_offset, "iy", 2) == 0){
			str_offset += 2;
			
			//go to equality sign, first by skipping space 
			str_offset += strspn(str + str_offset, " \n\r\t");
			if(str[str_offset] != '='){
				fail = 1;
				break;
			}
			str_offset += 1;
			//skip any space after the equality sign
			str_offset += strspn(str + str_offset, " \n\r\t");
			
			if(sscanf(str + str_offset, "%d", &placeholder) != 1){
				fail = 1;
				break;
			}
			
			//go past the number
			str_offset += strspn(str + str_offset, "0123456789-.");
			
			rect_img.y = placeholder;
		}
		else if(strncmp(str + str_offset, "iw", 2) == 0){
			str_offset += 2;
			
			//go to equality sign, first by skipping space 
			str_offset += strspn(str + str_offset, " \n\r\t");
			if(str[str_offset] != '='){
				fail = 1;
				break;
			}
			str_offset += 1;
			//skip any space after the equality sign
			str_offset += strspn(str + str_offset, " \n\r\t");
			
			if(sscanf(str + str_offset, "%d", &placeholder) != 1){
				fail = 1;
				break;
			}
			
			//go past the number
			str_offset += strspn(str + str_offset, "0123456789-.");
			
			rect_img.w = placeholder;
		}
		else if(strncmp(str + str_offset, "ih", 2) == 0){
			str_offset += 2;
			
			//go to equality sign, first by skipping space 
			str_offset += strspn(str + str_offset, " \n\r\t");
			if(str[str_offset] != '='){
				fail = 1;
				break;
			}
			str_offset += 1;
			//skip any space after the equality sign
			str_offset += strspn(str + str_offset, " \n\r\t");
			
			if(sscanf(str + str_offset, "%d", &placeholder) != 1){
				fail = 1;
				break;
			}
			
			//go past the number
			str_offset += strspn(str + str_offset, "0123456789-.");
			
			rect_img.h = placeholder;
		}
		else if(strncmp(str + str_offset, "text", 4) == 0){
			str_offset += 4;
			
			//skip any whitespaces
			str_offset += strspn(str + str_offset, " \n\r\t");
			
			//check for bold keyword
			text_format_bold = 0;
			if(strncmp(str + str_offset, "bold", 4) == 0){
				text_format_bold = 1;
				str_offset += 4;
				str_offset += strspn(str + str_offset, " \n\r\t");
			}
			
			//check for opening string quote
			if(str[str_offset] != '\''){
				fail = 1;
				break;
			}
			
			ready_text[0] = '\0';
			//convert the offset length to a string by adding whitespace
			for(i = 0; i < text_offset.y - rect_text.y; i += CHARSET_H){
				strcat(ready_text, "\n");
			}
			for(i = rect_text.x; i + TAB_WIDTH < text_offset.x; i = (i / TAB_WIDTH * TAB_WIDTH) + TAB_WIDTH){
				strcat(ready_text, "\t");
			}
			for(; i < text_offset.x; i += 4){
				strcat(ready_text, " ");
			}
			
			//parse the formatted string
			parse_return_value = parse_format_str(str + str_offset, ready_text + strlen(ready_text), 4096);
			if(parse_return_value == NULL){
				fail = 1;
				break;
			}
			//note that we are not incrementing here
			str_offset = parse_return_value - str;
			
			//now we are ready to draw the text
			pw_graphics_draw_str_wrapped(ready_text, page_surface, &rect_text, 4096, 0x000000);
			if(text_format_bold){
				rect_text.x += 1;
				pw_graphics_draw_str_wrapped(ready_text, page_surface, &rect_text, 4096, 0x000000);
				rect_text.x -= 1;
			}
			
			//add text offset
			text_offset.x = rect_text.x + pw_graphics_get_offset_x(ready_text, &rect_text, 4096);
			text_offset.y = rect_text.y + pw_graphics_get_offset_y(ready_text, &rect_text, 4096);
		}
		else if(strncmp(str + str_offset, "image", 5) == 0){
			str_offset += 5;
			
			//skip whitespace
			str_offset += strspn(str + str_offset, " \n\r\t");
			
			//parse the filename string
			parse_return_value = parse_format_str(str + str_offset, image_filename, 128);
			if(parse_return_value == NULL){
				fail = 1;
				break;
			}
			str_offset = parse_return_value - str;
			
			//open the image
			image = load_image(image_filename);
			
			if(!image){
				fail = 1;
				break;
			}
			
			//calculate the scaling factor
			w_scale = ((float)(rect_img.w))/((float)(image->w));
			h_scale = ((float)(rect_img.h))/((float)(image->h));
			//map image's pixel to the page's pixel
			for(j = 0; j < rect_img.h; ++j){
				for(i = 0; i < rect_img.w; ++i){
					//get the pixel from the image
					switch(image->format->BytesPerPixel){
					case 4:
						image_pixel = *(Uint32*)((char*)(image->pixels) + (size_t)(1.0 * i / w_scale) * image->format->BytesPerPixel + (size_t)(1.0 * j / h_scale) * image->pitch);
						break;
					case 2:
						image_pixel = *(Uint16*)((char*)(image->pixels) + (size_t)(1.0 * i / w_scale) * image->format->BytesPerPixel + (size_t)(1.0 * j / h_scale) * image->pitch);
						break;
					case 1:
						image_pixel = *(Uint8*)((char*)(image->pixels) + (size_t)(1.0 * i / w_scale) * image->format->BytesPerPixel + (size_t)(1.0 * j / h_scale) * image->pitch);
						break;
					default:
						fail = 1;
						break;
					}
					
					//do conversion
					SDL_GetRGBA(image_pixel, image->format, &img_R, &img_G, &img_B, &img_A);
					image_pixel = SDL_MapRGBA(page_surface->format, img_R, img_G, img_B, img_A);
					
					//copy to the page surface
					switch(page_surface->format->BytesPerPixel){
					case 4:
						*(Uint32*)((char*)(page_surface->pixels) + (i + rect_img.x) * page_surface->format->BytesPerPixel + (j + rect_img.y) * page_surface->pitch) = image_pixel;
						break;
					case 2:
						*(Uint16*)((char*)(page_surface->pixels) + (i + rect_img.x) * page_surface->format->BytesPerPixel + (j + rect_img.y) * page_surface->pitch) = image_pixel;
						break;
					case 1:
						*(Uint8*)((char*)(page_surface->pixels) + (i + rect_img.x) * page_surface->format->BytesPerPixel + (j + rect_img.y) * page_surface->pitch) = image_pixel;
						break;
					default:
						fail = 1;
						break;
					}
				}
			}
			SDL_FreeSurface(image);
			image = NULL;
		}
		else{
			fail = 1;
			break;
		}
		//skip any spaces afterward.
		str_offset += strspn(str + str_offset, " \n\r\t");
	}
	
	if(fail){
		if(page_surface){
			SDL_FreeSurface(page_surface);
		}
		printf("Failed to create page at position %d in file\n", str_offset);
		return NULL;
	}
	
	return page_surface;
}

const char *parse_format_str(const char *src, char *dest, int n){
	/*index used for char *src*/
	int j = 0;
	/*index for dest*/
	int str_index = 0;
	int close_quote_found = 0;
	
	dest[0] = 0;
	
	/*skip the first single quote*/
	if(src[0] == '\''){
		j++;
	}
	else{
		/*return the pointer to end of string if there is no opening quote*/
		return NULL;
	}
	
	while(src[j]){
		/*break out of loop if end quote is met*/
		if(src[j] == '\''){
			close_quote_found = 1;
			break;
		}
		if(str_index < n - 1){
			/*escape sequence*/
			if(src[j] == '\\'){
				j++;
				switch(src[j]){
				case '\\':
					dest[str_index] = '\\';
					break;
				case '\'':
					dest[str_index] = '\'';
					break;
				case '"':
					dest[str_index] = '"';
					break;
				case '0':
					dest[str_index] = '\0';
					break;
				case 'n':
					dest[str_index] = '\n';
					break;
				case 'r':
					dest[str_index] = '\r';
					break;
				case 't':
					dest[str_index] = '\t';
					break;
				default:
					break;
				}
				j++;
			}
			/*normal character*/
			else{
				dest[str_index] = src[j];
				j++;
			}
			str_index++;
		}
		/*in the case of the string literal being longer than n, ignore the characters of the rest
		 *of the string literal until the end quote is found*/
		else{
			if(src[j] == '\\'){
				j += 2;
			}
			else{
				j++;
			}
		}
	}
	
	dest[str_index] = '\0';
	
	/*return the character after the close quote*/
	if(close_quote_found){
		return src + j + 1;
	}
	/*if there is no close quote, src+j points to the end of the string*/
	else{
		return NULL;
	}
}
