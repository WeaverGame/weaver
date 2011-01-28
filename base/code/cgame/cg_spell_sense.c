/*
===========================================================================
This file is part of the weaver HUD.

It is used to display player's sensors for players who are channeling
===========================================================================
*/

#include "cg_local.h"

#define POWER_LEVEL_3 120
#define POWER_LEVEL_2 70
#define POWER_LEVEL_1 10

static int      lastUpdate;
static float    dotAngles[MAX_CLIENTS];
static int      dotPowers[MAX_CLIENTS];
static int      dotNumber;

void CG_UpdateWeaveSense(int clientnum, vec3_t origin, int power)
{
	vec3_t          relOrigin;
	vec3_t          relAngle;

	VectorSubtract(origin, cg.refdef.vieworg, relOrigin);
	VectorToAngles(relOrigin, relAngle);
	dotAngles[clientnum] = AngleNormalize180(relAngle[1] - cg.refdefViewAngles[1] + 90.0f);
	dotPowers[clientnum] = power;
	lastUpdate = cg.time;
}

void CG_DrawWeaveSense(void)
{
	int             i;
	float           w, h;
	float           x, y;
	int             radius;
	char           *thread;
	qhandle_t       dShader;
	vec4_t          colorWhite = { 1.0f, 1.0f, 1.0f, 1.0f };

	vec4_t          dcolor = { 1.0f, 1.0f, 1.0f, 1.0f };
	radius = cg_weaverSenseRadius.integer;

	for(i = 0; i < MAX_CLIENTS; i++)
	{
		if(dotPowers[i] <= 0)
		{
			continue;
		}

		x = 320.0f;				//640/2;
		y = 240.0f;				//480/2;

		dShader = cgs.media.weaverSenseDot1;
		x += radius * cos(DEG2RAD((float)dotAngles[i]));
		y -= radius * sin(DEG2RAD((float)dotAngles[i]));

		thread = va("c=%i p=%i", i, dotPowers[i]);
		CG_Text_PaintAligned(100, (10 + 10 * i), thread, 0.125f, UI_LEFT, colorWhite, &cgs.media.freeSansBoldFont);

		CG_Text_PaintAligned(x, y, "*", 0.125f, UI_LEFT, colorWhite, &cgs.media.freeSansBoldFont);

		if(dotPowers[i] >= POWER_LEVEL_3)
		{
			//Draw largest indicator
			dcolor[3] = 1.0f;
			w = h = 10 * (POWER_LEVEL_3 / dotPowers[i]);
			dShader = cgs.media.weaverSenseDot3;
		}
		else if(dotPowers[i] >= POWER_LEVEL_2)
		{
			//Draw medium indicator
			dcolor[3] = 1.0f;
			w = h = 10 * (POWER_LEVEL_2 / dotPowers[i]);
			dShader = cgs.media.weaverSenseDot2;
		}
		else if(dotPowers[i] >= POWER_LEVEL_1)
		{
			//Draw small indicator
			dcolor[3] = 0.85f;
			w = h = 10 * (POWER_LEVEL_1 / dotPowers[i]);
		}
		else
		{
			//Draw minimum indicator
			dcolor[3] = 0.70f;
			w = h = 10;
		}

		trap_R_SetColor(dcolor);
		CG_DrawPic(x - 0.5 * w, y - 0.5 * h, w, h, dShader);
	}
}
