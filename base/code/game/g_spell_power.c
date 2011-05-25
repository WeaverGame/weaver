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

// g_spell_power -- misc weaver power functions, including linking functions
#include "../../../code/qcommon/q_shared.h"
#include "g_local.h"
#include "g_spell_util.h"

/*
=================
ClientPowerShielded

Returns true if the given client is shielded
=================
*/
qboolean ClientPowerShielded(gclient_t * holdingClient)
{
	return (holdingClient->ps.powerups[PW_SHIELDED] > 0);
}

/*
=================
ClientPowerInUse

Returns the amount of power which the given client is currently expending
=================
*/
int ClientPowerInUse(gclient_t * holdingClient)
{
	int             power;

	DEBUGWEAVEING_LVL("ClientPowerInUse: start", 3);
	if(ClientPowerShielded(holdingClient))
	{
		holdingClient->ps.stats[STAT_POWER] = 0;
		return 0;
	}

	power = holdingClient->powerUsed + holdingClient->powerThreading;
	if(holdingClient->linkFollower)
	{
		power += ClientPowerInUse(holdingClient->linkFollower);
	}
	DEBUGWEAVEING_LVL("ClientPowerInUse: end", 3);
	return power;
}

/*
=================
ClientPowerMax

Returns the maximum amount of power given client can have.
This is a method so it can be modified by stuff like angreal
=================
*/
int ClientPowerMax(gclient_t * holdingClient)
{
	return holdingClient->powerMax;
}

/*
=================
ClientPowerAvailable

Returns the power available to a given client.
Includes power available from other clients, through a link.
This is the power used for casting things.
=================
*/
int ClientPowerAvailable(gclient_t * holdingClient)
{
	int             power;

	DEBUGWEAVEING_LVL("ClientPowerAvailable: start", 5);

	if(ClientPowerShielded(holdingClient))
	{
		return 0;
	}

	power = holdingClient->powerMax - (holdingClient->powerUsed + holdingClient->powerThreading);
	if(holdingClient->linkFollower)
	{
		power += ClientPowerAvailable(holdingClient->linkFollower);
	}
	DEBUGWEAVEING_LVL("ClientPowerAvailable: end", 5);
	return power;
}

/*
=================
ClientPowerHeldConsume

A held weave consumes power from this client, and all the clients in the circle.
The consumption is shared equally.
=================
*/
void ClientPowerHeldConsume(gclient_t * holdingClient, int amount)
{
	int             linkCount;
	gclient_t      *linkI;

	DEBUGWEAVEING_LVL("ClientPowerHeldConsume: start", 4);
	//Com_Printf("HeldConsume c=%d p=%d\n", holdingClient->ps.clientNum, amount);

	//Count links in chain
	linkCount = 0;
	linkI = holdingClient;
	do
	{
		linkCount++;
		if(!linkI->linkFollower)
		{
			break;
		}
		linkI = linkI->linkFollower;
	} while(linkI);

	//Divide
	amount = amount / linkCount;

	//Use power for each player in chain
	linkI = holdingClient;
	do
	{
		linkI->powerUsed += amount;
		if(!linkI->linkFollower)
		{
			break;
		}
		linkI = linkI->linkFollower;
	} while(linkI);
	DEBUGWEAVEING_LVL("ClientPowerHeldConsume: end", 4);
}

/*
=================
ClientPowerHeldRelease

Frees up power being used in a weave in progress.
This is used so the power can be recounted next frame.
=================
*/
void ClientPowerHeldRelease(gclient_t * holdingClient)
{
	DEBUGWEAVEING_LVL("ClientPowerHeldRelease: start", 4);
	//Only free power if this player is not linked, or is the head link
	if(holdingClient->linkTarget)
	{
		return;
	}

	//For each link in chain, set power used to 0
	do
	{
		holdingClient->powerUsed = 0;
		if(!holdingClient->linkFollower)
		{
			break;
		}
		holdingClient = holdingClient->linkFollower;
	} while(holdingClient);
	DEBUGWEAVEING_LVL("ClientPowerHeldRelease: end", 4);
}

/*
=================
ClientPowerConsume

Consumes power temporarily (powerThreading, not powerUsed) from this client.
Shared equally amoung all clients in circle.
=================
*/
void ClientPowerConsume(gclient_t * holdingClient, int amount)
{
	int             linkCount;
	gclient_t      *linkI;

	DEBUGWEAVEING("ClientPowerConsume: start");
	//Count clients in link chain
	linkCount = 0;
	linkI = holdingClient;
	do
	{
		linkCount++;
		if(!linkI->linkFollower)
		{
			break;
		}
		linkI = linkI->linkFollower;
	} while(linkI);

	amount = amount / linkCount;

	//Use power for each player in chain
	linkI = holdingClient;
	do
	{
		linkI->powerThreading += amount;
		if(!linkI->linkFollower)
		{
			break;
		}
		linkI = linkI->linkFollower;
	} while(linkI);
	DEBUGWEAVEING("ClientPowerConsume: end");
}

/*
=================
ClientPowerRelease

Releases temporarily consumed power from this client, and all clients in circle.
=================
*/
void ClientPowerRelease(gclient_t * holdingClient)
{
	DEBUGWEAVEING_LVL("ClientPowerRelease: start", 5);
	if(holdingClient->linkTarget)
	{
		return;
	}

	do
	{
		holdingClient->powerThreading = 0;
		if(!holdingClient->linkFollower)
		{
			break;
		}
		holdingClient = holdingClient->linkFollower;
	} while(holdingClient);
	DEBUGWEAVEING_LVL("ClientPowerRelease: end", 5);
}

/*
=================
ClientPowerInitialize

Prepares a client's power variables.
Called each spawn.
=================
*/
void ClientPowerInitialize(gclient_t * holdingClient)
{
	holdingClient->powerMax = FULL_POWER;
	holdingClient->ps.stats[STAT_POWER] = holdingClient->powerMax;
	holdingClient->ps.stats[STAT_MAX_POWER] = holdingClient->powerMax;
}

/*
=================
ClientLink

Creates link between two clients, ie	
	Lead       Follow
Becomes
	Lead  <--  Follow

Pre:
	Follow can link to Lead
	Lead can be followed
	Follow has a linkHeld weave set

Post:
	Follow is linked to Lead
	Follow's heldweave for the link has reference to the link entity
=================
*/
void ClientLink(gclient_t * leadClient, gclient_t * followClient)
{
	gentity_t      *bolt;

	DEBUGWEAVEING("ClientLink: start");

	//Link clients
	leadClient->linkFollower = followClient;
	followClient->linkTarget = leadClient;

	//Create link ent
	bolt = G_Spawn();
	bolt->classname = LINK_CLASSNAME;
	bolt->nextthink = level.time + WEAVE_LINKTHINK_TIME;
	bolt->think = RunLinkEnt;
	bolt->s.eType = ET_WEAVE_LINK;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WVW_D_SPIRIT_LINK;
	bolt->r.ownerNum = followClient->ps.clientNum;
	bolt->s.otherEntityNum = leadClient->ps.clientNum;
	bolt->s.otherEntityNum2 = followClient->linkHeld->s.number;
	bolt->parent = &g_entities[followClient->ps.clientNum];
	bolt->damage = 0;
	bolt->splashDamage = 0;
	bolt->splashRadius = 0;
	bolt->methodOfDeath = MOD_UNKNOWN;	//Incapable of killing
	bolt->splashMethodOfDeath = MOD_UNKNOWN;	//Incapable of killing
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = &g_entities[leadClient->ps.clientNum];
	bolt->freeAfterEvent = qfalse;

	bolt->s.pos.trType = TR_INTERPOLATE;

	DEBUGWEAVEING("ClientLink: bolt created, about to run\n");
	
	//VectorCopy(bolt->parent->s.origin, bolt->s.origin);
	//VectorCopy(bolt->parent->s.origin, bolt->s.pos.trBase);
	//VectorCopy(bolt->target_ent->s.origin, bolt->r.currentOrigin);

	RunLinkEnt(bolt);

	followClient->linkEnt = bolt;

	//reference this from held
	followClient->linkHeld->target_ent = bolt;
	//held weave is now in progress
	G_HeldWeave_SetState(followClient->linkHeld, WST_INPROCESS);
	//prevent held weave expiring
	followClient->linkHeld->nextthink = 0;

	trap_LinkEntity(bolt);
	DEBUGWEAVEING("ClientLink: end");
}

/*
=================
ClientUnlink

Removes link between two clients, ie	
	Lead  <--  Follow
Becomes
	Lead       Follow

Pre:
	Follow is linked to a client
	Follow has a linkHeld weave set

Post:
	Follow is unlinked from whatever it was following
	Follow's heldweave is removed if not temporary
	Follow's heldweave is kept, and reference to linkEnt cleared if temporary unlink
=================
*/
void ClientUnlink(gclient_t * followClient, qboolean temporary)
{
	DEBUGWEAVEING("ClientUnlink: start");
	//unlink entitities
	followClient->linkTarget->linkFollower = NULL;
	followClient->linkTarget = NULL;
	DEBUGWEAVEING("ClientUnlink: link ent\n");
	//eliminate link entity
	G_FreeEntity(followClient->linkEnt);
	followClient->linkEnt = NULL;
	DEBUGWEAVEING("ClientUnlink: temp select\n");
	//if temporary, leave the held weave
	if(temporary)
	{
		//temporary unlink, ie when c is unlinked to be relinked to b
		//b <- a <- c
		//b <- c    a
		DEBUGWEAVEING("ClientUnlink: temp, held->target_ent = null\n");
		followClient->linkHeld->target_ent = NULL;
	}
	else
	{
		//unlink, ie when a unlinked
		//b <- a <- c
		//b <- c    a
		DEBUGWEAVEING("ClientUnlink: nontemp, held cleared\n");
		ClearHeldWeave(followClient->linkHeld);
	}
	DEBUGWEAVEING("ClientUnlink: end");
}

/*
=================
RunLinkEnt

Updates the location of a linkEnt.
=================
*/
void RunLinkEnt(gentity_t * link)
{
	vec3_t          nextPos;

	DEBUGWEAVEING("RunLinkEnt: start");

	// New link origin = average of old origin and players' origins
	VectorAdd(link->target_ent->s.pos.trBase, link->parent->s.pos.trBase, nextPos);
	VectorMA(vec3_origin, 0.5f, nextPos, nextPos);

	//VectorSubtract(nextPos, link->s.pos.trBase, link->s.pos.trDelta);
	VectorCopy(nextPos, link->s.pos.trBase);
	VectorCopy(nextPos, link->r.currentOrigin);

	link->s.pos.trTime = level.time;
	link->nextthink = level.time + WEAVE_LINKTHINK_TIME;

	//G_Printf("LinkEnt type=%d weapon=%d pos=%f %f %f\n", link->s.eType, link->s.weapon, link->s.pos.trBase[0], link->s.pos.trBase[1], link->s.pos.trBase[2]);

	DEBUGWEAVEING("RunLinkEnt: end");
}

/*
=================
ClientLinkJoin

Adds a player to a circle.
Finds the end of the link that the leader is in and adds the follower to the end.
=================
*/
qboolean ClientLinkJoin(gclient_t * leadClient, gclient_t * followClient)
{
	gclient_t      *leader;

	DEBUGWEAVEING("ClientLinkJoin: start");
	//check target client isn't higher in list
	//to avoid circular linked list
	leader = leadClient;
	if(leader->ps.clientNum == followClient->ps.clientNum)
	{
		return qfalse;
	}
	while(leader->linkTarget)
	{
		leader = leader->linkTarget;
		if(leader->ps.clientNum == followClient->ps.clientNum)
		{
			return qfalse;
		}
	}

	DEBUGWEAVEING("ClientLinkJoin: find end of linked list");
	//find end of linked list
	if(leadClient->ps.clientNum == followClient->ps.clientNum)
	{
		return qfalse;
	}
	while(leadClient->linkFollower)
	{
		leadClient = leadClient->linkFollower;
		if(leadClient->ps.clientNum == followClient->ps.clientNum)
		{
			return qfalse;
		}
	}
	if(DEBUGWEAVEING_TST(1))
	{
		Com_Printf("ClientLinkJoin: lead=%d follow=%d", leadClient->ps.clientNum, followClient->ps.clientNum);
	}
	//create the link
	ClientLink(leadClient, followClient);

	DEBUGWEAVEING("ClientLinkJoin: end");
	return qtrue;
}

/*
=================
ClientLinkLeave

Removes a client from the circle (ie, client dies).
Circle remains, and players are linked around the leaver.
=================
*/
void ClientLinkLeave(gclient_t * followClient)
{
	gclient_t      *target;
	gclient_t      *follower;

	DEBUGWEAVEING("ClientLinkLeave: start");

	target = followClient->linkTarget;
	follower = followClient->linkFollower;

	if(!target && !follower)
	{
		DEBUGWEAVEING("ClientLinkLeave: no link");
		return;
	}
	else if(!target)
	{
		// This client is the top of the list, only unlink next link
		DEBUGWEAVEING("ClientLinkLeave: top unlink");
		// Permenent unlink
		ClientUnlink(follower, qfalse);
	}
	else if(!follower)
	{
		// This client is the end of the list, only unlink this client
		DEBUGWEAVEING("ClientLinkLeave: end unlink");
		// Permenent unlink
		ClientUnlink(followClient, qfalse);
	}
	else
	{
		// This client is in the middle of the list
		// Unlink this client (both sides) and link around it
		DEBUGWEAVEING("ClientLinkLeave: mid unlink");

		// Unlink this client
		// Permenent unlink
		ClientUnlink(followClient, qfalse);
		// Unlink next client
		// Temporary unlink, this client will be linked around the one being removed
		ClientUnlink(follower, qtrue);
		// Relink around this client
		ClientLink(target, follower);
	}
	DEBUGWEAVEING("ClientLinkLeave: end");
}
