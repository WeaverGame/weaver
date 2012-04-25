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

// g_spell_weave.c -- functions for held weaves
#include "g_local.h"
#include "g_spell_effects.h"
#include "g_spell_util.h"

qboolean        HeldWeaveEffectExecute(gentity_t * heldWeave);
void            HeldWeaveEffectEnd(gentity_t * heldWeave);
void            HeldWeaveClearCast(gentity_t * heldWeave, int castClear);

/*
=================
HeldWeaveAddToPlayer

Adds a weave to a players held weaves list.

heldWeave is a ET_HELD_WEAVE
player is playerState of player to add it to
=================
*/
void HeldWeaveAddToPlayer(gentity_t * heldWeave, playerState_t * player)
{
	int             i;
	gentity_t      *pent;

	if(!heldWeave)
	{
		DEBUGWEAVEING("HeldWeaveAddToPlayer: no heldWeave");
		return;
	}
	if(!player)
	{
		DEBUGWEAVEING("HeldWeaveAddToPlayer: no player");
		return;
	}

	DEBUGWEAVEING("HeldWeaveAddToPlayer: start");

	assert(heldWeave->s.eType == ET_WEAVE_HELD && "HeldWeaveAddToPlayer: heldWeave not a ET_WEAVE_HELD");

	for(i = MIN_WEAPON_WEAVE; i < MAX_WEAPONS; i++)
	{
		if(player->ammo[i] <= 0)
		{
			player->ammo[i] = heldWeave->s.number;
			heldWeave->s.modelindex2 = i;	// remember slot
			player->weapon = i;
			player->stats[STAT_WEAPONS] |= 1 << i;
			// Event to indicate new weave
			pent = &g_entities[heldWeave->s.otherEntityNum2];
			G_AddEvent(pent, EV_WEAVE_ADDHELD, i);
			break;
		}
	}
	DEBUGWEAVEING("HeldWeaveAddToPlayer: end");
}

/*
=================
HeldWeaveBelongsToPlayer

Returns true if heldWeave has been added to player.

heldWeave is a ET_HELD_WEAVE
player is playerState of player to add it to
=================
*/
qboolean HeldWeaveBelongsToPlayer(gentity_t * heldWeave, playerState_t * player)
{
	// Held must be a valid entity
	if((!heldWeave) || (!heldWeave->inuse))
	{
		return qfalse;
	}
	// Held must be a heldweave entity
	if(heldWeave->s.eType != ET_WEAVE_HELD)
	{
		return qfalse;
	}
	// Must belong to the specific player
	if(heldWeave->s.otherEntityNum2 != player->clientNum)
	{
		return qfalse;
	}
	// Player should have the weave available in a slot
	if(player->ammo[heldWeave->s.modelindex2] != heldWeave->s.number)
	{
		return qfalse;
	}
	return qtrue;
}

/*
=================
HeldWeaveExpire

Expires a given heldWeave, causing it to fire immediately.

heldWeave is a ET_HELD_WEAVE
=================
*/
void HeldWeaveExpire(gentity_t * heldWeave)
{
	if(!heldWeave)
	{
		DEBUGWEAVEING("HeldWeaveExpire: no ent");
		return;
	}

	DEBUGWEAVEING("HeldWeaveExpire: start");

	assert(heldWeave->s.eType == ET_WEAVE_HELD && "HeldWeaveExpire: heldWeave not a ET_WEAVE_HELD");

	if(G_HeldWeave_GetState(heldWeave) == WST_HELD)
	{
		G_HeldWeave_SetState(heldWeave, WST_EXPIRED);
		HeldWeaveUse(heldWeave);
	}
	DEBUGWEAVEING("HeldWeaveExpire: end");
}

/*
=================
HeldWeaveCreate

Creates a held weave entity using parameters.
=================
*/
gentity_t      *HeldWeaveCreate(gentity_t * self, int weaveID, int holdTime, int holdPower, int switchable, qboolean runAtTime)
{
	gentity_t      *weave;

	DEBUGWEAVEING("HeldWeaveCreate: start");

	weave = G_Spawn();

	weave->classname = HELD_CLASSNAME;
	weave->nextthink = level.time + holdTime;
	if(runAtTime)
	{
		weave->think = HeldWeaveExpire;
	}
	else
	{
		weave->think = HeldWeaveClear;
	}
	weave->r.svFlags = SVF_BROADCAST;
	//weave->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	weave->r.ownerNum = self->s.number;

	VectorCopy(self->s.origin, weave->s.origin);
	VectorCopy(self->s.origin, weave->pos1);
	VectorCopy(self->s.origin, weave->s.pos.trBase);
	VectorCopy(self->s.origin, weave->r.currentOrigin);

	weave->s.eType = ET_WEAVE_HELD;
	//weave id
	weave->s.weapon = weaveID;
	//owner client num
	weave->s.otherEntityNum2 = self->s.number;
	//amount of power being used to hold this
	G_HeldWeave_SetPower(weave, holdPower);
	//1 is switchable
	weave->s.legsAnim = switchable;
	//charges
	G_HeldWeave_SetCharges(weave, WeaveCharges(weaveID));
	//weave state
	G_HeldWeave_SetState(weave, WST_HELD);

	weave->parent = self;

	//artifact params
	weave->damage = 0;
	weave->splashDamage = 0;
	weave->splashRadius = 0;
	weave->methodOfDeath = MOD_UNKNOWN;
	weave->target_ent = NULL;
	weave->clipmask = MASK_SOLID;

	//weave->splashMethodOfDeath = MOD_ROCKET_SPLASH;
	//unlagged - projectile nudge
	// we'll need this for nudging projectiles later
	weave->s.otherEntityNum = self->s.number;
	//unlagged - projectile nudge

	if(DEBUGWEAVEING_TST(1))
	{
		Com_Printf("Weave ent num = %i, holdPower=%d\n", weave->s.number, G_HeldWeave_GetPower(weave));
	}
	trap_LinkEntity(weave);

	DEBUGWEAVEING("HeldWeaveCreate: end");
	return weave;
}

/*
=================
CreateWeaveID

=================
*/
void CreateWeaveID(gentity_t * self, int weaveID, int powerUsing)
{
	gentity_t      *heldWeave = NULL;

	if(!self)
	{
		DEBUGWEAVEING("CreateWeaveID: no self");
		return;
	}

	DEBUGWEAVEING("CreateWeaveID: start");

	switch (weaveID)
	{
			/*begin end on exec */
		case WVW_A_AIRFIRE_SWORD:
			//give item
			heldWeave = HeldWeaveCreate(self, weaveID, TIME_TWO_MIN, powerUsing, 1, qtrue);
			break;
		case WVW_D_AIRFIRE_LIGHT:
		case WVW_D_AIRWATER_FOG:
			//spawn ent (lasting 2 mins max)
			heldWeave = HeldWeaveCreate(self, weaveID, TIME_TWO_MIN, powerUsing, 1, qtrue);
			break;
		case WVW_D_SPIRIT_TRAVEL:
			//spawn ent (lasting 20 sec max)
			heldWeave = HeldWeaveCreate(self, weaveID, TIME_TWENTY_SEC, powerUsing, 1, qtrue);
			break;
		case WVW_D_AIR_PROTECT:
		case WVW_D_FIRE_PROTECT:
		case WVW_D_EARTH_PROTECT:
		case WVW_D_WATER_PROTECT:
			//add powerup to self
			heldWeave = HeldWeaveCreate(self, weaveID, TIME_TWO_MIN, powerUsing, 1, qtrue);
			break;
			/*end end on exec */

			/*begin holdable, no switch */
		case WVW_D_SPIRIT_LINK:
		case WVW_D_AIRFIRE_WALL:
		case WVW_A_FIRE_BLOSSOMS:
		case WVW_D_EARTH_UNLOCK:
		case WVW_A_EARTHFIRE_IGNITE:
		case WVW_D_EARTHFIRE_EXPLOSIVE_S:
		case WVW_D_EARTHFIRE_EXPLOSIVE_M:
		case WVW_D_WATER_HEAL_S:
		case WVW_D_WATER_CURE:
		case WVW_D_WATER_HEAL_M:
		case WVW_D_SPIRIT_STAMINA:
		case WVW_D_AIRFIRE_INVIS:
		case WVW_A_SPIRIT_STILL:
		case WVW_A_SPIRIT_BALEFIRE:
		case WVW_A_SPIRIT_DEATHGATE:
			//add held weave
			heldWeave = HeldWeaveCreate(self, weaveID, TIME_THREE_SEC, powerUsing, 0, qfalse);
			break;
			/*end holdable, no switch */

			/*begin holdable, no switch, fire on expire */
		case WVW_A_SPIRIT_SLICE_S:
		case WVW_A_SPIRIT_SLICE_M:
		case WVW_A_SPIRIT_SLICE_L:
			//add held weave
			heldWeave = HeldWeaveCreate(self, weaveID, TIME_THREE_SEC, powerUsing, 0, qtrue);
			break;
			/*end holdable, no switch, fire on expire */

			/*begin holdable, switch */
		case WVW_D_AIR_GRAB:
		case WVW_A_AIR_BLAST:
		case WVW_A_AIR_GRABPLAYER:
		case WVW_A_AIR_BINDPLAYER:
		case WVW_A_EARTH_QUAKE_S:
		case WVW_A_EARTH_QUAKE_M:
		case WVW_A_EARTH_QUAKE_L:
		case WVW_A_FIRE_DARTS:
		case WVW_A_FIRE_BALL:
		case WVW_A_FIRE_MULTIDARTS:
		case WVW_A_AIRFIRE_LIGHTNING:
		case WVW_A_AIRWATER_DARTS_S:
		case WVW_A_AIRWATER_DARTS_M:
		case WVW_A_EARTHWATER_SLOW:
		case WVW_A_EARTHWATER_POISON:
		case WVW_A_WATER_ICESHARDS_S:
		case WVW_A_WATER_ICESHARDS_M:
			//add held weave
			heldWeave = HeldWeaveCreate(self, weaveID, TIME_TWO_MIN, powerUsing, 1, qfalse);
			break;
			// separate because it has a different hold time limit
		case WVW_A_AIRWATER_RIP:
			// add held weave
			heldWeave = HeldWeaveCreate(self, weaveID, TIME_THREE_SEC, powerUsing, 1, qfalse);
			break;
		case WVW_A_SPIRIT_SHIELD:
			//add held weave
			heldWeave = HeldWeaveCreate(self, weaveID, TIME_TWENTY_SEC, powerUsing, 1, qfalse);
			break;
			/*end holdable, switch */

			/*anything else is an error. */
		case WVW_NONE:
		case -1:
		default:
			break;
	}

	if(heldWeave != NULL)
	{
		HeldWeaveAddToPlayer(heldWeave, &self->client->ps);
	}

	DEBUGWEAVEING("CreateWeaveID: end");
	return;
}

/*
=================
HeldWeaveUse

Logic to see if a held weave can be used.
Calls ExecuteWeave() to run the weave.
Updates charges and power usage.

heldWeave is a ET_HELD_WEAVE
=================
*/
void HeldWeaveUse(gentity_t * heldWeave)
{
	float           newPower;
	int             maxCharges;
	int             cast = 0;

	if(!heldWeave)
	{
		DEBUGWEAVEING("HeldWeaveUse: no ent");
		return;
	}

	DEBUGWEAVEING("HeldWeaveUse: start");

	assert(heldWeave->s.eType == ET_WEAVE_HELD && "HeldWeaveUse: heldWeave not a ET_WEAVE_HELD");

	heldWeave->parent->client->ps.weaponTime = WeaveCastTime(heldWeave->s.weapon);

	if(DEBUGWEAVEING_TST(1))
	{
		Com_Printf("Using held weave %i charges %i of %i\n", heldWeave->s.number, G_HeldWeave_GetCharges(heldWeave),
				   WeaveCharges(heldWeave->s.weapon));
	}

	//check if weave is being held, held weaves will be ended, not execed
	if(G_HeldWeave_GetState(heldWeave) == WST_INPROCESS || G_HeldWeave_GetState(heldWeave) == WST_INPROCESSRELEASED)
	{
		//weave is being held, this use should end it
		HeldWeaveEffectEnd(heldWeave);
		//use a shot
		G_HeldWeave_SetCharges(heldWeave, G_HeldWeave_GetCharges(heldWeave)-1);
	}
	//if weave is being released
	else if(G_HeldWeave_GetState(heldWeave) == WST_RELEASED || G_HeldWeave_GetState(heldWeave) == WST_INPROCESSRELEASED)
	{
		//Do not fire, releasing
	}
	//if weave has charges
	else if(G_HeldWeave_GetCharges(heldWeave) > 0)
	{
		//execute the weave
		if(!HeldWeaveEffectExecute(heldWeave))
		{
			if(DEBUGWEAVEING_TST(1))
			{
				Com_Printf("Weave execution returned 0. WeaveID=%d\n", heldWeave->s.weapon);
			}
			// This attempt to cast failed (e.g. no target), do not delay next attempt.
			heldWeave->parent->client->ps.weaponTime = 10;
			return;
		}

		cast = heldWeave->s.weapon;

		//held weave now consumes less power    
		if(DEBUGWEAVEING_TST(1))
		{
			Com_Printf("HeldWeaveUse: previous power consumed = %d\n", G_HeldWeave_GetPower(heldWeave));
		}
		maxCharges = (WeaveCharges(heldWeave->s.weapon) >> 1) + 1;
		newPower = G_HeldWeave_GetPower(heldWeave);
		newPower /= (float)(G_HeldWeave_GetCharges(heldWeave) + maxCharges);

		//if weave does not go in process
		if(G_HeldWeave_GetState(heldWeave) != WST_INPROCESS)
		{
			//use a shot
			G_HeldWeave_SetCharges(heldWeave, G_HeldWeave_GetCharges(heldWeave)-1);
			if(DEBUGWEAVEING_TST(1))
			{
				Com_Printf("HeldWeaveUse: shot expended %d remain\n", G_HeldWeave_GetCharges(heldWeave));
			}
		}

		//set new power consumption
		newPower *= (float)(G_HeldWeave_GetCharges(heldWeave) + maxCharges);
		G_HeldWeave_SetPower(heldWeave, newPower);

		if(DEBUGWEAVEING_TST(1))
		{
			Com_Printf("HeldWeaveUse: new power consumed = %d\n", G_HeldWeave_GetPower(heldWeave));
		}
	}

	//if no shots remaining or expired
	if(G_HeldWeave_GetCharges(heldWeave) <= 0 || G_HeldWeave_GetState(heldWeave) == WST_EXPIRED ||
	   G_HeldWeave_GetState(heldWeave) == WST_RELEASED || G_HeldWeave_GetState(heldWeave) == WST_INPROCESSRELEASED)
	{
		//then the weave should be removed
		HeldWeaveClearCast(heldWeave, cast);
	}
	DEBUGWEAVEING("HeldWeaveUse: end");
}

/*
=================
HeldWeaveEnd

End a held weave, and all its effects in any state.

heldWeave is a ET_HELD_WEAVE
=================
*/
void HeldWeaveEnd(gentity_t * heldWeave)
{
	if(heldWeave == NULL)
	{
		return;
	}

	assert(heldWeave->s.eType == ET_WEAVE_HELD && "HeldWeaveEnd: heldWeave not a ET_WEAVE_HELD");

	switch (G_HeldWeave_GetState(heldWeave))
	{
		case WST_HELD:
		case WST_EXPIRED:
			G_HeldWeave_SetState(heldWeave, WST_RELEASED);
			break;
		case WST_INPROCESS:
			G_HeldWeave_SetState(heldWeave, WST_INPROCESSRELEASED);
			break;
		case WST_RELEASED:
		case WST_INPROCESSRELEASED:
		default:
			break;
	}
	HeldWeaveUse(heldWeave);
}

/*
=================
HeldWeaveEffectEnd

Ends a weave in process.
This should only be called if the weave is in process.
The spell specific EndWeave functions assume that their spell is inprocess.

This should probably only be called from UseHeldWeave.

heldWeave is a ET_HELD_WEAVE, it must be WST_INPROCESS or WST_INPROCESSRELEASED.
=================
*/
void HeldWeaveEffectEnd(gentity_t * heldWeave)
{
	vec3_t          start;
	vec3_t          direction;
	gentity_t      *player;
	int             heldWeaveNum;

	if(!heldWeave)
	{
		DEBUGWEAVEING("EndWeave: no ent");
		return;
	}

	DEBUGWEAVEING("EndWeave: start");

	assert(heldWeave->s.eType == ET_WEAVE_HELD && "HeldWeaveEffectEnd: heldWeave not a ET_WEAVE_HELD");
	assert((G_HeldWeave_GetState(heldWeave) == WST_INPROCESS || G_HeldWeave_GetState(heldWeave) == WST_INPROCESSRELEASED)
		&& "HeldWeaveEffectEnd: heldWeave not WST_INPROCESS or WST_INPROCESSRELEASED");

	//calc parameters
	heldWeaveNum = heldWeave->s.number;
	player = heldWeave->parent;
	if(player)
	{
		VectorCopy(player->s.pos.trBase, start);
		start[2] += player->client->ps.viewheight;
		AnglesToVector(player->client->ps.viewangles, direction);
	}
	else
	{
		DEBUGWEAVEING("EndWeave: no player");
	}

	switch (heldWeave->s.weapon)
	{
			//Held special
		case WVW_A_AIRFIRE_SWORD:
			break;
		case WVW_D_AIR_PROTECT:
			EndWeave_ProtectAir(player, start, direction, heldWeaveNum);
			break;
		case WVW_D_FIRE_PROTECT:
			EndWeave_ProtectFire(player, start, direction, heldWeaveNum);
			break;
		case WVW_D_EARTH_PROTECT:
			EndWeave_ProtectEarth(player, start, direction, heldWeaveNum);
			break;
		case WVW_D_WATER_PROTECT:
			EndWeave_ProtectWater(player, start, direction, heldWeaveNum);
			break;
		case WVW_D_AIRFIRE_LIGHT:
			EndWeave_LightSource(player, start, direction, heldWeaveNum);
			break;
		case WVW_D_AIRWATER_FOG:
			EndWeave_Fog(player, start, direction, heldWeaveNum);
			break;
		case WVW_D_SPIRIT_TRAVEL:
			break;
			//Spawn Ent
		case WVW_D_AIRFIRE_WALL:
			break;
			//Target
		case WVW_D_SPIRIT_LINK:
			EndWeave_Link(player, start, direction, heldWeaveNum);
			//return instead of break, heldweave is cleared when disconnecting from linktarget
			return;
		case WVW_A_FIRE_BLOSSOMS:
		case WVW_A_SPIRIT_DEATHGATE:
			break;
		case WVW_D_SPIRIT_STAMINA:
			EndWeave_Stamina(player, start, direction, heldWeaveNum);
			break;
		case WVW_D_AIRFIRE_INVIS:
			break;
		case WVW_D_AIR_GRAB:
			break;
		case WVW_A_AIR_GRABPLAYER:
			EndWeave_GrabPlayer(player, start, direction, heldWeaveNum);
			break;
		case WVW_A_AIR_BINDPLAYER:
			break;
		case WVW_A_SPIRIT_SHIELD:
			EndWeave_Shield(player, start, direction, heldWeaveNum);
			break;
		case WVW_A_AIRFIRE_LIGHTNING:
			EndWeave_Lightning(player, start, direction, heldWeaveNum);
			break;
		case WVW_D_WATER_HEAL_S:
		case WVW_D_WATER_HEAL_M:
			EndWeave_Heal(player, start, direction, heldWeaveNum);
			break;
		case WVW_D_EARTHFIRE_EXPLOSIVE_S:
		case WVW_D_EARTHFIRE_EXPLOSIVE_M:
			EndWeave_Explosive(player, start, direction, heldWeaveNum);
			break;
			//Never need to be ended explicitly
		case WVW_A_EARTHFIRE_IGNITE:
			break;
		case WVW_D_WATER_CURE:
		case WVW_D_EARTH_UNLOCK:
		case WVW_A_SPIRIT_STILL:
		case WVW_A_SPIRIT_SLICE_S:
		case WVW_A_SPIRIT_SLICE_M:
		case WVW_A_SPIRIT_SLICE_L:
		case WVW_A_SPIRIT_BALEFIRE:
		case WVW_A_AIR_BLAST:
			break;
		case WVW_A_EARTH_QUAKE_S:
		case WVW_A_EARTH_QUAKE_M:
		case WVW_A_EARTH_QUAKE_L:
			EndWeave_EarthQuake(player, start, direction, heldWeaveNum);
			break;
		case WVW_A_FIRE_DARTS:
		case WVW_A_FIRE_BALL:
		case WVW_A_FIRE_MULTIDARTS:
		case WVW_A_AIRWATER_DARTS_S:
		case WVW_A_AIRWATER_DARTS_M:
		case WVW_A_EARTHWATER_SLOW:
		case WVW_A_EARTHWATER_POISON:
			break;
			//Fail
		case WVW_NONE:
		case -1:
		default:
			break;
	}
	DEBUGWEAVEING("EndWeave: end");
}

/*
=================
HeldWeaveEffectExecute

Actually executes a weave, producing a weaveEffect for the held weave.

This should probably only be called from UseHeldWeave.

heldWeave is a ET_HELD_WEAVE, it must be WST_HELD or WST_EXPIRED.
=================
*/
qboolean HeldWeaveEffectExecute(gentity_t * heldWeave)
{
	vec3_t          start;
	vec3_t          direction;
	gentity_t      *player;
	int             heldWeaveNum;
	vec3_t          surfNormal;
	vec3_t          forward, right, up;
	qboolean        result = qfalse;

	if(!heldWeave)
	{
		DEBUGWEAVEING("ExecuteWeave: no ent");
		return qfalse;
	}

	DEBUGWEAVEING("ExecuteWeave: start");

	assert(heldWeave->s.eType == ET_WEAVE_HELD && "HeldWeaveEffectExecute: heldWeave not a ET_WEAVE_HELD");
	assert((G_HeldWeave_GetState(heldWeave) == WST_HELD || G_HeldWeave_GetState(heldWeave) == WST_EXPIRED)
		 && "HeldWeaveEffectExecute: heldWeave not WST_HELD or WST_EXPIRED");

	//calc parameters
	heldWeaveNum = heldWeave->s.number;
	player = heldWeave->parent;
	AnglesToVector(player->client->ps.viewangles, direction);

	// From CalcMuzzlePoint
	AngleVectors(player->client->ps.viewangles, forward, right, up);
	VectorSet(surfNormal, 0.0f, 0.0f, 1.0f);
	VectorMA(player->client->ps.origin, player->client->ps.viewheight, surfNormal, start);
	VectorMA(start, 14, forward, start);
	// HACK: correct forward vector for the projectile so it will fly towards the crosshair
	//VectorMA(start, 8192, forward, end);
	//VectorSubtract(end, start, forward);
	VectorNormalize(forward);
	// snap to integer coordinates for more efficient network bandwidth usage
	SnapVector(start);

	switch (heldWeave->s.weapon)
	{
			//Held special
		case WVW_A_AIRFIRE_LIGHTNING:	//target like arty
			result = FireWeave_Lightning(player, start, direction, heldWeaveNum);
			break;
		case WVW_A_AIRFIRE_SWORD:
			break;
		case WVW_D_AIR_PROTECT:
			result = FireWeave_ProtectAir(player, start, direction, heldWeaveNum);
			break;
		case WVW_D_FIRE_PROTECT:
			result = FireWeave_ProtectFire(player, start, direction, heldWeaveNum);
			break;
		case WVW_D_EARTH_PROTECT:
			result = FireWeave_ProtectEarth(player, start, direction, heldWeaveNum);
			break;
		case WVW_D_WATER_PROTECT:
			result = FireWeave_ProtectWater(player, start, direction, heldWeaveNum);
			break;
		case WVW_D_AIRFIRE_LIGHT:
			result = FireWeave_LightSource(player, start, direction, heldWeaveNum);
			break;
		case WVW_D_AIRWATER_FOG:
			result = FireWeave_Fog(player, start, direction, heldWeaveNum);
			break;
		case WVW_D_SPIRIT_TRAVEL:
			break;
			//Spawn Ent
		case WVW_D_AIRFIRE_WALL:
			break;
		case WVW_D_EARTHFIRE_EXPLOSIVE_S:
			result = FireWeave_ExplosiveSmall(player, start, direction, heldWeaveNum);
			break;
		case WVW_D_EARTHFIRE_EXPLOSIVE_M:
			result = FireWeave_ExplosiveMed(player, start, direction, heldWeaveNum);
			break;
			//Target
		case WVW_D_SPIRIT_LINK:
			result = FireWeave_Link(player, start, direction, heldWeaveNum);
			break;
		case WVW_A_FIRE_BLOSSOMS:
		case WVW_A_SPIRIT_DEATHGATE:
		case WVW_D_EARTH_UNLOCK:
		case WVW_A_EARTHFIRE_IGNITE:
			break;
		case WVW_D_WATER_HEAL_S:
			result = FireWeave_HealS(player, start, direction, heldWeaveNum);
			break;
		case WVW_D_WATER_HEAL_M:
			result = FireWeave_HealM(player, start, direction, heldWeaveNum);
			break;
		case WVW_D_WATER_CURE:
			result = FireWeave_Cure(player, start, direction, heldWeaveNum);
			break;
		case WVW_D_SPIRIT_STAMINA:
			result = FireWeave_Stamina(player, start, direction, heldWeaveNum);
			break;
		case WVW_D_AIRFIRE_INVIS:
		case WVW_A_SPIRIT_STILL:
		case WVW_D_AIR_GRAB:
			break;
		case WVW_A_AIR_GRABPLAYER:
			result = FireWeave_GrabPlayer(player, start, direction, heldWeaveNum);
			break;
		case WVW_A_AIR_BINDPLAYER:
			break;
		case WVW_A_SPIRIT_SHIELD:
			result = FireWeave_Shield(player, start, direction, heldWeaveNum);
			break;
		case WVW_A_AIRWATER_RIP:
			result = FireWeave_Rip(player, start, direction, heldWeaveNum);
			break;
			//Missiles
		case WVW_A_SPIRIT_SLICE_S:
			result = FireWeave_SliceS(player, start, direction, heldWeaveNum);
			break;
		case WVW_A_SPIRIT_SLICE_M:
			result = FireWeave_SliceM(player, start, direction, heldWeaveNum);
			break;
		case WVW_A_SPIRIT_SLICE_L:
			result = FireWeave_SliceL(player, start, direction, heldWeaveNum);
			break;
		case WVW_A_SPIRIT_BALEFIRE:
			break;
		case WVW_A_AIR_BLAST:
			result = FireWeave_AirBlast(player, start, direction, heldWeaveNum);
			break;
		case WVW_A_EARTH_QUAKE_S:
			result = FireWeave_EarthQuakeS(player, start, direction, heldWeaveNum);
			break;
		case WVW_A_EARTH_QUAKE_M:
			result = FireWeave_EarthQuakeM(player, start, direction, heldWeaveNum);
			break;
		case WVW_A_EARTH_QUAKE_L:
			result = FireWeave_EarthQuakeL(player, start, direction, heldWeaveNum);
			break;
		case WVW_A_FIRE_DARTS:
			result = FireWeave_FireDarts(player, start, direction, heldWeaveNum);
			break;
		case WVW_A_FIRE_BALL:
			result = FireWeave_Fireball(player, start, direction, heldWeaveNum);
			break;
		case WVW_A_FIRE_MULTIDARTS:
			result = FireWeave_FireMultiDarts(player, start, direction, heldWeaveNum);
			break;
		case WVW_A_WATER_ICESHARDS_S:
			result = FireWeave_IceShards(player, start, direction, heldWeaveNum);
			break;
		case WVW_A_WATER_ICESHARDS_M:
			result = FireWeave_IceMultiShards(player, start, direction, heldWeaveNum);
			break;
		case WVW_A_EARTHWATER_SLOW:
			result = FireWeave_Slow(player, start, direction, heldWeaveNum);
			break;
		case WVW_A_EARTHWATER_POISON:
			result = FireWeave_Poison(player, start, direction, heldWeaveNum);
			break;
		case WVW_A_AIRWATER_DARTS_S:
			result = FireWeave_WaterDarts(player, start, direction, heldWeaveNum);
			break;
		case WVW_A_AIRWATER_DARTS_M:
			result = FireWeave_WaterDartsM(player, start, direction, heldWeaveNum);
			break;
			//Fail
		case WVW_NONE:
		case -1:
		default:
			break;
	}

	if(result)
	{
		G_AddEvent(player, EV_WEAVE_CAST_EFFECT, heldWeave->s.weapon);
	}

	DEBUGWEAVEING("ExecuteWeave: end");
	return result;
}


/*
=================
HeldWeaveClearCast

Removes heldWeave from the player, optionally due to casting it.

If heldWeave is removed due to it being cast, castClear should be non zero (ie, not WVW_NONE).
A CAST + CLEAR event will be sent instead of a CLEAR event (which would overwrite a separate CAST).

Frees the weave_effect entity.
Frees the heldWeave.

heldWeave is a ET_HELD_WEAVE
castClear is a weaveId or WVW_NONE.
=================
*/
void HeldWeaveClearCast(gentity_t * heldWeave, int castClear)
{
	int             i;
	playerState_t  *player;
	gentity_t      *pent;

	if(!heldWeave)
	{
		DEBUGWEAVEING("HeldWeaveClearCast: no heldWeave");
		return;
	}

	DEBUGWEAVEING("HeldWeaveClearCast: start");

	assert(heldWeave->s.eType == ET_WEAVE_HELD && "HeldWeaveClearCast: heldWeave not a ET_WEAVE_HELD");

	//get player who is holding weave
	pent = &g_entities[heldWeave->s.otherEntityNum2];
	player = &pent->client->ps;

	// Weave being cleared no longer uses power
	ClientPowerHeldConsume(pent->client, -G_HeldWeave_GetPower(heldWeave));
	G_HeldWeave_SetPower(heldWeave, 0);

	for(i = MIN_WEAPON_WEAVE; i < MAX_WEAPONS; i++)
	{
		// Where the weave is being held
		if(player->ammo[i] == heldWeave->s.number)
		{
			// Weave is no longer held
			player->ammo[i] = 0;
			// Weapon slot is no longer available
			player->stats[STAT_WEAPONS] &= ~(1 << i);
			if(i == player->weapon)
			{
				if(castClear != WVW_NONE)
				{
					G_AddEvent(pent, EV_WEAVE_CASTCLEARED, castClear);
				}
				else
				{
					G_AddEvent(pent, EV_WEAVE_CLEAREDCHANGE, 0);
				}
			}
			break;
		}
	}

	if(heldWeave->target_ent)
	{
		G_FreeEntity(heldWeave->target_ent);
	}

	//clear the held weave
	G_FreeEntity(heldWeave);

	DEBUGWEAVEING("HeldWeaveClearCast: end");
}

/*
=================
HeldWeaveClear

Removes weave from the player
Frees the weave entity.

heldWeave is a ET_HELD_WEAVE
=================
*/
void HeldWeaveClear(gentity_t * heldWeave)
{
	HeldWeaveClearCast(heldWeave, WVP_NONE);
}

