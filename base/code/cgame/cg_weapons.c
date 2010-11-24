/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2002 Juergen Hoffmann
Copyright (C) 2006-2009 Robert Beckebans <trebor_7@users.sourceforge.net>

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
// cg_weapons.c -- events and effects dealing with weapons
#include "cg_local.h"

/*
==========================
CG_MachineGunEjectBrass
==========================
*/
/*static void CG_MachineGunEjectBrass(centity_t * cent)
{
	localEntity_t  *le;
	refEntity_t    *re;
	vec3_t          velocity, xvelocity;
	vec3_t          offset, xoffset;
	float           waterScale = 1.0f;
	vec3_t          v[3];

	if(cg_brassTime.integer <= 0)
	{
		return;
	}

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	velocity[0] = 0;
	velocity[1] = -50 + 40 * crandom();
	velocity[2] = 100 + 50 * crandom();

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + cg_brassTime.integer + (cg_brassTime.integer / 4) * random();

	le->pos.trType = TR_GRAVITY;
	le->pos.trAcceleration = cg_gravity.value;
	le->pos.trTime = cg.time - (rand() & 15);

	AnglesToAxis(cent->lerpAngles, v);

	offset[0] = 8;
	offset[1] = -4;
	offset[2] = 24;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
	VectorAdd(cent->lerpOrigin, xoffset, re->origin);

	VectorCopy(re->origin, le->pos.trBase);

	if(CG_PointContents(re->origin, -1) & CONTENTS_WATER)
	{
		waterScale = 0.10f;
	}

	xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
	xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
	xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
	VectorScale(xvelocity, waterScale, le->pos.trDelta);

	AxisCopy(axisDefault, re->axis);
	re->hModel = cgs.media.machinegunBrassModel;

	le->bounceFactor = 0.4 * waterScale;

	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	le->angles.trBase[0] = rand() & 31;
	le->angles.trBase[1] = rand() & 31;
	le->angles.trBase[2] = rand() & 31;
#if 0
	le->angles.trDelta[0] = 2;
	le->angles.trDelta[1] = 1;
	le->angles.trDelta[2] = 0;
#else
	// Tr3B - new quaternion code
	QuatFromAngles(le->quatOrient, le->angles.trBase[PITCH], le->angles.trBase[YAW], le->angles.trBase[ROLL]);
	le->angVel = 10 * random();
	le->rotAxis[0] = crandom();
	le->rotAxis[1] = crandom();
	le->rotAxis[2] = crandom();
	VectorNormalize(le->rotAxis);
	le->radius = 4;
	QuatClear(le->quatRot);
#endif
	le->leFlags = LEF_TUMBLE;
	le->leBounceSoundType = LEBS_BRASS;
	le->leMarkType = LEMT_NONE;
}*/

/*
==========================
CG_ShotgunEjectBrass
==========================
*/
static void CG_ShotgunEjectBrass(centity_t * cent)
{
	localEntity_t  *le;
	refEntity_t    *re;
	vec3_t          velocity, xvelocity;
	vec3_t          offset, xoffset;
	vec3_t          v[3];
	int             i;

	if(cg_brassTime.integer <= 0)
	{
		return;
	}

	for(i = 0; i < 2; i++)
	{
		float           waterScale = 1.0f;

		le = CG_AllocLocalEntity();
		re = &le->refEntity;

		velocity[0] = 60 + 60 * crandom();
		if(i == 0)
		{
			velocity[1] = 40 + 10 * crandom();
		}
		else
		{
			velocity[1] = -40 + 10 * crandom();
		}
		velocity[2] = 100 + 50 * crandom();

		le->leType = LE_FRAGMENT;
		le->startTime = cg.time;
		le->endTime = le->startTime + cg_brassTime.integer * 3 + cg_brassTime.integer * random();

		le->pos.trType = TR_GRAVITY;
		le->pos.trAcceleration = cg_gravity.value;
		le->pos.trTime = cg.time;

		AnglesToAxis(cent->lerpAngles, v);

		offset[0] = 8;
		offset[1] = 0;
		offset[2] = 24;

		xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
		xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
		xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
		VectorAdd(cent->lerpOrigin, xoffset, re->origin);
		VectorCopy(re->origin, le->pos.trBase);
		if(CG_PointContents(re->origin, -1) & CONTENTS_WATER)
		{
			waterScale = 0.10f;
		}

		xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
		xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
		xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
		VectorScale(xvelocity, waterScale, le->pos.trDelta);

		AxisCopy(axisDefault, re->axis);
		re->hModel = cgs.media.shotgunBrassModel;
		le->bounceFactor = 0.3f;

		le->angles.trType = TR_LINEAR;
		le->angles.trTime = cg.time;
		le->angles.trBase[0] = rand() & 31;
		le->angles.trBase[1] = rand() & 31;
		le->angles.trBase[2] = rand() & 31;
#if 0
		le->angles.trDelta[0] = 1;
		le->angles.trDelta[1] = 0.5;
		le->angles.trDelta[2] = 0;
#else
		// Tr3B - new quaternion code
		QuatFromAngles(le->quatOrient, le->angles.trBase[PITCH], le->angles.trBase[YAW], le->angles.trBase[ROLL]);
		le->angVel = 10 * random();
		le->rotAxis[0] = crandom();
		le->rotAxis[1] = crandom();
		le->rotAxis[2] = crandom();
		VectorNormalize(le->rotAxis);
		le->radius = 6;
		QuatClear(le->quatRot);
#endif

		le->leFlags = LEF_TUMBLE;
		le->leBounceSoundType = LEBS_BRASS;
		le->leMarkType = LEMT_NONE;
	}
}


/*
==========================
CG_NailgunEjectBrass
==========================
*/
static void CG_NailgunEjectBrass(centity_t * cent)
{
	localEntity_t  *smoke;
	vec3_t          origin;
	vec3_t          v[3];
	vec3_t          offset;
	vec3_t          xoffset;
	vec3_t          up;

	AnglesToAxis(cent->lerpAngles, v);

	offset[0] = 0;
	offset[1] = -12;
	offset[2] = 24;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
	VectorAdd(cent->lerpOrigin, xoffset, origin);

	VectorSet(up, 0, 0, 64);

	smoke = CG_SmokePuff(origin, up, 32, 1, 1, 1, 0.33f, 700, cg.time, 0, 0, cgs.media.smokePuffShader);
	// use the optimized local entity add
	smoke->leType = LE_SCALE_FADE;
}


/*
==========================
CG_RailTrail
==========================
*/
void CG_RailTrail(clientInfo_t * ci, vec3_t start, vec3_t end)
{
	if(cg_railType.integer == 1)
	{
		localEntity_t  *le;
		refEntity_t    *re;

		// rings
		le = CG_AllocLocalEntity();
		re = &le->refEntity;

		le->leType = LE_FADE_RGB;
		le->startTime = cg.time;
		le->endTime = cg.time + cg_railTrailTime.value;
		le->lifeRate = 1.0 / (le->endTime - le->startTime);

		re->shaderTime = cg.time / 1000.0f;
		re->reType = RT_RAIL_RINGS;
		re->customShader = cgs.media.railRingsShader;

		VectorCopy(start, re->origin);
		VectorCopy(end, re->oldorigin);

		// nudge down a bit so it isn't exactly in center
		re->origin[2] -= 8;
		re->oldorigin[2] -= 8;

		le->color[0] = ci->color2[0] * 0.75;
		le->color[1] = ci->color2[1] * 0.75;
		le->color[2] = ci->color2[2] * 0.75;
		le->color[3] = 1.0f;

		AxisClear(re->axis);

		// core
		le = CG_AllocLocalEntity();
		re = &le->refEntity;

		le->leType = LE_FADE_RGB;
		le->startTime = cg.time;
		le->endTime = cg.time + cg_railTrailTime.value;
		le->lifeRate = 1.0 / (le->endTime - le->startTime);

		re->shaderTime = cg.time / 1000.0f;
		re->reType = RT_RAIL_CORE;
		re->customShader = cgs.media.railCoreShader;

		VectorCopy(start, re->origin);
		VectorCopy(end, re->oldorigin);

		// nudge down a bit so it isn't exactly in center
		re->origin[2] -= 8;
		re->oldorigin[2] -= 8;

		le->color[0] = ci->color1[0] * 0.75;
		le->color[1] = ci->color1[1] * 0.75;
		le->color[2] = ci->color1[2] * 0.75;
		le->color[3] = 1.0f;

		AxisClear(re->axis);
	}
	else if(cg_railType.integer == 2)
	{
		vec3_t          axis[36], move, move2, next_move, vec, temp;
		float           len;
		int             i, j, skip;

		localEntity_t  *le;
		refEntity_t    *re;

#define RADIUS   4
#define ROTATION 1
#define SPACING  5

		start[2] -= 4;
		VectorCopy(start, move);
		VectorSubtract(end, start, vec);
		len = VectorNormalize(vec);
		PerpendicularVector(temp, vec);
		for(i = 0; i < 36; i++)
			RotatePointAroundVector(axis[i], vec, temp, i * 10);

		le = CG_AllocLocalEntity();
		re = &le->refEntity;

		le->leType = LE_FADE_RGB;
		le->startTime = cg.time;
		le->endTime = cg.time + cg_railTrailTime.value;
		le->lifeRate = 1.0 / (le->endTime - le->startTime);

		re->shaderTime = cg.time / 1000.0f;
		re->reType = RT_RAIL_CORE;
		re->customShader = cgs.media.railCoreShader;

		VectorCopy(start, re->origin);
		VectorCopy(end, re->oldorigin);

		re->shaderRGBA[0] = ci->color1[0] * 255;
		re->shaderRGBA[1] = ci->color1[1] * 255;
		re->shaderRGBA[2] = ci->color1[2] * 255;
		re->shaderRGBA[3] = 255;

		le->color[0] = ci->color1[0] * 0.75;
		le->color[1] = ci->color1[1] * 0.75;
		le->color[2] = ci->color1[2] * 0.75;
		le->color[3] = 1.0f;

		AxisClear(re->axis);

		VectorMA(move, 20, vec, move);
		VectorCopy(move, next_move);
		VectorScale(vec, SPACING, vec);

		skip = -1;

		j = 18;
		for(i = 0; i < len; i += SPACING)
		{
			if(i != skip)
			{
				skip = i + SPACING;
				le = CG_AllocLocalEntity();
				re = &le->refEntity;
				le->leFlags = LEF_PUFF_DONT_SCALE;
				le->leType = LE_MOVE_SCALE_FADE;
				le->startTime = cg.time;
				le->endTime = cg.time + (i >> 1) + 600;
				le->lifeRate = 1.0 / (le->endTime - le->startTime);

				re->shaderTime = cg.time / 1000.0f;
				re->reType = RT_SPRITE;
				re->radius = 1.1f;
				re->customShader = cgs.media.railRings2Shader;

				re->shaderRGBA[0] = ci->color2[0] * 255;
				re->shaderRGBA[1] = ci->color2[1] * 255;
				re->shaderRGBA[2] = ci->color2[2] * 255;
				re->shaderRGBA[3] = 255;

				le->color[0] = ci->color2[0] * 0.75;
				le->color[1] = ci->color2[1] * 0.75;
				le->color[2] = ci->color2[2] * 0.75;
				le->color[3] = 1.0f;

				le->pos.trType = TR_LINEAR;
				le->pos.trTime = cg.time;

				VectorCopy(move, move2);
				VectorMA(move2, RADIUS, axis[j], move2);
				VectorCopy(move2, le->pos.trBase);

				le->pos.trDelta[0] = axis[j][0] * 6;
				le->pos.trDelta[1] = axis[j][1] * 6;
				le->pos.trDelta[2] = axis[j][2] * 6;
			}

			VectorAdd(move, vec, move);

			j = j + ROTATION < 36 ? j + ROTATION : (j + ROTATION) % 36;
		}
	}
}

/*
==========================
CG_RocketTrail
==========================
*/
static void CG_RocketTrail(centity_t * ent, const weaponInfo_t * wi)
{
	int             step;
	vec3_t          origin, lastPos;
	int             t;
	int             startTime, contents;
	int             lastContents;
	entityState_t  *es;
	vec3_t          up;
	localEntity_t  *smoke;

	if(cg_noProjectileTrail.integer)
	{
		return;
	}

	up[0] = 0;
	up[1] = 0;
	up[2] = 0;

	step = 50;

	es = &ent->currentState;
	startTime = ent->trailTime;
	t = step * ((startTime + step) / step);

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

	for(; t <= ent->trailTime; t += step)
	{
		BG_EvaluateTrajectory(&es->pos, t, lastPos);

		smoke = CG_SmokePuff(lastPos, up, wi->trailRadius, 1, 1, 1, 0.33f, wi->wiTrailTime, t, 0, 0, cgs.media.smokePuffShader);

		// use the optimized local entity add
		smoke->leType = LE_SCALE_FADE;
	}


//  BG_EvaluateTrajectory(&es->pos, ent->trailTime, lastPos);
//  CG_ParticleRocketFire(origin, lastPos);
}


/*
==========================
CG_NailTrail
==========================
*/
static void CG_NailTrail(centity_t * ent, const weaponInfo_t * wi)
{
	int             step;
	vec3_t          origin, lastPos;
	int             t;
	int             startTime, contents;
	int             lastContents;
	entityState_t  *es;
	vec3_t          up;
	localEntity_t  *smoke;

	if(cg_noProjectileTrail.integer)
	{
		return;
	}

	up[0] = 0;
	up[1] = 0;
	up[2] = 0;

	step = 50;

	es = &ent->currentState;
	startTime = ent->trailTime;
	t = step * ((startTime + step) / step);

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

	for(; t <= ent->trailTime; t += step)
	{
		BG_EvaluateTrajectory(&es->pos, t, lastPos);

		smoke = CG_SmokePuff(lastPos, up, wi->trailRadius, 1, 1, 1, 0.33f, wi->wiTrailTime, t, 0, 0, cgs.media.nailPuffShader);
		// use the optimized local entity add
		smoke->leType = LE_SCALE_FADE;
	}
}


/*
==========================
CG_PlasmaTrail
==========================
*/
/*static void CG_PlasmaTrail(centity_t * cent, const weaponInfo_t * wi)
{
	localEntity_t  *le;
	refEntity_t    *re;
	entityState_t  *es;
	vec3_t          velocity, xvelocity, origin;
	vec3_t          offset, xoffset;
	vec3_t          v[3];
	int             t, startTime, step;
	float           waterScale = 1.0f;

	if(cg_noProjectileTrail.integer)
		return;

	step = 50;

	es = &cent->currentState;
	startTime = cent->trailTime;
	t = step * ((startTime + step) / step);

	BG_EvaluateTrajectory(&es->pos, cg.time, origin);

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	velocity[0] = 60 - 120 * crandom();
	velocity[1] = 40 - 80 * crandom();
	velocity[2] = 100 - 200 * crandom();

	le->leType = LE_MOVE_SCALE_FADE;
	le->leFlags = LEF_TUMBLE;
	le->leBounceSoundType = LEBS_NONE;
	le->leMarkType = LEMT_NONE;

	le->startTime = cg.time;
	le->endTime = le->startTime + 600;

	le->pos.trType = TR_GRAVITY;
	le->pos.trAcceleration = cg_gravity.value;
	le->pos.trTime = cg.time;

	AnglesToAxis(cent->lerpAngles, v);

	offset[0] = 2;
	offset[1] = 2;
	offset[2] = 2;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];

	VectorAdd(origin, xoffset, re->origin);
	VectorCopy(re->origin, le->pos.trBase);

	if(CG_PointContents(re->origin, -1) & CONTENTS_WATER)
	{
		waterScale = 0.10f;
	}

	xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
	xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
	xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
	VectorScale(xvelocity, waterScale, le->pos.trDelta);

	AxisCopy(axisDefault, re->axis);
	re->shaderTime = cg.time / 1000.0f;
	re->reType = RT_SPRITE;
	re->radius = 0.25f;
	re->customShader = cgs.media.railRings2Shader;
	le->bounceFactor = 0.3f;

	re->shaderRGBA[0] = wi->flashLightColor[0] * 63;
	re->shaderRGBA[1] = wi->flashLightColor[1] * 63;
	re->shaderRGBA[2] = wi->flashLightColor[2] * 63;
	re->shaderRGBA[3] = 63;

	le->color[0] = wi->flashLightColor[0] * 0.2;
	le->color[1] = wi->flashLightColor[1] * 0.2;
	le->color[2] = wi->flashLightColor[2] * 0.2;
	le->color[3] = 0.25f;

	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	le->angles.trBase[0] = rand() & 31;
	le->angles.trBase[1] = rand() & 31;
	le->angles.trBase[2] = rand() & 31;
	le->angles.trDelta[0] = 1;
	le->angles.trDelta[1] = 0.5;
	le->angles.trDelta[2] = 0;

}*/

/*
==========================
CG_GrappleTrail
==========================
*/
void CG_GrappleTrail(centity_t * ent, const weaponInfo_t * wi)
{
	vec3_t          origin;
	entityState_t  *es;
	vec3_t          forward, up;
	refEntity_t     beam;

	es = &ent->currentState;

	BG_EvaluateTrajectory(&es->pos, cg.time, origin);
	ent->trailTime = cg.time;

	memset(&beam, 0, sizeof(beam));
	//FIXME adjust for muzzle position
	VectorCopy(cg_entities[ent->currentState.otherEntityNum].lerpOrigin, beam.origin);
	beam.origin[2] += 26;
	AngleVectors(cg_entities[ent->currentState.otherEntityNum].lerpAngles, forward, NULL, up);
	VectorMA(beam.origin, -6, up, beam.origin);
	VectorCopy(origin, beam.oldorigin);

	if(Distance(beam.origin, beam.oldorigin) < 64)
		return;					// Don't draw if close

	beam.reType = RT_LIGHTNING;
	beam.customShader = cgs.media.lightningShader;

	AxisClear(beam.axis);
	beam.shaderRGBA[0] = 0xff;
	beam.shaderRGBA[1] = 0xff;
	beam.shaderRGBA[2] = 0xff;
	beam.shaderRGBA[3] = 0xff;
	trap_R_AddRefEntityToScene(&beam);
}

/*
==========================
CG_GrenadeTrail
==========================
*/
static void CG_GrenadeTrail(centity_t * ent, const weaponInfo_t * wi)
{
	CG_RocketTrail(ent, wi);
}


static qboolean CG_RegisterWeaponAnimation(animation_t * anim, const char *filename, qboolean loop, qboolean reversed,
										   qboolean clearOrigin)
{
	int             frameRate;

	anim->handle = trap_R_RegisterAnimation(filename);
	if(!anim->handle)
	{
		Com_Printf("Failed to load animation file %s\n", filename);
		return qfalse;
	}

	anim->firstFrame = 0;
	anim->numFrames = trap_R_AnimNumFrames(anim->handle);
	frameRate = trap_R_AnimFrameRate(anim->handle);

	if(frameRate == 0)
	{
		frameRate = 1;
	}
	anim->frameTime = 1000 / frameRate;
	anim->initialLerp = 1000 / frameRate;

	if(loop)
	{
		anim->loopFrames = anim->numFrames;
	}
	else
	{
		anim->loopFrames = 0;
	}

	anim->reversed = reversed;
	anim->clearOrigin = clearOrigin;

	return qtrue;
}

/*
=================
CG_RegisterWeapon

The server says this item is used on this level
=================
*/
void CG_RegisterWeapon(int weaponNum)
{
	weaponInfo_t   *weaponInfo;
	gitem_t        *item, *ammo;
	char            path[MAX_QPATH];
	vec3_t          mins, maxs;
	int             i;

	weaponInfo = &cg_weapons[weaponNum];

	if(weaponNum == 0)
	{
		return;
	}

	if(weaponNum >= MIN_WEAPON_WEAVE)
	{
		//this is a weave.
		//Com_Printf("CG_FireWeapon: Weave registered\n");
		weaponInfo->registered = qtrue;
		return;
	}

	if(weaponInfo->registered)
	{
		return;
	}

	memset(weaponInfo, 0, sizeof(*weaponInfo));
	weaponInfo->registered = qtrue;

	for(item = bg_itemlist + 1; item->classname; item++)
	{
		if(item->giType == IT_WEAPON && item->giTag == weaponNum)
		{
			weaponInfo->item = item;
			break;
		}
	}
	if(!item->classname)
	{
		CG_Error("Couldn't find weapon %i", weaponNum);
	}
	CG_RegisterItemVisuals(item - bg_itemlist);

	// load cmodel before model so filecache works
	weaponInfo->weaponModel = trap_R_RegisterModel(item->models[0], qtrue);

	// try to load .md5mesh model if the .md3 could not be found
	if(!weaponInfo->weaponModel)
	{
		strcpy(path, item->models[0]);
		Com_StripExtension(path, path, sizeof(path));
		strcat(path, ".md5mesh");
		weaponInfo->weaponModel = trap_R_RegisterModel(path, qtrue);
	}

	// calc midpoint for rotation
	trap_R_ModelBounds(weaponInfo->weaponModel, mins, maxs);
	for(i = 0; i < 3; i++)
	{
		weaponInfo->weaponMidpoint[i] = mins[i] + 0.5 * (maxs[i] - mins[i]);
	}

	weaponInfo->weaponIcon = trap_R_RegisterShader(item->icon);
	weaponInfo->ammoIcon = trap_R_RegisterShader(item->icon);

	for(ammo = bg_itemlist + 1; ammo->classname; ammo++)
	{
		if(ammo->giType == IT_AMMO && ammo->giTag == weaponNum)
		{
			break;
		}
	}
	if(ammo->classname && ammo->models[0])
	{
		weaponInfo->ammoModel = trap_R_RegisterModel(ammo->models[0], qtrue);
	}

	strcpy(path, item->models[0]);
	Com_StripExtension(path, path, sizeof(path));
	strcat(path, "_flash.md3");
	weaponInfo->flashModel = trap_R_RegisterModel(path, qtrue);

	strcpy(path, item->models[0]);
	Com_StripExtension(path, path, sizeof(path));
	strcat(path, "_barrel.md3");
	weaponInfo->barrelModel = trap_R_RegisterModel(path, qtrue);

	strcpy(path, item->models[0]);
	Com_StripExtension(path, path, sizeof(path));
	strcat(path, "_hand.md3");
	weaponInfo->handsModel = trap_R_RegisterModel(path, qfalse);

	strcpy(path, item->models[0]);
	Com_StripExtension(path, path, sizeof(path));
	strcat(path, "_view.md5mesh");
	weaponInfo->viewModel = trap_R_RegisterModel(path, qfalse);

	if(weaponInfo->viewModel)
	{
		strcpy(path, item->models[0]);
		Com_StripExtension(path, path, sizeof(path));
		strcat(path, "_view_idle.md5anim");
		if(!CG_RegisterWeaponAnimation(&weaponInfo->viewModel_animations[WEAPON_READY], path, qtrue, qfalse, qfalse))
		{
			CG_Error("could not find '%s'", path);
		}

		// default all weapon animations to the idle animation
		for(i = 0; i < MAX_WEAPON_STATES; i++)
		{
			if(i == WEAPON_READY)
				continue;

			weaponInfo->viewModel_animations[i] = weaponInfo->viewModel_animations[WEAPON_READY];
		}

		strcpy(path, item->models[0]);
		Com_StripExtension(path, path, sizeof(path));
		strcat(path, "_view_raise.md5anim");
		CG_RegisterWeaponAnimation(&weaponInfo->viewModel_animations[WEAPON_RAISING], path, qfalse, qfalse, qfalse);

		strcpy(path, item->models[0]);
		Com_StripExtension(path, path, sizeof(path));
		strcat(path, "_view_lower.md5anim");
		CG_RegisterWeaponAnimation(&weaponInfo->viewModel_animations[WEAPON_DROPPING], path, qfalse, qfalse, qfalse);

		strcpy(path, item->models[0]);
		Com_StripExtension(path, path, sizeof(path));
		strcat(path, "_view_fire.md5anim");
		CG_RegisterWeaponAnimation(&weaponInfo->viewModel_animations[WEAPON_FIRING], path, qtrue, qfalse, qfalse);
	}

	/*
	   if(!weaponInfo->handsModel)
	   {
	   weaponInfo->handsModel = trap_R_RegisterModel("models/weapons/shotgun/shotgun_hand.md3");
	   }
	 */

	weaponInfo->loopFireSound = qfalse;

	switch (weaponNum)
	{
		case WP_GAUNTLET:
			MAKERGB(weaponInfo->flashLightColor, 0.6f, 0.6f, 1.0f);
			weaponInfo->projectileModel = trap_R_RegisterModel("models/sword/sword.md5mesh", qtrue);
			weaponInfo->projectileTrailFunc = CG_GrappleTrail;
			weaponInfo->projectileLight = 200;
			weaponInfo->wiTrailTime = 2000;
			weaponInfo->trailRadius = 64;
			MAKERGB(weaponInfo->projectileLightColor, 1, 0.75f, 0);
			//weaponInfo->readySound = trap_S_RegisterSound("sound/weapons/melee/fsthum.wav", qfalse);
			weaponInfo->firingSound = trap_S_RegisterSound("sound/weapons/gauntlet/electrocute.ogg");
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/gauntlet/slashkut.ogg");
			cgs.media.lightningShader = trap_R_RegisterShader("lightningBolt");
			break;

		default:
			MAKERGB(weaponInfo->flashLightColor, 1, 1, 1);
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.ogg");
			break;
	}
}

/*
=================
CG_RegisterItemVisuals

The server says this item is used on this level
=================
*/
void CG_RegisterItemVisuals(int itemNum)
{
	itemInfo_t     *itemInfo;
	gitem_t        *item;

	if(itemNum < 0 || itemNum >= bg_numItems)
	{
		CG_Error("CG_RegisterItemVisuals: itemNum %d out of range [0-%d]", itemNum, bg_numItems - 1);
	}

	itemInfo = &cg_items[itemNum];
	if(itemInfo->registered)
	{
		return;
	}

	item = &bg_itemlist[itemNum];

	memset(itemInfo, 0, sizeof(&itemInfo));
	itemInfo->registered = qtrue;

	itemInfo->models[0] = trap_R_RegisterModel(item->models[0], qtrue);
	if(item->skins[0])
	{
		itemInfo->skins[0] = trap_R_RegisterSkin(item->skins[0]);
	}

	itemInfo->icon = trap_R_RegisterShader(item->icon);

	if(item->giType == IT_WEAPON)
	{
		CG_RegisterWeapon(item->giTag);
	}

	// powerups have an accompanying ring or sphere
	if(item->giType == IT_POWERUP || item->giType == IT_HEALTH || item->giType == IT_ARMOR || item->giType == IT_HOLDABLE)
	{
		if(item->models[1])
		{
			itemInfo->models[1] = trap_R_RegisterModel(item->models[1], qtrue);
		}

		if(item->skins[1])
		{
			itemInfo->skins[1] = trap_R_RegisterSkin(item->skins[1]);
		}
	}
}


/*
========================================================================================

VIEW WEAPON

========================================================================================
*/

/*
=================
CG_MapTorsoToWeaponFrame
=================
*/
static int CG_MapTorsoToWeaponFrame(clientInfo_t * ci, int frame)
{
	// change weapon
	if(frame >= ci->animations[TORSO_DROP].firstFrame && frame < ci->animations[TORSO_DROP].firstFrame + 9)
	{
		return frame - ci->animations[TORSO_DROP].firstFrame + 6;
	}

	// stand attack
	if(frame >= ci->animations[TORSO_ATTACK].firstFrame && frame < ci->animations[TORSO_ATTACK].firstFrame + 6)
	{
		return 1 + frame - ci->animations[TORSO_ATTACK].firstFrame;
	}

	// stand attack 2
	if(frame >= ci->animations[TORSO_ATTACK2].firstFrame && frame < ci->animations[TORSO_ATTACK2].firstFrame + 6)
	{
		return 1 + frame - ci->animations[TORSO_ATTACK2].firstFrame;
	}

	return 0;
}


/*
===============
CG_SetWeaponLerpFrameAnimation

may include ANIM_TOGGLEBIT
===============
*/
static void CG_SetWeaponLerpFrameAnimation(weaponInfo_t * wi, lerpFrame_t * lf, int weaponNumber, int weaponAnimation,
										   int weaponTime)
{
	animation_t    *anim;
	int             shouldTime, wouldTime;

	// save old animation
	lf->old_animationNumber = lf->animationNumber;
	lf->old_animation = lf->animation;
	lf->old_weaponNumber = lf->weaponNumber;

	lf->weaponNumber = weaponNumber;
	lf->animationNumber = weaponAnimation;

	if(weaponAnimation < 0 || weaponAnimation >= MAX_WEAPON_STATES)
	{
		CG_Error("bad weapon animation number: %i", weaponAnimation);
	}

	anim = &wi->viewModel_animations[weaponAnimation];

	lf->animation = anim;
	lf->animationStartTime = lf->frameTime + anim->initialLerp;

	shouldTime = weaponTime;
	wouldTime = anim->numFrames * anim->frameTime;

	if(shouldTime != wouldTime && shouldTime > 0)
	{
		lf->animationScale = (float)wouldTime / shouldTime;
	}
	else
	{
		lf->animationScale = 1.0f;
	}

	if(lf->old_animationNumber <= 0 || lf->old_weaponNumber != lf->weaponNumber)
	{
		// skip initial / invalid blending
		lf->blendlerp = 0.0f;
		return;
	}

	// TODO: blend through two blendings!

	if((lf->blendlerp <= 0.0f))
		lf->blendlerp = 1.0f;
	else
		lf->blendlerp = 1.0f - lf->blendlerp;	// use old blending for smooth blending between two blended animations

#if 0
	memcpy(&lf->oldSkeleton, &lf->skeleton, sizeof(refSkeleton_t));
#else
	if(!trap_R_BuildSkeleton(&lf->oldSkeleton, lf->old_animation->handle, lf->oldFrame, lf->frame, lf->blendlerp, lf->old_animation->clearOrigin))
	{
		CG_Printf("CG_SetWeaponLerpFrameAnimation: can't build old skeleton\n");
		return;
	}
#endif

	if(cg_debugWeaponAnim.integer)
	{
		Com_Printf("CG_SetWeaponLerpFrameAnimation: weapon=%i new anim=%i old anim=%i time=%i\n", weaponNumber, weaponAnimation, lf->old_animationNumber, weaponTime);
	}
}

/*
===============
CG_RunWeaponLerpFrame

Sets cg.snap, cg.oldFrame, and cg.backlerp
cg.time should be between oldFrameTime and frameTime after exit
===============
*/
static void CG_RunWeaponLerpFrame(weaponInfo_t * wi, lerpFrame_t * lf, int weaponNumber, int weaponAnimation, int weaponTime,
								  float speedScale)
{
	int             f, numFrames;
	animation_t    *anim;
	qboolean        animChanged;

	// debugging tool to get no animations
	if(cg_animSpeed.integer == 0)
	{
		lf->oldFrame = lf->frame = lf->backlerp = 0;
		return;
	}

	// see if the animation sequence is switching
	if(weaponAnimation != lf->animationNumber || !lf->animation || lf->weaponNumber != weaponNumber)
	{
		CG_SetWeaponLerpFrameAnimation(wi, lf, weaponNumber, weaponAnimation, weaponTime);

		if(!lf->animation)
		{
			memcpy(&lf->oldSkeleton, &lf->skeleton, sizeof(refSkeleton_t));
		}

		animChanged = qtrue;
	}
	else
	{
		animChanged = qfalse;
	}

	// if we have passed the current frame, move it to
	// oldFrame and calculate a new frame
	if(cg.time >= lf->frameTime || animChanged)
	{
		if(animChanged)
		{
			lf->oldFrame = 0;
			lf->oldFrameTime = cg.time;
		}
		else
		{
			lf->oldFrame = lf->frame;
			lf->oldFrameTime = lf->frameTime;
		}

		// get the next frame based on the animation
		anim = lf->animation;
		if(!anim->frameTime)
		{
			if(cg_debugWeaponAnim.integer)
			{
				CG_Printf("!anim->frameTime\n");
			}
			return;				// shouldn't happen
		}

		if(cg.time < lf->animationStartTime)
		{
			lf->frameTime = lf->animationStartTime;	// initial lerp
		}
		else
		{
			lf->frameTime = lf->oldFrameTime + anim->frameTime;
		}

		f = (lf->frameTime - lf->animationStartTime) / anim->frameTime;
		f *= lf->animationScale;
		f *= speedScale;		// adjust for haste, etc

		//CG_Printf("CG_RunWeaponLerpFrame: lf->frameTime=%i anim->frameTime=%i startTime=%i frame=%i weapon=%i\n", lf->frameTime, anim->frameTime, lf->animationStartTime, f, weaponNumber);

		numFrames = anim->numFrames;

		if(anim->flipflop)
		{
			numFrames *= 2;
		}

		if(f >= numFrames)
		{
			f -= numFrames;

			if(anim->loopFrames)
			{
				//CG_Printf("CG_RunWeaponLerpFrame: looping animation %i for weapon %i\n", weaponAnimation, weaponNumber);

				//f %= anim->numFrames;
				f %= anim->loopFrames;
				f += anim->numFrames - anim->loopFrames;
			}
			else
			{
				f = numFrames - 1;
				// the animation is stuck at the end, so it
				// can immediately transition to another sequence
				lf->frameTime = cg.time;
			}
		}

		if(anim->reversed)
		{
			lf->frame = anim->firstFrame + anim->numFrames - 1 - f;
		}
		else if(anim->flipflop && f >= anim->numFrames)
		{
			lf->frame = anim->firstFrame + anim->numFrames - 1 - (f % anim->numFrames);
		}
		else
		{
			lf->frame = anim->firstFrame + f;
		}

		if(cg.time > lf->frameTime)
		{
			lf->frameTime = cg.time;
			if(cg_debugWeaponAnim.integer)
			{
				CG_Printf("clamp weapon lf->frameTime\n");
			}
		}
	}

	if(lf->frameTime > cg.time + 200)
	{
		lf->frameTime = cg.time;
	}

	if(lf->oldFrameTime > cg.time)
	{
		lf->oldFrameTime = cg.time;
	}

	// calculate current lerp value
	if(lf->frameTime == lf->oldFrameTime)
	{
		lf->backlerp = 0;
	}
	else
	{
		lf->backlerp = 1.0 - (float)(cg.time - lf->oldFrameTime) / (lf->frameTime - lf->oldFrameTime);
	}

	// blend old and current animation
	if(cg_animBlend.value <= 0.0f)
	{
		lf->blendlerp = 0.0f;
	}

	if((lf->blendlerp > 0.0f) && (cg.time > lf->blendtime))
	{
#if 0
		// linear blending
		lf->blendlerp -= 0.025f;
#else
		// exp blending
		lf->blendlerp -= lf->blendlerp / cg_animBlend.value;
#endif
		if(lf->blendlerp <= 0.0f)
		{
			lf->blendlerp = 0.0f;
		}

		if(lf->blendlerp >= 1.0f)
		{
			lf->blendlerp = 1.0f;
		}

		lf->blendtime = cg.time + 10;
	}

	if(!trap_R_BuildSkeleton(&lf->skeleton, lf->animation->handle, lf->oldFrame, lf->frame, 1.0 - lf->backlerp, lf->animation->clearOrigin))
	{
		CG_Printf("CG_RunWeaponLerpFrame: Can't build lf->skeleton\n");
	}

	// lerp between old and new animation if possible
	if(lf->blendlerp > 0.0f)
	{
		if(!trap_R_BlendSkeleton(&lf->skeleton, &lf->oldSkeleton, lf->blendlerp))
		{
			CG_Printf("CG_RunWeaponLerpFrame: Can't blend lf->skeleton\n");
			return;
		}
	}
}

/*
=================
CG_WeaponAnimation
=================
*/
static void CG_WeaponAnimation(centity_t * cent, weaponInfo_t * weapon, int weaponNumber, int weaponState, int weaponTime)
{
	clientInfo_t   *ci;
	int             clientNum;
	float           speedScale;

	clientNum = cent->currentState.clientNum;

	if(cent->currentState.powerups & (1 << PW_HASTE))
	{
		speedScale = 1.5;
	}
	else
	{
		speedScale = 1;
	}

	ci = &cgs.clientinfo[clientNum];

	// change weapon animation
	CG_RunWeaponLerpFrame(weapon, &cent->pe.gun, weaponNumber, weaponState, weaponTime, speedScale);
}

/*
==============
CG_CalculateWeaponPosition
==============
*/
static void CG_CalculateWeaponPosition(vec3_t origin, vec3_t angles)
{
	float           scale;
	int             delta;
	float           fracsin;

	VectorCopy(cg.refdef.vieworg, origin);
	VectorCopy(cg.refdefViewAngles, angles);

	// on odd legs, invert some angles
	if(cg.bobcycle & 1)
	{
		scale = -cg.xyspeed;
	}
	else
	{
		scale = cg.xyspeed;
	}

	// gun angles from bobbing
	angles[ROLL] += scale * cg.bobfracsin * 0.005;
	angles[YAW] += scale * cg.bobfracsin * 0.01;
	angles[PITCH] += cg.xyspeed * cg.bobfracsin * 0.005;

	// drop the weapon when landing
	delta = cg.time - cg.landTime;
	if(delta < LAND_DEFLECT_TIME)
	{
		origin[2] += cg.landChange * 0.25 * delta / LAND_DEFLECT_TIME;
	}
	else if(delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME)
	{
		origin[2] += cg.landChange * 0.25 * (LAND_DEFLECT_TIME + LAND_RETURN_TIME - delta) / LAND_RETURN_TIME;
	}

#if 0
	// drop the weapon when stair climbing
	delta = cg.time - cg.stepTime;
	if(delta < STEP_TIME / 2)
	{
		origin[2] -= cg.stepChange * 0.25 * delta / (STEP_TIME / 2);
	}
	else if(delta < STEP_TIME)
	{
		origin[2] -= cg.stepChange * 0.25 * (STEP_TIME - delta) / (STEP_TIME / 2);
	}
#endif

	// idle drift
	scale = cg.xyspeed + 40;
	fracsin = sin(cg.time * 0.001);
	angles[ROLL] += scale * fracsin * 0.01;
	angles[YAW] += scale * fracsin * 0.01;
	angles[PITCH] += scale * fracsin * 0.01;
}


/*
===============
JUHOX: CG_CurvedLine
===============
*/
/*static void CG_CurvedLine(const vec3_t start, const vec3_t end, const vec3_t startDir,
						  qhandle_t shader, float segmentLen, float scrollSpeed)
{
	float           dist;
	vec3_t          dir1;
	vec3_t          dir2;
	int             n;
	float           totalLength;
	vec3_t          currentPos;
	int             i;

	VectorSubtract(end, start, dir2);
	dist = VectorLength(dir2);
	VectorScale(startDir, dist, dir1);
	n = dist / 20;
	if(n <= 0)
		n = 1;
	dist /= n;					// segment length

	totalLength = 0;
	VectorCopy(start, currentPos);
	for(i = 0; i < n; i++)
	{
		float           x;
		vec3_t          p1, p2;
		vec3_t          nextPos;

		x = (i + 1.0f) / (float)n;
		VectorMA(start, x, dir1, p1);
		VectorMA(start, x, dir2, p2);
		VectorSubtract(p2, p1, p2);
		VectorMA(p1, x * x * x, p2, nextPos);

		totalLength = CG_DrawLineSegment(currentPos, nextPos, totalLength, segmentLen, scrollSpeed, shader);

		VectorCopy(nextPos, currentPos);
	}
}*/

/*
===============
JUHOX: CG_LightningBolt (new version)
===============
*/
static void CG_LightningBolt(centity_t * cent, vec3_t origin)
{
	trace_t         trace;
	refEntity_t     beam;
	vec3_t          forward, right, up;
	vec3_t          muzzlePoint, endPoint;
	vec3_t          surfNormal;
	int             anim;

	memset(&beam, 0, sizeof(beam));

	// CPMA  "true" lightning
#if 0
	if((cent->currentState.number == cg.predictedPlayerState.clientNum) && (cg_trueLightning.value != 0))
	{
		vec3_t          angle;
		int             i;

		for(i = 0; i < 3; i++)
		{
			float           a = cent->lerpAngles[i] - cg.refdefViewAngles[i];

			if(a > 180)
			{
				a -= 360;
			}
			if(a < -180)
			{
				a += 360;
			}

			angle[i] = cg.refdefViewAngles[i] + a * (1.0 - cg_trueLightning.value);
			if(angle[i] < 0)
			{
				angle[i] += 360;
			}
			if(angle[i] > 360)
			{
				angle[i] -= 360;
			}
		}

		AngleVectors(angle, forward, NULL, NULL);
		VectorCopy(cent->lerpOrigin, muzzlePoint);
//      VectorCopy(cg.refdef.vieworg, muzzlePoint );
	}
	else
#endif
	{
		if(cent->currentState.eFlags & EF_WALLCLIMB)
		{
			if(cent->currentState.eFlags & EF_WALLCLIMBCEILING)
			{
				VectorSet(surfNormal, 0.0f, 0.0f, -1.0f);
			}
			else
			{
				VectorCopy(cent->currentState.angles2, surfNormal);
			}
		}
		else
		{
			VectorSet(surfNormal, 0.0f, 0.0f, 1.0f);
		}

		// !CPMA
		AngleVectors(cent->lerpAngles, forward, right, up);
		VectorCopy(cent->lerpOrigin, muzzlePoint);
	}

	anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
	if(anim == LEGS_WALKCR || anim == LEGS_IDLECR)
	{
		VectorMA(muzzlePoint, CROUCH_VIEWHEIGHT, surfNormal, muzzlePoint);
	}
	else
	{
		VectorMA(muzzlePoint, DEFAULT_VIEWHEIGHT, surfNormal, muzzlePoint);
	}

	VectorMA(muzzlePoint, 14, forward, muzzlePoint);

	// project forward by the lightning range
	VectorMA(muzzlePoint, LIGHTNING_RANGE, forward, endPoint);

	// see if it hit a wall
	CG_Trace(&trace, muzzlePoint, vec3_origin, vec3_origin, endPoint, cent->currentState.number, MASK_SHOT);

#if 1
	// this is the endpoint
	VectorCopy(trace.endpos, beam.oldorigin);

	// use the provided origin, even though it may be slightly
	// different than the muzzle origin
	VectorCopy(origin, beam.origin);

	beam.reType = RT_LIGHTNING;
	beam.customShader = cgs.media.lightningShader;
	trap_R_AddRefEntityToScene(&beam);

#else
	// Tr3B: new lightning curves
	if(trace.fraction != 1.0f)
	{
		/*
		VectorInverse(forward);

		angle = AngleBetweenVectors(trace.plane.normal, forward);
		if(angle > 45.0f)
		{
			LerpVect
		}
		*/

		// collided with a surface so calculate the lightning curve backwards to the player
		CG_CurvedLine(trace.endpos, origin, trace.plane.normal, cgs.media.lightningShader, 256.0, -2.0);
	}
	else
	{
#if 0
		matrix_t        rot;

		// we did hit anything so let the lightning bolt play crazy
		randomAngles[PITCH] = crandom() * 10;
		randomAngles[YAW] = crandom() * 1;
		randomAngles[ROLL] = crandom() * 5;

		MatrixFromAngles(rot, randomAngles[PITCH], randomAngles[YAW], randomAngles[ROLL]);
		MatrixTransformNormal2(rot, forward);
#endif
		CG_CurvedLine(origin, trace.endpos, forward, cgs.media.lightningShader, 256.0, -2.0);
	}
#endif
}

/*
===============
CG_SpawnRailTrail

Origin will be the exact tag point, which is slightly
different than the muzzle point used for determining hits.
===============
*/
static void CG_SpawnRailTrail(centity_t * cent, vec3_t origin)
{
	clientInfo_t   *ci;

	if(!cent->pe.railgunFlash)
	{
		return;
	}
	cent->pe.railgunFlash = qtrue;
	ci = &cgs.clientinfo[cent->currentState.clientNum];
	CG_RailTrail(ci, origin, cent->pe.railgunImpact);
}


/*
======================
CG_MachinegunSpinAngle
======================
*/
#define		SPIN_SPEED	0.9
#define		COAST_TIME	1000
static float CG_MachinegunSpinAngle(centity_t * cent)
{
	int             delta;
	float           angle;
	float           speed;

	delta = cg.time - cent->pe.barrelTime;
	if(cent->pe.barrelSpinning)
	{
		angle = cent->pe.barrelAngle + delta * SPIN_SPEED;
	}
	else
	{
		if(delta > COAST_TIME)
		{
			delta = COAST_TIME;
		}

		speed = 0.5 * (SPIN_SPEED + (float)(COAST_TIME - delta) / COAST_TIME);
		angle = cent->pe.barrelAngle + delta * speed;
	}

	if(cent->pe.barrelSpinning == !(cent->currentState.eFlags & EF_FIRING))
	{
		cent->pe.barrelTime = cg.time;
		cent->pe.barrelAngle = AngleNormalize360(angle);
		cent->pe.barrelSpinning = !!(cent->currentState.eFlags & EF_FIRING);
#ifdef MISSIONPACK
		if(cent->currentState.weapon == WP_CHAINGUN && !cent->pe.barrelSpinning)
		{
			trap_S_StartSound(NULL, cent->currentState.number, CHAN_WEAPON,
							  trap_S_RegisterSound("sound/weapons/vulcan/wvulwind.wav", qfalse));
		}
#endif
	}

	return angle;
}


/*
========================
CG_AddWeaponWithPowerups
========================
*/
static void CG_AddWeaponWithPowerups(refEntity_t * gun, int powerups)
{
#if 0
	// add powerup effects
	if(powerups & (1 << PW_INVIS))
	{
		gun->customShader = cgs.media.invisShader;
		trap_R_AddRefEntityToScene(gun);
	}
	else
	{
		trap_R_AddRefEntityToScene(gun);

		if(powerups & (1 << PW_BATTLESUIT))
		{
			gun->customShader = cgs.media.battleWeaponShader;
			trap_R_AddRefEntityToScene(gun);
		}
		if(powerups & (1 << PW_QUAD))
		{
			gun->customShader = cgs.media.quadWeaponShader;
			trap_R_AddRefEntityToScene(gun);
		}
	}
#else
	trap_R_AddRefEntityToScene(gun);
#endif
}

/*
=============
CG_PositionEntityOnWeaponTag

Positions a refEntity on the player's weapon tag (i.e. in their hands).
=============
*/
void CG_PositionEntityOnWeaponTag(refEntity_t *gun, const refEntity_t * parent, qhandle_t parentModel, playerState_t * ps, centity_t * cent)
{
#ifdef XPPM
	int             boneIndex;

	if(ps)
	{
		CG_PositionEntityOnTag(gun, parent, parentModel, "hand.R");
	}
	else
	{
		do
		{
			boneIndex = trap_R_BoneIndex(parentModel, "hand.R");
			if(boneIndex >= 0 && boneIndex < cent->pe.torso.skeleton.numBones)
			{
				AxisClear(gun->axis);
				CG_PositionRotatedEntityOnBone(gun, parent, parentModel, "hand.R");
				break;
			}
			boneIndex = trap_R_BoneIndex(parentModel, "hand.R");
			if(boneIndex >= 0 && boneIndex < cent->pe.torso.skeleton.numBones)
			{
				// HACK: this is bone specific
				vec3_t          angles;

				/*
				angles[PITCH] = -90;
				angles[YAW] = 0;
				angles[ROLL] = -90;
				*/

				AnglesToAxis(angles, gun->axis);

				CG_PositionRotatedEntityOnBone(gun, parent, parentModel, "hand.R");
				break;
			}
		} while(0);
	}
#else
	CG_PositionEntityOnTag(gun, parent, parent->hModel, "tag_weapon");
#endif
}


/*
=============
CG_AddPlayerWeapon

Used for both the view weapon (ps is valid) and the world model other character models (ps is NULL)
The main player will have this called for BOTH cases, so effects like light and
sound should only be done on the world model case.
=============
*/
void CG_AddPlayerWeapon(refEntity_t * parent, playerState_t * ps, centity_t * cent, int team)
{
	refEntity_t     gun;
	refEntity_t     barrel;
	refEntity_t     flash;
	vec3_t          angles;
	weapon_t        weaponNum;
	weaponInfo_t   *weapon;
	centity_t      *nonPredictedCent;

	weaponNum = cent->currentState.weapon;

	CG_RegisterWeapon(weaponNum);
	weapon = &cg_weapons[weaponNum];

	// add the weapon
	memset(&gun, 0, sizeof(gun));
	VectorCopy(parent->lightingOrigin, gun.lightingOrigin);
	gun.shadowPlane = parent->shadowPlane;
	gun.renderfx = parent->renderfx;
	gun.noShadowID = parent->noShadowID;

	// set custom shading for railgun refire rate
	if(ps)
	{
		//TODO: predicted weapon attacks
		/*
		if(cg.predictedPlayerState.weapon == WP_RAILGUN && cg.predictedPlayerState.weaponstate == WEAPON_FIRING)
		{
			float           f;

			f = (float)cg.predictedPlayerState.weaponTime / 1500;
			gun.shaderRGBA[1] = 0;
			gun.shaderRGBA[0] = gun.shaderRGBA[2] = 255 * (1.0 - f);
		}
		else
		{
			gun.shaderRGBA[0] = 255;
			gun.shaderRGBA[1] = 255;
			gun.shaderRGBA[2] = 255;
			gun.shaderRGBA[3] = 255;
		}
		*/
	}

	gun.hModel = weapon->weaponModel;

	if(weaponNum == WP_GAUNTLET)
	{
		gun.hModel = cgs.media.swordModel;
	}

	if(!gun.hModel)
	{
		return;
	}

	if(!ps)
	{
		// add weapon ready sound
		cent->pe.lightningFiring = qfalse;
		if((cent->currentState.eFlags & (EF_FIRING | EF_FIRING2)) && weapon->firingSound)
		{
			// lightning gun and guantlet make a different sound when fire is held down
			trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->firingSound);
			cent->pe.lightningFiring = qtrue;
		}
		else if(weapon->readySound)
		{
			trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->readySound);
		}
	}

	CG_PositionEntityOnWeaponTag(&gun, parent, parent->hModel, ps, cent);

	CG_AddWeaponWithPowerups(&gun, cent->currentState.powerups);

	// make sure we aren't looking at cg.predictedPlayerEntity for LG
	nonPredictedCent = &cg_entities[cent->currentState.clientNum];

	// if the index of the nonPredictedCent is not the same as the clientNum
	// then this is a fake player (like on teh single player podiums), so
	// go ahead and use the cent
	if((nonPredictedCent - cg_entities) != cent->currentState.clientNum)
	{
		nonPredictedCent = cent;
	}

	// add the spinning barrel
	if(weapon->barrelModel)
	{
		if(weaponNum != WP_GAUNTLET || (weaponNum == WP_GAUNTLET && !(nonPredictedCent->currentState.eFlags & EF_FIRING2)))
		{
			memset(&barrel, 0, sizeof(barrel));
			VectorCopy(parent->lightingOrigin, barrel.lightingOrigin);
			barrel.shadowPlane = parent->shadowPlane;
			barrel.renderfx = parent->renderfx;
			barrel.noShadowID = parent->noShadowID;

			barrel.hModel = weapon->barrelModel;
			angles[YAW] = 0;
			angles[PITCH] = 0;
			angles[ROLL] = CG_MachinegunSpinAngle(cent);
			AnglesToAxis(angles, barrel.axis);

			CG_PositionRotatedEntityOnTag(&barrel, &gun, weapon->weaponModel, "tag_barrel");

			CG_AddWeaponWithPowerups(&barrel, cent->currentState.powerups);
		}
	}

	// add the flash
	if((nonPredictedCent->currentState.eFlags & EF_FIRING))
	{
		// continuous flash
	}
	else
	{
		// impulse flash
		if(cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME && !cent->pe.railgunFlash)
		{
			return;
		}
	}

	memset(&flash, 0, sizeof(flash));
	VectorCopy(parent->lightingOrigin, flash.lightingOrigin);
	flash.shadowPlane = parent->shadowPlane;
	flash.renderfx = parent->renderfx;
	flash.noShadowID = parent->noShadowID;

	flash.hModel = weapon->flashModel;
	if(!flash.hModel)
	{
		return;
	}
	angles[YAW] = 0;
	angles[PITCH] = 0;
	angles[ROLL] = crandom() * 10;
	AnglesToAxis(angles, flash.axis);

	/*
	// colorize the railgun blast
	if(weaponNum == WP_RAILGUN)
	{
		clientInfo_t   *ci;

		ci = &cgs.clientinfo[cent->currentState.clientNum];
		flash.shaderRGBA[0] = 255 * ci->color1[0];
		flash.shaderRGBA[1] = 255 * ci->color1[1];
		flash.shaderRGBA[2] = 255 * ci->color1[2];
	}

	CG_PositionRotatedEntityOnTag(&flash, &gun, weapon->weaponModel, "tag_flash");
	trap_R_AddRefEntityToScene(&flash);

	if(ps || cg.renderingThirdPerson || cent->currentState.number != cg.predictedPlayerState.clientNum)
	{
		// add lightning bolt
		CG_LightningBolt(nonPredictedCent, flash.origin);

		// add rail trail
		CG_SpawnRailTrail(cent, flash.origin);

		if(weapon->flashLightColor[0] || weapon->flashLightColor[1] || weapon->flashLightColor[2])
		{
			trap_R_AddLightToScene(flash.origin, 300 + (rand() & 31), weapon->flashLightColor[0],
								   weapon->flashLightColor[1], weapon->flashLightColor[2]);
		}
	}
	*/
}

/*
==============
CG_AddViewWeapon

Add the weapon, and flash for the player's view
==============
*/
void CG_AddViewWeapon(playerState_t * ps)
{
	centity_t      *cent;
	clientInfo_t   *ci;
	float           fovOffset;
	vec3_t          angles;
	int             weaponNum;
	int             weaponState;
	int             weaponTime;
	weaponInfo_t   *weapon;

	if(ps->persistant[PERS_TEAM] == TEAM_SPECTATOR)
	{
		return;
	}

	if(ps->pm_type == PM_INTERMISSION)
	{
		return;
	}

	// no gun if in third person view or a camera is active
	//if ( cg.renderingThirdPerson || cg.cameraMode) {
	if(cg.renderingThirdPerson)
	{
		return;
	}


	// allow the gun to be completely removed
	if(!cg_drawGun.integer)
	{
		vec3_t          origin;

		if(cg.predictedPlayerState.eFlags & EF_FIRING)
		{
			// special hack for lightning gun...
			VectorCopy(cg.refdef.vieworg, origin);
			VectorMA(origin, -8, cg.refdef.viewaxis[2], origin);
			CG_LightningBolt(&cg_entities[ps->clientNum], origin);
		}
		return;
	}

	// don't draw if testing a gun model
	if(cg.testGun)
	{
		return;
	}

	// drop gun lower at higher fov
	if(cg_fov.integer > 90)
	{
		fovOffset = -0.2 * (cg_fov.integer - 90);
	}
	else
	{
		fovOffset = 0;
	}

	cent = &cg.predictedPlayerEntity;	// &cg_entities[cg.snap->ps.clientNum];

	weaponNum = ps->weapon;
	weaponState = ps->weaponstate;
	weaponTime = ps->weaponTime;
	CG_RegisterWeapon(weaponNum);
	weapon = &cg_weapons[weaponNum];

	if(weapon->viewModel && weapon->viewModel_animations[WEAPON_READY].handle)
	{
		refEntity_t     gun;
		vec3_t          angles;
		centity_t      *nonPredictedCent;
		int             boneIndex;
		vec3_t          flashOrigin;
		qboolean        addFlash;

		memset(&gun, 0, sizeof(gun));

		// set up gun position
		CG_CalculateWeaponPosition(gun.origin, angles);

		// HACK: tweak weapon positions
		switch (weaponNum)
		{
			case 0:
			{
				VectorMA(gun.origin, cg_gunX.value + 1, cg.refdef.viewaxis[0], gun.origin);
				VectorMA(gun.origin, cg_gunY.value - 2, cg.refdef.viewaxis[1], gun.origin);
				VectorMA(gun.origin, (cg_gunZ.value + 1 + fovOffset), cg.refdef.viewaxis[2], gun.origin);
				break;
			}

			default:
				VectorMA(gun.origin, cg_gunX.value, cg.refdef.viewaxis[0], gun.origin);
				VectorMA(gun.origin, cg_gunY.value, cg.refdef.viewaxis[1], gun.origin);
				VectorMA(gun.origin, (cg_gunZ.value + fovOffset), cg.refdef.viewaxis[2], gun.origin);
				break;
		}

		AnglesToAxis(angles, gun.axis);

		// get the animation state
		CG_WeaponAnimation(cent, weapon, weaponNum, weaponState, weaponTime);

		gun.hModel = weapon->viewModel;
		gun.renderfx = RF_DEPTHHACK | RF_FIRST_PERSON | RF_MINLIGHT;

		// set custom shading for railgun refire rate
		if(cg.predictedPlayerState.weaponstate == WEAPON_FIRING)
		{
			float           f;

			f = (float)cg.predictedPlayerState.weaponTime / 1500;
			gun.shaderRGBA[1] = 0;
			gun.shaderRGBA[0] = gun.shaderRGBA[2] = 255 * (1.0 - f);
		}
		else
		{
			gun.shaderRGBA[0] = 255;
			gun.shaderRGBA[1] = 255;
			gun.shaderRGBA[2] = 255;
			gun.shaderRGBA[3] = 255;
		}

		// copy legs skeleton to have a base
		memcpy(&gun.skeleton, &cent->pe.gun.skeleton, sizeof(refSkeleton_t));

		// transform relative bones to absolute ones required for vertex skinning
		CG_TransformSkeleton(&gun.skeleton, NULL);

		// make sure we aren't looking at cg.predictedPlayerEntity for LG
		nonPredictedCent = &cg_entities[cent->currentState.clientNum];

		// if the index of the nonPredictedCent is not the same as the clientNum
		// then this is a fake player (like on teh single player podiums), so
		// go ahead and use the cent
		if((nonPredictedCent - cg_entities) != cent->currentState.clientNum)
		{
			nonPredictedCent = cent;
		}

		addFlash = qfalse;

		// add the flash
		if((nonPredictedCent->currentState.eFlags & EF_FIRING))
		{
			// continuous flash
			//addFlash = qtrue;
		}
		else
		{
			// impulse flash
			if(cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME && !cent->pe.railgunFlash)
			{
				addFlash = qfalse;
			}
			else
			{
				addFlash = qtrue;
			}
		}

		// get flash origin
		if(addFlash)
		{
			gun.shaderTime = cg.time / 1000.0f;	//cent->pe.gun.frame;

			boneIndex = trap_R_BoneIndex(gun.hModel, "flash");

			if(boneIndex >= 0 && boneIndex < cent->pe.gun.skeleton.numBones)
			{
				matrix_t        modelToWorld;

				MatrixSetupTransformFromVectorsFLU(modelToWorld, gun.axis[0], gun.axis[1], gun.axis[2], gun.origin);
				MatrixTransformPoint(modelToWorld, gun.skeleton.bones[boneIndex].origin, flashOrigin);

				// add lightning bolt
				CG_LightningBolt(nonPredictedCent, flashOrigin);

				// add rail trail
				CG_SpawnRailTrail(cent, flashOrigin);

				// add light
				if(weapon->flashLightColor[0] || weapon->flashLightColor[1] || weapon->flashLightColor[2])
				{
					trap_R_AddLightToScene(flashOrigin, 300 + (rand() & 31), weapon->flashLightColor[0],
										   weapon->flashLightColor[1], weapon->flashLightColor[2]);
				}
			}
		}

		CG_AddWeaponWithPowerups(&gun, cent->currentState.powerups);
	}
	else
	{
		refEntity_t     hand;

		memset(&hand, 0, sizeof(hand));

		// set up gun position
		CG_CalculateWeaponPosition(hand.origin, angles);

		VectorMA(hand.origin, cg_gunX.value, cg.refdef.viewaxis[0], hand.origin);
		VectorMA(hand.origin, cg_gunY.value, cg.refdef.viewaxis[1], hand.origin);
		VectorMA(hand.origin, (cg_gunZ.value + fovOffset), cg.refdef.viewaxis[2], hand.origin);

		AnglesToAxis(angles, hand.axis);

		// map torso animations to weapon animations
		if(cg_gun_frame.integer)
		{
			// development tool
			hand.frame = hand.oldframe = cg_gun_frame.integer;
			hand.backlerp = 0;
		}
		else
		{
			// get clientinfo for animation map
			ci = &cgs.clientinfo[cent->currentState.clientNum];
			hand.frame = CG_MapTorsoToWeaponFrame(ci, cent->pe.torso.frame);
			hand.oldframe = CG_MapTorsoToWeaponFrame(ci, cent->pe.torso.oldFrame);
			hand.backlerp = cent->pe.torso.backlerp;
		}

		hand.hModel = weapon->handsModel;
		hand.renderfx = RF_DEPTHHACK | RF_FIRST_PERSON | RF_MINLIGHT;

		// add everything onto the hand
		CG_AddPlayerWeapon(&hand, ps, &cg.predictedPlayerEntity, ps->persistant[PERS_TEAM]);
	}
}

/*
==============================================================================

WEAPON SELECTION

==============================================================================
*/

/*
===================
CG_DrawWeaponSelectNew
===================
*/

#define HUD_ICONSIZE 28
#define HUD_ICONSIZESEL 8

#define HUD_ICONSPACE 4
#define HUD_X (320 - ( HUD_ICONSIZE / 2 ))

#define HUD_FADE_DIST 160

void CG_DrawWeaponSelectNew(void)
{
	int             i;
	int             bits;
	int             count;
	int             x, y;
	float          *color;
	vec4_t          fadecolor = { 1.0f, 1.0f, 1.0f, 1.0f };
	float           dist;
	int             diff = 1;
	int             weap = 0;


	color = CG_FadeColor(cg.weaponSelectTime, WEAPON_SELECT_TIME * 2);


	if(!color)
	{
		cg.bar_offset = 0;
		return;
	}

	cg.bar_offset = color[3] * color[3];

	trap_R_SetColor(color);


	// showing weapon select clears pickup item display, but not the blend blob
	cg.itemPickupTime = 0;

	// count the number of weapons owned
	bits = cg.snap->ps.stats[STAT_WEAPONS];
	count = 0;
	for(i = 1; i < 16; i++)
	{
		if(bits & (1 << i))
		{
			count++;
		}
	}
	cg.bar_count = count;

	y = 440;

	if(count <= 0)
		return;

	//draw current selection:
	CG_DrawPic(HUD_X - HUD_ICONSIZESEL / 2, y - HUD_ICONSIZESEL / 2, HUD_ICONSIZE + HUD_ICONSIZESEL,
			   HUD_ICONSIZE + HUD_ICONSIZESEL, cg_weapons[cg.weaponSelect].weaponIcon);
	//CG_DrawPic(x - HUD_ICONSPACE/2, y - HUD_ICONSPACE/2, HUD_ICONSIZE + HUD_ICONSPACE, HUD_ICONSIZE + HUD_ICONSPACE, cgs.media.weaponSelectShader);

	for(i = 0; i < 16; i++)
	{
		weap = cg.weaponSelect + i;

		if(!(bits & (1 << weap)))
		{
			continue;
		}

		CG_RegisterWeapon(weap);


		if(weap == cg.weaponSelect)
		{
			continue;

		}


		x = HUD_X + (HUD_ICONSIZE + HUD_ICONSPACE) * (diff++);

		dist = abs(x - HUD_X);
		if(dist > HUD_FADE_DIST)
			dist = HUD_FADE_DIST;

		fadecolor[3] = 1.0f - (dist / HUD_FADE_DIST);

		trap_R_SetColor(fadecolor);
		CG_DrawPic(x, y, HUD_ICONSIZE, HUD_ICONSIZE, cg_weapons[weap].weaponIcon);

		trap_R_SetColor(color);


		x = HUD_X + (HUD_ICONSIZE + HUD_ICONSPACE) * (diff - count - 1);

		dist = abs(x - HUD_X);
		if(dist > HUD_FADE_DIST)
			dist = HUD_FADE_DIST;

		fadecolor[3] = 1.0f - (dist / HUD_FADE_DIST);

		trap_R_SetColor(fadecolor);
		CG_DrawPic(x, y, HUD_ICONSIZE, HUD_ICONSIZE, cg_weapons[weap].weaponIcon);

		trap_R_SetColor(color);

	}

	diff = -1;
	weap = 0;

	for(i = 0; i < 16; i++)
	{
		weap = cg.weaponSelect - i;

		if(!(bits & (1 << weap)))
		{
			continue;
		}

		CG_RegisterWeapon(weap);

		if(weap == cg.weaponSelect)
		{
			continue;
		}

		x = HUD_X + (HUD_ICONSIZE + HUD_ICONSPACE) * (diff--);

		dist = abs(x - HUD_X);
		if(dist > HUD_FADE_DIST)
			dist = HUD_FADE_DIST;

		fadecolor[3] = 1.0f - (dist / HUD_FADE_DIST);

		trap_R_SetColor(fadecolor);
		CG_DrawPic(x, y, HUD_ICONSIZE, HUD_ICONSIZE, cg_weapons[weap].weaponIcon);

		trap_R_SetColor(color);

		x = HUD_X + (HUD_ICONSIZE + HUD_ICONSPACE) * (diff + count + 1);

		dist = abs(x - HUD_X);
		if(dist > HUD_FADE_DIST)
			dist = HUD_FADE_DIST;

		fadecolor[3] = 1.0f - (dist / HUD_FADE_DIST);

		trap_R_SetColor(fadecolor);
		CG_DrawPic(x, y, HUD_ICONSIZE, HUD_ICONSIZE, cg_weapons[weap].weaponIcon);

		trap_R_SetColor(color);

	}

/*
	diff = 0;

	for(i = cg.weaponSelect ; i > 0; i--)
	{
		if(!(bits & (1 << i)))
		{
			continue;
		}

		CG_RegisterWeapon(i);


		if(i == cg.weaponSelect){
			continue;

		}
		diff --;

		x = HUD_X + (HUD_ICONSIZE + HUD_ICONSPACE) * ( diff );
		if(x > HUD_X)
			CG_DrawPic(x, y, HUD_ICONSIZE, HUD_ICONSIZE, cg_weapons[i].weaponIcon);

	}
*/

	// draw the selected name
/*	if(cg_weapons[cg.weaponSelect].item)
	{
		name = cg_weapons[cg.weaponSelect].item->pickup_name;
		if(name)
		{
			w = CG_DrawStrlen(name) * BIGCHAR_WIDTH;
			x = (SCREEN_WIDTH - w) / 2;
			CG_DrawBigStringColor(x, y - 22, name, color);
		}
	}
*/
	trap_R_SetColor(NULL);
}


/*
===================
CG_DrawWeaponSelect
===================
*/
void CG_DrawWeaponSelect(void)
{
	int             i;
	int             bits;
	int             count;
	int             x, y, w;
	char           *name;
	float          *color;

	if(!cg_drawWeaponSelect.integer)
		return;


	// don't display if dead
	if(cg.predictedPlayerState.stats[STAT_HEALTH] <= 0)
	{
		return;
	}

	if(cg_drawStatus.integer == 3)
	{
		CG_DrawWeaponSelectNew();
		return;
	}

	color = CG_FadeColor(cg.weaponSelectTime, WEAPON_SELECT_TIME);
	if(!color)
	{
		return;
	}
	trap_R_SetColor(color);

	// showing weapon select clears pickup item display, but not the blend blob
	cg.itemPickupTime = 0;

	// count the number of weapons owned
	bits = cg.snap->ps.stats[STAT_WEAPONS];
	count = 0;
	for(i = 1; i < 16; i++)
	{
		if(bits & (1 << i))
		{
			count++;
		}
	}

	x = 320 - count * 20;
	y = 380;

	for(i = 1; i < 16; i++)
	{
		if(!(bits & (1 << i)))
		{
			continue;
		}

		CG_RegisterWeapon(i);

		// draw weapon icon
		CG_DrawPic(x, y, 32, 32, cg_weapons[i].weaponIcon);

		// draw selection marker
		if(i == cg.weaponSelect)
		{
			CG_DrawPic(x - 4, y - 4, 40, 40, cgs.media.weaponSelectShader);
		}

		// no ammo cross on top
		if(!cg.snap->ps.ammo[i])
		{
			CG_DrawPic(x, y, 32, 32, cgs.media.noammoShader);
		}

		x += 40;
	}

	// draw the selected name
	if(cg_weapons[cg.weaponSelect].item)
	{
		name = cg_weapons[cg.weaponSelect].item->pickup_name;
		if(name)
		{
			w = CG_DrawStrlen(name) * BIGCHAR_WIDTH;
			x = (SCREEN_WIDTH - w) / 2;
			CG_DrawBigStringColor(x, y - 22, name, color);
		}
	}

	trap_R_SetColor(NULL);
}


/*
===============
CG_WeaponSelectable
===============
*/
static qboolean CG_WeaponSelectable(int i)
{
	if(!cg.snap->ps.ammo[i])
	{
		return qfalse;
	}
	if(!(cg.snap->ps.stats[STAT_WEAPONS] & (1 << i)))
	{
		return qfalse;
	}

	return qtrue;
}

/*
===============
CG_NextWeapon_f
===============
*/
void CG_NextWeapon_f(void)
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

	for(i = 0; i < 16; i++)
	{
		cg.weaponSelect++;
		if(cg.weaponSelect == 16)
		{
			cg.weaponSelect = 0;
		}
		if(cg.weaponSelect == WP_GAUNTLET)
		{
			continue;           // never cycle to gauntlet
		}
		if(CG_WeaponSelectable(cg.weaponSelect))
		{
			break;
		}
	}
	if(i == 16)
	{
		cg.weaponSelect = original;
	}

}

/*
===============
CG_PrevWeapon_f
===============
*/
void CG_PrevWeapon_f(void)
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
		CG_OSDPrev_f();
		return;
	}

	if(cg.scoreBoardShowing)
	{
		cg.scoreboard_offset--;
		return;

	}
	cg.weaponSelectTime = cg.time;
	original = cg.weaponSelect;

	for(i = 0; i < 16; i++)
	{
		cg.weaponSelect--;
		if(cg.weaponSelect == -1)
		{
			cg.weaponSelect = 15;
		}
		if(cg.weaponSelect == WP_GAUNTLET)
		{
			continue;           // never cycle to gauntlet
		}
		if(CG_WeaponSelectable(cg.weaponSelect))
		{
			break;
		}
	}

	if(i == 16)
	{
		cg.weaponSelect = original;
	}
}

/*
===============
CG_Weapon_f
===============
*/
void CG_Weapon_f(void)
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

	if(num < 1 || num > 15)
	{
		return;
	}

	cg.weaponSelectTime = cg.time;

	if(!(cg.snap->ps.stats[STAT_WEAPONS] & (1 << num)))
	{
		return;					// don't have the weapon
	}

	if((cg.weaponSelect == WP_GAUNTLET) && (num == WP_GAUNTLET))
	{
		num = WP_NONE;
	}

	cg.weaponSelect = num;
}

/*
===================
CG_OutOfAmmoChange

The current weapon has just run out of ammo
===================
*/
void CG_OutOfAmmoChange(void)
{
	int             i;

	cg.weaponSelectTime = cg.time;

	for(i = 15; i > 0; i--)
	{
		if(CG_WeaponSelectable(i))
		{
			cg.weaponSelect = i;
			break;
		}
	}

	if(cg.weaponSelect == WP_GAUNTLET)
	{
		cg.weaponSelect = WP_NONE;
	}
}



/*
===================================================================================================

WEAPON EVENTS

===================================================================================================
*/

/*
================
CG_FireWeapon

Caused by an EV_FIRE_WEAPON event
================
*/
void CG_FireWeapon(centity_t * cent)
{
	entityState_t  *ent;
	int             c;
	weaponInfo_t   *weap;

	ent = &cent->currentState;
	if(ent->weapon == WP_NONE)
	{
		return;
	}
	if(ent->weapon >= WP_NUM_WEAPONS)
	{
		CG_Error("CG_FireWeapon: ent->weapon >= WP_NUM_WEAPONS");
		return;
	}
	weap = &cg_weapons[ent->weapon];

	// mark the entity as muzzle flashing, so when it is added it will
	// append the flash to the weapon model
	cent->muzzleFlashTime = cg.time;

	// play quad sound if needed
	if(cent->currentState.powerups & (1 << PW_QUAD))
	{
		trap_S_StartSound(NULL, cent->currentState.number, CHAN_ITEM, cgs.media.quadSound);
	}

	// play a sound
	for(c = 0; c < 10; c++)
	{
		if(!weap->flashSound[c])
		{
			break;
		}
	}
	if(c > 0)
	{
		c = rand() % c;
		if(weap->flashSound[c])
		{
			trap_S_StartSound(NULL, ent->number, CHAN_WEAPON, weap->flashSound[c]);
		}
	}

	// do brass ejection
	if(weap->ejectBrassFunc && cg_brassTime.integer > 0)
	{
		weap->ejectBrassFunc(cent);
	}
}

/*
================
CG_FireWeapon2

Caused by an EV_FIRE_WEAPON2 event

TODO
================
*/
void CG_FireWeapon2(centity_t * cent)
{
	entityState_t  *ent;
	int             c;
	weaponInfo_t   *weap;

	ent = &cent->currentState;
	if(ent->weapon == WP_NONE)
	{
		return;
	}
	if(ent->weapon >= WP_NUM_WEAPONS)
	{
		CG_Error("CG_FireWeapon: ent->weapon >= WP_NUM_WEAPONS");
		return;
	}
	weap = &cg_weapons[ent->weapon];

	/*
	// TODO
	switch (ent->weapon)
	{
		case WP_GAUNTLET:
		default:
			return;
	}
	*/

	// mark the entity as muzzle flashing, so when it is added it will
	// append the flash to the weapon model
	cent->muzzleFlashTime = cg.time;

	// lightning gun only does this this on initial press
	if(ent->weapon == WP_GAUNTLET)
	{
		if(cent->pe.lightningFiring)
		{
			return;
		}
	}

	// play quad sound if needed
	if(cent->currentState.powerups & (1 << PW_QUAD))
	{
		trap_S_StartSound(NULL, cent->currentState.number, CHAN_ITEM, cgs.media.quadSound);
	}

	// play a sound
	for(c = 0; c < 10; c++)
	{
		if(!weap->flashSound2[c])
		{
			break;
		}
	}
	if(c > 0)
	{
		c = rand() % c;
		if(weap->flashSound2[c])
		{
			trap_S_StartSound(NULL, ent->number, CHAN_WEAPON, weap->flashSound[c]);
		}
	}

	// do brass ejection
	if(weap->ejectBrassFunc2 && cg_brassTime.integer > 0)
	{
		weap->ejectBrassFunc2(cent);
	}
}

/*
=================
CG_AddBulletParticles
=================
*/
static void CG_AddBulletParticles(vec3_t origin, vec3_t dir, int speed, int duration, int count, float randScale)
{
	vec3_t          velocity, pos;
	int             i;

	// add the falling particles
	for(i = 0; i < count; i++)
	{
		VectorSet(velocity, dir[0] + crandom() * randScale, dir[1] + crandom() * randScale, dir[2] + crandom() * randScale);
		VectorScale(velocity, (float)speed, velocity);

		VectorCopy(origin, pos);
		VectorMA(pos, 2 + random() * 4, dir, pos);

		CG_ParticleBulletDebris(pos, velocity, 300 + rand() % 300);
	}
}

/*
=================
CG_AddSparks
=================
*/
static void CG_AddSparks(vec3_t origin, vec3_t dir, int speed, int duration, int count, float randScale)
{
	vec3_t          velocity, pos;
	int             i;

	// add the falling particles
	for(i = 0; i < count; i++)
	{
		VectorSet(velocity, dir[0] + crandom() * randScale, dir[1] + crandom() * randScale, dir[2] + crandom() * randScale);
		VectorScale(velocity, (float)speed, velocity);

		VectorCopy(origin, pos);
		VectorMA(pos, 2 + random() * 4, dir, pos);

		CG_ParticleSparks(pos, velocity, 300 + rand() % 300, 20, 30, 375);
	}
}

/*
=================
CG_MissileHitWall

Caused by an EV_MISSILE_MISS event, or directly by local bullet tracing
=================
*/
void CG_MissileHitWall(int weapon, int entityType, int clientNum, vec3_t origin, vec3_t dir, impactSound_t soundType)
{
	qhandle_t       mod;
	qhandle_t       mark;
	qhandle_t       shader;
	sfxHandle_t     sfx;
	float           radius;
	float           light;
	vec3_t          lightColor;
	localEntity_t  *le;
	int             r;
	qboolean        alphaFade;
	qboolean        isSprite;
	int             duration;
	vec3_t          partOrigin;
	vec3_t          partVel;

	mark = 0;
	radius = 32;
	sfx = 0;
	mod = 0;
	shader = 0;
	light = 0;
	lightColor[0] = 1;
	lightColor[1] = 1;
	lightColor[2] = 0;

	// set defaults
	isSprite = qfalse;
	duration = 600;
	VectorMA(origin, 12, dir, partOrigin);

	switch (weapon)
	{
		default:
		case WP_GAUNTLET:
			sfx = cgs.media.hookImpactSound;
			alphaFade = qtrue;
			break;
	}

	if(sfx)
	{
		trap_S_StartSound(origin, ENTITYNUM_WORLD, CHAN_AUTO, sfx);
	}

	// create the explosion
	if(mod || isSprite)
	{
		le = CG_MakeExplosion(origin, dir, mod, shader, duration, isSprite);
		le->light = light;
		VectorCopy(lightColor, le->lightColor);
	}

	// impact mark
	CG_ImpactMark(mark, origin, dir, random() * 360, 1, 1, 1, 1, alphaFade, radius, qfalse);
}


/*
=================
CG_MissileHitPlayer
=================
*/
void CG_MissileHitPlayer(int weapon, int entityType, vec3_t origin, vec3_t dir, int entityNum)
{
//	CG_Bleed(origin, entityNum);
//	CG_ParticleBlood(origin, dir, 3);

	// some weapons will make an explosion with the blood, while
	// others will just make the blood
	switch (weapon)
	{
		case WP_GAUNTLET:
			CG_MissileHitWall(weapon, entityType, 0, origin, dir, IMPACTSOUND_FLESH);
			break;
		default:
			break;
	}
}



/*
============================================================================

SHOTGUN TRACING

============================================================================
*/

/*
================
CG_ShotgunPellet
================
*/
static void CG_ShotgunPellet(vec3_t start, vec3_t end, int skipNum)
{
	trace_t         tr;
	int             sourceContentType, destContentType;

	CG_Trace(&tr, start, NULL, NULL, end, skipNum, MASK_SHOT);

	sourceContentType = trap_CM_PointContents(start, 0);
	destContentType = trap_CM_PointContents(tr.endpos, 0);

	// FIXME: should probably move this cruft into CG_BubbleTrail
	if(sourceContentType == destContentType)
	{
		if(sourceContentType & CONTENTS_WATER)
		{
			CG_BubbleTrail(start, tr.endpos, 32);
		}
	}
	else if(sourceContentType & CONTENTS_WATER)
	{
		trace_t         trace;

		trap_CM_BoxTrace(&trace, end, start, NULL, NULL, 0, CONTENTS_WATER);
		CG_BubbleTrail(start, trace.endpos, 32);
	}
	else if(destContentType & CONTENTS_WATER)
	{
		trace_t         trace;

		trap_CM_BoxTrace(&trace, start, end, NULL, NULL, 0, CONTENTS_WATER);
		CG_BubbleTrail(tr.endpos, trace.endpos, 32);
	}

	if(tr.surfaceFlags & SURF_NOIMPACT)
	{
		return;
	}

	if(cg_entities[tr.entityNum].currentState.eType == ET_PLAYER)
	{
		CG_MissileHitPlayer(0, ET_GENERAL, tr.endpos, tr.plane.normal, tr.entityNum);
	}
	else
	{
		if(tr.surfaceFlags & SURF_NOIMPACT)
		{
			// SURF_NOIMPACT will not make a flame puff or a mark
			return;
		}
		if(tr.surfaceFlags & SURF_METALSTEPS)
		{
			CG_MissileHitWall(0, ET_GENERAL, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_METAL);
		}
		else
		{
			CG_MissileHitWall(0, ET_GENERAL, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_DEFAULT);
		}
	}
}

/*
================
CG_ShotgunPattern

Perform the same traces the server did to locate the
hit splashes
================
*/
static void CG_ShotgunPattern(vec3_t origin, vec3_t origin2, int seed, int otherEntNum)
{
	int             i;
	float           r, u;
	vec3_t          end;
	vec3_t          forward, right, up;

	// derive the right and up vectors from the forward vector, because
	// the client won't have any other information
	VectorNormalize2(origin2, forward);
	PerpendicularVector(right, forward);
	CrossProduct(forward, right, up);

	// generate the "random" spread pattern
	for(i = 0; i < DEFAULT_SHOTGUN_COUNT; i++)
	{
		r = Q_crandom(&seed) * DEFAULT_SHOTGUN_SPREAD * 16;
		u = Q_crandom(&seed) * DEFAULT_SHOTGUN_SPREAD * 16;
		VectorMA(origin, 8192 * 16, forward, end);
		VectorMA(end, r, right, end);
		VectorMA(end, u, up, end);

		CG_ShotgunPellet(origin, end, otherEntNum);
	}
}

/*
==============
CG_ShotgunFire
==============
*/
void CG_ShotgunFire(entityState_t * es)
{
	vec3_t          v;
	vec3_t          up;
	int             contents;

	VectorSubtract(es->origin2, es->pos.trBase, v);
	VectorNormalize(v);
	VectorScale(v, 32, v);
	VectorAdd(es->pos.trBase, v, v);

	contents = trap_CM_PointContents(es->pos.trBase, 0);
	if(!(contents & CONTENTS_WATER))
	{
		VectorSet(up, 0, 0, 8);
		CG_SmokePuff(v, up, 32, 1, 1, 1, 0.33f, 900, cg.time, 0, LEF_PUFF_DONT_SCALE, cgs.media.shotgunSmokePuffShader);
	}
	CG_ShotgunPattern(es->pos.trBase, es->origin2, es->eventParm, es->otherEntityNum);
}

/*
============================================================================

BULLETS

============================================================================
*/


/*
===============
CG_Tracer
===============
*/
void CG_Tracer(vec3_t source, vec3_t dest)
{
	vec3_t          forward, right;
	polyVert_t      verts[4];
	vec3_t          line;
	float           len, begin, end;
	vec3_t          start, finish;
	vec3_t          midpoint;

	// tracer
	VectorSubtract(dest, source, forward);
	len = VectorNormalize(forward);

	// start at least a little ways from the muzzle
	if(len < 100)
	{
		return;
	}
	begin = 50 + random() * (len - 60);
	end = begin + cg_tracerLength.value;
	if(end > len)
	{
		end = len;
	}
	VectorMA(source, begin, forward, start);
	VectorMA(source, end, forward, finish);

	line[0] = DotProduct(forward, cg.refdef.viewaxis[1]);
	line[1] = DotProduct(forward, cg.refdef.viewaxis[2]);

	VectorScale(cg.refdef.viewaxis[1], line[1], right);
	VectorMA(right, -line[0], cg.refdef.viewaxis[2], right);
	VectorNormalize(right);

	VectorMA(finish, cg_tracerWidth.value, right, verts[0].xyz);
	verts[0].st[0] = 0;
	verts[0].st[1] = 1;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[2] = 255;
	verts[0].modulate[3] = 255;

	VectorMA(finish, -cg_tracerWidth.value, right, verts[1].xyz);
	verts[1].st[0] = 1;
	verts[1].st[1] = 0;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[2] = 255;
	verts[1].modulate[3] = 255;

	VectorMA(start, -cg_tracerWidth.value, right, verts[2].xyz);
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[2] = 255;
	verts[2].modulate[3] = 255;

	VectorMA(start, cg_tracerWidth.value, right, verts[3].xyz);
	verts[3].st[0] = 0;
	verts[3].st[1] = 0;
	verts[3].modulate[0] = 255;
	verts[3].modulate[1] = 255;
	verts[3].modulate[2] = 255;
	verts[3].modulate[3] = 255;

	trap_R_AddPolyToScene(cgs.media.tracerShader, 4, verts);

	midpoint[0] = (start[0] + finish[0]) * 0.5;
	midpoint[1] = (start[1] + finish[1]) * 0.5;
	midpoint[2] = (start[2] + finish[2]) * 0.5;

	// add the tracer sound
	trap_S_StartSound(midpoint, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.tracerSound);

}


/*
======================
CG_CalcMuzzlePoint
======================
*/
static qboolean CG_CalcMuzzlePoint(int entityNum, vec3_t muzzle)
{
	vec3_t          forward;
	centity_t      *cent;
	int             anim;
	vec3_t          surfNormal;

	// Tr3B: changed this for wallwalking
	if(entityNum == cg.snap->ps.clientNum)
	{
		if(cg.snap->ps.pm_flags & PMF_WALLCLIMBING)
		{
			if(cg.snap->ps.pm_flags & PMF_WALLCLIMBINGCEILING)
				VectorSet(surfNormal, 0.0f, 0.0f, -1.0f);
			else
				VectorCopy(cg.snap->ps.grapplePoint, surfNormal);
		}
		else
		{
			VectorSet(surfNormal, 0.0f, 0.0f, 1.0f);
		}

		VectorMA(cg.snap->ps.origin, cg.snap->ps.viewheight, surfNormal, muzzle);

		//VectorCopy(cg.snap->ps.origin, muzzle);
		//muzzle[2] += cg.snap->ps.viewheight;

		AngleVectors(cg.snap->ps.viewangles, forward, NULL, NULL);
		VectorMA(muzzle, 14, forward, muzzle);
		return qtrue;
	}

	cent = &cg_entities[entityNum];
	if(!cent->currentValid)
	{
		return qfalse;
	}

	VectorCopy(cent->currentState.pos.trBase, muzzle);

	if(cent->currentState.eFlags & EF_WALLCLIMB)
	{
		if(cent->currentState.eFlags & EF_WALLCLIMBCEILING)
			VectorSet(surfNormal, 0.0f, 0.0f, -1.0f);
		else
			VectorCopy(cent->currentState.angles2, surfNormal);
	}
	else
	{
		VectorSet(surfNormal, 0.0f, 0.0f, 1.0f);
	}

	//VectorMA(cent->currentState.pos.trBase, cg.snap->ps.viewheight, surfNormal, muzzle);

	AngleVectors(cent->currentState.apos.trBase, forward, NULL, NULL);
	anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
	if(anim == LEGS_WALKCR || anim == LEGS_IDLECR)
	{
		//muzzle[2] += CROUCH_VIEWHEIGHT;
		VectorMA(muzzle, CROUCH_VIEWHEIGHT, surfNormal, muzzle);
	}
	else
	{
		//muzzle[2] += DEFAULT_VIEWHEIGHT;
		VectorMA(muzzle, DEFAULT_VIEWHEIGHT, surfNormal, muzzle);
	}

	VectorMA(muzzle, 14, forward, muzzle);

	return qtrue;

}

/*
======================
CG_Bullet

Renders bullet effects.
======================
*/
void CG_Bullet(vec3_t end, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum)
{
	trace_t         trace;
	int             sourceContentType, destContentType;
	vec3_t          start;

	// if the shooter is currently valid, calc a source point and possibly
	// do trail effects
	if(sourceEntityNum >= 0 && cg_tracerChance.value > 0)
	{
		if(CG_CalcMuzzlePoint(sourceEntityNum, start))
		{
			sourceContentType = trap_CM_PointContents(start, 0);
			destContentType = trap_CM_PointContents(end, 0);

			// do a complete bubble trail if necessary
			if((sourceContentType == destContentType) && (sourceContentType & CONTENTS_WATER))
			{
				CG_BubbleTrail(start, end, 32);
			}
			// bubble trail from water into air
			else if((sourceContentType & CONTENTS_WATER))
			{
				trap_CM_BoxTrace(&trace, end, start, NULL, NULL, 0, CONTENTS_WATER);
				CG_BubbleTrail(start, trace.endpos, 32);
			}
			// bubble trail from air into water
			else if((destContentType & CONTENTS_WATER))
			{
				trap_CM_BoxTrace(&trace, start, end, NULL, NULL, 0, CONTENTS_WATER);
				CG_BubbleTrail(trace.endpos, end, 32);
			}

			// draw a tracer
			if(random() < cg_tracerChance.value)
			{
				CG_Tracer(start, end);
			}
		}
	}

	// impact splash and mark
	if(flesh)
	{
		//CG_Bleed(end, fleshEntityNum);  OLD
		//CG_ParticleBlood(end, trace.plane.normal, 3);
	}
	else
	{
		CG_MissileHitWall(0, ET_GENERAL, 0, end, normal, IMPACTSOUND_DEFAULT);
	}

}
