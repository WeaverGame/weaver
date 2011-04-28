/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>

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
//
//
// ui_team.c
//

#include "ui_local.h"


#define TEAMMAIN_FRAME	"menu/art/cut_frame"

#define ID_JOINRED		100
#define ID_JOINBLUE		101
#define ID_JOINGAME		102
#define ID_SPECTATE		103
#define ID_BACK			104

#define TEAM_VERTICAL_SPACING	42


typedef struct
{
	menuframework_s menu;
	menubitmap_s    back;

	menutext_s      banner;

	menutext_s      joinred;
	menutext_s      joinblue;
	menutext_s      joingame;
	menutext_s      spectate;
} teammain_t;

static teammain_t s_teammain;

/*
===============
TeamMain_MenuEvent
===============
*/
static void TeamMain_MenuEvent(void *ptr, int event)
{
	if(event != QM_ACTIVATED)
	{
		return;
	}

	switch (((menucommon_s *) ptr)->id)
	{
		case ID_JOINRED:
			trap_Cmd_ExecuteText(EXEC_APPEND, "cmd team red\n");
			UI_ForceMenuOff();
			break;

		case ID_JOINBLUE:
			trap_Cmd_ExecuteText(EXEC_APPEND, "cmd team blue\n");
			UI_ForceMenuOff();
			break;

		case ID_JOINGAME:
			trap_Cmd_ExecuteText(EXEC_APPEND, "cmd team free\n");
			UI_ForceMenuOff();
			break;

		case ID_SPECTATE:
			trap_Cmd_ExecuteText(EXEC_APPEND, "cmd team spectator\n");
			UI_ForceMenuOff();
			break;
		case ID_BACK:
			UI_PopMenu();
			break;
	}
}


/*
===============
TeamMain_MenuInit
===============
*/
void TeamMain_MenuInit(void)
{
	int             y;
	int             gametype;
	char            info[MAX_INFO_STRING];

	memset(&s_teammain, 0, sizeof(s_teammain));

	TeamMain_Cache();

	s_teammain.menu.wrapAround = qtrue;
	s_teammain.menu.fullscreen = qfalse;

	s_teammain.banner.generic.type = MTYPE_BTEXT;
	s_teammain.banner.generic.x = uis.screenXSize / 2;
	s_teammain.banner.generic.y = 16;
	s_teammain.banner.string = "Choose Team";
	s_teammain.banner.color = menu_banner_color;
	s_teammain.banner.style = UI_CENTER;



	y = 194;
	s_teammain.joingame.generic.type = MTYPE_PTEXT;
	s_teammain.joingame.generic.flags = QMF_CENTER_JUSTIFY | QMF_PULSEIFFOCUS;
	s_teammain.joingame.generic.id = ID_JOINGAME;
	s_teammain.joingame.generic.callback = TeamMain_MenuEvent;
	s_teammain.joingame.generic.x = uis.screenXSize / 2;
	s_teammain.joingame.generic.y = y;
	s_teammain.joingame.string = "Join";
	s_teammain.joingame.style = UI_CENTER;
	s_teammain.joingame.color = colorBlack;
	y += TEAM_VERTICAL_SPACING;

	s_teammain.joinred.generic.type = MTYPE_PTEXT;
	s_teammain.joinred.generic.flags = QMF_RIGHT_JUSTIFY | QMF_PULSEIFFOCUS;
	s_teammain.joinred.generic.id = ID_JOINRED;
	s_teammain.joinred.generic.callback = TeamMain_MenuEvent;
	s_teammain.joinred.generic.x = (uis.screenXSize / 2) - 100;
	s_teammain.joinred.generic.y = y;
	s_teammain.joinred.string = "Red";
	s_teammain.joinred.style = UI_CENTER | UI_DROPSHADOW;
	s_teammain.joinred.color = color_red;

	s_teammain.joinblue.generic.type = MTYPE_PTEXT;
	s_teammain.joinblue.generic.flags = QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS;
	s_teammain.joinblue.generic.id = ID_JOINBLUE;
	s_teammain.joinblue.generic.callback = TeamMain_MenuEvent;
	s_teammain.joinblue.generic.x = (uis.screenXSize / 2) + 100;
	s_teammain.joinblue.generic.y = y;
	s_teammain.joinblue.string = "Blue";
	s_teammain.joinblue.style = UI_CENTER;
	s_teammain.joinblue.color = color_blue;
	y += TEAM_VERTICAL_SPACING;


	s_teammain.spectate.generic.type = MTYPE_PTEXT;
	s_teammain.spectate.generic.flags = QMF_CENTER_JUSTIFY | QMF_PULSEIFFOCUS;
	s_teammain.spectate.generic.id = ID_SPECTATE;
	s_teammain.spectate.generic.callback = TeamMain_MenuEvent;
	s_teammain.spectate.generic.x = uis.screenXSize / 2;
	s_teammain.spectate.generic.y = y;
	s_teammain.spectate.string = "Spectate";
	s_teammain.spectate.style = UI_CENTER;
	s_teammain.spectate.color = colorBlack;
	y += TEAM_VERTICAL_SPACING;


	s_teammain.back.generic.type = MTYPE_BITMAP;
	s_teammain.back.generic.name = UI_ART_BUTTON;
	s_teammain.back.generic.flags = QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS;
	s_teammain.back.generic.id = ID_BACK;
	s_teammain.back.generic.callback = TeamMain_MenuEvent;
	s_teammain.back.generic.x = 0;
	s_teammain.back.generic.y = uis.screenYSize - 64;
	s_teammain.back.width = 128;
	s_teammain.back.height = 64;
	s_teammain.back.focuspic = UI_ART_BUTTON_FOCUS;
	s_teammain.back.generic.caption.text = "Back";
	s_teammain.back.generic.caption.style = UI_CENTER;
	s_teammain.back.generic.caption.fontsize = 0.6f;
	s_teammain.back.generic.caption.font = &uis.buttonFont;
	s_teammain.back.generic.caption.color = text_color_blackbar;
	s_teammain.back.generic.caption.focuscolor = text_color_highlight;


	trap_GetConfigString(CS_SERVERINFO, info, MAX_INFO_STRING);
	gametype = atoi(Info_ValueForKey(info, "g_gametype"));

	// set initial states
	switch (gametype)
	{
		case GT_SINGLE_PLAYER:
		case GT_FFA:
		case GT_TOURNAMENT:
			s_teammain.joinred.generic.flags |= QMF_GRAYED;
			s_teammain.joinblue.generic.flags |= QMF_GRAYED;
			break;

		case GT_OBJECTIVE:
		case GT_OBJECTIVE_SW:
			break;

		default:
		case GT_TEAM:
		case GT_CTF:
			s_teammain.joingame.generic.flags |= QMF_GRAYED;
			break;
	}

	Menu_AddItem(&s_teammain.menu, (void *)&s_teammain.banner);
	Menu_AddItem(&s_teammain.menu, (void *)&s_teammain.joinred);
	Menu_AddItem(&s_teammain.menu, (void *)&s_teammain.joinblue);
	Menu_AddItem(&s_teammain.menu, (void *)&s_teammain.joingame);
	Menu_AddItem(&s_teammain.menu, (void *)&s_teammain.spectate);
	Menu_AddItem(&s_teammain.menu, (void *)&s_teammain.back);
}


/*
===============
TeamMain_Cache
===============
*/
void TeamMain_Cache(void)
{
	//trap_R_RegisterShaderNoMip(TEAMMAIN_FRAME);
}


/*
===============
UI_TeamMainMenu
===============
*/
void UI_TeamMainMenu(void)
{
	TeamMain_MenuInit();
	UI_PushMenu(&s_teammain.menu);
}
