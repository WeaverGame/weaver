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

// g_spell_misc.c -- misc weaver functions
#include "g_local.h"

/*
=================
WeaveProtectCheck

Checks if player's Protects are expended
=================
*/
void WeaveProtectCheck(gclient_t * checkClient)
{
	DEBUGWEAVEING("WeaveProtectCheck: start");
	if(checkClient->ps.stats[STAT_FIREPROTECT] <= 0)
	{
		if(checkClient->protectHeldFire)
		{
			if(!HeldWeaveBelongsToPlayer(checkClient->protectHeldFire, &checkClient->ps)) G_Error("Bad protect fire\n");
			G_ReleaseWeave(checkClient->protectHeldFire->target_ent);
		}
	}
	if(checkClient->ps.stats[STAT_AIRPROTECT] <= 0)
	{
		if(checkClient->protectHeldAir)
		{
			if(!HeldWeaveBelongsToPlayer(checkClient->protectHeldAir, &checkClient->ps)) G_Error("Bad protect air\n");
			G_ReleaseWeave(checkClient->protectHeldAir->target_ent);
		}
	}
	if(checkClient->ps.stats[STAT_EARTHPROTECT] <= 0)
	{
		if(checkClient->protectHeldEarth)
		{
			if(!HeldWeaveBelongsToPlayer(checkClient->protectHeldEarth, &checkClient->ps)) G_Error("Bad protect earth\n");
			G_ReleaseWeave(checkClient->protectHeldEarth->target_ent);
		}
	}
	if(checkClient->ps.stats[STAT_WATERPROTECT] <= 0)
	{
		if(checkClient->protectHeldWater)
		{
			if(!HeldWeaveBelongsToPlayer(checkClient->protectHeldWater, &checkClient->ps)) G_Error("Bad protect water\n");
			G_ReleaseWeave(checkClient->protectHeldWater->target_ent);
		}
	}
	DEBUGWEAVEING("WeaveProtectCheck: end");
}

#define CHECK_PROTECT_ELEMENT(STAT, PROTECT) \
		do { \
		if(client->ps.stats[(STAT)] >= damage) \
		{ \
			client->ps.stats[(STAT)] -= damage; \
			damage -= (damage * (PROTECT)); \
		} \
		else if(client->ps.stats[(STAT)] > 0) \
		{ \
			damage -= (client->ps.stats[(STAT)] * (PROTECT)); \
			client->ps.stats[(STAT)] = 0; \
		} \
		} while(0);

int WeaveProtectDamage(gentity_t * targ, gclient_t *client, gentity_t * inflictor, gentity_t * attacker,
				  const vec3_t dir, const vec3_t point, int damageBase, int dflags, int mod)
{
	int             damage;

	//WEAVER
	float           airprotect;
	float           fireprotect;
	float           earthprotect;
	float           waterprotect;

	damage = damageBase;

	//Weaver
	//reduce damage if target client has a relevant protect
	//first check if the inflictor is a weave or weave effect
	//or a held weave - some cases like fire darts never actually spawn an effect ent
	if(client && (inflictor->s.eType == ET_WEAVE_EFFECT || inflictor->s.eType == ET_WEAVE_MISSILE || inflictor->s.eType == ET_WEAVE_HELD))
	{
		//Get protection factor for this weave
		//Note, air shield is prioritized? checks first? is this good?
		WeaveProtectScales(inflictor->s.weapon, &airprotect, &fireprotect, &earthprotect, &waterprotect);

		// Apply protection for each element.
		CHECK_PROTECT_ELEMENT(STAT_AIRPROTECT, airprotect);
		CHECK_PROTECT_ELEMENT(STAT_FIREPROTECT, fireprotect);
		CHECK_PROTECT_ELEMENT(STAT_EARTHPROTECT, earthprotect);
		CHECK_PROTECT_ELEMENT(STAT_WATERPROTECT, waterprotect);

		WeaveProtectCheck(client);
	}

	return damage;
}

int PowerEncode(int threads[MAX_THREADS], int offset, int count)
{
	int             n = 0;
	int             i = 0;

	for(i = 0; i < count; i++)
	{
		//Com_Printf("n=%d thread=%d mult=%d add=%d\n", n, threads[offset+i], (int)pow(WVP_NUMBER, i), (threads[offset+i] * (int)pow(WVP_NUMBER, i)));
		n += (threads[offset + i] * (int)pow(WVP_NUMBER, i));
	}
	return n;
}
