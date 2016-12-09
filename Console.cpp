/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#include "stdafx.h"
#include <SDL.h>

#include "Client_Vars.h"
#include "Console.h"
#include "Images.h"
#include "Input.h"
#include "Object.h"

const int AOD::Console::TYPE_NONE = 0;
const int AOD::Console::TYPE_DEBUG_IN = 1;
const int AOD::Console::TYPE_DEBUG_OUT = 2;

SDL_Keycode AOD_Engine::Console::key;

namespace EngCons = AOD_Engine::Console;
int EngCons::console_type = 0;
GLuint EngCons::console_image = 0;
std::vector<AOD::Text*> EngCons::console_text;
AOD::Text* EngCons::input,
         * EngCons::input_after,
         * EngCons::input_sig;
AOD::Object* EngCons::cursor,
           * EngCons::background;
int EngCons::console_input_cursor,
    EngCons::console_input_select_len;
std::vector<std::string> EngCons::to_console;
int EngCons::console_history = 9;
unsigned char EngCons::console_message_count;

void EngCons::Construct() {
  input = new AOD::Text(12, 100, "");
  input_after = new AOD::Text(10, 100, "");
  input_sig   = new AOD::Text(0, 100, ">>");
  input_sig->Set_Visible(0);
  cursor = new AOD::Object;
  cursor->Set_Image_Size(AOD::Vector(1, 10));
  cursor->Set_Visible(0);
  cursor->Set_Position(13, 96);
  background = new AOD::Object;
  background->Set_Image_Size(AOD::Vector(CV::screen_width, 103));
  background->Set_Visible(0);
  background->Set_Position(CV::screen_width/2, 103/2);
  AOD::Add(input);
  AOD::Add(input_after);
  AOD::Add(input_sig);
  AOD::Add(background, 50);
  AOD::Add(cursor, 50);
}
void EngCons::Deconstruct() {
  for ( auto tz : console_text )
    delete tz;
  console_text.clear();
  AOD::Remove(input);
  AOD::Remove(input_after);
  AOD::Remove(cursor);
}

void EngCons::Refresh() {
  if ( console_type == AOD::Console::TYPE_DEBUG_IN ||
       console_type == AOD::Console::TYPE_DEBUG_OUT ) {
    if ( AOD::Input::keys[ AOD_Engine::Console::key ] ) {
      AOD::Console::console_open ^= 1;
      if ( AOD::Console::console_open ) {
        for ( int i = 0; i != console_text.size(); ++ i ) {
          console_text[i]->Set_Visible(1);
          console_text[i]->Set_To_Default();
        }
        input->Set_Visible(1);
        input_after->Set_Visible(1);
        input_sig->Set_Visible(1);
        cursor->Set_Visible(1);
        background->Set_Visible(1);
        SDL_StartTextInput();
      } else {
        input->Set_Visible(0);
        input_sig->Set_Visible(0);
        input_after->Set_Visible(0);
        cursor->Set_Visible(0);
        background->Set_Visible(0);
        SDL_StopTextInput();
      }

    }
    AOD::Input::keys[ AOD_Engine::Console::key ] = 0;
    if ( AOD::Console::console_open )
      for ( int i = console_text.size()-1; i != -1; -- i )
        console_text[i]->Set_Position(3, 1 + (console_text.size()-i)*10);
  }
    // push back new texts
    for ( auto i : to_console ) {
      auto txt = new AOD::Text(-20,-20,i);
      console_text.insert(console_text.begin(), txt);
    }
    to_console.clear();
    // pop back old debugs
    while ( console_text.size() > console_history ) {
      AOD::Remove(console_text[console_text.size()-1]);
      console_text.pop_back();
    }
}

namespace AODCons = AOD::Console;
bool AODCons::console_open = 0;

void AODCons::Set_Open_Console_Key(SDL_Keycode k) {
  EngCons::key = k;
}

void AODCons::Set_Console_History(int history_limit) {
  EngCons::console_history = history_limit;
}
