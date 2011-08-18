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

void ClientWeaverCleanup(gclient_t * client)
{
	int             i;

	// Cleanup client's held weaves
	for(i = MIN_WEAPON_WEAVE; i < MAX_WEAPONS; i++)
	{
		if(client->ps.ammo[i] > 0)
		{
			ClearHeldWeave(&g_entities[client->ps.ammo[i]]);
		}
	}
	// Cleanup client's threads
	if(client->threadEnt)
	{
		if(DEBUGWEAVEING_TST(1))
		{
			Com_Printf("Free ThreadsEnt player %d", client->ps.clientNum);
		}
		G_FreeEntity(client->threadEnt);
	}
}

void ClientWeaverInitialize(gclient_t * client)
{
	// Give stamina and power
	client->ps.stats[STAT_STAMINA] = MAX_STAMINA;
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
}

void ClientWeaverDie(gentity_t * self)
{
	int             j;
	gentity_t      *heldWeave;

	// Free this players threads
	if(self->client->threadEnt)
	{
		if(DEBUGWEAVEING_TST(1))
		{
			Com_Printf("Free ThreadsEnt player %d\n", self->client->ps.clientNum);
		}
		G_FreeEntity(self->client->threadEnt);
		self->client->threadEnt = NULL;
	}

	ClientLinkLeave(self->client);

	// Clear held weaves
	for(j = MIN_WEAPON_WEAVE; j < MAX_WEAPONS; j++)
	{
		// Where the weave is being held
		if(self->client->ps.ammo[j] > 0)
		{
			// Clear held weave
			heldWeave = &g_entities[self->client->ps.ammo[j]];
			EndWeave(heldWeave);
			ClearHeldWeave(heldWeave);
			break;
		}
	}
}

int ClientWeaverProtectDamage(gentity_t * targ, gclient_t *client, gentity_t * inflictor, gentity_t * attacker,
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
	if(client && (inflictor->s.eType == ET_WEAVE_EFFECT || inflictor->s.eType == ET_WEAVE_MISSILE))
	{
		//Get protection factor for this weave
		//Note, air shield is prioritized? checks first? is this good?
		WeaveProtectScales(inflictor->s.weapon, &airprotect, &fireprotect, &earthprotect, &waterprotect);
		if(client->ps.stats[STAT_AIRPROTECT] >= damage)
		{
			//Shield is sufficient to take the whole blow
			client->ps.stats[STAT_AIRPROTECT] -= damage;
			damage -= damage * airprotect;
		}
		else if(client->ps.stats[STAT_AIRPROTECT])
		{
			damage -= (client->ps.stats[STAT_AIRPROTECT] * airprotect);
			client->ps.stats[STAT_AIRPROTECT] = 0;
		}

		if(client->ps.stats[STAT_FIREPROTECT] >= damage)
		{
			client->ps.stats[STAT_FIREPROTECT] -= damage;
			damage -= damage * fireprotect;
		}
		else if(client->ps.stats[STAT_FIREPROTECT])
		{
			damage -= (client->ps.stats[STAT_FIREPROTECT] * fireprotect);
			client->ps.stats[STAT_FIREPROTECT] = 0;
		}

		if(client->ps.stats[STAT_EARTHPROTECT] >= damage)
		{
			client->ps.stats[STAT_EARTHPROTECT] -= damage;
			damage -= damage * earthprotect;
		}
		else if(client->ps.stats[STAT_EARTHPROTECT])
		{
			damage -= (client->ps.stats[STAT_EARTHPROTECT] * earthprotect);
			client->ps.stats[STAT_EARTHPROTECT] = 0;
		}

		if(client->ps.stats[STAT_WATERPROTECT] >= damage)
		{
			client->ps.stats[STAT_WATERPROTECT] -= damage;
			damage -= damage * waterprotect;
		}
		else if(client->ps.stats[STAT_WATERPROTECT])
		{
			damage -= (client->ps.stats[STAT_WATERPROTECT] * waterprotect);
			client->ps.stats[STAT_WATERPROTECT] = 0;
		}

		WeaveProtectCheck(client);
	}

	return damage;
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

	client->weaving = qfalse;
	if(client->thread > 0)
	{
#if 0
		DEBUGWEAVEING(va("POWER: %i WEAVE: %s %s_%s_%s_%s_%s_%s_%s_%s_%s",
			ClientPowerAvailable(client),
			WeaveGroupName(client->currentWeaveGroup),
			WeavePowerName(client->currentWeaveThreads[0]),
			WeavePowerName(client->currentWeaveThreads[1]),
			WeavePowerName(client->currentWeaveThreads[2]),
			WeavePowerName(client->currentWeaveThreads[3]),
			WeavePowerName(client->currentWeaveThreads[4]),
			WeavePowerName(client->currentWeaveThreads[5]),
			WeavePowerName(client->currentWeaveThreads[6]),
			WeavePowerName(client->currentWeaveThreads[7])));
#else
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
#endif
		CreateWeave(ent, client->currentWeaveGroup, client->currentWeaveThreads);
	}
	client->ps.stats[STAT_THREADX] = 0;
	client->ps.stats[STAT_THREADY] = 0;

	for(i = 0; i < MAX_THREADS; i++)
	{
		client->currentWeaveThreads[i] = 0;
	}
}

void ClientWeaveUpdateStats(gentity_t * ent, gclient_t * client)
{
	// weaving stats
	client->ps.stats[STAT_POWER] = ClientPowerAvailable(client);
	client->ps.stats[STAT_MAX_POWER] = ClientPowerMax(client);
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
