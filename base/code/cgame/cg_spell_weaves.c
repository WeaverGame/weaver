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

// cg_weaves.c -- events and effects dealing with weaves
#include "cg_local.h"

static const vec3_t heldWeaveOffset[HELD_MAX][HELD_MAX] = {
	{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	{{1, 0, 0}, {-1, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	{{1, 0, 0}, {-0.707f, 0.707f, 0}, {-0.707f, -0.707f, 0}, {0, 0, 0}},
	{{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}}
};

int             threadColorR[WVP_NUMBER] = {
	0,							//none
	239,						//air
	239,						//airfire
	249,						//fire
	215,						//earthfire
	18,							//earth
	5,							//earthwater
	0,							//water
	119,						//airwater
	255							//spirit
};

int             threadColorG[WVP_NUMBER] = {
	0,							//none
	230,						//air
	135,						//airfire
	41,							//fire
	174,						//earthfire
	195,						//earth
	202,						//earthwater
	82,							//water
	156,						//airwater
	255							//spirit
};

int             threadColorB[WVP_NUMBER] = {
	0,							//none
	0,							//air
	20,							//airfire
	41,							//fire
	0,							//earthfire
	30,							//earth
	176,						//earthwater
	231,						//water
	115,						//airwater
	255							//spirit
};

static const vec3_t protectOffset = { 0, 0, 40 };

void PowerDecode(playerEntity_t * pe, int n, int offset, int count)
{
	int             i = 0;

	for(i = 0; i < count; i++)
	{
		pe->threads[offset + i] = (n / (int)pow(WVP_NUMBER, i)) % WVP_NUMBER;
	}
}

int PowerColor(int threads[MAX_THREADS], vec3_t * color)
{
	int             i = 0;
	int             total = 0;
	int             outColor[3] = { 0, 0, 0 };

	for(i = 0; i < MAX_THREADS; i++)
	{
		if(threads[i] != WVP_NONE)
		{
			//Weight earlier colors
			outColor[0] <<= 1;
			outColor[1] <<= 1;
			outColor[2] <<= 1;
			total <<= 1;
			//Add this color
			outColor[0] += threadColorR[threads[i]];
			outColor[1] += threadColorG[threads[i]];
			outColor[2] += threadColorB[threads[i]];
			total++;
		}
	}

	(*color)[0] = (float)(outColor[0] / total) / 255.0f;
	(*color)[1] = (float)(outColor[1] / total) / 255.0f;
	(*color)[2] = (float)(outColor[2] / total) / 255.0f;
	return total;
}

/*
=================
CG_ShowThreads

Handles thread entities.
These are the entities which specify the threads to show infront
of other players' models, and on this player's hud.
=================
*/
void CG_ShowThreads(centity_t * cent)
{
	playerEntity_t *pe;

	if(cg.predictedPlayerState.clientNum == cent->currentState.otherEntityNum2)
	{
		//Threads belong to this client.
		pe = &cg.predictedPlayerEntity.pe;
	}
	else
	{
		//Threads belong to another player.
		pe = &cg_entities[cent->currentState.otherEntityNum2].pe;
		//Use info to update weave sense dots
		//CG_UpdateWeaveSense(cent->currentState.otherEntityNum2, cent->currentState.origin, cent->currentState.powerups);
	}

	PowerDecode(pe, cent->currentState.constantLight, 0, 8);

	//CG_Printf("ThreadsEnt: client=%d raw=%d threads=%d", cent->currentState.otherEntityNum2, cent->currentState.constantLight, pe->threads[0]);
	//CG_Printf(" dec=%d,%d,%d,%d,%d,%d,%d,%d\n", pe->threads[0], pe->threads[1], pe->threads[2], pe->threads[3], pe->threads[4], pe->threads[5], pe->threads[6], pe->threads[7]);

	if(pe->threads[0] == WVP_NONE)
	{
		return;
	}
	else
	{
		// If threads belong to this client, and this client is holding the sword
		if((cg.weaponSelect == WP_GAUNTLET) && (cg.predictedPlayerState.clientNum == cent->currentState.otherEntityNum2))
		{
			// Change weapon to none
			cg.weaponSelectTime = cg.time;
			cg.weaponSelect = WP_NONE;
		}
	}

	pe->threadsLight = PowerColor(pe->threads, &pe->threadsColor);
}

qboolean CG_PlayerIsCasting(playerState_t * ps)
{
	return (ps->eFlags & EF_WEAVEA || ps->eFlags & EF_WEAVED);
}

qboolean CG_PlayerIsThreading(playerState_t * ps)
{
	return (ps->eFlags & EF_THREAD);
}

/*
=================
CG_AddPlayerProtects

Adds protect weave entities to a player model
=================
*/
void CG_AddPlayerProtects(centity_t * player, playerState_t * ps, refEntity_t * body)
{
	int             i;
	playerEntity_t *pe;
	centity_t      *protectWeave;
	const weaver_weaveCGInfo *weave;
	/*
	int             entinit = 0;
	refEntity_t     ent;
	int             rf;

	if(player->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson)
	{
		rf = RF_THIRD_PERSON;	// only show in mirrors
	}
	else
	{
		rf = 0;
	}
	*/

	pe = &player->pe;

	for(i = 0; i <= 3; i++)
	{
		if(!pe->protectWeaveOn[i])
		{
			continue;
		}

		protectWeave = &cg_entities[pe->protectWeaveEnt[i]];

		if((protectWeave == NULL) || (protectWeave->currentValid < 1))
		{
			// this ent number is no longer a protect weave
			pe->protectWeaveEnt[i] = 0;
			pe->protectWeaveOn[i] = qfalse;
			continue;
		}

		weave = &cg_weaves[protectWeave->currentState.weapon];

		// Model overlay
		body->customShader = weave->instanceShader[1];
		trap_R_AddRefEntityToScene(body);

		/*
		// Extra torso shield model
		if(entinit == 0)
		{
			memset(&ent, 0, sizeof(ent));

			VectorCopy(body->origin, ent.origin);
			//VectorAdd(ent.origin, protectOffset, ent.origin);
			AxisCopy(body->axis, ent.axis);
			entinit = 1;
			ent.renderfx = rf;
		}
		ent.customShader = weave->instanceShader[0];
		ent.hModel = weave->instanceModel[0];
		RotateAroundAxis(ent.axis, cg.time / 2, 2);
		trap_R_AddRefEntityToScene(&ent);
		*/
	}
}

/*
=================
CG_AddPlayerWeaveEffects

Adds weave effect entities to a player model
=================
*/
void CG_AddPlayerWeaveEffects(centity_t * player, playerState_t * ps, refEntity_t * body)
{
	int             i;
	playerEntity_t *pe;
	centity_t      *weaveEffect;

	pe = &player->pe;

	for(i = 0; i < MAX_PLAYER_WEAVE_EFFECTS; i++)
	{
		if(!pe->weaveEffectEnt[i])
		{
			continue;
		}

		weaveEffect = &cg_entities[pe->weaveEffectEnt[i]];

		// Clean up old effects
		if((weaveEffect == NULL) ||
			(weaveEffect->currentValid < 1) ||
			(weaveEffect->currentState.eType != ET_WEAVE_EFFECT) ||
			(weaveEffect->currentState.generic1 != player->currentState.clientNum))
		{
			// this ent number is no longer a weave effect
			pe->weaveEffectEnt[i] = 0;
			continue;
		}

		CG_WeaveEffect_OnPlayer(weaveEffect, player, ps, body);
	}
}

/*
=================
CG_AddPlayerThreads

Adds thread entities to a player model
=================
*/
void CG_AddPlayerThreads(centity_t * player, playerState_t * ps, refEntity_t * parent)
{
	int             i;
	refEntity_t     ent;
	int             boneIndex;
	playerEntity_t *pe;
	vec3_t          pos;
	float           d;

	pe = &player->pe;

	//CG_Printf("ThreadsEnt: player=%d cg_ents=%d threads=%d threads=%d\n", player, &cg_entities, player->pe.threads[0], pe->threads[0]);

	if(pe->threads[0] == WVP_NONE)
	{
		return;
	}

	//CG_Printf("ThreadsEnt: player=%d raw=%d dec=%d,%d,%d,%d,%d,%d,%d,%d\n", player->currentState.clientNum,
	//          pe->threads[0], pe->threads[1], pe->threads[2], pe->threads[3], pe->threads[4], pe->threads[5], pe->threads[6], pe->threads[7]);

	//initialize render entity
	memset(&ent, 0, sizeof(ent));
	AxisClear(ent.axis);

	if(ps)
	{
		CG_PositionEntityOnTag(&ent, parent, parent->hModel, "spellControl");
	}
	else
	{
		boneIndex = trap_R_BoneIndex(parent->hModel, "spellControl");
		if(boneIndex >= 0 && boneIndex < pe->torso.skeleton.numBones)
		{
			AxisClear(ent.axis);
			CG_PositionRotatedEntityOnBone(&ent, parent, parent->hModel, "spellControl");
		}
		else
		{
			CG_Error("No tag found while adding threads.");
		}
	}

	if(player->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson)
	{
		ent.renderfx = RF_THIRD_PERSON;	// only show in mirrors
	}
	else
	{
		ent.renderfx = 0;
	}

	ent.renderfx |= RF_NOSHADOW;
	ent.reType = RT_SPRITE;
	ent.radius = 15.0f;

	VectorCopy(ent.origin, pos);

	for(i = 0; i < 3 && (pe->threads[i] != WVP_NONE); i++)
	{
		ent.rotation = cg.time / (20.0f - (i*3));

		VectorCopy(pos, ent.origin);
		d = (400.0f + (100.0f * i));
		if(i & 1)
		{
			ent.rotation *= -1;
			ent.origin[0] += 4 * sin(cg.time / d);
		}
		if(i & 2)
		{
			ent.origin[1] += 4 * sin((cg.time + 212) / d);
		}
		if(i & 3)
		{
			ent.origin[2] += 4 * sin((cg.time + 37) / d);
		}

		if(pe->threads[i] & 1)
		{
			// Thread is a primary power
			ent.customShader = cgs.media.weaverThreads[pe->threads[i]][i];
			trap_R_AddRefEntityToScene(&ent);
			ent.radius -= 5.0f;
		}
		else if(i < 2)
		{
			ent.radius -= 5.0f;
			// Thread is secondary power
			if (pe->threads[i] == WVP_AIRWATER)
			{
				// Special case, combination element not flanked by primaries
				ent.customShader = cgs.media.weaverThreads[WVP_WATER][i+1];
				trap_R_AddRefEntityToScene(&ent);
				ent.customShader = cgs.media.weaverThreads[WVP_AIR][i+1];
				trap_R_AddRefEntityToScene(&ent);
			}
			else if(pe->threads[i] > 1 && pe->threads[i] < 7)
			{
				// Combination element flanked by primaries
				ent.customShader = cgs.media.weaverThreads[pe->threads[i]-1][i];
				trap_R_AddRefEntityToScene(&ent);
				ent.customShader = cgs.media.weaverThreads[pe->threads[i]+1][i];
				trap_R_AddRefEntityToScene(&ent);
			}
		}
	}

	//Com_Printf("color: R=%d G=%d B=%d\n", (int)(color[0]*255), (int)(color[1]*255), (int)(color[2]*255));
	//TODO: use light shader instead of this code
	trap_R_AddLightToScene(ent.origin, 50 + pe->threadsLight + sin(cg.time / 20.0), pe->threadsColor[0], pe->threadsColor[1], pe->threadsColor[2]);
}

/*
=================
CG_AddPlayerHeldWeave

Adds heldweave entities to a player model
=================
*/
void CG_AddPlayerHeldWeave(centity_t * player, playerState_t * ps, refEntity_t * parent)
{
	refEntity_t     ent[HELD_MAX];
	playerEntity_t *pe;
	int             boneIndex;
	int             i;
	weaver_weaveCGInfo *weaveInfo;
	centity_t      *heldWeave;
	vec3_t          origin;
	int             rf;
	int             count;

	pe = &player->pe;

	//initialize render entity
	memset(&ent[0], 0, sizeof(ent[0]));

#ifdef XPPM
	if(ps)
	{
		CG_PositionEntityOnTag(&ent[0], parent, parent->hModel, "Head");
	}
	else
	{
		boneIndex = trap_R_BoneIndex(parent->hModel, "Head");
		if(boneIndex >= 0 && boneIndex < pe->torso.skeleton.numBones)
		{
			AxisClear(ent[0].axis);
			CG_PositionRotatedEntityOnBone(&ent[0], parent, parent->hModel, "Head");
		}
		else
		{
			CG_Error("No tag found while adding held weave.");
		}
	}
#else
	CG_PositionEntityOnTag(&ent[0], parent, parent->hModel, "tag_weapon");
#endif

	if(player->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson)
	{
		rf = RF_THIRD_PERSON;	// only show in mirrors
	}
	else
	{
		rf = 0;
	}

	// use the same origin for them all
	AxisClear(ent[0].axis);
	VectorCopy(ent[0].origin, origin);
	origin[2] += 15;

	count = 0;
	for(i = 0; i < HELD_MAX; i++)
	{
		if(pe->heldWeaveEnt[i] == 0)
		{
			continue;
		}

		heldWeave = &cg_entities[pe->heldWeaveEnt[i]];
		if((heldWeave == NULL) || (heldWeave->currentValid < 1))
		{
			// this ent number is no longer a held weave
			pe->heldWeaveEnt[i] = 0;
			continue;
		}
		count++;
	}

	for(i = 0; i < HELD_MAX; i++)
	{
		if(pe->heldWeaveEnt[i] == 0)
		{
			continue;
		}

		heldWeave = &cg_entities[pe->heldWeaveEnt[i]];

		memset(&ent[i], 0, sizeof(ent[i]));

		VectorCopy(origin, ent[i].origin);

		VectorMA(ent[i].origin, 10, heldWeaveOffset[count - 1][i], ent[i].origin);

		weaveInfo = &cg_weaves[heldWeave->currentState.weapon];

		ent[i].customShader = weaveInfo->icon;
		ent[i].reType = RT_SPRITE;
		ent[i].radius = 6;
		ent[i].renderfx = rf | RF_NOSHADOW;

		trap_R_AddRefEntityToScene(&ent[i]);
	}
}

/*
==================
CG_HeldWeave
//WEAVER
==================
*/
void CG_HeldWeave(centity_t * cent)
{
	playerEntity_t *pe;
	int             slot;

	if(cg.predictedPlayerState.clientNum == cent->currentState.otherEntityNum2)
	{
		//Heldweave belong to this client.
		pe = &cg.predictedPlayerEntity.pe;
	}
	else
	{
		//Heldweave belong to another player.
		pe = &cg_entities[cent->currentState.otherEntityNum2].pe;
	}

	slot = cent->currentState.modelindex2 - MIN_WEAPON_WEAVE;

	pe->heldWeaveEnt[slot] = cent->currentState.number;
}

/*
===============
CG_WeaveSelect_f

Weave nums are 1, 2, 3, 4...
Corresponds with weaponbanks 15, 14, 13, 12...
===============
*/
void CG_WeaveSelect_f(void)
{
	int             num;

	if(!cg.snap)
	{
		return;
	}
	if(cg.snap->ps.pm_flags & PMF_FOLLOW)
	{
		return;
	}

	num = atoi(CG_Argv(1));

	if(num < 1 || num > HELD_MAX)
	{
		return;
	}

	num = num + MIN_WEAPON_WEAVE - 1;

	cg.weaponSelectTime = cg.time;

	if(!(cg.snap->ps.stats[STAT_WEAPONS] & (1 << num)))
	{
		return;					// don't have the weapon
	}

	cg.weaponSelect = num;
}

/*
===============
CG_WeaveIncr_f
===============
*/
void CG_WeaveIncr_f(int increment)
{
	int             i;
	int             original;

	if(!cg.snap)
	{
		return;
	}
	if(cg.snap->ps.pm_flags & PMF_FOLLOW)
	{
		return;
	}

	if(cg.osd.input)
	{
		CG_OSDNext_f();
		return;
	}

	if(cg.scoreBoardShowing)
	{
		cg.scoreboard_offset++;
		return;
	}

	cg.weaponSelectTime = cg.time;
	original = cg.weaponSelect;

	for(i = 0; i < HELD_MAX; i++)
	{
		cg.weaponSelect += increment;
		if(cg.weaponSelect > MAX_WEAPON_WEAVE)
		{
			cg.weaponSelect = MIN_WEAPON_WEAVE;
		}
		if(cg.weaponSelect < MIN_WEAPON_WEAVE)
		{
			cg.weaponSelect = MAX_WEAPON_WEAVE;
		}

		if(cg.snap->ps.ammo[cg.weaponSelect] && (cg.snap->ps.stats[STAT_WEAPONS] & (1 << cg.weaponSelect)))
		{
			break;
		}
	}
	if(i >= HELD_MAX)
	{
		cg.weaponSelect = original;
	}
}

/*
===============
CG_WeaveNext_f
CG_WeavePrev_f
===============
*/
void CG_WeaveNext_f(void)
{
	CG_WeaveIncr_f(+1);
}

void CG_WeavePrev_f(void)
{
	CG_WeaveIncr_f(-1);
}

/*
===================
CG_WeaveClearedChange

The current spell has run out of shots, or was released
===================
*/
void CG_WeaveClearedChange(void)
{
	switch(cg_weaverSpellSwitch.integer)
	{
		case 0:
			break;
		default:
		case 1:
			CG_WeaveIncr_f(+1);
			break;
		case 2:
			CG_WeaveIncr_f(-1);
			break;
	}
}

/*
=================
CG_RegisterWeapon

The server says this item is used on this level
=================
*/
void CG_RegisterWeave(int weaveNum)
{
	weaver_weaveCGInfo *weaveInfo;
	int             num;
	char           *s = NULL;

	//gitem_t        *item, *ammo;
	//char            path[MAX_QPATH];

	weaveInfo = &cg_weaves[weaveNum];

	if(weaveNum == 0)
	{
		return;
	}

	if(weaveInfo->registered)
	{
		return;
	}

	memset(weaveInfo, 0, sizeof(*weaveInfo));
	weaveInfo->registered = qtrue;

	//Defaults
	weaveInfo->exploLight = 0;
	weaveInfo->exploMarkAlphaFade = qfalse;
	weaveInfo->exploDuration = 0;
	weaveInfo->missileModelCount = 0;

	weaveInfo->missileTrailStep = 50;

	//Set defined info
	LoadWeaveInfo(&weaveInfo->info, weaveNum);

	//Register Icon
	s = va("gfx/icons/spells/%s", weaveInfo->info.name);
	weaveInfo->icon = trap_R_RegisterShader(s);

	switch (weaveNum)
	{
		case WVW_NONE:
			break;

		case WVW_A_AIR_BLAST:

			//misc resources
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weaves/airblast/cast.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/airblast/airblast.md5mesh");
			weaveInfo->missileLight = 20;
			MAKERGB(weaveInfo->missileLightColor, 0.3f, 0.3f, 1);
			//weaveInfo->missileRenderfx;
			weaveInfo->missileSound = trap_S_RegisterSound("sound/weaves/airblast/fly.ogg");

			weaveInfo->missileTrailRadius = 80.0f;
			weaveInfo->missileTrailAlpha = 0.05f;
			weaveInfo->missileTrailDuration = 2000;
			weaveInfo->missileTrailShader[0] = cgs.media.smokePuffShader;

			//weaveInfo->exploModel;
			//weaveInfo->exploMark;
			//weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->exploSfx = trap_S_RegisterSound("sound/weaves/airblast/impact.ogg");
			//weaveInfo->exploMark = cgs.media.burnMarkShader;
			//weaveInfo->exploMarkRadius = 64;
			//weaveInfo->exploLight = 0;
			//MAKERGB(weaveInfo->exploLightColor, 0.75f, 0.5f, 0.1f);
			//weaveInfo->exploDuration = 0;

			//instance
			//weaveInfo->instanceModel;
			//weaveInfo->instanceLight;
			//weaveInfo->instanceLightColor;
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_A_FIRE_MULTIDARTS:

			//misc resources
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weaves/firedart/cast.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/firedart/firedart.md5mesh");
			//weaveInfo->missileLight = 50;
			//MAKERGB(weaveInfo->missileLightColor, 0.5f, 0.1f, 0.1f);
			//weaveInfo->missileRenderfx;
			//weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav", qfalse);

			//weaveInfo->exploModel;
			//weaveInfo->exploMark;
			//weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->exploSfx = trap_S_RegisterSound("sound/weaves/firedart/impact.ogg");
			weaveInfo->exploMark = cgs.media.bulletMarkShader;
			weaveInfo->exploMarkRadius = 8;
			//weaveInfo->exploLight = 100;
			//MAKERGB(weaveInfo->exploLightColor, 0.75f, 0.5f, 0.1f);
			//weaveInfo->exploDuration = 800;
			//weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			//weaveInfo->exploSfx = trap_S_RegisterSound("sound/weapons/rocket/rocklx1a.ogg", qfalse);

			//instance
			//weaveInfo->instanceShader1 = trap_R_RegisterShader("");
			//weaveInfo->instanceModel;
			//weaveInfo->instanceLight;
			//weaveInfo->instanceLightColor;
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_A_FIRE_DARTS:

			//misc resources
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weaves/firedart/cast.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/firedart/firedart.md5mesh");
			//weaveInfo->missileLight = 10;
			//MAKERGB(weaveInfo->missileLightColor, 0.5f, 0.1f, 0.1f);
			//weaveInfo->missileRenderfx;
			//weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav");

			//weaveInfo->exploModel;
			//weaveInfo->exploMark;
			//weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->exploSfx = trap_S_RegisterSound("sound/weaves/firedart/impact.ogg");
			weaveInfo->exploMark = cgs.media.bulletMarkShader;
			weaveInfo->exploMarkRadius = 8;
			//weaveInfo->exploLight = 100;
			//MAKERGB(weaveInfo->exploLightColor, 0.75f, 0.5f, 0.1f);
			//weaveInfo->exploDuration = 800;

			//instance
			//weaveInfo->instanceShader1 = trap_R_RegisterShader("");
			//weaveInfo->instanceModel;
			//weaveInfo->instanceLight;
			//weaveInfo->instanceLightColor;
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_A_AIRWATER_DARTS_S:

			//misc resources
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weaves/firedart/cast.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/needles/needle.md5mesh");
			//weaveInfo->missileLight = 10;
			//MAKERGB(weaveInfo->missileLightColor, 0.5f, 0.1f, 0.1f);
			//weaveInfo->missileRenderfx;
			//weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav");

			//weaveInfo->exploModel;
			//weaveInfo->exploMark;
			//weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->exploSfx = trap_S_RegisterSound("sound/weaves/firedart/impact.ogg");
			weaveInfo->exploMark = cgs.media.bulletMarkShader;
			weaveInfo->exploMarkRadius = 8;
			//weaveInfo->exploLight = 100;
			//MAKERGB(weaveInfo->exploLightColor, 0.75f, 0.5f, 0.1f);
			//weaveInfo->exploDuration = 800;

			//instance
			//weaveInfo->instanceShader1 = trap_R_RegisterShader("");
			//weaveInfo->instanceModel;
			//weaveInfo->instanceLight;
			//weaveInfo->instanceLightColor;
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_A_AIRWATER_DARTS_M:

			//misc resources
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weaves/firedart/cast.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/needles/needle.md5mesh");
			//weaveInfo->missileLight = 10;
			//MAKERGB(weaveInfo->missileLightColor, 0.5f, 0.1f, 0.1f);
			//weaveInfo->missileRenderfx;
			//weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav");

			//weaveInfo->exploModel;
			//weaveInfo->exploMark;
			//weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->exploSfx = trap_S_RegisterSound("sound/weaves/firedart/impact.ogg");
			weaveInfo->exploMark = cgs.media.bulletMarkShader;
			weaveInfo->exploMarkRadius = 8;
			//weaveInfo->exploLight = 100;
			//MAKERGB(weaveInfo->exploLightColor, 0.75f, 0.5f, 0.1f);
			//weaveInfo->exploDuration = 800;

			//instance
			//weaveInfo->instanceShader1 = trap_R_RegisterShader("");
			//weaveInfo->instanceModel;
			//weaveInfo->instanceLight;
			//weaveInfo->instanceLightColor;
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_D_AIR_PROTECT:

			//misc resources
			//weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/machinegun/par_shot_1.ogg");

			//missiles
			//weaveInfo->missileModel;
			//weaveInfo->missileLight;
			//weaveInfo->missileLightColor;
			//weaveInfo->missileRenderfx;
			//weaveInfo->missileSound;

			//instance
			weaveInfo->instanceModel[0] = trap_R_RegisterModel("models/weaves/protect/protect.md5mesh");
			weaveInfo->instanceLight = 250;
			MAKERGB(weaveInfo->instanceLightColor, 1.0f, 0.8f, 0.0f);
			weaveInfo->instanceShader[0] = trap_R_RegisterShader("models/weaves/protect/protect_air");
			weaveInfo->instanceShader[1] = trap_R_RegisterShader("models/weaves/protect/protect_air_skin");
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_D_FIRE_PROTECT:

			//misc resources
			//weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/machinegun/par_shot_1.ogg");

			//missiles
			//weaveInfo->missileModel;
			//weaveInfo->missileLight;
			//weaveInfo->missileLightColor;
			//weaveInfo->missileRenderfx;
			//weaveInfo->missileSound;
			//weaveInfo->missileTrailFunc;

			//instance
			weaveInfo->instanceModel[0] = trap_R_RegisterModel("models/weaves/protect/protect.md5mesh");
			weaveInfo->instanceLight = 250;
			MAKERGB(weaveInfo->instanceLightColor, 1, 0, 0);
			weaveInfo->instanceShader[0] = trap_R_RegisterShader("models/weaves/protect/protect_fire");
			weaveInfo->instanceShader[1] = trap_R_RegisterShader("models/weaves/protect/protect_fire_skin");
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_D_EARTH_PROTECT:

			//misc resources
			//weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/machinegun/par_shot_1.ogg");

			//missiles
			//weaveInfo->missileModel;
			//weaveInfo->missileLight;
			//weaveInfo->missileLightColor;
			//weaveInfo->missileRenderfx;
			//weaveInfo->missileSound;
			//weaveInfo->missileTrailFunc;

			//instance
			weaveInfo->instanceModel[0] = trap_R_RegisterModel("models/weaves/protect/protect.md5mesh");
			weaveInfo->instanceLight = 250;
			MAKERGB(weaveInfo->instanceLightColor, 1, 0, 0);
			weaveInfo->instanceShader[0] = trap_R_RegisterShader("models/weaves/protect/protect_earth");
			weaveInfo->instanceShader[1] = trap_R_RegisterShader("models/weaves/protect/protect_earth_skin");
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_D_WATER_PROTECT:

			//misc resources
			//weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/machinegun/par_shot_1.ogg");

			//missiles
			//weaveInfo->missileModel;
			//weaveInfo->missileLight;
			//weaveInfo->missileLightColor;
			//weaveInfo->missileRenderfx;
			//weaveInfo->missileSound;
			//weaveInfo->missileTrailFunc;

			//instance
			weaveInfo->instanceModel[0] = trap_R_RegisterModel("models/weaves/protect/protect.md5mesh");
			weaveInfo->instanceLight = 250;
			MAKERGB(weaveInfo->instanceLightColor, 1, 0, 0);
			weaveInfo->instanceShader[0] = trap_R_RegisterShader("models/weaves/protect/protect_water");
			weaveInfo->instanceShader[1] = trap_R_RegisterShader("models/weaves/protect/protect_water_skin");
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_A_FIRE_BALL:

			//misc resources
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/machinegun/par_shot_1.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/fireball/fireball.md5mesh");
			weaveInfo->missileLight = 250;
			MAKERGB(weaveInfo->missileLightColor, 0.83f, 0.4f, 0.1f);
			//weaveInfo->missileRenderfx;
			weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav");

			weaveInfo->missileTrailRadius = 20.0f;
			weaveInfo->missileTrailAlpha = 0.1f;
			weaveInfo->missileTrailDuration = 150;
			weaveInfo->missileTrailShader[0] = trap_R_RegisterShader("models/weaves/fireball/trail");
			weaveInfo->missileTrailStep = 80;

			//weaveInfo->exploModel;
			//weaveInfo->exploMark;
			weaveInfo->exploRadius = 100.0f;
			weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->exploSfx = trap_S_RegisterSound("sound/weapons/rocket/rocklx1a.ogg");
			weaveInfo->exploMark = cgs.media.burnMarkShader;
			weaveInfo->exploMarkRadius = 100;
			weaveInfo->exploLight = 350;
			MAKERGB(weaveInfo->exploLightColor, 0.95f, 0.4f, 0.1f);
			weaveInfo->exploDuration = 1000;

			//instance
			//weaveInfo->instanceModel = trap_R_RegisterModel("models/projectiles/spike/spike.md3");
			//weaveInfo->instanceLight = 250;
			//MAKERGB(weaveInfo->instanceLightColor, 1, 0, 0);
			//weaveInfo->instanceShader1 = trap_R_RegisterShader("models/weaves/protect_fire/protect_fire");
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_A_EARTH_QUAKE_S:

			//misc resources
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/machinegun/par_shot_1.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/earthquake_s/proj_s.md5mesh");
			weaveInfo->missileLight = 50;
			MAKERGB(weaveInfo->missileLightColor, 0.5f, 0.1f, 0.1f);
			//weaveInfo->missileRenderfx;
			weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav");
			//weaveInfo->missileTrailFunc;

			//weaveInfo->exploModel;
			//weaveInfo->exploMark;
			//weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			//weaveInfo->exploSfx = trap_S_RegisterSound("sound/weapons/rocket/rocklx1a.ogg", qfalse);
			//weaveInfo->exploMark = cgs.media.bulletMarkShader;
			//weaveInfo->exploMarkRadius = 8;
			//weaveInfo->exploLight = 100;
			//MAKERGB(weaveInfo->exploLightColor, 0.75f, 0.5f, 0.1f);
			//weaveInfo->exploDuration = 800;

			//instance
			weaveInfo->instanceModel[0] = trap_R_RegisterModel("models/weaves/earthquake_s/earthquake_fx.md5mesh");
			weaveInfo->instanceModel[1] = trap_R_RegisterModel("models/weaves/earthquake_s/earthquake_s02.md5mesh");
			weaveInfo->instanceModel[2] = trap_R_RegisterModel("models/weaves/earthquake_s/earthquake_s03.md5mesh");
			weaveInfo->instanceModel[3] = trap_R_RegisterModel("models/weaves/earthquake_s/earthquake_s04.md5mesh");
			weaveInfo->instanceLight = 500;
			weaveInfo->instanceLightShader = trap_R_RegisterShader("weave_effects/earthquake/glow");
			MAKERGB(weaveInfo->instanceLightColor, 0.6f, 0.3f, 0.1f);
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			weaveInfo->instanceShader[0] = trap_R_RegisterShader("models/weaves/fireball/trail");
			break;
		case WVW_A_EARTH_QUAKE_M:

			//misc resources
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/machinegun/par_shot_1.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/earthquake_s/proj_m.md5mesh");
			weaveInfo->missileLight = 50;
			MAKERGB(weaveInfo->missileLightColor, 0.5f, 0.1f, 0.1f);
			//weaveInfo->missileRenderfx;
			weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav");
			//weaveInfo->missileTrailFunc;

			//weaveInfo->exploModel;
			//weaveInfo->exploMark;
			//weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			//weaveInfo->exploSfx = trap_S_RegisterSound("sound/weapons/rocket/rocklx1a.ogg", qfalse);
			//weaveInfo->exploMark = cgs.media.bulletMarkShader;
			//weaveInfo->exploMarkRadius = 8;
			//weaveInfo->exploLight = 100;
			//MAKERGB(weaveInfo->exploLightColor, 0.75f, 0.5f, 0.1f);
			//weaveInfo->exploDuration = 800;

			//instance
			weaveInfo->instanceModel[0] = trap_R_RegisterModel("models/weaves/earthquake_s/earthquake_s01.md5mesh");
			weaveInfo->instanceModel[1] = trap_R_RegisterModel("models/weaves/earthquake_s/earthquake_s02.md5mesh");
			weaveInfo->instanceModel[2] = trap_R_RegisterModel("models/weaves/earthquake_s/earthquake_s03.md5mesh");
			weaveInfo->instanceModel[3] = trap_R_RegisterModel("models/weaves/earthquake_s/earthquake_s04.md5mesh");
			weaveInfo->instanceLight = 800;
			weaveInfo->instanceLightShader = trap_R_RegisterShader("weave_effects/earthquake/glow");
			MAKERGB(weaveInfo->instanceLightColor, 0.6f, 0.3f, 0.1f);
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			weaveInfo->instanceShader[0] = trap_R_RegisterShader("models/weaves/fireball/trail");
			break;
		case WVW_A_EARTH_QUAKE_L:

			//misc resources
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/machinegun/par_shot_1.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/earthquake_s/proj_m.md5mesh");
			weaveInfo->missileLight = 50;
			MAKERGB(weaveInfo->missileLightColor, 0.5f, 0.1f, 0.1f);
			//weaveInfo->missileRenderfx;
			weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav");
			//weaveInfo->missileTrailFunc;

			//weaveInfo->exploModel;
			//weaveInfo->exploMark;
			//weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			//weaveInfo->exploSfx = trap_S_RegisterSound("sound/weapons/rocket/rocklx1a.ogg", qfalse);
			//weaveInfo->exploMark = cgs.media.bulletMarkShader;
			//weaveInfo->exploMarkRadius = 8;
			//weaveInfo->exploLight = 100;
			//MAKERGB(weaveInfo->exploLightColor, 0.75f, 0.5f, 0.1f);
			//weaveInfo->exploDuration = 800;

			//instance
			weaveInfo->instanceModel[0] = trap_R_RegisterModel("models/weaves/earthquake_s/earthquake_s01.md5mesh");
			weaveInfo->instanceModel[1] = trap_R_RegisterModel("models/weaves/earthquake_s/earthquake_s02.md5mesh");
			weaveInfo->instanceModel[2] = trap_R_RegisterModel("models/weaves/earthquake_s/earthquake_s03.md5mesh");
			weaveInfo->instanceModel[3] = trap_R_RegisterModel("models/weaves/earthquake_s/earthquake_s04.md5mesh");
			weaveInfo->instanceLight = 900;
			weaveInfo->instanceLightShader = trap_R_RegisterShader("weave_effects/earthquake/glow");
			MAKERGB(weaveInfo->instanceLightColor, 0.6f, 0.3f, 0.1f);
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			weaveInfo->instanceShader[0] = trap_R_RegisterShader("models/weaves/fireball/trail");
			break;
		case WVW_A_SPIRIT_SLICE_S:

			//misc resources
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/machinegun/par_shot_1.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/slice/slice1.md3");
			weaveInfo->missileLight = 100;
			MAKERGB(weaveInfo->missileLightColor, 0.6f, 0.6f, 0.6f);
			//weaveInfo->missileRenderfx;
			weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav");
			//weaveInfo->missileTrailFunc;

			//instance
			//weaveInfo->instanceModel;
			//weaveInfo->instanceLight;
			//weaveInfo->instanceLightColor;
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_A_SPIRIT_SLICE_M:

			//misc resources
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/machinegun/par_shot_1.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/slice/slice1.md3");
			weaveInfo->missileLight = 100;
			MAKERGB(weaveInfo->missileLightColor, 0.6f, 0.6f, 0.6f);
			//weaveInfo->missileRenderfx;
			weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav");
			//weaveInfo->missileTrailFunc;

			//instance
			//weaveInfo->instanceModel;
			//weaveInfo->instanceLight;
			//weaveInfo->instanceLightColor;
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_A_SPIRIT_SLICE_L:

			//misc resources
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/machinegun/par_shot_1.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/slice/slice1.md3");
			weaveInfo->missileLight = 100;
			MAKERGB(weaveInfo->missileLightColor, 0.6f, 0.6f, 0.6f);
			//weaveInfo->missileRenderfx;
			weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav");
			//weaveInfo->missileTrailFunc;

			//instance
			//weaveInfo->instanceModel;
			//weaveInfo->instanceLight;
			//weaveInfo->instanceLightColor;
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_A_SPIRIT_SHIELD:

			//misc resources
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/machinegun/par_shot_1.ogg");

			//missiles
			//weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/slice/slice1.md3", qtrue);
			//weaveInfo->missileLight = 100;
			//MAKERGB(weaveInfo->missileLightColor, 0.6f, 0.6f, 0.6f);
			//weaveInfo->missileRenderfx;
			//weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav", qfalse);
			//weaveInfo->missileTrailFunc;

			//instance
			//weaveInfo->instanceModel;
			//weaveInfo->instanceLight;
			//weaveInfo->instanceLightColor;
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			weaveInfo->instanceShader[0] = trap_R_RegisterShader("weave_effects/shield/blocked");
			break;
		case WVW_D_SPIRIT_LINK:

			//misc resources
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/machinegun/par_shot_1.ogg");

			//missiles
			//weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/slice/slice1.md3");
			//weaveInfo->missileLight = 100;
			//MAKERGB(weaveInfo->missileLightColor, 0.6f, 0.6f, 0.6f);
			//weaveInfo->missileRenderfx;
			//weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav", qfalse);
			//weaveInfo->missileTrailFunc;

			//instance
			//weaveInfo->instanceModel;
			//weaveInfo->instanceLight;
			MAKERGB(weaveInfo->instanceLightColor, 0.8f, 0.8f, 1.0f);
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			weaveInfo->instanceShader[0] = trap_R_RegisterShader("weave_effects/link/stream");
			break;
		case WVW_A_WATER_ICESHARDS_S:

			//misc resources
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weaves/iceshards/cast.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/iceshards/iceshard01.md5mesh");
			weaveInfo->missileModel[1] = trap_R_RegisterModel("models/weaves/iceshards/iceshard02.md5mesh");
			weaveInfo->missileModel[2] = trap_R_RegisterModel("models/weaves/iceshards/iceshard03.md5mesh");
			weaveInfo->missileModel[3] = trap_R_RegisterModel("models/weaves/iceshards/iceshard04.md5mesh");
			//weaveInfo->missileLight = 50;
			//MAKERGB(weaveInfo->missileLightColor, 0.1f, 0.1f, 0.3f);
			//weaveInfo->missileRenderfx;
			weaveInfo->missileSound = trap_S_RegisterSound("sound/weaves/iceshards/fly.ogg");
			//weaveInfo->missileTrailFunc;

			//weaveInfo->exploModel;
			//weaveInfo->exploMark;
			//weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->exploSfx = trap_S_RegisterSound("sound/weaves/iceshards/impact.ogg");
			weaveInfo->exploMark = cgs.media.bulletMarkShader;
			weaveInfo->exploMarkRadius = 8;
			weaveInfo->exploLight = 100;
			MAKERGB(weaveInfo->exploLightColor, 0.1f, 0.25f, 0.75f);
			weaveInfo->exploDuration = 800;

			//instance
			//weaveInfo->instanceModel;
			//weaveInfo->instanceLight;
			//weaveInfo->instanceLightColor;
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_A_WATER_ICESHARDS_M:

			//misc resources
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weaves/iceshards/cast.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/iceshards/iceshard01.md5mesh");
			weaveInfo->missileModel[1] = trap_R_RegisterModel("models/weaves/iceshards/iceshard02.md5mesh");
			weaveInfo->missileModel[2] = trap_R_RegisterModel("models/weaves/iceshards/iceshard03.md5mesh");
			weaveInfo->missileModel[3] = trap_R_RegisterModel("models/weaves/iceshards/iceshard04.md5mesh");
			//weaveInfo->missileLight = 50;
			//MAKERGB(weaveInfo->missileLightColor, 0.1f, 0.1f, 0.3f);
			//weaveInfo->missileRenderfx;
			weaveInfo->missileSound = trap_S_RegisterSound("sound/weaves/iceshards/fly.ogg");
			//weaveInfo->missileTrailFunc;

			//weaveInfo->exploModel;
			//weaveInfo->exploMark;
			//weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->exploSfx = trap_S_RegisterSound("sound/weaves/iceshards/impact.ogg");
			weaveInfo->exploMark = cgs.media.bulletMarkShader;
			weaveInfo->exploMarkRadius = 8;
			weaveInfo->exploLight = 100;
			MAKERGB(weaveInfo->exploLightColor, 0.1f, 0.25f, 0.75f);
			weaveInfo->exploDuration = 800;

			//instance
			//weaveInfo->instanceModel;
			//weaveInfo->instanceLight;
			//weaveInfo->instanceLightColor;
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_D_AIRFIRE_LIGHT:

			//misc resources
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/machinegun/par_shot_1.ogg");

			weaveInfo->instanceShader[0] = trap_R_RegisterShader("lightningBolt");
			//weaveInfo->instanceModel = trap_R_RegisterModel("models/projectiles/spike/spike.md3");
			weaveInfo->instanceLight = 400;
			MAKERGB(weaveInfo->instanceLightColor, 1, 1, 1);

			break;
		case WVW_D_AIRWATER_FOG:

			//misc resources
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/machinegun/par_shot_1.ogg");

			//weaveInfo->instanceShader[0] = trap_R_RegisterShader("lightningBolt");
			//weaveInfo->instanceModel = trap_R_RegisterModel("models/projectiles/spike/spike.md3");
			//weaveInfo->instanceLight = 400;
			//MAKERGB(weaveInfo->instanceLightColor, 1, 1, 1);

			break;
		case WVW_A_EARTHWATER_SLOW:

			//misc resources
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/machinegun/par_shot_1.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/projectiles/spike/spike.md3");
			weaveInfo->missileLight = 50;
			MAKERGB(weaveInfo->missileLightColor, 0.0f, 0.0f, 0.0f);
			//weaveInfo->missileRenderfx;
			weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav");
			//weaveInfo->missileTrailFunc;

			//weaveInfo->exploModel;
			//weaveInfo->exploMark;
			//weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			//weaveInfo->exploSfx = trap_S_RegisterSound("sound/weapons/rocket/rocklx1a.ogg", qfalse);
			weaveInfo->exploMark = cgs.media.bulletMarkShader;
			weaveInfo->exploMarkRadius = 8;
			weaveInfo->exploLight = 100;
			MAKERGB(weaveInfo->exploLightColor, 0.5f, 0.0f, 0.0f);
			weaveInfo->exploDuration = 800;

			//instance
			//weaveInfo->instanceModel;
			//weaveInfo->instanceLight;
			//weaveInfo->instanceLightColor;
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;

			break;
		case WVW_A_EARTHWATER_POISON:

			//misc resources
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/machinegun/par_shot_1.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/projectiles/spike/spike.md3");
			weaveInfo->missileLight = 50;
			MAKERGB(weaveInfo->missileLightColor, 0.0f, 0.0f, 0.0f);
			//weaveInfo->missileRenderfx;
			weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav");
			//weaveInfo->missileTrailFunc;

			//weaveInfo->exploModel;
			//weaveInfo->exploMark;
			//weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			//weaveInfo->exploSfx = trap_S_RegisterSound("sound/weapons/rocket/rocklx1a.ogg", qfalse);
			weaveInfo->exploMark = cgs.media.bulletMarkShader;
			weaveInfo->exploMarkRadius = 8;
			weaveInfo->exploLight = 100;
			MAKERGB(weaveInfo->exploLightColor, 0.5f, 0.0f, 0.0f);
			weaveInfo->exploDuration = 800;

			//instance
			//weaveInfo->instanceModel;
			//weaveInfo->instanceLight;
			//weaveInfo->instanceLightColor;
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;

		case WVW_D_WATER_CURE:
			break;

		case WVW_A_AIRWATER_RIP:
			break;

		case WVW_A_AIR_GRABPLAYER:
			break;

		case WVW_D_WATER_HEAL_S:
			weaveInfo->instanceShader[0] = trap_R_RegisterShader("weave_effects/heal/healparticle");
			break;

		case WVW_D_WATER_HEAL_M:
			weaveInfo->instanceShader[0] = trap_R_RegisterShader("weave_effects/heal/healparticle");
			break;

		case WVW_D_SPIRIT_STAMINA:
			break;

		case WVW_A_AIRFIRE_LIGHTNING:

			weaveInfo->instanceLight = 600;
			MAKERGB(weaveInfo->instanceLightColor, 0.8f, 0.8f, 1.0f);

			//instance
			//qhandle_t       instanceModel[MAX_WEAVE_INSTANCE_MODELS];
			//float           instanceLight;
			//vec3_t          instanceLightColor;
			//int             instanceRenderfx;
			//sfxHandle_t     instanceSound;
			//qhandle_t       instanceShader[1];

			break;

		case WVW_D_EARTHFIRE_EXPLOSIVE_S:

			weaveInfo->instanceModel[0] = trap_R_RegisterModel("models/projectiles/rocket/rocket.md3");
			weaveInfo->instanceLight = 70;
			MAKERGB(weaveInfo->instanceLightColor, 0.5f, 0.1f, 0.2f);

			weaveInfo->exploRadius = 65.0f;
			weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->exploSfx = trap_S_RegisterSound("sound/weapons/rocket/rocklx1a.ogg");
			weaveInfo->exploMark = cgs.media.burnMarkShader;
			weaveInfo->exploMarkRadius = 100;
			weaveInfo->exploLight = 350;
			MAKERGB(weaveInfo->exploLightColor, 0.95f, 0.4f, 0.1f);
			weaveInfo->exploDuration = 1000;

			break;

		case WVW_D_EARTHFIRE_EXPLOSIVE_M:

			weaveInfo->instanceModel[0] = trap_R_RegisterModel("models/projectiles/rocket/rocket.md3");
			weaveInfo->instanceLight = 100;
			MAKERGB(weaveInfo->instanceLightColor, 0.8f, 0.1f, 0.2f);

			weaveInfo->exploRadius = 70.0f;
			weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->exploSfx = trap_S_RegisterSound("sound/weapons/rocket/rocklx1a.ogg");
			weaveInfo->exploMark = cgs.media.burnMarkShader;
			weaveInfo->exploMarkRadius = 100;
			weaveInfo->exploLight = 350;
			MAKERGB(weaveInfo->exploLightColor, 0.95f, 0.4f, 0.1f);
			weaveInfo->exploDuration = 1000;

			break;

		default:
			break;
	}

	for(num = 0; num < MAX_WEAVE_MISSILE_MODELS; num++)
	{
		if(weaveInfo->missileModel[num])
		{
			weaveInfo->missileModelCount++;
		}
		else
		{
			break;
		}
	}
	Com_Printf("Loading weave %d, missile count %d\n", weaveNum, weaveInfo->missileModelCount);
}
