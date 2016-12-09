/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */

#include "stdafx.h"
#include <SDL.h>
#include <glew/glew.h>
#include <SDL_Mixer.h>
//#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

#include <string>
#include <map>
#include <vector>

#include "Sounds.h"
#include "AOD.h"

// ---- engine ----------------------------------------------------------------

const int AOD_Engine::Sounds::Max_channels = 64;
int AOD_Engine::Sounds::channel_playing[AOD_Engine::Sounds::Max_channels] = { };

int AOD_Engine::Sounds::sound_size = 0,
    AOD_Engine::Sounds::music_size = 0;
void AOD_Engine::Sounds::Set_Up() {
  
  if ( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 4096 ) == -1 )
    AOD::Output("Init Audio Error: " + (std::string)Mix_GetError());
  int t = Mix_AllocateChannels(AOD_Engine::Sounds::Max_channels);
  int rate, channels;
  Uint16 format;
  Mix_QuerySpec(&rate, &format, &channels);



  AOD::Output("Audio specs:");
  AOD::Output(std::to_string(rate) + " Hz");
  AOD::Output(std::to_string(format&0xFF) + " bitrate " +
    (channels > 2 ? "surround" : (channels > 1) ? "stereo" : "mono") +
    "(" + (format&0x1000 ? "BE" : "LE") + ")");
  AOD::Output("1024 bytes of audio buffer");
  AOD::Output(std::to_string(t) + " channels allocated");

  Mix_Init(MIX_INIT_OGG);
  Mix_ChannelFinished(AOD_Engine::Sounds::Stop_Sound);
}
void AOD_Engine::Sounds::Stop_Sound(int channel) {
  AOD_Engine::Sounds::channel_playing[channel] = 0;
}


// ---- Sounds ----------------------------------------------------------------

Mix_Chunk* AOD::Sounds::Load_Sound(std::string str) {
  Mix_Chunk* sample = Mix_LoadWAV(str.c_str());
  if ( sample <= 0 ) {
    AOD_Engine::Debug_Output("Error loading " + str + ": " + Mix_GetError());
    return nullptr;
  }
  return sample;
}
void AOD::Sounds::Delete_Sound(Mix_Chunk*& mix) {
  Mix_FreeChunk(mix);
  mix = nullptr;
}
int AOD::Sounds::Play_Sound(Mix_Chunk* mix, int volume, int rep) {
  if ( mix == nullptr ) return -1;
  int a = Mix_PlayChannelTimed(-1, mix, rep, -1);
  if ( a < 0 ) {
    return -1;
  } else {
    Mix_Volume(a, volume);
    AOD_Engine::Sounds::channel_playing[a] = 1;
  }
  return a;
}
bool AOD::Sounds::Channel_State(int channel) {
  return AOD_Engine::Sounds::channel_playing[channel];
}
int AOD::Sounds::R_Max_Channels() { return AOD_Engine::Sounds::Max_channels; }



// ---- Music -----------------------------------------------------------------

Mix_Music* AOD::Sounds::Load_Music(std::string str) {
  Mix_Music* sample = Mix_LoadMUS(str.c_str());
  if ( sample <= 0 ) {
    AOD_Engine::Debug_Output("Error loading " + str + ": " + Mix_GetError());
    return nullptr;
  }
  return sample;
}

void AOD::Sounds::Delete_Music(Mix_Music* mix) {
  Mix_FreeMusic(mix);
}

bool AOD::Sounds::Play_Music(Mix_Music* mix, int vol, int rep) {
  int a = Mix_PlayMusic(mix, rep);  
  Mix_VolumeMusic(vol);
  if ( a == -1 ) {
    AOD_Engine::Debug_Output("Could not play music: " +
                            std::string(Mix_GetError()));
  }
  return a;
}

void AOD::Sounds::Stop_Music() {
  Mix_HaltMusic();
}

bool AOD::Sounds::Music_State() {
  return Mix_PlayingMusic();
}
