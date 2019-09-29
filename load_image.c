#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include "load_image.h"

SDL_Surface *load_image(const char *filename){
	SDL_Surface *loaded_image = NULL;
	SDL_Surface *optimized_image = NULL;
	
	loaded_image = IMG_Load(filename);
	
	if(loaded_image){
		optimized_image = SDL_DisplayFormat(loaded_image);
		SDL_FreeSurface(loaded_image);
	}
	
	return optimized_image;
}