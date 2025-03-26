#ifndef _UI_MAIN_
#define _UI_MAIN_

/* While GUI relies on the rendering subsystem,
 * it uses a different unit for length */

#include <SDL2/SDL.h>

//init the ui subsystem (ui unit)
void ui_init(void);

/* draws formatted string at specified coordinates with
 * the passed texture as fontsheet.
 * align can be 0 or 1.
 * 0 for left
 * 1 for center
 * 2 for right */
void ui_drawText(SDL_Texture* texture, SDL_FRect* coords,
        int align, char* format, ...);

#endif
