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

void ThreadsThink(void)
{
	int             i;
	gentity_t      *player;
	gclient_t      *pclient;
	gentity_t      *ent;

	DEBUGWEAVEING_LVL("ThreadsThink: start", 2);

	player = &g_entities[0];
	for(i = 0; i < MAX_CLIENTS; i++, player++)
	{
		if(!player->inuse)
		{
			continue;
		}

		if(!player->client)
		{
			continue;
		}

		if(player->client->threadEnt)
		{
			pclient = player->client;
			ent = pclient->threadEnt;

			// Update location
			VectorCopy(player->r.currentOrigin, ent->s.origin);
			// Update thread info
			if(pclient->weaving)
			{
				ent->s.constantLight = PowerEncode(pclient->currentWeaveThreads, 0, 8);
			}
			else
			{
				ent->s.constantLight = 0;
			}

			// Copy Trajectory
			ent->s.pos.trAcceleration = player->s.pos.trAcceleration;
			VectorCopy(player->s.pos.trBase, ent->s.pos.trBase);
			VectorCopy(player->s.pos.trDelta, ent->s.pos.trDelta);
			ent->s.pos.trDuration = player->s.pos.trDuration;
			ent->s.pos.trTime = player->s.pos.trTime;
			ent->s.pos.trType = player->s.pos.trType;

			// Copy power in use
			ent->s.powerups = ClientPowerInUse(pclient);

			// Update status
			ent->s.frame = 0;
			if(pclient->weaving)
			{
				ent->s.frame |= 1 << 0;
			}
			if(pclient->threading)
			{
				ent->s.frame |= 1 << 1;
			}

			ent->s.frame += (pclient->currentWeaveGroup << 2);

			trap_LinkEntity(ent);

			ent = NULL;
		}
	}

	DEBUGWEAVEING_LVL("ThreadsThink: end", 2);
}

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
			G_WeaveEffectRelease(checkClient->protectHeldFire->target_ent);
		}
	}
	if(checkClient->ps.stats[STAT_AIRPROTECT] <= 0)
	{
		if(checkClient->protectHeldAir)
		{
			if(!HeldWeaveBelongsToPlayer(checkClient->protectHeldAir, &checkClient->ps)) G_Error("Bad protect air\n");
			G_WeaveEffectRelease(checkClient->protectHeldAir->target_ent);
		}
	}
	if(checkClient->ps.stats[STAT_EARTHPROTECT] <= 0)
	{
		if(checkClient->protectHeldEarth)
		{
			if(!HeldWeaveBelongsToPlayer(checkClient->protectHeldEarth, &checkClient->ps)) G_Error("Bad protect earth\n");
			G_WeaveEffectRelease(checkClient->protectHeldEarth->target_ent);
		}
	}
	if(checkClient->ps.stats[STAT_WATERPROTECT] <= 0)
	{
		if(checkClient->protectHeldWater)
		{
			if(!HeldWeaveBelongsToPlayer(checkClient->protectHeldWater, &checkClient->ps)) G_Error("Bad protect water\n");
			G_WeaveEffectRelease(checkClient->protectHeldWater->target_ent);
		}
	}
	DEBUGWEAVEING("WeaveProtectCheck: end");
}

static ID_INLINE int dmgScale(int stat, float protect, gclient_t *client, int damage)
{
	float protectc = (1.0f - protect);
	float fdamage = (float)damage;
	float fdamageprotect = fdamage * protect;
	float fdamageprotectc = fdamage * protectc;

	if(client->ps.stats[stat] > 0) {
		if(client->ps.stats[stat] >= ((int)fdamageprotect))
		{
			// Client will have protect remaining after absorbing part of this damage
			client->ps.stats[stat] -= (int)(fdamageprotect);
			damage = (int)(fdamageprotectc);
		}
		else
		{
			// This is going to use the rest of the protect
			damage = (int)(fdamageprotectc) + ((int)fdamageprotect - client->ps.stats[stat]);
			client->ps.stats[stat] = 0;
		}
	}
	else
	{
		// Client was not protected for this element
	}
	return damage;
}

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

		if(g_debugDamage.integer)
		{
			G_Printf("Protect scales for weaveId=%d: air=%f fire=%f earth=%f water=%f\n", inflictor->s.weapon, airprotect, fireprotect, earthprotect, waterprotect);
		}

		// Apply protection for each element.

		damage = dmgScale(STAT_AIRPROTECT, airprotect, client, damage);
		damage = dmgScale(STAT_FIREPROTECT, fireprotect, client, damage);
		damage = dmgScale(STAT_EARTHPROTECT, earthprotect, client, damage);
		damage = dmgScale(STAT_WATERPROTECT, waterprotect, client, damage);

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
