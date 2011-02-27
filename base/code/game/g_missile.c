/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006-2009 Robert Beckebans <trebor_7@users.sourceforge.net>
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
//
#include "g_local.h"

#define	MISSILE_PRESTEP_TIME	50

/*
================
G_BounceMissile

================
*/
void G_BounceMissile(gentity_t * ent, trace_t * trace)
{
	vec3_t          velocity;
	float           dot;
	int             hitTime;

	// reflect the velocity on the trace plane
	hitTime = level.previousTime + (level.time - level.previousTime) * trace->fraction;
	BG_EvaluateTrajectoryDelta(&ent->s.pos, hitTime, velocity);
	dot = DotProduct(velocity, trace->plane.normal);
	VectorMA(velocity, -2 * dot, trace->plane.normal, ent->s.pos.trDelta);

	if(ent->s.eFlags & EF_BOUNCE_HALF)
	{
		VectorScale(ent->s.pos.trDelta, 0.65, ent->s.pos.trDelta);
		// check for stop
		if(trace->plane.normal[2] > 0.2 && VectorLength(ent->s.pos.trDelta) < 40)
		{
			G_SetOrigin(ent, trace->endpos);
			return;
		}
	}

	VectorAdd(ent->r.currentOrigin, trace->plane.normal, ent->r.currentOrigin);
	VectorCopy(ent->r.currentOrigin, ent->s.pos.trBase);
	ent->s.pos.trTime = level.time;
}


/*
================
G_ExplodeMissile

Explode a missile without an impact
================
*/
void G_ExplodeMissile(gentity_t * ent)
{
	vec3_t          dir;
	vec3_t          origin;

	BG_EvaluateTrajectory(&ent->s.pos, level.time, origin);
	SnapVector(origin);
	G_SetOrigin(ent, origin);

	// we don't have a valid direction, so just point straight up
	dir[0] = dir[1] = 0;
	dir[2] = 1;

	// Tr3B: don't change the entity type because it is required by the EV_PROJECTILE_* events
	// the ent->freeAfterEvent = qtrue; will do the same effect
	//ent->s.eType = ET_GENERAL;

	G_AddEvent(ent, EV_PROJECTILE_MISS, DirToByte(dir));

	ent->s.modelindex = 0;
	ent->freeAfterEvent = qtrue;

	// splash damage
	if(ent->splashDamage)
	{
		if(G_RadiusDamage(ent->r.currentOrigin, ent->parent, ent->splashDamage, ent->splashRadius, ent, ent->splashMethodOfDeath))
		{
			g_entities[ent->r.ownerNum].client->accuracy_hits++;
		}
	}

	trap_LinkEntity(ent);
}

/*
================
G_MissileImpact
================
*/
void G_MissileImpact(gentity_t * ent, trace_t * trace)
{
	gentity_t      *other;
	qboolean        hitClient = qfalse;

	other = &g_entities[trace->entityNum];

	// check for bounce
	if(!other->takedamage && (ent->s.eFlags & (EF_BOUNCE | EF_BOUNCE_HALF)))
	{
		G_BounceMissile(ent, trace);
		G_AddEvent(ent, EV_GRENADE_BOUNCE, 0);
		return;
	}

	// impact damage
	if(other->takedamage)
	{
		// FIXME: wrong damage direction?
		if(ent->damage)
		{
			vec3_t          velocity;

			if(LogAccuracyHit(other, &g_entities[ent->r.ownerNum]))
			{
				g_entities[ent->r.ownerNum].client->accuracy_hits++;
				hitClient = qtrue;
			}
			BG_EvaluateTrajectoryDelta(&ent->s.pos, level.time, velocity);
			if(VectorLength(velocity) == 0)
			{
				velocity[2] = 1;	// stepped on a grenade
			}

			G_Damage(other, ent, &g_entities[ent->r.ownerNum], velocity, ent->s.origin, ent->damage, 0, ent->methodOfDeath);
		}
	}

	// is it cheaper in bandwidth to just remove this ent and create a new
	// one, rather than changing the missile into the explosion?

	if(other->takedamage && other->client)
	{
		G_AddEvent(ent, EV_PROJECTILE_HIT, DirToByte(trace->plane.normal));
		ent->s.otherEntityNum = other->s.number;
	}
	else if(trace->surfaceFlags & SURF_METALSTEPS)
	{
		G_AddEvent(ent, EV_PROJECTILE_MISS_METAL, DirToByte(trace->plane.normal));
	}
	else
	{
		G_AddEvent(ent, EV_PROJECTILE_MISS, DirToByte(trace->plane.normal));
	}

	ent->freeAfterEvent = qtrue;

	// change over to a normal entity right at the point of impact
	ent->s.eType = ET_GENERAL;

	SnapVectorTowards(trace->endpos, ent->s.pos.trBase);	// save net bandwidth

	G_SetOrigin(ent, trace->endpos);

	// splash damage (doesn't apply to person directly hit)
	if(ent->splashDamage)
	{
		if(G_RadiusDamage(trace->endpos, ent->parent, ent->splashDamage, ent->splashRadius, other, ent->splashMethodOfDeath))
		{
			if(!hitClient)
			{
				g_entities[ent->r.ownerNum].client->accuracy_hits++;
			}
		}
	}

	trap_LinkEntity(ent);
}

/*
================
G_RunMissile
================
*/
void G_RunMissile(gentity_t * ent)
{
	vec3_t          origin;
	trace_t         tr;
	int             passent;

	// get current position
	BG_EvaluateTrajectory(&ent->s.pos, level.time, origin);

	// if this missile bounced off an invulnerability sphere
	if(ent->target_ent)
	{
		passent = ent->target_ent->s.number;
	}
	else
	{
		// ignore interactions with the missile owner
		passent = ent->r.ownerNum;
	}

	// trace a line from the previous position to the current position
	trap_Trace(&tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passent, ent->clipmask);

	if(tr.startsolid || tr.allsolid)
	{
		// make sure the tr.entityNum is set to the entity we're stuck in
		trap_Trace(&tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, ent->r.currentOrigin, passent, ent->clipmask);
		tr.fraction = 0;
	}
	else
	{
		VectorCopy(tr.endpos, ent->r.currentOrigin);
	}

	trap_LinkEntity(ent);

	if(tr.fraction != 1)
	{
		// never explode or bounce on sky
		if(tr.surfaceFlags & SURF_NOIMPACT)
		{
			// If grapple, reset owner
			if(ent->parent && ent->parent->client && ent->parent->client->hook == ent)
			{
				ent->parent->client->hook = NULL;
			}
			G_FreeEntity(ent);
			return;
		}

		G_MissileImpact(ent, &tr);

		if(ent->s.eType != ET_PROJECTILE && ent->s.eType != ET_PROJECTILE2)
		{
			return;				// exploded
		}
	}

	// otty: added
	G_TouchTriggers(ent);

	// check think function after bouncing
	G_RunThink(ent);
}

/*
================
G_Missile_Die
================
*/
static void G_Missile_Die(gentity_t * ent, gentity_t * inflictor, gentity_t * attacker, int damage, int mod)
{
	ent->nextthink = level.time + 1;
	ent->think = G_ExplodeMissile;
}


//=============================================================================
