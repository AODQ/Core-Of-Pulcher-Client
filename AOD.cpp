/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */

#include "stdafx.h"
#include <fstream>
#include <map>
#include <mutex>
#include <SDL.h>
#include <memory>

#include "AOD.h"
#include "Camera.h"
#include "Console.h"
#include "Input.h"
#include "Object.h"
#include "Realm.h"
#include "Sounds.h"
#include "Text.h"
#include "Utility.h"

// ----- CoP INCLUDES ---------------------------
#include "Console_Handler.h"
#include "Client_Vars.h"
#include "Game_Manager.h"
#include "Network.h"
#include "Player.h"

SDL_Window* AOD_Engine::screen = nullptr;
std::vector<GLuint> AOD_Engine::images;

AOD_Engine::Realm* AOD::realm = nullptr;

Uint32 AOD_Engine::ms_dt = 0;

bool AOD_Engine::started = 0;
int  AOD_Engine::start_ticks = 0;
AOD::Text* AOD::fps_display = nullptr;
float AOD_Engine::fps[20] = { 0 };

void AOD::Initialize(int window_width,int window_height, Uint32 fps,
                     char* window_name, char* icon) {
  std::ofstream fil("DEBUG.txt");
  fil << " --- init ---- \n";
  if ( realm == nullptr ) {
    if ( window_name == "" )
      window_name = "Art Of Dwarficorn";
    if ( icon == "" )
      icon = "AOD_img.bmp";
    realm = new AOD_Engine::Realm(window_width, window_height,
                                  window_name, icon);
    AOD_Engine::ms_dt = fps;
  }
  AOD::Camera::Set_Position(Vector(0,0));
  AOD::Camera::Set_Size(Vector((float)window_width,(float)window_height));
}
void AOD::Initialize_Console(bool debug, SDL_Keycode key, std::string cons) {
  if ( debug )
    AOD_Engine::Console::console_type = AOD::Console::TYPE_DEBUG_IN;
  else
    AOD_Engine::Console::console_type = AOD::Console::TYPE_DEBUG_OUT;
  AOD_Engine::Debug_Output("Created new console");
  AOD_Engine::Console::key = key;
  AOD_Engine::Console::Construct();
  //AOD_Engine::Console::console_image = Util::Load_Image(cons.c_str());
}

void AOD::Change_MSDT(Uint32 x) {
  if ( x > 0 )
    AOD_Engine::ms_dt = x;
  else
    AOD_Engine::Debug_Output("Trying to change the MS DeltaTime to a "
                             "value <= 0");
}

void AOD::Reset() {
  if ( realm != nullptr )
    realm->Reset();
}
void AOD::End() {
  if ( realm != nullptr )
    realm->~Realm();
  SDL_DestroyWindow(AOD_Engine::screen);
  SDL_Quit();
}

std::map<unsigned int, AOD::Object*> obj_list;


std::mutex add_obj_mutex;

int AOD::Add(AOD::Object* o,int layer) {
  std::lock_guard<std::mutex> add_lock ( add_obj_mutex );
  static unsigned int id_counter = 0;
  if ( realm != nullptr && o && layer >= 0 ) {
    realm->__Add(o, layer);
    o->Set_ID(id_counter++);
    obj_list.insert(std::pair<unsigned int, AOD::Object*>(o->Ret_ID(), o));
    return o->Ret_ID();
  } else {
    if ( o == nullptr )
      AOD_Engine::Debug_Output("Error: Adding nullptr text to realm");
    if ( layer >= 0 )
      AOD_Engine::Debug_Output("Error: Negative layer not allowed");
    return -1;
  }
}

std::mutex add_mutex;

void AOD::Add(Text* t) {
  std::lock_guard<std::mutex> add_lock ( add_mutex );
  if ( realm != nullptr && t != nullptr )
    realm->__Add(t);
  else {
    if ( t == nullptr )
      AOD_Engine::Debug_Output("Error: Adding nullptr text to realm");
  }
}

std::mutex rem_mutex;

void AOD::Remove(AOD::Object* o) {
  std::lock_guard<std::mutex> rem_lock ( rem_mutex );
  if ( realm != nullptr )
    realm->__Remove(o);
}
void AOD::Remove(Text* t) {
  if ( realm != nullptr )
    realm->__Remove(t);
}

void AOD::Set_BG_Colour(GLfloat r, GLfloat g, GLfloat b) {
  if ( realm == nullptr ) return;
  realm->Set_BG_Colours(r, g, b);
}

void AOD::Run() {
  if ( realm == nullptr ) return;
  float prev_dt = 0, // DT from previous frame
        curr_dt = 0, // DT for beginning of current frame
        elapsed_dt = 0, // DT elapsed between previous frame and this frame
        accumulated_dt = 0; // DT needing to be processed
  AOD_Engine::started = 1;
  AOD_Engine::start_ticks = SDL_GetTicks();
  SDL_Event _event;
  _event.user.code = 2;
  _event.user.data1 = nullptr;
  _event.user.data2 = nullptr;
  SDL_PushEvent(&_event);

  // so I can set up keys and not have to rely that update is ran first
  SDL_PumpEvents();
  AOD_Engine::Input::Refresh_Input();
  SDL_PumpEvents();
  AOD_Engine::Input::Refresh_Input();

  while ( SDL_PollEvent(&_event) ) {
    switch ( _event.type ) {
      case SDL_QUIT:
        return;
    }
  }
  prev_dt = (float)SDL_GetTicks();
  while ( true ) {
    // refresh time handlers
    curr_dt = (float)SDL_GetTicks();
    elapsed_dt = curr_dt - prev_dt;
    accumulated_dt += elapsed_dt;

    // refresh calculations
    while ( accumulated_dt >= AOD_Engine::ms_dt ) {
      PulNet::Handle_Network();
      // COP
      game_manager->Update();

      // sdl
      SDL_PumpEvents();
      AOD_Engine::Input::Refresh_Input();

      // actual update
      accumulated_dt -= AOD_Engine::ms_dt;
      AOD::realm->Update();

      std::string tex;
      std::string to_handle;
      bool alnum;
      char* chptr = nullptr;

      namespace ECons = AOD_Engine::Console;


      while ( SDL_PollEvent(&_event) ) {
        switch ( _event.type ) {
          case SDL_MOUSEWHEEL:
            if ( _event.wheel.y > 0 ) // positive away from user
              AOD::Input::keys[ MOUSEBIND::MWHEELUP ] = true;
            else if ( _event.wheel.y < 0 )
              AOD::Input::keys[ MOUSEBIND::MWHEELDOWN ] = true;
          break;
          case SDL_KEYDOWN:
            // check if backspace or copy/paste
            if ( Console::console_open ) {
              switch ( _event.key.keysym.sym ) {
                case SDLK_BACKSPACE:
                  if ( ECons::input->R_Str().size() > 0 ) {
                    ECons::input->R_Str().pop_back();
                    Update_Console_Input_Position();
                  }
                break;
                case SDLK_DELETE:
                  if ( ECons::input_after->R_Str().size() > 0 ) {
                    ECons::input_after->R_Str().erase(0, 1);
                    Update_Console_Input_Position();
                  }
                break;
                case SDLK_c: // copy
                  if ( SDL_GetModState() & KMOD_CTRL ) {
                    SDL_SetClipboardText( ECons::input->R_Str().c_str() );
                    Update_Console_Input_Position();
                  }
                break;
                case SDLK_v: // paste
                  if ( SDL_GetModState() & KMOD_CTRL ) {
                    chptr = SDL_GetClipboardText();
                    ECons::input->Set_String( chptr );
                    SDL_free(chptr);
                    Update_Console_Input_Position();
                  }
                break;
                case SDLK_LEFT: // navigate cursor left
                  tex = ECons::input->R_Str();
                  if ( tex.size() > 0 ) {
                    tex = tex[tex.size()-1];
                    ECons::input->R_Str().pop_back();
                    ECons::input_after->R_Str().insert(0, tex);

                    // skip word
                    if ( SDL_GetModState() & KMOD_CTRL ) {
                      alnum = isalnum(tex[0]);
                      while ( ECons::input->R_Str().size() > 0 ) {
                        tex = ECons::input->R_Str();
                        tex = tex[tex.size()-1];
                        if ( (bool)isalnum(tex[0]) == alnum ) {
                          ECons::input->R_Str().pop_back();
                          ECons::input_after->R_Str().insert(0, tex);
                        } else break;
                      }
                    }

                    Update_Console_Input_Position();
                  }
                break;
                case SDLK_RIGHT: // navigate cursor right
                  tex = ECons::input_after->R_Str();
                  if ( tex.size() > 0 ) {
                    tex = tex[0];
                    ECons::input_after->R_Str().erase(0, 1);
                    ECons::input->R_Str().push_back(tex[0]);

                    // skip word
                    if ( SDL_GetModState() & KMOD_CTRL ) {
                      alnum = isalnum(tex[0]);
                      while ( ECons::input_after->R_Str().size() > 0 ) {
                        tex = ECons::input_after->R_Str();
                        tex = tex[0];
                        if ( (bool)isalnum(tex[0]) == alnum ) {
                          ECons::input_after->R_Str().erase(0, 1);
                          ECons::input->R_Str().push_back(tex[0]);
                        } else break;
                      }
                    }

                    Update_Console_Input_Position();
                  }
                break;
                case SDLK_RETURN: case SDLK_RETURN2:
                  to_handle = ECons::input->R_Str() +
                              ECons::input_after->R_Str();
                  if ( to_handle != "" ) {
                    ECons::input->Set_String("");
                    ECons::input_after->Set_String("");
                    Handle_Console_Input(to_handle);
                    Update_Console_Input_Position();
                  }
                break;
                case SDLK_END:
                  ECons::input->R_Str() += ECons::input_after->R_Str();
                  ECons::input_after->R_Str().clear();
                  Update_Console_Input_Position();
                break;
                case SDLK_HOME:
                  // since appending is faster than prepending
                  ECons::input->R_Str() += ECons::input_after->R_Str();
                  ECons::input_after->R_Str() = ECons::input->R_Str();
                  ECons::input->R_Str().clear();
                  Update_Console_Input_Position();
                break;
              }
            }
          break;
          case SDL_TEXTINPUT:
            if ( AOD::Console::console_open ) {
              if ( (SDL_GetModState() & KMOD_CTRL) ||
                    _event.text.text[0] == '~' || _event.text.text[0] == '`' )
                break;
              ECons::input->R_Str() += _event.text.text;
              Update_Console_Input_Position();
            }
          break;
          case SDL_QUIT:
            return;
        }
      }
    }

    { // refresh screen
      float _FPS = 0;
      for ( int i = 0; i != 19; ++ i ) {
        AOD_Engine::fps[i+1] = AOD_Engine::fps[i];
        _FPS += AOD_Engine::fps[i+1];
      }
      AOD_Engine::fps[0] = elapsed_dt;
      _FPS += AOD_Engine::fps[0];

      if ( fps_display != nullptr ) {
        fps_display->Set_String(std::to_string(int(20000/_FPS)) + " FPS");
      }

      AOD_Engine::Console::Refresh();
      realm->Render(); // render the screen
    }

    { // sleep until temp dt reaches ms_dt
      float temp_dt = accumulated_dt;
      temp_dt = float(SDL_GetTicks()) - curr_dt;
      while ( temp_dt < AOD_Engine::ms_dt ) {
        SDL_PumpEvents();
        temp_dt = float(SDL_GetTicks()) - curr_dt;
      }
    }

    // set current frame timemark
    prev_dt = curr_dt;
  }
}

float AOD::R_MS() { return (float)AOD_Engine::ms_dt; }
float AOD::To_MS(float x) { return (x*AOD_Engine::ms_dt)/1000; }

void D_Output(std::string out) {
  std::ofstream fil("DEBUG.txt", std::ios::app);
  fil << out << '\n';
  fil.close();
  AOD_Engine::Console::to_console.push_back(out);
}

static std::mutex output_lock;

void AOD::Output(std::string out) {
  std::lock_guard<std::mutex> lock(output_lock);
  D_Output(out);
}
void AOD_Engine::Debug_Output(std::string out) {
  if ( AOD_Engine::Console::console_type == AOD::Console::TYPE_DEBUG_IN )
    D_Output(out);
}
