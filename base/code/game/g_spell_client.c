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

// g_spell_client.c -- misc weaver functions for dealing with clients and magic
#include <q_shared.h>
#include "g_local.h"

/*
=======================
Release all the spells, in any state, belonging to this client.
=======================
*/
void ClientWeaverCleanupSpells(gclient_t * client)
{
	int             i;

	DEBUGWEAVEING("ClientWeaverCleanupSpells: start");

	// Cleanup client's held weaves
	for(i = MIN_WEAPON_WEAVE; i < MAX_WEAPONS; i++)
	{
		if(client->ps.ammo[i] > 0)
		{
			HeldWeaveEnd(&g_entities[client->ps.ammo[i]]);
		}
	}

	DEBUGWEAVEING("ClientWeaverCleanupSpells: end");
}

/*
=======================
Create threadsEnt for this client.
=======================
*/
void ClientWeaverCreateThreads(gclient_t * client)
{
	gentity_t      *threadsEnt;
	gentity_t      *clientEnt;

	if(!client)
	{
		DEBUGWEAVEING("ClientWeaverCreateThreads: no client");
		return;
	}

	DEBUGWEAVEING("ClientWeaverCreateThreads: start");

	clientEnt = &g_entities[client->ps.clientNum];

	threadsEnt = G_Spawn();

	threadsEnt->classname = THREAD_CLASSNAME;
	threadsEnt->nextthink = 0;
	threadsEnt->parent = clientEnt;
	threadsEnt->r.ownerNum = client->ps.clientNum;
	threadsEnt->r.svFlags = SVF_BROADCAST;
	threadsEnt->s.eType = ET_WEAVE_THREADS;
	threadsEnt->s.otherEntityNum2 = client->ps.clientNum;
	//set owner
	threadsEnt->s.torsoAnim = client->ps.clientNum;

	if(DEBUGWEAVEING_TST(1))
	{
		Com_Printf("Making Player ThreadsEnt for %d, svflags=%d threadsEnt=%d\n",
				threadsEnt->s.torsoAnim, threadsEnt->r.svFlags, threadsEnt->s.number);
	}

	//Weave group & status: threadsEnt->s.frame;
	//8 threads, 4 in each, see PowerEncode(): threadsEnt->s.constantLight;
	//finished thread num; threadsEnt->s.weapon;

	threadsEnt->damage = 0;
	threadsEnt->splashDamage = 0;
	threadsEnt->splashRadius = 0;
	threadsEnt->methodOfDeath = 0;
	threadsEnt->splashMethodOfDeath = 0;
	threadsEnt->clipmask = 0;
	threadsEnt->target_ent = 0;
	threadsEnt->takedamage = qfalse;

	client->threadEnt = threadsEnt;

	trap_LinkEntity(threadsEnt);

	DEBUGWEAVEING("ClientWeaverCreateThreads: end");

	return;
}

/*
=======================
Initialize this client's facility for weaving.

Applied when a client is spawned or revived.
=======================
*/
void ClientWeaverInitialize(gclient_t * client)
{
	DEBUGWEAVEING("ClientWeaverInitialize: start");

	// Give power
	ClientPowerInitialize(client);

	// Clear Links
	client->linkEnt = NULL;
	client->linkFollower = NULL;
	client->linkHeld = NULL;
	client->linkTarget = NULL;

	// Clear protects
	client->ps.stats[STAT_AIRPROTECT] = 0;
	client->ps.stats[STAT_FIREPROTECT] = 0;
	client->ps.stats[STAT_EARTHPROTECT] = 0;
	client->ps.stats[STAT_WATERPROTECT] = 0;
	WeaveProtectCheck(client);

	// create threads entity
	ClientWeaverCreateThreads(client);

	DEBUGWEAVEING("ClientWeaverInitialize: end");
}

/*
=======================
Destroy this client's facility for weaving.

Applied when a client is killed or disconnects.
May be applied multiple times (i.e. dead/wounded player who then disconnects).
=======================
*/
void ClientWeaverDestroy(gclient_t * client)
{
	DEBUGWEAVEING("ClientWeaverDestroy: start");

	// Leave any links
	ClientLinkLeave(client);

	// Release all spells
	ClientWeaverCleanupSpells(client);

	// Cleanup client's threads
	if(client->threadEnt)
	{
		if(DEBUGWEAVEING_TST(1))
		{
			Com_Printf("Free ThreadsEnt player %d\n", client->ps.clientNum);
		}
		G_FreeEntity(client->threadEnt);
		client->threadEnt = NULL;
	}

	// Block weaving on this player to terminate any weaving thats going on.
	client->ps.powerups[PW_SHIELDED] = level.time + WEAVE_SHIELD_PULSE_TIME;

	DEBUGWEAVEING("ClientWeaverDestroy: end");
}

/*
=======================
Weaver weaving events
=======================
*/
void ClientWeaveStart(gclient_t * client)
{
	int             j;

	client->weaving = qtrue;

	client->threading = qfalse;
	client->thread = 0;

	// Set if its aggressive or defensive
	if(client->ps.eFlags & EF_WEAVEA)
	{
		client->currentWeaveGroup = WVG_AGRESSIVE;
	}
	else if(client->ps.eFlags & EF_WEAVED)
	{
		client->currentWeaveGroup = WVG_DEFENSIVE;
	}

	// Clear threads
	for(j = 0; j < MAX_THREADS; j++)
	{
		client->currentWeaveThreads[j] = WVP_NONE;
	}
}

void ClientThreadStart(gclient_t * client)
{
	if(!client)
	{
		return;
	}

	// But only if player has enough available power
	if(ClientPowerAvailable(client) >= POWER_PER_THREAD)
	{
		client->threading = qtrue;
	}
}

void ClientThreadEnd(gclient_t * client)
{
	float           angle;
	float           distance;
	weaver_powers   threadPower;

	if(!client)
	{
		return;
	}

	client->threading = qfalse;
	// Cartesian to Polar
	angle    = (atan2f(client->ps.stats[STAT_THREADY], client->ps.stats[STAT_THREADX]) * (180 / M_PI));
	distance = sqrt((client->ps.stats[STAT_THREADX] * client->ps.stats[STAT_THREADX]) + (client->ps.stats[STAT_THREADY] * client->ps.stats[STAT_THREADY]));

	if(distance > WEAVER_CENTER)
	{
		// Add half a power div so the bottom of fire (-half a power div) is on 0, and normalize angle
		angle = AngleNormalize360(angle + WEAVER_POWERDIV * 0.5);
		if(angle < WEAVER_POWERDIV * 4)
		{
			if(angle < WEAVER_POWERDIV * 2)
			{
				if(angle < WEAVER_POWERDIV * 1)
				{
					threadPower = WVP_FIRE;
				}
				else
				{
					threadPower = WVP_AIRFIRE;
				}
			}
			else
			{
				if(angle < WEAVER_POWERDIV * 3)
				{
					threadPower = WVP_AIR;
				}
				else
				{
					threadPower = WVP_AIRWATER;
				}
			}
		}
		else
		{
			if(angle < WEAVER_POWERDIV * 6)
			{
				if(angle < WEAVER_POWERDIV * 5)
				{
					threadPower = WVP_WATER;
				}
				else
				{
					threadPower = WVP_EARTHWATER;
				}
			}
			else
			{
				if(angle < WEAVER_POWERDIV * 7)
				{
					threadPower = WVP_EARTH;
				}
				else
				{
					threadPower = WVP_EARTHFIRE;
				}
			}
		}
	}
	else
	{
		threadPower = WVP_SPIRIT;
	}

	client->currentWeaveThreads[client->thread] = threadPower;

	ClientPowerConsume(client, POWER_PER_THREAD);

	client->thread++;
}

void ClientWeaveEnd(gclient_t * client, gentity_t * ent)
{
	int             i;
	int             powerUsing;
	int             weaveID;

	client->weaving = qfalse;
	if(client->thread > 0)
	{
		if(DEBUGWEAVEING_TST(1))
		{
			Com_Printf("POWER: %i WEAVE: %s_%s_%s_%s_%s_%s_%s_%s_%s\n",
					   ClientPowerAvailable(client),
					   WeaveGroupNames[client->currentWeaveGroup],
					   WeavePowerNames[client->currentWeaveThreads[0]],
					   WeavePowerNames[client->currentWeaveThreads[1]],
					   WeavePowerNames[client->currentWeaveThreads[2]],
					   WeavePowerNames[client->currentWeaveThreads[3]],
					   WeavePowerNames[client->currentWeaveThreads[4]],
					   WeavePowerNames[client->currentWeaveThreads[5]],
					   WeavePowerNames[client->currentWeaveThreads[6]],
					   WeavePowerNames[client->currentWeaveThreads[7]]);
		}

		//count up ammount of power being used
		powerUsing = 0;
		for(i = 0; i < MAX_THREADS; i++)
		{
			if(client->currentWeaveThreads[i] > 0)
			{
				powerUsing += POWER_PER_THREAD;
			}
			else
			{
				break;
			}
		}

		weaveID = -1;
		weaveID = ThreadsToWeaveID(client->currentWeaveGroup, client->currentWeaveThreads);
		CreateWeaveID(ent, weaveID, powerUsing);
	}
	client->ps.stats[STAT_THREADX] = 0;
	client->ps.stats[STAT_THREADY] = 0;

	for(i = 0; i < MAX_THREADS; i++)
	{
		client->currentWeaveThreads[i] = 0;
	}
}

void ClientWeaverReleaseCostly(gclient_t * client) {
	int             i;
	gentity_t      *heldWeave;

	DEBUGWEAVEING("ClientWeaverReleaseCostly: start");

	// Simple: Remove the first one we find
	for(i = MIN_WEAPON_WEAVE; i < MAX_WEAPONS; i++)
	{
		if(client->ps.ammo[i] > 0)
		{
			heldWeave = &g_entities[client->ps.ammo[i]];
			if(DEBUGWEAVEING_TST(1))
			{
				Com_Printf("ClientWeaverReleaseCostly: remove heldweave ent=%d weapon=%d\n", client->ps.ammo[i], heldWeave->s.weapon);
			}
			HeldWeaveEnd(heldWeave);
			break;
		}
	}

	DEBUGWEAVEING("ClientWeaverReleaseCostly: end");
}

void ClientWeaveUpdateStats(gentity_t * ent, gclient_t * client)
{
	// weaving stats
	client->ps.stats[STAT_POWER] = ClientPowerAvailable(client);
	client->ps.stats[STAT_MAX_POWER] = ClientPowerMax(client);

	// Check if the player no longer has enough power for all their held spells
	// If so, release spells until they have enough power to sustain them all
	while(client->ps.stats[STAT_POWER] < 0)
	{
		ClientWeaverReleaseCostly(client);
		client->ps.stats[STAT_POWER] = ClientPowerAvailable(client);
	}

	if(client->ps.eFlags & EF_WEAVEA || client->ps.eFlags & EF_WEAVED)
	{
		if(!client->weaving)
		{
			// start a weave
			ClientWeaveStart(client);
		}
		if(client->ps.eFlags & EF_THREAD)
		{
			if(!client->threading)
			{
				// start a thread
				ClientThreadStart(client);
			}
		}
		else
		{
			if(client->threading)
			{
				// end a thread
				ClientThreadEnd(client);
			}
			client->ps.stats[STAT_THREADX] = 0;
			client->ps.stats[STAT_THREADY] = 0;
		}
	}
	else
	{
		if(client->weaving)
		{
			//end a weave
			ClientWeaveEnd(client, ent);
		}
		else
		{
			//player is not weaving
			client->ps.stats[STAT_THREADX] = 0;
			client->ps.stats[STAT_THREADY] = 0;
			ClientPowerRelease(client);
		}
	}
}

void ClientPoisonUpdateStats(gentity_t *ent)
{
	if(ent->client->ps.powerups[PW_SLOWPOISONED])
	{
		ent->client->slowTicks--;

		if(ent->client->slowTicks <= 0)
		{
			G_Damage(ent, ent->client->slowAttacker, ent->client->slowAttacker, NULL, NULL, WEAVE_SLOWPOISON_DPS, 0, MOD_A_EARTHWATER_SLOW);
			ent->client->slowTicks = WEAVE_SLOWPOISON_TICKS;
		}
	}
	if(ent->client->ps.powerups[PW_POISONED])
	{
		ent->client->poisonTicks--;

		if(ent->client->poisonTicks <= 0)
		{
			G_Damage(ent, ent->client->poisonAttacker, ent->client->poisonAttacker, NULL, NULL, ent->client->poisonDamage, 0, MOD_A_EARTHWATER_POISON);
			ent->client->poisonTicks = WEAVE_POISON_TICKS;
			// Damage increases
			ent->client->poisonDamage += WEAVE_POISON_DMGINCREMENT;
			if(ent->client->poisonDamage > WEAVE_POISON_MAXDMG)
			{
				ent->client->poisonDamage = WEAVE_POISON_MAXDMG;
			}
		}
	}
}

/*
=================
ClientWeaveHeldCount

Returns number of weaves being held by a given client
=================
*/
int ClientWeaveHeldCount(gentity_t * holdingClient)
{
	int             heldCount;
	int             i;

	DEBUGWEAVEING("ClientWeaveHeldCount: start");

	heldCount = 0;

	for(i = MIN_WEAPON_WEAVE; i < MAX_WEAPONS; i++)
	{
		if(holdingClient->client->ps.ammo[i] > 0)
		{
			heldCount++;
		}
	}
	DEBUGWEAVEING("ClientWeaveHeldCount: end");
	return heldCount;
}
