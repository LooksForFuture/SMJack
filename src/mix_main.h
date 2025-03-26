#ifndef _MIX_MAIN_
#define _MIX_MAIN_

#include <SDL2/SDL_mixer.h>

//init the audio system (sound mixing)
void mix_init(void);

//shutdown the audio devices
void mix_shutdown(void);

//loads the specified music from disk
Mix_Music* mix_loadMusic(const char* file);

//loads the specified audio chunk from disk
Mix_Chunk* mix_loadChunk(const char* file);

#endif
