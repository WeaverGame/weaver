/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// g_weapon.c
// perform the server side effects of a weapon firing

#include "g_local.h"

static float    s_quadFactor;
static vec3_t   forward, right, up;
static vec3_t   muzzle;

#define NUM_NAILSHOTS 15

/*
================
G_BounceProjectile
================
*/
void G_BounceProjectile(vec3_t start, vec3_t impact, vec3_t dir, vec3_t endout)
{
	vec3_t          v, newv;
	float           dot;

	VectorSubtract(impact, start, v);
	dot = DotProduct(v, dir);
	VectorMA(v, -2 * dot, dir, newv);

	VectorNormalize(newv);
	VectorMA(impact, 8192, newv, endout);
}


/*
======================================================================

GAUNTLET

======================================================================
*/

void Weapon_Gauntlet(gentity_t * ent)
{

}

/*
===============
CheckGauntletAttack
===============
*/
qboolean CheckGauntletAttack(gentity_t * ent)
{
	trace_t         tr;
	vec3_t          end;
	gentity_t      *tent;
	gentity_t      *traceEnt;
	int             damage;

	// set aiming directions
	AngleVectors(ent->client->ps.viewangles, forward, right, up);

	CalcMuzzlePoint(ent, forward, right, up, muzzle, WP_GAUNTLET, qfalse);

	VectorMA(muzzle, 32, forward, end);

	trap_Trace(&tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT);
	if(tr.surfaceFlags & SURF_NOIMPACT)
	{
		return qfalse;
	}

	traceEnt = &g_entities[tr.entityNum];

	// send blood impact
	if(traceEnt->takedamage && traceEnt->client)
	{
		tent = G_TempEntity(tr.endpos, EV_PROJECTILE_HIT);
		tent->s.otherEntityNum = traceEnt->s.number;
		tent->s.eventParm = DirToByte(tr.plane.normal);
		tent->s.weapon = ent->s.weapon;
	}

	if(!traceEnt->takedamage)
	{
		return qfalse;
	}

	if(ent->client->ps.powerups[PW_QUAD])
	{
		G_AddEvent(ent, EV_POWERUP_QUAD, 0);
		s_quadFactor = g_quadfactor.value;
	}
	else
	{
		s_quadFactor = 1;
	}
#ifdef MISSIONPACK
	if(ent->client->persistantPowerup && ent->client->persistantPowerup->item &&
	   ent->client->persistantPowerup->item->giTag == PW_DOUBLER)
	{
		s_quadFactor *= 2;
	}
#endif

	damage = 50 * s_quadFactor;
	G_Damage(traceEnt, ent, ent, forward, tr.endpos, damage, 0, MOD_GAUNTLET);

	return qtrue;
}

/*
===============
LogAccuracyHit
===============
*/
qboolean LogAccuracyHit(gentity_t * target, gentity_t * attacker)
{
	if(!target->takedamage)
	{
		return qfalse;
	}

	if(target == attacker)
	{
		return qfalse;
	}

	if(!target->client)
	{
		return qfalse;
	}

	if(!attacker->client)
	{
		return qfalse;
	}

	if(target->client->ps.stats[STAT_HEALTH] <= 0)
	{
		return qfalse;
	}

	if(OnSameTeam(target, attacker))
	{
		return qfalse;
	}

	return qtrue;
}


/*
===============
CalcMuzzlePoint

set muzzle location relative to pivoting eye
===============
*/
void CalcMuzzlePoint(gentity_t * ent, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint, int weapon, qboolean secondary)
{
	vec3_t          surfNormal;
	vec3_t          offset;
	vec3_t          end;

	if(ent->client)
	{
		if(ent->client->ps.pm_flags & PMF_WALLCLIMBING)
		{
			if(ent->client->ps.pm_flags & PMF_WALLCLIMBINGCEILING)
				VectorSet(surfNormal, 0.0f, 0.0f, -1.0f);
			else
				VectorCopy(ent->client->ps.grapplePoint, surfNormal);
		}
		else
		{
			VectorSet(surfNormal, 0.0f, 0.0f, 1.0f);
		}

		VectorMA(ent->client->ps.origin, ent->client->ps.viewheight, surfNormal, muzzlePoint);
	}
	else
	{
		VectorCopy(ent->s.pos.trBase, muzzlePoint);
	}

	switch (weapon)
	{
			//TODO: try this code instead
			/*
			AngleVectors(ent->client->ps.viewangles, forward, right, up);

			VectorSet(offset, 14, 8, -16);
			G_ProjectSource(muzzlePoint, offset, forward, right, muzzlePoint);
			break;
			*/

		default:
			AngleVectors(ent->client->ps.viewangles, forward, right, up);

#if 0
			VectorMA(muzzlePoint, 1, forward, muzzlePoint);
			VectorMA(muzzlePoint, 1, right, muzzlePoint);
#else
			VectorMA(muzzlePoint, 14, forward, muzzlePoint);
#endif
			break;
	}

	// HACK: correct forward vector for the projectile so it will fly towards the crosshair
	VectorMA(muzzlePoint, 8192, forward, end);
	VectorSubtract(end, muzzlePoint, forward);
	VectorNormalize(forward);

	// snap to integer coordinates for more efficient network bandwidth usage
	SnapVector(muzzlePoint);
}

/*
===============
CalcMuzzlePointOrigin

set muzzle location relative to pivoting eye
===============
*/
/*
void CalcMuzzlePointOrigin(gentity_t * ent, vec3_t origin, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint)
{
	VectorCopy(ent->s.pos.trBase, muzzlePoint);
	muzzlePoint[2] += ent->client->ps.viewheight;
	VectorMA(muzzlePoint, 1, forward, muzzlePoint);
	VectorMA(muzzlePoint, 1, right, muzzlePoint);

	// snap to integer coordinates for more efficient network bandwidth usage
	SnapVector(muzzlePoint);
}
*/


/*
===============
FireWeapon
===============
*/
void FireWeapon(gentity_t * ent)
{
	if(ent->client->ps.powerups[PW_QUAD])
	{
		s_quadFactor = g_quadfactor.value;
	}
	else
	{
		s_quadFactor = 1;
	}

	// track shots taken for accuracy tracking.  Grapple is not a weapon and gauntlet is just not tracked
	if(ent->s.weapon != WP_GAUNTLET)
	{
		ent->client->accuracy_shots++;
	}

	if(ent->client)
	{
		// set aiming directions
		CalcMuzzlePoint(ent, forward, right, up, muzzle, ent->s.weapon, qfalse);
	}
	else
	{
		AngleVectors(ent->s.angles2, forward, right, up);
		VectorCopy(ent->s.pos.trBase, muzzle);
	}

	// fire the specific weapon
	switch (ent->s.weapon)
	{
		case WP_GAUNTLET:
			Weapon_Gauntlet(ent);
			break;
		default:
			G_Error( "Bad ent->s.weapon - see g_weapon.c FireWeapon(ent)" );
			break;
	}
}


/*
===============
FireWeapon2
===============
*/
void FireWeapon2(gentity_t * ent)
{
	if(ent->client->ps.powerups[PW_QUAD])
	{
		s_quadFactor = g_quadfactor.value;
	}
	else
	{
		s_quadFactor = 1;
	}


#if 0
	// track shots taken for accuracy tracking.  Grapple is not a weapon and gauntlet is just not tracked
	if(ent->s.weapon != WP_GAUNTLET)
	{
		ent->client->accuracy_shots++;
	}
#endif

	if(ent->client)
	{
		// set aiming directions
		CalcMuzzlePoint(ent, forward, right, up, muzzle, ent->s.weapon, qtrue);
	}
	else
	{
		AngleVectors(ent->s.angles2, forward, right, up);
		VectorCopy(ent->s.pos.trBase, muzzle);
	}

	// fire the specific weapon
	switch (ent->s.weapon)
	{
		default:
			break;
	}
}
