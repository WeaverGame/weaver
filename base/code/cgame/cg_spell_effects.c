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

// cg_spell_effects.c -- spell effect functions
#include "cg_local.h"


/*
==========================
CG_WeaveMissile_Trail

From CG_RocketTrail
==========================
*/
void CG_WeaveMissile_Trail(centity_t * ent)
{
	vec3_t          origin, lastPos;
	int             t;
	int             startTime, contents;
	int             lastContents;
	entityState_t  *es;
	vec3_t          up;
	localEntity_t  *smoke;
	const weaver_weaveCGInfo *weave;

	if(cg_noProjectileTrail.integer)
	{
		return;
	}

	weave = &cg_weaves[ent->currentState.weapon];

	if((weave->missileTrailDuration <= 0) || (weave->missileTrailStep <= 0))
	{
		return;
	}

	up[0] = 0;
	up[1] = 0;
	up[2] = 0;

	es = &ent->currentState;
	startTime = ent->trailTime;
	t = weave->missileTrailStep * ((startTime + weave->missileTrailStep) / weave->missileTrailStep);

	BG_EvaluateTrajectory(&es->pos, cg.time, origin);
	contents = CG_PointContents(origin, -1);

	// if object (e.g. grenade) is stationary, don't toss up smoke
	if(es->pos.trType == TR_STATIONARY)
	{
		ent->trailTime = cg.time;
		return;
	}

	BG_EvaluateTrajectory(&es->pos, ent->trailTime, lastPos);
	lastContents = CG_PointContents(lastPos, -1);

	ent->trailTime = cg.time;

	if(contents & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
	{
		if(contents & lastContents & CONTENTS_WATER)
		{
			CG_BubbleTrail(lastPos, origin, 8);
		}
		return;
	}

	for(; t <= ent->trailTime; t += weave->missileTrailStep)
	{
		BG_EvaluateTrajectory(&es->pos, t, lastPos);

		smoke =
			CG_SmokePuff(lastPos, up, weave->missileTrailRadius, 1, 1, 1, weave->missileTrailAlpha, weave->missileTrailDuration,
						 t, 0, 0, weave->missileTrailShader[0]);

		// use the optimized local entity add
		smoke->leType = LE_SCALE_FADE;
	}
}

void WeaveEffect_Shot(centity_t * cent)
{
	localEntity_t  *le;
	refEntity_t    *re;
	const weaver_weaveCGInfo *weave;
	vec3_t          dir;

	weave = &cg_weaves[cent->currentState.weapon];

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	VectorSubtract(cent->currentState.apos.trBase, cent->currentState.pos.trBase, dir);

	//Com_Printf("WeaveEffect_Shot time=%f\n", (VectorLength(dir)));

	//Initialize LE
	le->leType = LE_MOVING_REFENT;
	le->startTime = cg.time - 20;
	le->endTime = le->startTime + (VectorLength(dir) * 1000.0f / 16384.0f);
	le->length = 300;
	//le->lifeRate = 1.0 / (le->endTime - le->startTime);

	//Initialize RE
	//re->shaderTime = cg.time / 1000.0f;
	//re->reType = RT_LIGHTNING;
	//re->customShader = weave->instanceShader1;
	// convert direction of travel into axis
	re->hModel = weave->missileModel[0];
	re->renderfx = weave->missileRenderfx | RF_NOSHADOW;

	//Initialize movement
	le->pos.trType = TR_LINEAR;
	le->pos.trTime = le->startTime;
	VectorCopy(cent->currentState.pos.trBase, le->pos.trBase);
	VectorNormalize(dir);
	VectorScale(dir, 16384, le->pos.trDelta);
	VectorNormalize2(le->pos.trDelta, re->axis[0]);
	RotateAroundDirection(re->axis, le->startTime);

	CG_WeaveMissileHitWall(cent->currentState.weapon, cent->currentState.otherEntityNum, cent->currentState.apos.trBase, dir,
						   cent->currentState.number);
}

/*
================
WeaveEffect
Missile

Based on CG_Missile
================
*/
void WeaveEffect_Missile(centity_t * cent)
{
	refEntity_t     ent;
	entityState_t  *s1;
	const weaver_weaveCGInfo *weave;
	vec3_t          velocity;

	s1 = &cent->currentState;
	if(s1->weapon > WVW_NUM_WEAVES)
	{
		s1->weapon = 0;
	}
	weave = &cg_weaves[s1->weapon];

	if(!weave->registered)
	{
		return;
	}

	// calculate the axis
	VectorCopy(s1->angles, cent->lerpAngles);

	// add trails
	if(weave->missileTrailRadius)
	{
		CG_WeaveMissile_Trail(cent);
	}

	// add dynamic light
	if(weave->missileLight)
	{
		trap_R_AddLightToScene(cent->lerpOrigin, weave->missileLight, weave->missileLightColor[0], weave->missileLightColor[1],
							   weave->missileLightColor[2]);
	}

	// add missile sound
	if(weave->missileSound)
	{
		BG_EvaluateTrajectoryDelta(&cent->currentState.pos, cg.time, velocity);
		trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, velocity, weave->missileSound);
	}

	// create the render entity
	memset(&ent, 0, sizeof(ent));
	VectorCopy(cent->lerpOrigin, ent.origin);
	VectorCopy(cent->lerpOrigin, ent.oldorigin);

	// flicker between two skins
	ent.skinNum = cg.clientFrame & 1;
	if((weave->missileModelCount > 1) && (cent->currentState.modelindex > 0))
	{
		ent.hModel = weave->missileModel[cent->currentState.modelindex % weave->missileModelCount];
	}
	else
	{
		ent.hModel = weave->missileModel[0];
	}
	ent.renderfx = weave->missileRenderfx | RF_NOSHADOW;

	// convert direction of travel into axis
	if(VectorNormalize2(s1->pos.trDelta, ent.axis[0]) == 0)
	{
		ent.axis[0][2] = 1;
	}

	// spin as it moves
	if(s1->pos.trType != TR_STATIONARY)
	{
		RotateAroundDirection(ent.axis, cg.time / 4);
	}
	else
	{
		RotateAroundDirection(ent.axis, s1->time);
	}

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);
}

/*
================
WeaveEffect
Instance

For stuff that just sits there (and typically does stuff in server side think)
================
*/
void WeaveEffect_Instance(centity_t * cent)
{
	refEntity_t     ent;
	entityState_t  *s1;
	const weaver_weaveCGInfo *weave;

	s1 = &cent->currentState;
	if(s1->weapon > WVW_NUM_WEAVES)
	{
		s1->weapon = 0;
	}
	weave = &cg_weaves[s1->weapon];

	if(!weave->registered)
	{
		return;
	}

	// calculate the axis
	VectorCopy(s1->angles, cent->lerpAngles);

	// add dynamic light
	if(weave->instanceLight)
	{
		trap_R_AddLightToScene(cent->lerpOrigin, weave->instanceLight, weave->instanceLightColor[0], weave->instanceLightColor[1],
							   weave->instanceLightColor[2]);
	}

	// add missile sound
	if(weave->instanceSound)
	{
		trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, 0, weave->instanceSound);
	}

	// create the render entity
	memset(&ent, 0, sizeof(ent));
	VectorCopy(cent->lerpOrigin, ent.origin);
	VectorCopy(cent->lerpOrigin, ent.oldorigin);

	// flicker between two skins
	ent.skinNum = cg.clientFrame & 1;
	ent.hModel = weave->instanceModel[0];
	ent.renderfx = weave->instanceRenderfx | RF_NOSHADOW;

	// convert direction of travel into axis
	if(VectorNormalize2(s1->pos.trDelta, ent.axis[0]) == 0)
	{
		ent.axis[0][2] = 1;
	}

	// spin as it moves
	if(s1->pos.trType != TR_STATIONARY)
	{
		RotateAroundDirection(ent.axis, cg.time / 4);
	}
	else
	{
		RotateAroundDirection(ent.axis, s1->time);
	}

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);
}

/*
================
WeaveEffect
Earthquake

Instance + multiple models + vibrations + light
================
*/
void WeaveEffect_Earthquake(centity_t * cent)
{
	refEntity_t     ent;
	entityState_t  *s1;
	const weaver_weaveCGInfo *weave;
	vec3_t          normalaxis[3];
	int             k;
	refLight_t      light;

	s1 = &cent->currentState;
	if(s1->weapon > WVW_NUM_WEAVES)
	{
		s1->weapon = 0;
	}
	weave = &cg_weaves[s1->weapon];

	if(!weave->registered)
	{
		return;
	}

	memset(&light, 0, sizeof(light));

	// calculate the axis
	VectorCopy(s1->angles, cent->lerpAngles);

	// surface normal axis
	VectorNormalize2(cent->currentState.angles, normalaxis[2]);
	PerpendicularVector(normalaxis[1], normalaxis[2]);
	CrossProduct(normalaxis[1], normalaxis[2], normalaxis[0]);

	//Setup light
	QuatClear(light.rotation);
	VectorCopy(weave->instanceLightColor, light.color);
	VectorMA(cent->lerpOrigin, 10.0f, cent->currentState.angles, light.origin);
	light.radius[0] = weave->instanceLight;
	light.radius[1] = weave->instanceLight;
	light.radius[2] = weave->instanceLight;
	light.attenuationShader = weave->instanceLightShader;

	trap_R_AddRefLightToScene(&light);

	// add missile sound
	if(weave->instanceSound)
	{
		trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, 0, weave->instanceSound);
	}

	//Com_Printf("Normal=%s\n", vtos(normalaxis[2]));
	memset(&ent, 0, sizeof(ent));
	AxisCopy(normalaxis, ent.axis);

	for(k = 0; k < 4; k++)
	{
		// create the render entity

		VectorMA(cent->lerpOrigin, 4 * sin(k + (double)(cg.time >> 4) / 4.0f), cent->currentState.angles, ent.origin);

		// flicker between two skins
		ent.skinNum = cg.clientFrame;
		ent.hModel = weave->instanceModel[k];
		ent.renderfx = weave->instanceRenderfx | RF_NOSHADOW;

		// add to refresh list
		trap_R_AddRefEntityToScene(&ent);
	}
}

/*
===============
WeaveEffect_Protect
===============
*/
void WeaveEffect_Protect(centity_t * cent)
{
	playerEntity_t *pe;
	int             slot;

	if(cg.predictedPlayerState.clientNum == cent->currentState.generic1)
	{
		//Heldweave belong to this client.
		pe = &cg.predictedPlayerEntity.pe;
	}
	else
	{
		//Heldweave belong to another player.
		pe = &cg_entities[cent->currentState.generic1].pe;
	}

	switch(cent->currentState.weapon)
	{
		case WVW_D_AIR_PROTECT:
			slot = 0;
			break;
		case WVW_D_EARTH_PROTECT:
			slot = 1;
			break;
		case WVW_D_FIRE_PROTECT:
			slot = 2;
			break;
		case WVW_D_WATER_PROTECT:
			slot = 3;
			break;
		default:
			return;
	}

	pe->protectWeaveEnt[slot] = cent->currentState.number;
	pe->protectWeaveOn[slot] = qtrue;
}


/*
===============
CG_WeaveEffect_Link
===============
*/
void CG_WeaveEffect_Link(centity_t * cent)
{
	refEntity_t     beam;
	entityState_t  *s1;
	const weaver_weaveCGInfo *weave;

	//vec3_t          velocity;

	s1 = &cent->currentState;
	if(s1->weapon > WVW_NUM_WEAVES)
	{
		s1->weapon = 0;
	}
	weave = &cg_weaves[s1->weapon];

	memset(&beam, 0, sizeof(beam));

	VectorCopy(cent->lerpOrigin, beam.origin);
	//VectorCopy(cent->currentState.pos.trBase, beam.lightingOrigin);
	//AnglesToAxis(cent->currentState.angles, beam.axis);
	AxisClear(beam.axis);

	//Com_Printf("DRAWING A LINK\n");
	beam.reType = RT_MODEL;
	//beam.customShader = weave->instanceShader[0];
	beam.hModel = cgs.media.swordModel;
	trap_R_AddRefEntityToScene(&beam);
}

/*
===============
WeaveEffect_Light
===============
*/
void WeaveEffect_Light(centity_t * cent)
{
	centity_t      *player;
	refEntity_t     ent;
	entityState_t  *s1;
	const weaver_weaveCGInfo *weave;
	vec3_t          offset = { 0, 0, 50 };

	s1 = &cent->currentState;
	if(s1->weapon > WVW_NUM_WEAVES)
	{
		s1->weapon = 0;
	}
	weave = &cg_weaves[s1->weapon];

	if(!weave->registered)
	{
		return;
	}

	player = &cg_entities[cent->currentState.generic1];

	//initialize render entity
	memset(&ent, 0, sizeof(ent));

	VectorCopy(player->lerpOrigin, ent.origin);
	VectorAdd(ent.origin, offset, ent.origin);

	ent.customShader = weave->instanceShader[0];
	ent.reType = RT_SPRITE;
	ent.radius = 16;

	trap_R_AddLightToScene(ent.origin, weave->instanceLight, weave->instanceLightColor[0], weave->instanceLightColor[1],
						   weave->instanceLightColor[2]);

	trap_R_AddRefEntityToScene(&ent);
}

/*
===============
WeaveEffect_Lightning
===============
*/
void WeaveEffect_Lightning(centity_t * cent)
{
	vec3_t          mins = { -4.0f, -4.0f, -1 };
	vec3_t          maxs = { 4.0f, 4.0f, 20 };
	int             i;
	refEntity_t     beam;

	if(cent->currentState.eType != ET_WEAVE_EFFECT)
	{
		//Strike event
		memset(&beam, 0, sizeof(beam));

		for(i = 0; i < 4; i++)
		{
			CG_ExplosiveRubble(cent->lerpOrigin, mins, maxs, cgs.media.debrisModels[ENTMAT_STONE][0][i & 1]);
			CG_ExplosiveRubble(cent->lerpOrigin, mins, maxs, cgs.media.debrisModels[ENTMAT_STONE][1][i & 1]);
		}

		//Visual
		VectorCopy(cent->currentState.origin2, beam.oldorigin);
		VectorCopy(cent->lerpOrigin, beam.origin);
		beam.reType = RT_LIGHTNING;
		beam.radius = 100.0f;
		beam.customShader = cgs.media.lightningShader;
		trap_R_AddRefEntityToScene(&beam);

		//TODO: sound

		//TODO: light
	}
	else
	{
		// Regular lightning entity, warning static
		WeaveEffect_Instance(cent);
	}
}

/*
===============
WeaveEffect_Fog
===============
*/
void WeaveEffect_Fog(centity_t * cent)
{
	vec3_t          maxs = { 368.0f, 368.0f, 96.0f };
	int             i;
	vec3_t          vel;
	vec3_t          org;
	localEntity_t  *smoke;

	for(i = 0; i < 1; i++)
	{
		VectorCopy(vec3_origin, vel);
		VectorRandomUniform(vel, maxs);
		VectorCopy(cent->lerpOrigin, org);
		VectorRandomUniform(org, maxs);
		smoke = CG_SmokePuff(org, vel, 100.0f, 1.0f, 1.0f, 1.0f, 0.5, 5000,
			cg.time, 2000, 0, cgs.media.smokePuffShader);

		smoke->leType = LE_SCALE_FADE;
	}
}

/*
===============
CG_WeaveEffect
===============
*/
void CG_WeaveEffect(centity_t * cent)
{
	//CG_RegisterWeave(cent->currentState.weapon);
	switch (cent->currentState.weapon)
	{
			//Held special
		case WVW_A_AIRFIRE_LIGHTNING:
			WeaveEffect_Lightning(cent);
			break;
			//Remove effect (effect added when weaved, this executed to end it)
		case WVW_A_AIRFIRE_SWORD:
			break;
		case WVW_D_SPIRIT_TRAVEL:
			break;
		case WVW_D_AIRFIRE_LIGHT:
			WeaveEffect_Light(cent);
			break;
		case WVW_D_AIRWATER_FOG:
			WeaveEffect_Fog(cent);
			break;
			//protects - sprite on player
		case WVW_D_AIR_PROTECT:
		case WVW_D_FIRE_PROTECT:
		case WVW_D_EARTH_PROTECT:
		case WVW_D_WATER_PROTECT:
			WeaveEffect_Protect(cent);
			break;
			//Spawn Ent
		case WVW_D_AIRFIRE_WALL:
			break;
		case WVW_D_EARTHFIRE_EXPLOSIVE_S:
		case WVW_D_EARTHFIRE_EXPLOSIVE_M:
			WeaveEffect_Instance(cent);
			break;
			//Target
		case WVW_D_SPIRIT_LINK:
		case WVW_A_FIRE_BLOSSOMS:
		case WVW_A_SPIRIT_DEATHGATE:
		case WVW_D_EARTH_UNLOCK:
		case WVW_A_EARTHFIRE_IGNITE:
		case WVW_D_WATER_HEAL_S:
		case WVW_D_WATER_CURE:
		case WVW_D_WATER_HEAL_M:
		case WVW_D_SPIRIT_STAMINA:
		case WVW_D_AIRFIRE_INVIS:
		case WVW_A_SPIRIT_STILL:
		case WVW_D_AIR_GRAB:
		case WVW_A_AIR_GRABPLAYER:
		case WVW_A_AIR_BINDPLAYER:
		case WVW_A_SPIRIT_SHIELD:
			break;
		case WVW_A_AIRWATER_RIP:
			CG_ParticleBlood(cent->lerpOrigin, cent->lerpAngles, 75);
			break;
			//Sometimes Missiles
		case WVW_A_EARTH_QUAKE_S:
		case WVW_A_EARTH_QUAKE_M:
		case WVW_A_EARTH_QUAKE_L:
			if(cent->currentState.eType == ET_WEAVE_EFFECT)
			{
				//Ruptured ground effect
				WeaveEffect_Earthquake(cent);
			}
			else
			{
				//Missile
				WeaveEffect_Missile(cent);
			}
			break;
			//Shots
		case WVW_A_FIRE_DARTS:
		case WVW_A_FIRE_MULTIDARTS:
			WeaveEffect_Shot(cent);
			break;
			//Missiles
		case WVW_A_SPIRIT_SLICE_S:
		case WVW_A_SPIRIT_SLICE_M:
		case WVW_A_SPIRIT_SLICE_L:
		case WVW_A_SPIRIT_BALEFIRE:
		case WVW_A_AIR_BLAST:
		case WVW_A_FIRE_BALL:
		case WVW_A_AIRWATER_DARTS_S:
		case WVW_A_AIRWATER_DARTS_M:
		case WVW_A_EARTHWATER_SLOW:
		case WVW_A_EARTHWATER_POISON:
		case WVW_A_WATER_ICESHARDS_S:
		case WVW_A_WATER_ICESHARDS_M:
			WeaveEffect_Missile(cent);
			break;
			//Fail
		case WVW_NONE:
		case -1:
		default:
			break;
	}
}

/*
=================
CG_WeaveMissileHitWall

Based on CG_MissileHitWall
Caused by an EV_MISSILE_MISS event, or directly by local bullet tracing
=================
*/
void CG_WeaveMissileHitWall(int weapon, int clientNum, vec3_t origin, vec3_t dir, int entityNum)
{
	localEntity_t  *le;
	vec3_t          partOrigin;
	vec3_t          partVel;
	qboolean        isSprite;
	const weaver_weaveCGInfo *weave;

	weave = &cg_weaves[weapon];

	if(weave->exploSprite)
	{
		isSprite = qtrue;
	}
	else
	{
		isSprite = qfalse;
	}

	VectorMA(origin, weave->exploSparksOffset, dir, partOrigin);
	VectorScale(dir, weave->exploSparksVelScale, partVel);

	// Sparks
	if(weave->exploSparksDuration > 0)
	{
		CG_ParticleSparks(partOrigin, partVel, weave->exploSparksDuration, weave->exploSparksX, weave->exploSparksY,
						  weave->exploSparksSpeed);
	}

	// Sound
	if(weave->exploSfx)
	{
		trap_S_StartSound(origin, entityNum, CHAN_WEAPON, weave->exploSfx);
	}

	// create the explosion
	if(weave->exploModel || weave->exploSprite)
	{
		le = CG_MakeExplosion(origin, dir, weave->exploModel, weave->exploSprite, weave->exploDuration, isSprite);
		le->light = weave->exploLight;
		if(weave->exploRadius)
		{
			le->radius = weave->exploRadius;
		}
		VectorCopy(weave->exploLightColor, le->lightColor);
	}

	// impact mark
	if(weave->exploMark)
	{
		CG_ImpactMark(weave->exploMark, origin, dir, random() * 360, 1, 1, 1, 1, weave->exploMarkAlphaFade,
					  weave->exploMarkRadius, qfalse);
	}
}

/*
=================
CG_WeaveMissileHitPlayer
=================
*/
void CG_WeaveMissileHitPlayer(int weapon, vec3_t origin, vec3_t dir, int entityNum)
{
	// CG_Bleed(origin, entityNum);
	CG_ParticleBlood(origin, dir, 3);

	// some weapons will make an explosion with the blood, while
	// others will just make the blood
	CG_WeaveMissileHitWall(weapon, 0, origin, dir, entityNum);
}
