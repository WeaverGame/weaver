/*
===========================================================================
This file is part of the weaver game.

It creates projectiles for weaves and other weave effects.
===========================================================================
*/
#include "g_local.h"
#include "g_spell_effects.h"

qboolean G_IsTeamGame()
{
	if(g_gametype.integer >= GT_TEAM)
	{
		return qtrue;
	}
	else
	{
		return qfalse;
	}
}

void G_KnockClient(gentity_t * targ, vec3_t dir, float knockback)
{
	if(knockback && targ->client)
	{
		vec3_t          kvel;
		float           mass;

		mass = KNOCKBACK_PLAYER_MASS;

		VectorScale(dir, knockback / mass, kvel);
		VectorAdd(targ->client->ps.velocity, kvel, targ->client->ps.velocity);

		// set the timer so that the other client can't cancel
		// out the movement immediately
		if(!targ->client->ps.pm_time)
		{
			int             t;

			t = knockback * 2;
			if(t < KNOCKBACK_TIME_MIN)
			{
				t = KNOCKBACK_TIME_MIN;
			}
			if(t > KNOCKBACK_TIME_MAX)
			{
				t = KNOCKBACK_TIME_MAX;
			}
			targ->client->ps.pm_time = t;
			targ->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
		}
	}
}

void RunWeave_MoveToTarget(gentity_t * ent)
{
	//weave effect stays with target
	G_SetOrigin(ent, ent->target_ent->r.currentOrigin);
	trap_LinkEntity(ent);
}

/*
================
RunWeave
Impact

Based off
G_MissileImpact
================
*/
void RunWeave_Impact(gentity_t * ent, trace_t * trace)
{
	gentity_t      *other;
	qboolean        hitClient = qfalse;

	other = &g_entities[trace->entityNum];

	// impact damage
	if(other->takedamage)
	{
		vec3_t          velocity;
		vec3_t          direction;

		if(ent->knockback)
		{
			VectorCopy(ent->s.pos.trDelta, direction);
			G_KnockClient(other, direction, ent->knockback);
		}

		if(ent->damage)
		{
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
		G_AddEvent(ent, EV_WEAVEMISSILE_HIT, DirToByte(trace->plane.normal));
		ent->s.otherEntityNum = other->s.number;
	}
	else
	{
		G_AddEvent(ent, EV_WEAVEMISSILE_MISS, DirToByte(trace->plane.normal));
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

void RunWeave_Explode(gentity_t * ent)
{
	G_AddEvent(ent, EV_WEAVEMISSILE_MISS, DirToByte(ent->s.pos.trDelta));
	ent->freeAfterEvent = qtrue;
	ent->s.eType = ET_GENERAL;

	trap_LinkEntity(ent);
}

/*
=================
RunWeave_Missile

Generic to many missile weaves
=================
*/
void RunWeave_Missile(gentity_t * ent)
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
			G_FreeEntity(ent);
			return;
		}

		//Custom impact
		if(ent->impact)
		{
			ent->impact(ent, &tr);
		}
		else
		{
			//Default, for explosion+mark
			RunWeave_Impact(ent, &tr);
		}

		if(ent->s.eType != ET_WEAVE_MISSILE)
		{
			return;				// exploded
		}
	}

	// otty: added
	G_TouchTriggers(ent);

	G_RunThink(ent);
}

void Shard_Think(gentity_t * ent)
{
	vec3_t          velocity;

	BG_EvaluateTrajectoryDelta(&ent->s.pos, level.time, velocity);

	//Com_Printf("Velocity:%f\n", VectorLength(velocity)); 

	if(VectorLength(velocity) <= WEAVE_ICESHARDS_STOP_SPEED)
	{
		G_FreeEntity(ent);
		return;
	}
	else
	{
		ent->count--;
		ent->nextthink = level.time + TIME_ICESHARD_THINK;
	}

	if(ent->count <= 0)
	{
		G_FreeEntity(ent);
	}
}

team_t EnemyTeamOf(team_t thisTeam)
{
	if(thisTeam == TEAM_BLUE)
	{
		return TEAM_RED;
	}
	else
	{
		return TEAM_BLUE;
	}
}

qboolean GetTargetPlayerMasked(vec3_t start, vec3_t dir, int length, int passent, team_t team, int *result, int contentmask)
{
	vec3_t          end;
	trace_t         trace;
	gentity_t      *traceEnt;

	VectorMA(start, length, dir, end);

	trap_Trace(&trace, start, NULL, NULL, end, passent, contentmask);
	if(trace.entityNum < ENTITYNUM_MAX_NORMAL)
	{
		traceEnt = &g_entities[trace.entityNum];
	}
	else
	{
		return qfalse;
	}
	//Com_Printf("GETTARGETPLAYER: TargetEnt=%d\n",traceEnt->s.number);
	//Com_Printf("GETTARGETPLAYER: TargetTeam=%d, TargetContents=%d\n",traceEnt->client->sess.sessionTeam, trace.contents);
	if(trace.contents & CONTENTS_SOLID)
	{
		return qfalse;
	}
	if((team != TEAM_FREE) && (traceEnt->client->sess.sessionTeam != team))
	{
		//If a team is selected, only continue if it is correct
		return qfalse;
	}

	*result = traceEnt->s.number;
	//Com_Printf("GETTARGETPLAYER: returning true\n");
	return qtrue;
}

/*
=================
GetTargetPlayer

Finds the player in the crosshair
=================
*/
qboolean GetTargetPlayer(vec3_t start, vec3_t dir, int length, int passent, team_t team, int *result)
{
	return GetTargetPlayerMasked(start, dir, length, passent, team, result, MASK_PLAYERSOLID);
}

//Begin AirBlast
/*
=================
FireWeave
AirBlast
=================
*/
qboolean FireWeave_AirBlast(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	gentity_t      *bolt;

	VectorNormalize(dir);

	bolt = G_Spawn();
	bolt->classname = EFFECT_CLASSNAME;
	bolt->nextthink = level.time + WEAVE_AIRBLAST_TIME;
	bolt->think = G_FreeEntity;
	bolt->s.eType = ET_WEAVE_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WVW_A_AIR_BLAST;
	bolt->r.ownerNum = self->s.number;
	//unlagged - projectile nudge
	// we'll need this for nudging projectiles later
	//bolt->s.otherEntityNum = self->s.number;
	//unlagged - projectile nudge
	bolt->s.otherEntityNum2 = heldWeaveNum;
	bolt->parent = self;
	bolt->damage = WEAVE_AIRBLAST_DAMAGE;
	bolt->splashDamage = WEAVE_AIRBLAST_SPLASH_DAMAGE;
	bolt->splashRadius = WEAVE_AIRBLAST_SPLASH;
	bolt->methodOfDeath = MOD_A_AIR_BLAST;
	bolt->splashMethodOfDeath = MOD_A_AIR_BLAST;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;
	bolt->knockback = WEAVE_AIRBLAST_KNOCK;

	bolt->s.pos.trType = TR_LINEAR;
	VectorScale(dir, WEAVE_AIRBLAST_SPEED, bolt->s.pos.trDelta);

	bolt->s.pos.trTime = level.time - WEAVE_AIRBLAST_START;	// move a bit on the very first frame
	VectorCopy(start, bolt->s.pos.trBase);

	SnapVector(bolt->s.pos.trDelta);	// save net bandwidth
	VectorCopy(start, bolt->r.currentOrigin);

	//weave = bolt;
	return qtrue;
}

//End AirBlast

//Begin Fireball
/*
=================
FireWeave
Fireball
=================
*/
qboolean FireWeave_Fireball(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	gentity_t      *bolt;

	VectorNormalize(dir);

	bolt = G_Spawn();
	bolt->classname = EFFECT_CLASSNAME;
	bolt->nextthink = level.time + TIME_PROJECTILE_EXPIRE;
	bolt->think = RunWeave_Explode;
	bolt->s.eType = ET_WEAVE_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WVW_A_FIRE_BALL;
	bolt->r.ownerNum = self->s.number;
//unlagged - projectile nudge
	// we'll need this for nudging projectiles later
	//bolt->s.otherEntityNum = self->s.number;
//unlagged - projectile nudge
	bolt->s.otherEntityNum2 = heldWeaveNum;
	bolt->parent = self;
	bolt->damage = WEAVE_FIREBALL_DAMAGE;
	bolt->splashDamage = WEAVE_FIREBALL_SPLASH_DAMAGE;
	bolt->splashRadius = WEAVE_FIREBALL_SPLASH;
	bolt->methodOfDeath = MOD_A_FIRE_BALL;
	bolt->splashMethodOfDeath = MOD_A_FIRE_BALL;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;

	bolt->s.pos.trType = TR_LINEAR;
	VectorScale(dir, WEAVE_FIREBALL_SPEED, bolt->s.pos.trDelta);

	bolt->s.pos.trTime = level.time - WEAVE_FIREBALL_START;	// move a bit on the very first frame
	VectorCopy(start, bolt->s.pos.trBase);

	SnapVector(bolt->s.pos.trDelta);	// save net bandwidth
	VectorCopy(start, bolt->r.currentOrigin);

	//weave = bolt;
	return qtrue;
}

//End Fireball

//Begin FireDarts
qboolean FireWeave_FireDartsBase(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum, int weaveID)
{
	trace_t         tr;
	vec3_t          end;

	gentity_t      *tent;
	gentity_t      *traceEnt;

	VectorMA(start, 8192 * 16, dir, end);

	trap_Trace(&tr, start, NULL, NULL, end, self->s.number, MASK_SHOT);

	traceEnt = &g_entities[tr.entityNum];

	// snap the endpos to integers, but nudged towards the line
	SnapVectorTowards(tr.endpos, start);

	if(traceEnt->takedamage && traceEnt->client)
	{
		if(LogAccuracyHit(traceEnt, self))
		{
			self->client->accuracy_hits++;
		}
	}

	if(traceEnt->takedamage)
	{
		//TODO: change mod per weaveID
		G_Damage(traceEnt, self, self, dir, tr.endpos, WEAVE_FIREDARTS_DAMAGE, 0, MOD_MACHINEGUN);
	}

	VectorCopy(tr.endpos, end);
	SnapVector(end);
	tent = G_TempEntity(start, EV_WEAVE_SHOT);
	VectorCopy(end, tent->s.apos.trBase);
	tent->s.otherEntityNum = self->s.number;
	tent->s.weapon = weaveID;
	//weave = tent;
	return qtrue;
}

qboolean FireWeave_FireMultiDarts(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	return FireWeave_FireDartsBase(self, start, dir, heldWeaveNum, WVW_A_FIRE_MULTIDARTS);
}

qboolean FireWeave_FireDarts(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	return FireWeave_FireDartsBase(self, start, dir, heldWeaveNum, WVW_A_FIRE_DARTS);
}

//End FireDarts

//Begin Protects
/*
=================
EndWeave
Protects
=================
*/
gentity_t      *EndWeave_ProtectAir(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	gentity_t      *heldWeave;

	heldWeave = &g_entities[heldWeaveNum];

	//Remove shield effect
	G_FreeEntity(heldWeave->target_ent);

	//Clear shield 
	self->client->ps.stats[STAT_AIRPROTECT] = 0;
	self->client->protectHeldAir = NULL;

	return heldWeave;
}

gentity_t      *EndWeave_ProtectFire(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	gentity_t      *heldWeave;

	heldWeave = &g_entities[heldWeaveNum];

	//Remove shield effect
	G_FreeEntity(heldWeave->target_ent);

	//Clear shield 
	self->client->ps.stats[STAT_FIREPROTECT] = 0;
	self->client->protectHeldFire = NULL;

	return heldWeave;
}

gentity_t      *EndWeave_ProtectEarth(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	gentity_t      *heldWeave;

	heldWeave = &g_entities[heldWeaveNum];

	//Remove shield effect
	G_FreeEntity(heldWeave->target_ent);

	//Clear shield 
	self->client->ps.stats[STAT_EARTHPROTECT] = 0;
	self->client->protectHeldEarth = NULL;

	return heldWeave;
}

gentity_t      *EndWeave_ProtectWater(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	gentity_t      *heldWeave;

	heldWeave = &g_entities[heldWeaveNum];

	//Remove shield effect
	G_FreeEntity(heldWeave->target_ent);

	//Clear shield 
	self->client->ps.stats[STAT_WATERPROTECT] = 0;
	self->client->protectHeldWater = NULL;

	return heldWeave;
}

/*
=================
FireWeave
Protects
=================
*/
qboolean FireWeave_Protect(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum, int protectTime, int weaveID)
{
	gentity_t      *bolt;
	gentity_t      *heldWeave;

	heldWeave = &g_entities[heldWeaveNum];

	VectorNormalize(dir);

	bolt = G_Spawn();
	bolt->classname = EFFECT_CLASSNAME;
	bolt->nextthink = level.time + protectTime;
	bolt->think = G_ReleaseWeave;
	bolt->s.eType = ET_WEAVE_EFFECT;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = weaveID;
	bolt->r.ownerNum = self->s.number;
//unlagged - projectile nudge
	// we'll need this for nudging projectiles later
	bolt->s.generic1 = self->client->ps.clientNum;
//unlagged - projectile nudge
	bolt->s.otherEntityNum2 = heldWeaveNum;
	bolt->parent = self;
	bolt->damage = 0;
	bolt->splashDamage = 0;
	bolt->splashRadius = 0;
	bolt->methodOfDeath = MOD_UNKNOWN;	//Incapable of killing
	bolt->splashMethodOfDeath = MOD_UNKNOWN;	//Incapable of killing
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = self;
	bolt->freeAfterEvent = qfalse;

	if(weaveID == WVW_D_AIR_PROTECT)
	{
		self->client->ps.stats[STAT_AIRPROTECT] = WEAVE_PROTECTAIR;
		self->client->protectHeldAir = heldWeave;
	}
	if(weaveID == WVW_D_FIRE_PROTECT)
	{
		self->client->ps.stats[STAT_FIREPROTECT] = WEAVE_PROTECTFIRE;
		self->client->protectHeldFire = heldWeave;
	}
	if(weaveID == WVW_D_EARTH_PROTECT)
	{
		self->client->ps.stats[STAT_EARTHPROTECT] = WEAVE_PROTECTEARTH;
		self->client->protectHeldEarth = heldWeave;
	}
	if(weaveID == WVW_D_WATER_PROTECT)
	{
		self->client->ps.stats[STAT_WATERPROTECT] = WEAVE_PROTECTWATER;
		self->client->protectHeldWater = heldWeave;
	}

	trap_LinkEntity(bolt);

	//reference this from held
	heldWeave->target_ent = bolt;
	//held weave is now in progress
	heldWeave->s.frame = WST_INPROCESS;
	//prevent held weave expiring
	heldWeave->nextthink = 0;

	//weave = bolt;
	return qtrue;
}

qboolean FireWeave_ProtectAir(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	return FireWeave_Protect(self, start, dir, heldWeaveNum, WEAVE_PROTECTAIR_TIME, WVW_D_AIR_PROTECT);
}

qboolean FireWeave_ProtectFire(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	return FireWeave_Protect(self, start, dir, heldWeaveNum, WEAVE_PROTECTFIRE_TIME, WVW_D_FIRE_PROTECT);
}

qboolean FireWeave_ProtectEarth(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	return FireWeave_Protect(self, start, dir, heldWeaveNum, WEAVE_PROTECTEARTH_TIME, WVW_D_EARTH_PROTECT);
}

qboolean FireWeave_ProtectWater(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	return FireWeave_Protect(self, start, dir, heldWeaveNum, WEAVE_PROTECTWATER_TIME, WVW_D_WATER_PROTECT);
}

//End Air Protect

//Begin Slice
/*
=================
FireWeave
Slice
=================
*/
qboolean FireWeave_Slice(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum, int weaveID, int strength)
{
	gentity_t      *bolt;

	VectorNormalize(dir);

	bolt = G_Spawn();
	bolt->classname = EFFECT_CLASSNAME;
	bolt->nextthink = level.time + TIME_PROJECTILE_EXPIRE;
	bolt->think = G_FreeEntity;
	bolt->s.eType = ET_WEAVE_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = weaveID;
	bolt->r.ownerNum = self->s.number;
//unlagged - projectile nudge
	// we'll need this for nudging projectiles later
	//bolt->s.otherEntityNum = self->s.number;
//unlagged - projectile nudge
	bolt->s.torsoAnim = self->s.number;	//player just hit (skip this player)
	bolt->s.otherEntityNum2 = heldWeaveNum;
	bolt->parent = self;
	bolt->damage = strength;	//Highest tier this can slice
	bolt->splashDamage = 0;
	bolt->splashRadius = WEAVE_SLICE_SPREAD;	//Use for missile width
	bolt->methodOfDeath = MOD_UNKNOWN;	//Incapable of killing
	bolt->splashMethodOfDeath = MOD_UNKNOWN;	//Incapable of killing
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;

	bolt->s.pos.trType = TR_LINEAR;
	VectorScale(dir, WEAVE_SLICE_SPEED, bolt->s.pos.trDelta);

	bolt->s.pos.trTime = level.time - WEAVE_SLICE_START;	// move a bit on the very first frame
	VectorCopy(start, bolt->s.pos.trBase);

	SnapVector(bolt->s.pos.trDelta);	// save net bandwidth
	VectorCopy(start, bolt->r.currentOrigin);

	//Com_Printf("SLICE fired | ent %d \n", bolt->s.number);

	//weave = bolt;
	return qtrue;
}

qboolean FireWeave_SliceS(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	return FireWeave_Slice(self, start, dir, heldWeaveNum, WVW_A_SPIRIT_SLICE_S, WEAVE_SLICES_TIER);
}

qboolean FireWeave_SliceM(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	return FireWeave_Slice(self, start, dir, heldWeaveNum, WVW_A_SPIRIT_SLICE_M, WEAVE_SLICEM_TIER);
}

qboolean FireWeave_SliceL(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	return FireWeave_Slice(self, start, dir, heldWeaveNum, WVW_A_SPIRIT_SLICE_L, WEAVE_SLICEL_TIER);
}


void RunWeave_Slice(gentity_t * ent)
{
	vec3_t          origin;
	trace_t         tr;
	int             passent;
	int             num;
	int             touch[MAX_GENTITIES];
	gentity_t      *hit;
	gentity_t      *heldWeave;
	vec3_t          mins, maxs;
	int             i, j;
	static vec3_t   range;
	int             freeMe;

	range[0] = ent->splashRadius;
	range[1] = ent->splashRadius;
	range[2] = ent->splashRadius;

	freeMe = 0;

	// get current position
	BG_EvaluateTrajectory(&ent->s.pos, level.time, origin);

	// ignore interactions with the missile owner
	passent = ent->r.ownerNum;

	// trace a line from the previous position to the current position
	trap_Trace(&tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passent, ent->clipmask);

	VectorSubtract(ent->r.currentOrigin, range, mins);
	VectorAdd(ent->r.currentOrigin, range, maxs);

	// get nearby entities
	num = trap_EntitiesInBox(mins, maxs, touch, MAX_GENTITIES);

	// for each nearby entity
	for(i = 0; i < num; i++)
	{
		hit = &g_entities[touch[i]];
		// check if this entity is a weave
		if((hit->s.eType == ET_WEAVE_EFFECT || hit->s.eType == ET_WEAVE_MISSILE) && hit->r.ownerNum != ent->r.ownerNum)
		{
			Com_Printf("SLICE TRAP Hit weave | ent %d hit %d \n", ent->s.number, hit->s.number);

			// weave strength check
			if(ent->damage >= WeaveTier(hit->s.weapon))
			{
				heldWeave = &g_entities[hit->s.otherEntityNum2];
				if(hit->target_ent && hit->target_ent->s.number == ent->r.ownerNum)
				{
					Com_Printf("SLICE destroying weave (targeting self)\n");
				}
				if(heldWeave)
				{
					EndWeave(heldWeave);
					ClearHeldWeave(heldWeave);
				}
				G_FreeEntity(hit);
				freeMe = 1;
				Com_Printf("SLICE destroying weave\n");
			}
			else
			{
				Com_Printf("SLICE not powerfull enough\n");
			}
		}

		if(hit->s.eType == ET_PLAYER && hit->s.number != ent->r.ownerNum)
		{
			if(hit->s.number == ent->s.torsoAnim)
			{
				continue;
			}
			ent->s.torsoAnim = hit->s.number;

			Com_Printf("SLICE TRAP Hit player | ent %d hit %d \n", ent->s.number, hit->s.number);

			for(j = MIN_WEAPON_WEAVE; j < MAX_WEAPONS; j++)
			{
				if(hit->client->ps.ammo[j] > 0)
				{
					heldWeave = &g_entities[hit->client->ps.ammo[j]];
					//Must be: valid weave, in process, slice high enough to defeat this heldweave
					if(heldWeave && heldWeave->s.frame == WST_INPROCESS)
					{
						if(ent->damage >= WeaveTier(heldWeave->s.weapon))
						{
							if(heldWeave->target_ent && heldWeave->target_ent->target_ent &&
							   heldWeave->target_ent->target_ent->s.number == ent->r.ownerNum)
							{
								Com_Printf("SLICE destroying heldweave (targeting self)\n");
							}
							//End the held weave which is in process.
							UseHeldWeave(heldWeave);
							freeMe = 1;
							Com_Printf("SLICE destroying heldweave\n");
						}
						else
						{
							Com_Printf("SLICE not powerfull enough\n");
						}
					}
				}
			}
		}
	}
	if(freeMe)
	{
		G_FreeEntity(ent);
	}

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
			G_FreeEntity(ent);
			return;
		}
		if(ent->s.eType != ET_WEAVE_MISSILE)
		{
			return;				// exploded
		}
	}

	G_RunThink(ent);
}

//End Slice

//Begin Shield
/*
=================
EndWeave
Shield
=================
*/
gentity_t      *EndWeave_Shield(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	gentity_t      *heldWeave;

	heldWeave = &g_entities[heldWeaveNum];

	//Com_Printf("SHIELD FIRE: in progress, ending weave effect=%d \n", heldWeave->target_ent->s.number);

	G_FreeEntity(heldWeave->target_ent);

	heldWeave->target_ent = NULL;

	return heldWeave;
}

/*
=================
FireWeave
Shield
=================
*/
qboolean FireWeave_Shield(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	int             targetNum;
	gentity_t      *bolt;
	gentity_t      *target;
	team_t          targetTeam;
	gentity_t      *heldWeave;

	//Com_Printf("SHIELD FIRE: after declarations, heldweavenum=%d\n",heldWeaveNum);
	heldWeave = &g_entities[heldWeaveNum];

	VectorNormalize(dir);

	if(G_IsTeamGame())
	{
		targetTeam = EnemyTeamOf(self->client->sess.sessionTeam);	//enemies
	}
	else
	{
		targetTeam = TEAM_FREE;	//anyone
	}

	if(!GetTargetPlayer(start, dir, WEAVE_SHIELD_CAST_RANGE, self->s.number, targetTeam, &targetNum))
	{
		//Com_Printf("SHIELD FIRE: returning false (no target)\n");
		return qfalse;
	}
	target = &g_entities[targetNum];

	//Com_Printf("SHIELD FIRE: has target num=%d\n", targetNum);
	//Com_Printf("SHIELD FIRE: not in progress, Target=%d, maxclients=%d \n",target->s.number,level.maxclients);

	if(target->s.number >= level.maxclients || target->s.number < 0)
	{
		return qfalse;
	}

	bolt = G_Spawn();
	bolt->classname = EFFECT_CLASSNAME;
	bolt->nextthink = level.time + WEAVE_SHIELD_TIME;
	bolt->think = G_ReleaseWeave;
	bolt->s.eType = ET_WEAVE_EFFECT;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WVW_A_SPIRIT_SHIELD;
	bolt->r.ownerNum = self->s.number;
//unlagged - projectile nudge
	// we'll need this for nudging projectiles later
	bolt->s.otherEntityNum = target->s.number;
//unlagged - projectile nudge
	bolt->s.otherEntityNum2 = heldWeaveNum;
	bolt->parent = self;
	bolt->damage = 0;
	bolt->splashDamage = 0;
	bolt->splashRadius = 0;
	bolt->methodOfDeath = MOD_UNKNOWN;	//Incapable of killing
	bolt->splashMethodOfDeath = MOD_UNKNOWN;	//Incapable of killing
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = target;
	bolt->freeAfterEvent = qfalse;

	//reference this from held
	heldWeave->target_ent = bolt;
	//held weave is now in progress
	heldWeave->s.frame = WST_INPROCESS;
	//prevent held weave expiring
	heldWeave->nextthink = 0;

	//Com_Printf("SHIELD FIRE: shild spawned, EffectEnt = %d\n", bolt->s.number, bolt->s.otherEntityNum);
	RunWeave_Shield(bolt);

	//weave = bolt;
	return qtrue;
}

/*
=================
RunWeave
Shield
=================
*/
void RunWeave_Shield(gentity_t * ent)
{
	//apply shielding until next run
	ent->target_ent->client->ps.powerups[PW_SHIELDED] = level.time + WEAVE_SHIELD_PULSE_TIME;

	RunWeave_MoveToTarget(ent);

	//should probably copy trajectory too

	//Com_Printf("SHIELD RUN, leveltime=%d nextthink=%d free after event=%d\n", level.time, ent->nextthink, ent->freeAfterEvent);
	G_RunThink(ent);
}

//End Shield

//Begin Link
/*
=================
EndWeave
Link
=================
*/
gentity_t      *EndWeave_Link(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	gentity_t      *heldWeave;

	heldWeave = &g_entities[heldWeaveNum];

	//unlink
	ClientLinkLeave(self->client);

	return heldWeave;
}

/*
=================
FireWeave
Link
=================
*/
qboolean FireWeave_Link(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	int             targetNum;
	gentity_t      *target;
	team_t          targetTeam;
	gentity_t      *heldWeave;

	heldWeave = &g_entities[heldWeaveNum];

	if(G_IsTeamGame())
	{
		targetTeam = self->client->sess.sessionTeam;	//allies
	}
	else
	{
		targetTeam = TEAM_FREE;	//anyone
		//TODO: return?
	}

	VectorNormalize(dir);

	// scan for target
	if(!GetTargetPlayer(start, dir, WEAVE_SHIELD_CAST_RANGE, self->s.number, targetTeam, &targetNum))
	{
		return qfalse;
	}
	target = &g_entities[targetNum];

	// check target is valid
	if(target->s.number >= level.maxclients || target->s.number < 0)
	{
		return qfalse;
	}

	self->client->linkHeld = &g_entities[heldWeaveNum];

	// link
	if(!ClientLinkJoin(target->client, self->client))
	{
		return qfalse;
	}

	//weave = self->client->linkEnt;

	return qtrue;
}

//End Link

//Begin Grab Player
/*
=================
FireWeave
GrabPlayer
=================
*/
qboolean FireWeave_GrabPlayer(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	int             targetNum;
	gentity_t      *target;
	team_t          targetTeam;
	gentity_t      *heldWeave;
	gentity_t      *bolt;

	heldWeave = &g_entities[heldWeaveNum];

	targetTeam = TEAM_FREE;		//any

	VectorNormalize(dir);

	// scan for target
	if(!GetTargetPlayer(start, dir, WEAVE_GRABPLAYER_RANGE, self->s.number, targetTeam, &targetNum))
	{
		return qfalse;
	}
	target = &g_entities[targetNum];

	// check target is valid
	if(target->s.number >= level.maxclients || target->s.number < 0)
	{
		return qfalse;
	}

	bolt = G_Spawn();
	bolt->classname = EFFECT_CLASSNAME;
	bolt->nextthink = level.time + WEAVE_GRABPLAYER_TIME;
	bolt->think = G_ReleaseWeave;
	bolt->s.eType = ET_WEAVE_EFFECT;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WVW_A_AIR_GRABPLAYER;
	bolt->r.ownerNum = self->s.number;
//unlagged - projectile nudge
	// we'll need this for nudging projectiles later
	bolt->s.otherEntityNum = target->s.number;
//unlagged - projectile nudge
	bolt->s.otherEntityNum2 = heldWeaveNum;
	bolt->parent = self;
	bolt->damage = target->client->ps.pm_type;	//Remember previous pm_type
	bolt->splashDamage = 0;
	bolt->splashRadius = 0;
	bolt->methodOfDeath = MOD_UNKNOWN;	//Incapable of killing //TODO: work out a way to give kill if you drop a player off a cliff
	bolt->splashMethodOfDeath = MOD_UNKNOWN;	//Incapable of killing
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = target;
	bolt->freeAfterEvent = qfalse;

	//freeze target
	target->client->grabHolder = self;

	//reference this from held
	heldWeave->target_ent = bolt;
	//held weave is now in progress
	heldWeave->s.frame = WST_INPROCESS;
	//prevent held weave expiring
	heldWeave->nextthink = 0;

	//Com_Printf("GRABPLAYER FIRE: shild spawned, EffectEnt = %d\n", bolt->s.number, bolt->s.otherEntityNum);

	//weave = bolt;
	return qtrue;
}

/*
=================
RunWeave
GrabPlayer
=================
*/
void RunWeave_GrabPlayer(gentity_t * ent)
{
	vec3_t          offsetDir;
	vec3_t          playerOrigin;

	RunWeave_MoveToTarget(ent);

	if(ent->target_ent && ent->parent && ent->target_ent->client && ent->parent->client)
	{
		AngleVectors(ent->parent->client->ps.viewangles, offsetDir, NULL, NULL);
		VectorNormalize(offsetDir);

		VectorMA(ent->parent->client->ps.origin, 200, offsetDir, playerOrigin);

		//Move player
		VectorSubtract(playerOrigin, ent->target_ent->client->ps.origin, ent->target_ent->client->ps.velocity);

		//VectorCopy(playerOrigin, ent->target_ent->client->ps.origin);

		//Com_Printf("GRABPLAYER RUN, leveltime=%d nextthink=%d free after event=%d, pm_t=%d\n", level.time, ent->nextthink, ent->freeAfterEvent, ent->target_ent->client->ps.pm_type);
	}
	else
	{
		Com_Printf("GRABPLAYER RUN: Bad Entity Reference\n");
	}
	G_RunThink(ent);
}

/*
=================
EndWeave
GrabPlayer
=================
*/
gentity_t      *EndWeave_GrabPlayer(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	gentity_t      *heldWeave;
	gentity_t      *targetPlayer;

	heldWeave = &g_entities[heldWeaveNum];

	targetPlayer = heldWeave->target_ent->target_ent;

	//Free player
	targetPlayer->client->grabHolder = NULL;

	G_FreeEntity(heldWeave->target_ent);

	heldWeave->target_ent = NULL;

	return heldWeave;
}

//End GrabPlayer

//Begin Heals

/*
=================
FireWeave
Heal
=================
*/
qboolean FireWeave_Heal(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum, int weaveID, int healRate)
{
	int             targetNum;
	gentity_t      *bolt;
	gentity_t      *target;
	team_t          targetTeam;
	gentity_t      *heldWeave;

	heldWeave = &g_entities[heldWeaveNum];

	VectorNormalize(dir);


	if(G_IsTeamGame())
	{
		targetTeam = self->client->sess.sessionTeam;	//allies
	}
	else
	{
		targetTeam = TEAM_FREE;	//anyone
		//TODO: return?
	}

	if(!GetTargetPlayerMasked(start, dir, WEAVE_HEAL_CAST_RANGE, self->s.number, targetTeam, &targetNum, MASK_PLAYERSOLID | CONTENTS_CORPSE))
	{
		return qfalse;
	}
	target = &g_entities[targetNum];

	//Com_Printf("SHIELD FIRE: has target num=%d\n", targetNum);
	//Com_Printf("SHIELD FIRE: not in progress, Target=%d, maxclients=%d \n",target->s.number,level.maxclients);

	if(target->s.number >= level.maxclients || target->s.number < 0)
	{
		return qfalse;
	}

	bolt = G_Spawn();
	bolt->classname = EFFECT_CLASSNAME;
	bolt->nextthink = level.time + WEAVE_HEAL_TIME;
	bolt->think = G_ReleaseWeave;
	bolt->s.eType = ET_WEAVE_EFFECT;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = weaveID;
	bolt->r.ownerNum = self->s.number;
//unlagged - projectile nudge
	// we'll need this for nudging projectiles later
	bolt->s.otherEntityNum = target->s.number;
//unlagged - projectile nudge
	bolt->s.otherEntityNum2 = heldWeaveNum;
	bolt->parent = self;
	bolt->damage = healRate;
	bolt->splashDamage = healRate;
	bolt->splashRadius = 0;
	bolt->methodOfDeath = MOD_UNKNOWN;	//Incapable of killing
	bolt->splashMethodOfDeath = MOD_UNKNOWN;	//Incapable of killing
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = target;
	bolt->freeAfterEvent = qfalse;

	//reference this from held
	heldWeave->target_ent = bolt;
	//held weave is now in progress
	heldWeave->s.frame = WST_INPROCESS;
	//prevent held weave expiring
	heldWeave->nextthink = 0;

	//Com_Printf("SHIELD FIRE: shild spawned, EffectEnt = %d\n", bolt->s.number, bolt->s.otherEntityNum);

	RunWeave_Heal(bolt);

	//weave = bolt;
	return qtrue;
}

qboolean FireWeave_HealS(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	return FireWeave_Heal(self, start, dir, heldWeaveNum, WVW_D_WATER_HEAL_S, WEAVE_HEALS_DELAY);
}

qboolean FireWeave_HealM(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	return FireWeave_Heal(self, start, dir, heldWeaveNum, WVW_D_WATER_HEAL_M, WEAVE_HEALM_DELAY);
}

/*
=================
EndWeave
Heal
=================
*/
gentity_t      *EndWeave_Heal(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	gentity_t      *heldWeave;

	heldWeave = &g_entities[heldWeaveNum];

	if(heldWeave && heldWeave->target_ent && heldWeave->target_ent->target_ent && heldWeave->target_ent->target_ent->client)
	{
		heldWeave->target_ent->target_ent->client->ps.powerups[PW_REGEN] = 0;
	}

	//Com_Printf("SHIELD FIRE: in progress, ending weave effect=%d \n", heldWeave->target_ent->s.number);

	G_FreeEntity(heldWeave->target_ent);

	heldWeave->target_ent = NULL;

	return heldWeave;
}

/*
=================
RunWeave
Heal
=================
*/
void RunWeave_Heal(gentity_t * ent)
{
	//apply shielding until next run
	if(ent->damage <= 0)
	{
		ent->target_ent->health++;
		if(ent->target_ent->health > ent->target_ent->client->ps.stats[STAT_MAX_HEALTH] * 1.2)
		{
			ent->target_ent->health = ent->target_ent->client->ps.stats[STAT_MAX_HEALTH] * 1.2;
		}
		ent->damage = ent->splashDamage;
		ent->target_ent->client->ps.powerups[PW_REGEN] = level.time + WEAVE_HEAL_PULSE_TIME;
	}
	else
	{
		ent->damage--;
	}

	RunWeave_MoveToTarget(ent);

	//should probably copy trajectory too
	G_RunThink(ent);
}

//End Heal

//Begin Earthquake
/*
=================
EndWeave
Heal
=================
*/
gentity_t      *EndWeave_EarthQuake(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	gentity_t      *heldWeave;

	heldWeave = &g_entities[heldWeaveNum];

	//Com_Printf("SHIELD FIRE: in progress, ending weave effect=%d \n", heldWeave->target_ent->s.number);

	G_FreeEntity(heldWeave->target_ent);

	heldWeave->target_ent = NULL;

	return heldWeave;
}

/*
=================
FireWeave
Earthquake
=================
*/
qboolean FireWeave_EarthQuake(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum, int weaveID)
{
	vec3_t          right, up;
	gentity_t      *bolt;
	gentity_t      *heldWeave;

	heldWeave = &g_entities[heldWeaveNum];

	VectorNormalize(dir);
	PerpendicularVector(right, dir);
	CrossProduct(dir, right, up);

	bolt = G_Spawn();
	bolt->classname = EFFECT_CLASSNAME;
	bolt->nextthink = level.time + TIME_PROJECTILE_EXPIRE;
	bolt->think = G_ReleaseWeave;
	bolt->s.eType = ET_WEAVE_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = weaveID;
	bolt->r.ownerNum = self->s.number;
//unlagged - projectile nudge
	// we'll need this for nudging projectiles later
	//bolt->s.otherEntityNum = self->s.number;
//unlagged - projectile nudge
	bolt->s.otherEntityNum2 = heldWeaveNum;
	bolt->parent = self;
	switch (weaveID)
	{
		case WVW_A_EARTH_QUAKE_S:
			bolt->damage = WEAVE_EARTHQUAKES_PROJDAMAGE;
			bolt->splashDamage = WEAVE_EARTHQUAKES_PROJSPLASH_DAMAGE;
			bolt->splashRadius = WEAVE_EARTHQUAKES_PROJSPLASH;
			bolt->methodOfDeath = MOD_A_EARTH_QUAKE_S;
			bolt->splashMethodOfDeath = MOD_A_EARTH_QUAKE_S;
			break;
		case WVW_A_EARTH_QUAKE_M:
			bolt->damage = WEAVE_EARTHQUAKEM_PROJDAMAGE;
			bolt->splashDamage = WEAVE_EARTHQUAKEM_PROJSPLASH_DAMAGE;
			bolt->splashRadius = WEAVE_EARTHQUAKEM_PROJSPLASH;
			bolt->methodOfDeath = MOD_A_EARTH_QUAKE_M;
			bolt->splashMethodOfDeath = MOD_A_EARTH_QUAKE_M;
			break;
		case WVW_A_EARTH_QUAKE_L:
			bolt->damage = WEAVE_EARTHQUAKEL_PROJDAMAGE;
			bolt->splashDamage = WEAVE_EARTHQUAKEL_PROJSPLASH_DAMAGE;
			bolt->splashRadius = WEAVE_EARTHQUAKEL_PROJSPLASH;
			bolt->methodOfDeath = MOD_A_EARTH_QUAKE_L;
			bolt->splashMethodOfDeath = MOD_A_EARTH_QUAKE_L;
			break;
		default:
			G_FreeEntity(bolt);
			return qfalse;
	}

	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;
	bolt->impact = RunWeave_EarthQuake_Impact;

	//reference this from held
	heldWeave->target_ent = bolt;
	//held weave is now in progress
	heldWeave->s.frame = WST_INPROCESS;
	//prevent held weave expiring
	heldWeave->nextthink = 0;

	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trAcceleration = WEAVE_EARTHQUAKE_GRAVITY;
	switch (weaveID)
	{
		case WVW_A_EARTH_QUAKE_S:
			VectorScale(dir, WEAVE_EARTHQUAKES_PROJSPEED, bolt->s.pos.trDelta);
			break;
		case WVW_A_EARTH_QUAKE_M:
			VectorScale(dir, WEAVE_EARTHQUAKEM_PROJSPEED, bolt->s.pos.trDelta);
			break;
		case WVW_A_EARTH_QUAKE_L:
			VectorScale(dir, WEAVE_EARTHQUAKEL_PROJSPEED, bolt->s.pos.trDelta);
			break;
		default:
			//Caught previously
			break;
	}

	bolt->s.pos.trTime = level.time - WEAVE_EARTHQUAKE_START;	// move a bit on the very first frame
	VectorCopy(start, bolt->s.pos.trBase);

	SnapVector(bolt->s.pos.trDelta);	// save net bandwidth
	VectorCopy(start, bolt->r.currentOrigin);

	//weave = bolt;
	return qtrue;
}

qboolean FireWeave_EarthQuakeS(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	return FireWeave_EarthQuake(self, start, dir, heldWeaveNum, WVW_A_EARTH_QUAKE_S);
}

qboolean FireWeave_EarthQuakeM(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	return FireWeave_EarthQuake(self, start, dir, heldWeaveNum, WVW_A_EARTH_QUAKE_M);
}

qboolean FireWeave_EarthQuakeL(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	return FireWeave_EarthQuake(self, start, dir, heldWeaveNum, WVW_A_EARTH_QUAKE_L);
}

/*
=================
RunWeave
Earthquake Impact
=================
*/
void RunWeave_EarthQuake_Impact(gentity_t * ent, trace_t * trace)
{
	gentity_t      *other;
	gentity_t      *bolt;
	qboolean        hitClient = qfalse;
	gentity_t      *heldWeave;

	heldWeave = &g_entities[ent->s.otherEntityNum2];

	other = &g_entities[trace->entityNum];

	// impact damage
	if(other->takedamage)
	{
		vec3_t          velocity;
		vec3_t          direction;

		if(ent->knockback)
		{
			VectorCopy(ent->s.pos.trDelta, direction);
			G_KnockClient(other, direction, ent->knockback);
		}

		if(ent->damage)
		{
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

	//Spawn the effect
	bolt = G_Spawn();
	bolt->classname = EFFECT_CLASSNAME;

	switch (ent->s.weapon)
	{
		case WVW_A_EARTH_QUAKE_S:
			bolt->nextthink = level.time + WEAVE_EARTHQUAKES_DURATION;
			bolt->splashDamage = WEAVE_EARTHQUAKES_DPS;
			bolt->splashRadius = WEAVE_EARTHQUAKES_RADIUS;
			bolt->methodOfDeath = MOD_A_EARTH_QUAKE_S;
			bolt->splashMethodOfDeath = MOD_A_EARTH_QUAKE_S;
			break;
		case WVW_A_EARTH_QUAKE_M:
			bolt->nextthink = level.time + WEAVE_EARTHQUAKEM_DURATION;
			bolt->splashDamage = WEAVE_EARTHQUAKEM_DPS;
			bolt->splashRadius = WEAVE_EARTHQUAKEM_RADIUS;
			bolt->methodOfDeath = MOD_A_EARTH_QUAKE_M;
			bolt->splashMethodOfDeath = MOD_A_EARTH_QUAKE_M;
			break;
		case WVW_A_EARTH_QUAKE_L:
			bolt->nextthink = level.time + WEAVE_EARTHQUAKEL_DURATION;
			bolt->splashDamage = WEAVE_EARTHQUAKEL_DPS;
			bolt->splashRadius = WEAVE_EARTHQUAKEL_RADIUS;
			bolt->methodOfDeath = MOD_A_EARTH_QUAKE_L;
			bolt->splashMethodOfDeath = MOD_A_EARTH_QUAKE_L;
			break;
		default:
			G_FreeEntity(bolt);
			G_ReleaseWeave(ent);
			return;
	}
	bolt->think = G_ReleaseWeave;
	bolt->s.eType = ET_WEAVE_EFFECT;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = ent->s.weapon;
	bolt->r.ownerNum = ent->r.ownerNum;
//unlagged - projectile nudge
	// we'll need this for nudging projectiles later
	//bolt->s.otherEntityNum = ent->s.otherEntityNum;
//unlagged - projectile nudge
	bolt->s.otherEntityNum2 = ent->s.otherEntityNum2;
	bolt->parent = ent->parent;
	bolt->damage = 0;
	bolt->clipmask = MASK_SOLID;
	bolt->target_ent = NULL;
	bolt->freeAfterEvent = qfalse;

	//reference this from held
	heldWeave->target_ent = bolt;
	//Already done:
	//held weave is now in progress
	//heldWeave->s.frame = WST_INPROCESS;
	//prevent held weave expiring
	//heldWeave->nextthink = 0;

	//Send Normal
	VectorCopy(trace->plane.normal, bolt->s.angles);

	//Com_Printf("SPAWN QUAKE EFFECT @ %f,%f,%f \n", trace->endpos[0], trace->endpos[1], trace->endpos[2]);

	//SnapVector(bolt->s.pos.trDelta);  // save net bandwidth
	//VectorCopy(ent->r.currentOrigin, bolt->r.currentOrigin);
	VectorCopy(trace->endpos, bolt->r.currentOrigin);
	VectorCopy(trace->endpos, bolt->s.pos.trBase);
	VectorCopy(trace->endpos, bolt->s.origin);

	trap_LinkEntity(bolt);

	G_FreeEntity(ent);
}

/*
=================
RunWeave
EarthquakeS
=================
*/
void RunWeave_EarthQuake(gentity_t * ent)
{
	int             num;
	int             touch[MAX_GENTITIES];
	gentity_t      *hit;
	vec3_t          mins, maxs;
	vec3_t          delta;
	int             i;
	static vec3_t   range;

	range[0] = ent->splashRadius;
	range[1] = ent->splashRadius;
	range[2] = ent->splashRadius;

	VectorSubtract(ent->r.currentOrigin, range, mins);
	VectorAdd(ent->r.currentOrigin, range, maxs);

	// get nearby entities
	num = trap_EntitiesInBox(mins, maxs, touch, MAX_GENTITIES);

	// for each nearby entity
	for(i = 0; i < num; i++)
	{
		hit = &g_entities[touch[i]];

		//Only check Damageable targets
		if(hit->takedamage)
		{
			//Check range
			//Trap only traps entities within a cube. 
			//This test will check within a flat cylinder aligned with the model.
			if(hit->client)
			{
				VectorSubtract(hit->client->ps.origin, ent->s.origin, delta);
			}
			else
			{
				VectorSubtract(hit->s.origin, ent->s.origin, delta);
			}
			//Resolve length of delta on normal. Check height
			if(abs(DotProduct(ent->s.angles, delta)) > 50.0f)
			{
				//Out of range, skip damaging
				continue;
			}
			//Check distance
			if(VectorLength(delta) > ent->splashRadius)
			{
				//Out of range, skip damaging
				continue;
			}
			//Com_Printf("DeltaRes=%s  Norm.Delta=%f\n", vtos(delta), DotProduct(ent->s.angles, delta));

			G_Damage(hit, ent, &g_entities[ent->r.ownerNum], 0, ent->s.origin, ent->splashDamage, 0, ent->methodOfDeath);
		}
	}

	G_RunThink(ent);
}

//End Earthquake

//Begin IceShards
/*
=================
RunWeave
IceShard Impact
=================
*/
void RunWeave_IceShard_Impact(gentity_t * ent, trace_t * trace)
{
	vec3_t          velocity;
	float           dot;
	int             hitTime;
	gentity_t      *other;
	qboolean        hitClient = qfalse;

	other = &g_entities[trace->entityNum];

	// check for bounce
	if(!other->takedamage)
	{
		// reflect the velocity on the trace plane
		hitTime = level.previousTime + (level.time - level.previousTime) * trace->fraction;
		BG_EvaluateTrajectoryDelta(&ent->s.pos, hitTime, velocity);
		dot = DotProduct(velocity, trace->plane.normal);

		//Reflect
		VectorMA(velocity, -2 * dot, trace->plane.normal, velocity);

		VectorScale(velocity, WEAVE_ICESHARDS_BOUNCE_SCALE, ent->s.pos.trDelta);

		//ent->nextthink = (ent->nextthink - level.time) / 2 + level.time;
		ent->damage = ent->damage * WEAVE_ICESHARDS_BOUNCE_DMGSCALE;

		VectorAdd(ent->r.currentOrigin, trace->plane.normal, ent->r.currentOrigin);
		VectorCopy(ent->r.currentOrigin, ent->s.pos.trBase);
		ent->s.pos.trTime = level.time;

		G_AddEvent(ent, EV_WEAVEMISSILE_MISS, 0);

		return;
	}

	// impact damage
	if(other->takedamage)
	{
		vec3_t          velocity;
		vec3_t          direction;

		if(ent->knockback)
		{
			VectorCopy(ent->s.pos.trDelta, direction);
			G_KnockClient(other, direction, ent->knockback);
		}

		if(ent->damage)
		{
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
		G_AddEvent(ent, EV_WEAVEMISSILE_HIT, DirToByte(trace->plane.normal));
		ent->s.otherEntityNum = other->s.number;
	}
	else
	{
		G_AddEvent(ent, EV_WEAVEMISSILE_MISS, DirToByte(trace->plane.normal));
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
=================
FireWeave
IceShards

Based on weapon_supershotgun_fire()
=================
*/
qboolean FireWeave_IceShardsBase(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum, int weaveID, int count)
{
	int             i;
	vec3_t          right, up;
	gentity_t      *bolt;
	vec3_t          velocity;

	VectorNormalize(dir);
	PerpendicularVector(right, dir);
	CrossProduct(dir, right, up);

	VectorScale(dir, WEAVE_ICESHARDS_SPEED, velocity);

	for(i = 0; i < count; i++)
	{
		bolt = G_Spawn();
		bolt->classname = EFFECT_CLASSNAME;
		bolt->nextthink = level.time + TIME_ICESHARD_THINK;
		bolt->count = TIME_ICESHARD_COUNT;
		bolt->think = Shard_Think;
		bolt->s.eType = ET_WEAVE_MISSILE;
		bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
		bolt->s.weapon = weaveID;
		bolt->r.ownerNum = self->s.number;
		//unlagged - projectile nudge
		// we'll need this for nudging projectiles later
		//bolt->s.otherEntityNum = self->s.number;
		//unlagged - projectile nudge
		bolt->s.otherEntityNum2 = heldWeaveNum;
		bolt->parent = self;
		bolt->damage = WEAVE_ICESHARDS_DAMAGE;
		bolt->splashDamage = WEAVE_ICESHARDS_SPLASH_DAMAGE;
		bolt->splashRadius = WEAVE_ICESHARDS_SPLASH;
		switch (bolt->s.weapon)
		{
			case WVW_A_WATER_ICESHARDS_S:
				bolt->methodOfDeath = MOD_A_WATER_ICESHARDS_S;
				bolt->splashMethodOfDeath = MOD_A_WATER_ICESHARDS_S;
				break;
			case WVW_A_WATER_ICESHARDS_M:
				bolt->methodOfDeath = MOD_A_WATER_ICESHARDS_M;
				bolt->splashMethodOfDeath = MOD_A_WATER_ICESHARDS_M;
				break;
			default:
				G_FreeEntity(bolt);
				return qfalse;
		}
		bolt->clipmask = MASK_SHOT;
		bolt->target_ent = NULL;
		bolt->impact = RunWeave_IceShard_Impact;
		bolt->s.modelindex = i;

		bolt->s.pos.trType = TR_ACCELERATION;
		bolt->s.pos.trAcceleration = WEAVE_ICESHARDS_ACCELERATION;
		VectorCopy(velocity, bolt->s.pos.trDelta);

		VectorCopy(start, bolt->s.pos.trBase);

		bolt->s.pos.trBase[0] += (crandom() * WEAVE_ICESHARDS_SPREAD);
		bolt->s.pos.trBase[1] += (crandom() * WEAVE_ICESHARDS_SPREAD);
		bolt->s.pos.trBase[2] += (crandom() * WEAVE_ICESHARDS_SPREAD);

		bolt->s.pos.trTime = level.time - (WEAVE_ICESHARDS_START + (WEAVE_ICESHARDS_SEPARATION * i));	// move a bit on the very first frame

		SnapVector(bolt->s.pos.trDelta);	// save net bandwidth
		VectorCopy(start, bolt->r.currentOrigin);
	}

	//weave = bolt;
	return qtrue;
}

qboolean FireWeave_IceMultiShards(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	return FireWeave_IceShardsBase(self, start, dir, heldWeaveNum, WVW_A_WATER_ICESHARDS_M, WEAVE_ICEMULTISHARDS_COUNT);
}

qboolean FireWeave_IceShards(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	return FireWeave_IceShardsBase(self, start, dir, heldWeaveNum, WVW_A_WATER_ICESHARDS_S, WEAVE_ICESHARDS_COUNT);
}

//End IceShards

//Begin Light Source
/*
=================
FireWeave
Light Source
=================
*/
gentity_t      *EndWeave_LightSource(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	gentity_t      *heldWeave;

	heldWeave = &g_entities[heldWeaveNum];

	G_FreeEntity(heldWeave->target_ent);
	heldWeave->target_ent = NULL;

	return heldWeave;
}

qboolean FireWeave_LightSource(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	gentity_t      *bolt;
	gentity_t      *heldWeave;

	heldWeave = &g_entities[heldWeaveNum];

	VectorNormalize(dir);

	bolt = G_Spawn();
	bolt->classname = EFFECT_CLASSNAME;
	bolt->nextthink = level.time + WEAVE_LIGHTSOURCE_TIME;
	bolt->think = G_ReleaseWeave;
	bolt->s.eType = ET_WEAVE_EFFECT;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WVW_D_AIRFIRE_LIGHT;
	bolt->r.ownerNum = self->s.number;
//unlagged - projectile nudge
	// we'll need this for nudging projectiles later
//unlagged - projectile nudge
	bolt->s.otherEntityNum2 = heldWeaveNum;
	bolt->target_ent = self;
	bolt->s.generic1 = self->s.number;
	bolt->parent = self;
	bolt->damage = 0;
	bolt->splashDamage = 0;
	bolt->splashRadius = 0;
	bolt->methodOfDeath = MOD_UNKNOWN;	//Incapable of killing
	bolt->splashMethodOfDeath = MOD_UNKNOWN;	//Incapable of killing
	bolt->clipmask = MASK_SHOT;
	//bolt->target_ent = target;
	bolt->freeAfterEvent = qfalse;

	trap_LinkEntity(bolt);

	//reference this from held
	heldWeave->target_ent = bolt;
	//held weave is now in progress
	heldWeave->s.frame = WST_INPROCESS;
	//prevent held weave expiring
	heldWeave->nextthink = 0;

	//weave = bolt; 
	return qtrue;
}

/*
=================
FireWeave
Slow
=================
*/
qboolean FireWeave_Slow(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	gentity_t      *bolt;

	bolt = G_Spawn();
	bolt->classname = EFFECT_CLASSNAME;
	bolt->nextthink = level.time + TIME_PROJECTILE_EXPIRE;
	bolt->think = G_FreeEntity;
	bolt->s.eType = ET_WEAVE_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WVW_A_EARTHWATER_SLOW;
	bolt->r.ownerNum = self->s.number;
//unlagged - projectile nudge
	// we'll need this for nudging projectiles later
	//bolt->s.otherEntityNum = self->s.number;
//unlagged - projectile nudge
	bolt->s.otherEntityNum2 = heldWeaveNum;
	bolt->parent = self;

	bolt->damage = WEAVE_SLOWPOISON_PROJDAMAGE;
	bolt->splashDamage = WEAVE_SLOWPOISON_PROJSPLASH_DAMAGE;
	bolt->splashRadius = WEAVE_SLOWPOISON_PROJSPLASH;
	bolt->methodOfDeath = MOD_A_EARTHWATER_SLOW;
	bolt->splashMethodOfDeath = MOD_A_EARTHWATER_SLOW;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;
	bolt->impact = RunWeave_Slow_Impact;

	bolt->s.pos.trType = TR_LINEAR;
	VectorScale(dir, WEAVE_SLOWPOISON_PROJSPEED, bolt->s.pos.trDelta);

	bolt->s.pos.trTime = level.time - WEAVE_SLOWPOISON_START;	// move a bit on the very first frame
	VectorCopy(start, bolt->s.pos.trBase);

	SnapVector(bolt->s.pos.trDelta);	// save net bandwidth
	VectorCopy(start, bolt->r.currentOrigin);

	//weave = bolt;
	return qtrue;
}

/*
=================
RunWeave
Slow Impact
=================
*/
void RunWeave_Slow_Impact(gentity_t * ent, trace_t * trace)
{
	gentity_t      *other;
	qboolean        hitClient = qfalse;

	other = &g_entities[trace->entityNum];

	// impact damage
	if(other->takedamage)
	{
		vec3_t          velocity;

		if(ent->damage)
		{
			if(LogAccuracyHit(other, &g_entities[ent->r.ownerNum]))
			{
				g_entities[ent->r.ownerNum].client->accuracy_hits++;
				hitClient = qtrue;
			}

			BG_EvaluateTrajectoryDelta(&ent->s.pos, level.time, velocity);
			G_Damage(other, ent, &g_entities[ent->r.ownerNum], velocity, ent->s.origin, ent->damage, 0, ent->methodOfDeath);
		}
	}

	if(other->takedamage && other->client)
	{
		G_AddEvent(ent, EV_WEAVEMISSILE_HIT, DirToByte(trace->plane.normal));

		other->client->slowAttacker = &g_entities[ent->r.ownerNum];
		other->client->ps.powerups[PW_SLOWPOISONED] = level.time + WEAVE_SLOWPOISON_DURATION;
		other->client->slowTicks = WEAVE_SLOWPOISON_TICKS;

		ent->s.otherEntityNum = other->s.number;
	}
	else
	{
		G_AddEvent(ent, EV_WEAVEMISSILE_MISS, DirToByte(trace->plane.normal));
	}

	ent->freeAfterEvent = qtrue;

	// change over to a normal entity right at the point of impact
	ent->s.eType = ET_GENERAL;

	SnapVectorTowards(trace->endpos, ent->s.pos.trBase);	// save net bandwidth

	G_SetOrigin(ent, trace->endpos);

	trap_LinkEntity(ent);
}

/*
=================
FireWeave
Poison
=================
*/
qboolean FireWeave_Poison(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	gentity_t      *bolt;

	bolt = G_Spawn();
	bolt->classname = EFFECT_CLASSNAME;
	bolt->nextthink = level.time + TIME_PROJECTILE_EXPIRE;
	bolt->think = G_FreeEntity;
	bolt->s.eType = ET_WEAVE_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WVW_A_EARTHWATER_POISON;
	bolt->r.ownerNum = self->s.number;
//unlagged - projectile nudge
	// we'll need this for nudging projectiles later
	//bolt->s.otherEntityNum = self->s.number;
//unlagged - projectile nudge
	bolt->s.otherEntityNum2 = heldWeaveNum;
	bolt->parent = self;

	bolt->damage = WEAVE_POISON_PROJDAMAGE;
	bolt->splashDamage = WEAVE_POISON_PROJSPLASH_DAMAGE;
	bolt->splashRadius = WEAVE_POISON_PROJSPLASH;
	bolt->methodOfDeath = MOD_A_EARTHWATER_POISON;
	bolt->splashMethodOfDeath = MOD_A_EARTHWATER_POISON;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;
	bolt->impact = RunWeave_Poison_Impact;

	bolt->s.pos.trType = TR_LINEAR;
	VectorScale(dir, WEAVE_POISON_PROJSPEED, bolt->s.pos.trDelta);

	bolt->s.pos.trTime = level.time - WEAVE_POISON_START;	// move a bit on the very first frame
	VectorCopy(start, bolt->s.pos.trBase);

	SnapVector(bolt->s.pos.trDelta);	// save net bandwidth
	VectorCopy(start, bolt->r.currentOrigin);

	//weave = bolt;
	return qtrue;
}

/*
=================
RunWeave
Poison Impact
=================
*/
void RunWeave_Poison_Impact(gentity_t * ent, trace_t * trace)
{
	gentity_t      *other;
	qboolean        hitClient = qfalse;

	other = &g_entities[trace->entityNum];

	// impact damage
	if(other->takedamage)
	{
		vec3_t          velocity;

		if(ent->damage)
		{
			if(LogAccuracyHit(other, &g_entities[ent->r.ownerNum]))
			{
				g_entities[ent->r.ownerNum].client->accuracy_hits++;
				hitClient = qtrue;
			}

			BG_EvaluateTrajectoryDelta(&ent->s.pos, level.time, velocity);
			G_Damage(other, ent, &g_entities[ent->r.ownerNum], velocity, ent->s.origin, ent->damage, 0, ent->methodOfDeath);
		}
	}

	if(other->takedamage && other->client)
	{
		G_AddEvent(ent, EV_WEAVEMISSILE_HIT, DirToByte(trace->plane.normal));

		other->client->poisonDamage = WEAVE_POISON_MINDMG;
		other->client->poisonTicks = WEAVE_POISON_TICKS;
		other->client->poisonAttacker = &g_entities[ent->r.ownerNum];
		other->client->ps.powerups[PW_POISONED] = level.time + WEAVE_POISON_DURATION;

		ent->s.otherEntityNum = other->s.number;
	}
	else
	{
		G_AddEvent(ent, EV_WEAVEMISSILE_MISS, DirToByte(trace->plane.normal));
	}

	ent->freeAfterEvent = qtrue;

	// change over to a normal entity right at the point of impact
	ent->s.eType = ET_GENERAL;

	SnapVectorTowards(trace->endpos, ent->s.pos.trBase);	// save net bandwidth

	G_SetOrigin(ent, trace->endpos);

	trap_LinkEntity(ent);
}

/*
=================
RunWeave
Cure
=================
*/
qboolean FireWeave_Cure(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	int             targetNum;
	gentity_t      *target;
	team_t          targetTeam;

	if(G_IsTeamGame())
	{
		targetTeam = self->client->sess.sessionTeam;	//allies
	}
	else
	{
		targetTeam = TEAM_FREE;	//anyone
		//TODO: return?
	}

	//Get target
	if(!GetTargetPlayer(start, dir, WEAVE_CURE_CAST_RANGE, self->s.number, targetTeam, &targetNum))
	{
		return qfalse;
	}
	target = &g_entities[targetNum];

	//Check target is valid
	if(target->s.number >= level.maxclients || target->s.number < 0)
	{
		return qfalse;
	}

	if(target->health <= 0)
	{
		return qfalse;
	}

	//Minor heal
	if(target->health < target->client->ps.stats[STAT_MAX_HEALTH])
	{
		target->health += WEAVE_CURE_HEALTH;
		if(target->health > target->client->ps.stats[STAT_MAX_HEALTH])
		{
			target->health = target->client->ps.stats[STAT_MAX_HEALTH];
		}
	}

	//Cure
	target->client->ps.powerups[PW_SLOWPOISONED] = 0;
	target->client->slowAttacker = NULL;
	target->client->ps.powerups[PW_POISONED] = 0;
	target->client->poisonAttacker = NULL;

	return qtrue;
}

/*
=================
RunWeave
RIP
=================
*/
qboolean FireWeave_Rip(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	int             targetNum;
	gentity_t      *tent;
	gentity_t      *traceEnt;
	team_t          targetTeam;

	if(G_IsTeamGame())
	{
		targetTeam = EnemyTeamOf(self->client->sess.sessionTeam);	//enemies
	}
	else
	{
		targetTeam = TEAM_FREE;	//anyone
	}

	//Find target
	if(!GetTargetPlayer(start, dir, WEAVE_RIP_CAST_RANGE, self->s.number, targetTeam, &targetNum))
	{
		//Com_Printf("RIP FIRE: returning false (no target)\n");
		return qfalse;
	}
	traceEnt = &g_entities[targetNum];

	//Check target is valid client
	if(traceEnt->s.number >= level.maxclients || traceEnt->s.number < 0)
	{
		return qfalse;
	}

	//Damage target
	if(traceEnt->takedamage)
	{
		G_Damage(traceEnt, self, self, NULL, traceEnt->r.currentOrigin, WEAVE_RIP_DAMAGE, 0, MOD_TARGET_LASER);

		tent = G_TempEntity(start, EV_WEAVE_SHOT);
		G_SetOrigin(tent, traceEnt->r.currentOrigin);
		tent->s.otherEntityNum = self->s.number;
		tent->s.weapon = WVW_A_AIRWATER_RIP;
		//weave = tent;
	}
	else
	{
		return qfalse;
	}

	//weave = tent; 
	return qtrue;
}

//Begin Stamina
/*
=================
EndWeave
Stamina
=================
*/
gentity_t      *EndWeave_Stamina(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	gentity_t      *heldWeave;

	heldWeave = &g_entities[heldWeaveNum];
	G_FreeEntity(heldWeave->target_ent);
	heldWeave->target_ent = NULL;

	return heldWeave;
}

/*
=================
RunWeave
Stamina
=================
*/
qboolean FireWeave_Stamina(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	int             targetNum;
	gentity_t      *target;
	gentity_t      *bolt;
	team_t          targetTeam;
	gentity_t      *heldWeave;

	heldWeave = &g_entities[heldWeaveNum];

	if(G_IsTeamGame())
	{
		targetTeam = self->client->sess.sessionTeam;	//allies
	}
	else
	{
		targetTeam = TEAM_FREE;	//anyone
		//TODO: return?
	}

	if(!GetTargetPlayer(start, dir, WEAVE_STAMINA_CAST_RANGE, self->s.number, targetTeam, &targetNum))
	{
		return qfalse;
	}
	target = &g_entities[targetNum];

	if(target->s.number >= level.maxclients || target->s.number < 0)
	{
		return qfalse;
	}

	bolt = G_Spawn();
	bolt->classname = EFFECT_CLASSNAME;
	bolt->nextthink = level.time + WEAVE_STAMINA_TIME;
	bolt->think = G_ReleaseWeave;
	bolt->s.eType = ET_WEAVE_EFFECT;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WVW_D_SPIRIT_STAMINA;
	bolt->r.ownerNum = self->s.number;
//unlagged - projectile nudge
	// we'll need this for nudging projectiles later
	bolt->s.otherEntityNum = target->s.number;
//unlagged - projectile nudge
	bolt->s.otherEntityNum2 = heldWeaveNum;
	bolt->parent = self;
	bolt->damage = 0;
	bolt->splashDamage = 0;
	bolt->splashRadius = 0;
	bolt->methodOfDeath = MOD_UNKNOWN;	//Incapable of killing
	bolt->splashMethodOfDeath = MOD_UNKNOWN;	//Incapable of killing
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = target;
	bolt->freeAfterEvent = qfalse;

	//reference this from held
	heldWeave->target_ent = bolt;
	//held weave is now in progress
	heldWeave->s.frame = WST_INPROCESS;
	//prevent held weave expiring
	heldWeave->nextthink = 0;

	//Com_Printf("STAMINAFIRE: stamina spawned, EffectEnt = %d\n", bolt->s.number, bolt->s.otherEntityNum);
	RunWeave_Stamina(bolt);

	//weave = bolt;
	return qtrue;
}

/*
=================
RunWeave
Stamina
=================
*/
void RunWeave_Stamina(gentity_t * ent)
{
	//apply stamina until next run
	ent->target_ent->client->ps.powerups[PW_HASTE] = level.time + WEAVE_STAMINA_PULSE_TIME;

	RunWeave_MoveToTarget(ent);

	G_RunThink(ent);
}

//End Stamina


//Begin Lightning
/*
=================
EndWeave
Lightning
=================
*/
gentity_t      *EndWeave_Lightning(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	gentity_t      *heldWeave;

	heldWeave = &g_entities[heldWeaveNum];
	G_FreeEntity(heldWeave->target_ent);
	heldWeave->target_ent = NULL;

	return heldWeave;
}


/*
=================
RunWeave
Lightning
=================
*/
qboolean FireWeave_Lightning(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	gentity_t      *bolt;
	gentity_t      *heldWeave;
	trace_t         trace;
	vec3_t          end;
	vec3_t          wallhit;

	heldWeave = &g_entities[heldWeaveNum];

	VectorMA(start, WEAVE_LIGHTNING_CAST_RANGE, dir, end);
	trap_Trace(&trace, start, NULL, NULL, end, self->s.number, MASK_SHOT);

	if(trace.contents & CONTENTS_SOLID)
	{
		// hit a solid surface, gotta check if its under skybox
		VectorMA(trace.endpos, WEAVE_LIGHTNING_OFFSET, trace.plane.normal, wallhit);
		VectorMA(wallhit, WEAVE_LIGHTNING_SKYHEIGHT, axisDefault[2], end);
		trap_Trace(&trace, wallhit, NULL, NULL, end, self->s.number, MASK_SHOT);

		if(trace.surfaceFlags & SURF_SKY)
		{
			// under sky and can spawn the lightning ent
		}
		else
		{
			// not under sky
			return qfalse;
		}
	}
	else
	{
		// no target surface
		return qfalse;
	}

	bolt = G_Spawn();
	bolt->classname = EFFECT_CLASSNAME;
	bolt->nextthink = level.time + WEAVE_LIGHTNING_TIME;
	bolt->think = G_ReleaseWeave;
	bolt->s.eType = ET_WEAVE_EFFECT;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WVW_A_AIRFIRE_LIGHTNING;
	bolt->r.ownerNum = self->s.number;
//unlagged - projectile nudge
	// we'll need this for nudging projectiles later
//unlagged - projectile nudge
	bolt->s.otherEntityNum2 = heldWeaveNum;
	bolt->parent = self;
	bolt->damage = WEAVE_LIGHTNING_DAMAGE;
	bolt->splashDamage = WEAVE_LIGHTNING_SPLASH_DAMAGE;
	bolt->splashRadius = WEAVE_LIGHTNING_RADIUS;
	bolt->s.modelindex = 0;		//hits
	//time of next hit
	bolt->wait = bolt->spawnTime + WEAVE_LIGHTNING_DELAY +	//spawn time, initial delay
		(WEAVE_LIGHTNING_PERIOD * bolt->s.modelindex) +	//offset number of hits
		(random() * WEAVE_LIGHTNING_PERIOD);	//randomize
	bolt->methodOfDeath = MOD_A_AIRFIRE_LIGHTNING;
	bolt->splashMethodOfDeath = MOD_A_AIRFIRE_LIGHTNING;
	bolt->clipmask = MASK_SHOT;
	bolt->freeAfterEvent = qfalse;

	G_SetOrigin(bolt, wallhit);

	trap_LinkEntity(bolt);

	//reference this from held
	heldWeave->target_ent = bolt;
	//held weave is now in progress
	heldWeave->s.frame = WST_INPROCESS;
	//prevent held weave expiring
	heldWeave->nextthink = 0;

	RunWeave_Lightning(bolt);

	//weave = bolt;
	return qtrue;
}

/*
=================
RunWeave
Lightning
=================
*/
void RunWeave_Lightning(gentity_t * ent)
{
	// If its time to explode again
	if(level.time > ent->wait)
	{
		//another hit
		ent->s.modelindex++;
		//time of next hit
		ent->wait = ent->spawnTime + WEAVE_LIGHTNING_DELAY +	//spawn time, initial delay
			(WEAVE_LIGHTNING_PERIOD * ent->s.modelindex) +	//offset number of hits
			(random() * WEAVE_LIGHTNING_PERIOD);	//randomize

		//TODO: randomize damage a bit. Damage should be proportional to client side effect (strike intensity as event param)
		G_RadiusDamage(ent->r.currentOrigin, ent->parent, ent->splashDamage, ent->splashRadius, ent, ent->splashMethodOfDeath);
		G_AddEvent(ent, EV_WEAVE_SHOT, 0);
		G_Printf("Lightning explode\n");
	}
}

//End Lightning

//Begin Explosives
/*
=================
EndWeave
Explosives
=================
*/
gentity_t      *EndWeave_Explosive(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	gentity_t      *heldWeave;
	gentity_t      *ent;

	heldWeave = &g_entities[heldWeaveNum];

	ent = heldWeave->target_ent;
	if(heldWeave->s.frame == WST_INPROCESS)
	{
		G_RadiusDamage(ent->r.currentOrigin, ent->parent, ent->splashDamage, ent->splashRadius, ent, ent->splashMethodOfDeath);
		G_AddEvent(ent, EV_WEAVEMISSILE_MISS, ent->s.generic1);

		ent->freeAfterEvent = qtrue;
	}
	else
	{
		G_FreeEntity(ent);
	}
	heldWeave->target_ent = NULL;

	return heldWeave;
}


/*
=================
RunWeave
Explosives
=================
*/
qboolean FireWeave_ExplosiveBase(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum, int weaveID)
{
	gentity_t      *bolt;
	gentity_t      *heldWeave;
	trace_t         trace;
	vec3_t          wallhit;

	heldWeave = &g_entities[heldWeaveNum];

	VectorMA(start, WEAVE_EXPLOSIVE_CAST_RANGE, dir, wallhit);
	trap_Trace(&trace, start, NULL, NULL, wallhit, self->s.number, MASK_SHOT);
	VectorMA(trace.endpos, WEAVE_EXPLOSIVE_OFFSET, trace.plane.normal, wallhit);

	if(!(trace.contents & CONTENTS_SOLID))
	{
		// no target surface
		return qfalse;
	}

	bolt = G_Spawn();
	bolt->classname = EFFECT_CLASSNAME;
	bolt->nextthink = 0;
	bolt->think = G_ReleaseWeave;
	bolt->s.eType = ET_WEAVE_EFFECT;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = weaveID;
	bolt->r.ownerNum = self->s.number;
//unlagged - projectile nudge
	// we'll need this for nudging projectiles later
//unlagged - projectile nudge
	bolt->s.otherEntityNum2 = heldWeaveNum;
	bolt->parent = self;
	bolt->s.generic1 = DirToByte(trace.plane.normal);
	switch (bolt->s.weapon)
	{
		case WVW_D_EARTHFIRE_EXPLOSIVE_S:
			bolt->splashDamage = WEAVE_EXPLOSIVE_S_SPLASH_DAMAGE;
			bolt->splashRadius = WEAVE_EXPLOSIVE_S_RADIUS;
			bolt->methodOfDeath = MOD_D_EARTHFIRE_EXPLOSIVE_S;
			bolt->splashMethodOfDeath = MOD_D_EARTHFIRE_EXPLOSIVE_S;
			break;
		case WVW_D_EARTHFIRE_EXPLOSIVE_M:
			bolt->splashDamage = WEAVE_EXPLOSIVE_M_SPLASH_DAMAGE;
			bolt->splashRadius = WEAVE_EXPLOSIVE_M_RADIUS;
			bolt->methodOfDeath = MOD_D_EARTHFIRE_EXPLOSIVE_M;
			bolt->splashMethodOfDeath = MOD_D_EARTHFIRE_EXPLOSIVE_M;
			break;
		default:
			G_FreeEntity(bolt);
			return qfalse;
	}
	bolt->damage = 0;
	bolt->clipmask = MASK_DEADSOLID;
	bolt->freeAfterEvent = qfalse;

	G_SetOrigin(bolt, wallhit);

	trap_LinkEntity(bolt);

	//reference this from held
	heldWeave->target_ent = bolt;
	//held weave is now in progress
	heldWeave->s.frame = WST_INPROCESS;
	//prevent held weave expiring
	heldWeave->nextthink = 0;

	return qtrue;
}

qboolean FireWeave_ExplosiveSmall(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	return FireWeave_ExplosiveBase(self, start, dir, heldWeaveNum, WVW_D_EARTHFIRE_EXPLOSIVE_S);
}

qboolean FireWeave_ExplosiveMed(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum)
{
	return FireWeave_ExplosiveBase(self, start, dir, heldWeaveNum, WVW_D_EARTHFIRE_EXPLOSIVE_M);
}

//End Explosion
