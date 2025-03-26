#include "mix_main.h"

#include <SDL2/SDL_log.h>
#include <SDL2/SDL_mixer.h>

int musicCount = 0; //number of loaded musics
Mix_Music* musics[2];

int chunkCount = 0; //number of loaded audio chunks
Mix_Chunk* chunks[4];

void mix_init(void) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "mix | initializing the system");

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "mix | could not init audio loading library");
        exit(-1);
    }
}

void mix_shutdown(void) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "mix | shutting down the system");

    for (int i = 0; i < musicCount; i++) {
        Mix_FreeMusic(musics[i]);
    }
    
    for (int i = 0; i < chunkCount; i++) {
        Mix_FreeChunk(chunks[i]);
    }

    Mix_Quit();
}

Mix_Music* mix_loadMusic(const char* file) {
    /* add the loaded music to the music array
     * to be freed upon exit */
    Mix_Music* music = Mix_LoadMUS(file);
    if (music != NULL) {
        musics[musicCount] = music;
        musicCount++;
    }

    else SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "mix | failed to load music \"%s\"", file);

    return music;
}

Mix_Chunk* mix_loadChunk(const char* file) {
    /* add the loaded audio chunk to the chunks array 
     * to be freed upon exit */
    Mix_Chunk* chunk = Mix_LoadWAV(file);
    if (chunk != NULL) {
        chunks[chunkCount] = chunk;
        chunkCount++;
    }
    
    else SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "mix | failed to load music \"%s\"", file);

    return chunk;
}
