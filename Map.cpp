/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#include "Game_Manager.h"
#include "Data.h"
#include "Map.h"

#include <atomic>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

// ---- tile sheet container --------------------
Tile_Sheet_Container::Tile_Sheet_Container() {
  sheet = SheetContainer();
  file_name = "";
  edges.clear();
}

// ---- map loading utility ---------------------


static std::string Extract_File_Name(char* str, int& pos) {
    bool hit_dot = 0; // check for file extension before space
    std::string dat = "";
    while ( ++pos ) {
        if (  hit_dot && str[pos] == ' ' ) return dat;
        if ( !hit_dot && str[pos] == '.' ) hit_dot = 1;
        if ( str[pos] == '\\' )            hit_dot = 0;
        dat += str[pos];
    }
}
static std::string Extract_File_Name(std::ifstream& fil) {
    bool hit_dot = 0;
    std::string dat = "";
    while ( !isalnum(fil.peek()) ) fil.get(); // remove unwanted characters
    while ( true ) {
        if (  hit_dot && fil.peek() == ' ' ) return dat;
        if ( !hit_dot && fil.peek() == '.' ) hit_dot = 1;
        dat += fil.get();
    }
}
static void Remove_Whitespace(std::ifstream& fil) {
    // remove all whitespace until non whitespace character
    while ( fil.peek() == '\n' || fil.peek() == ' ' ) fil.get();
}

// Whitespace extract
static std::string WS_Extract(std::string& t, int& it) {
  std::string res = "";
  if ( it >= t.size() ) return "";
  while ( t[it] == ' ' || t[it] == '\n' ) {
    ++ it;
    if ( it >= t.size() ) return "";
  }
  while ( t[it] != ' ' || t[it] == '\n' ) {
    res += t[it];
    ++ it;
    if ( t.size() <= it ) return res;
  }
  return res;
}

template <typename T>
static inline T& last(std::vector<T>& x) { return x[x.size()-1]; }

// ---- map loader ------------------------------


static std::string Load_Map_Data(const char* file_nam) {
  std::ifstream fil(file_nam, std::ios::binary);
  ret_assert2(fil.good(), "Error opening file " + std::string(file_nam), "");
  std::string map_version = "";
  int it = 0;
  {
    char c;
    while ( true ) {
      fil.read((char*)&c, sizeof c);
      if ( c == ' ' ) break;
      map_version += c;
    }
    // get integer
    int map_v = int(std::stof(map_version)*1000.f);
    switch ( map_v ) {
      case 100:
        return "m_outdated";
      case 200: break; // current
      default:
        return "outdated";
    }
  }

  while ( true )
      if ( fil.get() == '@' ) break;
  // create files
  std::stringstream map_str(std::ios::binary | std::ios::in | std::ios::out);
  bool first_at_tripped = false;
  char c;
  // load data. it should follow format:
  /* MAPNAME@210 20 (height, width)
      0 1 64 96 0 0 1 1 ( tile information)
  */

  game_manager->State_Update(Loading_State::loading_map_data);

  while ( true ) {
      fil.read(     (char*)&c, sizeof c);
      map_str.write((char*)&c, sizeof c);
      if ( fil.peek() == '@' )
          if ( first_at_tripped ) break;
          else first_at_tripped = 1;
  }
    // convert new lines to spaces
  auto map_str_n = map_str.str();
  for ( int i = 0; i != map_str_n.size(); ++ i )
      if ( map_str_n[i] == '\n' ) map_str_n[i] = ' ';
  while ( map_str_n[map_str_n.size()-1] == ' ' ) // just to double check that whitespace isn't filling the end
      map_str_n.pop_back();
  map_str_n.push_back(' '); // however still want a single whitespace character in there just to check for EOF
  fil.close();
  return map_str_n;
}

static void Skip_WS(std::string& t, int& it) {
  while ( t[it] == ' ' || t[it] == '\n' )
    ++it;
}

// creates directory, loads files into that directory, and returns list
// of file information
static std::string Load_Files(const char* file_nam) {
    std::string file_name = file_nam;
    // extract information from files (sheets and backgrounds)
    std::vector<std::string> sheet_info;
    std::ifstream fil(file_name, std::ios::binary);
    ret_assert2(fil.good(), "Error opening file " + file_name, "");
    char c;
    // skip version
    while ( true ) {
    fil.read((char*)&c, sizeof c);
      if ( c == ' ' ) break;
    }
    game_manager->State_Update(Loading_State::extracting_sheet_data);
    while ( fil.peek() != '^' ) {
        sheet_info.push_back("");
        sheet_info[sheet_info.size()-1] = "temp\\" + Extract_File_Name(fil);
        Remove_Whitespace(fil);
    }
    fil.get();
    using background_type =
          std::tuple<std::string,bool,bool,bool, int,int,int,int>;
    std::vector<background_type> background_info;
    
    game_manager->State_Update(Loading_State::extracting_background_data);    
    while ( fil.peek() != '&' ) { // background information to load
      background_info.push_back(background_type("",0,0,0,0,0,0,0));
      std::get<0>(last(background_info)) = "temp\\" + Extract_File_Name(fil);
      fil >> std::get<1>(last(background_info)) >>
             std::get<2>(last(background_info)) >>
             std::get<3>(last(background_info)) >>
             std::get<4>(last(background_info)) >>
             std::get<5>(last(background_info)) >>
             std::get<6>(last(background_info)) >>
             std::get<7>(last(background_info));
      Remove_Whitespace(fil);
    }
    fil.get();

    // scripts
    std::vector<std::string> script_info;
    game_manager->State_Update(Loading_State::extracting_script_data);
    while ( fil.peek() != '@' ) {
      script_info.push_back("");
      script_info[script_info.size()-1] = "temp\\" + Extract_File_Name(fil);
      Remove_Whitespace(fil);
    }
    // skip map data
    while ( fil.peek() != '$' ) fil.get(); fil.get();
    //Remove_Whitespace(fil);

    std::string results = "";
    char d;

    // output file information from the sheet info (order matters still).
    // we use string @ENDOFFILE@ to tell the file that it has ended 
    // The probabilities of a file containing that specific sub-str by accident
    // probably contains a lot of digits.
    game_manager->State_Update(Loading_State::loading_header_data);
    for ( int i = 0; i != sheet_info.size(); ++ i ) {
      fil.read((char*)&d, sizeof d); // Skip '\0'
      std::ofstream t_out(sheet_info[i], std::ios::binary);
      results += " \"" + sheet_info[i] + '"';
      ret_assert2(t_out.good(), "Error loading file: " + sheet_info[i], "");
      // loop until '@' hit, save a copy of next 12 iterations
      // so that it can be placed back if it was not "@ENDOFFILE@"
      while ( true ) {
          fil.read((char*)&d, sizeof d);
          t_out.write((char*)&d, sizeof d);
          if ( fil.peek() == '@' ) { // check end of file,
                                      // keep a copy in case !EOF
              std::stringstream t(std::ios::binary | std::ios::in |
                                  std::ios::out);
              for ( int i = 0; i != 12; ++ i ) {
                  fil.read((char*)&d, sizeof d);
                  t.write((char*)&d, sizeof d);
                  // we might have hit the actual @ENDOFFILE@.
                  if ( fil.peek() == '@' ) {
                      fil.read((char*)&d, sizeof d);
                      t.write( (char*)&d, sizeof d);
                      break;
                  }
              }
              if ( t.str() == std::string("@ENDOFFILE@") ) {
                std::cerr << "(Success)\n";
                break;
              } else {
                  while ( t.good() ) // put contents back into t_out
                      while ( t.read((char*)&d, sizeof d) )
                          t_out.write((char*)&d, sizeof d);
              }
          }
      }
      t_out.close();
    }

    game_manager->State_Update(Loading_State::loading_backgrounds);
    // input background information (same process as above)
    results += " ^";
    for ( int i = 0; i != background_info.size(); ++ i ) {
      fil.read((char*)&d, sizeof d);
      std::ofstream t_out; 
      t_out.open("temp\\" + std::get<0>(background_info[i]), std::ios::binary);
      ret_assert2(t_out.good(), "Error opening background " + std::string(
                  std::get<0>(background_info[i])), "");

      results += "\"" + std::get<0>(background_info[i]) + "\" ";
      results += std::to_string(std::get<1>(background_info[i])) + ' ' +
                 std::to_string(std::get<2>(background_info[i])) + ' ' +
                 std::to_string(std::get<3>(background_info[i])) + ' ' +
                 std::to_string(std::get<4>(background_info[i])) + ' ' +
                 std::to_string(std::get<5>(background_info[i])) + ' ' +
                 std::to_string(std::get<6>(background_info[i])) + ' ' +
                 std::to_string(std::get<7>(background_info[i])) + ' ';
      while ( true ) { 
        fil.read((char*)&d, sizeof d);
        t_out.write((char*)&d, sizeof d);
        std::cerr << "Writing: " << d << '\n';
        if ( fil.peek() == '@' ) {
          std::cerr << "Found an '@'";
          std::stringstream t(std::ios::binary | std::ios::in | std::ios::out );
          for ( int i = 0; i != 12; ++ i ) {
            fil.read((char*)&d, sizeof d); 
            t.write((char*)&d, sizeof d);
            if ( fil.peek() == '@' ) {
              fil.read((char*)&d, sizeof d);
              t.write( (char*)&d, sizeof d);
              break;
            }
          }
          if ( t.str() == std::string("@ENDOFFILE@") ) {
            std::cerr << "(Success)\n";
            break;
          } else {
            while ( t.good() )
              while ( t.read((char*)&d, sizeof d) )
                  t_out.write((char*)&d, sizeof d);
          }
        }
      }
      t_out.close();
    }
    results += " &";
    
    game_manager->State_Update(Loading_State::loading_scripts);
    // input scripts information
    fil.read((char*)&d, sizeof d); // skip space
    for ( int i = 0; i != script_info.size(); ++ i ) {
      std::ofstream t_out;
      t_out.open(script_info[i], std::ios::binary);
      ret_assert2(t_out.good(), "Error opening script " + script_info[i], "");
      results += "\"" + script_info[i] + "\" ";
      while ( true ) {
        fil.read((char*)&d, sizeof d);
        t_out.write((char*)&d, sizeof d);
        if ( fil.peek() == '@' ) {
          std::stringstream t(std::ios::binary | std::ios::in | std::ios::out );
          for ( int i = 0; i != 12; ++ i ) {
            fil.read((char*)&d, sizeof d);
            t.write( (char*)&d, sizeof d);
            if ( fil.peek() == '@' ) {
              fil.read((char*)&d, sizeof d);
              t.write( (char*)&d, sizeof d);
              break;
            }
          }
          if ( t.str() == std::string("@ENDOFFILE@") ) break;
          else {
            while ( t.good() )
              while ( t.read((char*)&d, sizeof d) );
                  t_out.write((char*)&d, sizeof d);
          }
        }
      }
    }
    results += " @";
    fil.close();
    // convert new lines to spaces

    // give file name for all files
    return results;
}

static std::atomic<bool> hint_update_finished;
static void Hint_Update();
static void Map_Load_Error();

#define ASSERT_MAP_ERROR(X,Y) if ( !(X) ) { \
  AOD::Output(std::string(Y)); \
  Map_Load_Error(); \
  return; \
}

Map::Map(const char* map_name) {
  AOD::Output("Loading " + std::string(map_name));
  game_manager->State_Update(static_cast<Loading_State>(0));

  hint_update_finished = 0;
  std::thread hint_update(Hint_Update);
  
  std::string map_data (Load_Map_Data(map_name));
  ASSERT_MAP_ERROR(map_data != "", "Could not load header data");
  ASSERT_MAP_ERROR(map_data != "outdated", "Map version unknown -- probably ahead of client. "\
                                           "Update your client.");
  ASSERT_MAP_ERROR(map_data != "c_outdated",
                   "Map is outdated, please update the map\n");
  std::string fil_data (Load_Files(map_name));
  ASSERT_MAP_ERROR(fil_data != "", "Could not load file data");

  game_manager->State_Update(Loading_State::finalizing_loading);
  ///////////////////////////////////////////////////////
  ////////////// fil_data ///////////////////////////////
  int fil_it = 0;

  // get information & sheets (info, sheets, info, sheets ....)
  std::vector<std::string> sheets;
  while ( fil_data[fil_it] == ' ' ) ++ fil_it;
  while ( fil_data[fil_it] != '^' ) {
    // get "filename"
    while ( fil_data[fil_it] != '"' ) ++ fil_it;
    sheets.push_back("");
    while ( fil_data[++fil_it] != '"' )
      sheets[sheets.size()-1] += fil_data[fil_it];
    ++fil_it; // skip "
    while ( fil_data[fil_it] == ' ' ) ++fil_it; // skip to whitespace
  }
  ++fil_it;

  // get backgrounds
  std::vector<std::pair<std::string, Background>> bgs;
  while ( fil_data[fil_it] == ' ' ) ++fil_it; // skip to whitespace
  while ( fil_data[fil_it] != '&' ) {
    // get "filename"
    while ( fil_data[fil_it] != '"' ) ++ fil_it;
    bgs.push_back(std::pair<std::string, Background>(std::string(""),
                  Background()));
    while ( fil_data[++fil_it] != '"' )
      std::get<0>(bgs[bgs.size()-1]) += fil_data[fil_it];
    ++fil_it; // skip "
    while ( fil_data[fil_it] == ' ' ) ++ fil_it; // skip to whitespace
    --fil_it; // we do pre-inc in next loop
    std::string temp = "";
    // get all attributes for file name
    auto& bg_mod = bgs[bgs.size()-1].second;
    for ( int i = 0; i != 7; ++ i ) {
      while ( fil_data[++fil_it] != ' ' ) temp += fil_data[fil_it];
      
      switch ( i ) {
        case 0: bg_mod.vertical   = (bool) std::stoi(temp);  break;
        case 1: bg_mod.horizontal = (bool) std::stoi(temp);  break;
        case 2: bg_mod.repeats    = (bool) std::stoi(temp);  break;
        case 3: bg_mod.speed_x    = (float)std::stof(temp);  break;
        case 4: bg_mod.speed_y    = (float)std::stof(temp);  break;
        case 5: bg_mod.parralax   = (bool) std::stoi(temp);  break;
        case 6: bg_mod.layer      = (int)  std::stoi(temp);  break;
      }
      while ( fil_data[fil_it] == ' ' ) ++fil_it; // skip whitespace
    }
  }
  ++ fil_it; // skip ampersand
  while ( fil_data[fil_it] == ' ' ) ++fil_it; // skip whitespace

  std::vector<std::string> scripts;
  // get scripts
  while ( fil_data[fil_it] != '@' ) {
    while ( fil_data[fil_it] != '"' ) ++ fil_it;
    scripts.push_back("");
    while ( fil_data[++fil_it] != '"' )
      scripts[scripts.size()-1] += fil_data[fil_it];
    ++fil_it; // skip "
    while ( fil_data[fil_it] == ' ' ) ++fil_it; // skip to whitespace
  }

  // load sheets (info, sheet, info, sheet, .... )
  for ( int sheet_it = 0; sheet_it != sheets.size(); ++ sheet_it ) {
    tile_sheets.push_back(Tile_Sheet_Container());
    Tile_Sheet_Container& tsc = tile_sheets[tile_sheets.size()-1];
    ASSERT_MAP_ERROR(std::ifstream(sheets[sheet_it]),
                     "Could not open file " + sheets[sheet_it]);
    
    // ask main thread to create texture for us
    while ( !game_manager->Load_Image_Main_Thread(sheets[sheet_it]) );
    SheetContainer t;
    while ( !game_manager->Poll_Image_Loaded(t) );

    // store texture
    ASSERT_MAP_ERROR(t.texture != -1,
            "Could not load image from file " + sheets[sheet_it]);
    tile_sheets[tile_sheets.size()-1].sheet = t;
    // delete file
    remove(sheets[sheet_it].c_str());
  }

  // ----- load backgrounds ---------------------------------------------------
  for ( auto t_bg : bgs ) {
    /*// push back to main backgrond vec and load image
    backgrounds.push_back(std::get<1>(t_bg));
    ASSERT_MAP_ERROR(std::ifstream(std::get<0>(t_bg)),
                     "Could not open file " + std::get<0>(t_bg));
    backgrounds[backgrounds.size()-1].image =
      new SheetContainer(Load_Image(std::get<0>(t_bg).c_str()));
    ASSERT_MAP_ERROR(backgrounds[backgrounds.size()-1].image->texture != 0,
                     "Could not load image from file " + std::get<0>(t_bg));
    remove(std::get<0>(t_bg).c_str()); // delete file*/
  }

  // load scripts
  for ( auto s_nam : scripts ) {
//    Scripting::Compile_Script(s_nam);
    //remove(s_nam.c_str()); // delete it
  }

  ///////////////////////////////////////////////////////
  ////////////// map_data ///////////////////////////////
  map_name = "";
  int map_it = 0;
  while ( map_data[map_it] == ' ' ) ++ map_it; // skip whitespace
  while ( map_data[map_it] != '@' )
    map_name += map_data[map_it++];
  ++map_it; // skip @
  // map width -> height
  std::string temp = "";
  while ( map_data[map_it] == ' ' ) ++ map_it; // skip whitespace
  while ( map_data[map_it] != ' ' )
    temp += map_data[map_it++];
  map_tiles.resize(std::stoi(temp)/32);
  temp = "";
  while ( map_data[map_it] == ' ' ) ++ map_it; // skip whitespace
  while ( map_data[map_it] != ' ' )
    temp += map_data[map_it++];
  { // generate map tiles
    auto t_height = std::stoi(temp)/32;
    for ( int i = 0; i != map_tiles.size(); ++ i )
      map_tiles[i].resize(t_height);
  }
  // retrieve tile information
  int sheet_indices_amt = 0;
  game_manager->State_Update(Loading_State::loading_tiles);
  while ( map_data[map_it] != '*' ) {
    Tile_Base t_base;
    { // looong code
      // sheet
      temp = "";
      while ( map_data[map_it] == ' ' ) ++ map_it;
      while ( map_data[map_it] != ' ' )
        temp += map_data[map_it++];
      t_base.sheet_index = std::stoi(temp);
      temp = "";
      // tile
      temp = "";
      while ( map_data[map_it] == ' ' ) ++ map_it;
      while ( map_data[map_it] != ' ' )
        temp += map_data[map_it++];
      t_base.tile_index = std::stoi(temp);
      // set offsets
      {
        int t_ind = std::stoi(temp);
        if ( t_ind != 0 ) {
          t_base.sheet_off_w = tile_sheets[ t_base.sheet_index ].sheet.width/32;
          t_base.sheet_off_w = t_ind % t_base.sheet_off_w;
          t_base.sheet_off_w *= 32;
          
          t_base.sheet_off_h = tile_sheets[ t_base.sheet_index ].sheet.width/32;
          t_base.sheet_off_h = t_ind / t_base.sheet_off_h;
          t_base.sheet_off_h *= 32;
        } else {
          t_base.sheet_off_h =
          t_base.sheet_off_w = 0;
        }
      }
      temp = "";
      // x
      temp = "";
      while ( map_data[map_it] == ' ' ) ++ map_it;
      while ( map_data[map_it] != ' ' )
        temp += map_data[map_it++];
      t_base.Set_Position((float)std::stoi(temp), 0);
      // y
      temp = "";
      while ( map_data[map_it] == ' ' ) ++ map_it;
      while ( map_data[map_it] != ' ' )
        temp += map_data[map_it++];
      t_base.Set_Position(t_base.R_Position().x,
                                  (float)std::stoi(temp));
      // layer
      temp = "";
      while ( map_data[map_it] == ' ' ) ++ map_it;
      while ( map_data[map_it] != ' ' )
        temp += map_data[map_it++];
      t_base.layer = std::stoi(temp);
      // angle
      temp = "";
      while ( map_data[map_it] == ' ' ) ++ map_it;
      while ( map_data[map_it] != ' ' )
        temp += map_data[map_it++];
      t_base.Set_Rotation( std::stof(temp) );
      // flip x axis
      temp = "";
      while ( map_data[map_it] == ' ' ) ++ map_it;
      while ( map_data[map_it] != ' ' )
        temp += map_data[map_it++];
      t_base.flip_x = std::stoi(temp);
      // flip y axis
      temp = "";
      while ( map_data[map_it] == ' ' ) ++ map_it;
      while ( map_data[map_it] != ' ' )
        temp += map_data[map_it++];
      t_base.flip_y =  !(std::stoi(temp));
      // visible
      temp = "";
      while ( map_data[map_it] == ' ' ) ++ map_it;
      while ( map_data[map_it] != ' ' )
        temp += map_data[map_it++];
      t_base.visible = std::stoi(temp);
      // collideable
      temp = "";
      while ( map_data[map_it] == ' ' ) ++ map_it;
      while ( map_data[map_it] != ' ' )
        temp += map_data[map_it++];
      t_base.collideable = std::stoi(temp);
      // tags
      temp = "";
      while ( map_data[map_it] == ' ' ) ++ map_it;
    }
    const auto tsheet = tile_sheets[t_base.sheet_index].sheet;
    t_base.Set_Sprite(tsheet.texture);
    t_base.Set_UVs(
      AOD::Vector(    float(t_base.sheet_off_w)/float(tsheet.width),
                  1 - float(t_base.sheet_off_h)/float(tsheet.height)),
      AOD::Vector(    float(t_base.sheet_off_w + 32.f)/float(tsheet.width),
                  1 - float(t_base.sheet_off_h + 32.f)/float(tsheet.height)));
    t_base.Set_Size(32, 32);
    t_base.Set_Image_Size({32, 32});
    if ( t_base.flip_x )
      t_base.Flip_X();
    
    if ( t_base.flip_y )
      t_base.Flip_Y();

    // insert tile into map
    AOD::Vector pos = t_base.R_Position()/32;
    int i;
    // find first layer that is larger than this
    for ( i = 0; i != map_tiles[pos.x][pos.y].size(); ++ i )
      if ( map_tiles[pos.x][pos.y][i].layer > t_base.layer ) {
        map_tiles[pos.x][pos.y].insert(map_tiles[pos.x][pos.y].begin() + i,
                t_base); // insert before that ind
        break;
      }
    if ( i == map_tiles[pos.x][pos.y].size() ) // needs to push back
      map_tiles[pos.x][pos.y].push_back(t_base);
    if ( (t_base.sheet_index) + 1 > sheet_indices_amt )
      sheet_indices_amt = (t_base.sheet_index) + 1;
  }
  ++map_it;
  // spawners ( TODO )

  // ----- extract tile information
  Skip_WS(map_data, map_it);
  while ( map_data[map_it] == '*' ) ++map_it;
  Skip_WS(map_data, map_it);
  while ( map_data[map_it] == '*' ) ++map_it;
  // gen sheets
  Sheet* info_sheets = new Sheet[sheet_indices_amt];

  int it = 0;
  // read info
  while ( true  ) {
    info_sheets[it].width  = std::stoi(WS_Extract(map_data, map_it));
    info_sheets[it].height = std::stoi(WS_Extract(map_data, map_it));
    auto& polys = info_sheets[it].tile_polys;

    // read tile info
    while ( true ) {
      // check end (if not end then it's X)
      std::string end = WS_Extract(map_data, map_it);
      if ( end == ">>" ) break;
      int t = std::stoi(WS_Extract(map_data, map_it)); // grab Y
      const int X = std::stoi(end), Y = t;
      // store id
      std::pair<int, AOD::PolyObj> pair (info_sheets[it].width/32 * Y + X,
                                                          AOD::PolyObj());
      std::vector<int> ints;
      while ( map_data[map_it] == ' ' ) ++ map_it;
      while ( map_data[map_it] != '*' ) {
        // grab ints
        std::string _int = "";
        while ( map_data[map_it] != ' ' )
          _int.push_back(map_data[map_it++]);
        // store ints
        ints.push_back(std::stoi(_int) - 16); // 16 OFFSET for middle
        while ( map_data[map_it] == ' ' ) ++ map_it;
      }
      ++ map_it;

      Skip_WS(map_data, map_it);
      // make sense of ints vector
      std::vector < AOD::Vector > aodvecs;
      for ( int i = 0; i < ints.size()-1; i += 2 ) {
        aodvecs.push_back( {(float)ints[i], (float)ints[i+1]} );
      }
      // set tile
      pair.second.Set_Vertices(aodvecs);
      polys.push_back( pair );
    }
    // check if eof
    Skip_WS(map_data, map_it);
    if ( map_data[map_it] == '$' ) break;
    ++ it;
  }

  // Iterate through all tiles and assign polygon
  for ( int x = 0; x != R_Width(); ++ x ) {
    for ( int y = 0; y != R_Height(); ++ y )
    for ( int z = 0; z != R_Layer_Size(x, y); ++ z ) {
      Tile_Base* til = R_Tile(x, y, z);
      // assign tile info
      if ( til->collideable ) {
        auto& polys = info_sheets[til->sheet_index].tile_polys;
        til->Set_Vertices(polys[til->tile_index].second.R_Vertices(), 0);
        til->R_Transformed_Vertices( true );
      }
    }
  }
  delete[] info_sheets;
  // --------------------------------------------

  game_manager->State_Update(Loading_State::generating_spawners);
  ++map_it;
  // while ( map_data[map_it] == ' ' ) ++ map_it;
  /*while ( map_it+1 < map_data.size() ) {
    spawners.push_back(Spawner());
    Spawner& t_spawn = spawners[spawners.size()-1];
    temp = "";
    while ( map_data[map_it] == ' ' ) ++ map_it;
    while ( map_data[map_it] != ' ' )
      temp += map_data[map_it++];
    t_spawn.pos_x = String_To_Int(temp);
    temp = "";
    while ( map_data[map_it] == ' ' ) ++ map_it;
    while ( map_data[map_it] != ' ' )
      temp += map_data[map_it++];
    t_spawn.pos_y = String_To_Int(temp);
    temp = "";
    while ( map_data[map_it] == ' ' ) ++ map_it;
    while ( map_data[map_it] != ' ' )
      temp += map_data[map_it++];
    t_spawn.sp_type = Spawner::Type( String_To_Int(temp) );
    if ( t_spawn.sp_type == Spawner::Type::pl_left ||
         t_spawn.sp_type == Spawner::Type::pl_right )
      player_spawns.push_back(&t_spawn);
  }*/

  // wait for user to click, also tell hint update to die
  game_manager->State_Update(Loading_State::waiting_on_user);
  
  while ( AOD::Input::R_LMB() ); // if user already has mouse down
  while ( !AOD::Input::R_LMB() );
  hint_update_finished = 1;

  // wait until hint update dies
  hint_update.join();

  game_manager->State_Update(Loading_State::send_done_loading);
}

std::vector<Tile_Base*> Map::R_Tile_Vec(int x, int y, int w, int h) {
  std::vector<Tile_Base*> vec;
  if ( x+w < 0 || y+h < 0 ) return vec;

  int end_x = float(x+w)/32.f + 2.5f,
      end_y = float(y+h)/32.f + 2.5f;
  x -= 3; y -= 3;
  x /= 32; y /= 32;
  if ( x < 0 ) x = 0;
  if ( y < 0 ) y = 0;
  // grab all tiles
  for ( int i_x = x; i_x != end_x && i_x < map_tiles.size();    ++ i_x )
  for ( int i_y = y; i_y != end_y && i_y < map_tiles[0].size(); ++ i_y ) {
    for ( auto& i : map_tiles[i_x][i_y] )
      vec.push_back(&i);
  }
  return vec;
}

int Map::R_Height() {
  if ( map_tiles.size() > 0 )
    return map_tiles[0].size();
  return 0;
}

int Map::R_Width() {
  return map_tiles.size();
}

int Map::R_Layer_Size(int x, int y) {
  if ( map_tiles.size() > x && map_tiles[x].size() > y )
    return map_tiles[x][y].size();
  return 0;
}

Tile_Base * Map::R_Tile(int x, int y, int z) {
  if ( map_tiles.size() > x && map_tiles[x].size() > y &&
                            map_tiles[x][y].size() > z )
    return &map_tiles[x][y][z];
  return nullptr;
}

static void Hint_Update() {
  std::vector<AOD::Text*> text_refs;
  int hint = Util::R_Rand(0, loading_hints.size());

  SDL_Delay(1000);

  while ( true ) {
    // remove old
    for ( int i = 0; i != text_refs.size(); ++ i )
      AOD::Remove(text_refs[i]);
    text_refs.clear();

    // put break here so we can remove old updates
    if ( hint_update_finished ) break;

    // gen new & approx wait time
    int wait_time = 11000;
    for ( int i = 0; i != loading_hints[hint].size(); ++ i ) {
      auto t = new AOD::Text(CV::screen_width-400,
                    CV::screen_height-20 + (9*i), loading_hints[hint][i]);
      t->Set_Font("DejaVuSansMono.ttf", 8);
      t->Set_String(loading_hints[hint][i]);
      text_refs.push_back(t);
      AOD::Add(t);
    }
    wait_time += 1000*std::pow(loading_hints[hint].size(), 2);
    
    // wait & continue
    for ( int i = 0; i != wait_time; ++ i ) {
      SDL_Delay(1);
      if ( hint_update_finished ) break;
    }
    hint = Util::R_Rand(0, loading_hints.size());
  }
}

static void Map_Load_Error() {
  game_manager->State_Update(Loading_State::error);
  SDL_Delay(-1);
}

// ---- tile base -------------------------------------------------------------

Tile_Base::Tile_Base() : AOD::PolyObj() {
}
