/*
===========================================================================
Copyright (C) 2012 Andrew Browne <dersaidin@dersaidin.net>

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

// cg_spell_hands.c -- display player's hands/sword/spells/etc. in first person
#include "cg_local.h"

/*
==============
CG_AddWeaverViewWeapon

Add the hands/sword/spells/etc. for the player's view
==============
*/
void CG_AddWeaverViewWeapon(playerState_t * ps)
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
				//CG_LightningBolt(nonPredictedCent, flashOrigin);

				// add rail trail
				//CG_SpawnRailTrail(cent, flashOrigin);

				// add light
				if(weapon->flashLightColor[0] || weapon->flashLightColor[1] || weapon->flashLightColor[2])
				{
					trap_R_AddLightToScene(flashOrigin, 300 + (rand() & 31), weapon->flashLightColor[0],
										   weapon->flashLightColor[1], weapon->flashLightColor[2]);
				}
			}
		}

		//CG_AddWeaponWithPowerups(&gun, cent->currentState.powerups);
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
