/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>
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
// cg_effects.c -- these functions generate localentities, usually as a result
// of event processing

#include "cg_local.h"

vec3_t          zeroVector = { 0, 0, 0 };

/*
==================
CG_BubbleTrail

Bullets shot underwater
==================
*/
void CG_BubbleTrail(vec3_t start, vec3_t end, float spacing)
{
	vec3_t          move;
	vec3_t          vec;
	float           len;
	int             i;

	if(cg_noProjectileTrail.integer)
	{
		return;
	}

	VectorCopy(start, move);
	VectorSubtract(end, start, vec);
	len = VectorNormalize(vec);

	// advance a random amount first
	i = rand() % (int)spacing;
	VectorMA(move, i, vec, move);

	VectorScale(vec, spacing, vec);

	for(; i < len; i += spacing)
	{
		localEntity_t  *le;
		refEntity_t    *re;

		le = CG_AllocLocalEntity();
		le->leFlags = LEF_PUFF_DONT_SCALE;
		le->leType = LE_MOVE_SCALE_FADE;
		le->startTime = cg.time;
		le->endTime = cg.time + 1000 + random() * 250;
		le->lifeRate = 1.0 / (le->endTime - le->startTime);

		re = &le->refEntity;
		re->shaderTime = -cg.time / 1000.0f;

		re->reType = RT_SPRITE;
		re->rotation = 0;
		re->radius = 3;
		re->customShader = cgs.media.waterBubbleShader;
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0xff;
		re->shaderRGBA[2] = 0xff;
		re->shaderRGBA[3] = 0xff;

		le->color[3] = 1.0;

		le->pos.trType = TR_LINEAR;
		le->pos.trTime = cg.time;
		VectorCopy(move, le->pos.trBase);
		le->pos.trDelta[0] = crandom() * 5;
		le->pos.trDelta[1] = crandom() * 5;
		le->pos.trDelta[2] = crandom() * 5 + 6;

		VectorAdd(move, vec, move);
	}
}

/*
=====================
CG_SmokePuff

Adds a smoke puff or blood trail localEntity.
=====================
*/
localEntity_t  *CG_SmokePuff(const vec3_t p, const vec3_t vel,
							 float radius,
							 float r, float g, float b, float a,
							 float duration, int startTime, int fadeInTime, int leFlags, qhandle_t hShader)
{
	static int      seed = 0x92;
	localEntity_t  *le;
	refEntity_t    *re;

//  int fadeInTime = startTime + duration / 2;

	le = CG_AllocLocalEntity();
	le->leFlags = leFlags;
	le->radius = radius;

	re = &le->refEntity;
	re->rotation = Q_random(&seed) * 360;
	re->radius = radius;
	re->shaderTime = -startTime / 1000.0f;

	le->leType = LE_MOVE_SCALE_FADE;
	le->startTime = startTime;
	le->fadeInTime = fadeInTime;
	le->endTime = startTime + duration;
	if(fadeInTime > startTime)
	{
		le->lifeRate = 1.0 / (le->endTime - le->fadeInTime);
	}
	else
	{
		le->lifeRate = 1.0 / (le->endTime - le->startTime);
	}
	le->color[0] = r;
	le->color[1] = g;
	le->color[2] = b;
	le->color[3] = a;


	le->pos.trType = TR_LINEAR;
	le->pos.trTime = startTime;
	VectorCopy(vel, le->pos.trDelta);
	VectorCopy(p, le->pos.trBase);

	VectorCopy(p, re->origin);
	re->customShader = hShader;

	re->shaderRGBA[0] = le->color[0] * 0xff;
	re->shaderRGBA[1] = le->color[1] * 0xff;
	re->shaderRGBA[2] = le->color[2] * 0xff;
	re->shaderRGBA[3] = 0xff;

	re->reType = RT_SPRITE;
	re->radius = le->radius;

	return le;
}

/*
===============
CG_LightningBoltBeam
===============
*/
void CG_LightningBoltBeam(vec3_t start, vec3_t end)
{
	localEntity_t  *le;
	refEntity_t    *beam;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_SHOWREFENTITY;
	le->startTime = cg.time;
	le->endTime = cg.time + 50;

	beam = &le->refEntity;

	VectorCopy(start, beam->origin);
	VectorCopy(end, beam->oldorigin);

	beam->reType = RT_LIGHTNING;
	beam->customShader = cgs.media.lightningShader;
}

/*
==================
CG_KamikazeEffect
==================
*/
void CG_KamikazeEffect(vec3_t org)
{
	localEntity_t  *le;
	refEntity_t    *re;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_KAMIKAZE;
	le->startTime = cg.time;
	le->endTime = cg.time + 3000;	//2250;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);

	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;

	VectorClear(le->angles.trBase);

	re = &le->refEntity;

	re->reType = RT_MODEL;
	re->shaderTime = -cg.time / 1000.0f;

	re->hModel = cgs.media.kamikazeEffectModel;

	VectorCopy(org, re->origin);
}


/*
==================
CG_RailExplode
==================
*/
void CG_RailExplode(vec3_t org)
{
	localEntity_t  *le;
	refEntity_t    *re;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_RAILEXPLOSION;
	le->startTime = cg.time;
	le->endTime = cg.time + 3000;	//2250;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);

	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;

	VectorClear(le->angles.trBase);

	re = &le->refEntity;

	re->reType = RT_MODEL;
	re->shaderTime = -cg.time / 1000.0f;

	re->hModel = cgs.media.kamikazeEffectModel;

	VectorCopy(org, re->origin);
}


/*
==================
CG_ObeliskExplode
==================
*/
void CG_ObeliskExplode(vec3_t org, int entityNum)
{
	localEntity_t  *le;
	vec3_t          origin;

	// create an explosion
	VectorCopy(org, origin);
	origin[2] += 64;
	le = CG_MakeExplosion(origin, vec3_origin, cgs.media.dishFlashModel, cgs.media.rocketExplosionShader, 600, qtrue);
	le->light = 300;
	le->lightColor[0] = 1;
	le->lightColor[1] = 0.75;
	le->lightColor[2] = 0.0;
}

/*
==================
CG_ObeliskPain
==================
*/
void CG_ObeliskPain(vec3_t org)
{
	float           r;
	sfxHandle_t     sfx;

	// hit sound
	r = rand() & 3;
	if(r < 2)
	{
		sfx = cgs.media.obeliskHitSound1;
	}
	else if(r == 2)
	{
		sfx = cgs.media.obeliskHitSound2;
	}
	else
	{
		sfx = cgs.media.obeliskHitSound3;
	}
	trap_S_StartSound(org, ENTITYNUM_NONE, CHAN_BODY, sfx);
}

#ifdef MISSIONPACK
/*
==================
CG_InvulnerabilityImpact
==================
*/
void CG_InvulnerabilityImpact(vec3_t org, vec3_t angles)
{
	localEntity_t  *le;
	refEntity_t    *re;
	int             r;
	sfxHandle_t     sfx;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_INVULIMPACT;
	le->startTime = cg.time;
	le->endTime = cg.time + 1000;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);

	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;

	re = &le->refEntity;

	re->reType = RT_MODEL;
	re->shaderTime = -cg.time / 1000.0f;

	re->hModel = cgs.media.invulnerabilityImpactModel;

	VectorCopy(org, re->origin);
	AnglesToAxis(angles, re->axis);

	r = rand() & 3;
	if(r < 2)
	{
		sfx = cgs.media.invulnerabilityImpactSound1;
	}
	else if(r == 2)
	{
		sfx = cgs.media.invulnerabilityImpactSound2;
	}
	else
	{
		sfx = cgs.media.invulnerabilityImpactSound3;
	}
	trap_S_StartSound(org, ENTITYNUM_NONE, CHAN_BODY, sfx);
}

/*
==================
CG_InvulnerabilityJuiced
==================
*/
void CG_InvulnerabilityJuiced(vec3_t org)
{
	localEntity_t  *le;
	refEntity_t    *re;
	vec3_t          angles;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_INVULJUICED;
	le->startTime = cg.time;
	le->endTime = cg.time + 10000;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);

	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;

	re = &le->refEntity;

	re->reType = RT_MODEL;
	re->shaderTime = -cg.time / 1000.0f;

	re->hModel = cgs.media.invulnerabilityJuicedModel;

	VectorCopy(org, re->origin);
	VectorClear(angles);
	AnglesToAxis(angles, re->axis);

	trap_S_StartSound(org, ENTITYNUM_NONE, CHAN_BODY, cgs.media.invulnerabilityJuicedSound);
}

#endif

/*
==================
CG_ScorePlum
==================
*/
void CG_ScorePlum(int client, vec3_t org, int score)
{
	localEntity_t  *le;
	refEntity_t    *re;
	vec3_t          angles;
	static vec3_t   lastPos;

	// only visualize for the client that scored
	if(client != cg.predictedPlayerState.clientNum || cg_scorePlum.integer == 0)
	{
		return;
	}

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_SCOREPLUM;
	le->startTime = cg.time;
	le->endTime = cg.time + 4000;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);


	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;
	le->radius = score;

	VectorCopy(org, le->pos.trBase);
	if(org[2] >= lastPos[2] - 20 && org[2] <= lastPos[2] + 20)
	{
		le->pos.trBase[2] -= 20;
	}

	//CG_Printf( "Plum origin %i %i %i -- %i\n", (int)org[0], (int)org[1], (int)org[2], (int)Distance(org, lastPos));
	VectorCopy(org, lastPos);


	re = &le->refEntity;

	re->reType = RT_SPRITE;
	re->radius = 16;

	VectorClear(angles);
	AnglesToAxis(angles, re->axis);
}


/*
====================
CG_MakeExplosion
====================
*/
localEntity_t  *CG_MakeExplosion(vec3_t origin, vec3_t dir, qhandle_t hModel, qhandle_t shader, int msec, qboolean isSprite)
{
	float           ang;
	localEntity_t  *ex;
	int             offset;
	vec3_t          tmpVec, newOrigin;

	if(msec <= 0)
	{
		CG_Error("CG_MakeExplosion: msec = %i", msec);
	}

	// skew the time a bit so they aren't all in sync
	offset = rand() & 63;

	ex = CG_AllocLocalEntity();
	if(isSprite)
	{
		ex->leType = LE_SPRITE_EXPLOSION;

		// randomly rotate sprite orientation
		ex->refEntity.rotation = rand() % 360;
		VectorScale(dir, 16, tmpVec);
		VectorAdd(tmpVec, origin, newOrigin);
	}
	else
	{
		ex->leType = LE_EXPLOSION;
		VectorCopy(origin, newOrigin);

		// set axis with random rotate
		if(!dir)
		{
			AxisClear(ex->refEntity.axis);
		}
		else
		{
			ang = rand() % 360;
			VectorCopy(dir, ex->refEntity.axis[0]);
			RotateAroundDirection(ex->refEntity.axis, ang);
		}
	}

	ex->startTime = cg.time - offset;
	ex->endTime = ex->startTime + msec;

	// bias the time so all shader effects start correctly
	ex->refEntity.shaderTime = -ex->startTime / 1000.0f;

	ex->refEntity.hModel = hModel;
	ex->refEntity.customShader = shader;

	// set origin
	VectorCopy(newOrigin, ex->refEntity.origin);
	VectorCopy(newOrigin, ex->refEntity.oldorigin);

	ex->color[0] = ex->color[1] = ex->color[2] = 1.0;

	return ex;
}


/*
=================
CG_Bleed

This is the spurt of blood when a character gets hit
=================
*/
void CG_Bleed(vec3_t origin, int entityNum)
{
	localEntity_t  *ex;

	if(!cg_blood.integer)
	{
		return;
	}

	ex = CG_AllocLocalEntity();
	ex->leType = LE_EXPLOSION;

	ex->startTime = cg.time;
	ex->endTime = ex->startTime + 500;

	VectorCopy(origin, ex->refEntity.origin);
	ex->refEntity.reType = RT_SPRITE;
	ex->refEntity.rotation = rand() % 360;
	ex->refEntity.radius = 24;

	ex->refEntity.customShader = cgs.media.bloodExplosionShader;

	// don't show player's own blood in view
	if(entityNum == cg.snap->ps.clientNum)
	{
		ex->refEntity.renderfx |= RF_THIRD_PERSON;
	}
}



/*
==================
CG_LaunchGib
==================
*/
void CG_LaunchGib(vec3_t origin, vec3_t velocity, qhandle_t hModel)
{
	localEntity_t  *le;
	refEntity_t    *re;

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + 5000 + random() * 3000;

	VectorCopy(origin, re->origin);
	AxisCopy(axisDefault, re->axis);
	re->hModel = hModel;

	le->pos.trType = TR_GRAVITY;
	le->pos.trAcceleration = -cg_gravityZ.value;
	VectorCopy(origin, le->pos.trBase);
	VectorCopy(velocity, le->pos.trDelta);
	le->pos.trTime = cg.time;

	le->bounceFactor = 0.6f;

	le->leBounceSoundType = LEBS_BLOOD;
	le->leMarkType = LEMT_BLOOD;

	// Tr3B - new quaternion code
	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	le->angVel = 20 * crandom();	// random angular velocity
	le->rotAxis[0] = crandom();	// random axis of rotation
	le->rotAxis[1] = crandom();
	le->rotAxis[2] = crandom();
	VectorNormalize(le->rotAxis);	// normalize the rotation axis
	QuatClear(le->quatRot);
	QuatClear(le->quatOrient);
	le->radius = 12;
	le->leFlags = LEF_TUMBLE;
}

/*
===================
CG_GibPlayer

Generated a bunch of gibs launching out from the bodies location
===================
*/
#define	GIB_VELOCITY	250
#define	GIB_JUMP		250
void CG_GibPlayer(vec3_t playerOrigin)
{
	vec3_t          origin, velocity;

	if(!cg_blood.integer)
	{
		return;
	}

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom() * GIB_VELOCITY;
	velocity[1] = crandom() * GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom() * GIB_VELOCITY;
	if(rand() & 1)
	{
		CG_LaunchGib(origin, velocity, cgs.media.gibSkull);
	}
	else
	{
		CG_LaunchGib(origin, velocity, cgs.media.gibBrain);
	}

	// allow gibs to be turned off for speed
	if(!cg_gibs.integer)
	{
		return;
	}

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom() * GIB_VELOCITY;
	velocity[1] = crandom() * GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom() * GIB_VELOCITY;
	CG_LaunchGib(origin, velocity, cgs.media.gibAbdomen);

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom() * GIB_VELOCITY;
	velocity[1] = crandom() * GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom() * GIB_VELOCITY;
	CG_LaunchGib(origin, velocity, cgs.media.gibArm);

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom() * GIB_VELOCITY;
	velocity[1] = crandom() * GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom() * GIB_VELOCITY;
	CG_LaunchGib(origin, velocity, cgs.media.gibChest);

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom() * GIB_VELOCITY;
	velocity[1] = crandom() * GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom() * GIB_VELOCITY;
	CG_LaunchGib(origin, velocity, cgs.media.gibFist);

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom() * GIB_VELOCITY;
	velocity[1] = crandom() * GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom() * GIB_VELOCITY;
	CG_LaunchGib(origin, velocity, cgs.media.gibFoot);

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom() * GIB_VELOCITY;
	velocity[1] = crandom() * GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom() * GIB_VELOCITY;
	CG_LaunchGib(origin, velocity, cgs.media.gibForearm);

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom() * GIB_VELOCITY;
	velocity[1] = crandom() * GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom() * GIB_VELOCITY;
	CG_LaunchGib(origin, velocity, cgs.media.gibIntestine);

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom() * GIB_VELOCITY;
	velocity[1] = crandom() * GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom() * GIB_VELOCITY;
	CG_LaunchGib(origin, velocity, cgs.media.gibLeg);

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom() * GIB_VELOCITY;
	velocity[1] = crandom() * GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom() * GIB_VELOCITY;
	CG_LaunchGib(origin, velocity, cgs.media.gibLeg);
}

void CG_FireEffect(vec3_t org, vec3_t mins, vec3_t maxs, float flameSize, int particles, float intensity)
{
	cparticle_t    *p;
	refEntity_t     re;
	int             i;
	refLight_t      light;
	const vec3_t    velmax = {25.0f, 25.0f, 25.0f};

	memset(&re, 0, sizeof(re));
	memset(&light, 0, sizeof(light));

	//Setup light
	QuatClear(light.rotation);
	light.color[0] = 0.99f;
	light.color[1] = 0.54f;
	light.color[2] = 0.21f;
	VectorCopy(org, light.origin);
	light.radius[0] = intensity;
	light.radius[1] = intensity;
	light.radius[2] = intensity;
	light.attenuationShader = cgs.media.fireLight;

	trap_R_AddRefLightToScene(&light);

	//Configure sprite
	VectorCopy(org, re.origin);
	re.origin[2] += flameSize * 0.7;
	re.reType = RT_SPRITE;
	re.radius = (float)flameSize;
	re.renderfx = 0;
	re.customShader = cgs.media.fire;

	trap_R_AddRefEntityToScene(&re);

	i = random() * 10;

	//Add extra particles
	//This should be cleaned up to use the particles variable
	if((cg.time % 200) >= 100)
	{
		p = CG_AllocParticle();
		if(!p)
			return;

		p->flags = PF_AIRONLY;
		p->time = cg.time - (int)(random() * 100);
		p->endTime = p->time + 750 + (random() * 250);

		p->roll = crandom() * 20;
		p->rotate = qtrue;

		p->type = P_SMOKE_IMPACT;
		p->pshader = cgs.media.flames[i % 3];

		p->width = 8.0f + (crandom() * 4.0f);
		p->height = p->width;

		p->endHeight = p->height / 4;
		p->endWidth = p->width / 4;

		VectorCopy(org, p->org);
		VectorRandom(p->org, mins, maxs);
		VectorClear(p->vel);
		VectorRandomUniform(p->vel, velmax);

		p->accel[0] = -p->vel[0] / 2;
		p->accel[1] = -p->vel[0] / 2;
		p->accel[2] = random() * 32 + 8;
	}
}

/*
==================
CG_Fire

==================
*/
void CG_Fire(centity_t * cent)
{
	vec3_t          mins, maxs;

	VectorSubtract(cent->currentState.origin2, cent->lerpOrigin, maxs);
	VectorSubtract(cent->lerpOrigin, cent->currentState.origin2, mins);

	CG_FireEffect(cent->lerpOrigin, mins, maxs, (float)cent->currentState.generic1, cent->currentState.weapon, (float)cent->currentState.constantLight);
}

void CG_AddFire(localEntity_t * le)
{
	float           flameSize;

	flameSize = le->radius + 15;
	if(flameSize > 80.0)
	{
		flameSize = 80.0;
	}
	if(flameSize < 30.0)
	{
		flameSize = 30.0;
	}

	CG_FireEffect(le->pos.trBase, zeroVector, zeroVector, flameSize, 1, le->light);
}

/*
==================
CG_ExplosiveRubble
Rubble chunks
==================
*/
void CG_ExplosiveModel(vec3_t origin, vec3_t mins, vec3_t maxs, qhandle_t model, vec3_t matrix[3])
{
	localEntity_t  *le;
	refEntity_t    *re;
	const vec3_t    avelmax = {100.0f, 100.0f, 100.0f};
	const vec3_t    velmax = {200.0f, 200.0f, 100.0f};
	vec3_t          tmp;

	//Spawn debris ents
	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	//Initialize type
	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + 10000 + random() * 5000;

	//Initialize appearance
	re->hModel = model;

	//Initialize random rotations
	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	VectorClear(le->angles.trDelta);
	VectorRandomUniform(le->angles.trDelta, avelmax);
	le->angVel = 20 * crandom();	// random angular velocity
	le->rotAxis[0] = crandom();	// random axis of rotation
	le->rotAxis[1] = crandom();
	le->rotAxis[2] = crandom();
	VectorNormalize(le->rotAxis);	// normalize the rotation axis
	QuatClear(le->quatRot);
	QuatClear(le->quatOrient);
	le->radius = 16;
	le->leFlags = LEF_TUMBLE;

	//Initialize location
	AxisCopy(axisDefault, re->axis);
	VectorCopy(origin, le->pos.trBase);
	VectorCopy(origin, re->origin);
	le->pos.trTime = cg.time;
	if(matrix != NULL)
	{
		VectorCopy(mins, tmp);
		VectorRotate(tmp, matrix, mins);
		VectorCopy(maxs, tmp);
		VectorRotate(tmp, matrix, maxs);
	}
	//Randomly offset base within model bounds
	VectorRandom(le->pos.trBase, mins, maxs);

	//Initialize velocity
	le->pos.trType = TR_GRAVITY;
	le->pos.trAcceleration = -cg_gravityZ.value;
	VectorClear(le->pos.trDelta);
	VectorRandomUniform(le->pos.trDelta, velmax);
	if(matrix != NULL)
	{
		VectorCopy(le->pos.trDelta, tmp);
		VectorRotate(tmp, matrix, le->pos.trDelta);
	}

	//Initialize bouncing
	le->bounceFactor = 0.4f;
	le->leBounceSoundType = LEBS_NONE;
	le->leMarkType = LEMT_NONE;
}

/*
==================
CG_ExplosiveRubble
Rubble chunks
==================
*/
void CG_ExplosiveRubble(vec3_t origin, vec3_t mins, vec3_t maxs, qhandle_t model)
{
	CG_ExplosiveModel(origin, mins, maxs, model, NULL);
}

/*
==================
CG_ExplosiveBlood
Blood
==================
*/
void CG_ExplosiveBlood(vec3_t origin, vec3_t mins, vec3_t maxs, int count)
{
	vec3_t          bloodOrigin;
	vec3_t          impactVel;
	int             i;

	VectorCopy(zeroVector, impactVel);
	for(i = count; i > 0; i--)
	{
		VectorCopy(origin, bloodOrigin);
		VectorRandom(bloodOrigin, mins, maxs);
		CG_ParticleBlood(bloodOrigin, impactVel, 4);
	}
}

/*
==================
CG_ExplosiveDust
Small particles and smoke
==================
*/
void CG_ExplosiveDust(vec3_t org, vec3_t mins, vec3_t maxs, int smokes, int dusts)
{
	cparticle_t    *p;
	int             i;
	const vec3_t    smoke_velmax = {128.0f, 128.0f, 32.0f};
	const vec3_t    dust_velmax = {180.0f, 180.0f, 100.0f};

	for(i = smokes; i > 0; i--)
	{
		//CG_ExplosiveParticles(cent->lerpOrigin, debrisVel, debrisAccel, 10000, cgs.media.smokePuffShader, 5.0f , 5.0f);

		p = CG_AllocParticle();
		if(!p)
			return;

		p->flags = PF_AIRONLY;
		p->time = cg.time;
		p->endTime = cg.time + 10000;

		p->color[0] = 0.1f;
		p->color[1] = 0.1f;
		p->color[2] = 0.1f;
		p->color[3] = 0.60f;

		p->colorVel[0] = 0.7f;
		p->colorVel[1] = 0.7f;
		p->colorVel[2] = 0.7f;
		p->colorVel[3] = -1.0 / (0.5 + random() * 0.5);

		p->type = P_SMOKE_IMPACT;
		p->pshader = cgs.media.smokePuffShader;

		p->width = 5.0f + (i * 0.5f);
		p->height = 5.0f + (i * 0.5f);

		p->endHeight = p->height * 2;
		p->endWidth = p->width * 2;

		VectorCopy(org, p->org);
		VectorRandom(p->org, mins, maxs);

		VectorClear(p->vel);
		VectorRandomUniform(p->vel, smoke_velmax);

		p->accel[0] = crandom() * 64;
		p->accel[1] = crandom() * 64;
		p->accel[2] = 25;
	}

	for(i = dusts; i > 0; i--)
	{
		p = CG_AllocParticle();
		if(!p)
			return;

		p->time = cg.time;

		p->endTime = cg.time + 4000;
		p->startfade = cg.time + 4000 / 2;

		//p->color = EMISIVEFADE;
		p->color[3] = 1.0;
		p->colorVel[3] = 0;

		p->height = 0.5;
		p->width = 0.5;
		p->endHeight = 0.5;
		p->endWidth = 0.5;

		p->pshader = cgs.media.debrisBit;

		p->type = P_SMOKE;

		VectorCopy(org, p->org);

		VectorClear(p->vel);
		VectorRandomUniform(p->vel, dust_velmax);

		VectorClear(p->accel);
		p->accel[2] = -100;
		p->vel[2] += -20;
	}
}

void CG_ExplosivePlaster(vec3_t org, vec3_t mins, vec3_t maxs, int plasters)
{
	cparticle_t    *p;
	int             i;
	const vec3_t    plaster_velmax = {180.0f, 180.0f, 100.0f};

	for(i = plasters; i > 0; i--)
	{
		p = CG_AllocParticle();
		if(!p)
			return;

		p->time = cg.time;

		p->endTime = cg.time + 5000;
		p->startfade = cg.time + 4000 / 2;

		//no alpha fade
		p->color[3] = 1.0;
		p->colorVel[3] = 0;

		p->height = 6.0f;
		p->width = 6.0f;
		p->endHeight = 6.0f;
		p->endWidth = 6.0f;

		p->pshader = cgs.media.debrisPlaster;

		p->type = P_SMOKE;

		VectorCopy(org, p->org);

		VectorClear(p->vel);
		VectorRandomUniform(p->vel, plaster_velmax);

		VectorClear(p->accel);
		p->accel[2] = -100;
		p->vel[2] += -20;
	}
}

void CG_ExplosiveSmoke(vec3_t org, vec3_t mins, vec3_t maxs, int smokes)
{
	cparticle_t    *p;
	int             i;
	const vec3_t    smoke_velmax = {128.0f, 128.0f, 32.0f};

	for(i = smokes; i > 0; i--)
	{
		p = CG_AllocParticle();
		if(!p)
			return;

		p->flags = PF_AIRONLY;
		p->time = cg.time;
		p->endTime = cg.time + 10000;

		p->color[0] = 0.1f;
		p->color[1] = 0.1f;
		p->color[2] = 0.1f;
		p->color[3] = 0.60f;

		p->colorVel[0] = 0.2f;
		p->colorVel[1] = 0.2f;
		p->colorVel[2] = 0.2f;
		p->colorVel[3] = (float)(-(0.4 + random() * 0.4));

		p->type = P_SMOKE_IMPACT;
		p->pshader = cgs.media.smokePuffShader;

		p->width = 8.0f + (i * 1.0f);
		p->height = 8.0f + (i * 1.0f);

		p->endHeight = p->height * 2;
		p->endWidth = p->width * 2;

		VectorCopy(org, p->org);
		VectorRandom(p->org, mins, maxs);

		VectorClear(p->vel);
		VectorRandomUniform(p->vel, smoke_velmax);

		p->accel[0] = crandom() * 64;
		p->accel[1] = crandom() * 64;
		p->accel[2] = 25;
	}
}

void CG_ExplosiveGas(vec3_t org, vec3_t mins, vec3_t maxs, int smokes)
{
	cparticle_t    *p;
	int             i;

	for(i = smokes; i > 0; i--)
	{
		p = CG_AllocParticle();
		if(!p)
			return;

		p->flags = PF_AIRONLY;
		p->time = cg.time;
		p->endTime = cg.time + 10000;

		p->color[0] = 0.1f;
		p->color[1] = 0.1f;
		p->color[2] = 0.1f;
		p->color[3] = 0.60f;

		p->colorVel[0] = 0.2f;
		p->colorVel[1] = 0.2f;
		p->colorVel[2] = 0.2f;
		p->colorVel[3] = (float)(-(0.4 + random() * 0.4));

		p->type = P_SMOKE_IMPACT;
		p->pshader = cgs.media.smokePuffShader;

		p->width = 8.0f + (i * 1.0f);
		p->height = 8.0f + (i * 1.0f);

		p->endHeight = p->height * 2;
		p->endWidth = p->width * 2;

		VectorCopy(org, p->org);
		VectorRandom(p->org, mins, maxs);

		p->vel[0] = crandom() * 128;
		p->vel[1] = crandom() * 128;
		p->vel[2] = crandom() * 16;

		p->accel[0] = crandom() * 64;
		p->accel[1] = crandom() * 64;
		p->accel[2] = -64;
	}
}

void CG_ExplosiveFire(vec3_t org, vec3_t mins, vec3_t maxs, int fires)
{
	localEntity_t  *le;

	le = CG_AllocLocalEntity();
	VectorCopy(org, le->pos.trBase);
	le->radius = fires;
	le->light = fires * 4;
	le->leType = LE_FIRE;
	le->startTime = cg.time;
	le->endTime = le->startTime + 700;
}

/*
==================
CG_ExplosiveExplode
Client side effects for a func_explosive's death
==================
*/
void CG_ExplosiveExplode(centity_t * cent)
{
	qhandle_t       centmodel;
	vec3_t          mins, maxs;
	int             i;

	//Prevent excessively long loops
	if(cent->currentState.torsoAnim > 10)
	{
		cent->currentState.torsoAnim = 10;
	}
	if(cent->currentState.legsAnim > 15)
	{
		cent->currentState.legsAnim = 15;
	}
	if(cent->currentState.weapon > 20)
	{
		cent->currentState.weapon = 20;
	}

	//Get area of model
	centmodel = cgs.inlineDrawModel[cent->currentState.modelindex];
	trap_R_ModelBounds(centmodel, mins, maxs);

	switch (cent->currentState.generic1)	//Type
	{
		case ENTMAT_WOOD:
			//Sawdust?
			break;
		case ENTMAT_GLASS:
			break;
		case ENTMAT_METAL:
			//Sparks?
			break;
		case ENTMAT_GIBS:
		case ENTMAT_BODY:
			//Blood
			CG_ExplosiveBlood(cent->lerpOrigin,
							  mins,
							  maxs,
							  (3 * cent->currentState.torsoAnim + 2 * cent->currentState.legsAnim + cent->currentState.weapon));
			break;
		case ENTMAT_BRICK:
		case ENTMAT_STONE:
		case ENTMAT_TILES:
			//Dust
			CG_ExplosiveDust(cent->lerpOrigin,
							 mins,
							 maxs,
							 (2 * cent->currentState.torsoAnim + cent->currentState.legsAnim),
							 (10 * cent->currentState.torsoAnim + 5 * cent->currentState.legsAnim +
							  2 * cent->currentState.weapon));
			break;
		case ENTMAT_PLASTER:
			//Dust & flakes?
			CG_ExplosivePlaster(cent->lerpOrigin,
								mins,
								maxs,
								(8 * cent->currentState.torsoAnim + 4 * cent->currentState.legsAnim +
								 1 * cent->currentState.weapon));
			break;
		case ENTMAT_FIBERS:
			//Fiber flakes?
			break;
		case ENTMAT_SPRITE:
			//Sprite
			break;
		case ENTMAT_SMOKE:
			//Smoke (lighter than air)
			CG_ExplosiveSmoke(cent->lerpOrigin,
							  mins,
							  maxs,
							  (4 * cent->currentState.torsoAnim + 2 * cent->currentState.legsAnim +
							   1 * cent->currentState.weapon));
			break;
		case ENTMAT_GAS:
			//Gas (heavier than air)
			CG_ExplosiveGas(cent->lerpOrigin,
							mins,
							maxs,
							(4 * cent->currentState.torsoAnim + 2 * cent->currentState.legsAnim + 1 * cent->currentState.weapon));
			break;
		case ENTMAT_FIRE:
			//Flames
			CG_ExplosiveFire(cent->lerpOrigin,
							 mins,
							 maxs,
							 (9 * cent->currentState.torsoAnim + 5 * cent->currentState.legsAnim +
							  3 * cent->currentState.weapon));
			break;
		case ENTMAT_NONE:
		default:
			break;
	}

	for(i = cent->currentState.torsoAnim; i > 0; i--)
	{
		//Largest rubble
		CG_ExplosiveRubble(cent->lerpOrigin, mins, maxs, cgs.media.debrisModels[cent->currentState.generic1][2][i & 1]);
	}

	for(i = cent->currentState.legsAnim; i > 0; i--)
	{
		//Medium rubble
		CG_ExplosiveRubble(cent->lerpOrigin, mins, maxs, cgs.media.debrisModels[cent->currentState.generic1][1][i & 1]);
	}

	for(i = cent->currentState.weapon; i > 0; i--)
	{
		//Small rubble
		CG_ExplosiveRubble(cent->lerpOrigin, mins, maxs, cgs.media.debrisModels[cent->currentState.generic1][0][i & 1]);
	}
}
