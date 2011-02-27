/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2011 Andrew Browne <dersaidin@dersaidin.net>

This file is part of Weaver source code.

Weaver source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Weaver source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Weaver source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

// cg_info.c -- display information while data is being loading
#include "cg_local.h"

vec4_t          colorLines = { 0.6f, 0.6f, 0.8f, 0.15f };	// lines color
vec4_t          colorText = { 1.0f, 1.0f, 1.0f, 1.0f };	// text color
vec4_t          colorTextGrey = { 0.7f, 0.7f, 0.7f, 1.0f };	// text color grey
vec4_t          colorProgress = { 0.7f, 0.7f, 1.0f, 0.2f };	// progress color

vec4_t          text_color_disabled = { 0.50f, 0.50f, 0.50f, 0.75f };	// light gray
vec4_t          text_color_normal = { 0.9f, 0.9f, 1.0f, 0.8f };	// light blue/gray
vec4_t          text_color_highlight = { 0.90f, 0.90f, 1.00f, 0.95f };	// bright white

vec4_t          text_color_warning = { 0.90f, 0.10f, 0.10f, 0.75f };	// bright white

qhandle_t       load0 = 0;
qhandle_t       load1 = 0;

qhandle_t       levelshot = 0;
qhandle_t       levelshotDefault = 0;
qhandle_t       menuback = 0;

qhandle_t       black_gradient = 0;
qhandle_t       logo_dark = 0;
qhandle_t       title = 0;
qhandle_t       title_white = 0;

/*
======================
CG_LoadingString
======================
*/
void CG_LoadingString(const char *s, qboolean strong)
{

	Q_strncpyz(cg.progressInfo[cg.progress].info, s, sizeof(cg.progressInfo[cg.progress].info));
	cg.progressInfo[cg.progress].strong = strong;
	cg.progress++;

	if(cg.progress > NUM_PROGRESS - 1)
		cg.progress = NUM_PROGRESS - 1;


#if 1
	//find out how many cg.progress we made...
	Com_Printf("Progress: %i\n", cg.progress);
#endif

	trap_UpdateScreen();
}

/*
======================
CG_DrawProgress
======================
*/
static void CG_DrawProgress(void)
{
	int             x, y;
	int             i;
	vec4_t          color;
	int             style = 0;
	char           *s;
	int             pc_h;

	x = CG_INFO_SIDE_GRADIENT_WIDTH + 4;
	y = 480 - 8;

	if(cg.progress == 0)
	{
	}
	else
	{
		s = va("%i%%", (int)(100 / NUM_PROGRESS * cg.progress));
		pc_h = CG_Text_Height(s, 0.8f, 0, &cgs.media.freeSansFont);
		CG_Text_PaintAligned(cgs.screenXSize - 2, cgs.screenYSize - 2 - pc_h, s, 0.85f, UI_RIGHT,
			(levelshot ? colorText : colorTextGrey), &cgs.media.freeSansFont);
	}

	for(i = 0; i < NUM_PROGRESS; i++)
	{
		CG_DrawPic(x + (i * CG_INFO_LOADTILE_WIDTH), y, 6, 6, load0);
	}

	for(i = 0; i < cg.progress; i++)
	{
		VectorCopy4(colorProgress, color);

		if(i == cg.progress - 1)
		{
			style = UI_DROPSHADOW;
			VectorCopy4(text_color_highlight, color);
		}
		else if(cg.progressInfo[i].strong)
		{
			style = 0;
			color[3] *= 2;
		}

		CG_Text_PaintAligned(10, cgs.screenYSize - 10 - (i * 16), cg.progressInfo[i].info, 0.3f, style, color, &cgs.media.freeSansBoldFont);

		CG_DrawPic(x + (i * CG_INFO_LOADTILE_WIDTH), y, 6, 6, load1);

		/*
		if(i == cg.progress - 1)
		{
			CG_Text_PaintAligned(x + i * 16 + 8, y - 8, cg.progressInfo[i].info, 0.2f, UI_RIGHT | UI_DROPSHADOW,
								 text_color_highlight, &cgs.media.freeSansBoldFont);
		}
		*/
	}


}

/*
====================
CG_DrawInformation

Draw all the status / pacifier stuff during level loading
====================
*/
void CG_DrawInformation(void)
{
	const char     *s = NULL;
	const char     *loadingmap = NULL;
	const char     *info;
	const char     *sysInfo;
	int             x, y;
	int             value;

	char            buf[1024];

	int             y_offset;

	const int       title_h = 64;
	int             loading_h;

	const float     infoScale = 0.35f;

	info = CG_ConfigString(CS_SERVERINFO);
	sysInfo = CG_ConfigString(CS_SYSTEMINFO);

	s = Info_ValueForKey(info, "mapname");

	if(!levelshot)
		levelshot = trap_R_RegisterShaderNoMip(va("levelshots/%s", s));
	if(!levelshotDefault)
		levelshotDefault = trap_R_RegisterShaderNoMip("gfx/menu/logo");

	if(!load0)
		load0 = trap_R_RegisterShaderNoMip("gfx/menu/load0");
	if(!load1)
		load1 = trap_R_RegisterShaderNoMip("gfx/menu/load1");

	if(!title)
		title = trap_R_RegisterShaderNoMip("gfx/menu/title");
	if(!title_white)
		title_white = trap_R_RegisterShaderNoMip("gfx/menu/title_white");
	if(!black_gradient)
		black_gradient = trap_R_RegisterShaderNoMip("gfx/menu/black_gradient");
	if(!logo_dark)
		logo_dark = trap_R_RegisterShaderNoMip("gfx/menu/logo_dark");

	if(!menuback)
		menuback = trap_R_RegisterShaderNoMip("white");

	CG_DrawPic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, menuback);


	//mapshot
	trap_R_SetColor(NULL);
	loadingmap = va("Loading %s", s);
	loading_h = CG_Text_Height(loadingmap, 0.41f, 0, &cgs.media.freeSansFont);
	if(levelshot)
	{
		trap_R_DrawStretchPic(0, 0, cgs.screenXSize, cgs.screenYSize, 0, 0, 1, 1, levelshot);
	}
	else
	{
		trap_R_DrawStretchPic((cgs.screenXSize - CG_INFO_LOGO_WIDTH)/2, cgs.screenYSize/2 - CG_INFO_LOGO_WIDTH, CG_INFO_LOGO_WIDTH, CG_INFO_LOGO_WIDTH*2, 0, 0, 1, 1, levelshotDefault);
	}
	trap_R_DrawStretchPic((CG_INFO_SIDE_GRADIENT_WIDTH * cgs.screenXScale) - 9, cgs.screenYSize - title_h - (8 * cgs.screenYScale), title_h*4, title_h, 0, 0, 1, 1, (levelshot ? title_white : title));
	CG_Text_PaintAligned(cgs.screenXSize - (CG_INFO_PERCENT_WIDTH * cgs.screenXScale) - 2, cgs.screenYSize - (8 * cgs.screenYScale) - loading_h, loadingmap, 0.4f, UI_RIGHT, (levelshot ? colorText : colorTextGrey), &cgs.media.freeSansFont);

	// left side
	trap_R_DrawStretchPic(0, 0, CG_INFO_SIDE_GRADIENT_WIDTH * cgs.screenXScale, cgs.screenYSize, 0, 0, 1, 1, black_gradient);
	trap_R_DrawStretchPic(((CG_INFO_SIDE_GRADIENT_WIDTH * cgs.screenXScale) - CG_INFO_SIDE_LOGO_WIDTH)/2, 0, CG_INFO_SIDE_LOGO_WIDTH, CG_INFO_SIDE_LOGO_WIDTH*2, 0, 0, 1, 1, logo_dark);

	// draw the cg.progress
	CG_DrawProgress();

	// server-specific message of the day
	s = CG_ConfigString(CS_MOTD);
	if(s[0])
	{
		CG_Text_PaintAligned(cgs.screenXSize/2, 110, s, 0.5f, UI_CENTER | UI_DROPSHADOW, text_color_normal, &cgs.media.freeSansFont);
	}

	// draw info string information
	y = 100;
	y_offset = 24;
	x = 6;

	// don't print server lines if playing a local game
	trap_Cvar_VariableStringBuffer("sv_running", buf, sizeof(buf));
	if(!atoi(buf))
	{
		// server hostname
		Q_strncpyz(buf, Info_ValueForKey(info, "sv_hostname"), 1024);
		Q_CleanStr(buf);
		s = va("%s", buf);
		CG_Text_PaintAligned(x, y, s, infoScale, UI_DROPSHADOW, text_color_normal, &cgs.media.freeSansFont);
		y += y_offset;

		// pure server
		s = Info_ValueForKey(sysInfo, "sv_pure");
		if(s[0] == '1')
		{
			CG_Text_PaintAligned(x, y, "Pure Server", infoScale, UI_DROPSHADOW, text_color_normal, &cgs.media.freeSansFont);
			y += y_offset;
		}
	}

	// map-specific message (long map name)
	s = CG_ConfigString(CS_MESSAGE);
	if(s[0])
	{
		CG_Text_PaintAligned(x, y, s, infoScale, UI_DROPSHADOW, text_color_normal, &cgs.media.freeSansFont);
		y += y_offset;
	}

	// cheats warning
	s = Info_ValueForKey(sysInfo, "sv_cheats");
	if(s[0] == '1')
	{
		CG_Text_PaintAligned(x, y, "Cheats enabled", infoScale, UI_DROPSHADOW, text_color_normal, &cgs.media.freeSansFont);

		//CG_Text_PaintAligned(x - 10, y + 1, ">", 0.2f, 0, text_color_warning, &cgs.media.freeSansFont);
		//CG_Text_PaintAligned(x + 110, y + 1, "<", 0.2f, 0, text_color_warning, &cgs.media.freeSansFont);

		y += y_offset;
	}

	// game type
	switch (cgs.gametype)
	{
		case GT_FFA:
			s = "Free For All";
			break;
		case GT_SINGLE_PLAYER:
			s = "Single Player";
			break;
		case GT_TOURNAMENT:
			s = "Tournament";
			break;
		case GT_TEAM:
			s = "Team Deathmatch";
			break;
		case GT_CTF:
			s = "Capture The Flag";
			break;
		case GT_1FCTF:
			s = "One Flag CTF";
			break;
		case GT_OBELISK:
			s = "Overload";
			break;
		case GT_HARVESTER:
			s = "Harvester";
			break;
		case GT_OBJECTIVE:
			s = "Objective";
			break;
		case GT_OBJECTIVE_SW:
			s = "Stopwatch Objective";
			break;
		default:
			s = "Unknown Gametype";
			break;
	}
	CG_Text_PaintAligned(x, y, s, infoScale, UI_DROPSHADOW, text_color_normal, &cgs.media.freeSansFont);
	y += y_offset;

	value = atoi(Info_ValueForKey(info, "timelimit"));
	if(value)
	{

		CG_Text_PaintAligned(x, y, va("Timelimit %i", value), infoScale, UI_DROPSHADOW, text_color_normal,
							 &cgs.media.freeSansFont);
		y += y_offset;
	}

	if(cgs.gametype < GT_CTF)
	{
		value = atoi(Info_ValueForKey(info, "fraglimit"));
		if(value)
		{
			CG_Text_PaintAligned(x, y, va("Fraglimit %i", value), infoScale, UI_DROPSHADOW, text_color_normal,
								 &cgs.media.freeSansFont);

			y += y_offset;
		}
	}
	else if(cgs.gametype >= GT_CTF)
	{
		value = atoi(Info_ValueForKey(info, "capturelimit"));
		if(value)
		{
			CG_Text_PaintAligned(x, y, va("Capturelimit %i", value), infoScale, UI_DROPSHADOW, text_color_normal,
								 &cgs.media.freeSansFont);
			y += y_offset;
		}
	}

}
