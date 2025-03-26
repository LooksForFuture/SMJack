#include "ui_main.h"

#include "gamedef.h"
#include "r_main.h"

#include <string.h>

float uiUnit = 0.0f; //length measuement in ui space
char textBuffer[UI_TEXT_LENGTH]; //for storing formatted text

void ui_init(void) {
    uiUnit = ((float)HEIGHT_LENGTH)/UI_HEIGHT;
}

void ui_drawText(SDL_Texture* texture, SDL_FRect* coords,
        int align, char* format, ...) {
    //clear the text buffer
    memset(&textBuffer, '\0', sizeof(textBuffer));
    int textureWidth, textureHeight;
    SDL_QueryTexture(texture, NULL, NULL, &textureWidth, &textureHeight);
    textureWidth /= 19;
    textureHeight /= 5;

    va_list args;
    va_start(args, format);
    vsprintf(textBuffer, format, args);
    va_end(args);

    int len = strlen(textBuffer);
    SDL_FRect dest;
    dest.y = coords->y * uiUnit;
    dest.w = coords->w * uiUnit;
    dest.h = coords->h * uiUnit;
    SDL_Rect crop;
    crop.w = textureWidth;
    crop.h = textureHeight;

    switch (align) {
        //align left
        case 0:
           dest.x = coords->x - coords->w;
           break;

        case 1:
           dest.x = coords->x - len / 2.0f * coords->w;
           break;

        //align right
        case 2:
            dest.x = coords->x - (len + 1) * coords->w;
            break;
    }

    dest.x *= uiUnit;
    for (int i = 0; i < len; i++) {
        dest.x += dest.w;
        char c = textBuffer[i] - 32;
        crop.y = (int)(c / 19);
        crop.x = c - crop.y * 19;
        crop.x *= textureWidth;
        crop.y *= textureHeight;
        r_renderCrop(texture, &crop, &dest);
    }
}
