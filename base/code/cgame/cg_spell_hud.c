/*
===========================================================================
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

// cg_spell_hud.c -- Weaver HUD
#include "cg_local.h"
#include "cg_spell_util.h"

const vec4_t    colorAir = { 0.937f, 0.90f, 0.00f, 0.7f };
const vec4_t    colorFire = { 0.94f, 0.16f, 0.16f, 0.7f };
const vec4_t    colorEarth = { 0.0f, 0.87f, 0.00f, 0.7f };
const vec4_t    colorWater = { 0.0f, 0.32f, 0.906f, 0.7f };

const vec4_t    colorTeamBlue = { 0.2f, 0.2f, 1.0f, 0.7f };	// blue
const vec4_t    colorTeamRed = { 1.0f, 0.2f, 0.2f, 0.7f };	// red
const vec4_t    colorTeamSpec = { 0.15f, 0.8f, 0.8f, 0.7f };	// cyan
const vec4_t    colorTeamAll = { 0.3f, 0.7f, 0.3f, 0.7f };	// green

const vec4_t    colorFull = { 1.0f, 1.0f, 1.0f, 1.0f };
const vec4_t    colorEmpty = { 1.0f, 1.0f, 1.0f, 0.3f };

vec2_t          tutTextOffset[WVP_NUMBER] = { {0.0f, 0.0f},
{0.0f, 1.1f}, {0.778f, 0.778f},
{1.1f, 0.0f}, {0.778f, -0.778f},
{0.0f, -1.1f}, {-0.778f, -0.778f},
{-1.1f, 0.0f}, {-0.778f, 0.778f},
{0.0f, 0.0f}
};

int             powerCount[WVP_NUMBER];

typedef enum
{
	DRFD_DOWN,
	DRFD_UP,
	DRFD_LEFT,
	DRFD_RIGHT
} DrawRectFillDir;

/*
================
CG_FillRectUp

Utility method.
=================
*/
void CG_DrawFillRect(float x, float y, float width, float height, const vec4_t color, DrawRectFillDir dir)
{
	const float s1 = 0;
	const float t1 = 0;
	const float s2 = 0;
	const float t2 = 0;

	trap_R_SetColor(color);
	switch(dir)
	{
		case DRFD_UP:
			trap_R_DrawStretchPic(x, y-height, width, height, s1, t1, s2, t2, cgs.media.whiteShader);
			break;
		case DRFD_LEFT:
			trap_R_DrawStretchPic(x-width, y, width, height, s1, t1, s2, t2, cgs.media.whiteShader);
			break;
		default:
		case DRFD_DOWN:
		case DRFD_RIGHT:
			trap_R_DrawStretchPic(x, y, width, height, s1, t1, s2, t2, cgs.media.whiteShader);
			break;
	}
	trap_R_SetColor(NULL);
}

typedef struct hudSizes_s {
	float           f;

	float           health_offset_x;
	float           health_offset_y;
	float           health_w;
	float           health_h;

	float           health_val_offset_x;
	float           health_val_offset_y;

	float           stamina_offset_x;
	float           stamina_offset_y;
	float           stamina_w;
	float           stamina_h;

	float           power_offset_left_w;
	float           power_offset_right_w;
	float           power_offset_x;
	float           power_offset_y;
	float           power_div_w;
	float           power_div_h;
	float           power_spell_h;

	float           spellicon_w;
	float           spellicon_frame_w;
	float           spellicon_frame_offset_x;

	float           protect_1_offset_x;
	float           protect_1_offset_y;
	float           protect_2_offset_x;
	float           protect_2_offset_y;
	float           protect_3_offset_x;
	float           protect_3_offset_y;
	float           protect_4_offset_x;
	float           protect_4_offset_y;
	float           protect_w;
	float           protect_h;

	float           notify_x;
	float           notify_y;
	float           notify_text_h;

	float           cross_health_w;
	float           cross_health_h;

	float           chat_x;
	float           chat_offset_y;
	float           chat_text_h;

	float           s16;
	float           s32;
	float           s64;
	float           s128;
	float           s256;
	float           s512;

	float           power;

	// Calculated from those above
	float           a;
} hudSizes_t;

static hudSizes_t s;

void CG_HudSizesRecalc(void)
{
	// Calculated from other fields
	s.spellicon_frame_offset_x = s.spellicon_frame_w - s.spellicon_w;
	s.spellicon_frame_offset_x *= 0.5f;
	s.spellicon_frame_offset_x += s.spellicon_w;
}

void CG_HudSizesInit(void)
{
	s.f = 1.0f;

	s.health_offset_x = 50.0f;
	s.health_offset_y = 132.0f;
	s.health_w = 28.0f;
	s.health_h = 270.0f;

	s.health_val_offset_x = 5.0f;
	s.health_val_offset_y = 20.0f;

	s.stamina_offset_x = 18.0f;
	s.stamina_offset_y = 139.0f;
	s.stamina_w = 14.0f;
	s.stamina_h = 238.0f;

	s.power_offset_left_w = 180.0f;
	s.power_offset_right_w = 105.0f;
	s.power_offset_x = 151.0f;
	s.power_offset_y = 8.0f;
	s.power_div_w = 16.0f;
	s.power_div_h = 64.0f;
	s.power_spell_h = 18.0f;

	s.spellicon_w = 80.0f;
	s.spellicon_frame_w = 85.0f;

	s.protect_1_offset_x = 87.0f;
	s.protect_1_offset_y = 120.0f;
	s.protect_2_offset_x = 78.0f;
	s.protect_2_offset_y = 120.0f;
	s.protect_3_offset_x = 69.0f;
	s.protect_3_offset_y = 120.0f;
	s.protect_4_offset_x = 60.0f;
	s.protect_4_offset_y = 120.0f;
	s.protect_w = 6.0f;
	s.protect_h = 214.0f;

	s.notify_x = 10.0f;
	s.notify_y = 180.0f;
	s.notify_text_h = CG_Text_Height("!", 0.5f, 0, &cgs.media.freeSansBoldFont);

	s.cross_health_w = 150;
	s.cross_health_h = 10;

	s.chat_x = 10.0f;
	s.chat_offset_y = 260.0f;
	s.chat_text_h = CG_Text_Height("!", 0.5f, 0, &cgs.media.freeSansBoldFont);

	s.s16 = 16.0f;
	s.s32 = 32.0f;
	s.s64 = 64.0f;
	s.s128 = 128.0f;
	s.s256 = 256.0f;
	s.s512 = 512.0f;

	s.power = 0.5f;

	CG_HudSizesRecalc();
}

/*
 * Scale hud by a given factor
 */
void CG_HudSizesScale(float f)
{
	s.f *= f;

	s.health_offset_x *= f;
	s.health_offset_y *= f;
	s.health_w *= f;
	s.health_h *= f;

	s.health_val_offset_x *= f;
	s.health_val_offset_y *= f;

	s.stamina_offset_x *= f;
	s.stamina_offset_y *= f;
	s.stamina_w *= f;
	s.stamina_h *= f;

	s.power_offset_left_w *= f;
	s.power_offset_right_w *= f;
	s.power_offset_x *= f;
	s.power_offset_y *= f;
	s.power_div_w *= f;
	s.power_div_h *= f;
	s.power_spell_h *= f;

	s.spellicon_w *= f;
	s.spellicon_frame_w *= f;

	s.protect_1_offset_x *= f;
	s.protect_1_offset_y *= f;
	s.protect_2_offset_x *= f;
	s.protect_2_offset_y *= f;
	s.protect_3_offset_x *= f;
	s.protect_3_offset_y *= f;
	s.protect_4_offset_x *= f;
	s.protect_4_offset_y *= f;
	s.protect_w *= f;
	s.protect_h *= f;

	s.notify_x *= f;
	s.notify_y *= f;
	s.notify_text_h *= f;

	s.cross_health_w *= f;
	s.cross_health_h *= f;

	s.chat_x *= f;
	s.chat_offset_y *= f;
	s.chat_text_h *= f;

	s.s16 *= f;
	s.s32 *= f;
	s.s64 *= f;
	s.s128 *= f;
	s.s256 *= f;
	s.s512 *= f;

	s.power *= f;

	CG_HudSizesRecalc();
}

void WeaverSetTeamColor(void)
{
	if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE)
	{
		trap_R_SetColor(colorTeamBlue);
	}
	else if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED)
	{
		trap_R_SetColor(colorTeamRed);
	}
}

/*
=================
CG_DrawWeaverChat
Based on CG_DrawTeamInfo.
=================
*/
void CG_DrawWeaverChat(void)
{
	int             i;
	vec4_t          hcolor;
	int             chatHeight;
	int             mode;
	float           fade;

	if(cg_chatHeight.integer < TEAMCHAT_HEIGHT)
		chatHeight = cg_chatHeight.integer;
	else
		chatHeight = TEAMCHAT_HEIGHT;
	if(chatHeight <= 0)
		return;					// disabled

	if(cgs.chatLastPos != cgs.chatPos)
	{
		if(cg.time - cgs.chatMsgTimes[cgs.chatLastPos % chatHeight] > cg_chatTime.integer)
		{
			cgs.chatLastPos++;
		}

		trap_R_SetColor(NULL);

		Vector4Copy(colorFull, hcolor);

		for(i = cgs.chatPos - 1; i >= cgs.chatLastPos; i--)
		{
			/*
			CG_DrawStringExt(CHATLOC_X + TINYCHAR_WIDTH,
							 CHATLOC_Y - (cgs.teamChatPos - i) * TINYCHAR_HEIGHT,
							 cgs.teamChatMsgs[i % chatHeight], hcolor, qfalse, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0);
			*/
			mode = cgs.chatModes[i % chatHeight];
			if (mode == CHAT_MODE_ALL)
			{
				// Global Chat
			}
			else if (mode == CHAT_MODE_TEAM)
			{
				// Team Chat
			}
			else if (mode == CHAT_MODE_TELL)
			{
				// Team Chat
			}
			else
			{
				// Unkown mode
				continue;
			}

			// Only fade for the last 1/3 of the time.
			fade = 3.0f - (3.0f * ((cg.time - cgs.chatMsgTimes[i % chatHeight]) / (float)cg_chatTime.integer));
			// Clamp
			if (fade > 1.0f)
			{
				fade = 1.0f;
			}
			else if (fade <= 0.0f)
			{
				continue;
			}
			hcolor[3] = fade;

			CG_Text_PaintAligned(s.chat_x, cgs.screenYSize - s.chat_offset_y - ((cgs.chatPos - i - 1) * s.chat_text_h), cgs.chatMsgs[i % chatHeight],
				s.f * 0.4f, UI_LEFT, hcolor, &cgs.media.freeSansBoldFont);
		}
	}
}


/*
=================
CG_DrawWeaverNotify
Based on CG_DrawWeaverChat.
=================
*/
void CG_DrawWeaverNotify(void)
{
	int             i;
	vec4_t          hcolor;
	int             notifyHeight;
	int             mode;
	float           fade;

	if(cg_notifyHeight.integer < NOTIFY_HEIGHT)
		notifyHeight = cg_notifyHeight.integer;
	else
		notifyHeight = NOTIFY_HEIGHT;
	if(notifyHeight <= 0)
		return;					// disabled

	if(cgs.notifyLastPos != cgs.notifyPos)
	{
		if(cg.time - cgs.notifyMsgTimes[cgs.notifyLastPos % notifyHeight] > cg_notifyTime.integer)
		{
			cgs.notifyLastPos++;
		}

		trap_R_SetColor(NULL);

		Vector4Copy(colorFull, hcolor);

		for(i = cgs.notifyPos - 1; i >= cgs.notifyLastPos; i--)
		{
			mode = cgs.notifyModes[i % notifyHeight];

			// Only fade for the last 1/3 of the time.
			fade = 3.0f - (3.0f * ((cg.time - cgs.notifyMsgTimes[i % notifyHeight]) / (float)cg_notifyTime.integer));
			// Clamp
			if (fade > 1.0f)
			{
				fade = 1.0f;
			}
			else if (fade <= 0.0f)
			{
				continue;
			}
			hcolor[3] = fade;

			CG_Text_PaintAligned(s.notify_x, s.notify_y + ((cgs.notifyPos - i - 1) * s.notify_text_h), cgs.notifyMsgs[i % notifyHeight],
				s.f * 0.4f, UI_LEFT, hcolor, &cgs.media.freeSansBoldFont);
		}
	}
}

/*
=====================
ColorHealPercent
Set colorHealth based on amount of HP: high => green, low => red.
=====================
*/
void ColorHealPercent(vec4_t colorHealth, int healthPC)
{
	if(healthPC < 0)
	{
		colorHealth[0] = 0.0f;
		colorHealth[1] = 0.0f;
		colorHealth[2] = 0.0f;
	}
	else
	{
		colorHealth[0] = (100 - healthPC) / 100.0f;
		colorHealth[1] = healthPC / 100.0f;
		colorHealth[2] = 0.0f;
	}
}

void CG_DrawTargetPlayerName(float x, float y, int clientNum, int lastTime)
{
	float          *color;
	char           *name;
	int             healthPC;
	vec4_t          colorHealth;
	float           w;

	name = cgs.clientinfo[clientNum].name;
	healthPC = cgs.clientinfo[clientNum].health;

	// Only show names for players on the same team
	if (cgs.clientinfo[clientNum].team != cg.snap->ps.persistant[PERS_TEAM])
	{
		return;
	}

	// Check time for fade out
	color = CG_FadeColor(lastTime, 1000);
	if(!color)
	{
		trap_R_SetColor(NULL);
		return;
	}

	// Draw name
	color[3] *= 0.65f;
	w = CG_Text_Width(name, 0.25f, 0, &cgs.media.freeSansBoldFont);
	CG_Text_Paint(x - (w / 2), y, 0.25f, color, name, 0, 0, 0, &cgs.media.freeSansBoldFont);

	// Background
	
	trap_R_DrawStretchPic(x - (s.cross_health_w/2) - 3, y - s.cross_health_h - 3, s.cross_health_w + 6, s.cross_health_h + 8, 0, 0, 1, 1, cgs.media.weaverBarExt);

	// Cap to 100 for bar
	if(healthPC > 100)
	{
		healthPC = 100;
	}

	// Calculate HP bar color
	ColorHealPercent(colorHealth, healthPC);
	colorHealth[3] = color[3];

	// HP Bar is horizontal
	CG_DrawFillRect(x - (s.cross_health_w/2), y - s.cross_health_h, s.cross_health_w * (healthPC/100.0f), s.cross_health_h, colorHealth, DRFD_RIGHT);
	trap_R_SetColor(NULL);
}

/*
=====================
CG_DrawWeaverCrosshairNames
=====================
*/
void CG_DrawWeaverCrosshairNames(void)
{
	float           x, y;

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

	x = cgs.screenXSize / 2;
	if(cg_drawStatus.integer == 3)
	{
		//think this is a better place
		y = (2 * cgs.screenYSize) / 3;
	}
	else
	{
		y = cgs.screenYSize / 3;
	}

	CG_DrawTargetPlayerName(x, y, cg.crosshairClientNum, cg.crosshairClientTime);
}

/*
================
CG_DrawStatusBarWeaver
================
*/
static void CG_DrawWeaverStatusBar(void)
{
	char           *hpString;

	int             currentHealth = cg.snap->ps.stats[STAT_HEALTH];
	int             currentHealthPC = currentHealth;
	int             currentStamina = cg.snap->ps.stats[STAT_STAMINA];
	float           hpFraction = currentHealth / 100.0f;
	float           stFraction = (currentStamina / MAX_STAMINA);

	vec4_t          colorHealth;

	float power_full_w = s.power * cg.snap->ps.stats[STAT_MAX_POWER];
	//float power_avil_w = s.power * cg.snap->ps.stats[STAT_POWER];
	float power_full_mid_w = power_full_w - (s.power_offset_left_w + s.power_offset_right_w);

	// Cap to 100 for bar, currentHealth may be higher when displayed as digits
	if(currentHealthPC > 100)
	{
		currentHealthPC = 100;
	}

	// Calculate HP bar color
	ColorHealPercent(colorHealth, currentHealthPC);
	colorHealth[3] = 1.0f;

	// Don't let HP fraction go negative for wounded players
	if(hpFraction < 0.0f)
	{
		hpFraction = 0.0f;
	}

#if 0
	/*
	//cent = &cg_entities[cg.snap->ps.clientNum];
	//ps = &cg.snap->ps;

	VectorClear(angles);

	if(cg.predictedPlayerState.powerups[PW_REDFLAG])
	{
		CG_DrawStatusBarFlag(185 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE + ICON_SIZE, TEAM_RED);
	}
	else if(cg.predictedPlayerState.powerups[PW_BLUEFLAG])
	{
		CG_DrawStatusBarFlag(185 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE + ICON_SIZE, TEAM_BLUE);
	}
	else if(cg.predictedPlayerState.powerups[PW_NEUTRALFLAG])
	{
		CG_DrawStatusBarFlag(185 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE + ICON_SIZE, TEAM_FREE);
	}
	*/
#endif

	// HP string
	hpString = va("%iHP", currentHealth);
	CG_Text_PaintAligned(cgs.screenXSize - s.health_val_offset_x, cgs.screenYSize - s.health_val_offset_y, hpString, s.f * 0.5f, UI_RIGHT, colorWhite, &cgs.media.freeSansBoldFont);

	if(currentHealth > 0)
	{
		// HP bar
		CG_DrawFillRect(cgs.screenXSize - s.health_offset_x, cgs.screenYSize - s.health_offset_y, s.health_w, s.health_h * hpFraction, colorHealth, DRFD_UP);
		trap_R_SetColor(colorHealth);
		trap_R_DrawStretchPic(cgs.screenXSize - s.health_offset_x, cgs.screenYSize - s.health_offset_y, 32, 16, 0, 0, 1, 1, cgs.media.hpBot);
		trap_R_SetColor(NULL);

		// Stamina bar
		CG_DrawFillRect(cgs.screenXSize - s.stamina_offset_x, cgs.screenYSize - s.stamina_offset_y, s.stamina_w, s.stamina_h * stFraction, colorFull, DRFD_UP);
		trap_R_SetColor(colorFull);
		trap_R_DrawStretchPic(cgs.screenXSize - s.stamina_offset_x, cgs.screenYSize - s.stamina_offset_y, 16, 16, 0, 0, 1, 1, cgs.media.stBot);
		trap_R_SetColor(NULL);
	}

	// Protect bars
	if(cg.predictedPlayerState.stats[STAT_AIRPROTECT] > 0)
	{
		CG_DrawFillRect(cgs.screenXSize - s.protect_1_offset_x, cgs.screenYSize - s.protect_1_offset_y, s.protect_w,
					  (s.protect_h * ((float)cg.predictedPlayerState.stats[STAT_AIRPROTECT] / (float)WEAVE_PROTECTAIR)),
					  colorAir, DRFD_UP);
	}
	if(cg.predictedPlayerState.stats[STAT_FIREPROTECT] > 0)
	{
		CG_DrawFillRect(cgs.screenXSize - s.protect_2_offset_x, cgs.screenYSize - s.protect_2_offset_y, s.protect_w,
					  (s.protect_h * ((float)cg.predictedPlayerState.stats[STAT_FIREPROTECT] / (float)WEAVE_PROTECTFIRE)),
					  colorFire, DRFD_UP);
	}
	if(cg.predictedPlayerState.stats[STAT_EARTHPROTECT] > 0)
	{
		CG_DrawFillRect(cgs.screenXSize - s.protect_3_offset_x, cgs.screenYSize - s.protect_3_offset_y, s.protect_w,
					  (s.protect_h * ((float)cg.predictedPlayerState.stats[STAT_EARTHPROTECT] / (float)WEAVE_PROTECTEARTH)),
					  colorEarth, DRFD_UP);
	}
	if(cg.predictedPlayerState.stats[STAT_WATERPROTECT] > 0)
	{
		CG_DrawFillRect(cgs.screenXSize - s.protect_4_offset_x, cgs.screenYSize - s.protect_4_offset_y, s.protect_w,
					  (s.protect_h * ((float)cg.predictedPlayerState.stats[STAT_WATERPROTECT] / (float)WEAVE_PROTECTWATER)),
					  colorWater, DRFD_UP);
	}


	// Hud Decoration
	WeaverSetTeamColor();

	trap_R_DrawStretchPic(cgs.screenXSize-s.s256, cgs.screenYSize-s.s512, s.s256, s.s512, 0, 0, 1, 1, cgs.media.weaverCorner);
	trap_R_DrawStretchPic(cgs.screenXSize-(s.s256+s.s128), cgs.screenYSize-s.s32, s.s128, s.s32, 0, 0, 1, 1, cgs.media.weaverBarExt0);
	trap_R_DrawStretchPic(cgs.screenXSize-(s.s256+(power_full_mid_w)), cgs.screenYSize-s.s32, (power_full_mid_w-s.s128), s.s32, 0, 0, 1, 1, cgs.media.weaverBarExt);
	trap_R_DrawStretchPic(cgs.screenXSize-(s.s256+s.s256+power_full_mid_w), cgs.screenYSize-s.s64, s.s256, s.s64, 0, 0, 1, 1, cgs.media.weaverBarEnd);

	trap_R_SetColor(NULL);
}

/*
=================
CG_DrawWeaverStroke

Animation for each stroke in the thread (ie, a line or something)
=================
*/
static void CG_DrawWeaverStroke(float x, float y)
{
	float           w, h;
	float           xo, yo;
	float           moveScale;
	qhandle_t       weaverThread;

	//Com_Printf("cg.predictedPlayerState.eFlags: %i \n", cg.predictedPlayerState.eFlags);
	if(!(cg.predictedPlayerState.eFlags & EF_WEAVEA) && !(cg.predictedPlayerState.eFlags & EF_WEAVED))
	{
		return;
	}

	if(cg_drawWeaverThread.integer < 1)
	{
		return;
	}

	w = h = 16.0;

	moveScale = (float)cg_weaverThreadMoveScale.value * 1.0;

	xo = cg.predictedPlayerState.stats[STAT_THREADX] * moveScale;
	yo = cg.predictedPlayerState.stats[STAT_THREADY] * moveScale;

	weaverThread = cgs.media.weaverSenseDot3;

	trap_R_DrawStretchPic(x + xo - (0.5 * w), y - yo - (0.5 * w), w, w, 0, 0, 1, 1, weaverThread);

}

/*
=================
CG_DrawWeaverTutorialWeave

Adds tutorial infromation to the weaver disk.
=================
*/
static void CG_DrawWeaverTutorialWeave(float x, float y, float size, int weaveID, int element, int elementCount, int depth)
{
	weaver_weaveCGInfo *weaveInfo;
	float           xo, yo;
	int             offsetCount;
	char           *str;

	//Offsets
	xo = tutTextOffset[element][0] * size;
	yo = tutTextOffset[element][1] * size;
	if(cg_weaverTutorial.integer >= 2)
	{
		//Display name of possible spells
		weaveInfo = &cg_weaves[weaveID];
		if(weaveInfo && weaveInfo->info.nameP)
		{
			str = va("(%d) %s", depth, weaveInfo->info.nameP);
			//half length * char width
			offsetCount = elementCount * CG_Text_Height("(", 0.36f, 0, &cgs.media.freeSansBoldFont);
			if(depth == 1)
			{
				// Green
				CG_Text_PaintAligned(x + xo, y - (yo + offsetCount), str, 0.34f, UI_CENTER, colorGreen, &cgs.media.freeSansBoldFont);
			}
			else
			{
				// White
				CG_Text_PaintAligned(x + xo, y - (yo + offsetCount), str, 0.34f, UI_CENTER, colorWhite, &cgs.media.freeSansBoldFont);
			}
		}
	}
}

/*
=================
CG_DrawWeaverTutorialWeave

Adds tutorial infromation to the weaver disk.
=================
*/
static void CG_DrawWeaverTutorialChildren(weaver_threadsMap_t * current, float x, float y, float size, int element, int depth)
{
	int             i;
	int             elementUsed;

	// This is the element which the Weave will be drawn on, it will not change after the first level.
	elementUsed = element;

	for(i = WVP_NONE; i < WVP_NUMBER; i++)
	{
		if(element == -1)
		{
			elementUsed = i;
		}
		if(powerCount[elementUsed] > cg_weaverTutorialMaxElement.integer)
		{
			// This element has a list of too many spells, skip drawing the rest
			continue;
		}
		if(current->next[i] == NULL)
		{
			continue;
		}
		if(current->next[i]->weaveA > 0)
		{
			CG_DrawWeaverTutorialWeave(x, y, size, current->next[i]->weaveA, elementUsed, powerCount[elementUsed], depth + 1);
			// Add to count of spells listed under this element
			powerCount[elementUsed]++;
		}
		if(depth < cg_weaverTutorialMaxDepth.integer)
		{
			// While we haven't gone too deep yet, recurse
			CG_DrawWeaverTutorialChildren(current->next[i], x, y, size, elementUsed, depth + 1);
		}
	}
}

/*
=================
CG_DrawWeaverTutorial

Adds tutorial infromation to the weaver disk.
=================
*/
static void CG_DrawWeaverTutorial(float x, float y, float size)
{
	weaver_weaveCGInfo *weaveInfo;
	weaver_threadsMap_t *current;
	char           *str;

	current = ThreadsTree(cg.predictedPlayerEntity.pe.threads);

	if((cg_weaverTutorial.integer >= 2) && (current->weaveA >= 0))
	{
		//Display name of current spell
		weaveInfo = &cg_weaves[current->weaveA];
		if(weaveInfo && weaveInfo->info.nameP)
		{
			str = va("Release +weave for %s", weaveInfo->info.nameP);
			CG_Text_PaintAligned(cg.refdef.width/2, 80, str, 0.4f, UI_CENTER, colorWhite, &cgs.media.freeSansBoldFont);
		}
	}

	memset(&powerCount, 0, sizeof(int) * WVP_NUMBER);

	//TODO: only calculate this on change of pe.threads.
	CG_DrawWeaverTutorialChildren(current, x, y, size, -1, 0);
}

/*
=================
CG_DrawWeaverDisc

Draws the disc in the center of the screen
=================
*/
static void CG_DrawWeaverDisc(void)
{
	float           w, h;
	float           x, y;

	//float           moveScale;
	qhandle_t       weaverDisc;
	int             da;
	//char           *thread;

	//Com_Printf("cg.predictedPlayerState.eFlags: %i \n", cg.predictedPlayerState.eFlags);
	if(!(cg.predictedPlayerState.eFlags & EF_WEAVEA) && !(cg.predictedPlayerState.eFlags & EF_WEAVED))
	{
		return;
	}

	da = cg_drawWeaverDisc.integer;
	w = h = cg_weaverDiscSize.integer * (cgs.screenMinSize / 100);

	x = cg_weaverDiscX.integer + (cgs.screenXSize / 2);
	y = cg_weaverDiscY.integer + (cgs.screenYSize / 2);

	if(da >= 1 && da <= NUM_WEAVERDISCS)
	{
		//TODO: reimplement move scale
		//moveScale = (float)cg_weaverDiscMoveScale.value * 1.0;

		//thread = va("%i x, %i y", cg.predictedPlayerState.stats[STAT_THREADX], cg.predictedPlayerState.stats[STAT_THREADY]);
		//CG_Text_PaintAligned(8, 17, thread, 0.25f, UI_LEFT, colorWhite, &cgs.media.freeSansBoldFont);

		//x += cg.predictedPlayerState.stats[STAT_THREADX] * moveScale;
		//y -= cg.predictedPlayerState.stats[STAT_THREADY] * moveScale;

		weaverDisc = cgs.media.weaverDiscShader[da];

		trap_R_DrawStretchPic(x - (0.5 * w), y - (0.5 * w), w, w, 0, 0, 1, 1, weaverDisc);
	}

	CG_DrawWeaverStroke(x, y);

	CG_DrawWeaverTutorial(x, y, (0.25 * cgs.screenMinSize));
}

int HeldSpecial_healing;

/*
=================
CG_DrawWeaverSpellSpecialInit

Prepare for drawing special information for some heldWeaves/weaveEffects.
This allows us to have a state while drawing info for the heldWeave/weaveEffect,
and do something smarter if the same type of weave shows up multiple times.
=================
*/
void CG_DrawWeaverSpellSpecialInit(void)
{
	// If we have multiple healing spells, we need to keep track of where we can display them.
	HeldSpecial_healing = 0;
}

/*
=================
CG_DrawWeaverEffectSpecial

If one of our weaveEffects requires some extra info to be shown on the hud, do it from here.
=================
*/
void CG_DrawWeaverEffectSpecial(centity_t * weave)
{
	switch (weave->currentState.weapon)
	{
		case WVW_D_WATER_HEAL_S:
		case WVW_D_WATER_HEAL_M:
			// This is an effect we own
			if (weave->currentState.otherEntityNum == cg.clientNum)
			{
				// Show who we're healing
				if (HeldSpecial_healing == 0) {
					CG_Text_PaintAligned(cgs.screenXSize / 4.0f, 15.0f, "Healing...", 0.25f, UI_LEFT, colorWhite, &cgs.media.freeSansBoldFont);
				}
				CG_DrawTargetPlayerName(cgs.screenXSize / 4.0f, (25.0f * HeldSpecial_healing) + 35.0f, weave->currentState.generic1, cg.time);
				HeldSpecial_healing++;
			}
			break;
		default:
			return;
	}
}

/*
=================
CG_DrawWeaverHeldSpecial

If one of our heldWeave requires some extra info to be shown on the hud, do it from here.

x is the x location of the center of the held weave icon
=================
*/
void CG_DrawWeaverHeldSpecial(centity_t * heldWeave, float x)
{
	centity_t      *cent;
	switch (heldWeave->currentState.weapon)
	{
		case WVW_D_WATER_HEAL_S:
		case WVW_D_WATER_HEAL_M:
			if (CG_HeldWeave_GetState(heldWeave) != WST_INPROCESS) break;
			cent = &cg_entities[heldWeave->currentState.groundEntityNum];
			if (cent->currentState.eType != ET_WEAVE_EFFECT) break;
			if (cent->currentState.otherEntityNum2 != cent->currentState.number) break;
			CG_DrawWeaverEffectSpecial(cent);
			break;
		default:
			return;
	}
}

/*
=================
CG_DrawWeaverHeld

Draws the disc in the center of the screen
=================
*/
static void CG_DrawWeaverHeld(void)
{
	int             i;
	char           *thread;
	centity_t      *cent;
	weaver_weaveCGInfo *weaveInfo;
	float           x, y;
	qhandle_t       frameShader;

	float           power_spell_w;

	float           y_div = cgs.screenYSize - s.power_div_h;
	//float           y_text = cgs.screenYSize - (s.power_div_h + (s.f * 20));
	float           y_icon = cgs.screenYSize - (s.f * 60);

	float power_full_w = cg.snap->ps.stats[STAT_MAX_POWER];
	float power_avil_w = cg.snap->ps.stats[STAT_POWER];
	float power_used_w = s.power * (power_full_w - power_avil_w);

	x = cgs.screenXSize - s.power_offset_x;
	y = cgs.screenYSize - s.power_offset_y - s.power_spell_h;

	// Draw bar used (including current casting)
	CG_DrawFillRect(x, y, power_used_w, s.power_spell_h, colorEmpty, DRFD_LEFT);

	// Prepare for a new round of drawing special hud info.
	CG_DrawWeaverSpellSpecialInit();

	// Draw each held weave
	for(i = MIN_WEAPON_WEAVE; i < MAX_WEAPONS; i++)
	{
		if(cg.predictedPlayerState.ammo[i] > 0)
		{
			// Aquire heldWeave entity
			cent = &cg_entities[cg.predictedPlayerState.ammo[i]];
			weaveInfo = &cg_weaves[cent->currentState.weapon];

			// Width of bar for this spell
			power_spell_w = s.power * CG_HeldWeave_GetPower(cent);

			// Draw power bar
			CG_DrawFillRect(x, y, power_spell_w, s.power_spell_h, colorWhite, DRFD_LEFT);

			// Draw Spell Icon
			trap_R_DrawStretchPic(x - s.spellicon_w, y_icon - s.spellicon_w, s.spellicon_w, s.spellicon_w, 0, 0, 1, 1, weaveInfo->icon);

			/*
			thread =
				va("%i: e=%i w=%d p=%d a=%d/%d", i, cg.predictedPlayerState.ammo[i], cent->currentState.weapon,
				   cent->currentState.generic1, cent->currentState.torsoAnim, weaveInfo->info.castCharges);
			CG_Text_PaintAligned(x, y_icon + 8, thread, 0.20f, UI_RIGHT, colorWhite, &cgs.media.freeSansBoldFont);
			*/

			thread = va("%d/%d", cent->currentState.torsoAnim, weaveInfo->info.castCharges);
			CG_Text_PaintAligned(x - (s.spellicon_w/2), y_icon + 9, thread, s.f * 0.22f, UI_CENTER, colorWhite, &cgs.media.freeSansBoldFont);

			// Draw frame
			if(CG_HeldWeave_GetState(cent) == WST_INPROCESS)
			{
				frameShader = cgs.media.spell_frame[2 + (i == cg.weaponSelect)];
			}
			else
			{
				frameShader = cgs.media.spell_frame[(i == cg.weaponSelect)];
			}
			trap_R_DrawStretchPic(x - s.spellicon_frame_offset_x, y_icon - s.spellicon_frame_offset_x, s.spellicon_frame_w, s.spellicon_frame_w, 0, 0, 1, 1, frameShader);

			// Draw any extra info for this heldWeave.
			CG_DrawWeaverHeldSpecial(cent, x);

			// Move left
			x -= power_spell_w;

			// Division marker
			trap_R_DrawStretchPic(x - (s.power_div_w / 2), y_div, s.power_div_w, s.power_div_h, 0, 0, 1, 1, cgs.media.weaverBarDiv);
		}
	}
}

/*
=================
CG_DrawWeaverPowerups

Statuses such as shielded, poisoned, protection
=================
*/
static void CG_DrawWeaverPowerups(void)
{
	playerState_t  *ps;
	//char           *timeRemaining;
	int             i;
	int             t;
	int             x, y;
	centity_t      *obj;

	ps = &cg.snap->ps;

	if(ps->stats[STAT_HEALTH] <= 0)
	{
		return;
	}

	// draw the icons and timers
	x = 48;
	y = 480 - 28 - HUD_STATUS_HEIGHT;
	for(i = 0; i < PW_NUM_POWERUPS; i++)
	{
		if(!ps->powerups[i])
		{
			continue;
		}
		if(!cgs.media.weaverStatus[i])
		{
			continue;
		}
		t = ps->powerups[i] - cg.time;
		if(t < 0)
		{
			continue;
		}
		/*
		if(t < 999000)
		{
			timeRemaining = va("%d", t / 1000 + 1);
			CG_Text_PaintAligned(x + HUD_STATUS_WIDTH, y + 11, timeRemaining, 0.125f, UI_LEFT, colorWhite, &cgs.media.freeSansBoldFont);
		}
		*/
		CG_DrawPic(x, y, HUD_STATUS_WIDTH, HUD_STATUS_HEIGHT, cgs.media.weaverStatus[i]);

		y -= HUD_STATUS_HEIGHT;
	}

	y -= HUD_STATUS_HEIGHT;

	// Draw objective icons
	obj = CG_ObjItem(&cg.predictedPlayerEntity.pe, cg.predictedPlayerState.clientNum);
	if(obj != NULL)
	{
		CG_DrawPic(x, y, HUD_STATUS_WIDTH, HUD_STATUS_HEIGHT, cgs.media.objItemIcon);

		y -= HUD_STATUS_HEIGHT;
	}
}

/*
===================
CG_DrawBindsTutorial
===================
*/
static void CG_DrawBindsTutorial(void)
{
	if( !cg_tutorial.integer )
		return;

	CG_Text_PaintAlignedBlock(5, 80, CG_TutorialText(), s.f * 0.30f, UI_LEFT, colorWhite, &cgs.media.freeSansBoldFont);
}

/*
=================
CG_DrawWeaverHUD

Calls all HUD drawing for players.
=================
*/
void CG_DrawWeaverHUD(void)
{
	//Held Weaves
	CG_DrawWeaverHeld();

	//Health and Power
	CG_DrawWeaverStatusBar();

	//Weaver Disc, includes strokes and tutorial
	CG_DrawWeaverDisc();

	//Weaver Sense
	//CG_DrawWeaveSense();

	//Binds tutorial
	CG_DrawBindsTutorial();

	//Powerups
	CG_DrawWeaverPowerups();
}
