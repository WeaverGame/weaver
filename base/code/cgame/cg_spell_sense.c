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

// cg_spell_sense.c -- display player's perception of other players who are using magic
#include "cg_local.h"

#define POWER_LEVEL_3 120
#define POWER_LEVEL_2 70
#define POWER_LEVEL_1 10

static int      lastUpdate;
static float    dotAngles[MAX_CLIENTS];
static int      dotPowers[MAX_CLIENTS];
//static int      dotNumber;

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
