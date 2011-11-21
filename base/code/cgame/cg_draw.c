/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2006 Josef Soentgen <cnuke@users.sourceforge.net>
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
//
// cg_draw.c -- draw all of the graphical elements during
// active (after loading) gameplay

#include "cg_local.h"

vec4_t          redTeamColor = { 1.0f, 0.0f, 0.1f, 0.95f };
vec4_t          blueTeamColor = { 0.1f, 0.0f, 1.0f, 0.95f };
vec4_t          baseTeamColor = { 1.0f, 1.0f, 1.0f, 0.80f };




int             drawTeamOverlayModificationCount = -1;

int             sortedTeamPlayers[TEAM_MAXOVERLAY];
int             numSortedTeamPlayers;

char            systemChat[256];
char            teamChat1[256];
char            teamChat2[256];

int CG_Text_Width(const char *text, float scale, int limit, const fontInfo_t * font)
{
	int             count, len;
	float           out;
	const glyphInfo_t *glyph;
	float           useScale;

// FIXME: see ui_main.c, same problem
//  const unsigned char *s = text;
	const char     *s = text;

	useScale = scale * font->glyphScale;
	out = 0;
	if(text)
	{
		len = strlen(text);
		if(limit > 0 && len > limit)
		{
			len = limit;
		}
		count = 0;
		while(s && *s && count < len)
		{
			if(Q_IsColorString(s))
			{
				s += 2;
				continue;
			}
			else
			{
				glyph = &font->glyphs[(int)*s];
				out += glyph->xSkip;
				s++;
				count++;
			}
		}
	}

	return out * useScale;
}

int CG_Text_Height(const char *text, float scale, int limit, const fontInfo_t * font)
{
	int             len, count;
	float           max;
	const glyphInfo_t *glyph;
	float           useScale;

// TTimo: FIXME
//  const unsigned char *s = text;
	const char     *s = text;

	useScale = scale * font->glyphScale;
	max = 0;
	if(text)
	{
		len = strlen(text);
		if(limit > 0 && len > limit)
		{
			len = limit;
		}
		count = 0;
		while(s && *s && count < len)
		{
			if(Q_IsColorString(s))
			{
				s += 2;
				continue;
			}
			else
			{
				glyph = &font->glyphs[(int)*s];
				if(max < glyph->height)
				{
					max = glyph->height;
				}
				s++;
				count++;
			}
		}
	}

	return max * useScale;
}

void CG_Text_PaintChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2,
					   qhandle_t hShader)
{
	float           w, h;

	w = width * scale;
	h = height * scale;

	trap_R_DrawStretchPic(x, y, w, h, s, t, s2, t2, hShader);
}

void CG_Text_Paint(float x, float y, float scale, const vec4_t color, const char *text, float adjust, int limit, int style,
				   const fontInfo_t * font)
{
	int             len, count;
	vec4_t          newColor;
	const glyphInfo_t *glyph;
	float           useScale;

	useScale = scale * font->glyphScale;
	if(text)
	{
// TTimo: FIXME
//      const unsigned char *s = text;
		const char     *s = text;

		trap_R_SetColor(color);
		memcpy(&newColor[0], &color[0], sizeof(vec4_t));
		len = strlen(text);
		if(limit > 0 && len > limit)
		{
			len = limit;
		}
		count = 0;
		while(s && *s && count < len)
		{
			glyph = &font->glyphs[(int)*s];

			if(Q_IsColorString(s))
			{
				memcpy(newColor, (float *)g_color_table[ColorIndex(*(s + 1))], sizeof(newColor));
				newColor[3] = color[3];
				trap_R_SetColor(newColor);
				s += 2;
				continue;
			}
			else
			{
				float           yadj = useScale * glyph->top;

				if(style & UI_DROPSHADOW)	// || style == ITEM_TEXTSTYLE_SHADOWEDMORE)
				{
					int             ofs = 1;	//style == ITEM_TEXTSTYLE_SHADOWED ? 1 : 2;

					colorBlack[3] = newColor[3];
					trap_R_SetColor(colorBlack);
					CG_Text_PaintChar(x + ofs, y - yadj + ofs,
									  glyph->imageWidth,
									  glyph->imageHeight, useScale, glyph->s, glyph->t, glyph->s2, glyph->t2, glyph->glyph);
					colorBlack[3] = 1.0;
					trap_R_SetColor(newColor);
				}
				CG_Text_PaintChar(x, y - yadj,
								  glyph->imageWidth,
								  glyph->imageHeight, useScale, glyph->s, glyph->t, glyph->s2, glyph->t2, glyph->glyph);

				x += (glyph->xSkip * useScale) + adjust;
				s++;
				count++;
			}
		}
		trap_R_SetColor(NULL);
	}
}

void CG_Text_PaintAligned(int x, int y, const char *s, float scale, int style, const vec4_t color, const fontInfo_t * font)
{
	int             w, h;

	w = CG_Text_Width(s, scale, 0, font);
	h = CG_Text_Height(s, scale, 0, font);

	if(style & UI_CENTER)
	{
		CG_Text_Paint(x - w / 2, y + h / 2, scale, color, s, 0, 0, style, font);
	}
	else if(style & UI_RIGHT)
	{
		CG_Text_Paint(x - w, y + h / 2, scale, color, s, 0, 0, style, font);
	}
	else
	{
		// UI_LEFT
		CG_Text_Paint(x, y + h / 2, scale, color, s, 0, 0, style, font);
	}
}

void CG_Text_PaintAlignedBlock(int x, int y, const char *s, float scale, int style, const vec4_t color, const fontInfo_t * font)
{
	char           *lines[32];
	unsigned int    lineNum = 0;
	char           *p;
	int             h;
	int             i;

	if(s == NULL)
		return;

	h = CG_Text_Height(s, scale, 0, font);
	h = h + (h/2); // Add some spacing.

	// Find new lines, separate strings reference each line
	lines[lineNum] = s;
	p = s;
	do
	{
		// Check for end of string.
		if(*p == '\0') break;

		// Check for new line
		if(*p == '\n')
		{
			// End line as a complete string
			*p = '\0';
			lineNum++;
			p++;
			lines[lineNum] = p;
			continue;
		}

		p++;
	} while(p < (s + 1024));
	// Should terminate on '\0', but limit the loop anyway.

	// Number of lines is just above the last index
	lineNum++;

	// Draw each line
	for(i = 0; i < lineNum; i++)
	{
		if(*(lines[i]) != '\0')
			CG_Text_PaintAligned(x, y, lines[i], scale, style, color, font);
		y += h;
	}
}

/*
================
CG_DrawTeamBackground

================
*/
void CG_DrawTeamBackground(int x, int y, int w, int h, float alpha, int team)
{
	vec4_t          hcolor;

	hcolor[3] = alpha;
	if(team == TEAM_RED)
	{
		hcolor[0] = 1;
		hcolor[1] = 0;
		hcolor[2] = 0;
	}
	else if(team == TEAM_BLUE)
	{
		hcolor[0] = 0;
		hcolor[1] = 0;
		hcolor[2] = 1;
	}
	else
	{
		return;
	}
	trap_R_SetColor(hcolor);
	CG_DrawPic(x, y, w, h, cgs.media.teamStatusBar);
	trap_R_SetColor(NULL);
}

/*
==================
CG_DrawSnapshot
==================
*/
static float CG_DrawSnapshot(float y)
{
	char           *s;

	s = va("time:%i snap:%i cmd:%i", cg.snap->serverTime, cg.latestSnapshotNum, cgs.serverCommandSequence);

	CG_Text_PaintAligned(635, y + 4, s, 0.2f, UI_RIGHT | UI_DROPSHADOW, colorWhite, &cgs.media.freeSansBoldFont);


	return y + 16;
}

/*
==================
CG_DrawFPS
==================
*/
#define	FPS_FRAMES 40
static float CG_DrawFPS(float y)
{
	char           *s;
	static int      previousTimes[FPS_FRAMES];
	static int      index;
	int             i, total;
	int             fps;
	static int      previous;
	int             t, frameTime;

	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	t = trap_Milliseconds();
	frameTime = t - previous;
	previous = t;

	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;

	if(index > FPS_FRAMES)
	{
		// average multiple frames together to smooth changes out a bit
		total = 0;
		for(i = 0; i < FPS_FRAMES; i++)
		{
			total += previousTimes[i];
		}
		if(!total)
		{
			total = 1;
		}
		fps = 1000 * FPS_FRAMES / total;

		s = va("%ifps", fps);


		CG_Text_PaintAligned(cgs.screenXSize - 5, y, s, 0.5f, UI_RIGHT | UI_DROPSHADOW, colorWhite, &cgs.media.freeSansBoldFont);
	}

	return y + 16;
}

/*
=================
CG_DrawSpawnTimer
=================
*/
static float CG_DrawSpawnTimer(float y)
{
	char           *time;

	if(cg.teamSpawnPrevious == 0)
	{
		return y;
	}

	time = va("%d", ((cg.teamSpawnPeriod / 1000) - (((cg.time - cg.teamSpawnPrevious) % cg.teamSpawnPeriod) / 1000)));
	CG_Text_PaintAligned(cgs.screenXSize - 5, y, time, 0.70f, UI_RIGHT, colorWhite, &cgs.media.freeSansBoldFont);

	return y + 28;
}

/*
=================
CG_DrawTimer
=================
*/
static float CG_DrawTimer(float y)
{
	char           *s;
	int             mins, seconds, tens;
	int             msec;

	if(cg.warmup == 0)
	{
		msec = (cgs.timelimit * 60.0f * 1000.0f) - (cg.time - cgs.levelStartTime);

		seconds = msec / 1000;
		mins = seconds / 60;
		seconds -= mins * 60;
		tens = seconds / 10;
		seconds -= tens * 10;

		s = va("%i:%i%i", mins, tens, seconds);
	}
	else
	{
		s = va("WARMUP");
	}
	//w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;

	//CG_DrawBigString(635 - w, y + 2, s, 1.0F);
	CG_Text_PaintAligned(cgs.screenXSize - 5, y, s, 0.36f, UI_RIGHT | UI_DROPSHADOW, colorWhite, &cgs.media.freeSansBoldFont);

	return y + 20;

}


/*
=================
CG_DrawTeamOverlay
=================
*/
static float CG_DrawTeamOverlay(float y, qboolean right, qboolean upper)
{
	float           x, w, h, xx;
	int             i, j, len;
	const char     *p;
	vec4_t          hcolor;
	int             pwidth, lwidth;
	int             plyrs;
	char            st[16];
	clientInfo_t   *ci;
	gitem_t        *item;
	int             ret_y, count;

	if(!cg_drawTeamOverlay.integer)
	{
		return y;
	}

	if(cg.snap->ps.persistant[PERS_TEAM] != TEAM_RED && cg.snap->ps.persistant[PERS_TEAM] != TEAM_BLUE)
	{
		return y;				// Not on any team
	}

	plyrs = 0;

	// max player name width
	pwidth = 0;
	count = (numSortedTeamPlayers > 8) ? 8 : numSortedTeamPlayers;
	for(i = 0; i < count; i++)
	{
		ci = cgs.clientinfo + sortedTeamPlayers[i];
		if(ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM])
		{
			plyrs++;
			len = CG_DrawStrlen(ci->name);
			if(len > pwidth)
				pwidth = len;
		}
	}

	if(!plyrs)
		return y;

	if(pwidth > TEAM_OVERLAY_MAXNAME_WIDTH)
		pwidth = TEAM_OVERLAY_MAXNAME_WIDTH;

	// max location name width
	lwidth = 0;
	for(i = 1; i < MAX_LOCATIONS; i++)
	{
		p = CG_ConfigString(CS_LOCATIONS + i);
		if(p && *p)
		{
			len = CG_DrawStrlen(p);
			if(len > lwidth)
				lwidth = len;
		}
	}

	if(lwidth > TEAM_OVERLAY_MAXLOCATION_WIDTH)
		lwidth = TEAM_OVERLAY_MAXLOCATION_WIDTH;

	w = (pwidth + lwidth + 4 + 7) * TINYCHAR_WIDTH;

	if(right)
		x = 640 - w;
	else
		x = 0;

	h = plyrs * TINYCHAR_HEIGHT;

	if(upper)
	{
		ret_y = y + h;
	}
	else
	{
		y -= h;
		ret_y = y;
	}

	CG_AdjustFrom640(&x, &y, &w, &h);

	if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED)
	{
		hcolor[0] = 1.0f;
		hcolor[1] = 0.0f;
		hcolor[2] = 0.0f;
		hcolor[3] = 0.33f;
	}
	else
	{							// if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE )
		hcolor[0] = 0.0f;
		hcolor[1] = 0.0f;
		hcolor[2] = 1.0f;
		hcolor[3] = 0.33f;
	}
	trap_R_SetColor(hcolor);
	CG_DrawPic(x, y, w, h, cgs.media.teamStatusBar);
	trap_R_SetColor(NULL);

	for(i = 0; i < count; i++)
	{
		ci = cgs.clientinfo + sortedTeamPlayers[i];
		if(ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM])
		{

			hcolor[0] = hcolor[1] = hcolor[2] = hcolor[3] = 1.0;

			xx = x + TINYCHAR_WIDTH;

			CG_DrawStringExt(xx, y,
							 ci->name, hcolor, qfalse, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, TEAM_OVERLAY_MAXNAME_WIDTH);

			if(lwidth)
			{
				p = CG_ConfigString(CS_LOCATIONS + ci->location);
				if(!p || !*p)
					p = "unknown";
				len = CG_DrawStrlen(p);
				if(len > lwidth)
					len = lwidth;

				xx = x + TINYCHAR_WIDTH * 2 + TINYCHAR_WIDTH * pwidth;
				CG_DrawStringExt(xx, y,
								 p, hcolor, qfalse, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, TEAM_OVERLAY_MAXLOCATION_WIDTH);
			}

			CG_GetColorForHealth(ci->health, ci->armor, hcolor);

			Com_sprintf(st, sizeof(st), "%3i %3i", ci->health, ci->armor);

			xx = x + TINYCHAR_WIDTH * 3 + TINYCHAR_WIDTH * pwidth + TINYCHAR_WIDTH * lwidth;

			CG_DrawStringExt(xx, y, st, hcolor, qfalse, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0);

			// draw weapon icon
			xx += TINYCHAR_WIDTH * 3;

			if(cg_weapons[ci->curWeapon].weaponIcon)
			{
				CG_DrawPic(xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, cg_weapons[ci->curWeapon].weaponIcon);
			}
			else
			{
				CG_DrawPic(xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, cgs.media.deferShader);
			}

			// Draw powerup icons
			if(right)
			{
				xx = x;
			}
			else
			{
				xx = x + w - TINYCHAR_WIDTH;
			}
			for(j = 0; j <= PW_NUM_POWERUPS; j++)
			{
				if(ci->powerups & (1 << j))
				{

					item = BG_FindItemForPowerup(j);

					if(item)
					{
						CG_DrawPic(xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, trap_R_RegisterShader(item->icon));
						if(right)
						{
							xx -= TINYCHAR_WIDTH;
						}
						else
						{
							xx += TINYCHAR_WIDTH;
						}
					}
				}
			}

			y += TINYCHAR_HEIGHT;
		}
	}

	return ret_y;
}


/*
=====================
CG_DrawUpperRight

=====================
*/

static void CG_DrawUpperRight(void)
{
	float           y;

	y = 10;

	if(cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 1)
	{
		y = CG_DrawTeamOverlay(y, qtrue, qtrue);
	}
	if(cg_drawSnapshot.integer)
	{
		y = CG_DrawSnapshot(y);
	}
	if(cg_drawFPS.integer)
	{
		y = CG_DrawFPS(y);
	}
}

static void CG_DrawTimers(void)
{
	float           y;

	y = cgs.screenYSize / 2;
	y = (y < (cgs.screenYSize - 512)) ? y : (cgs.screenYSize - 512);

	y = CG_DrawSpawnTimer(y);
	if(cg_drawTimer.integer)
	{
		y = CG_DrawTimer(y);
	}
}


//===========================================================================================

/*
=================
CG_DrawTeamInfo
=================
*/
#if 0
static void CG_DrawTeamInfo(void)
{
	int             w, h;
	int             i, len;
	vec4_t          hcolor;
	int             chatHeight;

#define CHATLOC_Y 420			// bottom end
#define CHATLOC_X 0

	if(cg_teamChatHeight.integer < TEAMCHAT_HEIGHT)
		chatHeight = cg_teamChatHeight.integer;
	else
		chatHeight = TEAMCHAT_HEIGHT;
	if(chatHeight <= 0)
		return;					// disabled

	if(cgs.teamLastChatPos != cgs.teamChatPos)
	{
		if(cg.time - cgs.teamChatMsgTimes[cgs.teamLastChatPos % chatHeight] > cg_teamChatTime.integer)
		{
			cgs.teamLastChatPos++;
		}

		h = (cgs.teamChatPos - cgs.teamLastChatPos) * TINYCHAR_HEIGHT;

		w = 0;

		for(i = cgs.teamLastChatPos; i < cgs.teamChatPos; i++)
		{
			len = CG_DrawStrlen(cgs.teamChatMsgs[i % chatHeight]);
			if(len > w)
				w = len;
		}
		w *= TINYCHAR_WIDTH;
		w += TINYCHAR_WIDTH * 2;

		if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED)
		{
			hcolor[0] = 1.0f;
			hcolor[1] = 0.0f;
			hcolor[2] = 0.0f;
			hcolor[3] = 0.33f;
		}
		else if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE)
		{
			hcolor[0] = 0.0f;
			hcolor[1] = 0.0f;
			hcolor[2] = 1.0f;
			hcolor[3] = 0.33f;
		}
		else
		{
			hcolor[0] = 0.0f;
			hcolor[1] = 1.0f;
			hcolor[2] = 0.0f;
			hcolor[3] = 0.33f;
		}

		trap_R_SetColor(hcolor);
		CG_DrawPic(CHATLOC_X, CHATLOC_Y - h, 640, h, cgs.media.teamStatusBar);
		trap_R_SetColor(NULL);

		hcolor[0] = hcolor[1] = hcolor[2] = 1.0f;
		hcolor[3] = 1.0f;

		for(i = cgs.teamChatPos - 1; i >= cgs.teamLastChatPos; i--)
		{
			CG_DrawStringExt(CHATLOC_X + TINYCHAR_WIDTH,
							 CHATLOC_Y - (cgs.teamChatPos - i) * TINYCHAR_HEIGHT,
							 cgs.teamChatMsgs[i % chatHeight], hcolor, qfalse, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0);
		}
	}
}
#endif

/*
===============================================================================

LAGOMETER

===============================================================================
*/

#define	LAG_SAMPLES		128


typedef struct
{
	int             frameSamples[LAG_SAMPLES];
	int             frameCount;
	int             snapshotFlags[LAG_SAMPLES];
	int             snapshotSamples[LAG_SAMPLES];
	int             snapshotCount;
} lagometer_t;

lagometer_t     lagometer;

/*
==============
CG_AddLagometerFrameInfo

Adds the current interpolate / extrapolate bar for this frame
==============
*/
void CG_AddLagometerFrameInfo(void)
{
	int             offset;

	offset = cg.time - cg.latestSnapshotTime;
	lagometer.frameSamples[lagometer.frameCount & (LAG_SAMPLES - 1)] = offset;
	lagometer.frameCount++;
}

/*
==============
CG_AddLagometerSnapshotInfo

Each time a snapshot is received, log its ping time and
the number of snapshots that were dropped before it.

Pass NULL for a dropped packet.
==============
*/
void CG_AddLagometerSnapshotInfo(snapshot_t * snap)
{
	// dropped packet
	if(!snap)
	{
		lagometer.snapshotSamples[lagometer.snapshotCount & (LAG_SAMPLES - 1)] = -1;
		lagometer.snapshotCount++;
		return;
	}

	// add this snapshot's info
	lagometer.snapshotSamples[lagometer.snapshotCount & (LAG_SAMPLES - 1)] = snap->ping;
	lagometer.snapshotFlags[lagometer.snapshotCount & (LAG_SAMPLES - 1)] = snap->snapFlags;
	lagometer.snapshotCount++;
}

/*
==============
CG_DrawDisconnect

Should we draw something differnet for long lag vs no packets?
==============
*/
static void CG_DrawDisconnect(void)
{
	float           x, y;
	int             cmdNum;
	usercmd_t       cmd;
	const char     *s;
	int             w;

	// draw the phone jack if we are completely past our buffers
	cmdNum = trap_GetCurrentCmdNumber() - CMD_BACKUP + 1;
	trap_GetUserCmd(cmdNum, &cmd);
	if(cmd.serverTime <= cg.snap->ps.commandTime || cmd.serverTime > cg.time)
	{
		// special check for map_restart
		return;
	}

	// also add text in center of screen
	s = "Connection Interrupted";
	w = CG_Text_Width(s, 0.5f, 0, &cgs.media.freeSansBoldFont);
	CG_Text_Paint((cgs.screenXSize/2) - (w/2), 100, 0.5f, colorRed, s, 0, 0, UI_DROPSHADOW, &cgs.media.freeSansBoldFont);

	//otty: readjusted lagometer
	x = 640 - 68;
	y = 480 - 120;

	trap_R_SetColor(baseTeamColor);
	CG_DrawPic(x - 8, y - 8, 48 + 16, 48 + 16, cgs.media.lagometer_lagShader);
	trap_R_SetColor(NULL);

	// blink the icon
	if((cg.time >> 9) & 1)
	{
		return;
	}

	CG_DrawPic(x, y, 48, 48, trap_R_RegisterShader("gfx/2d/net.tga"));
}


#define	MAX_LAGOMETER_PING	900
#define	MAX_LAGOMETER_RANGE	300

/*
==============
CG_DrawLagometer
==============
*/
static void CG_DrawLagometer(void)
{
	int             a, x, y, i;
	float           v;
	float           ax, ay, aw, ah, mid, range;
	int             color;
	float           vscale;
	qboolean        lag = qfalse;
	vec4_t          basecolor;
	vec4_t          fadecolor;

	playerState_t  *ps;
	centity_t      *cent;

	ps = &cg.snap->ps;
	cent = &cg_entities[cg.snap->ps.clientNum];

	// Tr3B: even draw the lagometer when connected to a local server
	if(!cg_lagometer.integer /*|| cgs.localServer */ )
	{
		CG_DrawDisconnect();
		return;
	}

	//
	// draw the graph
	//

	//otty: readjusted lagometer
	x = 68;
	y = 480 - 140;

	if(ps->persistant[PERS_TEAM] == TEAM_BLUE)
		Vector4Copy(blueTeamColor, basecolor);
	else if(ps->persistant[PERS_TEAM] == TEAM_RED)
		Vector4Copy(redTeamColor, basecolor);
	else
		Vector4Copy(baseTeamColor, basecolor);


	trap_R_SetColor(basecolor);
	CG_DrawPic(x - 8, y - 8, 48 + 16, 48 + 16, cgs.media.lagometerShader);
	trap_R_SetColor(NULL);


	ax = x;
	ay = y;
	aw = 48;
	ah = 48;
	CG_AdjustFrom640(&ax, &ay, &aw, &ah);

	color = -1;
	range = ah / 3;
	mid = ay + range;

	vscale = range / MAX_LAGOMETER_RANGE;

	// draw the frame interpoalte / extrapolate graph
	for(a = 0; a < aw; a++)
	{
		i = (lagometer.frameCount - 1 - a) & (LAG_SAMPLES - 1);
		v = lagometer.frameSamples[i];
		v *= vscale;
		if(v > 0)
		{
			if(color != 1)
			{
				color = 1;
				Vector4Copy(g_color_table[ColorIndex(COLOR_YELLOW)], fadecolor);
				fadecolor[3] = (float)((aw - a) / aw);
				trap_R_SetColor(fadecolor);
			}
			if(v > range)
			{
				v = range;
			}
			trap_R_DrawStretchPic(ax + aw - a, mid - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
		}
		else if(v < 0)
		{
			if(color != 2)
			{
				color = 2;
				Vector4Copy(g_color_table[ColorIndex(COLOR_BLUE)], fadecolor);
				fadecolor[3] = (float)((aw - a) / aw);
				trap_R_SetColor(fadecolor);

			}
			v = -v;
			if(v > range)
			{
				v = range;
			}
			trap_R_DrawStretchPic(ax + aw - a, mid, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
		}
	}

	// draw the snapshot latency / drop graph
	range = ah / 2;
	vscale = range / MAX_LAGOMETER_PING;

	for(a = 0; a < aw; a++)
	{
		i = (lagometer.snapshotCount - 1 - a) & (LAG_SAMPLES - 1);
		v = lagometer.snapshotSamples[i];
		if(v > 0)
		{
			if(lagometer.snapshotFlags[i] & SNAPFLAG_RATE_DELAYED)
			{
				if(color != 5)
				{
					color = 5;	// YELLOW for rate delay
					Vector4Copy(g_color_table[ColorIndex(COLOR_YELLOW)], fadecolor);
					fadecolor[3] = (float)((aw - a) / aw);
					trap_R_SetColor(fadecolor);
				}
			}
			else
			{
				if(color != 3)
				{
					color = 3;
					Vector4Copy(g_color_table[ColorIndex(COLOR_GREEN)], fadecolor);
					fadecolor[3] = (float)((aw - a) / aw) * 0.5f;
					trap_R_SetColor(fadecolor);
				}
			}
			v = v * vscale;
			if(v > range)
			{
				v = range;
			}
			trap_R_DrawStretchPic(ax + aw - a, ay + ah - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
		}
		else if(v < 0)
		{
			if(color != 4)
			{
				color = 4;		// RED for dropped snapshots
				Vector4Copy(g_color_table[ColorIndex(COLOR_RED)], fadecolor);
				//fadecolor[3] = (float)((aw - a) / aw);
				fadecolor[3] = 1.0f;
				trap_R_SetColor(fadecolor);
			}
			if(ah - range > 10)
				lag = qtrue;

			trap_R_DrawStretchPic(ax + aw - a, ay + ah - range, 1, range, 0, 0, 0, 0, cgs.media.whiteShader);
		}
	}
/*
	if(lag){
		trap_R_SetColor(basecolor);
		CG_DrawPic(x-8, y-8, 48+16, 48+16, cgs.media.lagometer_lagShader);
		trap_R_SetColor(NULL);
	}
*/
	trap_R_SetColor(NULL);

	if(cg_nopredict.integer || cg_synchronousClients.integer)
	{
		CG_Text_Paint(ax, ay, 0.4f, colorRed, "snc", 0, 0, UI_CENTER | UI_DROPSHADOW, &cgs.media.freeSansBoldFont);
	}

	CG_DrawDisconnect();
}



/*
===============================================================================

CENTER PRINTING

===============================================================================
*/


/*
==============
CG_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void CG_CenterPrint(const char *str, int y, int charWidth)
{
	char           *s;

	Q_strncpyz(cg.centerPrint, str, sizeof(cg.centerPrint));

	cg.centerPrintTime = cg.time;
	cg.centerPrintY = y;
	cg.centerPrintCharWidth = charWidth;

	// count the number of lines for centering
	cg.centerPrintLines = 1;
	s = cg.centerPrint;
	while(*s)
	{
		if(*s == '\n')
			cg.centerPrintLines++;
		s++;
	}
}


/*
===================
CG_DrawCenterString
===================
*/
static void CG_DrawCenterString(void)
{
	char           *start;
	int             l;
	int             x, y, w, h;
	float          *color;

	if(!cg.centerPrintTime)
	{
		return;
	}

	color = CG_FadeColor(cg.centerPrintTime, 1000 * cg_centertime.value);
	if(!color)
	{
		return;
	}

	trap_R_SetColor(color);

	color[3] *= 0.75f;

	start = cg.centerPrint;

	y = cg.centerPrintY - cg.centerPrintLines * BIGCHAR_HEIGHT / 2;

	while(1)
	{
		char            linebuffer[1024];

		for(l = 0; l < 50; l++)
		{
			if(!start[l] || start[l] == '\n')
			{
				break;
			}
			linebuffer[l] = start[l];
		}
		linebuffer[l] = 0;

		w = CG_Text_Width(linebuffer, 0.4f, 0, &cgs.media.freeSansBoldFont);
		h = CG_Text_Height(linebuffer, 0.4f, 0, &cgs.media.freeSansBoldFont);
		x = (cgs.screenXSize - w) / 2;
		CG_Text_Paint(x, y + h, 0.4f, color, linebuffer, 0, 0, UI_CENTER | UI_DROPSHADOW, &cgs.media.freeSansBoldFont);
		y += h + 6;

		while(*start && (*start != '\n'))
		{
			start++;
		}
		if(!*start)
		{
			break;
		}
		start++;
	}

	trap_R_SetColor(NULL);
}



/*
================================================================================

CROSSHAIR

================================================================================
*/

void CG_DrawCrosshairNew(void)
{
	qhandle_t       dot;
	qhandle_t       circle;
	qhandle_t       cross;

	float           w, h;
	float           x, y;
	float           f;


	if(cg_crosshairDot.integer <= 0)	// no dot
	{
		dot = 0;
	}
	else
	{
		dot = cgs.media.crosshairDot[cg_crosshairDot.integer - 1];
	}

	if(cg_crosshairCircle.integer <= 0)	// no circle
	{
		circle = 0;
	}
	else
	{
		circle = cgs.media.crosshairCircle[cg_crosshairCircle.integer - 1];
	}

	if(cg_crosshairCross.integer <= 0)	// no cross
	{
		cross = 0;
	}
	else
	{
		cross = cgs.media.crosshairCross[cg_crosshairCross.integer - 1];
	}

	w = h = cg_crosshairSize.value;

	if(cg_crosshairPulse.integer == 1)	// pulse the size of the crosshair when picking up items
	{
		f = cg.time - cg.itemPickupBlendTime;

		if(f > 0 && f < ITEM_BLOB_TIME)
		{
			f /= ITEM_BLOB_TIME;
			w *= (1 + f);
			h *= (1 + f);
		}

	}

	x = cg_crosshairX.integer;
	y = cg_crosshairY.integer;

	// set color based on health
	if(cg_crosshairHealth.integer)
	{
		vec4_t          hcolor;

		CG_ColorForHealth(hcolor);
		trap_R_SetColor(hcolor);
	}
	else
	{
		trap_R_SetColor(NULL);
	}


	if(dot)
		trap_R_DrawStretchPic(x + cg.refdef.x + 0.5 * (cg.refdef.width - w),
							  y + cg.refdef.y + 0.5 * (cg.refdef.height - h), w, h, 0, 0, 1, 1, dot);
	if(circle)
		trap_R_DrawStretchPic(x + cg.refdef.x + 0.5 * (cg.refdef.width - w),
							  y + cg.refdef.y + 0.5 * (cg.refdef.height - h), w, h, 0, 0, 1, 1, circle);
	if(cross)
		trap_R_DrawStretchPic(x + cg.refdef.x + 0.5 * (cg.refdef.width - w),
							  y + cg.refdef.y + 0.5 * (cg.refdef.height - h), w, h, 0, 0, 1, 1, cross);



	trap_R_SetColor(NULL);

}


/*
=================
CG_DrawCrosshair
=================
*/
static void CG_DrawCrosshair(void)
{
	float           w, h;
	qhandle_t       hShader;
	float           f;
	float           x, y;
	int             ca;

	if(!cg_drawCrosshair.integer)
	{
		return;
	}

	if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
	{
		return;
	}

	if(cg.renderingThirdPerson)
	{
		return;
	}

	if(cg_drawStatus.integer == 3)
	{
		CG_DrawCrosshairNew();
		return;
	}
	// set color based on health
	if(cg_crosshairHealth.integer)
	{
		vec4_t          hcolor;

		CG_ColorForHealth(hcolor);
		trap_R_SetColor(hcolor);
	}
	else
	{
		trap_R_SetColor(NULL);
	}

	w = h = cg_crosshairSize.value;

	// pulse the size of the crosshair when picking up items
	f = cg.time - cg.itemPickupBlendTime;
	if(f > 0 && f < ITEM_BLOB_TIME)
	{
		f /= ITEM_BLOB_TIME;
		w *= (1 + f);
		h *= (1 + f);
	}

	x = cg_crosshairX.integer;
	y = cg_crosshairY.integer;

	ca = cg_drawCrosshair.integer;
	if(ca < 0)
	{
		ca = 0;
	}
	hShader = cgs.media.crosshairShader[ca % NUM_CROSSHAIRS];

	trap_R_DrawStretchPic(x + cg.refdef.x + 0.5 * (cg.refdef.width - w),
						  y + cg.refdef.y + 0.5 * (cg.refdef.height - h), w, h, 0, 0, 1, 1, hShader);
}



/*
=================
CG_ScanForCrosshairEntity
=================
*/
static void CG_ScanForCrosshairEntity(void)
{
	trace_t         trace;
	vec3_t          start, end;
	int             content;

	VectorCopy(cg.refdef.vieworg, start);
	VectorMA(start, 131072, cg.refdef.viewaxis[0], end);

	CG_Trace(&trace, start, vec3_origin, vec3_origin, end, cg.snap->ps.clientNum, CONTENTS_SOLID | CONTENTS_BODY);
	if(trace.entityNum >= MAX_CLIENTS)
	{
		return;
	}

	// if the player is in fog, don't show it
	content = trap_CM_PointContents(trace.endpos, 0);
	if(content & CONTENTS_FOG)
	{
		return;
	}

	// if the player is invisible, don't show it
	if(cg_entities[trace.entityNum].currentState.powerups & (1 << PW_INVIS))
	{
		return;
	}

	// update the fade timer
	cg.crosshairClientNum = trace.entityNum;
	cg.crosshairClientTime = cg.time;
}


/*
=====================
CG_DrawCrosshairNames
=====================
*/
static void CG_DrawCrosshairNames(void)
{
	float          *color;
	char           *name;
	float           w;

	if(!cg_drawCrosshair.integer)
	{
		return;
	}
	if(!cg_drawCrosshairNames.integer)
	{
		return;
	}
	if(cg.renderingThirdPerson)
	{
		return;
	}

	// scan the known entities to see if the crosshair is sighted on one
	CG_ScanForCrosshairEntity();

	// draw the name of the player being looked at
	color = CG_FadeColor(cg.crosshairClientTime, 1000);
	if(!color)
	{
		trap_R_SetColor(NULL);
		return;
	}

	name = cgs.clientinfo[cg.crosshairClientNum].name;

	if(cg_drawStatus.integer == 3)
	{
		//think this is a better place

		color[3] *= 0.85f;
		w = CG_Text_Width(name, 0.2f, 0, &cgs.media.freeSansBoldFont);
		CG_Text_Paint((cgs.screenXSize - w) / 2, (2 * cgs.screenYSize) / 3, 0.2f, color, name, 0, 0, 0, &cgs.media.freeSansBoldFont);


		trap_R_SetColor(NULL);
		return;
	}

	color[3] *= 0.5f;
	w = CG_Text_Width(name, 0.3f, 0, &cgs.media.freeSansBoldFont);
	CG_Text_Paint((cgs.screenXSize - w) / 2, cgs.screenYSize / 3, 0.3f, color, name, 0, 0, UI_DROPSHADOW, &cgs.media.freeSansBoldFont);

	trap_R_SetColor(NULL);
}


//==============================================================================

/*
=================
CG_DrawSpectator
=================
*/
static void CG_DrawSpectator(void)
{
	CG_Text_PaintAligned(cgs.screenXSize/2, 440, "SPECTATOR", 0.45f, UI_CENTER | UI_DROPSHADOW, colorWhite, &cgs.media.freeSansBoldFont);

	if(cgs.gametype == GT_TOURNAMENT)
	{
		CG_Text_PaintAligned(cgs.screenXSize/2, 460, "waiting to play", 0.25f, UI_CENTER | UI_DROPSHADOW, colorWhite,
							 &cgs.media.freeSansBoldFont);
	}
	else if(cgs.gametype >= GT_TEAM)
	{
		CG_Text_PaintAligned(cgs.screenXSize/2, 460, "press ESC and use the JOIN menu to play", 0.25f, UI_CENTER | UI_DROPSHADOW, colorWhite,
							 &cgs.media.freeSansBoldFont);
	}
}

/*
=================
CG_DrawVote
=================
*/
static void CG_DrawVote(void)
{
	char           *s;
	int             sec;

	if(!cgs.voteTime)
	{
		return;
	}

	// play a talk beep whenever it is modified
	if(cgs.voteModified)
	{
		cgs.voteModified = qfalse;
		trap_S_StartLocalSound(cgs.media.talkSound, CHAN_LOCAL_SOUND);
	}

	sec = (VOTE_TIME - (cg.time - cgs.voteTime)) / 1000;
	if(sec < 0)
	{
		sec = 0;
	}

	s = va("VOTE(%i):%s yes:%i no:%i", sec, cgs.voteString, cgs.voteYes, cgs.voteNo);
	CG_DrawSmallString(0, 58, s, 1.0F);
}

/*
=================
CG_DrawTeamVote
=================
*/
static void CG_DrawTeamVote(void)
{
	char           *s;
	int             sec, cs_offset;

	if(cgs.clientinfo->team == TEAM_RED)
		cs_offset = 0;
	else if(cgs.clientinfo->team == TEAM_BLUE)
		cs_offset = 1;
	else
		return;

	if(!cgs.teamVoteTime[cs_offset])
	{
		return;
	}

	// play a talk beep whenever it is modified
	if(cgs.teamVoteModified[cs_offset])
	{
		cgs.teamVoteModified[cs_offset] = qfalse;
		trap_S_StartLocalSound(cgs.media.talkSound, CHAN_LOCAL_SOUND);
	}

	sec = (VOTE_TIME - (cg.time - cgs.teamVoteTime[cs_offset])) / 1000;
	if(sec < 0)
	{
		sec = 0;
	}
	s = va("TEAMVOTE(%i):%s yes:%i no:%i", sec, cgs.teamVoteString[cs_offset],
		   cgs.teamVoteYes[cs_offset], cgs.teamVoteNo[cs_offset]);
	CG_DrawSmallString(0, 90, s, 1.0F);
}


static qboolean CG_DrawScoreboard(void)
{
	return CG_DrawScoreboardNew();
}

/*
=================
CG_DrawIntermission
=================
*/
static void CG_DrawIntermission(void)
{
	if(cgs.gametype == GT_SINGLE_PLAYER)
	{
		CG_DrawCenterString();
		return;
	}

	cg.scoreFadeTime = cg.time;
	cg.scoreBoardShowing = CG_DrawScoreboard();
}

/*
=================
CG_DrawFollow
=================
*/
static qboolean CG_DrawFollow(void)
{
	const char     *name;

	if(!(cg.snap->ps.pm_flags & PMF_FOLLOW))
	{
		return qfalse;
	}

	CG_Text_PaintAligned(cgs.screenXSize/2, 64, "following", 0.35f, UI_CENTER | UI_DROPSHADOW, colorWhite, &cgs.media.freeSansBoldFont);

	name = cgs.clientinfo[cg.snap->ps.clientNum].name;
	CG_Text_PaintAligned(cgs.screenXSize/2, 90, name, 0.45f, UI_CENTER | UI_DROPSHADOW, colorWhite, &cgs.media.freeSansBoldFont);

	return qtrue;
}

/*
=================
CG_DrawAmmoWarning
=================
*/
static void CG_DrawAmmoWarning(void)
{

	if(cg_drawAmmoWarning.integer == 0)
	{
		return;
	}

	if(!cg.lowAmmoWarning)
	{
		return;
	}

	if(cg.lowAmmoWarning == 2)
	{
		CG_Text_PaintAligned(cgs.screenXSize/2, 400, "out of ammo", 0.4f, UI_CENTER | UI_DROPSHADOW, colorRed, &cgs.media.freeSansBoldFont);
	}
	else
	{

		CG_Text_PaintAligned(cgs.screenXSize/2, 400, "ammo low", 0.4f, UI_CENTER | UI_DROPSHADOW, colorYellow, &cgs.media.freeSansBoldFont);
	}
}

/*
=================
CG_DrawProxWarning
=================
*/
static void CG_DrawProxWarning(void)
{
	char            s[32];
	int             w;
	static int      proxTime;
	static int      proxCounter;
	static int      proxTick;

	if(!(cg.snap->ps.eFlags & EF_TICKING))
	{
		proxTime = 0;
		return;
	}

	if(proxTime == 0)
	{
		proxTime = cg.time + 5000;
		proxCounter = 5;
		proxTick = 0;
	}

	if(cg.time > proxTime)
	{
		proxTick = proxCounter--;
		proxTime = cg.time + 1000;
	}

	if(proxTick != 0)
	{
		Com_sprintf(s, sizeof(s), "INTERNAL COMBUSTION IN: %i", proxTick);
	}
	else
	{
		Com_sprintf(s, sizeof(s), "YOU HAVE BEEN MINED");
	}

	w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
	CG_DrawBigStringColor((cgs.screenXSize/2) - (w/2), 64 + BIGCHAR_HEIGHT, s, (float *)g_color_table[ColorIndex(COLOR_RED)]);
}

/*
=================
CG_DrawWarmup
=================
*/
static void CG_DrawWarmup(void)
{
	int             sec;
	int             i;
	float           scale;
	clientInfo_t   *ci1, *ci2;
	int             cw;
	const char     *s;

	sec = cg.warmup;
	if(!sec)
	{
		return;
	}

	if(sec < 0)
	{
		s = "Waiting for players";
		CG_Text_PaintAligned(cgs.screenXSize/2, 40, s, 0.5f, UI_CENTER, colorWhite, &cgs.media.freeSansBoldFont);
		cg.warmupCount = 0;
		return;
	}

	if(cgs.gametype == GT_TOURNAMENT)
	{
		// find the two active players
		ci1 = NULL;
		ci2 = NULL;
		for(i = 0; i < cgs.maxclients; i++)
		{
			if(cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team == TEAM_FREE)
			{
				if(!ci1)
				{
					ci1 = &cgs.clientinfo[i];
				}
				else
				{
					ci2 = &cgs.clientinfo[i];
				}
			}
		}

		if(ci1 && ci2)
		{
			s = va("%s vs %s", ci1->name, ci2->name);

			CG_Text_PaintAligned(cgs.screenXSize/2, 64, s, 0.4f, UI_CENTER | UI_DROPSHADOW, colorWhite, &cgs.media.freeSansBoldFont);
		}
	}
	else
	{
		if(cgs.gametype == GT_FFA)
		{
			s = "Free For All";
		}
		else if(cgs.gametype == GT_TEAM)
		{
			s = "Team Deathmatch";
		}
		else if(cgs.gametype == GT_CTF)
		{
			s = "Capture the Flag";
		}
		else if(cgs.gametype == GT_1FCTF)
		{
			s = "One Flag CTF";
		}
		else if(cgs.gametype == GT_OBELISK)
		{
			s = "Overload";
		}
		else if(cgs.gametype == GT_HARVESTER)
		{
			s = "Harvester";
		}
		else
		{
			s = "";
		}

		CG_Text_PaintAligned(cgs.screenXSize/2, 90, s, 0.4f, UI_CENTER | UI_DROPSHADOW, colorWhite, &cgs.media.freeSansBoldFont);
	}

	sec = (sec - cg.time) / 1000;
	if(sec < 0)
	{
		cg.warmup = 0;
		sec = 0;
	}
	s = va("Starts in: %i", sec + 1);
	if(sec != cg.warmupCount)
	{
		cg.warmupCount = sec;
		switch (sec)
		{
			case 0:
				trap_S_StartLocalSound(cgs.media.count1Sound, CHAN_ANNOUNCER);
				break;
			case 1:
				trap_S_StartLocalSound(cgs.media.count2Sound, CHAN_ANNOUNCER);
				break;
			case 2:
				trap_S_StartLocalSound(cgs.media.count3Sound, CHAN_ANNOUNCER);
				break;
			default:
				break;
		}
	}
	scale = 0.45f;
	switch (cg.warmupCount)
	{
		case 0:
			cw = 28;
			scale = 0.34f;
			break;
		case 1:
			cw = 24;
			scale = 0.31f;
			break;
		case 2:
			cw = 20;
			scale = 0.28f;
			break;
		default:
			cw = 16;
			scale = 0.25f;
			break;
	}

	CG_Text_PaintAligned(cgs.screenXSize/2, 125, s, scale, UI_CENTER | UI_DROPSHADOW, colorWhite, &cgs.media.freeSansBoldFont);
}

//==================================================================================



//************** otty debug  ***************//


/*
=================
CG_DrawDebug
=================
*/

static int debugModel(int x, int y)
{
	y += 12;

	CG_Text_Paint(x, y, 0.2f, colorRed, "Model:", 0.0f, 0, 0, &cgs.media.freeSansBoldFont);
	y += 10;

	CG_Text_Paint(x + 10, y, 0.15f, colorYellow, va("current anim: %i", debug_anim_current), 0.0f, 0, 0, &cgs.media.freeSansBoldFont);
	y += 8;
	CG_Text_Paint(x + 10, y, 0.15f, colorYellow, va("old anim: %i", debug_anim_old), 0.0f, 0, 0, &cgs.media.freeSansBoldFont);
	y += 8;
	CG_Text_Paint(x + 10, y, 0.15f, colorYellow, va("anim blend : %f", debug_anim_blend), 0.0f, 0, 0, &cgs.media.freeSansBoldFont);
	y += 8;


	return y;


}

int             lowPeak = 999;
int             highPeak = 0;

static int debugSystem(int x, int y)
{
	char           *s;
	int             mins, seconds, tens;
	int             msec;
	static int      previousTimes[FPS_FRAMES];
	static int      index;
	int             i, total;
	int             fps;
	static int      previous;
	int             t, frameTime;

	t = trap_Milliseconds();
	frameTime = t - previous;
	previous = t;
	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;


	CG_Text_Paint(x, y, 0.2f, colorRed, "System:", 0.0f, 0, 0, &cgs.media.freeSansBoldFont);
	y += 10;


	if(index > FPS_FRAMES)
	{
		// average multiple frames together to smooth changes out a bit
		total = 0;
		for(i = 0; i < FPS_FRAMES; i++)
		{
			total += previousTimes[i];
		}
		if(!total)
		{
			total = 1;
		}

		fps = 1000 * FPS_FRAMES / total;


		if(fps < lowPeak)
			lowPeak = fps;
		else if(fps > highPeak)
			highPeak = fps;

		s = va("Average FPS: %i ", fps);
		CG_Text_Paint(x + 10, y, 0.15f, colorYellow, s, 0.0f, 0, 0, &cgs.media.freeSansBoldFont);
		y += 8;

		s = va("HighPeak : %i ", highPeak);
		CG_Text_Paint(x + 10, y, 0.15f, colorYellow, s, 0.0f, 0, 0, &cgs.media.freeSansBoldFont);
		y += 8;

		s = va("LowPeak : %i ", lowPeak);
		CG_Text_Paint(x + 10, y, 0.15f, colorYellow, s, 0.0f, 0, 0, &cgs.media.freeSansBoldFont);
		y += 8;

	}


	msec = cg.time - cgs.levelStartTime;

	seconds = msec / 1000;
	mins = seconds / 60;
	seconds -= mins * 60;
	tens = seconds / 10;
	seconds -= tens * 10;


	s = va("Time : %i:%i%i", mins, tens, seconds);
	CG_Text_Paint(x + 10, y, 0.15f, colorYellow, s, 0.0f, 0, 0, &cgs.media.freeSansBoldFont);

	return y;
}

//==================================================================================

static void CG_DrawDebug(void)
{
	int             x = 5;
	int             y = 140;

	vec4_t          colorDebug = { 0.2f, 0.2f, 0.2f, 0.66f };

	CG_FillRect(x, y, 200, 180, colorDebug);
	x += 10;
	y += 10;


	y = debugSystem(x, y);
	y = debugModel(x, y);

}

void CG_DrawWoundedInfo(void)
{
	char           *info;

	int             x = cgs.screenXSize / 2;
	int             y = cgs.screenYSize * 0.65f;
	int             h;

	info = va("You are Wounded.");
	h = CG_Text_Height(info, 0.5f, 0, &cgs.media.freeSansBoldFont);
	CG_Text_PaintAligned(x, y - h, info, 0.5f, UI_CENTER, colorWhite, &cgs.media.freeSansBoldFont);

	//TODO: replace key names dynamically with binds.
	if(cg.snap->ps.eFlags & EF_TAPOUT)
	{
		info = va("Press CTRL to wait for an allied player to revive you.");
	}
	else
	{
		info = va("Press SPACE to respawn with the next wave.");
	}
	CG_Text_PaintAligned(x, y + 2, info, 0.5f, UI_CENTER, colorWhite, &cgs.media.freeSansBoldFont);
}

/*
=================
CG_Draw2D
=================
*/
static void CG_Draw2D(void)
{
	qboolean        noScoreboard;

	if(cg_debugHUD.integer == 1)
	{
		CG_DrawDebug();
		return;
	}

#ifdef MISSIONPACK
	if(cgs.orderPending && cg.time > cgs.orderTime)
	{
		CG_CheckOrderPending();
	}
#endif

	// if we are taking a levelshot for the menu, don't draw anything
	if(cg.levelShot)
	{
		return;
	}

	if(cg_draw2D.integer == 0)
	{
		return;
	}

	Vector4Set(baseTeamColor, cg_hudRed.value, cg_hudGreen.value, cg_hudBlue.value, cg_hudAlpha.value);

	redTeamColor[3] = cg_hudAlpha.value;
	blueTeamColor[3] = cg_hudAlpha.value;


	if(cg.snap->ps.pm_type == PM_INTERMISSION)
	{
		CG_DrawWeaverNotify();
		CG_DrawWeaverChat();
		CG_DrawIntermission();
		return;
	}

	noScoreboard = !CG_DrawScoreboardNew();

	CG_DrawOSD();

	if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
	{
		if(noScoreboard)
		{
			CG_DrawSpectator();
			CG_DrawCrosshairNew();
			CG_DrawCrosshairNames();
		}
	}
	else
	{
		// don't draw any status if dead or the scoreboard is being explicitly shown
		if(!cg.showScores && cg.snap->ps.stats[STAT_HEALTH] > 0)
		{
			CG_DrawWeaverHUD();

			CG_DrawCrosshairNew();
			CG_DrawCrosshairNames();
		}
		else if(cg.snap->ps.pm_type == PM_WOUNDED)
		{
			CG_DrawWoundedInfo();
		}
	}

	CG_DrawVote();
	CG_DrawTeamVote();

	CG_DrawUpperRight();

	if(noScoreboard && !CG_DrawFollow())
	{
		CG_DrawWarmup();
	}

	// don't draw center string if scoreboard is up
	//cg.scoreBoardShowing = CG_DrawScoreboard();
	if(noScoreboard)
	{
		CG_DrawWeaverNotify();
		CG_DrawWeaverChat();

		CG_DrawLagometer();
		CG_DrawTimers();
		CG_DrawCenterString();
	}
}

/*
=====================
CG_DrawActive

Perform all drawing needed to completely fill the screen
=====================
*/
void CG_DrawActive(stereoFrame_t stereoView)
{
	float           separation;
	vec3_t          baseOrg;

	// optionally draw the info screen instead
	if(!cg.snap)
	{
		CG_DrawInformation();
		return;
	}

	switch (stereoView)
	{
		case STEREO_CENTER:
			separation = 0;
			break;
		case STEREO_LEFT:
			separation = -cg_stereoSeparation.value / 2;
			break;
		case STEREO_RIGHT:
			separation = cg_stereoSeparation.value / 2;
			break;
		default:
			separation = 0;
			CG_Error("CG_DrawActive: Undefined stereoView");
	}


	// clear around the rendered view if sized down
	CG_TileClear();

	// offset vieworg appropriately if we're doing stereo separation
	VectorCopy(cg.refdef.vieworg, baseOrg);
	if(separation != 0)
	{
		VectorMA(cg.refdef.vieworg, -separation, cg.refdef.viewaxis[1], cg.refdef.vieworg);
	}

	// draw 3D view
	trap_R_RenderScene(&cg.refdef);

	// restore original viewpoint if running stereo
	if(separation != 0)
	{
		VectorCopy(baseOrg, cg.refdef.vieworg);
	}

	// draw status bar and other floating elements
	CG_Draw2D();
}
