/*January 24, 2019 - Function to draw text in any colour!*/
#include <string.h>
#include <stdio.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "load_image.h"  /*for basic image loading*/
#include "default_text.h"

SDL_Surface *charset = NULL;

const char CHARSET_W[128] = 
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*0x00*/
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*0x10*/
	2,2,4,8,6,10,9,2,4,4,6,6,2,6,2,6, /*0x20*/
	6,6,6,6,6,6,6,6,6,6,2,2,5,6,5,6, /*0x30*/
	10,8,7,8,8,7,7,8,7,6,9,7,7,8,8,8, /*0x40*/
	7,9,7,7,8,8,8,10,8,8,8,4,6,4,6,7, /*0x50*/
	3,6,6,5,6,6,6,6,6,2,5,6,2,8,6,6, /*0x60*/
	6,6,5,6,4,6,6,10,6,6,6,4,2,4,8,0 /*0x70*/
};

int pw_graphics_init_default_text(){
	if(charset){
		return 0;
	}
	
	charset = load_image("charset2.png");
	
	if(!charset){
		return -1;
	}
	
	return 0;
}

void pw_graphics_quit_default_text(){
	if(charset){
		SDL_FreeSurface(charset);
	}
	charset = NULL;
}

int pw_graphics_draw_str_wrapped(const char *src, SDL_Surface *dest, SDL_Rect *offset, int len, unsigned int colour){
	int result = 0;
	SDL_Rect default_offset;
	SDL_Rect charset_clip;
	SDL_Rect dest_clip;
	int i;
	
	/*for pixels*/
	Uint32 white_pixel;
	Uint32 colour_pixel;
	Uint32 *charset_pixels;
	
	/*for word lengths*/
	size_t word_length;
	size_t word_pixel_width;
	size_t str_offset = 0;
	
	if(charset == NULL || src == NULL || dest == NULL){
		return -1;
	}
	
	/*if no offset defined, default to the destination's image width and height*/
	if(offset == NULL){
		offset = &default_offset;
		SDL_GetClipRect(dest, offset);
	}
	
	/*by default the colour key is 0x000000 (black), so don't let the colour be black*/
	if(colour == 0x000000){
		colour = 0x010101;
	}
	
	white_pixel = SDL_MapRGBA(
		charset->format,
		0xFF,
		0xFF,
		0xFF,
		0xFF);
	
	colour_pixel = SDL_MapRGBA(
		charset->format,
		(colour & 0xFF0000) >> 16,
		(colour & 0x00FF00) >> 8,
		colour & 0x0000FF,
		0xFF);
	
	
	charset_pixels = (Uint32*)charset->pixels;
	
	/*change the colour*/
	for(int i = 0; i < 12*16*12*16; ++i){
		if(charset_pixels[i] == white_pixel){
			charset_pixels[i] = colour_pixel;
		}
	}
	
	/*find where on the charset map to copy*/
	charset_clip.h = CHARSET_H;
	
	/*find where on the destination to paste*/
	dest_clip.x = offset->x;
	dest_clip.y = offset->y;
	dest_clip.w = offset->w;
	dest_clip.h = offset->h;
	
	/*draw the string*/
	while(str_offset < strlen(src) && str_offset < len){
		/*draw whitespace*/
		for(i = 0; (i < strspn(src + str_offset, " \n\r\t")) && (str_offset + i < len); ++i){
			switch(src[str_offset + i]){
			case '\n': 
				dest_clip.y += 12;
				dest_clip.h -= 12;
				break;
			case '\r':
				dest_clip.x = offset->x;
				dest_clip.w = offset->w;
				break;
			case '\t':
				dest_clip.x = (dest_clip.x / 48) * 48 + 48;
				dest_clip.w = offset->w - dest_clip.x;
				break;
			case ' ':
				dest_clip.x += CHARSET_W[' '] + 2;
				break;
			default:
				break;
			}
			
			/*do any necessary wrap-arounds*/
			if(dest_clip.x - offset->x >= offset->w){
				dest_clip.x = offset->x;
				dest_clip.w = offset->w;
				dest_clip.y += 12;
				dest_clip.h -= 12;
			}
			
			/*if it has reached beyond the bottom of the destination clip*/
			if(dest_clip.y - offset->y >= offset->h){
				break;
			}
		}
		str_offset += strspn(src + str_offset, " \n\r\t");
		
		/*individual word*/
		word_pixel_width = 0;
		for(i = 0; i < strcspn(src + str_offset, " \n\r\t"); ++i){
			word_pixel_width += CHARSET_W[src[str_offset + i]] + 2;
		}
		dest_clip.w = offset->w + offset->x - dest_clip.x;
		
		/*go to the next line if the next word is too big to fit in current line*/
		if(word_pixel_width > dest_clip.w){
			dest_clip.x = offset->x;
			dest_clip.w = offset->w;
			dest_clip.y += 12;
			dest_clip.h -= 12;
		}
		/*if it has reached beyond the bottom of the destination clip*/
		if(dest_clip.y - offset->y >= offset->h){
			break;
		}
		
		/*draw the word*/
		for(i = 0; (i < strcspn(src + str_offset, " \n\r\t")) && (str_offset + i < len); ++i){
			/*get the dimension of the current character*/
			charset_clip.w = CHARSET_W[src[str_offset + i]] + 2;
			
			/*get the offset of the character on the sprite sheet*/
			charset_clip.x = 12 * (src[str_offset + i] % 16);
			charset_clip.y = 12 * (src[str_offset + i] / 16);
			
			/*draw the character*/
			SDL_BlitSurface(charset, &charset_clip, dest, &dest_clip);
			dest_clip.x += charset_clip.w;
			dest_clip.w -= charset_clip.w;
		}
		str_offset += strcspn(src + str_offset, " \n\r\t");
	}
		
	/*change the colour back*/
	for(int i = 0; i < 12*16*12*16; ++i){
		if(charset_pixels[i] == colour_pixel){
			charset_pixels[i] = white_pixel;
		}
	}
	
	return 0;
}

int pw_graphics_get_offset_x(const char *src, SDL_Rect *offset, int len){
	size_t i = 0;
	size_t str_offset = 0;
	size_t word_pixel_width = 0;
	size_t x = 0;
	size_t y = 0;
	
	while(str_offset < strlen(src) && str_offset < len){
		//skip whitespace
		for(i = 0; i < strspn(src + str_offset, " \n\r\t") && str_offset + i < len; ++i){
			switch(src[str_offset + i]){
			case '\n':
				y += CHARSET_H;
				break;
			case '\r':
				x = 0;
				break;
			case '\t':
				x = (x / TAB_WIDTH) * TAB_WIDTH + TAB_WIDTH;
				break;
			case ' ':
				x += CHARSET_W[' '] + 2;
				break;
			default:
				break;
			}
			
			if(x >= offset->w){
				x = 0;
				y += CHARSET_H;
			}
			if(y >= offset->h){
				return y;
			}
		}
		str_offset += strspn(src + str_offset, " \n\r\t");
		
		//get word length
		
		word_pixel_width = 0;
		for(i = 0; i < strcspn(src + str_offset, " \n\r\t"); ++i){
			word_pixel_width += CHARSET_W[src[str_offset + i]] + 2;
		}
		if(x + word_pixel_width >= offset->w){
			x = 0;
			y += CHARSET_H;
		}
		x += word_pixel_width;
		str_offset += strcspn(src + str_offset, " \n\r\t");
	}
	return x;
}

int pw_graphics_get_offset_y(const char *src, SDL_Rect *offset, int len){
	size_t i = 0;
	size_t str_offset = 0;
	size_t word_pixel_width = 0;
	size_t x = 0;
	size_t y = 0;
	
	while(str_offset < strlen(src) && str_offset < len){
		//skip whitespace
		for(i = 0; i < strspn(src + str_offset, " \n\r\t") && str_offset + i < len; ++i){
			switch(src[str_offset + i]){
			case '\n':
				y += CHARSET_H;
				break;
			case '\r':
				x = 0;
				break;
			case '\t':
				x = (x / TAB_WIDTH) * TAB_WIDTH + TAB_WIDTH;
				break;
			case ' ':
				x += CHARSET_W[' '] + 2;
				break;
			default:
				break;
			}
			
			if(x >= offset->w){
				x = 0;
				y += CHARSET_H;
			}
			if(y >= offset->h){
				return y;
			}
		}
		str_offset += strspn(src + str_offset, " \n\r\t");
		
		//get word length
		
		word_pixel_width = 0;
		for(i = 0; i < strcspn(src + str_offset, " \n\r\t"); ++i){
			word_pixel_width += CHARSET_W[src[str_offset + i]] + 2;
		}
		if(x + word_pixel_width >= offset->w){
			x = 0;
			y += CHARSET_H;
		}
		x += word_pixel_width;
		str_offset += strcspn(src + str_offset, " \n\r\t");
	}
	return y;
}

int pw_graphics_draw_str(const char *src, SDL_Surface *dest, SDL_Rect *offset, int len, unsigned int colour){
	int result = 0;
	SDL_Rect default_offset;
	SDL_Rect charset_clip;
	SDL_Rect dest_clip;
	int i;
	
	/*for pixels*/
	Uint32 white_pixel;
	Uint32 colour_pixel;
	Uint32 *charset_pixels;
	
	if(charset == NULL || src == NULL || dest == NULL){
		return -1;
	}
	
	/*if no offset defined, default to the destination's image width and height*/
	if(offset == NULL){
		offset = &default_offset;
		SDL_GetClipRect(dest, offset);
	}
	
	/*by default the colour key is 0x000000 (black), so don't let the colour be black*/
	if(colour == 0x000000){
		colour = 0x010101;
	}
	
	white_pixel = SDL_MapRGBA(
		charset->format,
		0xFF,
		0xFF,
		0xFF,
		0xFF);
	
	colour_pixel = SDL_MapRGBA(
		charset->format,
		(colour & 0xFF0000) >> 16,
		(colour & 0x00FF00) >> 8,
		colour & 0x0000FF,
		0xFF);
	
	
	charset_pixels = (Uint32*)charset->pixels;
	
	/*change the colour*/
	for(int i = 0; i < 12*16*12*16; ++i){
		if(charset_pixels[i] == white_pixel){
			charset_pixels[i] = colour_pixel;
		}
	}
	
	/*find where on the charset map to copy*/
	charset_clip.h = CHARSET_H;
	
	/*find where on the destination to paste*/
	dest_clip.x = offset->x;
	dest_clip.y = offset->y;
	dest_clip.w = offset->w;
	dest_clip.h = offset->h;
	
	/*draw the string*/
	for(i = 0; (i < strlen(src)) && (i < len); ++i){
		/*get the dimension of the current character*/
		charset_clip.w = CHARSET_W[src[i]] + 2;
		
		/*get the offset of the character on the sprite sheet*/
		charset_clip.x = 12 * (src[i] % 16);
		charset_clip.y = 12 * (src[i] / 16);
		
		/*draw the character*/
		if(src[i] < 32){
			/*special characters*/
			switch(src[i]){
			case '\n': 
				dest_clip.y += 12;
				dest_clip.h -= 12;
				break;
			case '\r':
				dest_clip.x = offset->x;
				dest_clip.w = offset->w;
				break;
			case '\t':
				dest_clip.x = (dest_clip.x / 48) * 48 + 48;
				dest_clip.w = offset->w - dest_clip.x;
				break;
			default: ;
			}
		}
		else{
			/*normal characters*/
			SDL_BlitSurface(charset, &charset_clip, dest, &dest_clip);
			dest_clip.x += charset_clip.w;
		}
		
		/*do any necessary wrap-arounds*/
		if(dest_clip.x - offset->x >= offset->w){
			dest_clip.x = offset->x;
			dest_clip.w = offset->w;
			dest_clip.y += 12;
			dest_clip.h -= 12;
		}
		
		/*if it has reached beyond the bottom of the destination clip*/
		if(dest_clip.y - offset->y >= offset->h){
			break;
		}
	}
	
	/*change the colour back*/
	for(int i = 0; i < 12*16*12*16; ++i){
		if(charset_pixels[i] == colour_pixel){
			charset_pixels[i] = white_pixel;
		}
	}
	
	return 0;
}