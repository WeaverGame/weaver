/*
===========================================================================
This file is part of the weaver cgame.

It has weave effect functions.
===========================================================================
*/

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
	const weaver_weaveInfo *weave;

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

		smoke = CG_SmokePuff(lastPos, up, weave->missileTrailRadius, 1, 1, 1, weave->missileTrailAlpha, weave->missileTrailDuration, t, 0, 0, weave->missileTrailShader[0]);

		// use the optimized local entity add
		smoke->leType = LE_SCALE_FADE;
	}
}

void WeaveEffect_Shot(centity_t * cent)
{
	localEntity_t  *le;
	refEntity_t    *re;
	const weaver_weaveInfo *weave;
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

	CG_WeaveMissileHitWall(cent->currentState.weapon, cent->currentState.otherEntityNum, cent->currentState.apos.trBase, dir);
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
	const weaver_weaveInfo *weave;
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
		trap_R_AddLightToScene(cent->lerpOrigin, weave->missileLight,
							   weave->missileLightColor[0], weave->missileLightColor[1], weave->missileLightColor[2]);
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
	const weaver_weaveInfo *weave;

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
		trap_R_AddLightToScene(cent->lerpOrigin, weave->instanceLight,
							   weave->instanceLightColor[0], weave->instanceLightColor[1], weave->instanceLightColor[2]);
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
	refEntity_t     ent[4];
	entityState_t  *s1;
	const weaver_weaveInfo *weave;
	vec3_t          normalaxis[3];
	int             k;

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
	//TODO: use light shader
	if(weave->instanceLight)
	{
		trap_R_AddLightToScene(cent->lerpOrigin, weave->instanceLight,
							   weave->instanceLightColor[0], weave->instanceLightColor[1], weave->instanceLightColor[2]);
	}

	// add missile sound
	if(weave->instanceSound)
	{
		trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, 0, weave->instanceSound);
	}

	// surface normal axis
	VectorNormalize2(cent->currentState.angles, normalaxis[2]);
	PerpendicularVector(normalaxis[1], normalaxis[2]);
	CrossProduct(normalaxis[1], normalaxis[2], normalaxis[0]);

	//Com_Printf("Normal=%s\n", vtos(normalaxis[2]));

	for(k = 0; k < 4; k++)
	{
		// create the render entity
		memset(&ent[k], 0, sizeof(ent[k]));

		VectorMA(cent->lerpOrigin, 4 * sin(k + (double)(cg.time >> 4) / 4.0f), cent->currentState.angles, ent[k].origin);

		// flicker between two skins
		ent[k].skinNum = cg.clientFrame;
		ent[k].hModel = weave->instanceModel[k];
		ent[k].renderfx = weave->instanceRenderfx | RF_NOSHADOW;

		AxisCopy(normalaxis, ent[k].axis);

		// add to refresh list
		trap_R_AddRefEntityToScene(&ent[k]);
	}
}

/*
===============
WeaveEffect_Protect
===============
*/
void WeaveEffect_Protect(centity_t * cent)
{
	refEntity_t     ent;
	centity_t      *player;
	entityState_t  *s1;
	vec3_t          offset = { 0, 0, 20 };
	const weaver_weaveInfo *weave;

	//vec3_t          velocity;

	s1 = &cent->currentState;
	if(s1->weapon > WVW_NUM_WEAVES)
	{
		s1->weapon = 0;
	}
	weave = &cg_weaves[s1->weapon];

	if(cg.clientNum == cent->currentState.generic1 && !cg.renderingThirdPerson)
	{
		//Threads belong to this player
		return;
	}

	player = &cg_entities[cent->currentState.generic1];

	if(!player)
	{
		return;
	}

	memset(&ent, 0, sizeof(ent));

	VectorCopy(player->lerpOrigin, ent.origin);
	VectorAdd(ent.origin, offset, ent.origin);

	//Com_Printf("DRAWING A PROTECT weaveid=%d\n", cent->currentState.weapon);
	ent.customShader = weave->instanceShader[0];
	ent.reType = RT_SPRITE;
	ent.radius = 40;

	trap_R_AddRefEntityToScene(&ent);
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
	const weaver_weaveInfo *weave;

	//vec3_t          velocity;

	s1 = &cent->currentState;
	if(s1->weapon > WVW_NUM_WEAVES)
	{
		s1->weapon = 0;
	}
	weave = &cg_weaves[s1->weapon];

	memset(&beam, 0, sizeof(beam));

	VectorCopy(cent->currentState.pos.trBase, beam.origin);
	VectorCopy(cent->currentState.origin2, beam.oldorigin);

	Com_Printf("DRAWING A LINK\n");
	beam.reType = RT_BEAM;
	beam.customShader = weave->instanceShader[0];
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
	const weaver_weaveInfo *weave;
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

	trap_R_AddLightToScene(ent.origin,
						   weave->instanceLight,
						   weave->instanceLightColor[0], weave->instanceLightColor[1], weave->instanceLightColor[2]);

	trap_R_AddRefEntityToScene(&ent);
}

/*
===============
WeaveEffect_Lightning
===============
*/
void WeaveEffect_Lightning(centity_t * cent)
{
	vec3_t          mins = { -32, -32, -1 };
	vec3_t          maxs = { 32, 32, 20 };
	int             i;

	//TODO: better explosion
	i = 0;
	CG_ExplosiveRubble(cent->lerpOrigin, mins, maxs, cgs.media.debrisModels[ENTMAT_STONE][0][i & 1]);

	//TODO: don't use WeaveEffect_Instance for lighting, instead spawn a ent with a light shader

	//TODO: don't use WeaveEffect_Instance for sound, play sound and

	//TODO: don't call WeaveEffect_Instance when above are done.
	WeaveEffect_Instance(cent);
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
			//Only display on event (strike)
			if((cent->currentState.event & ~EV_EVENT_BITS) == EV_WEAVE_SHOT)
			{
				WeaveEffect_Lightning(cent);
			}
			break;
			//Remove effect (effect added when weaved, this executed to end it)
		case WVW_A_AIRFIRE_SWORD:
		case WVW_D_SPIRIT_TRAVEL:
		case WVW_D_AIRWATER_FOG:
			break;
		case WVW_D_AIRFIRE_LIGHT:
			WeaveEffect_Light(cent);
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
void CG_WeaveMissileHitWall(int weapon, int clientNum, vec3_t origin, vec3_t dir)
{
	localEntity_t  *le;
	vec3_t          partOrigin;
	vec3_t          partVel;
	qboolean        isSprite;
	const weaver_weaveInfo *weave;

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
		CG_ParticleSparks(partOrigin, partVel, weave->exploSparksDuration, weave->exploSparksX, weave->exploSparksY, weave->exploSparksSpeed);
	}

	// Sound
	if(weave->exploSfx)
	{
		trap_S_StartSound(origin, ENTITYNUM_WORLD, CHAN_AUTO, weave->exploSfx);
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
		CG_ImpactMark(weave->exploMark, origin, dir, random() * 360, 1, 1, 1, 1, weave->exploMarkAlphaFade, weave->exploMarkRadius, qfalse);
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
	CG_WeaveMissileHitWall(weapon, 0, origin, dir);
}