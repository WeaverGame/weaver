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

// g_spell_ents.c -- misc weaver entities
#include "g_local.h"

/*
===============================================================================

SHIELD

===============================================================================
*/

void func_shield_ActivateUse(gentity_t * ent, gentity_t * other, qboolean firstActivate)
{
	// Check if this player is on the right team.
	if(other->client->ps.persistant[PERS_TEAM] == TEAM_RED)
	{
		if(ent->blue_only)
		{
			return;
		}
	}
	else if(other->client->ps.persistant[PERS_TEAM] == TEAM_BLUE)
	{
		if(ent->red_only)
		{
			return;
		}
	}

	if(ent->count > 0)
	{
		//expend shield
		ent->count--;
	}
	//Update shield infos
	if(ent->shield_ent && ent->shield_ent->activate)
	{
		ent->shield_ent->activate(ent->shield_ent, other, firstActivate);
	}
	if(ent->slow && ent->count <= 0)
	{
		ent->die(ent, NULL, other, 0, 0);
	}
}

void func_shield_die(gentity_t * self, gentity_t * inflictor, gentity_t * attacker, int damage, int mod)
{
	if(!self->slow) return;
	// Shield is now dead, prevent this die function from being run multiple times.
	self->slow = qfalse;

	//Free any Shield infos
	if(self->shield_ent && self->shield_ent->die)
	{
		self->shield_ent->die(self->shield_ent, inflictor, attacker, damage, mod);
	}

	if(self->target_ent)
	{
		self->target_ent->shield_ent = NULL;
	}

	//Explosion event
	G_AddEvent(self, EV_SHIELD_POP, 0);
	self->freeAfterEvent = qtrue;
	self->s.solid = 0;
	self->s.otherEntityNum2 = 0;

#ifdef G_LUA
	// Lua API callbacks
	if(self->luaTrigger)
	{
		if(attacker)
		{
			G_LuaHook_EntityTrigger(self->luaTrigger, self->s.number, attacker->s.number);
		}
		else
		{
			G_LuaHook_EntityTrigger(self->luaTrigger, self->s.number, ENTITYNUM_WORLD);
		}
	}
#endif
}

/*
===============
Think_SetupShieldTargets
===============
*/
void Think_SetupShieldTargets(gentity_t * ent)
{
	ent->target_ent = G_Find(NULL, FOFS(name), ent->target);
	if(!ent->target_ent)
	{
		G_Printf("Warning: func_shield at %s with an unfound target\n", vtos(ent->s.origin));
		return;
	}
	if(g_debugEntities.integer >= 1)
	{
		G_Printf("func_shield at %s targeting %s\n", vtos(ent->s.origin), ent->target);
	}

	ent->s.otherEntityNum2 = ent->target_ent->s.number;

	ent->target_ent->shield_ent = ent;
}

/*QUAKED func_shield (0 .5 .8) ?
Shield entity which protects an entity (typically a func_explosive) from damage.
*/
void SP_func_shield(gentity_t * ent)
{
	trap_SetBrushModel(ent, ent->model);

	VectorCopy(ent->s.origin, ent->s.pos.trBase);
	VectorCopy(ent->s.origin, ent->r.currentOrigin);

	VectorCopy(ent->s.angles, ent->s.apos.trBase);
	VectorCopy(ent->s.angles, ent->r.currentAngles);

	G_SpawnInt("power", "1000", &ent->count);
	// Which team can activate (i.e, dispell) this shield?
	G_SpawnBoolean("red_only", "0", &ent->red_only);
	G_SpawnBoolean("blue_only", "0", &ent->blue_only);

	// Encode team ownership.
	ent->s.generic1 = (ent->red_only & (ent->blue_only << 1));

	//Full shield
	ent->s.torsoAnim = ent->count;

	ent->s.eType = ET_SHIELD;

	ent->r.svFlags = SVF_BROADCAST;

	// start trains on the second frame, to make sure their targets have had
	// a chance to spawn
	ent->nextthink = level.time + FRAMETIME;
	ent->think = Think_SetupShieldTargets;

	ent->activate = func_shield_ActivateUse;

	ent->takedamage = qfalse;

	// Alive
	ent->slow = qtrue;

	ent->die = func_shield_die;

	trap_LinkEntity(ent);
}

void func_shieldInfo_ActivateUse(gentity_t * ent, gentity_t * other, qboolean firstActivate)
{
	if(!ent->target_ent)
	{
		return;
	}
	ent->s.frame = ent->target_ent->count;
	if(!ent->shield_ent)
	{
		return;
	}
	if(ent->shield_ent && ent->shield_ent->activate)
	{
		ent->shield_ent->activate(ent->shield_ent, other, firstActivate);
	}
}


/*
===============
Think_SetupShieldInfoTargets
===============
*/
void Think_SetupShieldInfoTargets(gentity_t * ent)
{
	gentity_t      *shieldInfoChain;
	int             i = 0;

	ent->target_ent = G_Find(NULL, FOFS(name), ent->target);
	if(!ent->target_ent)
	{
		G_Printf("func_shield_info at %s with an unfound target (MAPPER: GO FIX THIS ENTITY)\n", vtos(ent->s.origin));
		return;
	}
	if(g_debugEntities.integer >= 1)
	{
		G_Printf("func_shield_info at %s targeting %s\n", vtos(ent->s.origin), ent->target);
	}

	ent->s.otherEntityNum2 = ent->target_ent->s.number;
	ent->s.frame = ent->target_ent->count;

	if(VectorLength(ent->s.angles) == 0)
	{
		VectorSubtract(ent->s.origin, ent->target_ent->s.origin, ent->s.angles);
	}
	VectorNormalize(ent->s.angles);

	// Start at shield
	shieldInfoChain = ent->target_ent;
	// If shield already has a shield_info attached, chain to the shield_info
	while(shieldInfoChain->shield_ent)
	{
		// Follow chain
		shieldInfoChain = shieldInfoChain->shield_ent;
		if((shieldInfoChain->s.eType != ET_SHIELD_INFO) && (shieldInfoChain->s.eType != ET_SHIELD))
		{
			G_Printf("func_shield_info at %s found unexpected entity type (MAPPER: GO FIX THIS ENTITY)\n", vtos(ent->s.origin));
			return;
		}
		// Protect against entities which target in a loop
		i++;
		if(i > 4)
		{
			G_Printf("func_shield_info at %s has a long chain, may be looped (MAPPER: GO FIX THIS ENTITY)\n", vtos(ent->s.origin));
			return;
		}
	}
	// Last shield or shield_info in chain, link to shield info.
	shieldInfoChain->shield_ent = ent;
}

void func_shield_info_die(gentity_t * self, gentity_t * inflictor, gentity_t * attacker, int damage, int mod)
{
	//Free Shield info
	if(self->shield_ent && self->shield_ent->die)
	{
		self->shield_ent->die(self->shield_ent, inflictor, attacker, damage, mod);
	}
	G_FreeEntity(self);
}

/*QUAKED func_shield_info (0 .5 .8) ?
Info location for a shield.
*/
void SP_func_shield_info(gentity_t * ent)
{
	VectorCopy(ent->s.origin, ent->s.pos.trBase);
	VectorCopy(ent->s.origin, ent->r.currentOrigin);

	VectorCopy(ent->s.angles, ent->s.apos.trBase);
	VectorCopy(ent->s.angles, ent->r.currentAngles);

	ent->s.eType = ET_SHIELD_INFO;

	ent->r.svFlags = SVF_BROADCAST;

	// start trains on the second frame, to make sure their targets have had
	// a chance to spawn
	ent->nextthink = level.time + 2*FRAMETIME;
	ent->think = Think_SetupShieldInfoTargets;

	ent->activate = func_shieldInfo_ActivateUse;

	ent->takedamage = qfalse;

	ent->die = func_shield_info_die;

	trap_LinkEntity(ent);
}

/*QUAKED team_OBJ_spawnflag (0 0 1) (-16 -16 -24) (16 16 32)
Capturable spawn flag, to alter spawn ownership in team games.
*/
const vec3_t    capPointMins = { -20, -20, 0 };
const vec3_t    capPointMaxs = { 20, 20, 180 };

void SP_team_OBJ_spawnflag(gentity_t * ent)
{
	VectorCopy(ent->s.origin, ent->s.pos.trBase);
	VectorCopy(ent->s.origin, ent->r.currentOrigin);

	VectorCopy(ent->s.angles, ent->s.apos.trBase);
	VectorCopy(ent->s.angles, ent->r.currentAngles);

	VectorCopy(capPointMins, ent->r.mins);
	VectorCopy(capPointMaxs, ent->r.maxs);
	ent->r.contents = -1;

	ent->s.eType = ET_CAPTURE_POINT;

	ent->r.svFlags = SVF_BROADCAST;

	if( Q_stricmp(ent->team, "red") == 0 || Q_stricmp(ent->team, "1") == 0)
	{
		ent->s.modelindex = TEAM_RED;
	}
	else if( Q_stricmp(ent->team, "blue") == 0 || Q_stricmp(ent->team, "2") == 0)
	{
		ent->s.modelindex = TEAM_BLUE;
	}
	else
	{
		ent->s.modelindex = TEAM_FREE;
	}

	trap_LinkEntity(ent);
}

void Obj_Item_ReadyPickup(gentity_t * ent)
{
	// Its on the ground to be picked up.
	ent->s.otherEntityNum = ENTITYNUM_WORLD;
}

void Return_Obj_Item(gentity_t * ent, gentity_t * other)
{
	team_t          pteam = TEAM_FREE;
	G_SetOrigin(ent, ent->pos1);

	if(other == NULL)
	{
		G_ObjectiveAnnounce(TEAM_FREE, va("An ancient magic returned %s!", ent->message));
	}
	else if (other->s.number == ENTITYNUM_WORLD)
	{
		G_ObjectiveAnnounce(TEAM_FREE, va("An ancient magic returned %s because it fell out of play!", ent->message));
	}
	else
	{
		if(other->client->ps.persistant[PERS_TEAM] == TEAM_RED)
		{
			pteam = TEAM_RED;
		}
		else
		{
			pteam = TEAM_BLUE;
		}
		G_ObjectiveAnnounce(pteam, va("%s" S_COLOR_WHITE " returned %s!", other->client->pers.netname, ent->message));
	}

	// No need to auto return
	ent->nextthink = 0;
	ent->think = NULL;
	ent->crusher = qfalse;
	// crusher if it is not at original location.

	Obj_Item_ReadyPickup(ent);
	trap_LinkEntity(ent);
}

void Return_Obj_Item_Think(gentity_t * ent)
{
	Return_Obj_Item(ent, NULL);
}

/*
================
Drop_Obj_Item
Based on Drop_Item
Spawns an item and tosses it forward
================
*/
gentity_t      *Drop_Obj_Item(gentity_t * ent, gentity_t * dropped, float angle)
{
	vec3_t          velocity;
	vec3_t          angles;
	team_t          pteam = TEAM_FREE;

	if(ent && ent->client)
	{
		// Plyar no longer has this.
		ent->client->objItem = NULL;
	}

	// Calculate drop
	VectorCopy(ent->s.apos.trBase, angles);
	angles[YAW] += angle;
	angles[PITCH] = 0;			// always forward

	AngleVectors(angles, velocity, NULL, NULL);
	VectorScale(velocity, 100, velocity);
	velocity[2] += 100 + crandom() * 50;

	// Set drop position and velocity
	G_SetOrigin(dropped, ent->s.pos.trBase);
	dropped->s.pos.trType = TR_GRAVITY;
	dropped->s.pos.trAcceleration = -g_gravityZ.value;
	dropped->s.pos.trTime = level.time;
	VectorCopy(velocity, dropped->s.pos.trDelta);
	dropped->s.eFlags |= EF_BOUNCE_HALF;
	
	// This is good for other team
	if(ent->client->ps.persistant[PERS_TEAM] == TEAM_RED)
	{
		pteam = TEAM_BLUE;
	}
	else
	{
		pteam = TEAM_RED;
	}

	G_ObjectiveAnnounce(pteam, va("%s" S_COLOR_WHITE " dropped %s!", ent->client->pers.netname, dropped->message));

	// Return on next think.
	dropped->think = Return_Obj_Item_Think;
	dropped->nextthink = level.time + (dropped->wait * 1000.0f);

	// Players can pick it up now.
	Obj_Item_ReadyPickup(dropped);

	trap_LinkEntity(ent);

	return dropped;
}

/*
=================
TossObjItems
Based on TossClientItems

Toss the objective items for the killed player
=================
*/
void G_TossObjItems(gentity_t * self)
{
	if(self->client->objItem)
	{
		// drop the item
		Drop_Obj_Item(self, self->client->objItem, 0);
	}
}

/*
===============
Touch_Obj_Item
Based on Touch_Item
===============
*/
void Touch_Obj_Item(gentity_t * ent, gentity_t * other, trace_t * trace)
{
	qboolean        predict;
	team_t          pteam;

	if(ent->s.otherEntityNum != ENTITYNUM_WORLD)	// Someone else already carrying this.
		return;
	if(!other->client)
		return;
	if(other->health < 1)
		return;					// dead people can't pickup
	if(other->client->objItem != NULL)	// Player is already carrying another item.
		return;

	// Check if this player is on the right team.
	if(other->client->ps.persistant[PERS_TEAM] == TEAM_RED)
	{
		if(ent->blue_only)
		{
			if(ent->crusher)
			{
				// Touching enemy objective returns it.
				Return_Obj_Item(ent, other);
			}
			return;
		}
	}
	else if(other->client->ps.persistant[PERS_TEAM] == TEAM_BLUE)
	{
		if(ent->red_only)
		{
			if(ent->crusher)
			{
				// Touching enemy objective returns it.
				Return_Obj_Item(ent, other);
			}
			return;
		}
	}
	else
	{
		return;
	}

	// Player is able to pick this up.

	predict = other->client->pers.predictItemPickup;

	if(other->client->ps.persistant[PERS_TEAM] == TEAM_RED)
	{
		pteam = TEAM_RED;
	}
	else
	{
		pteam = TEAM_BLUE;
	}
	G_ObjectiveAnnounce(pteam, va("%s" S_COLOR_WHITE " picked up %s!", other->client->pers.netname, ent->message));

	ent->s.otherEntityNum = other->s.number;
	other->client->objItem = ent;
	ent->crusher = qtrue; // No longer at original position


	// play the normal pickup sound
	if(predict)
	{
		G_AddPredictableEvent(other, EV_OBJ_ITEM_PICKUP, ent->s.modelindex);
	}
	else
	{
		G_AddEvent(other, EV_OBJ_ITEM_PICKUP, ent->s.modelindex);
	}

	// fire item targets
	G_UseTargets(ent, other);
}

/*QUAKED team_OBJ_captureitem (0 0 1) (-24 -24 -24) (24 24 24)
Capturable item.
*/
const vec3_t    capItemMins = { -24, -24, -24 };
const vec3_t    capItemMaxs = { 24, 24, 24 };

void SP_team_OBJ_captureitem(gentity_t * ent)
{
	VectorCopy(ent->s.origin, ent->s.pos.trBase);
	VectorCopy(ent->s.origin, ent->r.currentOrigin);

	VectorCopy(ent->s.angles, ent->s.apos.trBase);
	VectorCopy(ent->s.angles, ent->r.currentAngles);

	VectorCopy(capItemMins, ent->r.mins);
	VectorCopy(capItemMaxs, ent->r.maxs);

	VectorCopy(ent->s.origin, ent->pos1);
	ent->r.contents = CONTENTS_TRIGGER;

	if(ent->wait == 0)
	{
		// Default, return after 30 seconds on the ground.
		ent->wait = 30;
	}

	ent->s.eType = ET_CAPTURE_ITEM;

	ent->r.svFlags = SVF_BROADCAST;

	G_SpawnBoolean("red_only", "0", &ent->red_only);
	G_SpawnBoolean("blue_only", "0", &ent->blue_only);

	if(ent->gamemodel != NULL)
	{
		ent->s.modelindex = G_ModelIndex( ent->gamemodel );
	}
	else
	{
		G_Printf("Warnings: team_OBJ_captureitem at %s has no gamemodel.", vtos(ent->pos1));
	}

	// No need to auto return
	ent->nextthink = 0;
	ent->think = NULL;
	ent->crusher = qfalse;
	// crusher if it is not at original location.

	ent->touch = Touch_Obj_Item;

	Obj_Item_ReadyPickup(ent);

	trap_LinkEntity(ent);
}

/*
================
G_RunObjItem
Based on G_RunItem
================
*/
void G_RunObjItem(gentity_t * ent)
{
	vec3_t          origin;
	trace_t         tr;
	int             contents;
	int             mask;

	// if groundentity has been set to -1, it may have been pushed off an edge
	if(ent->s.groundEntityNum == -1)
	{
		if(ent->s.pos.trType != TR_GRAVITY)
		{
			ent->s.pos.trType = TR_GRAVITY;
			ent->s.pos.trAcceleration = -g_gravityZ.value;
			ent->s.pos.trTime = level.time;
		}
	}

	if(ent->s.pos.trType == TR_STATIONARY)
	{
		// check think function
		G_RunThink(ent);
		return;
	}

	// get current position
	BG_EvaluateTrajectory(&ent->s.pos, level.time, origin);

	// trace a line from the previous position to the current position
	if(ent->clipmask)
	{
		mask = ent->clipmask;
	}
	else
	{
		mask = MASK_PLAYERSOLID & ~CONTENTS_BODY;	//MASK_SOLID;
	}
	trap_Trace(&tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, ent->r.ownerNum, mask);

	VectorCopy(tr.endpos, ent->r.currentOrigin);

	if(tr.startsolid)
	{
		tr.fraction = 0;
	}

	trap_LinkEntity(ent);		// FIXME: avoid this for stationary?

	// check think function
	G_RunThink(ent);

	if(tr.fraction == 1)
	{
		return;
	}

	// if it is in a nodrop volume, return it
	contents = trap_PointContents(ent->r.currentOrigin, -1);
	if(contents & CONTENTS_NODROP)
	{
		Return_Obj_Item(ent, NULL);
		return;
	}

	G_BounceItem(ent, &tr);
}

/*
==============================================================================

team_OBJ_capturepoint

==============================================================================
*/

/*
================
multi_capturepoint_trigger
Based on: multi_flagonly_trigger
the trigger was just activated
================
*/
void multi_capturepoint_trigger(gentity_t * ent, gentity_t * activator)
{
	team_t          capTeam = TEAM_FREE;
	static const char objDefaultName[] = "the objective";
	const char     *objName;
	ent->activator = activator;

	if(!activator->client)
	{
		return;
	}

	if(activator->client->objItem == NULL)
	{
		// Not carying anything
		return;
	}

	// Check if this player is on the right team.
	if(activator->client->ps.persistant[PERS_TEAM] == TEAM_RED)
	{
		if(ent->blue_only)
		{
			return;
		}
		capTeam = TEAM_RED;
	}
	else if(activator->client->ps.persistant[PERS_TEAM] == TEAM_BLUE)
	{
		if(ent->red_only)
		{
			return;
		}
		capTeam = TEAM_BLUE;
	}

	if(activator->client->objItem->message != NULL)
	{
		objName = activator->client->objItem->message;
	}
	else
	{
		objName = objDefaultName;
	}

	G_ObjectiveAnnounce(capTeam, va("%s" S_COLOR_WHITE " captured %s!", activator->client->pers.netname, objName));

	G_UseTargets(ent, ent->activator);

#ifdef G_LUA
	// Lua API callbacks
	if(ent->luaUse)
	{
		if(activator)
		{
			G_LuaHook_EntityUse(ent->luaUse, ent->s.number, activator->client->objItem->s.number, activator->s.number);
		}
		else
		{
			G_LuaHook_EntityUse(ent->luaUse, ent->s.number, ENTITYNUM_WORLD, ENTITYNUM_WORLD);
		}
	}
#endif

	// Client is no logner carrying the objective
	activator->client->objItem = NULL;
}
/*
================
CapturePoint_Multi
Based on Touch_Flagonly_Multi
================
*/
void CapturePoint_Multi(gentity_t * self, gentity_t * other, trace_t * trace)
{
	if(!other->client)
	{
		return;
	}
	multi_capturepoint_trigger(self, other);
}

/*QUAKED SP_team_OBJ_capturepoint (.5 .5 .5) ?
Player must be carrying the appropriate flag for it to trigger.
Either red_only or blue_only must be set.
*/
void SP_team_OBJ_capturepoint(gentity_t * ent)
{
	G_SpawnBoolean("red_only", "0", &ent->red_only);
	G_SpawnBoolean("blue_only", "0", &ent->blue_only);

	ent->touch = CapturePoint_Multi;

	if(!VectorCompare(ent->s.angles, vec3_origin))
		G_SetMovedir(ent->s.angles, ent->movedir);

	if(strstr(ent->model, ".ase") || strstr(ent->model, ".lwo") || strstr(ent->model, ".prt"))
	{
		// don't set brush model
	}
	else
	{
		trap_SetBrushModel(ent, ent->model);
	}

	ent->r.contents = CONTENTS_TRIGGER;	// replaces the -1 from trap_SetBrushModel
	ent->r.svFlags = SVF_BROADCAST;

	trap_LinkEntity(ent);
}
