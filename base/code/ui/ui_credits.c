/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006-2008 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2008 Pat Raynor <raynorpat@gmail.com>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

/*
=======================================================================

CREDITS

=======================================================================
*/

#include "ui_local.h"

#define SCROLLSPEED	3

typedef struct
{
	menuframework_s menu;
} creditsmenu_t;

static creditsmenu_t s_credits;

int             starttime;		// game time at which credits are started
float           mvolume;		// records the original music volume level

//qhandle_t       BackgroundShader;

typedef struct
{
	char           *string;
	int             style;
	vec_t          *color;

} cr_line;

cr_line         credits[] = {
	{"Weaver Team", UI_CENTER | UI_GIANTFONT, colorRed},
	{"http://www.dersaidin.net/weaver/forum/", UI_CENTER | UI_SMALLFONT, colorBlue},
	{"", UI_CENTER | UI_SMALLFONT, colorBlack},

	{"Core Team:", UI_CENTER | UI_BIGFONT, colorRed},
	{"                   Andrew 'DerSaidin' Browne (AU) - Project Lead, Programming Lead, Map Design       ", UI_CENTER | UI_SMALLFONT, colorRed},
	{"                      Georges 'TRaK' Grondin (CA) - Artistic Lead, Environment Artist, Texture Artist", UI_CENTER | UI_SMALLFONT, colorRed},
	{"                          '10percentmachine' (CA) - Concept Artist                                   ", UI_CENTER | UI_SMALLFONT, colorRed},
	{"                                 'Endoperez' (CA) - Animator                                         ", UI_CENTER | UI_SMALLFONT, colorRed},
	{"                               Jani Reijonen (FI) - Graphics Artist                                  ", UI_CENTER | UI_SMALLFONT, colorRed},
	{"               Tuomas 'Bright Side' Kallinen (FI) - Audio Producer                                   ", UI_CENTER | UI_SMALLFONT, colorRed},
	{"                                'Karvajalka' (CA) - 3D Props Artist                                  ", UI_CENTER | UI_SMALLFONT, colorRed},
	{"                                'mikeplus64' (AU) - Environment Artist                               ", UI_CENTER | UI_SMALLFONT, colorRed},
	{"", UI_CENTER | UI_SMALLFONT, colorWhite},

	{"Past members:", UI_CENTER | UI_BIGFONT, colorBlack},
	{"                                       'eiM' (DE) - Programmer                                       ", UI_CENTER | UI_SMALLFONT, colorBlack},
	{"                       Ben 'MadMonki' Barham (AU) - Programmer                                       ", UI_CENTER | UI_SMALLFONT, colorRed},
	{"                               'ratty redemption' - Environment Artist, Texture Artist               ", UI_CENTER | UI_SMALLFONT, colorRed},
	{"                   Ryan 'ryguy_1617' Berkani (US) - 3D Props Artist                                  ", UI_CENTER | UI_SMALLFONT, colorBlack},
	{"                                        'leekenn' - Sound effects                                    ", UI_CENTER | UI_SMALLFONT, colorBlack},
	{"                                 'dutchmeat' (NL) - Programmer                                       ", UI_CENTER | UI_SMALLFONT, colorBlack},
	{"                           Patrick Delmastro (AU) - Character Artist                                 ", UI_CENTER | UI_SMALLFONT, colorBlack},
	{"", UI_CENTER | UI_SMALLFONT, colorBlack},

	{"", UI_CENTER | UI_SMALLFONT, colorBlack},
	{"", UI_CENTER | UI_SMALLFONT, colorBlack},
	{"", UI_CENTER | UI_SMALLFONT, colorBlack},
	{"", UI_CENTER | UI_SMALLFONT, colorBlack},

	{"XreaL - http://xreal-project.net/", UI_CENTER | UI_BIGFONT, colorRed},
	{"Robert 'Tr3B' Beckebans", UI_CENTER | UI_SMALLFONT, colorRed},
	{"Pat 'raynorpat' Raynor", UI_CENTER | UI_SMALLFONT, colorBlack},
//  {"Josef 'cnuke' Soentgen", UI_CENTER | UI_SMALLFONT, colorBlack},
	{"Adrian 'otty' Fuhrmann", UI_CENTER | UI_SMALLFONT, colorBlack},
	{"", UI_CENTER | UI_SMALLFONT, colorWhite},

	{"IOQuake 3 - http://www.ioquake3.org/", UI_CENTER | UI_BIGFONT, colorRed},
	{"Tim 'Timbo' Angus", UI_CENTER | UI_SMALLFONT, colorBlack},
	{"Vincent Cojot", UI_CENTER | UI_SMALLFONT, colorBlack},
	{"Ryan C. 'icculus' Gordon", UI_CENTER | UI_SMALLFONT, colorBlack},
	{"Aaron Gyes", UI_CENTER | UI_SMALLFONT, colorBlack},
	{"Ludwig Nussel", UI_CENTER | UI_SMALLFONT, colorBlack},
	{"Julian Priestley", UI_CENTER | UI_SMALLFONT, colorBlack},
	{"Scirocco Six", UI_CENTER | UI_SMALLFONT, colorBlack},
	{"Zachary J. Slater", UI_CENTER | UI_SMALLFONT, colorBlack},
	{"Tony J. White", UI_CENTER | UI_SMALLFONT, colorBlack},
	{"", UI_CENTER | UI_SMALLFONT, colorBlack},

	{"Unlagged - http://www.ra.is/unlagged/", UI_CENTER | UI_BIGFONT, colorRed},
	{"Neil 'haste' Toronto", UI_CENTER | UI_SMALLFONT, colorBlack},
	{"", UI_CENTER | UI_SMALLFONT, colorBlack},

	{"Darkradiant - http://darkradiant.sourceforge.net/", UI_CENTER | UI_BIGFONT, colorRed},
	{"", UI_CENTER | UI_SMALLFONT, colorBlack},

	{"Special Thanks To:", UI_CENTER | UI_BIGFONT, colorMdGrey},
	{"id Software", UI_CENTER | UI_SMALLFONT, colorBlack},
	{"", UI_CENTER | UI_SMALLFONT, colorBlack},

	{"Other Contributors", UI_CENTER | UI_BIGFONT, colorMdGrey},
	{"For a detailed list of contributors see the accompanying;", UI_CENTER | UI_SMALLFONT, colorBlack},
	{"ASSET_LICENSE.txt and XREAL_CONTRIBUTORS.txt", UI_CENTER | UI_SMALLFONT, colorBlack},
	{"", UI_CENTER | UI_SMALLFONT, colorBlack},

	{"Weaver(c) 2008-2011, Weaver Team and Contributors", UI_CENTER | UI_SMALLFONT, colorRed},
	{"XreaL(c) 2005-2009, XreaL Team and Contributors", UI_CENTER | UI_SMALLFONT, colorRed},

	{NULL}
};


/*
=================
UI_CreditMenu_Key
=================
*/
static sfxHandle_t UI_CreditMenu_Key(int key)
{
	if(key & K_CHAR_FLAG)
		return 0;

	// pressing the escape key or clicking the mouse will exit
	// we also reset the music volume to the user's original
	// choice here,  by setting s_musicvolume to the stored var
	trap_Cmd_ExecuteText(EXEC_APPEND, va("s_musicvolume %f; quit\n", mvolume));
	return 0;
}

/*
=================
ScrollingCredits_Draw

Main drawing function
=================
*/
static void ScrollingCredits_Draw(void)
{
	int             x = 320, y, n;
	float           textScale = 0.25f;
	vec4_t          color;
	float           textZoom;

	// first, fill the background with the specified shader
//  UI_DrawHandlePic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BackgroundShader);

	// draw the stuff by setting the initial y location
	y = 480 - SCROLLSPEED * (float)(uis.realtime - starttime) / 100;


	// loop through the entire credits sequence
	for(n = 0; n <= sizeof(credits) - 1; n++)
	{

		// this NULL string marks the end of the credits struct
		if(credits[n].string == NULL)
		{
			/*
			   // credits sequence is completely off screen
			   if(y < -16)
			   {
			   // TODO: bring up XreaL plaque and fade-in and wait for keypress?
			   break;
			   }
			 */
			break;
		}

		if(credits[n].style & UI_GIANTFONT)
			textScale = 0.5f;
		else if(credits[n].style & UI_BIGFONT)
			textScale = 0.35f;
		else
			textScale = 0.2f;

		VectorSet4(color, credits[n].color[0], credits[n].color[1], credits[n].color[2], 0.0f);

		if(y <= 0 || y >= 480)
		{
			color[3] = 0;
		}
		else
		{

			color[3] = sin(M_PI / 480.0f * y);

		}

		textZoom = color[3] * 4 * textScale;

		if(textZoom > textScale)
			textZoom = textScale;

		textScale = textZoom;

		/*
		if(credits[n].style & UI_GIANTFONT)
			UI_DrawRect(0, y - color[3] * 20, 640, 3 + color[3] * 40,
						color_cursorLines);
		else if(credits[n].style & UI_BIGFONT)
			UI_DrawRect(0, y - color[3] * 10, 640, 1 + color[3] * 20,
						color_cursorLines);
		*/


		UI_Text_Paint(x, y, textScale, color, credits[n].string, 0, 0, credits[n].style, &uis.freeSansBoldFont);

		y += SMALLCHAR_HEIGHT + 4;

		/*
		   if(credits[n].style & UI_SMALLFONT)
		   {
		   y += SMALLCHAR_HEIGHT;// * PROP_SMALL_SIZE_SCALE;
		   }
		   else if(credits[n].style & UI_BIGFONT)
		   {
		   y += BIGCHAR_HEIGHT;
		   }
		   else if(credits[n].style & UI_GIANTFONT)
		   {
		   y += GIANTCHAR_HEIGHT;// * (1 / PROP_SMALL_SIZE_SCALE);
		   }
		 */

		// if y is off the screen, break out of loop
		//if(y > 480)
	}

	if(y < 0)
	{
		// repeat the credits
		starttime = uis.realtime;
	}
}

/*
===============
UI_CreditMenu
===============
*/
void UI_CreditMenu(void)
{
	memset(&s_credits, 0, sizeof(s_credits));

	s_credits.menu.draw = ScrollingCredits_Draw;
	s_credits.menu.key = UI_CreditMenu_Key;
	s_credits.menu.fullscreen = qtrue;
	UI_PushMenu(&s_credits.menu);

	starttime = uis.realtime;	// record start time for credits to scroll properly
	mvolume = trap_Cvar_VariableValue("s_musicvolume");
	if(mvolume < 0.5)
		trap_Cmd_ExecuteText(EXEC_APPEND, "s_musicvolume 0.5\n");
	trap_Cmd_ExecuteText(EXEC_APPEND, "music music/credits.ogg\n");

	// load the background shader
//  BackgroundShader = trap_R_RegisterShaderNoMip("menubackcredits");
}
