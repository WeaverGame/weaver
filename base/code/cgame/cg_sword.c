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

// cg_sword.c -- the sword
#include "cg_local.h"

/*
=============
CG_AddPlayerSword

Used for both the view weapon (ps is valid) and the world model other character models (ps is NULL)
The main player will have this called for BOTH cases, so effects like light and
sound should only be done on the world model case.
=============
*/
void CG_AddPlayerSword(refEntity_t * parent, playerState_t * ps, centity_t * cent, int team)
{
	refEntity_t     sword;
	int             boneIndex;
	weapon_t        weaponNum;
	centity_t      *nonPredictedCent;

	weaponNum = cent->currentState.weapon;

	// add the weapon
	memset(&sword, 0, sizeof(sword));
	VectorCopy(parent->lightingOrigin, sword.lightingOrigin);
	sword.shadowPlane = parent->shadowPlane;
	sword.renderfx = parent->renderfx;
	sword.noShadowID = parent->noShadowID;

	sword.hModel = cgs.media.swordModel;

	if(!sword.hModel)
	{
		return;
	}

#if 0
	if(!ps)
	{
		// add weapon ready sound
		cent->pe.lightningFiring = qfalse;
		if((cent->currentState.eFlags & (EF_FIRING | EF_FIRING2)) && weapon->firingSound)
		{
			// lightning sword and guantlet make a different sound when fire is held down
			trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->firingSound);
			cent->pe.lightningFiring = qtrue;
		}
		else if(weapon->readySound)
		{
			trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->readySound);
		}
	}
#endif

	if(cent->currentState.weapon == WP_GAUNTLET)
	{
		//Holding sword in hand
#ifdef XPPM
		if(ps)
		{
			CG_PositionEntityOnTag(&sword, parent, parent->hModel, "swordControl");
		}
		else
		{
			boneIndex = trap_R_BoneIndex(parent->hModel, "swordControl");
			if(boneIndex >= 0 && boneIndex < cent->pe.torso.skeleton.numBones)
			{
				AxisClear(sword.axis);
				CG_PositionRotatedEntityOnBone(&sword, parent, parent->hModel, "swordControl");
			}
			else
			{
				CG_Error("No tag found while adding sword.");
			}
		}
#else
		CG_PositionEntityOnTag(&sword, parent, parent->hModel, "tag_weapon");
#endif
	}
	else
	{
		//Sword is in sheath
#ifdef XPPM
		if(ps)
		{
			CG_PositionEntityOnTag(&sword, parent, parent->hModel, "sheatheControl");
		}
		else
		{
			boneIndex = trap_R_BoneIndex(parent->hModel, "sheatheControl");
			if(boneIndex >= 0 && boneIndex < cent->pe.torso.skeleton.numBones)
			{
				AxisClear(sword.axis);
				CG_PositionRotatedEntityOnBone(&sword, parent, parent->hModel, "sheatheControl");
			}
			else
			{
				CG_Error("No tag found while adding sword.");
			}
		}
#else
		CG_PositionEntityOnTag(&sword, parent, parent->hModel, "tag_weapon");
#endif
	}

	trap_R_AddRefEntityToScene(&sword);

	// make sure we aren't looking at cg.predictedPlayerEntity for LG
	nonPredictedCent = &cg_entities[cent->currentState.clientNum];

	// if the index of the nonPredictedCent is not the same as the clientNum
	// then this is a fake player (like on teh single player podiums), so
	// go ahead and use the cent
	if((nonPredictedCent - cg_entities) != cent->currentState.clientNum)
	{
		nonPredictedCent = cent;
	}
}

