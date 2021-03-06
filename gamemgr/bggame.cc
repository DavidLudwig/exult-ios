/*
 *  Copyright (C) 2000-2013  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <SDL.h>
#include <SDL_events.h>

#include <typeinfo>
#include <vector>
#include <utility>
#include <string>
#include "array_size.h"
#include "items.h"
#include "files/U7file.h"
#include "files/utils.h"
#include "flic/playfli.h"
#include "gamewin.h"
#include "Audio.h"
#include "bggame.h"
#include "sigame.h"
#include "palette.h"
#include "databuf.h"
#include "font.h"
#include "txtscroll.h"
#include "data/exult_bg_flx.h"
#include "exult.h"
#include "Configuration.h"
#include "shapeid.h"
#include "modmgr.h"
#include "miscinf.h"
#include "gump_utils.h"
#include "AudioMixer.h"
#include "mappatch.h"

#include "imagewin/imagewin.h"
#include "imagewin/ArbScaler.h"

#include <cctype>
#include <cstring>

using std::abs;
using std::rand;
using std::strchr;
using std::strlen;

#ifdef __IPHONEOS__
#include "iphone_gumps.h"
#endif


enum {
    ultima_text_shp = 0x0D,
    butterfly_shp = 0x0E,
    lord_british_shp = 0x11,
    trees_shp = 0x12,

    guardian_mouth_shp = 0x1E,
    guardian_forehead_shp = 0x1F,
    guardian_eyes_shp = 0x20
};

enum {
    bird_song_midi = 0,
    home_song_midi = 1,
    guardian_midi = 2,
    menu_midi = 3,
    credits_midi = 4,
    quotes_midi = 5
};

BG_Game::BG_Game()
	: shapes(ENDSHAPE_FLX) {
	if (!read_game_xml()) {
		add_shape("gumps/check", 2);
		add_shape("gumps/fileio", 3);
		add_shape("gumps/fntext", 4);
		add_shape("gumps/loadbtn", 5);
		add_shape("gumps/savebtn", 6);
		add_shape("gumps/halo", 7);
		add_shape("gumps/disk", 24);
		add_shape("gumps/heart", 25);
		add_shape("gumps/statatts", 28);
		add_shape("gumps/musicbtn", 29);
		add_shape("gumps/speechbtn", 30);
		add_shape("gumps/soundbtn", 31);
		add_shape("gumps/spellbook", 43);
		add_shape("gumps/statsdisplay", 47);
		add_shape("gumps/combat", 46);
		add_shape("gumps/quitbtn", 56);
		add_shape("gumps/yesnobox", 69);
		add_shape("gumps/yesbtn", 70);
		add_shape("gumps/nobtn", 71);
		add_shape("gumps/book", 32);
		add_shape("gumps/scroll", 55);
		add_shape("gumps/combatmode", 12);
		add_shape("gumps/slider", 14);
		add_shape("gumps/slider_diamond", 15);
		add_shape("gumps/slider_right", 16);
		add_shape("gumps/slider_left", 17);

		add_shape("gumps/box", 0);
		add_shape("gumps/crate", 1);
		add_shape("gumps/barrel", 8);
		add_shape("gumps/bag", 9);
		add_shape("gumps/backpack", 10);
		add_shape("gumps/basket", 11);
		add_shape("gumps/chest", 22);
		add_shape("gumps/shipshold", 26);
		add_shape("gumps/drawer", 27);
		add_shape("gumps/woodsign", 49);
		add_shape("gumps/tombstone", 50);
		add_shape("gumps/goldsign", 51);
		add_shape("gumps/body", 53);

		add_shape("gumps/scroll_spells", 0xfff);
		add_shape("gumps/spell_scroll", 0xfff);
		add_shape("gumps/jawbone", 0xfff);
		add_shape("gumps/tooth", 0xfff);

		add_shape("sprites/map", 22);
		add_shape("sprites/cheatmap", EXULT_BG_FLX_BGMAP_SHP);

		const char *exultflx = BUNDLE_CHECK(BUNDLE_EXULT_FLX, EXULT_FLX);
		const char *gameflx = BUNDLE_CHECK(BUNDLE_EXULT_BG_FLX, EXULT_BG_FLX);

		add_resource("files/shapes/count", 0, 9);
		add_resource("files/shapes/0", SHAPES_VGA, 0);
		add_resource("files/shapes/1", FACES_VGA, 0);
		add_resource("files/shapes/2", GUMPS_VGA, 0);
		add_resource("files/shapes/3", SPRITES_VGA, 0);
		add_resource("files/shapes/4", MAINSHP_FLX, 0);
		add_resource("files/shapes/5", ENDSHAPE_FLX, 0);
		add_resource("files/shapes/6", FONTS_VGA, 0);
		add_resource("files/shapes/7", exultflx, 0);
		add_resource("files/shapes/8", gameflx, 0);

		add_resource("files/gameflx", gameflx, 0);

		add_resource("files/paperdolvga", gameflx, EXULT_BG_FLX_BG_PAPERDOL_VGA);
		add_resource("files/mrfacesvga", gameflx, EXULT_BG_FLX_BG_MR_FACES_VGA);
		add_resource("config/defaultkeys", gameflx, EXULT_BG_FLX_DEFAULTKEYS_TXT);
		add_resource("config/bodies", gameflx, EXULT_BG_FLX_BODIES_TXT);
		add_resource("config/paperdol_info", gameflx, EXULT_BG_FLX_PAPERDOL_INFO_TXT);
		add_resource("config/shape_info", gameflx, EXULT_BG_FLX_SHAPE_INFO_TXT);
		add_resource("config/shape_files", gameflx, EXULT_BG_FLX_SHAPE_FILES_TXT);
		add_resource("config/avatar_data", gameflx, EXULT_BG_FLX_AVATAR_DATA_TXT);
		add_resource("config/autonotes", gameflx, EXULT_BG_FLX_AUTONOTES_TXT);

		add_resource("palettes/count", 0, 18);
		add_resource("palettes/0", PALETTES_FLX, 0);
		add_resource("palettes/1", PALETTES_FLX, 1);
		add_resource("palettes/2", PALETTES_FLX, 2);
		add_resource("palettes/3", PALETTES_FLX, 3);
		add_resource("palettes/4", PALETTES_FLX, 4);
		add_resource("palettes/5", PALETTES_FLX, 5);
		add_resource("palettes/6", PALETTES_FLX, 6);
		add_resource("palettes/7", PALETTES_FLX, 7);
		add_resource("palettes/8", PALETTES_FLX, 8);
		add_resource("palettes/9", PALETTES_FLX, 10);
		add_resource("palettes/10", PALETTES_FLX, 11);
		add_resource("palettes/11", PALETTES_FLX, 12);
		add_resource("palettes/12", INTROPAL_DAT, 0);
		add_resource("palettes/13", INTROPAL_DAT, 1);
		add_resource("palettes/14", INTROPAL_DAT, 2);
		add_resource("palettes/15", INTROPAL_DAT, 3);
		add_resource("palettes/16", INTROPAL_DAT, 4);
		add_resource("palettes/17", INTROPAL_DAT, 5);

		add_resource("palettes/patch/0", PATCH_PALETTES, 0);
		add_resource("palettes/patch/1", PATCH_PALETTES, 1);
		add_resource("palettes/patch/2", PATCH_PALETTES, 2);
		add_resource("palettes/patch/3", PATCH_PALETTES, 3);
		add_resource("palettes/patch/4", PATCH_PALETTES, 4);
		add_resource("palettes/patch/5", PATCH_PALETTES, 5);
		add_resource("palettes/patch/6", PATCH_PALETTES, 6);
		add_resource("palettes/patch/7", PATCH_PALETTES, 7);
		add_resource("palettes/patch/8", PATCH_PALETTES, 8);
		add_resource("palettes/patch/9", PATCH_PALETTES, 10);
		add_resource("palettes/patch/10", PATCH_PALETTES, 11);
		add_resource("palettes/patch/11", PATCH_PALETTES, 12);
		add_resource("palettes/patch/12", PATCH_INTROPAL, 0);
		add_resource("palettes/patch/13", PATCH_INTROPAL, 1);
		add_resource("palettes/patch/14", PATCH_INTROPAL, 2);
		add_resource("palettes/patch/15", PATCH_INTROPAL, 3);
		add_resource("palettes/patch/16", PATCH_INTROPAL, 4);
		add_resource("palettes/patch/17", PATCH_INTROPAL, 5);

		add_resource("xforms/count", 0, 20);
		add_resource("xforms/0", XFORMTBL, 0);
		add_resource("xforms/1", XFORMTBL, 1);
		add_resource("xforms/2", XFORMTBL, 2);
		add_resource("xforms/3", XFORMTBL, 3);
		add_resource("xforms/4", XFORMTBL, 4);
		add_resource("xforms/5", XFORMTBL, 5);
		add_resource("xforms/6", XFORMTBL, 6);
		add_resource("xforms/7", XFORMTBL, 7);
		add_resource("xforms/8", XFORMTBL, 8);
		add_resource("xforms/9", XFORMTBL, 9);
		add_resource("xforms/10", XFORMTBL, 10);
		add_resource("xforms/11", XFORMTBL, 11);
		add_resource("xforms/12", XFORMTBL, 12);
		add_resource("xforms/13", XFORMTBL, 13);
		add_resource("xforms/14", XFORMTBL, 14);
		add_resource("xforms/15", XFORMTBL, 15);
		add_resource("xforms/16", XFORMTBL, 16);
		add_resource("xforms/17", XFORMTBL, 17);
		add_resource("xforms/18", XFORMTBL, 18);
		add_resource("xforms/19", XFORMTBL, 19);
	}
	fontManager.add_font("MENU_FONT", MAINSHP_FLX, PATCH_MAINSHP, 9, 1);
	fontManager.add_font("END2_FONT", ENDGAME, PATCH_ENDGAME, 4, -1);
	fontManager.add_font("END3_FONT", ENDGAME, PATCH_ENDGAME, 5, -2);
	fontManager.add_font("NORMAL_FONT", FONTS_VGA, PATCH_FONTS, 0, -1);
	fontManager.add_font("SMALL_BLACK_FONT", FONTS_VGA, PATCH_FONTS, 2, 0);
	fontManager.add_font("TINY_BLACK_FONT", FONTS_VGA, PATCH_FONTS, 4, 0);
	fontManager.add_font("GUARDIAN_FONT", MAINSHP_FLX, PATCH_MAINSHP, 3, -2);
	Map_patch_collection *mp = gwin->get_map_patches();
			// Sawdust in Iolo's hut is at lift 2, should be 0
			// FIXME - the original had some way to deal with this
			mp->add(new Map_patch_modify(Object_spec(
			                                 Tile_coord(481, 599, 2), 224, 7, 0),
			                             Object_spec(
			                                 Tile_coord(480, 598, 0), 224, 7, 0)));
			mp->add(new Map_patch_modify(Object_spec(
			                                 Tile_coord(482, 601, 2), 224, 7, 0),
			                             Object_spec(
			                                 Tile_coord(481, 600, 0), 224, 7, 0)));
			mp->add(new Map_patch_modify(Object_spec(
			                                 Tile_coord(482, 600, 2), 224, 7, 0),
			                             Object_spec(
			                                 Tile_coord(481, 599, 0), 224, 7, 0)));
			mp->add(new Map_patch_modify(Object_spec(
			                                 Tile_coord(481, 602, 2), 224, 7, 0),
			                             Object_spec(
			                                 Tile_coord(480, 601, 0), 224, 7, 0)));
			mp->add(new Map_patch_modify(Object_spec(
			                                 Tile_coord(479, 599, 2), 224, 7, 0),
			                             Object_spec(
			                                 Tile_coord(478, 598, 0), 224, 7, 0)));
			mp->add(new Map_patch_modify(Object_spec(
			                                 Tile_coord(477, 597, 2), 224, 7, 0),
			                             Object_spec(
			                                 Tile_coord(476, 596, 0), 224, 7, 0)));
			mp->add(new Map_patch_modify(Object_spec(
			                                 Tile_coord(476, 597, 2), 224, 7, 0),
			                             Object_spec(
			                                 Tile_coord(475, 596, 0), 224, 7, 0)));
			mp->add(new Map_patch_modify(Object_spec(
			                                 Tile_coord(472, 595, 2), 224, 7, 0),
			                             Object_spec(
			                                 Tile_coord(471, 594, 0), 224, 7, 0)));
			mp->add(new Map_patch_modify(Object_spec(
			                                 Tile_coord(473, 598, 2), 224, 7, 0),
			                             Object_spec(
			                                 Tile_coord(472, 597, 0), 224, 7, 0)));
			mp->add(new Map_patch_modify(Object_spec(
			                                 Tile_coord(472, 600, 2), 224, 7, 0),
			                             Object_spec(
			                                 Tile_coord(471, 599, 0), 224, 7, 0)));
			mp->add(new Map_patch_modify(Object_spec(
			                                 Tile_coord(470, 597, 2), 224, 7, 0),
			                             Object_spec(
			                                 Tile_coord(469, 596, 0), 224, 7, 0)));
			mp->add(new Map_patch_modify(Object_spec(
			                                 Tile_coord(469, 597, 2), 224, 7, 0),
			                             Object_spec(
			                                 Tile_coord(468, 596, 0), 224, 7, 0)));
			mp->add(new Map_patch_modify(Object_spec(
			                                 Tile_coord(467, 599, 2), 224, 7, 0),
			                             Object_spec(
			                                 Tile_coord(466, 598, 0), 224, 7, 0)));
			mp->add(new Map_patch_modify(Object_spec(
			                                 Tile_coord(468, 600, 2), 224, 7, 0),
			                             Object_spec(
			                                 Tile_coord(467, 599, 0), 224, 7, 0)));
			mp->add(new Map_patch_modify(Object_spec(
			                                 Tile_coord(467, 601, 2), 224, 7, 0),
			                             Object_spec(
			                                 Tile_coord(466, 600, 0), 224, 7, 0)));
			// pure BG shape 874 only has an empty frame. For FoV's Test of Truth
			// it was changed for creating a dungeon chasm while obviously forgetting
			// that frame #0 was used in two occassions:
			//       hole in Despise's rebel base to invisible stairway
			mp->add(new Map_patch_remove(Object_spec(
		                                     Tile_coord(681, 1192, 5), 874, 0, 0)));
			mp->add(new Map_patch_remove(Object_spec(
		                                     Tile_coord(682, 1192, 5), 874, 0, 0)));
			mp->add(new Map_patch_remove(Object_spec(
		                                     Tile_coord(681, 1195, 5), 874, 0, 0)));
			mp->add(new Map_patch_remove(Object_spec(
		                                     Tile_coord(682, 1195, 5), 874, 0, 0)));
			//       stairways hole in Shame's 2nd floor
			mp->add(new Map_patch_remove(Object_spec(
		                                     Tile_coord(807, 951, 5), 874, 0, 0)));
			mp->add(new Map_patch_remove(Object_spec(
		                                     Tile_coord(811, 951, 5), 874, 0, 0)));
			mp->add(new Map_patch_remove(Object_spec(
		                                     Tile_coord(807, 955, 5), 874, 0, 0)));
			mp->add(new Map_patch_remove(Object_spec(
		                                     Tile_coord(811, 955, 5), 874, 0, 0)));
}

BG_Game::~BG_Game() {
}

class UserSkipException : public UserBreakException {
};


#define WAITDELAY(x) switch(wait_delay(x)) { \
	case 1: throw UserBreakException(); break; \
	case 2: throw UserSkipException(); break; \
	}

#define WAITDELAYCYCLE1(x) switch (wait_delay((x), 16, 78, 50)) { \
	case 1: throw UserBreakException(); break; \
	case 2: throw UserSkipException(); break; \
	}

#define WAITDELAYCYCLE2(x) switch (wait_delay((x), 250, 5)) { \
	case 1: throw UserBreakException(); break; \
	case 2: throw UserSkipException(); break; \
	}

#define WAITDELAYCYCLE3(x) switch (wait_delay((x), 240, 15)) { \
	case 1: throw UserBreakException(); break; \
	case 2: throw UserSkipException(); break; \
	}

#define WAITDELAYCYCLE4(x) switch (wait_delay((x), 16, 78, 15)) { \
	case 1: throw UserBreakException(); break; \
	case 2: throw UserSkipException(); break; \
	}

#define WAITDELAYCYCLE5(x) switch (wait_delay((x), 16, -78, 50)) { \
	case 1: throw UserBreakException(); break; \
	case 2: throw UserSkipException(); break; \
	}

#define WAITDELAYCYCLE6(x) switch (wait_delay((x), 16, -78, 15)) { \
	case 1: throw UserBreakException(); break; \
	case 2: throw UserSkipException(); break; \
	}

void BG_Game::play_intro() {
	Audio *audio = Audio::get_ptr();
	audio->stop_music();
	MyMidiPlayer *midi = audio->get_midi();
	if (midi) midi->set_timbre_lib(MyMidiPlayer::TIMBRE_LIB_INTRO);

	gwin->clear_screen(true);

	// TODO: check/fix other resolutions

	// these backups store the area under the guardian shapes being drawn
	// the cbackups are 'clean' backups, ie. just background
	// the others may have other shapes on them, if those are static
	Image_buffer *backup, *backup2, *backup3;
	Image_buffer *cbackup, *cbackup2, *cbackup3;

	backup = backup2 = backup3 = 0;
	cbackup = cbackup2 = cbackup3 = 0;

	try {
		/********************************************************************
		 Lord British Presents
		********************************************************************/

		scene_lord_british();

		/********************************************************************
		 Ultima VII logo w/Trees
		********************************************************************/

		scene_butterfly();

		/********************************************************************
		 Enter guardian
		********************************************************************/

		scene_guardian();

		/********************************************************************
		 PC screen
		********************************************************************/

		scene_desk();

		/********************************************************************
		 The Moongate
		********************************************************************/

		scene_moongate();
	} catch (const UserBreakException &/*x*/) {
		// Waste disposal
		FORGET_OBJECT(backup);
		FORGET_OBJECT(backup2);
		FORGET_OBJECT(backup3);
		FORGET_OBJECT(cbackup);
		FORGET_OBJECT(cbackup2);
		FORGET_OBJECT(cbackup3);
	}

	gl_clear_win();
	enable_direct_gl_render();
	// Fade out the palette...
	pal->fade_out(c_fade_out_time);

	// ... and clean the screen.
	gwin->clear_screen(true);

	// Stop all audio output
	audio->cancel_streams();
}

void BG_Game::scene_lord_british() {
	/*
	 *  Enhancements to lip-syncing contributed by
	 *  Eric Wasylishen, Jun. 19, 2006.
	 */

	WAITDELAY(1500); // - give a little space between exult title music
	//     and LB presents screen


	// Lord British presents...  (sh. 0x11)
	pal->load(INTROPAL_DAT, PATCH_INTROPAL, 3);
	sman->paint_shape(topx, topy, shapes.get_shape(lord_british_shp, 0));

	pal->fade_in(c_fade_in_time);
	if (1 == wait_delay(2000))
		throw UserBreakException();
	pal->fade_out(c_fade_out_time);
	gwin->clear_screen(true);
}


#define BUTTERFLY_FRAME_DURATION    23 // - used to be 16.. too fast.

#define BUTTERFLY_SUB_FRAMES    3

#ifdef HAVE_OPENGL
#define BUTTERFLY_BACKGROUND()  do {    \
		if (GL_manager::get_instance()) {   \
			sman->paint_shape(topx,topy,shapes.get_shape(trees_shp,0)); \
			sman->paint_shape(topx+160,topy+50,shapes.get_shape(ultima_text_shp,0));    \
		}   \
	} while(0)
#else
#define BUTTERFLY_BACKGROUND()  do {    } while(0)
#endif


#define BUTTERFLY(x,y,frame,delay)  do {    \
		BUTTERFLY_BACKGROUND(); \
		win->get(backup, topx + (x) - butterfly->get_xleft(),   \
		         topy + (y) - butterfly->get_yabove());  \
		sman->paint_shape(topx + (x), topy + (y), shapes.get_shape(butterfly_shp, frame));  \
		win->show();    \
		WAITDELAY(delay);   \
		win->put(backup, topx + (x) - butterfly->get_xleft(),   \
		         topy + (y) - butterfly->get_yabove());  \
	} while(0)

#define BUTTERFLY_FLAP()    do {    \
		if ((rand() % 5)<4) {   \
			if (frame == 3) \
				dir = -1;   \
			else if (frame == 0)    \
				dir = +1;   \
			frame += dir;   \
		} } while(0)

static int butterfly_x[] = {
	6, 18, 30, 41, 52, 62, 70, 78, 86, 95,
	104, 113, 122, 132, 139, 146, 151, 155, 157, 158,
	157, 155, 151, 146, 139, 132, 124, 116, 108, 102,
	96, 93, 93, 93, 95, 99, 109, 111, 118, 125,
	132, 140, 148, 157, 164, 171, 178, 184, 190, 196,
	203, 211, 219, 228, 237, 246, 254, 259, 262, 264,
	265, 265, 263, 260, 256, 251, 245, 239, 232, 226,
	219, 212, 208, 206, 206, 209, 212, 216, 220, 224,
	227, 234, 231, 232, 233, 233, 233, 233, 234, 236,
	239, 243, 247, 250, 258, 265
};

static int butterfly_y[] = {
	155, 153, 151, 150, 149, 148, 148, 148, 148, 149,
	150, 150, 150, 149, 147, 142, 137, 131, 125, 118,
	110, 103, 98, 94, 92, 91, 91, 91, 92, 95,
	99, 104, 110, 117, 123, 127, 131, 134, 135, 135,
	135, 135, 135, 134, 132, 129, 127, 123, 119, 115,
	112, 109, 104, 102, 101, 102, 109, 109, 114, 119,
	125, 131, 138, 144, 149, 152, 156, 158, 159, 159,
	158, 155, 150, 144, 137, 130, 124, 118, 112, 105,
	99, 93, 86, 80, 73, 66, 59, 53, 47, 42,
	38, 35, 32, 29, 26, 25
};

static const int butterfly_num_coords = sizeof(butterfly_x) / sizeof(int);

static int butterfly_end_frames[] = { 3, 4, 3, 4, 3, 2, 1, 0 };
static int butterfly_end_delay[] = { 167, 416, 250, 416, 416, 416, 416, 333 };


void BG_Game::scene_butterfly() {
	Image_buffer *backup = 0;
	Shape_frame *butterfly = 0;
	int i, j, frame, dir = 0;

	try {
		pal->load(INTROPAL_DAT, PATCH_INTROPAL, 4);

		// Load the butterfly shape
		butterfly = shapes.get_shape(butterfly_shp, 0);
		backup = win->create_buffer(butterfly->get_width(), butterfly->get_height());

		// Start playing the birdsongs while still faded out
		Audio::get_ptr()->start_music(bird_song_midi, false, INTROMUS);

		// trees with "Ultima VII" on top of 'em
		sman->paint_shape(topx, topy, shapes.get_shape(trees_shp, 0));
		sman->paint_shape(topx + 160, topy + 50, shapes.get_shape(ultima_text_shp, 0));


		// Keep it dark for some more time, playing the music
		WAITDELAY(4500); //  - was WAITDELAY(3500);

		// Finally fade in
		pal->fade_in(c_fade_in_time);


		WAITDELAY(4000);

		win->show();

		WAITDELAY(7100);

		//
		// Move the butterfly along its path
		//
		frame = 0;
		Sint32 delay = BUTTERFLY_FRAME_DURATION;
		Sint32 ticks = SDL_GetTicks();
		for (i = 0; i < butterfly_num_coords - 1; ++i) {
			for (j = 0; j < BUTTERFLY_SUB_FRAMES; ++j) {

				ticks = SDL_GetTicks();
				int x = butterfly_x[i] + j * (butterfly_x[i + 1] - butterfly_x[i]) / BUTTERFLY_SUB_FRAMES;
				int y = butterfly_y[i] + j * (butterfly_y[i + 1] - butterfly_y[i]) / BUTTERFLY_SUB_FRAMES;
				BUTTERFLY(x, y, frame, delay);

				// Flap the wings; but not always, so that the butterfly "glides" from time to time
				BUTTERFLY_FLAP();

				// Calculate the difference between the time we wanted to spent and the time
				// we actually spent; then adjust 'delay' accordingly
				ticks = SDL_GetTicks() - ticks;
				delay = (delay + (2 * BUTTERFLY_FRAME_DURATION - ticks)) / 2;

				// ... but maybe we also have to skip frames..
				if (delay < 0) {
					// Calculate how many frames we should skip
					int frames_to_skip = (-delay) / BUTTERFLY_FRAME_DURATION + 1;
					int new_index = i * BUTTERFLY_SUB_FRAMES + j + frames_to_skip;
					i = new_index / BUTTERFLY_SUB_FRAMES;
					j = new_index % BUTTERFLY_SUB_FRAMES;

					// Did we skip over the end?
					if (i >= butterfly_num_coords - 1)
						break;

					while (frames_to_skip--)
						BUTTERFLY_FLAP();

					delay = 0;
				}
			}
		}

		// Finally, let it flutter a bit on the end spot
		for (i = 0; i < 8; i++) {
			BUTTERFLY(butterfly_x[butterfly_num_coords - 1],
			          butterfly_y[butterfly_num_coords - 1],
			          butterfly_end_frames[i],
			          butterfly_end_delay[i]);
		}

		WAITDELAY(1000);

		// Wait till the music finished playing
		while (Audio::get_ptr()->is_track_playing(bird_song_midi))
			WAITDELAY(20);
		FORGET_OBJECT(backup);
	} catch (const UserSkipException &/*x*/) {
		// Waste disposal
		FORGET_OBJECT(backup);
	} catch (const UserBreakException &/*x*/) {
		// Waste disposal
		FORGET_OBJECT(backup);
		throw;
	}
}

#define FLASH_SHAPE1(x,y,shape,frame,delay)  do {    \
		sman->paint_shape((x),(y),shapes.get_shape((shape),(frame)));   \
		non_gl_blit();  \
		WAITDELAYCYCLE1((delay));    \
		win->put(backup,(x)-s->get_xleft(),(y)-s->get_yabove());    \
	} while(0)

#define FLASH_SHAPE2(x,y,shape,frame,delay)  do {    \
		sman->paint_shape((x),(y),shapes.get_shape((shape),(frame)));   \
		non_gl_blit();  \
		WAITDELAYCYCLE4((delay));    \
		win->put(backup,(x)-s->get_xleft(),(y)-s->get_yabove());    \
	} while(0)

#define EYES_DIST       12
#define FORHEAD_DIST    49

static int text_times[] = {
	250, 1700, 5250, 8200, 10550, 12900, 16000, 19950, 22700, 27200, 32400,
		36400, 39650, 42400
};

static const int text_num_frames = sizeof(text_times) / sizeof(int);

static const char surfacing_data[] = {
	0x03, 0x00, 0x02,	// Eyebrows -> neutral
	0x03, 0x00, 0x05,	// Eyes -> half-open
	0x15, 0x00, 0x04,	// Eyes -> fully open
	0x33, 0x00, 0x07,	// Eyes -> Look down
	0x54, 0x00, 0x04,	// Eyes -> fully open
	0x77, 0x00, 0x00
};

// Lookup table for translating lipsynch data. The table is indexed by the
// current frame-1, then by read lipsynch data-1.
static const int eye_frame_LUT[6][9] = {
//   1, 2, 3, 4, 5, 6, 7, 8, 9		// Last eye frame (or the one before, if it was 10)
	{1, 2, 3, 1, 2, 3, 1, 2, 3},	// 1: Change eyebrows to angry
	{4, 5, 6, 4, 5, 6, 4, 5, 6},	// 2: Change eyebrows to neutral
	{7, 8, 9, 7, 8, 9, 7, 8, 9},	// 3: Change eyebrows to raised
	{1, 1, 1, 4, 4, 4, 7, 7, 7},	// 4: Change eyes to fully open
	{2, 2, 2, 5, 5, 5, 8, 8, 8},	// 5: Change eyes to half-open
	{3, 3, 3, 6, 6, 6, 9, 9, 9},	// 6: Change eyes to fully closed
};

class LipSynchReader {
	char * data;
	const unsigned char *ptr;
	const unsigned char *eptr;
public:
	LipSynchReader() {
		U7multiobject lipdata(MAINSHP_FLX, PATCH_MAINSHP, 0x0F);
		size_t len;
		data = lipdata.retrieve(len);
		ptr = reinterpret_cast<unsigned char *>(data);
		eptr = ptr + len;
	}
	LipSynchReader(char const *pp, int len) {
		data = new char[len];
		memcpy(data, pp, len);
		ptr = reinterpret_cast<unsigned char *>(data);
		eptr = ptr + len;
	}
	~LipSynchReader() {
		delete [] data;
	}
	bool have_events() const {
		return ptr < eptr;
	}
	void get_event(int &time, int &code) {
		if (ptr >= eptr) {
			time = 0;
			code = 0;
		} else {
			int curr_time = Read2(ptr);
			time = curr_time * 1000.0 / 60.0;
			code = Read1(ptr);
		}
	}
	void translate_code(int code, int &mouth, int &eyes, int &lasteyes) {
		// Set based on code read
		if (code > 8) {
			// This changes mouth frame
			mouth = code - 8;
		} else if (code == 7) {
			// This sets eyes to looking down
			// Don't update last eye frame
			eyes = 10;
		} else if (code > 0 && code < 7) {
			// This changes eyes and eyebrows
			eyes = eye_frame_LUT[code-1][lasteyes-1];
			lasteyes = eyes;
		}
	}
};

void BG_Game::scene_guardian() {
	Image_buffer *backup = 0, *backup2 = 0, *backup3 = 0;
	Image_buffer *cbackup = 0, *cbackup2 = 0, *cbackup3 = 0;
	Image_buffer *plasma;
	char *txt = 0;

	try {
		char *txt_ptr, *txt_end, *next_txt;
		Shape_frame *s, *s2, *s3;
		Uint32 ticks;
		int i;

		// create buffer containing a blue 'plasma' screen
		plasma = win->create_buffer(win->get_full_width(),
		                            win->get_full_height());
		gwin->plasma(win->get_full_width(), win->get_full_height(), win->get_start_x(), win->get_start_y(), 16, 16 + 76);
		win->get(plasma, win->get_start_x(), win->get_start_y());

		pal->load(INTROPAL_DAT, PATCH_INTROPAL, 2);
		pal->set_color(1, 0, 0, 0); //UGLY hack... set font background to black
		pal->apply();

		//play static SFX
		Audio::get_ptr()->play_sound_effect(115, AUDIO_MAX_VOLUME, 0, 0);

		//
		// Show some "static" alternating with the blue plasma
		//
		// TODO: Is this the right kind of static? Dosbox shows a mostly black
		//      with an ocassional white pixel static - but is this what it
		//      was really like?

		// TODO: have the static sound effect play for every time there's static in the intro
		ticks = SDL_GetTicks();
		while (1) {
			win->get_ibuf()->fill_static(0, 7, 15);
#ifdef HAVE_OPENGL
			if (GL_manager::get_instance())
				Delay();
			else
#endif
				win->show();
			WAITDELAYCYCLE1(2);
#ifdef HAVE_OPENGL
			if (GL_manager::get_instance())
				Delay();
#endif
			if (SDL_GetTicks() > ticks + 400)//400)
				break;
		}

		win->put(plasma, win->get_start_x(), win->get_start_y());
		non_gl_blit();
		WAITDELAYCYCLE1(200);

		ticks = SDL_GetTicks();
		while (1) {
			win->get_ibuf()->fill_static(0, 7, 15);
#ifdef HAVE_OPENGL
			if (GL_manager::get_instance())
				Delay();
			else
#endif
				win->show();
			WAITDELAYCYCLE1(2);
#ifdef HAVE_OPENGL
			if (GL_manager::get_instance())
				Delay();
#endif
			if (SDL_GetTicks() > ticks + 200)
				break;
		}

		win->put(plasma, win->get_start_x(), win->get_start_y());
		non_gl_blit();
		WAITDELAYCYCLE1(200);

		ticks = SDL_GetTicks();
		while (1) {
			win->get_ibuf()->fill_static(0, 7, 15);
#ifdef HAVE_OPENGL
			if (GL_manager::get_instance())
				Delay();
			else
#endif
				win->show();
			WAITDELAYCYCLE1(2);
#ifdef HAVE_OPENGL
			if (GL_manager::get_instance())
				Delay();
#endif
			if (SDL_GetTicks() > ticks + 100)
				break;
		}

		win->put(plasma, win->get_start_x(), win->get_start_y());
		non_gl_blit();
		FORGET_OBJECT(plasma);

		//
		// Start background music
		//
		Audio::get_ptr()->start_music(guardian_midi, false, INTROMUS);

		WAITDELAYCYCLE1(3800);

		//
		// First 'popup' (sh. 0x21)
		//

		// TODO: Play the 'bloop' sound for each popup
		s = shapes.get_shape(0x21, 0);
		backup = win->create_buffer(s->get_width(), s->get_height());
		win->get(backup, centerx - 53 - s->get_xleft(), centery - 68 - s->get_yabove());
		disable_direct_gl_render();
		for (i = 8; i >= -8; i--)
			FLASH_SHAPE2(centerx - 53, centery - 68, 0x21, 1 + abs(i), 80);
		FORGET_OBJECT(backup);
		WAITDELAYCYCLE1(2000);


		//
		// Second 'popup' (sh. 0x22)
		//
		s = shapes.get_shape(0x22, 0);
		backup = win->create_buffer(s->get_width(), s->get_height());
		win->get(backup, centerx - s->get_xleft(), centery - 45 - s->get_yabove());
		for (i = 9; i >= -9; i--)
			FLASH_SHAPE2(centerx, centery - 45, 0x22, 9 - abs(i), 80);
		FORGET_OBJECT(backup);
		WAITDELAYCYCLE1(2000);


		//
		// Successful 'popup' (sh. 0x23)
		//
		s = shapes.get_shape(0x23, 0);
		backup = win->create_buffer(s->get_width(), s->get_height());
		cbackup = win->create_buffer(s->get_width(), s->get_height());

		win->get(cbackup, centerx - s->get_xleft(), centery - s->get_yabove());
		sman->paint_shape(centerx, centery, s); // frame 0 is static background
		win->get(backup, centerx - s->get_xleft(), centery - s->get_yabove());
		for (i = 1; i < 16; i++)
			FLASH_SHAPE2(centerx, centery, 0x23, i, 70);

		sman->paint_shape(centerx, centery, shapes.get_shape(0x23, 15));
		non_gl_blit();

		WAITDELAYCYCLE1(500);    // - show his face for half a second
		// before he opens his eyes.

		win->put(cbackup, centerx - s->get_xleft(), centery - s->get_yabove());

		FORGET_OBJECT(backup);
		FORGET_OBJECT(cbackup);
		//
		// Actual appearance
		//


		// mouth
		s = shapes.get_shape(guardian_mouth_shp, 0);
		backup = win->create_buffer(s->get_width(), s->get_height());
		cbackup = win->create_buffer(s->get_width(), s->get_height());
		win->get(cbackup, centerx - s->get_xleft(), centery - s->get_yabove());
		sman->paint_shape(centerx, centery, s); // frame 0 is background
		win->get(backup, centerx - s->get_xleft(), centery - s->get_yabove());
		// eyes
		s2 = shapes.get_shape(guardian_eyes_shp, 0);
		backup2 = win->create_buffer(s2->get_width(), s2->get_height());
		cbackup2 = win->create_buffer(s2->get_width(), s2->get_height());
		win->get(cbackup2, centerx - s2->get_xleft(),
		         centery - EYES_DIST - s2->get_yabove());
		sman->paint_shape(centerx, centery - EYES_DIST, s2); // frame 0 is background
		win->get(backup2, centerx - s2->get_xleft(),
		         centery - EYES_DIST - s2->get_yabove());
		// forehead
		s3 = shapes.get_shape(guardian_forehead_shp, 0);
		cbackup3 = win->create_buffer(s3->get_width(), s3->get_height());
		win->get(cbackup3, centerx - s3->get_xleft(),
		         centery - FORHEAD_DIST - s3->get_yabove());
		sman->paint_shape(centerx, centery - FORHEAD_DIST, s3); // forehead isn't animated

		// prepare Guardian speech
		Font *font = fontManager.get_font("GUARDIAN_FONT");
		U7multiobject textobj(MAINSHP_FLX, PATCH_MAINSHP, 0x0D);
		size_t txt_len;
		next_txt = txt_ptr = txt = textobj.retrieve(txt_len);

		int txt_height = font->get_text_height();
		int txt_ypos = gwin->get_height() - txt_height - 16;

		// backup text area
		backup3 = win->create_buffer(win->get_full_width(), txt_height);
		win->get(backup3, win->get_start_x(), txt_ypos);

		// Lipsynching
		int eye_frame = 3, last_eye_frame = 3;
		int mouth_frame = 1;
		int text_index = -1;
		int next_time, next_code;
		LipSynchReader lipsync;
		LipSynchReader surfacing(surfacing_data, sizeof(surfacing_data));

		int time = 0;
		unsigned long start = SDL_GetTicks();

		bool speech = Audio::get_ptr()->is_audio_enabled() &&
		              Audio::get_ptr()->is_speech_enabled();
		bool speech_n_subs = Audio::get_ptr()->is_speech_with_subs();
		bool need_to_play_speech = speech; // speech timing can screw up due

#define ERASE_TEXT() do { \
		win->put(backup3, win->get_start_x(), txt_ypos); \
	} while(0)

#define DRAW_TEXT() if (!speech || speech_n_subs) \
		do { \
			font->center_text(win->get_ib8(), centerx, txt_ypos, txt_ptr); \
		} while (0)

#define ADVANCE_TEXT_POINTER() do { \
		txt_ptr = next_txt;     \
		txt_end = strchr(txt_ptr, '\r');\
		*txt_end = '\0';        \
		next_txt = txt_end+2;       \
	} while (0)

#define ERASE_AND_DRAW_EYES() do { \
		win->put(backup2, centerx - s2->get_xleft(),    \
		         centery-EYES_DIST - s2->get_yabove());  \
		sman->paint_shape(centerx,centery-EYES_DIST,    \
		                  shapes.get_shape(guardian_eyes_shp, eye_frame));    \
	} while (0)

#define ERASE_AND_DRAW_MOUTH() do { \
		win->put(backup, centerx - s->get_xleft(),  \
		         centery - s->get_yabove());     \
		sman->paint_shape(centerx,centery,      \
		                  shapes.get_shape(guardian_mouth_shp, mouth_frame));   \
	} while (0)

#define DRAW_SPEECH() do { \
		ERASE_TEXT();       \
		ERASE_AND_DRAW_EYES();  \
		ERASE_AND_DRAW_MOUTH(); \
		if (text_index > 0) \
			DRAW_TEXT();    \
	} while (0)

		// First event needs to be read here
		surfacing.get_event(next_time, next_code);
		DRAW_SPEECH();
		// before speech
		while (time < 2000) {
			if (surfacing.have_events() && time >= next_time) {
				surfacing.translate_code(next_code, mouth_frame, eye_frame, last_eye_frame);
				surfacing.get_event(next_time, next_code);
				DRAW_SPEECH();
			}

			non_gl_blit();
			WAITDELAYCYCLE5(15);
			non_gl_blit();
			time = (SDL_GetTicks() - start);
		}

		if (need_to_play_speech) {
			Audio::get_ptr()->playfile(INTROSND, PATCH_INTROSND, false);
		}

		time = 0;
		start = SDL_GetTicks();

		// First event needs to be read here
		lipsync.get_event(next_time, next_code);
		text_index = 0;
		// start speech
		while (time < 47537) {
			if (next_code && lipsync.have_events() && time >= next_time) {
				lipsync.translate_code(next_code, mouth_frame, eye_frame, last_eye_frame);
				lipsync.get_event(next_time, next_code);
				DRAW_SPEECH();
			}

			if (text_index < text_num_frames && time >= text_times[text_index]) {
				text_index++;
				ADVANCE_TEXT_POINTER();
				DRAW_SPEECH();
			}

			non_gl_blit();
			WAITDELAYCYCLE6(15);
			non_gl_blit();
			time = (SDL_GetTicks() - start);
		}

		non_gl_blit();
		WAITDELAYCYCLE6(1000);
		non_gl_blit();

		win->put(backup3, 0, txt_ypos);
		win->put(cbackup, centerx - s->get_xleft(), centery - s->get_yabove());
		win->put(cbackup2, centerx - s2->get_xleft(),
		         centery - EYES_DIST - s2->get_yabove());
		win->put(cbackup3, centerx - s3->get_xleft(),
		         centery - FORHEAD_DIST - s3->get_yabove());

		FORGET_ARRAY(txt);
		FORGET_OBJECT(backup);
		FORGET_OBJECT(backup2);
		FORGET_OBJECT(backup3);
		FORGET_OBJECT(cbackup);
		FORGET_OBJECT(cbackup2);
		FORGET_OBJECT(cbackup3);

		// G. disappears again (sp. 0x23 again)
		s = shapes.get_shape(0x23, 0);
		backup = win->create_buffer(s->get_width(), s->get_height());
		cbackup = win->create_buffer(s->get_width(), s->get_height());
		win->get(cbackup, centerx - s->get_xleft(), centery - s->get_yabove());
		sman->paint_shape(centerx, centery, s); // frame 0 is background
		win->get(backup, centerx - s->get_xleft(), centery - s->get_yabove());
		for (i = 15; i > 0; i--)
			FLASH_SHAPE2(centerx, centery, 0x23, i, 70);
		win->put(cbackup, centerx - s->get_xleft(), centery - s->get_yabove());
		FORGET_OBJECT(backup);
		FORGET_OBJECT(cbackup);

		non_gl_blit();
		enable_direct_gl_render();
		WAITDELAYCYCLE1(1200);

		Audio::get_ptr()->stop_music();

		//
		// More static
		//

		ticks = SDL_GetTicks();
		while (1) {
			win->get_ibuf()->fill_static(0, 7, 15);
#ifdef HAVE_OPENGL
			if (GL_manager::get_instance())
				Delay();
			else
#endif
				win->show();
			WAITDELAYCYCLE1(2);
#ifdef HAVE_OPENGL
			if (GL_manager::get_instance())
				Delay();
#endif
			if (SDL_GetTicks() > ticks + 400)
				break;
		}

		gwin->clear_screen(true);
		gl_clear_win();

		//
		// White dot
		//
		s = shapes.get_shape(0x14, 0);
		backup = win->create_buffer(s->get_width() + 2, s->get_height() + 2);
		win->get(backup, centerx - 1, centery - 1);

		ticks = SDL_GetTicks();
		while (true) {
			int x = centerx + rand() % 3 - 1;
			int y = centery + rand() % 3 - 1;
			FLASH_SHAPE1(x, y, 0x14, 0, 0);
			WAITDELAYCYCLE1(2);
			if (SDL_GetTicks() - ticks > 800)
				break;
		}
		FORGET_OBJECT(backup);
		gl_clear_win();
	} catch (const UserSkipException &/*x*/) {
		// Waste disposal
		Audio::get_ptr()->stop_speech();
		FORGET_ARRAY(txt);
		FORGET_OBJECT(backup);
		FORGET_OBJECT(backup2);
		FORGET_OBJECT(backup3);
		FORGET_OBJECT(cbackup);
		FORGET_OBJECT(cbackup2);
		FORGET_OBJECT(cbackup3);
		FORGET_OBJECT(plasma);
		gl_clear_win();
		enable_direct_gl_render();
	} catch (const UserBreakException &x) {
		// Waste disposal
		Audio::get_ptr()->stop_speech();
		FORGET_ARRAY(txt);
		FORGET_OBJECT(backup);
		FORGET_OBJECT(backup2);
		FORGET_OBJECT(backup3);
		FORGET_OBJECT(cbackup);
		FORGET_OBJECT(cbackup2);
		FORGET_OBJECT(cbackup3);
		FORGET_OBJECT(plasma);
		gl_clear_win();
		enable_direct_gl_render();

		throw;
	}
}

void BG_Game::scene_desk() {
	Shape_frame *s;
	Image_buffer *backup = 0;
	int i;
	int hits;

	try {
		Audio::get_ptr()->start_music(home_song_midi, false, INTROMUS);

		gwin->clear_screen();
		// Clip it to 320x200 region
		win->set_clip(centerx - 160, centery - 100, 320, 200);

		pal->load(INTROPAL_DAT, PATCH_INTROPAL, 1);
		pal->apply();

		// draw monitor (sh. 0x07, 0x08, 0x09, 0x0A: various parts of monitor)
		sman->paint_shape(centerx, centery, shapes.get_shape(0x07, 0));
		sman->paint_shape(centerx, centery, shapes.get_shape(0x09, 0));
		sman->paint_shape(centerx, centery, shapes.get_shape(0x08, 0));
		sman->paint_shape(centerx, centery, shapes.get_shape(0x0A, 0));

		// draw white dot in center of monitor (sh. 0x14)
		sman->paint_shape(centerx + 12, centery - 22, shapes.get_shape(0x14, 0));

		// Zoom out from zoomed in screen
		Image_buffer *unzoomed = win->create_buffer(320, 200);
		win->get(unzoomed, 0 + (win->get_game_width() - 320) / 2, 0 + (win->get_game_height() - 200) / 2);
		Image_buffer *zoomed = win->create_buffer(320, 200);

		const Image_window::ScalerInfo &scaler = Image_window::Scalers[Image_window::point];

		SDL_Surface *draw_surface = win->get_draw_surface();
		SDL_Surface *unzoomed_surf = SDL_CreateRGBSurfaceFrom(unzoomed->get_bits(), unzoomed->get_height(), unzoomed->get_width(), draw_surface->format->BitsPerPixel, unzoomed->get_line_width(), draw_surface->format->Rmask, draw_surface->format->Gmask, draw_surface->format->Bmask, draw_surface->format->Amask);
		SDL_Surface *zoomed_surf = SDL_CreateRGBSurfaceFrom(zoomed->get_bits(), zoomed->get_height(), zoomed->get_width(), draw_surface->format->BitsPerPixel, zoomed->get_line_width(), draw_surface->format->Rmask, draw_surface->format->Gmask, draw_surface->format->Bmask, draw_surface->format->Amask);

		try {
			const int zx = 88;
			const int zy = 22;
			const int zw = 166;
			const int zh = 112;

			uint32 next_ticks = SDL_GetTicks() + 10;
			for (i = 0; i < 40; i++) {
				int sw = zw + (320 - zw) * i / 40;
				int sh = zh + (200 - zh) * i / 40;
				int sx = zx + (0 - zx) * i / 40;
				int sy = zy + (0 - zy) * i / 40;

				// frame drop?
				if (next_ticks > SDL_GetTicks()) {
					scaler.arb->Scale(unzoomed_surf, sx, sy, sw, sh, zoomed_surf, 0, 0, 320, 200, true);
					win->put(zoomed, 0 + (win->get_game_width() - 320) / 2, 0 + (win->get_game_height() - 200) / 2);
					non_gl_blit();
					int delta = next_ticks - SDL_GetTicks();
					if (delta < 0) delta = 0;
					WAITDELAY(delta);
				}
				next_ticks += 10;
			}
		} catch (const UserBreakException &x) {
			SDL_FreeSurface(unzoomed_surf);
			SDL_FreeSurface(zoomed_surf);
			FORGET_OBJECT(unzoomed);
			FORGET_OBJECT(zoomed);
			throw;
		}

		win->put(unzoomed, 0 + (win->get_game_width() - 320) / 2, 0 + (win->get_game_height() - 200) / 2);
		win->show();

		SDL_FreeSurface(unzoomed_surf);
		SDL_FreeSurface(zoomed_surf);
		FORGET_OBJECT(unzoomed);
		FORGET_OBJECT(zoomed);


		// draw arm hitting pc (sh. 0x0C)
		s = shapes.get_shape(0x0C, 0);
#ifdef HAVE_OPENGL
		if (!GL_manager::get_instance())
#endif
			backup = win->create_buffer(s->get_width(), s->get_height());

		// TODO: add stuff on screen while hitting (static, butterfly scene, black)

		for (hits = 0; hits < 3; hits++) {
			WAITDELAY(100);
			for (i = 0; i < 5; i++) { //was i<9
#ifdef HAVE_OPENGL
				if (!GL_manager::get_instance())
#endif
					win->get(backup, centerx - 96 - 30 * abs(i % 4 - 2) - s->get_xleft(),
					         centery + 100 - s->get_yabove());
				sman->paint_shape(centerx - 96 - 30 * abs(i % 4 - 2), centery + 100, s);
				win->show();
#ifdef HAVE_OPENGL
				if (GL_manager::get_instance()) {
					sman->paint_shape(centerx, centery, shapes.get_shape(0x09, 0));
					sman->paint_shape(centerx, centery, shapes.get_shape(0x0A, 0));
				} else
#endif
					win->put(backup, centerx - 96 - 30 * abs(i % 4 - 2) - s->get_xleft(),
					         centery + 100 - s->get_yabove());
				WAITDELAY(80);
			}

		}

		// screen comes back up (sh. 0x1D)
		sman->paint_shape(centerx + 12, centery - 22, shapes.get_shape(0x1D, 0));
		win->show();
		WAITDELAY(1300); //

		FORGET_OBJECT(backup);

		// "Something is obviously amiss"
		sman->paint_shape(centerx, centery + 50, shapes.get_shape(0x15, 0));
		win->show();
		WAITDELAY(3000);

		// TODO: misaligned?

		// scroll right (sh. 0x06: map to the right of the monitor)
		for (i = 0; i <= 194; i += 2) { //was += 4
			sman->paint_shape(centerx - i, centery, shapes.get_shape(0x07, 0));
			sman->paint_shape(centerx - i, centery, shapes.get_shape(0x09, 0));
			sman->paint_shape(centerx - i, centery, shapes.get_shape(0x08, 0));
			sman->paint_shape(centerx - i, centery, shapes.get_shape(0x0A, 0));
			sman->paint_shape(centerx - i + 12, centery - 22,
			                  shapes.get_shape(0x1D, 0));
			sman->paint_shape(topx + 320 - i, topy, shapes.get_shape(0x06, 0));

			if (i > 75 && i < 194) {
				// "It has been a long time..."
				sman->paint_shape(centerx, centery + 50,
				                  shapes.get_shape(0x16, 0));
			}
			win->show();
			WAITDELAY(110); //was 30
		}

		WAITDELAY(1500);
		disable_direct_gl_render();
		// scroll down (sh. 0x0B: mouse + orb of moons, below map)
		for (i = 0; i <= 50; i += 2) {
			sman->paint_shape(centerx - 194, centery - i,
			                  shapes.get_shape(0x07, 0));
			sman->paint_shape(centerx - 194, centery - i,
			                  shapes.get_shape(0x09, 0));
			sman->paint_shape(centerx - 194, centery - i,
			                  shapes.get_shape(0x08, 0));
			sman->paint_shape(centerx - 194, centery - i,
			                  shapes.get_shape(0x0A, 0));
			sman->paint_shape(centerx - 194 + 12, centery - 22 - i,
			                  shapes.get_shape(0x1D, 0));
			sman->paint_shape(topx + 319 - 194, topy - i,
			                  shapes.get_shape(0x06, 0));
			sman->paint_shape(topx + 319, topy + 199 - i,
			                  shapes.get_shape(0x0B, 0));
			// "The mystical Orb beckons you"
			sman->paint_shape(centerx, topy, shapes.get_shape(0x17, 0));
			non_gl_blit();
			WAITDELAYCYCLE2(110);
		}

		sman->paint_shape(centerx - 194, centery - 50, shapes.get_shape(0x07, 0));
		sman->paint_shape(centerx - 194, centery - 50, shapes.get_shape(0x09, 0));
		sman->paint_shape(centerx - 194, centery - 50, shapes.get_shape(0x08, 0));
		sman->paint_shape(centerx - 194, centery - 50, shapes.get_shape(0x0A, 0));
		sman->paint_shape(centerx - 182, centery - 72, shapes.get_shape(0x1D, 0));
		sman->paint_shape(topx + 319 - 194, topy - 50, shapes.get_shape(0x06, 0));
		sman->paint_shape(topx + 319, topy + 149, shapes.get_shape(0x0B, 0));
		// "It has opened gateways to Britannia in the past"
		sman->paint_shape(centerx, topy, shapes.get_shape(0x18, 0));
		non_gl_blit();

		WAITDELAYCYCLE2(3200);
		pal->fade_out(100);
		gwin->clear_screen(true);
		gl_clear_win();
		enable_direct_gl_render();
	} catch (const UserSkipException &/*x*/) {
		win->clear_clip();
		FORGET_OBJECT(backup);
		enable_direct_gl_render();
	} catch (const UserBreakException &x) {
		win->clear_clip();
		// Waste disposal
		FORGET_OBJECT(backup);
		enable_direct_gl_render();
		throw;
	}
}

void BG_Game::scene_moongate() {
	// sh. 0x02, 0x03, 0x04, 0x05: various parts of moongate
	// sh. 0x00, 0x01: parting trees before clearing

	int i;

	gwin->clear_screen(false);
	pal->load(INTROPAL_DAT, PATCH_INTROPAL, 5);
	pal->apply();

	// "Behind your house is the circle of stones"
	sman->paint_shape(centerx, centery + 50, shapes.get_shape(0x19, 0));
	pal->fade_in(c_fade_in_time);

	// TODO: moongate sound

	// TODO: fade in screen while text is onscreen

	WAITDELAY(4000);

	disable_direct_gl_render();
	// Bushes move out of the way
	for (i = 50; i >= -170; i -= 2) { // was for(i=120;i>=-170;i-=6)
		sman->paint_shape(centerx + 1, centery + 1,
		                  shapes.get_shape(0x02, 0));
		sman->paint_shape(centerx + 1, centery + 1,
		                  shapes.get_shape(0x03, 0));
		sman->paint_shape(centerx + 1, centery + 1,
		                  shapes.get_shape(0x04, 0));
		sman->paint_shape(centerx + 1, centery + 1,
		                  shapes.get_shape(0x05, 0));

		// TODO: if you watch the original closely, the bushes are scaled up in size
		// slightly as they move out.
		sman->paint_shape(centerx + i, topy - ((i - 60) / 4), shapes.get_shape(0x00, 0));
		sman->paint_shape(centerx - i, topy - ((i - 60) / 4), shapes.get_shape(0x01, 0));

		if ((40 > i) && (i > -100)) {
			// "Why is a moongate already there?"
			sman->paint_shape(centerx, centery + 50, shapes.get_shape(0x1A, 0));
		} else if (i <= -100) {
			// "You have but one path to the answer"
			sman->paint_shape(centerx, centery + 50, shapes.get_shape(0x1C, 0));
		}

		non_gl_blit();
		WAITDELAYCYCLE3(80);

	}

	// Wait till the music finished playing
	while (Audio::get_ptr()->is_track_playing(home_song_midi))
		WAITDELAYCYCLE3(50);

	// zoom (run) into moongate

	sman->paint_shape(centerx + 1, centery + 1, shapes.get_shape(0x02, 0));
	sman->paint_shape(centerx + 1, centery + 1, shapes.get_shape(0x03, 0));
	sman->paint_shape(centerx + 1, centery + 1, shapes.get_shape(0x04, 0));
	sman->paint_shape(centerx + 1, centery + 1, shapes.get_shape(0x05, 0));

	Image_buffer *unzoomed = win->create_buffer(320, 200);
	win->get(unzoomed, 0 + (win->get_game_width() - 320) / 2, 0 + (win->get_game_height() - 200) / 2);
	Image_buffer *zoomed = win->create_buffer(320, 200);

	const Image_window::ScalerInfo &scaler = Image_window::Scalers[Image_window::point];

	SDL_Surface *draw_surface = win->get_draw_surface();
	SDL_Surface *unzoomed_surf = SDL_CreateRGBSurfaceFrom(unzoomed->get_bits(), unzoomed->get_height(), unzoomed->get_width(), draw_surface->format->BitsPerPixel, unzoomed->get_line_width(), draw_surface->format->Rmask, draw_surface->format->Gmask, draw_surface->format->Bmask, draw_surface->format->Amask);
	SDL_Surface *zoomed_surf = SDL_CreateRGBSurfaceFrom(zoomed->get_bits(), zoomed->get_height(), zoomed->get_width(), draw_surface->format->BitsPerPixel, zoomed->get_line_width(), draw_surface->format->Rmask, draw_surface->format->Gmask, draw_surface->format->Bmask, draw_surface->format->Amask);

	try {
		const int zx = 151;
		const int zy = 81;
		const int zw = 5;
		const int zh = 4;

		uint32 next_ticks = SDL_GetTicks() + 10;
		for (i = 159; i >= 0; i--) {
			int sw = zw + (320 - zw) * i / 160;
			int sh = zh + (200 - zh) * i / 160;
			int sx = zx + (0 - zx) * i / 160;
			int sy = zy + (0 - zy) * i / 160;

			// frame drop?
			if (next_ticks > SDL_GetTicks()) {
				scaler.arb->Scale(unzoomed_surf, sx, sy, sw, sh, zoomed_surf, 0, 0, 320, 200, true);
				win->put(zoomed, 0 + (win->get_game_width() - 320) / 2, 0 + (win->get_game_height() - 200) / 2);
				non_gl_blit();
				int delta = next_ticks - SDL_GetTicks();
				if (delta < 0) delta = 0;
				WAITDELAYCYCLE3(delta);
			}
			next_ticks += 5;
		}
	} catch (const UserBreakException &x) {
		SDL_FreeSurface(unzoomed_surf);
		SDL_FreeSurface(zoomed_surf);
		FORGET_OBJECT(unzoomed);
		FORGET_OBJECT(zoomed);
		throw;
	}
	SDL_FreeSurface(unzoomed_surf);
	SDL_FreeSurface(zoomed_surf);
	FORGET_OBJECT(unzoomed);
	FORGET_OBJECT(zoomed);

	enable_direct_gl_render();
}

Shape_frame *BG_Game::get_menu_shape() {
	return menushapes.get_shape(0x2, 0);
}


void BG_Game::top_menu() {
	Audio::get_ptr()->start_music(menu_midi, true, INTROMUS);
	sman->paint_shape(topx, topy, get_menu_shape());
	pal->load(INTROPAL_DAT, PATCH_INTROPAL, 6);
	pal->fade_in(60);
}

void BG_Game::show_journey_failed() {
	pal->fade_out(50);
	gwin->clear_screen(true);
	sman->paint_shape(topx, topy, get_menu_shape());
	journey_failed_text();
}

class ExVoiceBuffer {
private:
	const char *file;
	const char *patch;
	int index;
	bool played;
public:
	bool play_it();

	ExVoiceBuffer(const char *f, const char *p, int i)
		: file(f), patch(p), index(i), played(false)
	{ }
	~ExVoiceBuffer()
	{ }
	bool can_play() const {
		return file || patch;
	}
};

bool ExVoiceBuffer::play_it() {
	size_t  size;
	U7multiobject voc(file, patch, index);
	uint8 *buffer = reinterpret_cast<uint8 *>(voc.retrieve(size));
	uint8 *buf = buffer;
	if (!memcmp(buf, "voc", sizeof("voc") - 1)) {
		// IFF junk.
		buf += 8;
		size -= 8;
	}
	Audio::get_ptr()->copy_and_play(buf, size, false);
	FORGET_ARRAY(buffer);
	played = true;

	return false;
}

void BG_Game::end_game(bool success) {
	unsigned int next = 0;
	int starty;
	Font *font = fontManager.get_font("MENU_FONT");

	if (!success) {
		TextScroller text(MAINSHP_FLX, 0x15,
		                  font, 0);
		gwin->clear_screen();
		pal->load(INTROPAL_DAT, PATCH_INTROPAL, 0);
		for (sint32 i = 0; i < text.get_count(); i++) {
			text.show_line(gwin, topx, topx + 320, topy + 20 + i * 12, i);
		}

		pal->fade_in(c_fade_in_time);
		wait_delay(10000);
		pal->fade_out(c_fade_out_time);

		gwin->clear_screen();
		font->center_text(ibuf, centerx, centery - 10, get_text_msg(end_of_ultima7));
		pal->fade_in(c_fade_in_time);
		wait_delay(4000);
		pal->fade_out(c_fade_out_time);

		gwin->clear_screen();
		font->center_text(ibuf, centerx, centery - 10, get_text_msg(end_of_britannia));
		pal->fade_in(c_fade_in_time);
		wait_delay(4000);
		pal->fade_out(c_fade_out_time);
		gwin->clear_screen(true);
		return;
	}

	Audio *audio = Audio::get_ptr();
	audio->stop_music();
	MyMidiPlayer *midi = audio->get_midi();
	if (midi) midi->set_timbre_lib(MyMidiPlayer::TIMBRE_LIB_ENDGAME);

	bool speech = Audio::get_ptr()->is_audio_enabled() &&
	              Audio::get_ptr()->is_speech_enabled();
	bool speech_n_subs = Audio::get_ptr()->is_speech_with_subs();

	// Fli Buffers
	size_t  flisize;
	char    *fli_b[3];

	// Clear screen
	gwin->clear_screen(true);

	U7multiobject flic1(ENDGAME, PATCH_ENDGAME, 0);
	U7multiobject flic2(ENDGAME, PATCH_ENDGAME, 1);
	U7multiobject flic3(ENDGAME, PATCH_ENDGAME, 2);

	ExVoiceBuffer speech1(ENDGAME, PATCH_ENDGAME, 7);
	ExVoiceBuffer speech2(ENDGAME, PATCH_ENDGAME, 8);
	ExVoiceBuffer speech3(ENDGAME, PATCH_ENDGAME, 9);

	/* There seems to be something wrong with the shapes. Needs investigating
	U7multiobject shapes(ENDGAME, PATCH_ENDGAME, 10);
	shapes.retrieve("endgame.shp");
	Shape_file sf("endgame.shp");
	int x = get_width()/2-160;
	int y = get_height()/2-100;
	cout << "Shape in Endgame.dat has " << sf.get_num_frames() << endl;
	*/

	fli_b[0] = flic1.retrieve(flisize);
	playfli fli1(fli_b[0] + 8, flisize - 8);

	fli_b[1] = flic2.retrieve(flisize);
	playfli fli2(fli_b[1] + 8, flisize - 8);

	fli_b[2] = flic3.retrieve(flisize);
	playfli fli3(fli_b[2] + 8, flisize - 8);

	fli1.play(win, 0, 0, 0);

	// Start endgame music.
	if (midi) midi->start_music(ENDSCORE_XMI, 1, false);

	// A little hack
	bool do_break = false;
	do {

		for (unsigned int i = 0; i < 240; i++) {
			next = fli1.play(win, 0, 1, next);
			if (wait_delay(0)) {
				do_break = true;
				break;
			}
		}
		if (do_break) break;

		for (unsigned int i = 1; i < 150; i++) {
			next = fli1.play(win, i, i + 1, next);
			if (wait_delay(0)) {
				do_break = true;
				break;
			}
		}
		if (do_break) break;

		if (speech) speech1.play_it();
		Font *endfont2 = fontManager.get_font("END2_FONT");
		Font *endfont3 = fontManager.get_font("END3_FONT");
		Font *normal = fontManager.get_font("NORMAL_FONT");

		const char *message = get_text_msg(you_cannot_do_that);
		int height = topy + 200 - endfont2->get_text_height() * 2;
		int width = (gwin->get_width() - endfont2->get_text_width(message)) / 2;

		disable_direct_gl_render();
		for (unsigned int i = 150; i < 204; i++) {
			next = fli1.play(win, i, i, next);
			if (!speech || speech_n_subs)
				endfont2->draw_text(ibuf, width, height, message);
			non_gl_blit();
			if (wait_delay(0, 0, 1)) {
				do_break = true;
				break;
			}
		}
		if (do_break) break;

		// Set new music
		if (midi) midi->start_music(ENDSCORE_XMI, 2, false);

		// Set speech

		if (speech) speech2.play_it();

		message = get_text_msg(damn_avatar);
		width = (gwin->get_width() - endfont2->get_text_width(message)) / 2;

		for (unsigned int i = 0; i < 100; i++) {
			next = fli2.play(win, i, i, next);
			if (!speech  || speech_n_subs)
				endfont2->draw_text(ibuf, width, height, message);
			non_gl_blit();
			if (wait_delay(0, 0, 1)) {
				do_break = true;
				break;
			}
		}
		if (do_break) break;

		Palette *pal = fli2.get_palette();
		next = SDL_GetTicks();
		for (unsigned int i = 1000 + next; next < i; next += 10) {
			// Speed related frame skipping detection
			int skip_frame = Game_window::get_instance()->get_frame_skipping() && SDL_GetTicks() >= next;
			while (SDL_GetTicks() < next)
				;
			if (!skip_frame) {
				pal->set_brightness((i - next) / 10);
				pal->apply();
			}
			if (wait_delay(0, 0, 1)) {
				do_break = true;
				break;
			}
		}
		if (do_break) break;

		enable_direct_gl_render();
#ifdef HAVE_OPENGL
		if (GL_manager::get_instance())
			pal->set(0, 80, true);
		else
#endif
			pal->set_brightness(80);    // Set readable brightness
		// Text message 1

		// Paint backgound black
		win->fill8(0);

		// Paint text
		message = get_text_msg(blackgate_destroyed);
		width = (gwin->get_width() - normal->get_text_width(message)) / 2;
		height = (gwin->get_height() - normal->get_text_height()) / 2;

		normal->draw_text(ibuf, width, height, message);

		// Fade in for 1 sec (50 cycles)
		pal->fade(50, 1, 0);

		// Display text for 3 seconds
		for (unsigned int i = 0; i < 30; i++) {
			if (wait_delay(100)) {
				do_break = true;
				break;
			}
		}
		if (do_break) break;

		// Fade out for 1 sec (50 cycles)
		pal->fade(50, 0, 0);


		// Now the second text message


		// Paint backgound black
		win->fill8(0);

		// Paint text
		message = get_text_msg(guardian_has_stopped);
		width = (gwin->get_width() - normal->get_text_width(message)) / 2;

		normal->draw_text(ibuf, width, height, message);

		// Fade in for 1 sec (50 cycles)
		pal->fade(50, 1, 0);

		// Display text for approx 3 seonds
		for (unsigned int i = 0; i < 30; i++) {
			if (wait_delay(100)) {
				do_break = true;
				break;
			}
		}
		if (do_break) break;

		// Fade out for 1 sec (50 cycles)
		pal->fade(50, 0, 0);

		disable_direct_gl_render();
		next = fli3.play(win, 0, 0, next);
		pal = fli3.get_palette();
		next = SDL_GetTicks();
		for (unsigned int i = 1000 + next; next < i; next += 10) {
			// Speed related frame skipping detection
			int skip_frame = Game_window::get_instance()->get_frame_skipping() && SDL_GetTicks() >= next;
			while (SDL_GetTicks() < next)
				;
			if (!skip_frame) {
				pal->set_brightness(100 - (i - next) / 10);
				pal->apply();
			}
			if (wait_delay(0, 0, 1)) {
				do_break = true;
				break;
			}
		}
		if (do_break) break;

		if (speech) speech3.play_it();

		playfli::fliinfo finfo;
		fli3.info(&finfo);

		int m;
		starty = (gwin->get_height() - endfont3->get_text_height() * 8) / 2;

		next = SDL_GetTicks();
		for (unsigned int i = next + 28000; i > next;) {
			for (unsigned int j = 0; j < static_cast<unsigned>(finfo.frames); j++) {
				next = fli3.play(win, j, j, next);
				if (!speech || speech_n_subs) {
					for (m = 0; m < 8; m++)
						endfont3->center_text(ibuf, centerx, starty + endfont3->get_text_height()*m, get_text_msg(txt_screen0 + m));
				}
				non_gl_blit();
				if (wait_delay(10, 0, 1)) {
					do_break = true;
					break;
				}
			}
		}
		if (do_break) break;
		enable_direct_gl_render();

		next = SDL_GetTicks();
		for (unsigned int i = 1000 + next; next < i; next += 10) {
			// Speed related frame skipping detection
			int skip_frame = Game_window::get_instance()->get_frame_skipping() && SDL_GetTicks() >= next;
			while (SDL_GetTicks() < next)
				;
			if (!skip_frame) {
				pal->set_brightness((i - next) / 10);
				pal->apply();
			}
			if (wait_delay(0, 0, 1)) {
				do_break = true;
				break;
			}
		}
		if (do_break) break;

		// Text Screen 1

#ifdef HAVE_OPENGL
		if (GL_manager::get_instance())
			pal->set(0, 80, true);
		else
#endif
			pal->set_brightness(80);    // Set readable brightness

		// Paint backgound black
		win->fill8(0);
		
		//Because of the German version we have to fit 11 lines of height 20 into a screen of 200 pixels
		//so starty has needs to be a tiny bit in the negative but not -10
		starty = (gwin->get_height() - normal->get_text_height() * 11) / 2.5;

		for (unsigned int i = 0; i < 11; i++) {
			message = get_text_msg(txt_screen1 + i);
			normal->draw_text(ibuf, centerx - normal->get_text_width(message) / 2, starty + normal->get_text_height()*i, message);
		}

		// Fade in for 1 sec (50 cycles)
		pal->fade(50, 1, 0);

		// Display text for 20 seconds (only 10 at the moment)
		for (unsigned int i = 0; i < 100; i++) {
			if (wait_delay(100)) {
				do_break = true;
				break;
			}
		}
		if (do_break) break;

		// Fade out for 1 sec (50 cycles)
		pal->fade(50, 0, 0);

		if (wait_delay(10)) break;

		// Text Screen 2

		// Paint backgound black
		win->fill8(0);

		starty = (gwin->get_height() - normal->get_text_height() * 9) / 2;

		for (unsigned int i = 0; i < 9; i++) {
			message = get_text_msg(txt_screen2 + i);
			normal->draw_text(ibuf, centerx - normal->get_text_width(message) / 2, starty + normal->get_text_height()*i, message);
		}

		// Fade in for 1 sec (50 cycles)
		pal->fade(50, 1, 0);

		// Display text for 20 seonds (only 8 at the moment)
		for (unsigned int i = 0; i < 80; i++) {
			if (wait_delay(100)) {
				do_break = true;
				break;
			}
		}
		if (do_break) break;

		// Fade out for 1 sec (50 cycles)
		pal->fade(50, 0, 0);

		if (wait_delay(10)) break;

		// Text Screen 3

		// Paint backgound black
		win->fill8(0);

		starty = (gwin->get_height() - normal->get_text_height() * 8) / 2;

		for (unsigned int i = 0; i < 8; i++) {
			message = get_text_msg(txt_screen3 + i);
			normal->draw_text(ibuf, centerx - normal->get_text_width(message) / 2, starty + normal->get_text_height()*i, message);
		}

		// Fade in for 1 sec (50 cycles)
		pal->fade(50, 1, 0);

		// Display text for 20 seonds (only 8 at the moment)
		for (unsigned int i = 0; i < 80; i++) {
			if (wait_delay(100)) {
				do_break = true;
				break;
			}
		}
		if (do_break) break;

		// Fade out for 1 sec (50 cycles)
		pal->fade(50, 0, 0);

		if (wait_delay(10)) break;

		// Text Screen 4

		// Paint backgound black
		win->fill8(0);

		starty = (gwin->get_height() - normal->get_text_height() * 5) / 2;

		for (unsigned int i = 0; i < 5; i++) {
			message = get_text_msg(txt_screen4 + i);
			normal->draw_text(ibuf, centerx - normal->get_text_width(message) / 2, starty + normal->get_text_height()*i, message);
		}

		// Fade in for 1 sec (50 cycles)
		pal->fade(50, 1, 0);

		// Display text for 10 seonds (only 5 at the moment)
		for (unsigned int i = 0; i < 50; i++) {
			if (wait_delay(100)) {
				do_break = true;
				break;
			}
		}
		if (do_break) break;

		// Fade out for 1 sec (50 cycles)
		pal->fade(50, 0, 0);
#if 0
		//TODO: only when finishing a game and not when viewed from menu
		if (when not in menu) {
			if (wait_delay(10)) break;
		
			// Congratulations

			// Paint backgound black
			win->fill8(0);

			starty = (gwin->get_height() - normal->get_text_height() * 6) / 2;

			//TODO: figure out the time it took to complete the game
			// in exultmsg.txt it is "%d year s ,  %d month s , &  %d day s"
			// only showing years or months if there were any
			for (unsigned int i = 0; i < 9; i++) {
				message = get_text_msg(congrats + i);
				normal->draw_text(ibuf, centerx - normal->get_text_width(message) / 2, starty + normal->get_text_height()*i, message);
			}

			// Fade in for 1 sec (50 cycles)
			pal->fade(50, 1, 0);

			// Display text for 20 seonds (only 8 at the moment)
			for (unsigned int i = 0; i < 80; i++) {
				if (wait_delay(100)) {
					do_break = true;
					break;
				}
			}
			if (do_break) break;

			// Fade out for 1 sec (50 cycles)
			pal->fade(50, 0, 0);
		}
#endif

	} while (0);

	if (midi) {
		midi->stop_music();
		midi->set_timbre_lib(MyMidiPlayer::TIMBRE_LIB_GAME);
	}

	audio->stop_music();

	gwin->clear_screen(true);
	FORGET_ARRAY(fli_b[0]);
	FORGET_ARRAY(fli_b[1]);
	FORGET_ARRAY(fli_b[2]);
	enable_direct_gl_render();
}

void BG_Game::show_quotes() {
	Audio::get_ptr()->start_music(quotes_midi, false, INTROMUS);
	TextScroller quotes(MAINSHP_FLX, 0x10,
	                    fontManager.get_font("MENU_FONT"),
	                    menushapes.extract_shape(0x14)
	                   );
	quotes.run(gwin);
}

void BG_Game::show_credits() {
	pal->load(INTROPAL_DAT, PATCH_INTROPAL, 6);
	Audio::get_ptr()->start_music(credits_midi, false, INTROMUS);
	TextScroller credits(MAINSHP_FLX, 0x0E,
	                     fontManager.get_font("MENU_FONT"),
	                     menushapes.extract_shape(0x14)
	                    );
	if (credits.run(gwin)) { // Watched through the entire sequence?
		std::ofstream quotesflg;
		U7open(quotesflg, "<SAVEGAME>/quotes.flg");
		quotesflg.close();
	}
}

bool BG_Game::new_game(Vga_file &shapes) {
	SDL_EnableUNICODE(1);

	int menuy = topy + 110;
	Font *font = fontManager.get_font("MENU_FONT");

	Vga_file faces_vga;
	// Need to know if SI is installed
	bool si_installed =
	    (gamemanager->is_si_installed() || gamemanager->is_ss_installed())
	    && U7exists("<SERPENT_STATIC>/shapes.vga");

	// List of files to load.
	std::vector<std::pair<std::string, int> > source;
	source.push_back(std::pair<std::string, int>(FACES_VGA, -1));
	// Multiracial faces.
	const str_int_pair &resource = game->get_resource("files/mrfacesvga");
	source.push_back(std::pair<std::string, int>(resource.str, resource.num));
	source.push_back(std::pair<std::string, int>(PATCH_FACES, -1));
	faces_vga.load(source);

	const int max_name_len = 16;
	char npc_name[max_name_len + 1];
	char disp_name[max_name_len + 2];
	npc_name[0] = 0;

	int selected = 0;
#ifndef __IPHONEOS__
	int num_choices = 4;
#endif
	SDL_Event event;
	bool editing = true;
	bool redraw = true;
	bool ok = true;

	// Skin info
	Avatar_default_skin *defskin = Shapeinfo_lookup::GetDefaultAvSkin();
	Skin_data *skindata =
	    Shapeinfo_lookup::GetSkinInfoSafe(
	        defskin->default_skin, defskin->default_female, si_installed);

	Palette *pal = gwin->get_pal();
	// This should work because the palette in exult_bg.flx is
	// a single-file object.
	pal->load(INTROPAL_DAT,
	          File_spec(get_resource("files/gameflx").str, EXULT_BG_FLX_U7MENUPAL_PAL),
	          PATCH_INTROPAL, 6);
	Palette *oldpal = new Palette();
	oldpal->load(INTROPAL_DAT, PATCH_INTROPAL, 6);

	// Create palette translation table. Maybe make them static?
	unsigned char *transto = new unsigned char[256];
	oldpal->create_palette_map(pal, transto);
	delete oldpal;
	pal->apply(true);

	do {
		Delay();
		if (redraw || GL_manager::get_instance()) {
			gwin->clear_screen();
			sman->paint_shape(topx, topy, shapes.get_shape(0x2, 0), 0, transto);
			sman->paint_shape(topx + 10, menuy + 10, shapes.get_shape(0xC, selected == 0), 0, transto);
			Shape_frame *sex_shape = shapes.get_shape(0xA, selected == 1);
			sman->paint_shape(topx + 10, menuy + 25, sex_shape, 0, transto);
			int sex_width = sex_shape->get_width() + 10;
			if (sex_width > 35) sex_width += 25;
			else sex_width = 60;

			sman->paint_shape(topx + sex_width, menuy + 25, shapes.get_shape(0xB, skindata->is_female), 0, transto);

			Shape_frame *portrait = faces_vga.get_shape(skindata->face_shape, skindata->face_frame);
			sman->paint_shape(topx + 290, menuy + 61, portrait);

			sman->paint_shape(topx + 10, topy + 180, shapes.get_shape(0x8, selected == 2), 0, transto);
			sman->paint_shape(centerx + 10, topy + 180, shapes.get_shape(0x7, selected == 3), 0, transto);
			if (selected == 0)
				snprintf(disp_name, max_name_len + 2, "%s_", npc_name);
			else
				snprintf(disp_name, max_name_len + 2, "%s", npc_name);
			font->draw_text(ibuf, topx + 60, menuy + 10, disp_name, transto);
			gwin->get_win()->show();
			redraw = false;
		}
		while (SDL_PollEvent(&event)) {

#ifdef __IPHONEOS__
			/* Use touch based control instead of navigation with keyboard.
			 * Also there is no direction keys on native iPhone keyboard.
			 */
			if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
				SDL_Rect rectName   = { topx + 10,    menuy + 10, 130,  16 };
				SDL_Rect rectSex    = { topx + 10,    menuy + 25, 130,  16 };
				SDL_Rect rectOnward = { topx + 10,    topy + 180, 130,  16 };
				SDL_Rect rectReturn = { centerx + 10, topy + 180, 130,  16 };
				SDL_Point point;
				gwin->get_win()->screen_to_game(event.button.x, event.button.y, gwin->get_fastmouse(), point.x, point.y);
#if DEBUG
				std::cout << "x: " << point.x << " y:" << point.y << std::endl;
#endif
				if (SDL_EnclosePoints(&point, 1, &rectName, NULL)) {
					if (event.type == SDL_MOUSEBUTTONDOWN) {
						selected = 0;
					} else if (selected == 0) {
						touchui->promptForName(npc_name);
					}
					redraw = true;				
				} else if (SDL_EnclosePoints(&point, 1, &rectSex, NULL)) {
					if (event.type == SDL_MOUSEBUTTONDOWN) {
						selected = 1;
					} else if (selected == 1) {
						skindata = Shapeinfo_lookup::GetNextSelSkin(skindata, si_installed, true);
					}
					redraw = true;				
				} else if (SDL_EnclosePoints(&point, 1, &rectOnward, NULL)) {
					if (event.type == SDL_MOUSEBUTTONDOWN) {
						selected = 2;
					} else if (selected == 2) {
						editing = false;
						ok = true;
					}
					redraw = true;				
				} else if (SDL_EnclosePoints(&point, 1, &rectReturn, NULL)) {
					if (event.type == SDL_MOUSEBUTTONDOWN) {
						selected = 3;
					} else if (selected == 3) {
						editing = false;
						ok = false;
					}
					redraw = true;				
				}
			} else if (event.type == TouchUI::eventType) {
				if (event.user.code == TouchUI::EVENT_CODE_TEXT_INPUT) {
					if (selected == 0 && event.user.data1 != NULL) {
						strcpy(npc_name, (char*)event.user.data1);
						redraw = true;
					}
				}
			}
			
#else // __IPHONEOS__
			
			Uint16 keysym_unicode = 0;
#if SDL_VERSION_ATLEAST(2, 0, 0)
			bool isTextInput = false;
			if (event.type == SDL_TEXTINPUT) {
				isTextInput = true;
				event.type = SDL_KEYDOWN;
				event.key.keysym.sym = SDLK_UNKNOWN;
				keysym_unicode = event.text.text[0];
   #if !(SDL_VERSION_ATLEAST(2, 0, 0))
				event.key.keysym.unicode = keysym_unicode;
   #endif
			}
#endif
			if (event.type == SDL_KEYDOWN) {
				redraw = true;
				switch (event.key.keysym.sym) {
				case SDLK_SPACE:
					if (selected == 0) {
						int len = strlen(npc_name);
						if (len < max_name_len) {
							npc_name[len] = ' ';
							npc_name[len + 1] = 0;
						}
					} else if (selected == 1)
						skindata = Shapeinfo_lookup::GetNextSelSkin(skindata, si_installed, true);
					else if (selected == 2) {
						editing = false;
						ok = true;
					} else if (selected == 3)
						editing = ok = false;
					break;
				case SDLK_LEFT:
					if (selected == 1)
						skindata = Shapeinfo_lookup::GetPrevSelSkin(skindata, si_installed, true);
					break;
				case SDLK_RIGHT:
					if (selected == 1)
						skindata = Shapeinfo_lookup::GetNextSelSkin(skindata, si_installed, true);
					break;
				case SDLK_ESCAPE:
					editing = false;
					ok = false;
					break;
				case SDLK_TAB:
				case SDLK_DOWN:
					++selected;
					if (selected == num_choices)
						selected = 0;
					break;
				case SDLK_UP:
					--selected;
					if (selected < 0)
						selected = num_choices - 1;
					break;
				case SDLK_RETURN:
				case SDLK_KP_ENTER:
					if (selected < 2)
						++selected;
					else if (selected == 2) {
						editing = false;
						ok = true;
					} else
						editing = ok = false;
					break;
				case SDLK_BACKSPACE:
					if (selected == 0 && strlen(npc_name) > 0)
						npc_name[strlen(npc_name) - 1] = 0;
					break;
				default: {
#if SDL_VERSION_ATLEAST(2, 0, 0)
					if ((isTextInput && selected == 0) || (!isTextInput && keysym_unicode > +'~' && selected == 0))
#else
					if (selected == 0) // on the text input field?
#endif
					{
						int len = strlen(npc_name);
						char chr = 0;
#if !(SDL_VERSION_ATLEAST(2, 0, 0))
						keysym_unicode = event.key.keysym.unicode;
#endif
						if ((keysym_unicode & 0xFF80) == 0)
							chr = keysym_unicode & 0x7F;

						if (chr >= ' ' && len < max_name_len) {
							npc_name[len] = chr;
							npc_name[len + 1] = 0;
						}
					} else
						redraw = false;
				}
				break;
				}
			}
#endif // __IPHONEOS__
		}
	} while (editing);

	delete [] transto;
	gwin->clear_screen();

	if (ok) {
#ifdef DEBUG
		std::cout << "Skin is: " << skindata->skin_id << " Sex is: " << skindata->is_female << std::endl;
#endif
		set_avskin(skindata->skin_id);
		set_avname(npc_name);
		set_avsex(skindata->is_female);
		pal->fade_out(c_fade_out_time);
		gwin->clear_screen(true);
		ok = gwin->init_gamedat(true);
	} else {
		pal->load(INTROPAL_DAT, PATCH_INTROPAL, 6);
		sman->paint_shape(topx, topy, shapes.get_shape(0x2, 0));
		pal->apply();
	}

	SDL_EnableUNICODE(0);
	return ok;
}
