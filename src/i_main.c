#include "i_main.h"

#include <SDL2/SDL.h>

void i_init(void) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "internal | initializing the system");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                "internal | could not initialize SDL");
        exit(-1);
    }
}

void i_shutdown(void) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "internal | shutting down the system");

    SDL_Quit();
}
