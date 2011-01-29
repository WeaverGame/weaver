/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>

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
// cg_ents.c -- present snapshot entities, happens every single frame

#include "cg_local.h"


/*
======================
CG_PositionEntityOnTag

Modifies the entities position and axis by the given
tag location
======================
*/
void CG_PositionEntityOnTag(refEntity_t * entity, const refEntity_t * parent, qhandle_t parentModel, char *tagName)
{
	int             i;
	orientation_t   lerped;

	// lerp the tag
	trap_R_LerpTag(&lerped, parentModel, parent->oldframe, parent->frame, 1.0 - parent->backlerp, tagName);

	// FIXME: allow origin offsets along tag?
	VectorCopy(parent->origin, entity->origin);
	for(i = 0; i < 3; i++)
	{
		VectorMA(entity->origin, lerped.origin[i], parent->axis[i], entity->origin);
	}

	// had to cast away the const to avoid compiler problems...
	AxisMultiply(lerped.axis, ((refEntity_t *) parent)->axis, entity->axis);
	entity->backlerp = parent->backlerp;
}


/*
======================
CG_PositionRotatedEntityOnTag

Modifies the entities position and axis by the given
tag location
======================
*/
void CG_PositionRotatedEntityOnTag(refEntity_t * entity, const refEntity_t * parent, qhandle_t parentModel, char *tagName)
{
	int             i;
	orientation_t   lerped;
	vec3_t          tempAxis[3];

	// lerp the tag
	trap_R_LerpTag(&lerped, parentModel, parent->oldframe, parent->frame, 1.0 - parent->backlerp, tagName);

	// FIXME: allow origin offsets along tag?
	VectorCopy(parent->origin, entity->origin);
	for(i = 0; i < 3; i++)
	{
		VectorMA(entity->origin, lerped.origin[i], parent->axis[i], entity->origin);
	}

	// had to cast away the const to avoid compiler problems...
	AxisMultiply(entity->axis, lerped.axis, tempAxis);
	AxisMultiply(tempAxis, ((refEntity_t *) parent)->axis, entity->axis);
}

/*
======================
CG_PositionRotatedEntityOnTag

Modifies the entities position and axis by the given
tag location
======================
*/
qboolean CG_PositionRotatedEntityOnBone(refEntity_t * entity, const refEntity_t * parent, qhandle_t parentModel, char *tagName)
{
	int             i;
	int             boneIndex;
	orientation_t   lerped;
	vec3_t          tempAxis[3];

	// lerp the tag
	boneIndex = trap_R_BoneIndex(parentModel, tagName);
	if(boneIndex == -1)
		return qfalse;

	VectorCopy(parent->skeleton.bones[boneIndex].origin, lerped.origin);
	QuatToAxis(parent->skeleton.bones[boneIndex].rotation, lerped.axis);

	// FIXME: allow origin offsets along tag?
	VectorCopy(parent->origin, entity->origin);
	for(i = 0; i < 3; i++)
	{
		VectorMA(entity->origin, lerped.origin[i], parent->axis[i], entity->origin);
	}

	// had to cast away the const to avoid compiler problems...
	AxisMultiply(entity->axis, lerped.axis, tempAxis);
	AxisMultiply(tempAxis, ((refEntity_t *) parent)->axis, entity->axis);

	return qtrue;
}


/*
=================
CG_TransformSkeleton

transform relative bones to absolute ones required for vertex skinning
=================
*/
void CG_TransformSkeleton(refSkeleton_t * skel, const vec3_t scale)
{
	int             i;
	refBone_t      *bone;

	switch (skel->type)
	{
		case SK_INVALID:
		case SK_ABSOLUTE:
			return;

		default:
			break;
	}

	// calculate absolute transforms
	for(i = 0, bone = &skel->bones[0]; i < skel->numBones; i++, bone++)
	{
		if(bone->parentIndex >= 0)
		{
			vec3_t          rotated;
			quat_t          quat;

			refBone_t      *parent;

			parent = &skel->bones[bone->parentIndex];

			QuatTransformVector(parent->rotation, bone->origin, rotated);

			if(scale)
			{
				rotated[0] *= scale[0];
				rotated[1] *= scale[1];
				rotated[2] *= scale[2];
			}

			VectorAdd(parent->origin, rotated, bone->origin);

			QuatMultiply1(parent->rotation, bone->rotation, quat);
			QuatCopy(quat, bone->rotation);
		}
	}

	skel->type = SK_ABSOLUTE;

	if(scale)
	{
		VectorCopy(scale, skel->scale);
	}
	else
	{
		VectorSet(skel->scale, 1, 1, 1);
	}
}



/*
==========================================================================

FUNCTIONS CALLED EACH FRAME

==========================================================================
*/

/*
======================
CG_SetEntitySoundPosition

Also called by event processing code
======================
*/
void CG_SetEntitySoundPosition(centity_t * cent)
{
	if(cent->currentState.solid == SOLID_BMODEL)
	{
		vec3_t          origin;
		float          *v;

		v = cgs.inlineModelMidpoints[cent->currentState.modelindex];
		VectorAdd(cent->lerpOrigin, v, origin);
		trap_S_UpdateEntityPosition(cent->currentState.number, origin);
	}
	else
	{
		trap_S_UpdateEntityPosition(cent->currentState.number, cent->lerpOrigin);
	}
}

/*
==================
CG_EntityEffects

Add continuous entity effects, like local entity emission and lighting
==================
*/
static void CG_EntityEffects(centity_t * cent)
{

	// update sound origins
	CG_SetEntitySoundPosition(cent);

	// add loop sound
	if(cent->currentState.loopSound)
	{
		if(cent->currentState.eType != ET_SPEAKER)
		{
			trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin,
								   cgs.gameSounds[cent->currentState.loopSound]);
		}
		else
		{
			trap_S_AddRealLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin,
									   cgs.gameSounds[cent->currentState.loopSound]);
		}
	}


	// constant light glow
	if(cent->currentState.constantLight)
	{
		int             cl;
		int             i, r, g, b;

		cl = cent->currentState.constantLight;
		r = cl & 255;
		g = (cl >> 8) & 255;
		b = (cl >> 16) & 255;
		i = ((cl >> 24) & 255) * 4;
		trap_R_AddLightToScene(cent->lerpOrigin, i, r, g, b);
	}

}


/*
==================
CG_General
==================
*/
static void CG_General(centity_t * cent)
{
	refEntity_t     ent;
	entityState_t  *s1;

	s1 = &cent->currentState;

	// if set to invisible, skip
	if(!s1->modelindex)
	{
		return;
	}

	memset(&ent, 0, sizeof(ent));

	// set frame

	ent.frame = s1->frame;
	ent.oldframe = ent.frame;
	ent.backlerp = 0;

	VectorCopy(cent->lerpOrigin, ent.origin);
	VectorCopy(cent->lerpOrigin, ent.oldorigin);

	ent.hModel = cgs.gameModels[s1->modelindex];

	// player model
	if(s1->number == cg.snap->ps.clientNum)
	{
		ent.renderfx |= RF_THIRD_PERSON;	// only draw from mirrors
	}

	// convert angles to axis
	AnglesToAxis(cent->lerpAngles, ent.axis);

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);
}

/*
==================
CG_Speaker

Speaker entities can automatically play sounds
==================
*/
static void CG_Speaker(centity_t * cent)
{
	if(!cent->currentState.clientNum)
	{							// FIXME: use something other than clientNum...
		return;					// not auto triggering
	}

	if(cg.time < cent->miscTime)
	{
		return;
	}

	trap_S_StartSound(NULL, cent->currentState.number, CHAN_ITEM, cgs.gameSounds[cent->currentState.eventParm]);

	//  ent->s.frame = ent->wait * 10;
	//  ent->s.clientNum = ent->random * 10;
	cent->miscTime = cg.time + cent->currentState.frame * 100 + cent->currentState.clientNum * 100 * crandom();
}

/*
==================
CG_Item
==================
*/
static void CG_Item(centity_t * cent)
{
	refEntity_t     ent;
	entityState_t  *es;
	gitem_t        *item;
	int             msec;
	float           frac;
	float           scale;
	weaponInfo_t   *wi;

	es = &cent->currentState;
	if(es->modelindex >= bg_numItems)
	{
		CG_Error("Bad item index %i on entity", es->modelindex);
	}

	// if set to invisible, skip
	if(!es->modelindex || (es->eFlags & EF_NODRAW))
	{
		return;
	}

	item = &bg_itemlist[es->modelindex];
	if(cg_simpleItems.integer && item->giType != IT_TEAM)
	{
		memset(&ent, 0, sizeof(ent));
		ent.reType = RT_SPRITE;
		VectorCopy(cent->lerpOrigin, ent.origin);
		ent.radius = 14;
		ent.customShader = cg_items[es->modelindex].icon;
		ent.shaderRGBA[0] = 255;
		ent.shaderRGBA[1] = 255;
		ent.shaderRGBA[2] = 255;
		ent.shaderRGBA[3] = 255;
		trap_R_AddRefEntityToScene(&ent);
		return;
	}

	// items bob up and down continuously
	scale = 0.005 + cent->currentState.number * 0.00001;
	cent->lerpOrigin[2] += 4 + cos((cg.time + 1000) * scale) * 4;

	memset(&ent, 0, sizeof(ent));

	// autorotate at one of two speeds
	if(item->giType == IT_HEALTH)
	{
		VectorCopy(cg.autoAnglesFast, cent->lerpAngles);
		AxisCopy(cg.autoAxisFast, ent.axis);
	}
	else
	{
		VectorCopy(cg.autoAngles, cent->lerpAngles);
		AxisCopy(cg.autoAxis, ent.axis);
	}

	wi = NULL;
	// the weapons have their origin where they attatch to player
	// models, so we need to offset them or they will rotate
	// eccentricly
	if(item->giType == IT_WEAPON)
	{
		wi = &cg_weapons[item->giTag];
		cent->lerpOrigin[0] -=
			wi->weaponMidpoint[0] * ent.axis[0][0] +
			wi->weaponMidpoint[1] * ent.axis[1][0] + wi->weaponMidpoint[2] * ent.axis[2][0];
		cent->lerpOrigin[1] -=
			wi->weaponMidpoint[0] * ent.axis[0][1] +
			wi->weaponMidpoint[1] * ent.axis[1][1] + wi->weaponMidpoint[2] * ent.axis[2][1];
		cent->lerpOrigin[2] -=
			wi->weaponMidpoint[0] * ent.axis[0][2] +
			wi->weaponMidpoint[1] * ent.axis[1][2] + wi->weaponMidpoint[2] * ent.axis[2][2];

		cent->lerpOrigin[2] += 8;	// an extra height boost

		ent.hModel = wi->weaponModel;
	}
	else
	{
		ent.hModel = cg_items[es->modelindex].models[0];
	}

	VectorCopy(cent->lerpOrigin, ent.origin);
	VectorCopy(cent->lerpOrigin, ent.oldorigin);

	ent.nonNormalizedAxes = qfalse;

	// if just respawned, slowly scale up
	msec = cg.time - cent->miscTime;
	if(msec >= 0 && msec < ITEM_SCALEUP_TIME)
	{
		frac = (float)msec / ITEM_SCALEUP_TIME;
		VectorScale(ent.axis[0], frac, ent.axis[0]);
		VectorScale(ent.axis[1], frac, ent.axis[1]);
		VectorScale(ent.axis[2], frac, ent.axis[2]);
		ent.nonNormalizedAxes = qtrue;

		// don't cast shadows in this time period
		ent.renderfx |= RF_NOSHADOW;
	}
	else
	{
		frac = 1.0;
	}

	// items without glow textures need to keep a minimum light value
	// so they are always visible
	if((item->giType == IT_WEAPON) || (item->giType == IT_ARMOR))
	{
		ent.renderfx |= RF_MINLIGHT;
	}

	// increase the size of the weapons when they are presented as items
	if(item->giType == IT_WEAPON)
	{
		VectorScale(ent.axis[0], 1.5, ent.axis[0]);
		VectorScale(ent.axis[1], 1.5, ent.axis[1]);
		VectorScale(ent.axis[2], 1.5, ent.axis[2]);
		ent.nonNormalizedAxes = qtrue;
#if 0							//defined(MISSIONPACK)
		trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin, cgs.media.weaponHoverSound);
#endif
	}

	if(item->giType == IT_HOLDABLE && item->giTag == HI_KAMIKAZE)
	{
		VectorScale(ent.axis[0], 2, ent.axis[0]);
		VectorScale(ent.axis[1], 2, ent.axis[1]);
		VectorScale(ent.axis[2], 2, ent.axis[2]);
		ent.nonNormalizedAxes = qtrue;
	}

	// Tr3B: added skins support so we don't need multiple versions of the same team models
	if(item->skins[0])
	{
		ent.customSkin = cg_items[es->modelindex].skins[0];
	}

#if 0
	if(item->animations[0])
	{
		CG_RunLerpFrame(&cent->pe.flag, cg_items[es->modelindex].animations[0], es->an MAX_FLAG_ANIMATIONS, FLAG_IDLE, 1);

		memcpy(&flag.skeleton, &cent->pe.flag.skeleton, sizeof(refSkeleton_t));

		// transform relative bones to absolute ones required for vertex skinning
		CG_TransformSkeleton(&flag.skeleton, NULL);
	}
#endif

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);

	if(item->giType == IT_WEAPON && wi->barrelModel)
	{
		refEntity_t     barrel;

		memset(&barrel, 0, sizeof(barrel));

		barrel.hModel = wi->barrelModel;

		VectorCopy(ent.lightingOrigin, barrel.lightingOrigin);
		barrel.shadowPlane = ent.shadowPlane;
		barrel.renderfx = ent.renderfx;

		CG_PositionRotatedEntityOnTag(&barrel, &ent, wi->weaponModel, "tag_barrel");

		AxisCopy(ent.axis, barrel.axis);
		barrel.nonNormalizedAxes = ent.nonNormalizedAxes;

		trap_R_AddRefEntityToScene(&barrel);
	}

	// accompanying rings / spheres for powerups
	if(!cg_simpleItems.integer)
	{
		vec3_t          spinAngles;

		VectorClear(spinAngles);

		if(item->giType == IT_HEALTH || item->giType == IT_POWERUP)
		{
			if((ent.hModel = cg_items[es->modelindex].models[1]) != 0)
			{
				if(item->giType == IT_POWERUP)
				{
					ent.origin[2] += 12;
					spinAngles[1] = (cg.time & 1023) * 360 / -1024.0f;
				}
				AnglesToAxis(spinAngles, ent.axis);

				// scale up if respawning
				if(frac != 1.0)
				{
					VectorScale(ent.axis[0], frac, ent.axis[0]);
					VectorScale(ent.axis[1], frac, ent.axis[1]);
					VectorScale(ent.axis[2], frac, ent.axis[2]);
					ent.nonNormalizedAxes = qtrue;

					// don't cast shadows in this time period
					ent.renderfx |= RF_NOSHADOW;
				}
				trap_R_AddRefEntityToScene(&ent);
			}
		}
	}
}

//============================================================================

/*
===============
CG_Projectile
===============
*/
static void CG_Projectile(centity_t * cent)
{
	refEntity_t     ent;
	entityState_t  *s1;
	const weaponInfo_t *weapon;

	//CG_Printf("CG_Projectile(entity = %i, weapon = %i)\n", cent->currentState.number, cent->currentState.weapon);

//  int col;

	s1 = &cent->currentState;
	if(s1->weapon > WP_NUM_WEAPONS)
	{
		s1->weapon = 0;
	}
	weapon = &cg_weapons[s1->weapon];

	// calculate the axis
	VectorCopy(s1->angles, cent->lerpAngles);

	//CG_Printf("CG_Projectile(lerpOrigin = [%i %i %i])\n", (int)cent->lerpOrigin[0], (int)cent->lerpOrigin[1], (int)cent->lerpOrigin[2]);
	//CG_Printf("CG_Projectile(pos.trDelta = [%i %i %i])\n", (int)s1->pos.trDelta[0], (int)s1->pos.trDelta[1], (int)s1->pos.trDelta[2]);

	// add trails
	if(weapon->projectileTrailFunc)
	{
		weapon->projectileTrailFunc(cent, weapon);
	}
/*
	if ( cent->currentState.modelindex == TEAM_RED ) {
		col = 1;
	}
	else if ( cent->currentState.modelindex == TEAM_BLUE ) {
		col = 2;
	}
	else {
		col = 0;
	}

	// add dynamic light
	if ( weapon->projectileLight ) {
		trap_R_AddLightToScene(cent->lerpOrigin, weapon->projectileLight,
			weapon->projectileLightColor[col][0], weapon->projectileLightColor[col][1], weapon->projectileLightColor[col][2] );
	}
*/
	// add dynamic light
	if(weapon->projectileLight)
	{
		trap_R_AddLightToScene(cent->lerpOrigin, weapon->projectileLight,
							   weapon->projectileLightColor[0], weapon->projectileLightColor[1], weapon->projectileLightColor[2]);
	}

	// add missile sound
	if(weapon->projectileSound)
	{
		vec3_t          velocity;

		BG_EvaluateTrajectoryDelta(&cent->currentState.pos, cg.time, velocity);

		trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, velocity, weapon->projectileSound);
	}

	// create the render entity
	memset(&ent, 0, sizeof(ent));
	VectorCopy(cent->lerpOrigin, ent.origin);
	VectorCopy(cent->lerpOrigin, ent.oldorigin);

	// flicker between two skins
	ent.skinNum = cg.clientFrame & 1;
	ent.hModel = weapon->projectileModel;
	ent.renderfx = weapon->projectileRenderfx | RF_NOSHADOW;

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
===============
CG_Projectile2
===============
*/
static void CG_Projectile2(centity_t * cent)
{
	refEntity_t     ent;
	entityState_t  *s1;
	const weaponInfo_t *weapon;

//  int col;

	s1 = &cent->currentState;
	if(s1->weapon > WP_NUM_WEAPONS)
	{
		s1->weapon = 0;
	}
	weapon = &cg_weapons[s1->weapon];

	// calculate the axis
	VectorCopy(s1->angles, cent->lerpAngles);

	// add trails
	if(weapon->projectileTrailFunc2)
	{
		weapon->projectileTrailFunc2(cent, weapon);
	}
/*
	if ( cent->currentState.modelindex == TEAM_RED ) {
		col = 1;
	}
	else if ( cent->currentState.modelindex == TEAM_BLUE ) {
		col = 2;
	}
	else {
		col = 0;
	}

	// add dynamic light
	if ( weapon->projectileLight ) {
		trap_R_AddLightToScene(cent->lerpOrigin, weapon->projectileLight,
			weapon->projectileLightColor[col][0], weapon->projectileLightColor[col][1], weapon->projectileLightColor[col][2] );
	}
*/
	// add dynamic light
	if(weapon->projectileLight2)
	{
		trap_R_AddLightToScene(cent->lerpOrigin, weapon->projectileLight,
							   weapon->projectileLightColor2[0], weapon->projectileLightColor2[1],
							   weapon->projectileLightColor2[2]);
	}

	// add missile sound
	if(weapon->projectileSound2)
	{
		vec3_t          velocity;

		BG_EvaluateTrajectoryDelta(&cent->currentState.pos, cg.time, velocity);

		trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, velocity, weapon->projectileSound2);
	}

	// create the render entity
	memset(&ent, 0, sizeof(ent));
	VectorCopy(cent->lerpOrigin, ent.origin);
	VectorCopy(cent->lerpOrigin, ent.oldorigin);

	// flicker between two skins
	ent.skinNum = cg.clientFrame & 1;
	ent.hModel = weapon->projectileModel2;
	ent.renderfx = weapon->projectileRenderfx2 | RF_NOSHADOW;

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
#ifdef MISSIONPACK
		if(s1->weapon == WP_PROX_LAUNCHER)
		{
			AnglesToAxis(cent->lerpAngles, ent.axis);
		}
		else
#endif
		{
			RotateAroundDirection(ent.axis, s1->time);
		}
	}

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);
}

/*
===============
CG_Grapple

This is called when the grapple is sitting up against the wall
===============
*/
static void CG_Grapple(centity_t * cent)
{
	refEntity_t     ent;
	entityState_t  *s1;
	const weaponInfo_t *weapon;
	matrix_t        rotation;

	s1 = &cent->currentState;
	if(s1->weapon > WP_NUM_WEAPONS)
	{
		s1->weapon = 0;
	}
	weapon = &cg_weapons[s1->weapon];

	// calculate the axis
	VectorCopy(s1->angles, cent->lerpAngles);

#if 0							// FIXME add grapple pull sound here..?
	// add missile sound
	if(weapon->projectileSound)
	{
		trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->projectileSound);
	}
#endif

	// Will draw cable if needed
	CG_GrappleTrail(cent, weapon);

	// create the render entity
	memset(&ent, 0, sizeof(ent));
	VectorCopy(cent->lerpOrigin, ent.origin);
	VectorCopy(cent->lerpOrigin, ent.oldorigin);

	// flicker between two skins
	ent.skinNum = cg.clientFrame & 1;
	ent.hModel = weapon->projectileModel;
	ent.renderfx = weapon->projectileRenderfx | RF_NOSHADOW;

	// convert direction of travel into axis
	if(VectorNormalize2(s1->pos.trDelta, ent.axis[0]) == 0)
	{
		ent.axis[0][2] = 1;
	}

	RotateAroundDirection(ent.axis, cg.time);

	MatrixFromVectorsFLU(rotation, ent.axis[0], ent.axis[1], ent.axis[2]);
	MatrixMultiplyRotation(rotation, 0, 90, 0);
	MatrixToVectorsFLU(rotation, ent.axis[0], ent.axis[1], ent.axis[2]);

	trap_R_AddRefEntityToScene(&ent);
}

/*
===============
CG_Mover
===============
*/
static void CG_Mover(centity_t * cent)
{
	refEntity_t     ent;
	entityState_t  *s1;

	s1 = &cent->currentState;

	// create the render entity
	memset(&ent, 0, sizeof(ent));
	VectorCopy(cent->lerpOrigin, ent.origin);
	VectorCopy(cent->lerpOrigin, ent.oldorigin);
	AnglesToAxis(cent->lerpAngles, ent.axis);

	// Tr3B - let movers cast shadows
//  ent.renderfx = RF_NOSHADOW;

	// flicker between two skins (FIXME?)
	ent.skinNum = (cg.time >> 6) & 1;

	// get the model, either as a bmodel or a modelindex
	if(s1->solid == SOLID_BMODEL)
	{
		ent.hModel = cgs.inlineDrawModel[s1->modelindex];
	}
	else
	{
		ent.hModel = cgs.gameModels[s1->modelindex];
	}

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);

	// add the secondary model
	if(s1->modelindex2)
	{
		ent.skinNum = 0;
		ent.hModel = cgs.gameModels[s1->modelindex2];
		trap_R_AddRefEntityToScene(&ent);
	}
}

/*
===============
CG_Beam

Also called as an event
===============
*/
void CG_Beam(centity_t * cent)
{
#if 0
	refEntity_t     ent;
	entityState_t  *s1;

	//CG_Printf("CG_Beam()\n");

	s1 = &cent->currentState;

	// create the render entity
	memset(&ent, 0, sizeof(ent));
	VectorCopy(s1->pos.trBase, ent.origin);
	VectorCopy(s1->origin2, ent.oldorigin);
	AxisClear(ent.axis);

	ent.reType = RT_BEAM;
	ent.customShader = cgs.media.lightningShader;

	ent.renderfx = RF_NOSHADOW;

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);
#else
	refEntity_t     beam;
	entityState_t  *s1;

	//CG_Printf("CG_Beam()\n");

	s1 = &cent->currentState;

	memset(&beam, 0, sizeof(beam));

	VectorCopy(s1->pos.trBase, beam.origin);
	VectorCopy(s1->origin2, beam.oldorigin);

	beam.reType = RT_LIGHTNING;
	beam.customShader = cgs.media.lightningShader;
	trap_R_AddRefEntityToScene(&beam);
#endif
}


/*
===============
CG_Portal
===============
*/
static void CG_Portal(centity_t * cent)
{
	refEntity_t     ent;
	entityState_t  *s1;

	s1 = &cent->currentState;

	// create the render entity
	memset(&ent, 0, sizeof(ent));
	VectorCopy(cent->lerpOrigin, ent.origin);
	VectorCopy(s1->origin2, ent.oldorigin);
	ByteToDir(s1->eventParm, ent.axis[0]);
	PerpendicularVector(ent.axis[1], ent.axis[0]);

	// negating this tends to get the directions like they want
	// we really should have a camera roll value
	VectorSubtract(vec3_origin, ent.axis[1], ent.axis[1]);

	CrossProduct(ent.axis[0], ent.axis[1], ent.axis[2]);
	ent.reType = RT_PORTALSURFACE;
	ent.oldframe = s1->powerups;
	ent.frame = s1->frame;		// rotation speed
	ent.skinNum = s1->clientNum / 256.0 * 360;	// roll offset

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);
}


/*
==================
CG_AI_Node
==================
*/
static void CG_AI_Node(centity_t * cent)
{
	refEntity_t     ent;
	vec3_t          origin, delta, dir, vec, up = { 0, 0, 1 };
	float           len;
	int             i, node, digits[10], numdigits, negative;
	int             numberSize = 8;

	entityState_t  *s1;

	s1 = &cent->currentState;

	memset(&ent, 0, sizeof(ent));

#if 0

	// set frame
	VectorCopy(cent->lerpOrigin, ent.origin);
	VectorCopy(cent->lerpOrigin, ent.oldorigin);

	// convert angles to axis
	AnglesToAxis(cent->lerpAngles, ent.axis);

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);

#else
	// draw node number as sprite
	// code based on CG_AddScorePlum

	ent.reType = RT_SPRITE;
	ent.radius = 5;

	ent.shaderRGBA[0] = 0xff;
	ent.shaderRGBA[1] = 0xff;
	ent.shaderRGBA[2] = 0xff;
	ent.shaderRGBA[3] = 0xff;

	VectorCopy(cent->lerpOrigin, origin);
	origin[2] += 5;

	VectorSubtract(cg.refdef.vieworg, origin, dir);
	CrossProduct(dir, up, vec);
	VectorNormalize(vec);

	//VectorMA(origin, -10 + 20 * sin(c * 2 * M_PI), vec, origin);

	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw
	VectorSubtract(origin, cg.refdef.vieworg, delta);
	len = VectorLength(delta);
	if(len < 20)
	{
		return;
	}

	node = s1->otherEntityNum;

	negative = qfalse;
	if(node < 0)
	{
		negative = qtrue;
		node = -node;
	}

	for(numdigits = 0; !(numdigits && !node); numdigits++)
	{
		digits[numdigits] = node % 10;
		node = node / 10;
	}

	if(negative)
	{
		digits[numdigits] = 10;
		numdigits++;
	}

	for(i = 0; i < numdigits; i++)
	{
		VectorMA(origin, (float)(((float)numdigits / 2) - i) * numberSize, vec, ent.origin);
		ent.customShader = cgs.media.numberShaders[digits[numdigits - 1 - i]];
		trap_R_AddRefEntityToScene(&ent);
	}
#endif
}

/*
===============
CG_AI_Link
===============
*/
static void CG_AI_Link(centity_t * cent)
{
	refEntity_t     beam;
	entityState_t  *s1;

	s1 = &cent->currentState;

	memset(&beam, 0, sizeof(beam));

	VectorCopy(s1->pos.trBase, beam.origin);
	VectorCopy(s1->origin2, beam.oldorigin);

	//beam.reType = RT_BEAM;
	beam.reType = RT_LIGHTNING;
	beam.customShader = cgs.media.lightningShader;
	trap_R_AddRefEntityToScene(&beam);
}

/*
==================
CG_Physics_Box
==================
*/
#define ACTIVE_TAG 1
#define ISLAND_SLEEPING 2
#define WANTS_DEACTIVATION 3
#define DISABLE_DEACTIVATION 4
#define DISABLE_SIMULATION 5

static void CG_Physics_Box(centity_t * cent)
{
	polyVert_t      verts[4];
	int             i;
	vec3_t          mins;
	vec3_t          maxs;
	float           extx, exty, extz;
	vec3_t          corners[8];
	matrix_t        rotation;
	matrix_t        transform;

	{
		int             x, zd, zu;

		// otherwise grab the encoded bounding box
		x = (cent->currentState.solid & 255);
		zd = ((cent->currentState.solid >> 8) & 255);
		zu = ((cent->currentState.solid >> 16) & 255) - 32;

		mins[0] = mins[1] = -x;
		maxs[0] = maxs[1] = x;
		mins[2] = -zd;
		maxs[2] = zu;
	}

	// get the extents (size)
	extx = maxs[0] - mins[0];
	exty = maxs[1] - mins[1];
	extz = maxs[2] - mins[2];

	// set the polygon's texture coordinates
	verts[0].st[0] = 0;
	verts[0].st[1] = 0;
	verts[1].st[0] = 0;
	verts[1].st[1] = 1;
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[3].st[0] = 1;
	verts[3].st[1] = 0;

	// set the polygon's vertex colors

	switch (cent->currentState.generic1)
	{
		case ACTIVE_TAG:
			for(i = 0; i < 4; i++)
			{
				verts[i].modulate[0] = 255;
				verts[i].modulate[1] = 255;
				verts[i].modulate[2] = 255;
				verts[i].modulate[3] = 255;
			}
			break;

		case ISLAND_SLEEPING:
			for(i = 0; i < 4; i++)
			{
				verts[i].modulate[0] = 0;
				verts[i].modulate[1] = 255;
				verts[i].modulate[2] = 0;
				verts[i].modulate[3] = 255;
			}
			break;

		case WANTS_DEACTIVATION:
			for(i = 0; i < 4; i++)
			{
				verts[i].modulate[0] = 0;
				verts[i].modulate[1] = 255;
				verts[i].modulate[2] = 255;
				verts[i].modulate[3] = 255;
			}
			break;

		case DISABLE_DEACTIVATION:
			for(i = 0; i < 4; i++)
			{
				verts[i].modulate[0] = 255;
				verts[i].modulate[1] = 0;
				verts[i].modulate[2] = 0;
				verts[i].modulate[3] = 255;
			}
			break;

		case DISABLE_SIMULATION:
			for(i = 0; i < 4; i++)
			{
				verts[i].modulate[0] = 255;
				verts[i].modulate[1] = 255;
				verts[i].modulate[2] = 0;
				verts[i].modulate[3] = 255;
			}
			break;

		default:
			for(i = 0; i < 4; i++)
			{
				verts[i].modulate[0] = 255;
				verts[i].modulate[1] = 0;
				verts[i].modulate[2] = 0;
				verts[i].modulate[3] = 255;
			}
			break;
	}

	MatrixFromAngles(rotation, cent->lerpAngles[PITCH], cent->lerpAngles[YAW], cent->lerpAngles[ROLL]);
	MatrixSetupTransformFromRotation(transform, rotation, cent->lerpOrigin);

	//VectorAdd(cent->lerpOrigin, maxs, corners[3]);
	VectorCopy(maxs, corners[3]);

	VectorCopy(corners[3], corners[2]);
	corners[2][0] -= extx;

	VectorCopy(corners[2], corners[1]);
	corners[1][1] -= exty;

	VectorCopy(corners[1], corners[0]);
	corners[0][0] += extx;

	for(i = 0; i < 4; i++)
	{
		VectorCopy(corners[i], corners[i + 4]);
		corners[i + 4][2] -= extz;
	}

	for(i = 0; i < 8; i++)
	{
		MatrixTransformPoint2(transform, corners[i]);
	}

	// top
	VectorCopy(corners[0], verts[0].xyz);
	VectorCopy(corners[1], verts[1].xyz);
	VectorCopy(corners[2], verts[2].xyz);
	VectorCopy(corners[3], verts[3].xyz);
	trap_R_AddPolyToScene(cgs.media.debugPlayerAABB, 4, verts);

	// bottom
	VectorCopy(corners[7], verts[0].xyz);
	VectorCopy(corners[6], verts[1].xyz);
	VectorCopy(corners[5], verts[2].xyz);
	VectorCopy(corners[4], verts[3].xyz);
	trap_R_AddPolyToScene(cgs.media.debugPlayerAABB, 4, verts);

	// top side
	VectorCopy(corners[3], verts[0].xyz);
	VectorCopy(corners[2], verts[1].xyz);
	VectorCopy(corners[6], verts[2].xyz);
	VectorCopy(corners[7], verts[3].xyz);
	trap_R_AddPolyToScene(cgs.media.debugPlayerAABB_twoSided, 4, verts);

	// left side
	VectorCopy(corners[2], verts[0].xyz);
	VectorCopy(corners[1], verts[1].xyz);
	VectorCopy(corners[5], verts[2].xyz);
	VectorCopy(corners[6], verts[3].xyz);
	trap_R_AddPolyToScene(cgs.media.debugPlayerAABB_twoSided, 4, verts);

	// right side
	VectorCopy(corners[0], verts[0].xyz);
	VectorCopy(corners[3], verts[1].xyz);
	VectorCopy(corners[7], verts[2].xyz);
	VectorCopy(corners[4], verts[3].xyz);
	trap_R_AddPolyToScene(cgs.media.debugPlayerAABB_twoSided, 4, verts);

	// bottom side
	VectorCopy(corners[1], verts[0].xyz);
	VectorCopy(corners[0], verts[1].xyz);
	VectorCopy(corners[4], verts[2].xyz);
	VectorCopy(corners[5], verts[3].xyz);
	trap_R_AddPolyToScene(cgs.media.debugPlayerAABB_twoSided, 4, verts);
}

/*
===============
JUHOX: CG_DrawLineSegment
===============
*/
float CG_DrawLineSegment(const vec3_t start, const vec3_t end,
						 float totalLength, float segmentSize, float scrollspeed, qhandle_t shader)
{
	float           frac;
	refEntity_t     ent;

	frac = totalLength / segmentSize;
	frac -= (int)frac;

	memset(&ent, 0, sizeof(ent));
	ent.reType = RT_LIGHTNING;
	ent.customShader = shader;
	VectorCopy(start, ent.origin);
	VectorCopy(end, ent.oldorigin);

	ent.shaderTime = frac / -scrollspeed;
	trap_R_AddRefEntityToScene(&ent);

	return totalLength + Distance(start, end);
}

/*
=========================
CG_AdjustPositionForMover

Also called by client movement prediction code
=========================
*/
void CG_AdjustPositionForMover(const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out)
{
	centity_t      *cent;
	vec3_t          oldOrigin, origin, deltaOrigin;
	vec3_t          oldAngles, angles, deltaAngles;

	if(moverNum <= 0 || moverNum >= ENTITYNUM_MAX_NORMAL)
	{
		VectorCopy(in, out);
		return;
	}

	cent = &cg_entities[moverNum];
	if(cent->currentState.eType != ET_MOVER)
	{
		VectorCopy(in, out);
		return;
	}

	BG_EvaluateTrajectory(&cent->currentState.pos, fromTime, oldOrigin);
	BG_EvaluateTrajectory(&cent->currentState.apos, fromTime, oldAngles);

	BG_EvaluateTrajectory(&cent->currentState.pos, toTime, origin);
	BG_EvaluateTrajectory(&cent->currentState.apos, toTime, angles);

	VectorSubtract(origin, oldOrigin, deltaOrigin);
	VectorSubtract(angles, oldAngles, deltaAngles);

	VectorAdd(in, deltaOrigin, out);

	// FIXME: origin change when on a rotating object
}


/*
=============================
CG_InterpolateEntityPosition
=============================
*/
static void CG_InterpolateEntityPosition(centity_t * cent)
{
	vec3_t          current, next;
	float           f;

	// it would be an internal error to find an entity that interpolates without
	// a snapshot ahead of the current one
	if(cg.nextSnap == NULL)
	{
		CG_Error("CG_InterpoateEntityPosition: cg.nextSnap == NULL");
	}

	f = cg.frameInterpolation;

	// this will linearize a sine or parabolic curve, but it is important
	// to not extrapolate player positions if more recent data is available
	BG_EvaluateTrajectory(&cent->currentState.pos, cg.snap->serverTime, current);
	BG_EvaluateTrajectory(&cent->nextState.pos, cg.nextSnap->serverTime, next);

	cent->lerpOrigin[0] = current[0] + f * (next[0] - current[0]);
	cent->lerpOrigin[1] = current[1] + f * (next[1] - current[1]);
	cent->lerpOrigin[2] = current[2] + f * (next[2] - current[2]);

	BG_EvaluateTrajectory(&cent->currentState.apos, cg.snap->serverTime, current);
	BG_EvaluateTrajectory(&cent->nextState.apos, cg.nextSnap->serverTime, next);

	cent->lerpAngles[0] = LerpAngle(current[0], next[0], f);
	cent->lerpAngles[1] = LerpAngle(current[1], next[1], f);
	cent->lerpAngles[2] = LerpAngle(current[2], next[2], f);

}

/*
===============
CG_CalcEntityLerpPositions
===============
*/
static void CG_CalcEntityLerpPositions(centity_t * cent)
{
//unlagged - projectile nudge
	// this will be set to how far forward projectiles will be extrapolated
	int             timeshift = 0;

//unlagged - projectile nudge

//unlagged - smooth clients #2
	// this is done server-side now - cg_smoothClients is undefined
	// players will always be TR_INTERPOLATE
/*
	// if this player does not want to see extrapolated players
	if(!cg_smoothClients.integer)
	{
		// make sure the clients use TR_INTERPOLATE
		if(cent->currentState.number < MAX_CLIENTS)
		{
			cent->currentState.pos.trType = TR_INTERPOLATE;
			cent->nextState.pos.trType = TR_INTERPOLATE;
		}
	}
*/
//unlagged - smooth clients #2

	if(cent->interpolate && cent->currentState.pos.trType == TR_INTERPOLATE)
	{
		CG_InterpolateEntityPosition(cent);
		return;
	}

	// first see if we can interpolate between two snaps for
	// linear extrapolated clients
	if(cent->interpolate && cent->currentState.pos.trType == TR_LINEAR_STOP && cent->currentState.number < MAX_CLIENTS)
	{
		CG_InterpolateEntityPosition(cent);
		return;
	}

//unlagged - timenudge extrapolation
	// interpolating failed (probably no nextSnap), so extrapolate
	// this can also happen if the teleport bit is flipped, but that
	// won't be noticeable
	if(cent->currentState.number < MAX_CLIENTS && cent->currentState.clientNum != cg.predictedPlayerState.clientNum)
	{
		cent->currentState.pos.trType = TR_LINEAR_STOP;
		cent->currentState.pos.trTime = cg.snap->serverTime;
		cent->currentState.pos.trDuration = 1000 / sv_fps.integer;
	}
//unlagged - timenudge extrapolation

//unlagged - projectile nudge
	// if it's a missile but not a grappling hook
	if((cent->currentState.eType == ET_PROJECTILE || cent->currentState.eType == ET_PROJECTILE2) && cent->currentState.weapon != WP_GAUNTLET)
	{
		// if it's one of ours
		if(cent->currentState.otherEntityNum == cg.clientNum)
		{
			// extrapolate one server frame's worth - this will correct for tiny
			// visual inconsistencies introduced by backward-reconciling all players
			// one server frame before running projectiles
			timeshift = 1000 / sv_fps.integer;
		}
#if 0
		// if it's not, and it's not a grenade launcher
		else if(cent->currentState.weapon != WP_FLAK_CANNON)
		{
			// extrapolate based on cg_projectileNudge
			timeshift = cg_projectileNudge.integer + 1000 / sv_fps.integer;
		}
#endif
	}

	// just use the current frame and evaluate as best we can
//  BG_EvaluateTrajectory( &cent->currentState.pos, cg.time, cent->lerpOrigin );
//  BG_EvaluateTrajectory( &cent->currentState.apos, cg.time, cent->lerpAngles );
	BG_EvaluateTrajectory(&cent->currentState.pos, cg.time + timeshift, cent->lerpOrigin);
	BG_EvaluateTrajectory(&cent->currentState.apos, cg.time + timeshift, cent->lerpAngles);

	// if there's a time shift
	if(timeshift != 0)
	{
		trace_t         tr;
		vec3_t          lastOrigin;

		BG_EvaluateTrajectory(&cent->currentState.pos, cg.time, lastOrigin);

		CG_Trace(&tr, lastOrigin, vec3_origin, vec3_origin, cent->lerpOrigin, cent->currentState.number, MASK_SHOT);

		// don't let the projectile go through the floor
		if(tr.fraction < 1.0f)
		{
			cent->lerpOrigin[0] = lastOrigin[0] + tr.fraction * (cent->lerpOrigin[0] - lastOrigin[0]);
			cent->lerpOrigin[1] = lastOrigin[1] + tr.fraction * (cent->lerpOrigin[1] - lastOrigin[1]);
			cent->lerpOrigin[2] = lastOrigin[2] + tr.fraction * (cent->lerpOrigin[2] - lastOrigin[2]);
		}
	}
//unlagged - projectile nudge


	// adjust for riding a mover if it wasn't rolled into the predicted
	// player state
	if(cent != &cg.predictedPlayerEntity)
	{
		CG_AdjustPositionForMover(cent->lerpOrigin, cent->currentState.groundEntityNum,
								  cg.snap->serverTime, cg.time, cent->lerpOrigin);
	}
}

/*
===============
CG_TeamBase
===============
*/
static void CG_TeamBase(centity_t * cent)
{
	refEntity_t     model;
	vec3_t          angles;
	int             t, h;
	float           c;

	if(cgs.gametype == GT_CTF || cgs.gametype == GT_1FCTF)
	{
		// show the flag base
		memset(&model, 0, sizeof(model));
		model.reType = RT_MODEL;
		VectorCopy(cent->lerpOrigin, model.lightingOrigin);
		VectorCopy(cent->lerpOrigin, model.origin);
		AnglesToAxis(cent->currentState.angles, model.axis);
		if(cent->currentState.modelindex == TEAM_RED)
		{
			model.hModel = cgs.media.redFlagBaseModel;
		}
		else if(cent->currentState.modelindex == TEAM_BLUE)
		{
			model.hModel = cgs.media.blueFlagBaseModel;
		}
		else
		{
			model.hModel = cgs.media.neutralFlagBaseModel;
		}
		trap_R_AddRefEntityToScene(&model);
	}
	else if(cgs.gametype == GT_OBELISK)
	{
		// show the obelisk
		memset(&model, 0, sizeof(model));
		model.reType = RT_MODEL;
		VectorCopy(cent->lerpOrigin, model.lightingOrigin);
		VectorCopy(cent->lerpOrigin, model.origin);
		AnglesToAxis(cent->currentState.angles, model.axis);

		model.hModel = cgs.media.overloadBaseModel;

		trap_R_AddRefEntityToScene(&model);
		// if hit
		if(cent->currentState.frame == 1)
		{
			// show hit model
			// modelindex2 is the health value of the obelisk
			c = cent->currentState.modelindex2;
			model.shaderRGBA[0] = 0xff;
			model.shaderRGBA[1] = c;
			model.shaderRGBA[2] = c;
			model.shaderRGBA[3] = 0xff;
			//
			model.hModel = cgs.media.overloadEnergyModel;
			trap_R_AddRefEntityToScene(&model);
		}
		// if respawning
		if(cent->currentState.frame == 2)
		{
			if(!cent->miscTime)
			{
				cent->miscTime = cg.time;
			}
			t = cg.time - cent->miscTime;
			h = (cg_obeliskRespawnDelay.integer - 5) * 1000;
			//
			if(t > h)
			{
				c = (float)(t - h) / h;
				if(c > 1)
					c = 1;
			}
			else
			{
				c = 0;
			}
			// show the lights
			AnglesToAxis(cent->currentState.angles, model.axis);
			//
			model.shaderRGBA[0] = c * 0xff;
			model.shaderRGBA[1] = c * 0xff;
			model.shaderRGBA[2] = c * 0xff;
			model.shaderRGBA[3] = c * 0xff;

			model.hModel = cgs.media.overloadLightsModel;
			trap_R_AddRefEntityToScene(&model);
			// show the target
			if(t > h)
			{
				if(!cent->muzzleFlashTime)
				{
					trap_S_StartSound(cent->lerpOrigin, ENTITYNUM_NONE, CHAN_BODY, cgs.media.obeliskRespawnSound);
					cent->muzzleFlashTime = 1;
				}
				VectorCopy(cent->currentState.angles, angles);
				angles[YAW] += (float)16 *acos(1 - c) * 180 / M_PI;

				AnglesToAxis(angles, model.axis);

				VectorScale(model.axis[0], c, model.axis[0]);
				VectorScale(model.axis[1], c, model.axis[1]);
				VectorScale(model.axis[2], c, model.axis[2]);

				model.shaderRGBA[0] = 0xff;
				model.shaderRGBA[1] = 0xff;
				model.shaderRGBA[2] = 0xff;
				model.shaderRGBA[3] = 0xff;
				//
				model.origin[2] += 56;
				model.hModel = cgs.media.overloadTargetModel;
				trap_R_AddRefEntityToScene(&model);
			}
			else
			{
				//FIXME: show animated smoke
			}
		}
		else
		{
			cent->miscTime = 0;
			cent->muzzleFlashTime = 0;
			// modelindex2 is the health value of the obelisk
			c = cent->currentState.modelindex2;
			model.shaderRGBA[0] = 0xff;
			model.shaderRGBA[1] = c;
			model.shaderRGBA[2] = c;
			model.shaderRGBA[3] = 0xff;
			// show the lights
			model.hModel = cgs.media.overloadLightsModel;
			trap_R_AddRefEntityToScene(&model);
			// show the target
			model.origin[2] += 56;
			model.hModel = cgs.media.overloadTargetModel;
			trap_R_AddRefEntityToScene(&model);
		}
	}
	else if(cgs.gametype == GT_HARVESTER)
	{
		// show harvester model
		memset(&model, 0, sizeof(model));
		model.reType = RT_MODEL;
		VectorCopy(cent->lerpOrigin, model.lightingOrigin);
		VectorCopy(cent->lerpOrigin, model.origin);
		AnglesToAxis(cent->currentState.angles, model.axis);

		if(cent->currentState.modelindex == TEAM_RED)
		{
			model.hModel = cgs.media.harvesterModel;
			model.customSkin = cgs.media.harvesterRedSkin;
		}
		else if(cent->currentState.modelindex == TEAM_BLUE)
		{
			model.hModel = cgs.media.harvesterModel;
			model.customSkin = cgs.media.harvesterBlueSkin;
		}
		else
		{
			model.hModel = cgs.media.harvesterNeutralModel;
			model.customSkin = 0;
		}
		trap_R_AddRefEntityToScene(&model);
	}
}

/*
===============
CG_AddCEntity
===============
*/
static void CG_AddCEntity(centity_t * cent)
{
	// event-only entities will have been dealt with already
	if(cent->currentState.eType >= ET_EVENTS)
	{
		return;
	}

	// calculate the current origin
	CG_CalcEntityLerpPositions(cent);

	// add automatic effects
	CG_EntityEffects(cent);

	switch (cent->currentState.eType)
	{
		default:
			CG_Error("Bad entity type: %i\n", cent->currentState.eType);
			break;
		case ET_INVISIBLE:
		case ET_PUSH_TRIGGER:
		case ET_TELEPORT_TRIGGER:
			break;
		case ET_GENERAL:
			CG_General(cent);
			break;
		case ET_PLAYER:
			CG_Player(cent);
			break;
		case ET_ITEM:
			CG_Item(cent);
			break;
		case ET_PROJECTILE:
			CG_Projectile(cent);
			break;
		case ET_PROJECTILE2:
			CG_Projectile2(cent);
			break;
		case ET_MOVER:
			CG_Mover(cent);
			break;
		case ET_BEAM:
			CG_Beam(cent);
			break;
		case ET_PORTAL:
			CG_Portal(cent);
			break;
		case ET_SPEAKER:
			CG_Speaker(cent);
			break;
		case ET_GRAPPLE:
			CG_Grapple(cent);
			break;
		case ET_TEAM:
			CG_TeamBase(cent);
			break;
		case ET_AI_NODE:
			CG_AI_Node(cent);
			break;
		case ET_AI_LINK:
			CG_AI_Link(cent);
			break;
		case ET_EXPLOSIVE:
			CG_Mover(cent);
			break;
		case ET_FIRE:
			CG_Fire(cent);
			break;
		case ET_PHYSICS_BOX:
			CG_Physics_Box(cent);
			break;

			//WEAVER
		case ET_WEAVE_HELD:
			//weave is being held;
			CG_HeldWeave(cent);
			break;
		case ET_WEAVE_LINK:
			//link between players
			CG_WeaveEffect_Link(cent);
			break;
		case ET_WEAVE_EFFECT:
			//actual result of a weave
			CG_WeaveEffect(cent);
			break;
		case ET_WEAVE_MISSILE:
			//actual result of a weave
			CG_WeaveEffect(cent);
			break;
		case ET_WEAVE_THREADS:
			//weave being created by player
			CG_ShowThreads(cent);
			break;
		case ET_SHIELD:
			//shielding a func_exlosive or something
			CG_Mover(cent);
			break;
		case ET_SHIELD_INFO:
			//info for shield
			CG_ShieldInfo(cent);
			break;
	}
}

/*
===============
CG_AddPacketEntities
===============
*/
void CG_AddPacketEntities(void)
{
	int             num;
	centity_t      *cent;
	playerState_t  *ps;

	// set cg.frameInterpolation
	if(cg.nextSnap)
	{
		int             delta;

		delta = (cg.nextSnap->serverTime - cg.snap->serverTime);
		if(delta == 0)
		{
			cg.frameInterpolation = 0;
		}
		else
		{
			cg.frameInterpolation = (float)(cg.time - cg.snap->serverTime) / delta;
		}
	}
	else
	{
		cg.frameInterpolation = 0;	// actually, it should never be used, because
		// no entities should be marked as interpolating
	}

	// the auto-rotating items will all have the same axis
	cg.autoAngles[0] = 0;
	cg.autoAngles[1] = (cg.time & 2047) * 360 / 2048.0;
	cg.autoAngles[2] = 0;

	cg.autoAnglesFast[0] = 0;
	cg.autoAnglesFast[1] = (cg.time & 1023) * 360 / 1024.0f;
	cg.autoAnglesFast[2] = 0;

	AnglesToAxis(cg.autoAngles, cg.autoAxis);
	AnglesToAxis(cg.autoAnglesFast, cg.autoAxisFast);

	// generate and add the entity from the playerstate
	ps = &cg.predictedPlayerState;
	BG_PlayerStateToEntityState(ps, &cg.predictedPlayerEntity.currentState, qfalse);
	CG_AddCEntity(&cg.predictedPlayerEntity);

	// lerp the non-predicted value for lightning gun origins
	CG_CalcEntityLerpPositions(&cg_entities[cg.snap->ps.clientNum]);

//unlagged - early transitioning
	if(cg.nextSnap)
	{
		// pre-add some of the entities sent over by the server
		// we have data for them and they don't need to interpolate
		for(num = 0; num < cg.nextSnap->numEntities; num++)
		{
			cent = &cg_entities[cg.nextSnap->entities[num].number];
			if(cent->nextState.eType == ET_PROJECTILE || cent->nextState.eType == ET_PROJECTILE2 || cent->nextState.eType == ET_GENERAL)
			{
				// transition it immediately and add it
				CG_TransitionEntity(cent);
				cent->interpolate = qtrue;
				CG_AddCEntity(cent);
			}
		}
	}
//unlagged - early transitioning

	// add each entity sent over by the server
	for(num = 0; num < cg.snap->numEntities; num++)
	{
		cent = &cg_entities[cg.snap->entities[num].number];

//unlagged - early transitioning
		if(!cg.nextSnap || (cent->nextState.eType != ET_PROJECTILE && cent->nextState.eType != ET_PROJECTILE2 && cent->nextState.eType != ET_GENERAL))
		{
//unlagged - early transitioning
		CG_AddCEntity(cent);
		}
	}
}

/*
===============
CG_UniqueNoShadowID
===============
*/
int CG_UniqueNoShadowID(void)
{
	static int      noShadowID = 1;

	noShadowID++;

	if(!noShadowID)
	{
		noShadowID = 1;
	}

	return noShadowID;
}
