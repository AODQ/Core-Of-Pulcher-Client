/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#ifndef SOUNDS_H_AOD_
#define SOUNDS_H_AOD_
#pragma once

#include <GLew\glew.h>
#include <SDL.h>
#include <SDL_Mixer.h>
//#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

#include <string>
#include <vector>

namespace AOD_Engine {
  namespace Sounds { // abstracted from user
    extern int sound_size, music_size;
    extern const int Max_channels;
    extern int channel_playing[];
    void Stop_Sound(int channel);

    void Set_Up();
  }
}

namespace AOD {
  namespace Sounds { // handler from framework to user
    Mix_Chunk* Load_Sound(std::string);
    void Delete_Sound(Mix_Chunk*& mix);
    int Play_Sound(Mix_Chunk* mix, int vol = 256, int rep = 0);
    bool Channel_State(int);
    int R_Max_Channels();

    Mix_Music* Load_Music(std::string);
    void Delete_Music(Mix_Music* mix);
    bool Play_Music(Mix_Music*  mix, int vol = 256, int rep = 0);
    void Stop_Music();
    bool Music_State();
  };
}

#endif