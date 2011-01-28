/*
===========================================================================
This file is part of the weaver cgame.

It has weaver HUD.
===========================================================================
*/

#include "cg_local.h"

vec4_t          colorAir = { 0.937f, 0.90f, 0.00f, 1.0f };
vec4_t          colorFire = { 0.94f, 0.16f, 0.16f, 1.0f };
vec4_t          colorEarth = { 0.0f, 0.87f, 0.00f, 1.0f };
vec4_t          colorWater = { 0.0f, 0.32f, 0.906f, 1.0f };

vec2_t          tutTextOffset[WVP_NUMBER] = { {0.0f, 0.0f},
{0.0f, 1.1f}, {0.778f, 0.778f},
{1.1f, 0.0f}, {0.778f, -0.778f},
{0.0f, -1.1f}, {-0.778f, -0.778f},
{-1.1f, 0.0f}, {-0.778f, 0.778f},
{0.0f, 0.0f}
};

int             powerCount[WVP_NUMBER];

/*
================
CG_FillRectUp

Utility method.
Coordinates are 640*480 virtual values
=================
*/
void CG_FillRectUp(float x, float y, float width, float height, float *color)
{
	CG_FillRect(x, y - height, width, height, color);
}

/*
================
CG_DrawStatusBarWeaver
================
*/
static void CG_DrawWeaverStatusBar(void)
{
	int             rectx, recty, rectw, recth;
	char           *hpString;
	char           *pString;
	char           *protectString;

	vec4_t          colorHealth;
	vec4_t          colorWhite = { 1.0f, 1.0f, 1.0f, 1.0f };
	vec4_t          colorFull = { 1.0f, 1.0f, 1.0f, 1.0f };
	vec4_t          colorEmpty = { 1.0f, 1.0f, 1.0f, 0.3f };

	int             currentHealth = cg.snap->ps.stats[STAT_HEALTH];
	int             currentStamina = cg.snap->ps.stats[STAT_STAMINA];
	int             currentPower = cg.snap->ps.stats[STAT_POWER];
	float           hpFraction = currentHealth / 100.0;
	float           pFraction = currentPower / FULL_POWER;

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

	if(hpFraction < 0)
	{
		hpFraction = 0;
	}

	/*
	if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE)
	{
		colorItem = &colorTeamBlue;
	}
	else if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED)
	{
		colorItem = &colorTeamRed;
	}

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

	//Health
	rectx = 24;
	recty = 480 - 28;

	//CG_DrawRect(float x, float y, float width, float height, float size, const float *color);
	CG_FillRectUp(rectx, recty, HUD_HEALTH_WIDTH, HUD_HEALTH_HEIGHT, colorEmpty);
	CG_FillRectUp(rectx, recty, HUD_HEALTH_WIDTH, HUD_HEALTH_HEIGHT * hpFraction, colorHealth);

	CG_DrawPic(rectx, recty + 2, 12, 12, cgs.media.weaverIconHP);

	hpString = va("%ihp", currentHealth);
	CG_Text_PaintAligned(rectx - 4, recty + 17, hpString, 0.125f, UI_LEFT, colorWhite, &cgs.media.freeSansBoldFont);

	rectx += HUD_HEALTH_WIDTH;

	if(cg.predictedPlayerState.stats[STAT_AIRPROTECT] > 0)
	{
		CG_FillRectUp(rectx, recty, HUD_PROTECT_WIDTH,
					  (HUD_PROTECT_HEIGHT * ((float)cg.predictedPlayerState.stats[STAT_AIRPROTECT] / (float)WEAVE_PROTECTAIR)),
					  colorAir);
		rectx += HUD_PROTECT_WIDTH;
	}
	if(cg.predictedPlayerState.stats[STAT_FIREPROTECT] > 0)
	{
		CG_FillRectUp(rectx, recty, HUD_PROTECT_WIDTH,
					  (HUD_PROTECT_HEIGHT * ((float)cg.predictedPlayerState.stats[STAT_FIREPROTECT] / (float)WEAVE_PROTECTFIRE)),
					  colorFire);
		rectx += HUD_PROTECT_WIDTH;
	}
	if(cg.predictedPlayerState.stats[STAT_EARTHPROTECT] > 0)
	{
		CG_FillRectUp(rectx, recty, HUD_PROTECT_WIDTH,
					  (HUD_PROTECT_HEIGHT * ((float)cg.predictedPlayerState.stats[STAT_EARTHPROTECT] / (float)WEAVE_PROTECTEARTH)),
					  colorEarth);
		rectx += HUD_PROTECT_WIDTH;
	}
	if(cg.predictedPlayerState.stats[STAT_WATERPROTECT] > 0)
	{
		CG_FillRectUp(rectx, recty, HUD_PROTECT_WIDTH,
					  (HUD_PROTECT_HEIGHT * ((float)cg.predictedPlayerState.stats[STAT_WATERPROTECT] / (float)WEAVE_PROTECTWATER)),
					  colorWater);
		rectx += HUD_PROTECT_WIDTH;
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

	//Stamina
	rectx = 4;
	recty = 480 - 28;
	rectw = 12;
	recth = 90;

	//CG_DrawRect(float x, float y, float width, float height, float size, const float *color);
	CG_FillRectUp(rectx, recty, rectw, recth, colorEmpty);
	CG_FillRectUp(rectx, recty, rectw, recth * (currentStamina / MAX_STAMINA), colorFull);

	//Power
	rectx = 640 - 16;
	recty = 480 - 28;
	rectw = 12;
	recth = 90;

	//CG_DrawRect(float x, float y, float width, float height, float size, const float *color);
	CG_FillRectUp(rectx, recty, rectw, recth * (cg.snap->ps.stats[STAT_MAX_POWER] / FULL_POWER), colorEmpty);
	CG_FillRectUp(rectx, recty, rectw, recth * pFraction, colorFull);

	pString = va("%i", currentPower);
	CG_Text_PaintAligned(rectx - 14, recty + 17, pString, 0.125f, UI_LEFT, colorWater, &cgs.media.freeSansBoldFont);
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

	//CG_AdjustFrom640(&h, &h, &w, &h);

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
			offsetCount = elementCount * 8;
			if(depth == 1)
			{
				// Green
				CG_Text_PaintAligned((320 + xo), (240 - yo - offsetCount), str, 0.125f, UI_CENTER, colorGreen, &cgs.media.freeSansBoldFont);
			}
			else
			{
				// White
				CG_Text_PaintAligned((320 + xo), (240 - yo - offsetCount), str, 0.125f, UI_CENTER, colorWhite, &cgs.media.freeSansBoldFont);
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
			CG_Text_PaintAligned(320, 400, str, 0.25f, UI_CENTER, colorWhite, &cgs.media.freeSansBoldFont);
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
	w = h = cg_weaverDiscSize.integer;

	if(da >= 1 && da <= NUM_WEAVERDISCS)
	{
		//TODO: reimplement move scale
		//moveScale = (float)cg_weaverDiscMoveScale.value * 1.0;

		CG_AdjustFrom640(&x, &y, &w, &h);

		//thread = va("%i x, %i y ", cg.snap->ps.stats[STAT_THREADX], cg.snap->ps.stats[STAT_THREADY]);
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

	CG_DrawWeaverTutorial(x, y, (0.5 * w));
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
	float           xi, yi;
	int             count;

	count = 0;
	x = 0.0f;
	y = 20.0f;

	for(i = MIN_WEAPON_WEAVE; i < MAX_WEAPONS; i++)
	{
		if(cg.predictedPlayerState.ammo[i] > 0)
		{
			count++;
		}
	}

	xi = 324 - count * 20;		// 640/2-(count*20)+4
	yi = 440;					//480 - 40 = 40 up from bottom

	for(i = MIN_WEAPON_WEAVE; i < MAX_WEAPONS; i++)
	{
		if(cg.predictedPlayerState.ammo[i] > 0)
		{
			cent = &cg_entities[cg.predictedPlayerState.ammo[i]];
			weaveInfo = &cg_weaves[cent->currentState.weapon];

			thread =
				va("%i: ent=%i w=%d ammo=%d/%d", i, cg.predictedPlayerState.ammo[i], cent->currentState.weapon,
				   cent->currentState.torsoAnim, weaveInfo->info.castCharges);
			CG_Text_PaintAligned(x + 10, y + 27, thread, 0.125f, UI_LEFT, colorWhite, &cgs.media.freeSansBoldFont);

			y += 7;

			CG_DrawPic(xi, yi, 32, 32, weaveInfo->icon);

			// draw selection marker
			if(i == cg.weaponSelect)
			{
				CG_DrawPic(xi - 4, yi - 4, 40, 40, cgs.media.weaponSelectShader);
			}

			xi += 40;
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
	//Health and Power
	CG_DrawWeaverStatusBar();

	//Weaver Disc, includes strokes and tutorial
	CG_DrawWeaverDisc();

	//Weaver Sense
	CG_DrawWeaveSense();

	//Held Weaves
	CG_DrawWeaverHeld();

	//Powerups
	CG_DrawWeaverPowerups();
}
