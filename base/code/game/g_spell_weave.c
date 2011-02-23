/*
===========================================================================
This file is part of the weaver game.

It runs weaves
===========================================================================
*/
//
#include "g_local.h"
#include "g_spell_effects.h"
#include "g_spell_util.h"

void CreateThreads(gentity_t * player)
{
	gentity_t      *threadsEnt;

	if(!player)
	{
		DEBUGWEAVEING("CreateThreads: no player");
		return;
	}

	DEBUGWEAVEING("CreateThreads: start");

	threadsEnt = G_Spawn();

	threadsEnt->classname = THREAD_CLASSNAME;
	threadsEnt->nextthink = 0;
	threadsEnt->parent = player;
	threadsEnt->r.ownerNum = player->client->ps.clientNum;
	threadsEnt->r.svFlags = SVF_BROADCAST;
	threadsEnt->s.eType = ET_WEAVE_THREADS;
	threadsEnt->s.otherEntityNum2 = player->s.number;
	//set owner
	threadsEnt->s.torsoAnim = player->client->ps.clientNum;

	Com_Printf("Making Player ThreadsEnt for %d, svflags=%d threadsEnt=%d\n", threadsEnt->s.torsoAnim, threadsEnt->r.svFlags,
			   threadsEnt->s.number);

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

	player->client->threadEnt = threadsEnt;

	trap_LinkEntity(threadsEnt);
	DEBUGWEAVEING("CreateThreads: end");
	return;
}

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
		}
	}

	DEBUGWEAVEING_LVL("ThreadsThink: end", 2);
}

/*
=================
MakeWeaveHeld

Creates a held weave entity using parameters.
=================
*/
gentity_t      *MakeWeaveHeld(gentity_t * self, int weaveID, int holdTime, int holdPower, int switchable, qboolean runAtTime)
{
	gentity_t      *weave;

	DEBUGWEAVEING("MakeWeaveHeld: start");

	weave = G_Spawn();

	weave->classname = HELD_CLASSNAME;
	weave->nextthink = level.time + holdTime;
	if(runAtTime)
	{
		weave->think = ExpireHeldWeave;
	}
	else
	{
		weave->think = ClearHeldWeave;
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

	DEBUGWEAVEING("MakeWeaveHeld: end");
	return weave;
}

/*
=================
CreateWeave

This function invokes actions for a player's weaving.
It takes the basic thread inputs, then creates appropriate entities for this action.
=================
*/
void CreateWeave(gentity_t * self, int group, int threads[MAX_THREADS])
{
	int             weaveID;
	int             powerUsing;
	int             i;

	if(!self)
	{
		DEBUGWEAVEING("CreateWeave: no self");
		return;
	}

	DEBUGWEAVEING("CreateWeave: start");

	powerUsing = 0;
	weaveID = -1;

	//count up ammount of power being used
	for(i = 0; i < MAX_THREADS; i++)
	{
		if(threads[i] > 0)
		{
			powerUsing += POWER_PER_THREAD;
		}
		else
		{
			break;
		}
	}

	weaveID = ThreadsToWeaveID(group, threads);
	CreateWeaveID(self, weaveID, powerUsing);

	DEBUGWEAVEING("CreateWeave: end");
	return;
}

/*
=================
CreateWeaveID

=================
*/
void CreateWeaveID(gentity_t * self, int weaveID, int powerUsing)
{
	gentity_t      *weave;

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
			//add held weave
			weave = MakeWeaveHeld(self, weaveID, TIME_TWO_MIN, powerUsing, 1, qtrue);
			AddHeldWeaveToPlayer(weave, &self->client->ps);
			break;
		case WVW_D_AIRFIRE_LIGHT:
		case WVW_D_AIRWATER_FOG:
			//spawn ent (lasting 2 mins max)
			//add held weave
			weave = MakeWeaveHeld(self, weaveID, TIME_TWO_MIN, powerUsing, 1, qtrue);
			AddHeldWeaveToPlayer(weave, &self->client->ps);
			//weave should ref the new entity thats created.
			break;
		case WVW_D_SPIRIT_TRAVEL:
			//spawn ent (lasting 20 sec max)
			//add held weave
			weave = MakeWeaveHeld(self, weaveID, TIME_TWENTY_SEC, powerUsing, 1, qtrue);
			AddHeldWeaveToPlayer(weave, &self->client->ps);
			//weave should ref the new entity thats created.
			break;
		case WVW_D_AIR_PROTECT:
		case WVW_D_FIRE_PROTECT:
		case WVW_D_EARTH_PROTECT:
		case WVW_D_WATER_PROTECT:
			//add powerup to self
			//add held weave
			weave = MakeWeaveHeld(self, weaveID, TIME_TWO_MIN, powerUsing, 1, qtrue);
			AddHeldWeaveToPlayer(weave, &self->client->ps);
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
			weave = MakeWeaveHeld(self, weaveID, TIME_THREE_SEC, powerUsing, 0, qfalse);
			AddHeldWeaveToPlayer(weave, &self->client->ps);
			break;
			/*end holdable, no switch */

			/*begin holdable, no switch, fire on expire */
		case WVW_A_SPIRIT_SLICE_S:
		case WVW_A_SPIRIT_SLICE_M:
		case WVW_A_SPIRIT_SLICE_L:
			//add held weave
			weave = MakeWeaveHeld(self, weaveID, TIME_THREE_SEC, powerUsing, 0, qtrue);
			AddHeldWeaveToPlayer(weave, &self->client->ps);
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
			weave = MakeWeaveHeld(self, weaveID, TIME_TWO_MIN, powerUsing, 1, qfalse);
			AddHeldWeaveToPlayer(weave, &self->client->ps);
			break;
			//this one is special because it has a different hold time limit
		case WVW_A_AIRWATER_RIP:
			// add held weave
			weave = MakeWeaveHeld(self, weaveID, TIME_THREE_SEC, powerUsing, 1, qfalse);
			AddHeldWeaveToPlayer(weave, &self->client->ps);
			break;
		case WVW_A_SPIRIT_SHIELD:
			//add held weave
			weave = MakeWeaveHeld(self, weaveID, TIME_TWENTY_SEC, powerUsing, 1, qfalse);
			AddHeldWeaveToPlayer(weave, &self->client->ps);
			break;
			/*end holdable, switch */

			/*anything else is an error. */
		case WVW_NONE:
		case -1:
		default:
			break;
	}

	DEBUGWEAVEING("CreateWeaveID: end");
	return;
}

/*
=================
AddHeldWeaveToPlayer

Adds a weave to a players held weaves list.

ent is a ET_HELD_WEAVE
player is playerState of player to add it to
=================
*/
void AddHeldWeaveToPlayer(gentity_t * ent, playerState_t * player)
{
	int             i;

	if(!ent)
	{
		DEBUGWEAVEING("AddHeldWeaveToPlayer: no ent");
		return;
	}
	if(!player)
	{
		DEBUGWEAVEING("AddHeldWeaveToPlayer: no player");
		return;
	}

	DEBUGWEAVEING("AddHeldWeaveToPlayer: start");

	for(i = MIN_WEAPON_WEAVE; i < MAX_WEAPONS; i++)
	{
		if(player->ammo[i] <= 0)
		{
			player->ammo[i] = ent->s.number;
			ent->s.modelindex2 = i;	// remember slot
			player->weapon = i;
			player->stats[STAT_WEAPONS] |= 1 << i;

			break;
		}
	}
	DEBUGWEAVEING("AddHeldWeaveToPlayer: end");
}

/*
=================
ExpireHeldWeave

Expires a given weave
ent is a ET_HELD_WEAVE
=================
*/
void ExpireHeldWeave(gentity_t * heldWeave)
{
	if(!heldWeave)
	{
		DEBUGWEAVEING("ExpireHeldWeave: no ent");
		return;
	}

	DEBUGWEAVEING("ExpireHeldWeave: start");
	if(G_HeldWeave_GetState(heldWeave) == WST_HELD)
	{
		G_HeldWeave_SetState(heldWeave, WST_EXPIRED);
		UseHeldWeave(heldWeave);
	}
	DEBUGWEAVEING("ExpireHeldWeave: end");
}

/*
=================
UseHeldWeave

Logic to see if a held weave can be used.
Calls ExecuteWeave() to run the weave.
Updates charges and power usage.

ent is a ET_HELD_WEAVE
=================
*/
void UseHeldWeave(gentity_t * heldWeave)
{
	float           newPower;
	int             maxCharges;
	int             cast = 0;

	if(!heldWeave)
	{
		DEBUGWEAVEING("UseHeldWeave: no ent");
		return;
	}

	heldWeave->parent->client->ps.weaponTime = WeaveTime(heldWeave->s.weapon);

	DEBUGWEAVEING("UseHeldWeave: start");
	Com_Printf("Using held weave %i charges %i of %i\n", heldWeave->s.number, G_HeldWeave_GetCharges(heldWeave),
			   WeaveCharges(heldWeave->s.weapon));

	//check if weave is being held, held weaves will be ended, not execed
	if(G_HeldWeave_GetState(heldWeave) == WST_INPROCESS || G_HeldWeave_GetState(heldWeave) == WST_INPROCESSRELEASED)
	{
		//weave is being held, this use should end it
		EndWeave(heldWeave);
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
		if(!ExecuteWeave(heldWeave))
		{
			Com_Printf("Weave execution returned 0. WeaveID=%d\n", heldWeave->s.weapon);
			return;
		}

		cast = heldWeave->s.weapon;

		//held weave now consumes less power    
		if(DEBUGWEAVEING_TST(1))
		{
			Com_Printf("UseHeldWeave: previous power consumed = %d\n", G_HeldWeave_GetPower(heldWeave));
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
				Com_Printf("UseHeldWeave: shot expended %d remain\n", G_HeldWeave_GetCharges(heldWeave));
			}
		}

		//set new power consumption
		newPower *= (float)(G_HeldWeave_GetCharges(heldWeave) + maxCharges);
		G_HeldWeave_SetPower(heldWeave, newPower);

		if(DEBUGWEAVEING_TST(1))
		{
			Com_Printf("UseHeldWeave: new power consumed = %d\n", G_HeldWeave_GetPower(heldWeave));
		}
	}

	//if no shots remaining or expired
	if(G_HeldWeave_GetCharges(heldWeave) <= 0 || G_HeldWeave_GetState(heldWeave) == WST_EXPIRED ||
	   G_HeldWeave_GetState(heldWeave) == WST_RELEASED || G_HeldWeave_GetState(heldWeave) == WST_INPROCESSRELEASED)
	{
		//then the weave should be removed
		ClearHeldWeaveCast(heldWeave, cast);
	}
	DEBUGWEAVEING("UseHeldWeave: end");
}

/*
=================
ReleaseWeaveCmd

Given a weave effect in progress, 
this method gets the held weave 
and releases it (and subsequently this weave effect).
=================
*/
void ReleaseHeldWeave(gentity_t * ent)
{
	playerState_t  *pstate;
	gentity_t      *heldWeave;

	if(!ent || !ent->client)
	{
		return;
	}

	pstate = &ent->client->ps;

	if(pstate->weapon && (pstate->weapon >= MIN_WEAPON_WEAVE))
	{
		//this is a weave)
		if(pstate->ammo[pstate->weapon])
		{
			heldWeave = &g_entities[pstate->ammo[pstate->weapon]];
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
			UseHeldWeave(heldWeave);
		}
	}
}

/*
=================
G_ReleaseWeave

Given a weave effect in progress, 
this method gets the held weave 
and releases it (and subsequently this weave effect).
=================
*/
void G_ReleaseWeave(gentity_t * weave)
{
	gentity_t      *heldWeave;

	if(!weave)
	{
		DEBUGWEAVEING("G_ReleaseWeave: no ent");
		return;
	}

	DEBUGWEAVEING("G_ReleaseWeave: start");

	heldWeave = &g_entities[weave->s.otherEntityNum2];

	EndWeave(heldWeave);

	ClearHeldWeave(heldWeave);
	DEBUGWEAVEING("G_ReleaseWeave: end");
}

/*
=================
EndWeave

Ends a weave in progress

=================
*/
void EndWeave(gentity_t * weave)
{
	vec3_t          start;
	vec3_t          direction;
	gentity_t      *player;
	int             heldWeaveNum;

	if(!weave)
	{
		DEBUGWEAVEING("EndWeave: no ent");
		return;
	}

	DEBUGWEAVEING("EndWeave: start");

	//calc parameters
	heldWeaveNum = weave->s.number;
	player = weave->parent;
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

	switch (weave->s.weapon)
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
ExecuteWeave

Actually executes a weave

=================
*/
qboolean ExecuteWeave(gentity_t * heldWeave)
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
		case WVW_A_AIRWATER_DARTS_M:
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
ClearHeldWeave

Removes weave from the player
Frees the weave entity.

ent is a ET_HELD_WEAVE
castClear if non zero (ie, non WVW_NONE), this weave is being cleared because it was just cast.
	a CAST + CLEAR event can be sent instead of a CLEAR event which would overwrite the CAST.
	castClear is the weave ID. Use WVW_NONE if weave was not cast.
=================
*/
void ClearHeldWeaveCast(gentity_t * ent, int castClear)
{
	int             i;
	playerState_t  *player;
	gentity_t      *pent;

	if(!ent)
	{
		DEBUGWEAVEING("ClearHeldWeave: no ent");
		return;
	}

	DEBUGWEAVEING("ClearHeldWeave: start");

	//get player who is holding weave
	pent = &g_entities[ent->s.otherEntityNum2];
	player = &pent->client->ps;

	//for(i = 0; i < HELD_MAX; i++)
	//[MAX_WEAPONS - i - 1]
	for(i = MIN_WEAPON_WEAVE; i < MAX_WEAPONS; i++)
	{
		//where the weave is being held
		//Com_Printf("player->ammo[%i] = %i == ent->s.number = %i\n", i, player->ammo[i], ent->s.number);
		if(player->ammo[i] == ent->s.number)
		{
			//weave is no longer held
			player->ammo[i] = 0;
			//weapon slot is no longer available
			player->stats[STAT_WEAPONS] &= ~(1 << i);
			if(i == player->weapon)
			{
				if(castClear != 0)
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

	if(ent->target_ent)
	{
		G_FreeEntity(ent->target_ent);
	}

	//clear the held weave
	G_FreeEntity(ent);
	DEBUGWEAVEING("ClearHeldWeave: end");
}

void ClearHeldWeave(gentity_t * ent)
{
	ClearHeldWeaveCast(ent, WVP_NONE);
}


void G_RunWeaveEffect(gentity_t * ent)
{
	if(!ent)
	{
		DEBUGWEAVEING("G_RunWeaveEffect: no ent");
		return;
	}
	DEBUGWEAVEING_LVL("G_RunWeaveEffect: start", 2);
	switch (ent->s.weapon)
	{
			//Held special
		case WVW_A_AIRFIRE_LIGHTNING:	//target like arty
			RunWeave_Lightning(ent);
			break;
			//Misc
		case WVW_A_AIRFIRE_SWORD:
			break;
			//No entities
		case WVW_D_AIR_PROTECT:
		case WVW_D_FIRE_PROTECT:
		case WVW_D_EARTH_PROTECT:
		case WVW_D_WATER_PROTECT:
			RunWeave_MoveToTarget(ent);
			break;
		case WVW_D_AIRFIRE_LIGHT:
			RunWeave_MoveToTarget(ent);
			break;
		case WVW_D_AIRWATER_FOG:
		case WVW_D_SPIRIT_TRAVEL:
		case WVW_D_SPIRIT_LINK:
			//Spawn Ent
		case WVW_D_AIRFIRE_WALL:
		case WVW_D_EARTHFIRE_EXPLOSIVE_S:
		case WVW_D_EARTHFIRE_EXPLOSIVE_M:
			//Target
		case WVW_A_FIRE_BLOSSOMS:
		case WVW_A_SPIRIT_DEATHGATE:
		case WVW_D_EARTH_UNLOCK:
		case WVW_A_EARTHFIRE_IGNITE:
			break;
		case WVW_D_WATER_HEAL_S:
		case WVW_D_WATER_HEAL_M:
			RunWeave_Heal(ent);
			break;
		case WVW_D_SPIRIT_STAMINA:
			RunWeave_Stamina(ent);
			break;
		case WVW_D_WATER_CURE:
		case WVW_D_AIRFIRE_INVIS:
		case WVW_A_SPIRIT_STILL:
		case WVW_D_AIR_GRAB:
			break;
		case WVW_A_AIR_GRABPLAYER:
			RunWeave_GrabPlayer(ent);
			break;
		case WVW_A_AIR_BINDPLAYER:
			break;
		case WVW_A_SPIRIT_SHIELD:
			RunWeave_Shield(ent);
			break;
			//Missiles
		case WVW_A_SPIRIT_SLICE_S:
		case WVW_A_SPIRIT_SLICE_M:
		case WVW_A_SPIRIT_SLICE_L:
			RunWeave_Slice(ent);
			break;
		case WVW_A_SPIRIT_BALEFIRE:
			break;
		case WVW_A_AIR_BLAST:
			RunWeave_Missile(ent);
			break;
		case WVW_A_EARTH_QUAKE_S:
		case WVW_A_EARTH_QUAKE_M:
		case WVW_A_EARTH_QUAKE_L:
			if(ent->s.eType == ET_WEAVE_EFFECT)
			{
				RunWeave_EarthQuake(ent);
			}
			else
			{
				RunWeave_Missile(ent);
			}
			break;
		case WVW_A_FIRE_BALL:
			RunWeave_Missile(ent);
			break;
		case WVW_A_FIRE_DARTS:
		case WVW_A_FIRE_MULTIDARTS:
			RunWeave_Missile(ent);
			break;
		case WVW_A_WATER_ICESHARDS_S:
		case WVW_A_WATER_ICESHARDS_M:
			RunWeave_Missile(ent);
			break;
		case WVW_A_AIRWATER_DARTS_S:
		case WVW_A_AIRWATER_DARTS_M:
			break;
		case WVW_A_EARTHWATER_SLOW:
		case WVW_A_EARTHWATER_POISON:
			RunWeave_Missile(ent);
			break;
			//Fail
		case WVW_NONE:
		case -1:
		default:
			break;
	}
	DEBUGWEAVEING_LVL("G_RunWeaveEffect: end", 2);
}

int WeaveTime(int weaveID)
{
	switch (weaveID)
	{
			/*begin end on exec */
		case WVW_A_AIRFIRE_SWORD:
			return 1000;
		case WVW_D_AIRFIRE_LIGHT:
			return 400;
		case WVW_D_AIRWATER_FOG:
			return 400;
		case WVW_D_SPIRIT_TRAVEL:
			return 400;
		case WVW_D_AIR_PROTECT:
			return 400;
		case WVW_D_FIRE_PROTECT:
			return 400;
			/*end end on exec */

			/*begin holdable, no switch */
		case WVW_D_SPIRIT_LINK:
			return 500;
		case WVW_D_AIRFIRE_WALL:
			return 400;
		case WVW_A_FIRE_BLOSSOMS:
			return 400;
		case WVW_D_EARTH_UNLOCK:
			return 500;
		case WVW_A_EARTHFIRE_IGNITE:
			return 100;
		case WVW_D_EARTHFIRE_EXPLOSIVE_S:
		case WVW_D_EARTHFIRE_EXPLOSIVE_M:
			return 400;
		case WVW_D_WATER_CURE:
			return 400;
		case WVW_D_WATER_HEAL_S:
		case WVW_D_WATER_HEAL_M:
			return 400;
		case WVW_A_SPIRIT_SLICE_S:
		case WVW_A_SPIRIT_SLICE_M:
		case WVW_A_SPIRIT_SLICE_L:
			return 500;
		case WVW_D_SPIRIT_STAMINA:
			return 1000;
		case WVW_D_AIRFIRE_INVIS:
			return 1000;
		case WVW_A_SPIRIT_STILL:
			return 1000;
		case WVW_A_SPIRIT_BALEFIRE:
			return 1000;
		case WVW_A_SPIRIT_DEATHGATE:
			return 1000;
			/*end holdable, no switch */

			/*begin holdable, switch */
		case WVW_D_AIR_GRAB:
			return 1000;
		case WVW_A_AIR_BLAST:
			return 500;
		case WVW_A_AIR_GRABPLAYER:
			return 1500;
		case WVW_A_AIR_BINDPLAYER:
			return 1500;
		case WVW_A_EARTH_QUAKE_S:
		case WVW_A_EARTH_QUAKE_M:
		case WVW_A_EARTH_QUAKE_L:
			return 1500;
		case WVW_A_FIRE_BALL:
			return 800;
		case WVW_A_FIRE_DARTS:
			return 250;
		case WVW_A_FIRE_MULTIDARTS:
			return 200;
		case WVW_A_AIRFIRE_LIGHTNING:
			return 100;
		case WVW_A_AIRWATER_DARTS_S:
		case WVW_A_AIRWATER_DARTS_M:
			return 750;
		case WVW_A_EARTHWATER_SLOW:
			return 750;
		case WVW_A_EARTHWATER_POISON:
			return 750;
		case WVW_A_SPIRIT_SHIELD:
			return 1000;
			/*end holdable, switch */

			/*anything else is an error. */
		case WVW_NONE:
		case -1:
		default:
			return 400;
	}
}
