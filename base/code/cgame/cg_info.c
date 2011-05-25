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

qhandle_t       blackbar = 0;
qhandle_t       black_gradient = 0;

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


#if 0
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
	int             text_x, text_y;
	int             i;
	vec4_t          color;
	int             style = 0;
	char           *s;
	int             pc_w;
	int             numProgress;
	int             maxProgress;

	x = CG_INFO_LEFT_X;
	text_x = x;

	if(cg.progress == 0)
	{
		s = "Loading Complete!";
	}
	else
	{
		s = va("Loading %i%%", (int)(100 / NUM_PROGRESS * cg.progress));
	}
	CG_Text_PaintAligned(x, cgs.screenYSize - CG_INFO_BLACKBAR_H + 12,
		s, 0.43f, UI_LEFT, colorText, &cgs.media.freeSansFont);

	text_y = cgs.screenYSize - 2 - 10;
	y = text_y - 11 - CG_INFO_DOT_W;

	maxProgress = (cgs.screenXSize - CG_INFO_LEFT_X - CG_INFO_RIGHT_X) / CG_INFO_LOADTILE_WIDTH;
	numProgress = ((float)cg.progress / (float)NUM_PROGRESS) * maxProgress;

	// Loading Dots
	for(i = 0; i < maxProgress; i++)
	{
		if((i <= numProgress) || (cg.progress == 0))
		{
			// Full dot
			trap_R_DrawStretchPic(x + (i * CG_INFO_LOADTILE_WIDTH), y, CG_INFO_DOT_W, CG_INFO_DOT_W, 0, 0, 1, 1, load1);
		}
		else
		{
			// Empty dot
			trap_R_DrawStretchPic(x + (i * CG_INFO_LOADTILE_WIDTH), y, CG_INFO_DOT_W, CG_INFO_DOT_W, 0, 0, 1, 1, load0);
		}
	}

	// Loading Descriptions
	for(i = 0; i < cg.progress; i++)
	{
		Vector4Copy(colorProgress, color);

		if(i == cg.progress - 1)
		{
			style = UI_DROPSHADOW;
			Vector4Copy(text_color_highlight, color);
		}
		else if(cg.progressInfo[i].strong)
		{
			style = 0;
			color[3] *= 2;
		}

		pc_w = CG_Text_Width(cg.progressInfo[i].info, 0.28f, 0, &cgs.media.freeSansFont);
		if((text_x + pc_w) < (cgs.screenXSize - CG_INFO_RIGHT_X))
		{
			// Print CG info
			CG_Text_PaintAligned(text_x, text_y, cg.progressInfo[i].info, 0.28f, style, colorTextGrey, &cgs.media.freeSansFont);
			text_x += pc_w;
			if(cg.progressInfo[i].info[0] != '\0')
			{
				// Pad it a bit, unless its empty string.
				text_x += 5;
			}
		}
		else
		{
			// Loading information is too long for this screen resolution.
			// +5 cause this font has . vertically centered?
			CG_Text_PaintAligned(text_x, text_y + 5, "...", 0.28f, style, colorTextGrey, &cgs.media.freeSansFont);
			break;
		}
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
	const char     *info;
	const char     *sysInfo;
	int             x, y;
	int             value;
	int             pc_h;

	char            buf[1024];

	int             y_offset;

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

	if(!black_gradient)
		black_gradient = trap_R_RegisterShaderNoMip("gfx/menu/black_gradient");
	if(!blackbar)
		blackbar = trap_R_RegisterShaderNoMip("gfx/menu/black_bar");

	if(!menuback)
		menuback = trap_R_RegisterShaderNoMip("white");

	trap_R_DrawStretchPic(0, 0, cgs.screenXSize, cgs.screenYSize, 0, 0, 1, 1, menuback);

	//mapshot
	trap_R_SetColor(NULL);
	if(levelshot)
	{
		trap_R_DrawStretchPic(0, 0, cgs.screenXSize, cgs.screenYSize, 0, 0, 1, 1, levelshot);
	}
	else
	{
		trap_R_DrawStretchPic((cgs.screenXSize - CG_INFO_LOGO_WIDTH)/2, cgs.screenYSize/2 - CG_INFO_LOGO_WIDTH, CG_INFO_LOGO_WIDTH, CG_INFO_LOGO_WIDTH*2, 0, 0, 1, 1, levelshotDefault);
	}
	trap_R_DrawStretchPic(0, cgs.screenYSize - CG_INFO_BLACKBAR_H, cgs.screenXSize, CG_INFO_BLACKBAR_H, 0, 0, 1, 1, blackbar);

	pc_h = CG_Text_Height(s, 0.8f, 0, &cgs.media.freeSansFont);
	CG_Text_PaintAligned(cgs.screenXSize - 2, cgs.screenYSize - 2 - pc_h, s, 0.85f, UI_RIGHT,
		colorText, &cgs.media.freeSansFont);


	// left side
	//trap_R_DrawStretchPic(0, 0, CG_INFO_SIDE_GRADIENT_WIDTH * cgs.screenXScale, cgs.screenYSize, 0, 0, 1, 1, black_gradient);

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
