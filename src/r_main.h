#ifndef _R_MAIN_
#define _R_MAIN_

#include <SDL2/SDL.h>

//init the render system (window, texture, etc)
void r_init(void);

//shutdown the whole rendering system and free memory
void r_shutdown(void);

//sets the render viewport with pixel coords
void r_setViewport(SDL_Rect* coords);

//recalculates render unit based on the new window size
void r_refresh(int width, int height);

//present the rendered image on screen
void r_present(void);

//set color for rendering stuff
void r_setColor(Uint8 r, Uint8 g, Uint8, Uint8 a);

//changes color modulation of the given texture
void r_setTextureColor(SDL_Texture* texture, Uint8 r, Uint8 g, Uint8 b);

//clears the screen with the color given before
void r_clearScreen(void);

//fills a rectangle on at specified coordinates
void r_fillRect(SDL_FRect* coords);

//loads the specified texture from drive
SDL_Texture* r_loadTexture(const char* file);

//renders the texture on screen at specified coordinates
void r_renderTexture(SDL_Texture* texture, SDL_FRect* coords);

//crops and renders the texture on screen at specified coordinates
void r_renderCrop(SDL_Texture* texture, SDL_Rect* crop, SDL_FRect* coords);

#endif
