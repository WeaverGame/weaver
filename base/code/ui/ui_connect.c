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
#include "ui_local.h"

/*
===============================================================================

CONNECTION SCREEN

===============================================================================
*/

qboolean        passwordNeeded = qtrue;
menufield_s     passwordField;

static connstate_t lastConnState;
static char     lastLoadingText[MAX_INFO_VALUE];

static void UI_ReadableSize(char *buf, int bufsize, int value)
{
	if(value > 1024 * 1024 * 1024)
	{							// gigs
		Com_sprintf(buf, bufsize, "%d", value / (1024 * 1024 * 1024));
		Com_sprintf(buf + strlen(buf), bufsize - strlen(buf), ".%02d GB",
					(value % (1024 * 1024 * 1024)) * 100 / (1024 * 1024 * 1024));
	}
	else if(value > 1024 * 1024)
	{							// megs
		Com_sprintf(buf, bufsize, "%d", value / (1024 * 1024));
		Com_sprintf(buf + strlen(buf), bufsize - strlen(buf), ".%02d MB", (value % (1024 * 1024)) * 100 / (1024 * 1024));
	}
	else if(value > 1024)
	{							// kilos
		Com_sprintf(buf, bufsize, "%d KB", value / 1024);
	}
	else
	{							// bytes
		Com_sprintf(buf, bufsize, "%d bytes", value);
	}
}

// Assumes time is in msec
static void UI_PrintTime(char *buf, int bufsize, int time)
{
	time /= 1000;				// change to seconds

	if(time > 3600)
	{							// in the hours range
		Com_sprintf(buf, bufsize, "%d hr %d min", time / 3600, (time % 3600) / 60);
	}
	else if(time > 60)
	{							// mins
		Com_sprintf(buf, bufsize, "%d min %d sec", time / 60, time % 60);
	}
	else
	{							// secs
		Com_sprintf(buf, bufsize, "%d sec", time);
	}
}

#define DIX1 192
#define DIXO 32

static void UI_DisplayDownloadInfo(const char *downloadName)
{
	static char     dlText[] = "Downloading:";
	static char     etaText[] = "Estimated time left:";
	static char     xferText[] = "Transfer rate:";

	int             downloadSize, downloadCount, downloadTime;
	char            dlSizeBuf[64], totalSizeBuf[64], xferRateBuf[64], dlTimeBuf[64];
	int             xferRate;
	int             width, leftWidth;
	int             style = UI_LEFT | UI_SMALLFONT;
	const char     *s;

	downloadSize = trap_Cvar_VariableValue("cl_downloadSize");
	downloadCount = trap_Cvar_VariableValue("cl_downloadCount");
	downloadTime = trap_Cvar_VariableValue("cl_downloadTime");

#if 0							// bk010104
	fprintf(stderr, "\n\n-----------------------------------------------\n");
	fprintf(stderr, "DB: downloadSize:  %16d\n", downloadSize);
	fprintf(stderr, "DB: downloadCount: %16d\n", downloadCount);
	fprintf(stderr, "DB: downloadTime:  %16d\n", downloadTime);
	fprintf(stderr, "DB: UI realtime:   %16d\n", uis.realtime);	// bk
	fprintf(stderr, "DB: UI frametime:  %16d\n", uis.frametime);	// bk
#endif

	leftWidth = width = UI_ProportionalStringWidth(dlText) * UI_ProportionalSizeScale(style);
	width = UI_ProportionalStringWidth(etaText) * UI_ProportionalSizeScale(style);
	if(width > leftWidth)
		leftWidth = width;
	width = UI_ProportionalStringWidth(xferText) * UI_ProportionalSizeScale(style);
	if(width > leftWidth)
		leftWidth = width;
		
	leftWidth -= 64;

	UI_Text_Paint_AutoWrapped(14, DIX1, 0.3f, 600, dlText, style, color_white, &uis.freeSansFont);
	UI_Text_Paint_AutoWrapped(14, DIX1+DIXO, 0.3f, 600, etaText, style, color_white, &uis.freeSansFont);
	UI_Text_Paint_AutoWrapped(14, DIX1+2*DIXO, 0.3f, 600, xferText, style, color_white, &uis.freeSansFont);
	/*							  
	UI_DrawProportionalString(8, 128, dlText, style, color_white);
	UI_DrawProportionalString(8, 160, etaText, style, color_white);
	UI_DrawProportionalString(8, 224, xferText, style, color_white);
	*/
	if(downloadSize > 0)
	{
		s = va("%s (%d%%)", downloadName, (int)((float)downloadCount * 100.0f / downloadSize));
	}
	else
	{
		s = downloadName;
	}

	//UI_DrawProportionalString(leftWidth, 128, s, style, color_white);

	UI_Text_Paint_AutoWrapped(leftWidth, DIX1, 0.3f, 600, s, style, color_white, &uis.freeSansFont);


	UI_ReadableSize(dlSizeBuf, sizeof dlSizeBuf, downloadCount);
	UI_ReadableSize(totalSizeBuf, sizeof totalSizeBuf, downloadSize);

	if(downloadCount < 4096 || !downloadTime)
	{
		//UI_DrawProportionalString(leftWidth, 160, "estimating", style, color_white);
		//UI_DrawProportionalString(leftWidth, 192, va("(%s of %s copied)", dlSizeBuf, totalSizeBuf), style, color_white);

		UI_Text_Paint_AutoWrapped(leftWidth, DIX1+DIXO, 0.3f, 600, "estimating", style, color_white, &uis.freeSansFont);
		UI_Text_Paint_AutoWrapped(leftWidth, DIX1+DIXO, 0.3f, 600, va("(%s of %s copied)", dlSizeBuf, totalSizeBuf), style, color_white, &uis.freeSansFont);

	}
	else
	{
		// bk010108
		//float elapsedTime = (float)(uis.realtime - downloadTime); // current - start (msecs)
		//elapsedTime = elapsedTime * 0.001f; // in seconds
		//if ( elapsedTime <= 0.0f ) elapsedTime == 0.0f;
		if((uis.realtime - downloadTime) / 1000)
		{
			xferRate = downloadCount / ((uis.realtime - downloadTime) / 1000);
			//xferRate = (int)( ((float)downloadCount) / elapsedTime);
		}
		else
		{
			xferRate = 0;
		}

		//fprintf( stderr, "DB: elapsedTime:  %16.8f\n", elapsedTime );   // bk
		//fprintf( stderr, "DB: xferRate:   %16d\n", xferRate );  // bk

		UI_ReadableSize(xferRateBuf, sizeof xferRateBuf, xferRate);

		// Extrapolate estimated completion time
		if(downloadSize && xferRate)
		{
			int             n = downloadSize / xferRate;	// estimated time for entire d/l in secs

			// We do it in K (/1024) because we'd overflow around 4MB
			n = (n - (((downloadCount / 1024) * n) / (downloadSize / 1024))) * 1000;

			UI_PrintTime(dlTimeBuf, sizeof dlTimeBuf, n);	// bk010104
			//(n - (((downloadCount/1024) * n) / (downloadSize/1024))) * 1000);

			//UI_DrawProportionalString(leftWidth, 160, dlTimeBuf, style, color_white);
			//UI_DrawProportionalString(leftWidth, 192, va("(%s of %s copied)", dlSizeBuf, totalSizeBuf), style, color_white);

			UI_Text_Paint_AutoWrapped(leftWidth, DIX1+DIXO, 0.3f, 600, dlTimeBuf, style, color_white, &uis.freeSansFont);
			UI_Text_Paint_AutoWrapped(leftWidth, DIX1+2*DIXO, 0.3f, 600, va("(%s of %s copied)", dlSizeBuf, totalSizeBuf), style, color_white, &uis.freeSansFont);

		}
		else
		{
			//UI_DrawProportionalString(leftWidth, 160, "estimating", style, color_white);
			UI_Text_Paint_AutoWrapped(leftWidth, DIX1+DIXO, 0.3f, 600, "estimating", style, color_white, &uis.freeSansFont);
	
			if(downloadSize)
			{
			//	UI_DrawProportionalString(leftWidth, 192, va("(%s of %s copied)", dlSizeBuf, totalSizeBuf), style, color_white);
				UI_Text_Paint_AutoWrapped(leftWidth, DIX1+2*DIXO, 0.3f, 600, va("(%s of %s copied)", dlSizeBuf, totalSizeBuf), style, color_white, &uis.freeSansFont);

			}
			else
			{
			//	UI_DrawProportionalString(leftWidth, 192, va("(%s copied)", dlSizeBuf), style, color_white);
				UI_Text_Paint_AutoWrapped(leftWidth, DIX1+2*DIXO, 0.3f, 600, va("(%s copied)", dlSizeBuf), style, color_white, &uis.freeSansFont);

			}
		}

		if(xferRate)
		{
			//UI_DrawProportionalString(leftWidth, 224, va("%s/Sec", xferRateBuf), style, color_white);
			UI_Text_Paint_AutoWrapped(leftWidth, DIX1+3*DIXO, 0.3f, 600, va("(%s /Sec)", xferRateBuf), style, color_white, &uis.freeSansFont);
		}
	}
}

/*
========================
UI_DrawConnectScreen

This will also be overlaid on the cgame info screen during loading
to prevent it from blinking away too rapidly on local or lan games.
========================
*/
void UI_DrawConnectScreen(qboolean overlay)
{
	char           *s;
	uiClientState_t cstate;
	char            info[MAX_INFO_VALUE];

	Menu_Cache();

	if(!overlay)
	{
		// draw the dialog background
		UI_SetColor(color_white);
		UI_DrawHandlePic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, uis.whiteBack);
	}

	// see what information we should display
	trap_GetClientState(&cstate);

	/*
	// now drawn in cg_info.c
	info[0] = '\0';
	if(trap_GetConfigString(CS_SERVERINFO, info, sizeof(info)))
	{
		s = va("Loading %s", Info_ValueForKey(info, "mapname"));
		//50 = CG_INFO_PERCENT_WIDTH (cg_local.h)
		UI_Text_Paint(SCREEN_WIDTH - 50, SCREEN_HEIGHT - 15, 0.2f, color_white, s, 0, 0, UI_RIGHT, &uis.TextFont);
	}
	*/

	// display global MOTD at bottom
	//UI_DrawProportionalString(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 32,                             Info_ValueForKey(cstate.updateInfoString, "motd"), UI_CENTER | UI_SMALLFONT | UI_DROPSHADOW,                            menu_text_color);

	// print any server info (server full, bad version, etc)
	if(cstate.connState < CA_CONNECTED)
	{
		UI_DrawProportionalString_AutoWrapped(320, 192, 630, 20, cstate.messageString, UI_CENTER | UI_SMALLFONT | UI_DROPSHADOW,
											  menu_text_color);

		s = va("%s", cstate.messageString);
		UI_Text_Paint(320, 60, 0.25f, color_red, s, 0, 0, UI_CENTER, &uis.BTextFont);
	}
	else
	{
		//110 = CG_INFO_SIDE_GRADIENT_WIDTH (cg_local.h)
		s = va("Connecting to %s", cstate.servername);
		UI_Text_Paint(10, 10, 0.2f, color_white, s, 0, 0, UI_LEFT, &uis.TextFont);
	}

	if(lastConnState > cstate.connState)
	{
		lastLoadingText[0] = '\0';
	}
	lastConnState = cstate.connState;

	switch (cstate.connState)
	{
		case CA_CONNECTING:
			s = va("Awaiting challenge...%i", cstate.connectPacketCount);
			break;
		case CA_CHALLENGING:
			s = va("Awaiting connection...%i", cstate.connectPacketCount);
			break;
		case CA_CONNECTED:
		{
			char            downloadName[MAX_INFO_VALUE];

			trap_Cvar_VariableStringBuffer("cl_downloadName", downloadName, sizeof(downloadName));
			if(*downloadName)
			{
				UI_DisplayDownloadInfo(downloadName);
				return;
			}
		}
			s = "Awaiting gamestate...";
			break;
		case CA_LOADING:
			return;
		case CA_PRIMED:
			return;
		default:
			return;
	}

	UI_Text_Paint(10, 25, 0.2f, color_white, s, 0, 0, UI_LEFT, &uis.TextFont);
}


/*
===================
UI_KeyConnect
===================
*/
void UI_KeyConnect(int key)
{
	if(key == K_ESCAPE)
	{
		trap_Cmd_ExecuteText(EXEC_APPEND, "disconnect\n");
		return;
	}
}
