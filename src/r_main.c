#include "r_main.h"

#include "gamedef.h"

#include <string.h>

#include <SDL2/SDL_log.h>
#include <SDL2/SDL_image.h>

float renderUnit = 0.0f; //used for turning game length measures to pixels
float startPosX = 0.0f; //how much should the render coords be transformed in X axis
float startPosY = 0.0f; //how much should the render coords be transformed in Y axis

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

int textureCount = 0; //number of textures currently loaded
SDL_Texture* textures[16];

void r_init(void) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "render | initializing the system");

    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags)) & imgFlags) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "render | could not init image loading library");
        exit(-1);
    }

    window = SDL_CreateWindow("ShootEmUp",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            1280, 720,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "render | could not create window");
        exit(-1);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "render | failed to create a hardware renderer");
    }

    /* Our window may not have our desired size
     * because of the window manager */
    int windowWidth = 0, windowHeight = 0;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    r_refresh(windowWidth, windowHeight);
}

void r_shutdown(void) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "render | shutting down the system");

    for (int i=0; i<textureCount; i++) {
        SDL_DestroyTexture(textures[i]);
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    IMG_Quit();
};

void r_setViewport(SDL_Rect* coords) {
    SDL_RenderSetViewport(renderer, coords);
}

void r_refresh(int width, int height) {
    float w = (float)width;
    float h = (float) height;

    if (ASPECT_RATIO >= 1) {
        if (w/h >= ASPECT_RATIO) {
            renderUnit = h/HEIGHT_LENGTH;
            startPosX = w/2.0f - renderUnit * HEIGHT_LENGTH * ASPECT_RATIO / 2.0f;
            startPosY = 0.0f;
        }

        else {
            renderUnit = w / ASPECT_RATIO/ HEIGHT_LENGTH;
            startPosX = 0.0f;
            startPosY = h/2.0f - renderUnit * HEIGHT_LENGTH / 2.0f;
        }
    }

    //set new viewport
    SDL_Rect coords = {
        startPosX,
        startPosY,
        renderUnit * HEIGHT_LENGTH * ASPECT_RATIO,
        renderUnit * HEIGHT_LENGTH
    };
    r_setViewport(&coords);
};

void r_present(void) {
    SDL_RenderPresent(renderer);
}

void r_setColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}


void r_setTextureColor(SDL_Texture* texture, Uint8 r, Uint8 g, Uint8 b) {
    SDL_SetTextureColorMod(texture, r, g, b);
}

void r_fillRect(SDL_FRect* coords) {
    SDL_FRect dest;
    dest.x = coords->x * renderUnit;
    dest.y = coords->y * renderUnit;
    dest.w = coords->w * renderUnit;
    dest.h = coords->h * renderUnit;
    SDL_RenderFillRectF(renderer, &dest);
}

void r_clearScreen(void) {
    SDL_RenderClear(renderer);
}

SDL_Texture* r_loadTexture(const char* file) {
    /* add the loaded texture to the textures array
     * so that it would be cleared from memory on exit */
    SDL_Texture* texture = IMG_LoadTexture(renderer, file);
    if (texture != NULL) {
        textures[textureCount] = texture;
        textureCount++;
    }

    else SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "render | failed to load texture \"%s\"", file);

    return texture;
}

void r_renderTexture(SDL_Texture* texture, SDL_FRect* coords) {
    SDL_FRect dest;
    dest.x = coords->x * renderUnit;
    dest.y = coords->y * renderUnit;
    dest.w = coords->w * renderUnit;
    dest.h = coords->h * renderUnit;
    SDL_RenderCopyF(renderer, texture, NULL, &dest);
}

void r_renderCrop(SDL_Texture* texture, SDL_Rect* crop, SDL_FRect* coords) {
    SDL_FRect dest;
    dest.x = coords->x * renderUnit;
    dest.y = coords->y * renderUnit;
    dest.w = coords->w * renderUnit;
    dest.h = coords->h * renderUnit;

    SDL_RenderCopyF(renderer, texture, crop, &dest);
}
