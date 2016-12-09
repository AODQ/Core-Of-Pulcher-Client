/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#include "stdafx.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <map>
#include <fstream>
#include <sstream>

//#include <windows.h>

#include <gl/GLU.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "Camera.h"
#include "Object.h"
#include "Realm.h"
#include "Sounds.h"

// COP
#include "Client_Vars.h"
#include "Game_Manager.h"
#include "Map.h"

using AOD_Realm = AOD_Engine::Realm;


AOD_Realm::Realm(int window_width,int window_height,char* window_name,
								 char* icon) {
	{ // window/opengl stuff
		Debug_Output("Initializing SDL");
		SDL_Init(SDL_INIT_EVERYTHING);
		AOD_Engine::screen = SDL_CreateWindow(window_name,
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					window_width, window_height,
					SDL_WINDOW_OPENGL |
					SDL_WINDOW_SHOWN );
		if ( SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 ) == -1 )
			AOD::Output("Error CONTEXT_MAJOR: " +std::string(SDL_GetError()));
		if ( SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 ) == -1 )
			AOD::Output("Error CONTEXT_MINOR: " + std::string(SDL_GetError()));
		if ( SDL_GL_CreateContext( AOD_Engine::screen ) == nullptr ) {
			AOD::Output("Error window context: " + std::string(SDL_GetError()));
			std::stringstream st;
			st << AOD_Engine::screen;
			AOD::Output("screen value: " + st.str());
		}
		if ( SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1) == -1 )
			AOD::Output("Error DOUBLEBUFFER: " + std::string(SDL_GetError()));
		if ( SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8) == -1 )
			AOD::Output("Error ALPHA_SIZE: " + std::string(SDL_GetError()));
		//glShadeModel(GL_SMOOTH);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		if ( icon != "" ) {
			SDL_Surface* ico = SDL_LoadBMP((const char*)icon);
			SDL_SetWindowIcon(AOD_Engine::screen, ico);
		}

		glClearDepth(1.0f);
		glPolygonMode(GL_FRONT, GL_FILL);
		glShadeModel(GL_FLAT);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glMatrixMode(GL_PROJECTION);
		if ( !ilutRenderer(ILUT_OPENGL) )
			AOD::Output("Error setting ilut Renderer to ILUT_OPENGL");
		ilInit();
		iluInit();
		ilutInit();
		if ( !ilutRenderer(ILUT_OPENGL) )
			AOD::Output("Error setting ilut Renderer to ILUT_OPENGL");
		glEnable(GL_ALPHA);

		glLoadIdentity();
		
		glOrtho(0, window_width, window_height, 0, 0, 1);

		//glMatrixMode(GL_MODELVIEW);
		glDisable(GL_DEPTH_TEST);
		glMatrixMode(GL_MODELVIEW);
	}

	{ // others
		Debug_Output("Initializing Sounds Core");
		AOD_Engine::Sounds::Set_Up();
		Debug_Output("Initializing Font Core");
		AOD_Engine::Font::Init();
		objs_to_rem.clear();
		bg_red	 = 0;
		bg_blue	= 0;
		bg_green = 0;;
	}
}

AOD_Realm::~Realm() {
	Reset();
}

void AOD_Realm::Reset() {
	while ( AOD_Engine::images.size() > 0 )
		Util::Delete_Image(AOD_Engine::images[0]);

	for ( auto tz : objects )
		for ( auto i : tz )
			delete i;

	for ( auto tz : text )
		delete tz;

	for ( auto tz : fonts ) {
		FT_Done_Face(tz.second->R_Face());
		delete tz.second;
	}

	text.clear();
	fonts.clear();
	objects.clear();
	objs_to_rem.clear();

	AOD::fps_display = nullptr;
	FT_Done_FreeType(AOD_Engine::Font::FTLib);
}

void AOD_Realm::__Add(AOD::Object* a,int l) {
	a->layer = l;
	if ( l < 0 ) {
		l = 0;
		AOD_Engine::Debug_Output("Invalid layer is < 0, setting to 0");
	}
	while ( objects.size() <= l ) objects.push_back(std::vector<AOD::Object*>());
	objects[l].push_back(a);
}
void AOD_Realm::__Add(AOD::Text* t) {
	text.push_back(t);
}
void AOD_Realm::__Remove(AOD::Object* a) {
	objs_to_rem.push_back(a);
}
void AOD_Realm::__Remove(AOD::Text* t) {
	for ( int i = 0; i != text.size(); ++ i ) {
		if ( text[i] == t ) {
			text.erase(text.begin() + i,text.begin() + i + 1);
			return;
		}
	}
}

void AOD_Realm::Update() {
	// update objects
	for ( auto& l_i: objects )
	for ( auto& a_i : l_i )
		a_i->Update();
	AODP::Network::Update_NetObj_Sync();
	// remove objects
	for ( int rem_it = 0; rem_it != objs_to_rem.size(); ++ rem_it ) {
		int layer_it = objs_to_rem[rem_it]->layer;
		for ( int obj_it = 0; obj_it != objects[layer_it].size(); ++ obj_it ) {
			if ( objects[layer_it][obj_it] == objs_to_rem[rem_it] ) {
				delete objects[layer_it][obj_it];
				objects[layer_it].erase(objects[layer_it].begin() + obj_it);
				break;
			}
		}
	}
	objs_to_rem.clear();
}

void AOD_Realm::Render() {
	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
	glClearColor(bg_red,bg_green,bg_blue,0);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_TEXTURE_2D);

	float off_x = Camera::position.x - Camera::size.x/2,
				off_y = Camera::position.y - Camera::size.y/2;

	static GLubyte index[6] = { 0,1,2, 1,2,3 };
	// ---- TILES (core of pulcher only)
	if ( game_manager->curr_map ) {
		auto	c_pos_start = Camera::position/32.f -
							AOD::Vector((float)CV::screen_width /2.f-33.f,
													(float)CV::screen_height/2.f-33.f)/32.f,
					c_pos_end	 = Camera::position/32.f +
							AOD::Vector((float)CV::screen_width /2.f+33.f,
													(float)CV::screen_height/2.f+33.f)/32.f;
		--c_pos_start.x; // show tiles halfway on screen
		--c_pos_start.y;
		++c_pos_end.x;
		++c_pos_end.y;
		// limits
		if ( c_pos_start.x < 0 ) c_pos_start.x = 0;
		if ( c_pos_end.x	 < 0 ) c_pos_end.x	 = 0;
		if ( c_pos_start.y < 0 ) c_pos_start.y = 0;
		if ( c_pos_end.y	 < 0 ) c_pos_end.y	 = 0;

		if ( game_manager->curr_map != nullptr ) {
			int w = game_manager->curr_map->R_Width(),
					h = game_manager->curr_map->R_Height();

			if ( c_pos_start.x >= w ) c_pos_start.x = w-1;
			if ( c_pos_end.x	 >= w ) c_pos_end.x	 = w-1;
			if ( c_pos_start.y >= h ) c_pos_start.y = h-1;
			if ( c_pos_end.y	 >= h ) c_pos_end.y	 = h-1;

			for ( int x = c_pos_start.x; x != (int)c_pos_end.x; ++ x )
			for ( int y = c_pos_start.y; y != (int)c_pos_end.y; ++ y )
			for ( int z = 0; z != game_manager->curr_map->R_Layer_Size(x, y); ++ z ) {
				Tile_Base* t_tile = game_manager->curr_map->R_Tile(x, y, z);

				if ( t_tile->visible ) {
					glPushMatrix();
					glPushAttrib(GL_CURRENT_BIT);
						glBindTexture(GL_TEXTURE_2D, t_tile->R_Sprite_Texture());
						glTranslatef((int)t_tile->R_Position().x - (int)off_x,
												 (int)t_tile->R_Position().y - (int)off_y, 0);
						glRotatef(t_tile->R_Rotation()*180/std::_Pi, 0, 0, 1);
						glScalef (t_tile->R_Img_Size().x, t_tile->R_Img_Size().y, 0);
				
						glVertexPointer	(2, GL_FLOAT, 0, AOD::Object::Vertices);
						glTexCoordPointer(2, GL_FLOAT, 0, t_tile->_UV);
						glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, index);
						glLoadIdentity();
					glPopAttrib();
					glPopMatrix();
				}
			}
		}
	}

	// ---- OBJECTS

	for ( auto& az : objects )
	for ( auto lz : az ) {
		if ( !lz->R_Is_Visible() ) continue;
		auto position = lz->R_Position(),
				 size		 = lz->R_Size();
		if ( !lz->R_Is_Static_Pos() ) {
			position.x -= off_x;
			position.y -= off_y;
		}
		if ((position.x + size.x/2 < 0 ||
				 position.x - size.x/2 > Camera::size.x ) ||
				(position.y + size.y/2 < 0 ||
				 position.y - size.y/2 > Camera::size.y) )
			continue;

		glPushMatrix();
		glPushAttrib(GL_CURRENT_BIT);
			if ( lz->R_Is_Coloured() )
				glColor4f(lz->R_Red(),lz->R_Green(),lz->R_Blue(),lz->R_Alpha());
			glBindTexture(GL_TEXTURE_2D,lz->image);
			auto& origin = lz->R_Origin();
			int fx = lz->R_Flipped_X() ? -1 : 1,
					fy = lz->R_Flipped_Y() ?	1 :-1;
			glTranslatef(position.x + origin.x*fx,
									 position.y + origin.y*fy, 0);
			glRotatef(lz->rotation*180/std::_Pi, 0, 0, 1);
			glTranslatef(-origin.x*fx,
									 -origin.y*fy, 0);
			glScalef (lz->image_size.x, lz->image_size.y, 1);

			glVertexPointer	(2, GL_FLOAT, 0, AOD::Object::Vertices);
			glTexCoordPointer(2, GL_FLOAT, 0, lz->_UV);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, index);
			glLoadIdentity();
		glPopAttrib();
		glPopMatrix();
	}

	// ---- TEXTS

	for ( int i = 0; i < text.size(); ++ i ) {
		auto& tz = text[i];
		if ( !tz->R_Visible() ) continue;
		if ( !tz->R_FT_Font() ) {
			AOD_Engine::Debug_Output("Font face unitialized for "
																			+ tz->R_Font() + "\n");
			continue;
		}

		std::string t_str	= tz->R_Str();

		glPushMatrix();
			glTranslatef(tz->R_Position().x, tz->R_Position().y, 0);
			glListBase(tz->R_FT_Font()->R_Character_List());
			glCallLists( t_str.size(), GL_UNSIGNED_BYTE, t_str.c_str() );
		glPopMatrix();
	}

	// ---- CONSOLE
	if ( AOD_Engine::Console::console_type > AOD::Console::TYPE_NONE &&
			 AOD::Console::console_open ) {
		for ( auto tz : AOD_Engine::Console::console_text ) {
			std::string t_str = tz->R_Str();

			glPushMatrix();
				glTranslatef(tz->R_Position().x, tz->R_Position().y, 0);
				glListBase(tz->R_FT_Font()->R_Character_List());
				glCallLists( t_str.size(), GL_UNSIGNED_BYTE, t_str.c_str() );
			glPopMatrix();
		}
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_TEXTURE_2D);

	SDL_GL_SwapWindow(AOD_Engine::screen);
}

void AOD_Realm::Set_BG_Colours(GLfloat r, GLfloat g, GLfloat b) {
	bg_red = r;
	bg_green = g;
	bg_blue = g;
}
