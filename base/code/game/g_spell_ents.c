/*
===========================================================================
This file is part of the weaver entity support.

These are misc weaver entities
===========================================================================
*/

#include "g_local.h"

/*
===============================================================================

SHIELD

===============================================================================
*/

void func_shield_ActivateUse(gentity_t * ent, gentity_t * other, qboolean firstActivate)
{
	if(ent->s.generic1 > 0)
	{
		//expend shield
		ent->s.generic1--;
	}
	Com_Printf("ShieldUsed: %d\n", ent->s.generic1);
	if(ent->s.generic1 <= 0)
	{
		ent->die(ent, NULL, other, 0, 0);
	}
}

void func_shield_die(gentity_t * self, gentity_t * inflictor, gentity_t * attacker, int damage, int mod)
{
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
		G_Printf("func_shield at %s with an unfound target\n", vtos(ent->r.absmin));
		return;
	}
	G_Printf("func_shield at %s targeting %s\n", vtos(ent->r.absmin), ent->target);

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

	G_SpawnInt("power", "1000", &ent->s.generic1);

	//Full shield
	ent->s.torsoAnim = ent->s.generic1;

	ent->s.eType = ET_SHIELD;

	// start trains on the second frame, to make sure their targets have had
	// a chance to spawn
	ent->nextthink = level.time + FRAMETIME;
	ent->think = Think_SetupShieldTargets;

	ent->activate = func_shield_ActivateUse;

	ent->takedamage = qfalse;

	ent->die = func_shield_die;

	trap_LinkEntity(ent);
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
		G_Printf("func_shield_info at %s with an unfound target (MAPPER: GO FIX THIS ENTITY)\n", vtos(ent->r.absmin));
		return;
	}
	G_Printf("func_shield_info at %s targeting %s\n", vtos(ent->r.absmin), ent->target);

	ent->s.otherEntityNum2 = ent->target_ent->s.number;

	if(VectorLength(ent->s.angles) == 0)
	{
		VectorSubtract(ent->s.origin, ent->target_ent->s.origin, ent->s.angles);
		VectorNormalize(ent->s.angles);
	}

	// Start at shield
	shieldInfoChain = ent->target_ent;
	// If shield already has a shield_info attached, chain to the shield_info
	while(shieldInfoChain->shield_ent)
	{
		// Follow chain
		shieldInfoChain = shieldInfoChain->shield_ent;
		if((shieldInfoChain->s.eType != ET_SHIELD_INFO) && (shieldInfoChain->s.eType != ET_SHIELD))
		{
			G_Printf("func_shield_info at %s found unexpected entity type (MAPPER: GO FIX THIS ENTITY)\n", vtos(ent->r.absmin));
			return;
		}
		i++; // Protect against entities which target in a loop
		if(i > 8)
		{
			G_Printf("func_shield_info at %s has a long chain, may be looped (MAPPER: GO FIX THIS ENTITY)\n", vtos(ent->r.absmin));
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

	// start trains on the second frame, to make sure their targets have had
	// a chance to spawn
	ent->nextthink = level.time + FRAMETIME;
	ent->think = Think_SetupShieldInfoTargets;

	ent->takedamage = qfalse;

	ent->die = func_shield_info_die;

	trap_LinkEntity(ent);
}
