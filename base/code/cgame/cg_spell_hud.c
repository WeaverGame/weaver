/*
===========================================================================
This file is part of the weaver cgame.

It has weaver HUD.
===========================================================================
*/

#include "cg_local.h"
#include "cg_spell_util.h"

vec4_t          colorAir = { 0.937f, 0.90f, 0.00f, 0.7f };
vec4_t          colorFire = { 0.94f, 0.16f, 0.16f, 0.7f };
vec4_t          colorEarth = { 0.0f, 0.87f, 0.00f, 0.7f };
vec4_t          colorWater = { 0.0f, 0.32f, 0.906f, 0.7f };

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
void CG_DrawFillRect(float x, float y, float width, float height, float *color, DrawRectFillDir dir)
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

/*
================
CG_DrawStatusBarWeaver
================
*/
static void CG_DrawWeaverStatusBar(void)
{
	int             currentHealth = cg.snap->ps.stats[STAT_HEALTH];
	int             currentStamina = cg.snap->ps.stats[STAT_STAMINA];
	float           hpFraction = currentHealth / 100.0;

	const float     health_offset_x = 50.0f;
	const float     health_offset_y = 130.0f;
	float           health_w = 28.0f;
	float           health_h = 270.0f * hpFraction;

	const float     health_val_offset_x = 5.0f;
	const float     health_val_offset_y = 20.0f;

	const float     stamina_offset_x = 17.0f;
	const float     stamina_offset_y = 135.0f;
	float           stamina_w = 14.0f;
	float           stamina_h = 240.0f * (currentStamina / MAX_STAMINA);

	const float     power_offset_left_w = 180.0f;
	const float     power_offset_right_w = 105.0f;

	const vec4_t    colorTeamBlue = { 0.0f, 0.0f, 1.0f, 0.5f };	// blue
	const vec4_t    colorTeamRed = { 1.0f, 0.0f, 0.0f, 0.5f };	// red

	int             rectx, recty, rectw, recth;
	char           *hpString;
	char           *pString;
	char           *protectString;

	const float     protect_1_offset_x = 87.0f;
	const float     protect_1_offset_y = 120.0f;
	const float     protect_2_offset_x = 78.0f;
	const float     protect_2_offset_y = 120.0f;
	const float     protect_3_offset_x = 69.0f;
	const float     protect_3_offset_y = 120.0f;
	const float     protect_4_offset_x = 60.0f;
	const float     protect_4_offset_y = 120.0f;
	const float     protect_w = 6.0f;
	const float     protect_h = 200.0f;

	vec4_t          colorHealth;
	vec4_t          colorWhite = { 1.0f, 1.0f, 1.0f, 1.0f };
	vec4_t          colorFull = { 1.0f, 1.0f, 1.0f, 1.0f };
	vec4_t          colorEmpty = { 1.0f, 1.0f, 1.0f, 0.3f };

	float power_full_w = cg.snap->ps.stats[STAT_MAX_POWER] / 2;
	float power_avil_w = cg.snap->ps.stats[STAT_POWER] / 2;
	float power_full_mid_w = power_full_w - (power_offset_left_w + power_offset_right_w);

	colorHealth[3] = 1.0f;

	if(currentHealth > 100)
	{
		currentHealth = 100;
	}
	if(currentHealth < 0)
	{
		colorHealth[0] = 0.0f;
		colorHealth[1] = 0.0f;
		colorHealth[2] = 0.0f;
	}
	else
	{
		colorHealth[0] = (100 - currentHealth) / 100.0f;
		colorHealth[1] = currentHealth / 100.0f;
		colorHealth[2] = 0.0f;
	}

	currentHealth = cg.snap->ps.stats[STAT_HEALTH];

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

	hpString = va("%iHP", currentHealth);
	CG_Text_PaintAligned(cgs.screenXSize - health_val_offset_x, cgs.screenYSize - health_val_offset_y, hpString, 0.5f, UI_RIGHT, colorWhite, &cgs.media.freeSansBoldFont);

	CG_DrawFillRect(cgs.screenXSize - health_offset_x, cgs.screenYSize - health_offset_y, health_w, health_h, colorHealth, DRFD_UP);
	trap_R_SetColor(colorHealth);
	trap_R_DrawStretchPic(cgs.screenXSize - health_offset_x, cgs.screenYSize - health_offset_y, 32, 16, 0, 0, 1, 1, cgs.media.hpBot);
	trap_R_SetColor(NULL);

	CG_DrawFillRect(cgs.screenXSize - stamina_offset_x, cgs.screenYSize - stamina_offset_y, stamina_w, stamina_h, colorFull, DRFD_UP);

#if 1
	if(cg.predictedPlayerState.stats[STAT_AIRPROTECT] > 0)
	{
		CG_DrawFillRect(cgs.screenXSize - protect_1_offset_x, cgs.screenYSize - protect_1_offset_y, protect_w,
					  (protect_h * ((float)cg.predictedPlayerState.stats[STAT_AIRPROTECT] / (float)WEAVE_PROTECTAIR)),
					  colorAir, DRFD_UP);
	}
	if(cg.predictedPlayerState.stats[STAT_FIREPROTECT] > 0)
	{
		CG_DrawFillRect(cgs.screenXSize - protect_2_offset_x, cgs.screenYSize - protect_2_offset_y, protect_w,
					  (protect_h * ((float)cg.predictedPlayerState.stats[STAT_FIREPROTECT] / (float)WEAVE_PROTECTFIRE)),
					  colorFire, DRFD_UP);
	}
	if(cg.predictedPlayerState.stats[STAT_EARTHPROTECT] > 0)
	{
		CG_DrawFillRect(cgs.screenXSize - protect_3_offset_x, cgs.screenYSize - protect_3_offset_y, protect_w,
					  (protect_h * ((float)cg.predictedPlayerState.stats[STAT_EARTHPROTECT] / (float)WEAVE_PROTECTEARTH)),
					  colorEarth, DRFD_UP);
	}
	if(cg.predictedPlayerState.stats[STAT_WATERPROTECT] > 0)
	{
		CG_DrawFillRect(cgs.screenXSize - protect_4_offset_x, cgs.screenYSize - protect_4_offset_y, protect_w,
					  (protect_h * ((float)cg.predictedPlayerState.stats[STAT_WATERPROTECT] / (float)WEAVE_PROTECTWATER)),
					  colorWater, DRFD_UP);
	}

	recty -= HUD_PROTECT_HEIGHT;

	if(cg.predictedPlayerState.stats[STAT_AIRPROTECT] > 0)
	{
		protectString = va("%d", cg.predictedPlayerState.stats[STAT_AIRPROTECT]);
		CG_Text_PaintAligned(rectx, recty, protectString, 0.125f, UI_LEFT, colorAir, &cgs.media.freeSansBoldFont);
		recty += 7;
	}
	if(cg.predictedPlayerState.stats[STAT_FIREPROTECT] > 0)
	{
		protectString = va("%d", cg.predictedPlayerState.stats[STAT_FIREPROTECT]);
		CG_Text_PaintAligned(rectx, recty, protectString, 0.125f, UI_LEFT, colorFire, &cgs.media.freeSansBoldFont);
		recty += 7;
	}
	if(cg.predictedPlayerState.stats[STAT_EARTHPROTECT] > 0)
	{
		protectString = va("%d", cg.predictedPlayerState.stats[STAT_EARTHPROTECT]);
		CG_Text_PaintAligned(rectx, recty, protectString, 0.125f, UI_LEFT, colorEarth, &cgs.media.freeSansBoldFont);
		recty += 7;
	}
	if(cg.predictedPlayerState.stats[STAT_WATERPROTECT] > 0)
	{
		protectString = va("%d", cg.predictedPlayerState.stats[STAT_WATERPROTECT]);
		CG_Text_PaintAligned(rectx, recty, protectString, 0.125f, UI_LEFT, colorWater, &cgs.media.freeSansBoldFont);
		recty += 7;
	}
#endif

	// Hud Decoration
	if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE)
	{
		trap_R_SetColor(colorTeamBlue);
	}
	else if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED)
	{
		trap_R_SetColor(colorTeamRed);
	}

	trap_R_DrawStretchPic(cgs.screenXSize-256, cgs.screenYSize-512, 256, 512, 0, 0, 1, 1, cgs.media.weaverCorner);
	trap_R_DrawStretchPic(cgs.screenXSize-(256+128), cgs.screenYSize-32, 128, 32, 0, 0, 1, 1, cgs.media.weaverBarExt0);
	trap_R_DrawStretchPic(cgs.screenXSize-(256+(power_full_mid_w)), cgs.screenYSize-32, (power_full_mid_w-128), 32, 0, 0, 1, 1, cgs.media.weaverBarExt);
	trap_R_DrawStretchPic(cgs.screenXSize-(256+power_full_mid_w+256), cgs.screenYSize-64, 256, 64, 0, 0, 1, 1, cgs.media.weaverBarEnd);

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
	int             offset;
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

	elementUsed = element;

	for(i = WVP_NONE; i < WVP_NUMBER; i++)
	{
		if(element == -1)
		{
			elementUsed = i;
		}
		if(powerCount[elementUsed] > 4)
		{
			continue;
		}
		if(current->next[i] != NULL)
		{
			if(current->next[i]->weaveA > 0)
			{
				CG_DrawWeaverTutorialWeave(x, y, size, current->next[i]->weaveA, elementUsed, powerCount[elementUsed], depth + 1);
				powerCount[elementUsed]++;
			}
			else
			{
				if(depth < 6)
				{
					CG_DrawWeaverTutorialChildren(current->next[i], x, y, size, elementUsed, depth + 1);
				}
			}
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
	int             offset;
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
	char           *thread;

	//Com_Printf("cg.predictedPlayerState.eFlags: %i \n", cg.predictedPlayerState.eFlags);
	if(!(cg.predictedPlayerState.eFlags & EF_WEAVEA) && !(cg.predictedPlayerState.eFlags & EF_WEAVED))
	{
		return;
	}

	da = cg_drawWeaverDisc.integer;
	w = h = cg_weaverDiscSize.integer * (cgs.screenMinSize / 100);

	if(da >= 1 && da <= NUM_WEAVERDISCS)
	{
		//TODO: reimplement move scale
		//moveScale = (float)cg_weaverDiscMoveScale.value * 1.0;

		thread = va("%i x, %i y", cg.predictedPlayerState.stats[STAT_THREADX], cg.predictedPlayerState.stats[STAT_THREADY]);
		CG_Text_PaintAligned(8, 17, thread, 0.25f, UI_LEFT, colorWhite, &cgs.media.freeSansBoldFont);

		//x += cg.predictedPlayerState.stats[STAT_THREADX] * moveScale;
		//y -= cg.predictedPlayerState.stats[STAT_THREADY] * moveScale;

		weaverDisc = cgs.media.weaverDiscShader[da];

		x = cg_weaverDiscX.integer + cg.refdef.x + (0.5 * cg.refdef.width);
		y = cg_weaverDiscY.integer + cg.refdef.y + (0.5 * cg.refdef.height);

		trap_R_DrawStretchPic(x - (0.5 * w), y - (0.5 * w), w, w, 0, 0, 1, 1, weaverDisc);
	}
	else
	{
		x = cg_weaverDiscX.integer + cg.refdef.x + (0.5 * cg.refdef.width);
		y = cg_weaverDiscY.integer + cg.refdef.y + (0.5 * cg.refdef.height);
	}

	CG_DrawWeaverStroke(x, y);

	CG_DrawWeaverTutorial(x, y, (0.25 * cgs.screenMinSize));
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
	float           xc;

	vec4_t          colorEmpty = { 1.0f, 1.0f, 1.0f, 0.3f };

	const float     spellicon_w = 80.0f;

	float           power_spell_w;
	const float     power_spell_h = 18.0f;

	const float     power_div_w = 16.0f;
	const float     power_div_h = 64.0f;

	const float     power_offset_left_w = 180.0f;
	const float     power_offset_right_w = 105.0f;

	const float     power_offset_x = 151.0f;
	const float     power_offset_y = 8.0f;

	float           y_div = cgs.screenYSize - power_div_h;
	float           y_text = cgs.screenYSize - (power_div_h + 20);
	float           y_icon = cgs.screenYSize - 60;

	float power_full_w = cg.snap->ps.stats[STAT_MAX_POWER];
	float power_avil_w = cg.snap->ps.stats[STAT_POWER];
	float power_used_w = (power_full_w - power_avil_w) / 2;

	x = cgs.screenXSize - power_offset_x;
	y = cgs.screenYSize - power_offset_y - power_spell_h;

	// Draw bar used (including current casting)
	CG_DrawFillRect(x, y, power_used_w, power_spell_h, colorEmpty, DRFD_LEFT);

	// Draw each held weave
	for(i = MIN_WEAPON_WEAVE; i < MAX_WEAPONS; i++)
	{
		if(cg.predictedPlayerState.ammo[i] > 0)
		{
			// Aquire heldWeave entity
			cent = &cg_entities[cg.predictedPlayerState.ammo[i]];
			weaveInfo = &cg_weaves[cent->currentState.weapon];

			// Width of bar for this spell
			power_spell_w = CG_HeldWeave_GetPower(cent) / 2;

			// Draw power bar
			CG_DrawFillRect(x, y, power_spell_w, power_spell_h, colorWhite, DRFD_LEFT);

			// Draw Spell Icon
			trap_R_DrawStretchPic(x - spellicon_w, y_icon - spellicon_w, spellicon_w, spellicon_w, 0, 0, 1, 1, weaveInfo->icon);

			/*
			thread =
				va("%i: e=%i w=%d p=%d a=%d/%d", i, cg.predictedPlayerState.ammo[i], cent->currentState.weapon,
				   cent->currentState.generic1, cent->currentState.torsoAnim, weaveInfo->info.castCharges);
			CG_Text_PaintAligned(x, y_icon + 8, thread, 0.20f, UI_RIGHT, colorWhite, &cgs.media.freeSansBoldFont);
			*/

			thread = va("%d/%d", cent->currentState.torsoAnim, weaveInfo->info.castCharges);
			CG_Text_PaintAligned(x - (spellicon_w/2), y_icon + 9, thread, 0.22f, UI_CENTER, colorWhite, &cgs.media.freeSansBoldFont);

			// Draw selection marker
			if(i == cg.weaponSelect)
			{
				trap_R_DrawStretchPic(x - spellicon_w, y_icon - spellicon_w, spellicon_w, spellicon_w, 0, 0, 1, 1, cgs.media.weaponSelectShader);
			}

			// Move left
			x -= power_spell_w;

			// Division marker
			trap_R_DrawStretchPic(x - (power_div_w / 2), y_div, power_div_w, power_div_h, 0, 0, 1, 1, cgs.media.weaverBarDiv);
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
	char           *timeRemaining;
	int             i;
	int             t;
	int             x, y;

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
		if(t < 999000)
		{
			timeRemaining = va("%d", t / 1000 + 1);
			CG_Text_PaintAligned(x + HUD_STATUS_WIDTH, y + 11, timeRemaining, 0.125f, UI_LEFT, colorWhite, &cgs.media.freeSansBoldFont);
		}
		CG_DrawPic(x, y, HUD_STATUS_WIDTH, HUD_STATUS_HEIGHT, cgs.media.weaverStatus[i]);

		y -= HUD_STATUS_HEIGHT;
	}

	return;
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
	CG_DrawWeaveSense();

	//Powerups
	CG_DrawWeaverPowerups();
}
