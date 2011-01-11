/*
===========================================================================
This file is part of the weaver game.

It contains information for all the weaves in the game.
===========================================================================
*/

//Generic missile Run
void            RunWeave_Missile(gentity_t * ent);
void            RunWeave_Explode(gentity_t * ent);
void            RunWeave_MoveToTarget(gentity_t * ent);

//AirBlast
qboolean        FireWeave_AirBlast(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);

//Fireball
qboolean        FireWeave_Fireball(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);

//FireDarts
qboolean        FireWeave_FireDarts(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);

//FireMultiDarts
qboolean        FireWeave_FireMultiDarts(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);

//FireProtect
gentity_t      *EndWeave_ProtectFire(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
qboolean        FireWeave_ProtectFire(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);

//Slice
qboolean        FireWeave_SliceS(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
qboolean        FireWeave_SliceM(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
qboolean        FireWeave_SliceL(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
void            RunWeave_Slice(gentity_t * ent);

//Shield
gentity_t      *EndWeave_Shield(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
qboolean        FireWeave_Shield(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
void            RunWeave_Shield(gentity_t * ent);

//GrabPlayer
gentity_t      *EndWeave_GrabPlayer(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
qboolean        FireWeave_GrabPlayer(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
void            RunWeave_GrabPlayer(gentity_t * ent);

//Link
qboolean        FireWeave_Link(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
gentity_t      *EndWeave_Link(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);

//Heal
qboolean        FireWeave_HealS(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
qboolean        FireWeave_HealM(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
gentity_t      *EndWeave_Heal(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
void            RunWeave_Heal(gentity_t * ent);

//Earthquake
void            RunWeave_EarthQuake_Impact(gentity_t * ent, trace_t * trace);
void            RunWeave_EarthQuake(gentity_t * ent);
gentity_t      *EndWeave_EarthQuake(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
qboolean        FireWeave_EarthQuakeS(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
qboolean        FireWeave_EarthQuakeM(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
qboolean        FireWeave_EarthQuakeL(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);

//Ice Shards
void            RunWeave_IceShard_Impact(gentity_t * ent, trace_t * trace);
qboolean        FireWeave_IceShards(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
void            Shard_Think(gentity_t * ent);

//Slow Poison
void            RunWeave_Slow_Impact(gentity_t * ent, trace_t * trace);
qboolean        FireWeave_Slow(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);

//Poison
void            RunWeave_Poison_Impact(gentity_t * ent, trace_t * trace);
qboolean        FireWeave_Poison(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);

//Ice Multi Shards
void            RunWeave_IceShard_Impact(gentity_t * ent, trace_t * trace);
qboolean        FireWeave_IceMultiShards(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
void            Shard_Think(gentity_t * ent);

//Light Source
gentity_t      *EndWeave_LightSource(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
qboolean        FireWeave_LightSource(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);

//Protect Fire
gentity_t      *EndWeave_ProtectFire(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
qboolean        FireWeave_ProtectFire(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);

//Protect Air
gentity_t      *EndWeave_ProtectAir(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
qboolean        FireWeave_ProtectAir(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);

//Protect Earth
gentity_t      *EndWeave_ProtectEarth(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
qboolean        FireWeave_ProtectEarth(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);

//Protect Water
gentity_t      *EndWeave_ProtectWater(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
qboolean        FireWeave_ProtectWater(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);

//Cure
qboolean        FireWeave_Cure(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);

//RIP
qboolean        FireWeave_Rip(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);

//Stamina
gentity_t      *EndWeave_Stamina(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
qboolean        FireWeave_Stamina(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
void            RunWeave_Stamina(gentity_t * ent);

//Lightning
gentity_t      *EndWeave_Lightning(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
qboolean        FireWeave_Lightning(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
void            RunWeave_Lightning(gentity_t * ent);

//Explosives
gentity_t      *EndWeave_Explosive(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
qboolean        FireWeave_ExplosiveBase(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum, int weaveID);
qboolean        FireWeave_ExplosiveSmall(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
qboolean        FireWeave_ExplosiveMed(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);

//Fog
qboolean FireWeave_Fog(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
gentity_t      *EndWeave_Fog(gentity_t * self, vec3_t start, vec3_t dir, int heldWeaveNum);
