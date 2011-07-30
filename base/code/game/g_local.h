/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>
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

#ifndef __G_LOCAL_H
#define __G_LOCAL_H

// g_local.h -- local definitions for game module
#include "../../../code/qcommon/q_shared.h"
#include "bg_public.h"
#include "g_public.h"
#include "spell_common.h"
#include "spell_shared.h"
#include "objective_common.h"
#include "g_statistics.h"

//==================================================================

// the "gameversion" client command will print this plus compile date

#define	GAMEVERSION	"XreaL"

// Tr3B: added this to compile with different bot versions
//#define GLADIATOR
//#define BRAINWORKS
//#define ACEBOT


#define BODY_QUEUE_SIZE		8

#define INFINITE			1000000

#define	FRAMETIME			100	// msec
#define	CARNAGE_REWARD_TIME	3000
#define REWARD_SPRITE_TIME	2000

#define	INTERMISSION_DELAY_TIME	1000
#define	SP_INTERMISSION_DELAY_TIME	1000

// gentity->flags
#define	FL_GODMODE				0x00000010
#define	FL_NOTARGET				0x00000020
#define	FL_TEAMSLAVE			0x00000400	// not the first on the team
#define FL_NO_KNOCKBACK			0x00000800
#define FL_DROPPED_ITEM			0x00001000
#define FL_NO_BOTS				0x00002000	// spawn point not for bot use
#define FL_NO_HUMANS			0x00004000	// spawn point just for bots
#define FL_FORCE_GESTURE		0x00008000	// force gesture on client

// movers are things like doors, plats, buttons, etc
typedef enum
{
	MOVER_POS1,
	MOVER_POS2,
	MOVER_1TO2,
	MOVER_2TO1,

	ROTATOR_POS1,
	ROTATOR_POS2,
	ROTATOR_1TO2,
	ROTATOR_2TO1,

	MOVER_MISC
} moverState_t;

#define SP_PODIUM_MODEL		"models/meshes/ppodium.md5mesh"

//============================================================================

typedef struct gentity_s gentity_t;
typedef struct gclient_s gclient_t;

#if defined(ACEBOT)
typedef struct
{
	qboolean        isJumping;

	// for movement
	vec3_t          viewAngles;
	float           turnSpeed;

	vec3_t          moveVector;
	gentity_t      *moveTarget;
	gentity_t      *goalEntity;	// moveTarget of previous frame

	// timers
	float           next_move_time;
	float           wander_timeout;
	float           suicide_timeout;

	// for node code
	int             currentNode;	// current node
	int             goalNode;	// current goal node
	int             nextNode;	// the node that will take us one step closer to our goal
	int             lastNode;

	int             node_timeout;
	int             tries;
	int             state;

	// result
	// usercmd_t       cmd; // this user command is generated every time the bot thinks
	// using self->client->pers.cmd instead
} botState_t;
#endif

struct gentity_s
{
	entityState_t   s;			// communicated by server to clients
	entityShared_t  r;			// shared by both the server system and game

	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	// EXPECTS THE FIELDS IN THAT ORDER!
	//================================

	struct gclient_s *client;	// NULL if not a client

	qboolean        inuse;

	char           *classname;	// set in QuakeEd
	int             spawnflags;	// set in QuakeEd

	qboolean        neverFree;	// if true, FreeEntity will only unlink
	// bodyque uses this

	int             flags;		// FL_* variables

	char           *gamemodel;
	char           *model;
	char           *model2;

	int             spawnTime;	// level.time when the object was spawned
	int             freeTime;	// level.time when the object was freed

	int             eventTime;	// events will be cleared EVENT_VALID_MSEC after set
	qboolean        freeAfterEvent;
	qboolean        unlinkAfterEvent;

	qboolean        physicsObject;	// if true, it can be pushed by movers and fall off edges
	// all game items are physicsObjects,
	float           physicsBounce;	// 1.0 = continuous bounce, 0.0 = no bounce
	int             clipmask;	// brushes with this content value will be collided against
	// when moving.  items and corpses do not collide against
	// players, for instance

	// movers
	moverState_t    moverState;
	int             soundPos1;
	int             sound1to2;
	int             sound2to1;
	int             soundPos2;
	int             soundLoop;
	gentity_t      *parent;
	gentity_t      *nextTrain;
	gentity_t      *prevTrain;
	vec3_t          pos1, pos2;
	float           distance;

	char           *message;

	int             timestamp;	// body queue sinking, etc

	float           angle;		// set in editor, -1 = up, -2 = down
	char           *target;
	char           *name;
	char           *team;
	char           *targetShaderName;
	char           *targetShaderNewName;
	gentity_t      *target_ent;
	int             group;

	float           speed;
	vec3_t          movedir;

	int             nextthink;
	void            (*think) (gentity_t * self);
	void            (*reached) (gentity_t * self);	// movers call this when hitting endpoint
	void            (*blocked) (gentity_t * self, gentity_t * other);
	void            (*touch) (gentity_t * self, gentity_t * other, trace_t * trace);
	void            (*use) (gentity_t * self, gentity_t * other, gentity_t * activator);
	void            (*pain) (gentity_t * self, gentity_t * attacker, int damage);
	void            (*die) (gentity_t * self, gentity_t * inflictor, gentity_t * attacker, int damage, int mod);
	void            (*activate) (gentity_t * self, gentity_t * other, qboolean firstActivate);

	int             pain_debounce_time;
	int             fly_sound_debounce_time;	// wind tunnel
	int             last_move_time;

	int             health;

	gentity_t      *shield_ent;

	qboolean        takedamage;
	qboolean        crusher;	// doors that squeeze players
	int             damage;
	int             splashDamage;	// quad will increase this without increasing radius
	int             splashRadius;
	int             methodOfDeath;
	int             splashMethodOfDeath;

	// explosive
	int             materialType;

	// triggers / targets
	qboolean        start_on;
	qboolean        start_off;
	qboolean        silent;
	qboolean        no_protection;
	qboolean        slow;
	qboolean        red_only;
	qboolean        blue_only;
	qboolean        priv;

	// portal cameras
	qboolean        slowrotate;
	qboolean        fastrotate;
	qboolean        swing;

	int             count;

	gentity_t      *chain;
	gentity_t      *enemy;
	gentity_t      *activator;
	gentity_t      *teamchain;	// next entity in team
	gentity_t      *teammaster;	// master of the team

	int             kamikazeTime;
	int             kamikazeShockTime;

	int             watertype;
	int             waterlevel;

	int             soundIndex;
	qboolean        soundLooping;
	qboolean        soundWaitForTrigger;
	qboolean        soundGlobal;
	qboolean        soundActivator;

	// timing variables
	float           wait;
	float           random;

	qboolean        suspended;	// item will spawn where it was placed in map and won't drop to the floor
	gitem_t        *item;		// for bonus items

#if defined(G_LUA)
	// Lua scripting
	// like function pointers but pointing to
	// function names inside the .lua file that is loaded
	// for each map
	char           *luaThink;
	char           *luaTouch;
	char           *luaUse;
	char           *luaHurt;
	char           *luaDie;
	char           *luaFree;
	char           *luaTrigger;
	char           *luaSpawn;

	char           *luaParam1;
	char           *luaParam2;
	char           *luaParam3;
	char           *luaParam4;
#endif

#if defined(ACEBOT)
	botState_t      bs;

	int             node;
	float           weight;

#endif

	float           knockback;
	void            (*impact) (gentity_t * ent, trace_t * trace);
};


typedef enum
{
	CON_DISCONNECTED,
	CON_CONNECTING,
	CON_CONNECTED
} clientConnected_t;

typedef enum
{
	SPECTATOR_NOT,
	SPECTATOR_FREE,
	SPECTATOR_FOLLOW,
	SPECTATOR_SCOREBOARD
} spectatorState_t;

typedef enum
{
	TEAM_BEGIN,					// Beginning a team game, spawn at base
	TEAM_ACTIVE					// Now actively playing
} playerTeamStateState_t;

typedef struct
{
	playerTeamStateState_t state;

	int             location;

	int             captures;
	int             basedefense;
	int             carrierdefense;
	int             flagrecovery;
	int             fragcarrier;
	int             assists;

	float           lasthurtcarrier;
	float           lastreturnedflag;
	float           flagsince;
	float           lastfraggedcarrier;

	int             lastFlagEnt;
} playerTeamState_t;

// the auto following clients don't follow a specific client
// number, but instead follow the first two active players
#define	FOLLOW_ACTIVE1	-1
#define	FOLLOW_ACTIVE2	-2

// client data that stays across multiple levels or tournament restarts
// this is achieved by writing all the data to cvar strings at game shutdown
// time and reading them back at connection time.  Anything added here
// MUST be dealt with in G_InitSessionData() / G_ReadSessionData() / G_WriteSessionData()
typedef struct
{
	team_t          sessionTeam;
	team_t          sessionTeamNext;
	int             spectatorTime;	// for determining next-in-line to play
	spectatorState_t spectatorState;
	int             spectatorClient;	// for chasecam and follow mode
	int             wins, losses;	// tournament stats
	qboolean        teamLeader;	// true when this client is a team leader
} clientSession_t;

//
#define MAX_NETNAME			36
#define	MAX_VOTE_COUNT		3

//unlagged - true ping
#define NUM_PING_SAMPLES 64
//unlagged - true ping

// client data that stays across multiple respawns, but is cleared
// on each level change or team change at ClientBegin()
typedef struct
{
	clientConnected_t connected;
	usercmd_t       cmd;		// we would lose angles if not persistant
	qboolean        localClient;	// true if "ip" info key is "localhost"
	qboolean        initialSpawn;	// the first spawn should be at a cool location
	qboolean        predictItemPickup;	// based on cg_predictItems userinfo
	qboolean        pmoveFixed;	//
	char            netname[MAX_NETNAME];
	int             maxHealth;	// for handicapping
	int             enterTime;	// level.time the client entered the game
	playerTeamState_t teamState;	// status in teamplay games
	int             voteCount;	// to prevent people from constantly calling votes
	int             teamVoteCount;	// to prevent people from constantly calling votes
	qboolean        teamInfo;	// send team overlay updates?
	qboolean        ready;		// true if player is ready
//unlagged - client options
	// these correspond with variables in the userinfo string
	int             delag;
	int             debugDelag;
	int             cmdTimeNudge;
//unlagged - client options
//unlagged - lag simulation #2
	int             latentSnaps;
	int             latentCmds;
	int             plOut;
	usercmd_t       cmdqueue[MAX_LATENT_CMDS];
	int             cmdhead;
//unlagged - lag simulation #2
//unlagged - true ping
	int             realPing;
	int             pingsamples[NUM_PING_SAMPLES];
	int             samplehead;
//unlagged - true ping
} clientPersistant_t;

//unlagged - backward reconciliation #1
// the size of history we'll keep
#define NUM_CLIENT_HISTORY 17

// everything we need to know to backward reconcile
typedef struct
{
	vec3_t          mins, maxs;
	vec3_t          currentOrigin;
	int             leveltime;
} clientHistory_t;

//unlagged - backward reconciliation #1


// this structure is cleared on each ClientSpawn(),
// except for 'client->pers' and 'client->sess'
struct gclient_s
{
	// ps MUST be the first element, because the server expects it
	playerState_t   ps;			// communicated by server to clients

	// the rest of the structure is private to game
	clientPersistant_t pers;
	clientSession_t sess;

	qboolean        readyToExit;	// wishes to leave the intermission

	qboolean        noclip;

//unlagged - smooth clients #1
	// this is handled differently now
/*
	int             lastCmdTime;	// level.time of last usercmd_t, for EF_CONNECTION
	// we can't just use pers.lastCommand.time, because
	// of the g_sycronousclients case
*/
//unlagged - smooth clients #1

	int             buttons;
	int             oldbuttons;
	int             latched_buttons;

	vec3_t          oldOrigin;

	// sum up damage over an entire frame, so
	// shotgun blasts give a single big kick
	int             damage_armor;	// damage absorbed by armor
	int             damage_blood;	// damage taken out of health
	int             damage_knockback;	// impact damage
	vec3_t          damage_from;	// origin for vector calculation
	qboolean        damage_fromWorld;	// if true, don't use the damage_from vector

	int             accurateCount;	// for "impressive" reward sound

	int             accuracy_shots;	// total number of shots
	int             accuracy_hits;	// total number of hits

	//
	int             lastkilled_client;	// last client that this client killed
	int             lasthurt_client;	// last client that damaged this client
	int             lasthurt_mod;	// type of damage the client did
	int             lastused_ent;	// entity which was last +activated

	// timers
	int             respawnTime;	// can respawn when time > this, force after g_forcerespwan
	int             inactivityTime;	// kick players when time > this
	qboolean        inactivityWarning;	// qtrue if the five seoond warning has been given
	int             rewardTime;	// clear the EF_AWARD_IMPRESSIVE, etc when time > this

	int             airOutTime;

	int             lastKillTime;	// for multiple kill rewards

	qboolean        hookFireHeld;	// used for hook
	gentity_t      *hook;		// grapple hook if out

	int             switchTeamTime;	// time the player switched teams

	// timeResidual is used to handle events that happen every second
	// like health / armor countdowns and regeneration
	int             timeResidual;

#ifdef MISSIONPACK
	gentity_t      *persistantPowerup;
	int             portalID;
	int             ammoTimes[WP_NUM_WEAPONS];
	int             invulnerabilityTime;
#endif

	char           *areabits;

	gentity_t      *objItem;

	// WEAVER
	//current weave info
	qboolean        weaving;	//is this player weaving?
	qboolean        threading;	//is this player threading?
	int             thread;		//number of threads this player has done this weave
	int             currentWeaveGroup;	//agressive or defensive
	int             currentWeaveThreads[MAX_THREADS];	//current weave threads

	int             powerMax;
	int             powerThreading;
	int             powerUsed;
	int             slowTicks;
	int             poisonTicks;
	int             poisonDamage;

	gclient_t      *linkFollower;	//Client linking to this client
	gclient_t      *linkTarget;	//Client this client is linked to
	gentity_t      *linkEnt;	//Link effect entity to target
	gentity_t      *linkHeld;	//Held weave associated with the link to target

	gentity_t      *grabHolder;	//Player who is grabbing this player.

	gentity_t      *threadEnt;	//Threading effect entity for this player

	gentity_t      *protectHeldFire;
	gentity_t      *protectHeldAir;
	gentity_t      *protectHeldEarth;
	gentity_t      *protectHeldWater;

	gentity_t      *slowAttacker;
	gentity_t      *poisonAttacker;

//unlagged - backward reconciliation #1
	// the serverTime the button was pressed
	// (stored before pmove_fixed changes serverTime)
	int             attackTime;
	// the head of the history queue
	int             historyHead;
	// the history queue
	clientHistory_t history[NUM_CLIENT_HISTORY];
	// the client's saved position
	clientHistory_t saved;		// used to restore after time shift
	// an approximation of the actual server time we received this
	// command (not in 50ms increments)
	int             frameOffset;
//unlagged - backward reconciliation #1

//unlagged - smooth clients #1
	// the last frame number we got an update from this client
	int             lastUpdateFrame;
//unlagged - smooth clients #1
};


//
// this structure is cleared as each map is entered
//
#define	MAX_SPAWN_VARS			64
#define	MAX_SPAWN_VARS_CHARS	4096

typedef struct
{
	struct gclient_s *clients;	// [maxclients]

	struct gentity_s *gentities;
	int             gentitySize;
	int             numEntities;	// current number, <= MAX_GENTITIES

	int             warmupTime;	// restart match at this time
	qboolean        warmupForcedStart;	// restart match at this time

	fileHandle_t    logFile;

	// store latched cvars here that we want to get at often
	int             maxclients;

	int             framenum;
	int             time;		// in msec
	int             previousTime;	// so movers can back up when blocked

	int             startTime;	// level.time the map was started

	int             teamScores[TEAM_NUM_TEAMS];
	int             lastTeamLocationTime;	// last time of client team location update

	qboolean        newSession;	// don't use any old session data, because
	// we changed gametype

	qboolean        restarted;	// waiting for a map_restart to fire

	int             numConnectedClients;
	int             numNonSpectatorClients;	// includes connecting clients
	int             numPlayingClients;	// connected, non-spectators
	int             sortedClients[MAX_CLIENTS];	// sorted by score
	int             follow1, follow2;	// clientNums for auto-follow spectators

	int             warmupModificationCount;	// for detecting if g_warmup is changed
	qboolean        teamSwapped;

	// voting state
	char            voteString[MAX_STRING_CHARS];
	char            voteDisplayString[MAX_STRING_CHARS];
	int             voteTime;	// level.time vote was called
	int             voteExecuteTime;	// time the vote is executed
	int             voteYes;
	int             voteNo;
	int             numVotingClients;	// set by CalculateRanks

	// team voting state
	char            teamVoteString[2][MAX_STRING_CHARS];
	int             teamVoteTime[2];	// level.time vote was called
	int             teamVoteYes[2];
	int             teamVoteNo[2];
	int             numteamVotingClients[2];	// set by CalculateRanks

	// spawn variables
	qboolean        spawning;	// the G_Spawn*() functions are valid
	int             numSpawnVars;
	char           *spawnVars[MAX_SPAWN_VARS][2];	// key / value pairs
	int             numSpawnVarChars;
	char            spawnVarChars[MAX_SPAWN_VARS_CHARS];

	// player spawn state
	int             teamSpawnPreviousTimeRed;	// time of last spawn, per team. next spawn = this + spawn period.
	int             teamSpawnPreviousTimeBlue;	// time of last spawn, per team. next spawn = this + spawn period.
	int             teamSpawnPeriodRed;	// time between spawn waves.
	int             teamSpawnPeriodBlue;	// time between spawn waves.
	int             teamSpawningRed;	// 1 on the frame the team is spawning
	int             teamSpawningBlue;	// 1 on the frame the team is spawning

	// intermission state
	int             intermissionQueued;	// intermission was qualified, but
	// wait INTERMISSION_DELAY_TIME before
	// actually going there so the last
	// frag can be watched.  Disable future
	// kills during this delay
	int             intermissiontime;	// time the intermission was started
	char           *changemap;
	qboolean        readyToExit;	// at least one client wants to exit
	int             exitTime;
	vec3_t          intermission_origin;	// also used for spectator spawns
	vec3_t          intermission_angle;

	qboolean        locationLinked;	// target_locations get linked
	gentity_t      *locationHead;	// head of the location list
	int             bodyQueIndex;	// dead bodies
	gentity_t      *bodyQue[BODY_QUEUE_SIZE];
#ifdef MISSIONPACK
	int             portalSequence;
#endif

//unlagged - backward reconciliation #4
	// actual time this server frame started
	int             frameStartTime;
//unlagged - backward reconciliation #4
} level_locals_t;


//
// g_spawn.c
//
qboolean        G_SpawnString(const char *key, const char *defaultString, char **out);

// spawn string returns a temporary reference, you must CopyString() if you want to keep it
qboolean        G_SpawnFloat(const char *key, const char *defaultString, float *out);
qboolean        G_SpawnInt(const char *key, const char *defaultString, int *out);
qboolean        G_SpawnBoolean(const char *key, const char *defaultString, qboolean * out);
qboolean        G_SpawnVector(const char *key, const char *defaultString, float *out);
void            G_SpawnEntitiesFromString(void);
char           *G_NewString(const char *string);

//
// g_cmds.c
//
void            Cmd_Score_f(gentity_t * ent);
void            StopFollowing(gentity_t * ent);
void            BroadcastTeamChange(gclient_t * client, int oldTeam);
void            SetTeam(gentity_t * ent, char *s);
void            Cmd_FollowCycle_f(gentity_t * ent, int dir);
char           *ConcatArgs(int start);

//
// g_items.c
//
void            G_CheckTeamItems(void);
void            G_RunItem(gentity_t * ent);
void            RespawnItem(gentity_t * ent);

void            UseHoldableItem(gentity_t * ent);
void            PrecacheItem(gitem_t * it);
gentity_t      *Drop_Item(gentity_t * ent, gitem_t * item, float angle);
gentity_t      *LaunchItem(gitem_t * item, vec3_t origin, vec3_t velocity);
void            SetRespawn(gentity_t * ent, float delay);
void            G_SpawnItem(gentity_t * ent, gitem_t * item);
void            FinishSpawningItem(gentity_t * ent);
void            Think_Weapon(gentity_t * ent);
int             ArmorIndex(gentity_t * ent);
void            Add_Ammo(gentity_t * ent, int weapon, int count);
void            Touch_Item(gentity_t * ent, gentity_t * other, trace_t * trace);

void            ClearRegisteredItems(void);
void            RegisterItem(gitem_t * item);
void            SaveRegisteredItems(void);

//
// g_utils.c
//
int             G_ModelIndex(const char *name);
int             G_SoundIndex(const char *name);
int             G_EffectIndex(const char *name);
void            G_TeamCommand(team_t team, char *cmd);
void            G_KillBox(gentity_t * ent);
void            G_ProjectSource(vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
gentity_t      *G_Find(gentity_t * from, int fieldofs, const char *match);
gentity_t      *G_FindRadius(gentity_t * from, const vec3_t org, float rad);
qboolean        G_IsVisible(const gentity_t * self, const vec3_t goal);
gentity_t      *G_PickTarget(char *name);
void            G_UseTargets(gentity_t * ent, gentity_t * activator);
void            G_SetMovedir(vec3_t angles, vec3_t movedir);

void            G_ActivateUse(gentity_t * ent, gentity_t * other, qboolean firstActivate);
void            G_ActivateUseFirst(gentity_t * ent, gentity_t * other, qboolean firstActivate);

void            G_InitGentity(gentity_t * e);
gentity_t      *G_Spawn(void);
gentity_t      *G_TempEntity(vec3_t origin, int event);
void            G_Sound(gentity_t * ent, int channel, int soundIndex);
void            G_FreeEntity(gentity_t * e);
qboolean        G_EntitiesFree(void);

void            G_TouchTriggers(gentity_t * ent);
void            G_TouchSolids(gentity_t * ent);

float          *tv(float x, float y, float z);
char           *vtos(const vec3_t v);

float           vectoyaw(const vec3_t vec);

void            G_AddPredictableEvent(gentity_t * ent, int event, int eventParm);
void            G_AddEvent(gentity_t * ent, int event, int eventParm);
void            G_SetOrigin(gentity_t * ent, vec3_t origin);
void            AddRemap(const char *oldShader, const char *newShader, float timeOffset);
const char     *BuildShaderStateConfig();

//
// g_combat.c
//
qboolean        CanDamage(gentity_t * targ, vec3_t origin);
qboolean        G_Damage(gentity_t * targ, gentity_t * inflictor, gentity_t * attacker, const vec3_t dir, const vec3_t point, int damage,
						 int dflags, int mod);
qboolean        G_RadiusDamage(vec3_t origin, gentity_t * attacker, float damage, float radius, gentity_t * ignore, int mod);
int             G_InvulnerabilityEffect(gentity_t * targ, vec3_t dir, vec3_t point, vec3_t impactpoint, vec3_t bouncedir);
void            body_die(gentity_t * self, gentity_t * inflictor, gentity_t * attacker, int damage, int meansOfDeath);
void            TossClientItems(gentity_t * self);

void            TossClientPersistantPowerups(gentity_t * self);
void            TossClientCubes(gentity_t * self);

// damage flags
#define DAMAGE_RADIUS				0x00000001	// damage was indirect
#define DAMAGE_NO_ARMOR				0x00000002	// armour does not protect from this damage
#define DAMAGE_NO_KNOCKBACK			0x00000004	// do not affect velocity, just view angles
#define DAMAGE_NO_PROTECTION		0x00000008	// armor, shields, invulnerability, and godmode have no effect
#define DAMAGE_NO_TEAM_PROTECTION	0x00000010	// armor, shields, invulnerability, and godmode have no effect

//
// g_missile.c
//
void            G_RunMissile(gentity_t * ent);
void            G_ExplodeMissile(gentity_t * ent);

gentity_t      *fire_blaster(gentity_t * self, vec3_t start, vec3_t aimdir);
gentity_t      *fire_plasma(gentity_t * self, vec3_t start, vec3_t aimdir);
gentity_t      *fire_grenade(gentity_t * self, vec3_t start, vec3_t aimdir);
gentity_t      *fire_flakgrenade(gentity_t * self, vec3_t start, vec3_t aimdir);
gentity_t      *fire_clustergrenade(gentity_t * self, vec3_t start, vec3_t aimdir);
gentity_t      *fire_rocket(gentity_t * self, vec3_t start, vec3_t dir);
gentity_t      *fire_homing(gentity_t * self, vec3_t start, vec3_t dir);
gentity_t      *fire_bfg(gentity_t * self, vec3_t start, vec3_t dir);
gentity_t      *fire_grapple(gentity_t * self, vec3_t start, vec3_t dir);
gentity_t      *fire_nail(gentity_t * self, vec3_t start, vec3_t forward, vec3_t right, vec3_t up);
gentity_t      *fire_gravnail(gentity_t * self, vec3_t start, vec3_t forward, vec3_t right, vec3_t up);
gentity_t      *fire_prox(gentity_t * self, vec3_t start, vec3_t aimdir);
gentity_t      *fire_railsphere(gentity_t * self, vec3_t start, vec3_t aimdir);

//WEAVER
// g_spell_weave.c
//
void            CreateWeave(gentity_t * self, int group, int threads[MAX_THREADS]);
void            CreateWeaveID(gentity_t * self, int weaveID, int powerUsing);
void            CreateThreads(gentity_t * player);
qboolean        ExecuteWeave(gentity_t * weave);
void            EndWeave(gentity_t * weave);
void            AddHeldWeaveToPlayer(gentity_t * ent, playerState_t * player);
void            UseHeldWeave(gentity_t * ent);
void            ReleaseHeldWeave(gentity_t * ent);
void            ExpireHeldWeave(gentity_t * heldWeave);
void            ClearHeldWeave(gentity_t * ent);
void            ClearHeldWeaveCast(gentity_t * ent, int castClear);
void            G_RunWeaveEffect(gentity_t * ent);
void            G_ReleaseWeave(gentity_t * weave);
void            ThreadsThink(void);
int             WeaveTime(int weaveID);

// g_spell_power.c
qboolean        ClientPowerShielded(gclient_t * holdingClient);
int             ClientPowerInUse(gclient_t * holdingClient);
int             ClientPowerAvailable(gclient_t * holdingClient);
int             ClientPowerMax(gclient_t * holdingClient);
void            ClientPowerConsume(gclient_t * holdingClient, int amount);
void            ClientPowerRelease(gclient_t * holdingClient);
void            ClientPowerInitialize(gclient_t * holdingClient);
void            ClientPowerHeldRelease(gclient_t * holdingClient);
void            ClientPowerHeldConsume(gclient_t * holdingClient, int amount);

qboolean        ClientLinkJoin(gclient_t * leadClient, gclient_t * followClient);
void            ClientLinkLeave(gclient_t * followClient);
void            RunLinkEnt(gentity_t * bolt);

// g_spell_client.c
//
void            ClientWeaverCleanup(gclient_t * client);
void            ClientWeaverInitialize(gclient_t * client);

void            ClientWeaverDie(gentity_t * self);
int             ClientWeaverProtectDamage(gentity_t * targ, gclient_t *client, gentity_t * inflictor, gentity_t * attacker,
										  const vec3_t dir, const vec3_t point, int damageBase, int dflags, int mod);

void            ClientWeaveStart(gclient_t * client);
void            ClientThreadStart(gclient_t * client);
void            ClientThreadEnd(gclient_t * client);
void            ClientWeaveEnd(gclient_t * client, gentity_t * ent);
void            ClientWeaveUpdateStats(gentity_t * ent, gclient_t * client);
void            ClientPoisonUpdateStats(gentity_t *ent);

// g_spell_ents.c
//
void            G_RunObjItem(gentity_t * ent);
void            G_TossObjItems(gentity_t * self);

// g_spell_misc.c
//
void            WeaveProtectCheck(gclient_t * checkClient);
int             PowerEncode(int threads[MAX_THREADS], int offset, int count);

// g_spell_objective.c
//
void            G_ObjectiveAnnounce(objective_events objevp, gentity_t * ent, gentity_t * other);

/* g_statistics.c */
void            G_StatInitPlayer(gentity_t * ent);
void            G_StatInit(void);

void            G_StatCountAdd(statField_t f, gentity_t * ent, gentity_t * other, int weapon, int val);
void            G_StatTimeStart(statField_t f, gentity_t * ent, gentity_t * other, int weapon);
void            G_StatTimeAdd(statField_t f, gentity_t * ent, gentity_t * other, int weapon);

void            G_StatDump(void);

//
// g_mover.c
//
void            G_RunMover(gentity_t * ent);
void            Touch_DoorTrigger(gentity_t * ent, gentity_t * other, trace_t * trace);
void            Reached_Train(gentity_t * ent);
void            SetupTrainPath(gentity_t * ent, qboolean allowNoTarget);
void            SetMoverState(gentity_t * ent, moverState_t moverState, int time);

//
// g_trigger.c
//
void            trigger_teleporter_touch(gentity_t * self, gentity_t * other, trace_t * trace);


//
// g_misc.c
//
void            TeleportPlayer(gentity_t * player, vec3_t origin, vec3_t angles);
void            TeleportEntity(gentity_t * ent, vec3_t origin, vec3_t angles);



#ifdef MISSIONPACK
void            DropPortalSource(gentity_t * ent);
void            DropPortalDestination(gentity_t * ent);
#endif


//
// g_weapon.c
//
qboolean        LogAccuracyHit(gentity_t * target, gentity_t * attacker);
void            CalcMuzzlePoint(gentity_t * ent, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint, int weapon, qboolean secondary);
qboolean        CheckGauntletAttack(gentity_t * ent);
void            Weapon_HookFree(gentity_t * ent);
void            Weapon_HookThink(gentity_t * ent);

//unlagged - g_unlagged.c
void            G_ResetHistory(gentity_t * ent);
void            G_StoreHistory(gentity_t * ent);
void            G_TimeShiftAllClients(int time, gentity_t * skip);
void            G_UnTimeShiftAllClients(gentity_t * skip);
void            G_DoTimeShiftFor(gentity_t * ent);
void            G_UndoTimeShiftFor(gentity_t * ent);
void            G_UnTimeShiftClient(gentity_t * client);
void            G_PredictPlayerMove(gentity_t * ent, float frametime);

//unlagged - g_unlagged.c

//
// g_client.c
//
team_t          TeamCount(int ignoreClientNum, int team);
int             TeamLeader(int team);
team_t          PickTeam(int ignoreClientNum);
void            SetClientViewAngle(gentity_t * ent, vec3_t angle);
gentity_t      *SelectSpawnPoint(vec3_t avoidPoint, vec3_t origin, vec3_t angles);
void            CopyToBodyQue(gentity_t * ent);
void            respawn(gentity_t * ent);
void            BeginIntermission(void);
void            InitClientPersistant(gclient_t * client);
void            InitClientResp(gclient_t * client);
void            InitBodyQue(void);
void            ClientSpawn(gentity_t * ent);
void            player_die(gentity_t * self, gentity_t * inflictor, gentity_t * attacker, int damage, int mod);
void            AddScore(gentity_t * ent, vec3_t origin, int score);
void            CalculateRanks(void);
qboolean        SpotWouldTelefrag(gentity_t * spot);

//
// g_svcmds.c
//
qboolean        ConsoleCommand(void);
void            G_ProcessIPBans(void);
qboolean        G_FilterPacket(char *from);

//
// g_weapon.c
//
void            FireWeapon(gentity_t * ent);
void            FireWeapon2(gentity_t * ent);

void            G_StartKamikaze(gentity_t * ent);

//
// p_hud.c
//
void            MoveClientToIntermission(gentity_t * client);
void            G_SetStats(gentity_t * ent);
void            DeathmatchScoreboardMessage(gentity_t * client);

//
// g_cmds.c
//

//
// g_pweapon.c
//


//
// g_main.c
//
void            FindIntermissionPoint(void);
void            SetLeader(int team, int client);
void            CheckTeamLeader(int team);
void            G_RunThink(gentity_t * ent);
void QDECL      G_LogPrintf(const char *fmt, ...);
void            SendScoreboardMessageToAllClients(void);
void QDECL      G_Printf(const char *fmt, ...);
void QDECL      G_Error(const char *fmt, ...);
void QDECL      G_PrintfClient(gentity_t * ent, const char *fmt, ...);
void            LogExit(const char *string);
qboolean        GameIsInWarmup(void);
void            G_SWMapChange(int index);

//
// g_client.c
//
char           *ClientConnect(int clientNum, qboolean firstTime, qboolean isBot);
void            ClientUserinfoChanged(int clientNum);
void            ClientDisconnect(int clientNum);
void            ClientBegin(int clientNum);
void            ClientCommand(int clientNum);

//
// g_active.c
//
void            ClientThink(int clientNum);
void            ClientEndFrame(gentity_t * ent);
void            G_RunClient(gentity_t * ent);

//
// g_team.c
//
qboolean        OnSameTeam(gentity_t * ent1, gentity_t * ent2);
void            Team_CheckDroppedItem(gentity_t * dropped);
qboolean        CheckObeliskAttack(gentity_t * obelisk, gentity_t * attacker);
void            Team_ReadyPlayers(int team, qboolean readyness);
int             Team_CaptureFlag(gentity_t * ent, gentity_t * other, int team);
void            Team_SwapTeams(void);
void            Team_SwapTeamsMapping(void);

//
// g_mem.c
//
void           *G_Alloc(int size);
void            G_InitMemory(void);
void            Svcmd_GameMem_f(void);

//
// g_session.c
//
void            G_ReadSessionData(gclient_t * client);
void            G_InitSessionData(gclient_t * client, char *userinfo);

void            G_InitWorldSession(void);
void            G_WriteSessionData(void);

//
// g_arenas.c
//
void            UpdateTournamentInfo(void);
void            SpawnModelsOnVictoryPads(void);
void            Svcmd_AbortPodium_f(void);



//
// g_bot.c
//
#if defined(BRAINWORKS) || defined(GLADIATOR)

void            G_InitBots(qboolean restart);
void            G_CheckBotSpawn(void);
char           *G_GetBotInfoByName(const char *name);
void            G_RemoveQueuedBotBegin(int clientNum);
qboolean        G_BotConnect(int clientNum, qboolean restart);
void            Svcmd_AddBot_f(void);
void            Svcmd_BotList_f(void);
void            BotInterbreedEndMatch(void);

// ai_main.c
#define MAX_FILEPATH			144

//bot settings
typedef struct bot_settings_s
{
	char            characterfile[MAX_FILEPATH];
	float           skill;
	char            team[MAX_FILEPATH];
} bot_settings_t;


int             BotAISetup(int restart);
int             BotAIShutdown(int restart);
int             BotAILoadMap(int restart);
int             BotAISetupClient(int client, struct bot_settings_s *settings, qboolean restart);
int             BotAIShutdownClient(int client, qboolean restart);
int             BotAIStartFrame(int time);
void            BotAIDebug(void);	// brainworks

//void          BotTestAAS(vec3_t origin);
#endif

#if defined(ACEBOT)
#include "acebot.h"
#endif

#if defined(G_LUA)

//
// g_lua.c 
//
// Callbacks
void            G_LuaHook_InitGame(int levelTime, int randomSeed, int restart);
void            G_LuaHook_ShutdownGame(int restart);
void            G_LuaHook_RunFrame(int levelTime);
qboolean        G_LuaHook_ClientConnect(int clientNum, qboolean firstTime, qboolean isBot, char *reason);
void            G_LuaHook_ClientDisconnect(int clientNum);
void            G_LuaHook_ClientBegin(int clientNum);
void            G_LuaHook_ClientUserinfoChanged(int clientNum);
void            G_LuaHook_ClientSpawn(int clientNum);
qboolean        G_LuaHook_ClientCommand(int clientNum, char *command);
qboolean        G_LuaHook_ConsoleCommand(char *command);
void            G_LuaHook_Print(char *text);
qboolean        G_LuaHook_Obituary(int victim, int killer, int meansOfDeath, char *customObit);
qboolean        G_LuaHook_EntityThink(char *function, int entity);
qboolean        G_LuaHook_EntityTouch(char *function, int entity, int other);
qboolean        G_LuaHook_EntityUse(char *function, int entity, int other, int activator);
qboolean        G_LuaHook_EntityHurt(char *function, int entity, int inflictor, int attacker);
qboolean        G_LuaHook_EntityDie(char *function, int entity, int inflictor, int attacker, int dmg, int mod);
qboolean        G_LuaHook_EntityFree(char *function, int entity);
qboolean        G_LuaHook_EntityTrigger(char *function, int entity, int other);
qboolean        G_LuaHook_EntitySpawn(char *function, int entity);

// Other
void            G_LuaStatus(gentity_t * ent);
qboolean        G_LuaInit(void);
void            G_LuaShutdown(void);

#endif							// G_LUA


#include "g_team.h"				// teamplay specific stuff


extern level_locals_t level;
extern gentity_t g_entities[MAX_GENTITIES];

#define	FOFS(x) ((size_t)&(((gentity_t *)0)->x))

extern vmCvar_t g_gametype;
extern vmCvar_t g_dedicated;
extern vmCvar_t g_cheats;
extern vmCvar_t g_maxclients;	// allow this many total, including spectators
extern vmCvar_t g_maxGameClients;	// allow this many active
extern vmCvar_t g_restarted;

extern vmCvar_t g_dmflags;
extern vmCvar_t g_fraglimit;
extern vmCvar_t g_timelimit;
extern vmCvar_t g_capturelimit;
extern vmCvar_t g_friendlyFire;
extern vmCvar_t g_password;
extern vmCvar_t g_needpass;
extern vmCvar_t g_gravity;
extern vmCvar_t g_speed;
extern vmCvar_t g_knockback;
extern vmCvar_t g_knockbackZ;
extern vmCvar_t g_quadfactor;
extern vmCvar_t g_forcerespawn;
extern vmCvar_t g_inactivity;
extern vmCvar_t g_debugMove;
extern vmCvar_t g_debugAlloc;
extern vmCvar_t g_debugDamage;
extern vmCvar_t g_debugLua;
extern vmCvar_t g_weaponRespawn;
extern vmCvar_t g_weaponTeamRespawn;
extern vmCvar_t g_synchronousClients;
extern vmCvar_t g_motd;
extern vmCvar_t g_warmup;
extern vmCvar_t g_doWarmup;
extern vmCvar_t g_blood;
extern vmCvar_t g_allowVote;
extern vmCvar_t g_teamAutoJoin;
extern vmCvar_t g_teamForceBalance;
extern vmCvar_t g_banIPs;
extern vmCvar_t g_filterBan;
extern vmCvar_t g_obeliskHealth;
extern vmCvar_t g_obeliskRegenPeriod;
extern vmCvar_t g_obeliskRegenAmount;
extern vmCvar_t g_obeliskRespawnDelay;
extern vmCvar_t g_cubeTimeout;
extern vmCvar_t g_redteam;
extern vmCvar_t g_blueteam;
extern vmCvar_t g_smoothClients;

extern vmCvar_t g_debugWeaving;
extern vmCvar_t g_debugEntities;

extern vmCvar_t g_rankings;
extern vmCvar_t g_enableDust;
extern vmCvar_t g_enableBreath;
extern vmCvar_t g_singlePlayer;
extern vmCvar_t g_proxMineTimeout;

extern vmCvar_t g_rocketAcceleration;
extern vmCvar_t g_rocketVelocity;

extern vmCvar_t g_teleportMissiles;
extern vmCvar_t g_fallingDamage;

extern vmCvar_t g_teamSpawnWaves;
extern vmCvar_t g_teamSpawnBlue;
extern vmCvar_t g_teamSpawnRed;

extern vmCvar_t g_currentRound;
extern vmCvar_t g_nextTimeLimit;
extern vmCvar_t g_swTeamSwitching;
extern vmCvar_t g_swTeamToClan;
extern vmCvar_t g_swMaps;
extern vmCvar_t g_swMap;
extern vmCvar_t g_scoreA;
extern vmCvar_t g_scoreB;

extern vmCvar_t g_woundedHealth;

extern vmCvar_t pm_debugMove;
extern vmCvar_t pm_airControl;
extern vmCvar_t pm_fastWeaponSwitches;
extern vmCvar_t pm_fixedPmove;
extern vmCvar_t pm_fixedPmoveFPS;

extern vmCvar_t lua_allowedModules;
extern vmCvar_t lua_modules;


//unlagged - server options
// some new server-side variables
extern vmCvar_t g_delagHitscan;
extern vmCvar_t g_unlaggedVersion;
extern vmCvar_t g_truePing;

// server admins can adjust this if they *believe* the lightning
// gun is too powerful with lag compensation
extern vmCvar_t g_lightningDamage;

// this is for convenience - using "sv_fps.integer" is nice :)
extern vmCvar_t sv_fps;

//unlagged - server options


#if defined(ACEBOT)
extern vmCvar_t ace_debug;
extern vmCvar_t ace_showNodes;
extern vmCvar_t ace_showLinks;
extern vmCvar_t ace_showPath;
extern vmCvar_t ace_pickLongRangeGoal;
extern vmCvar_t ace_pickShortRangeGoal;
extern vmCvar_t ace_attackEnemies;
extern vmCvar_t ace_spSkill;
extern vmCvar_t ace_botsFile;
#endif


void            trap_Printf(const char *fmt);
void            trap_Error(const char *fmt);
int             trap_Milliseconds(void);
int             trap_Argc(void);
void            trap_Argv(int n, char *buffer, int bufferLength);
void            trap_Args(char *buffer, int bufferLength);
int             trap_FS_FOpenFile(const char *qpath, fileHandle_t * f, fsMode_t mode);
void            trap_FS_Read(void *buffer, int len, fileHandle_t f);
void            trap_FS_Write(const void *buffer, int len, fileHandle_t f);
void            trap_FS_FCloseFile(fileHandle_t f);
int             trap_FS_GetFileList(const char *path, const char *extension, char *listbuf, int bufsize);
int             trap_FS_Seek(fileHandle_t f, long offset, int origin);	// fsOrigin_t
void            trap_SendConsoleCommand(int exec_when, const char *text);
void            trap_Cvar_Register(vmCvar_t * cvar, const char *var_name, const char *value, int flags);
void            trap_Cvar_Update(vmCvar_t * cvar);
void            trap_Cvar_Set(const char *var_name, const char *value);
int             trap_Cvar_VariableIntegerValue(const char *var_name);
float           trap_Cvar_VariableValue(const char *var_name);
void            trap_Cvar_VariableStringBuffer(const char *var_name, char *buffer, int bufsize);
void            trap_LocateGameData(gentity_t * gEnts, int numGEntities, int sizeofGEntity_t, playerState_t * gameClients,
									int sizeofGameClient);
void            trap_DropClient(int clientNum, const char *reason);
void            trap_SendServerCommand(int clientNum, const char *text);
void            trap_SetConfigstring(int num, const char *string);
void            trap_GetConfigstring(int num, char *buffer, int bufferSize);
void            trap_GetUserinfo(int num, char *buffer, int bufferSize);
void            trap_SetUserinfo(int num, const char *buffer);
void            trap_GetServerinfo(char *buffer, int bufferSize);
void            trap_SetBrushModel(gentity_t * ent, const char *name);
void            trap_Trace(trace_t * results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
						   int passEntityNum, int contentmask);
void            trap_TraceNoEnts(trace_t * results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
								 int passEntityNum, int contentmask);
void            trap_TraceCapsule(trace_t * results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
								  int passEntityNum, int contentmask);
void            trap_TraceCapsuleNoEnts(trace_t * results, const vec3_t start, const vec3_t mins, const vec3_t maxs,
										const vec3_t end, int passEntityNum, int contentmask);
int             trap_PointContents(const vec3_t point, int passEntityNum);
qboolean        trap_InPVS(const vec3_t p1, const vec3_t p2);
qboolean        trap_InPVSIgnorePortals(const vec3_t p1, const vec3_t p2);
void            trap_AdjustAreaPortalState(gentity_t * ent, qboolean open);
qboolean        trap_AreasConnected(int area1, int area2);
void            trap_LinkEntity(gentity_t * ent);
void            trap_UnlinkEntity(gentity_t * ent);
int             trap_EntitiesInBox(const vec3_t mins, const vec3_t maxs, int *entityList, int maxcount);
qboolean        trap_EntityContact(const vec3_t mins, const vec3_t maxs, const gentity_t * ent);
qboolean        trap_EntityContactCapsule(const vec3_t mins, const vec3_t maxs, const gentity_t * ent);
int             trap_BotAllocateClient(void);	// NO BOTLIB
void            trap_BotFreeClient(int clientNum);	// NO BOTLIB
int             trap_BotGetSnapshotEntity(int clientNum, int sequence);	// NO BOTLIB
int             trap_BotGetServerCommand(int clientNum, char *message, int size);	// NO BOTLIB
void            trap_BotUserCommand(int client, usercmd_t * ucmd);	// NO BOTLIB
void            trap_BotClientCommand(int clientNum, char *command);	// NO BOTLIB
void            trap_GetUsercmd(int clientNum, usercmd_t * cmd);
qboolean        trap_GetEntityToken(char *buffer, int bufferSize);

int             trap_RealTime(qtime_t * qtime);

#define DEBUGWEAVEING_TST(level) (g_debugWeaving.integer >= (level))

#define DEBUGWEAVEING_LVL(str, level) {if(DEBUGWEAVEING_TST(level)) Com_Printf("%s\n", (str));}

#define DEBUGWEAVEING(str) DEBUGWEAVEING_LVL((str), 1)

#if defined(BRAINWORKS)
int             trap_BotLibSetup(void);
int             trap_BotLibShutdown(void);
int             trap_BotLibVarSet(char *var_name, char *value);
int             trap_BotLibVarGet(char *var_name, char *value, int size);
int             trap_BotLibDefine(char *string);
int             trap_BotLibStartFrame(float time);
int             trap_BotLibLoadMap(const char *mapname);
int             trap_BotLibUpdateEntity(int ent, void /* struct bot_updateentity_s */ *bue);	// COULD BE REMOVED

//#if defined(GLADIATOR)
int             trap_BotLibTest(int parm0, char *parm1, vec3_t parm2, vec3_t parm3);	// COULD BE REMOVED

//#endif

int             trap_AAS_BBoxAreas(vec3_t absmins, vec3_t absmaxs, int *areas, int maxareas);
int             trap_AAS_AreaInfo(int areanum, void /* struct aas_areainfo_s */ *info);

#if defined(GLADIATOR)
void            trap_AAS_EntityInfo(int entnum, void /* struct aas_entityinfo_s */ *info);
#endif

int             trap_AAS_Initialized(void);
void            trap_AAS_PresenceTypeBoundingBox(int presencetype, vec3_t mins, vec3_t maxs);

#if defined(GLADIATOR)
float           trap_AAS_Time(void);
#endif

int             trap_AAS_PointAreaNum(vec3_t point);

#if defined(GLADIATOR)
int             trap_AAS_PointReachabilityAreaIndex(vec3_t point);
#endif
int             trap_AAS_TraceAreas(vec3_t start, vec3_t end, int *areas, vec3_t * points, int maxareas);

int             trap_AAS_PointContents(vec3_t point);

#if defined(GLADIATOR)
int             trap_AAS_NextBSPEntity(int ent);
int             trap_AAS_ValueForBSPEpairKey(int ent, char *key, char *value, int size);
int             trap_AAS_VectorForBSPEpairKey(int ent, char *key, vec3_t v);
int             trap_AAS_FloatForBSPEpairKey(int ent, char *key, float *value);
int             trap_AAS_IntForBSPEpairKey(int ent, char *key, int *value);
#endif

int             trap_AAS_AreaReachability(int areanum);

int             trap_AAS_AreaTravelTimeToGoalArea(int areanum, vec3_t origin, int goalareanum, int travelflags);
int             trap_AAS_EnableRoutingArea(int areanum, int enable);
int             trap_AAS_PredictRoute(void /*struct aas_predictroute_s */ *route, int areanum, vec3_t origin,
									  int goalareanum, int travelflags, int maxareas, int maxtime,
									  int stopevent, int stopcontents, int stoptfl, int stopareanum);

#if defined(GLADIATOR)
int             trap_AAS_AlternativeRouteGoals(vec3_t start, int startareanum, vec3_t goal, int goalareanum, int travelflags,
											   void /*struct aas_altroutegoal_s */ *altroutegoals, int maxaltroutegoals,
											   int type);
int             trap_AAS_Swimming(vec3_t origin);
int             trap_AAS_PredictClientMovement(void /* aas_clientmove_s */ *move, int entnum, vec3_t origin, int presencetype,
											   int onground, vec3_t velocity, vec3_t cmdmove, int cmdframes, int maxframes,
											   float frametime, int stopevent, int stopareanum, int visualize);
#endif


void            trap_EA_Say(int client, char *str);
void            trap_EA_SayTeam(int client, char *str);
void            trap_EA_Command(int client, char *command);

void            trap_EA_Action(int client, int action);

#if defined(GLADIATOR)
void            trap_EA_Gesture(int client);
void            trap_EA_Talk(int client);
void            trap_EA_Attack(int client);
void            trap_EA_Use(int client);
void            trap_EA_Respawn(int client);
void            trap_EA_Crouch(int client);
void            trap_EA_MoveUp(int client);
void            trap_EA_MoveDown(int client);
void            trap_EA_MoveForward(int client);
void            trap_EA_MoveBack(int client);
void            trap_EA_MoveLeft(int client);
void            trap_EA_MoveRight(int client);
#endif
void            trap_EA_SelectWeapon(int client, int weapon);

#if defined(GLADIATOR)
void            trap_EA_Jump(int client);
void            trap_EA_DelayedJump(int client);
void            trap_EA_Move(int client, vec3_t dir, float speed);
#endif
void            trap_EA_View(int client, vec3_t viewangles);

#if defined(GLADIATOR)
void            trap_EA_EndRegular(int client, float thinktime);
#endif
void            trap_EA_GetInput(int client, float thinktime, void /* struct bot_input_s */ *input);
void            trap_EA_ResetInput(int client);

int             trap_BotLoadCharacter(char *charfile, float skill);
void            trap_BotFreeCharacter(int character);
float           trap_Characteristic_Float(int character, int index);
float           trap_Characteristic_BFloat(int character, int index, float min, float max);
int             trap_Characteristic_Integer(int character, int index);
int             trap_Characteristic_BInteger(int character, int index, int min, int max);
void            trap_Characteristic_String(int character, int index, char *buf, int size);

int             trap_BotAllocChatState(void);
void            trap_BotFreeChatState(int handle);
void            trap_BotQueueConsoleMessage(int chatstate, int type, char *message);
void            trap_BotRemoveConsoleMessage(int chatstate, int handle);
int             trap_BotNextConsoleMessage(int chatstate, void /* struct bot_consolemessage_s */ *cm);
int             trap_BotNumConsoleMessages(int chatstate);
void            trap_BotInitialChat(int chatstate, char *type, int mcontext, char *var0, char *var1, char *var2, char *var3,
									char *var4, char *var5, char *var6, char *var7);
int             trap_BotNumInitialChats(int chatstate, char *type);
int             trap_BotReplyChat(int chatstate, char *message, int mcontext, int vcontext, char *var0, char *var1, char *var2,
								  char *var3, char *var4, char *var5, char *var6, char *var7);
int             trap_BotChatLength(int chatstate);
void            trap_BotEnterChat(int chatstate, int client, int sendto);
void            trap_BotGetChatMessage(int chatstate, char *buf, int size);
int             trap_StringContains(char *str1, char *str2, int casesensitive);
int             trap_BotFindMatch(char *str, void /* struct bot_match_s */ *match, unsigned long int context);
void            trap_BotMatchVariable(void /* struct bot_match_s */ *match, int variable, char *buf, int size);
void            trap_UnifyWhiteSpaces(char *string);
void            trap_BotReplaceSynonyms(char *string, unsigned long int context);
int             trap_BotLoadChatFile(int chatstate, char *chatfile, char *chatname);
void            trap_BotSetChatGender(int chatstate, int gender);
void            trap_BotSetChatName(int chatstate, char *name, int client);

#if defined(GLADIATOR)
void            trap_BotResetGoalState(int goalstate);
void            trap_BotRemoveFromAvoidGoals(int goalstate, int number);
void            trap_BotResetAvoidGoals(int goalstate);
void            trap_BotPushGoal(int goalstate, void /* struct bot_goal_s */ *goal);
void            trap_BotPopGoal(int goalstate);
void            trap_BotEmptyGoalStack(int goalstate);
void            trap_BotDumpAvoidGoals(int goalstate);
void            trap_BotDumpGoalStack(int goalstate);
void            trap_BotGoalName(int number, char *name, int size);
int             trap_BotGetTopGoal(int goalstate, void /* struct bot_goal_s */ *goal);
int             trap_BotGetSecondGoal(int goalstate, void /* struct bot_goal_s */ *goal);
int             trap_BotChooseLTGItem(int goalstate, vec3_t origin, int *inventory, int travelflags);
int             trap_BotChooseNBGItem(int goalstate, vec3_t origin, int *inventory, int travelflags,
									  void /* struct bot_goal_s */ *ltg, float maxtime);
#endif
int             trap_BotTouchingGoal(vec3_t origin, void /* struct bot_goal_s */ *goal);
int             trap_BotItemGoalInVisButNotVisible(int viewer, vec3_t eye, vec3_t viewangles, void /* struct bot_goal_s */ *goal);
int             trap_BotGetNextCampSpotGoal(int num, void /* struct bot_goal_s */ *goal);

#if defined(GLADIATOR)
int             trap_BotGetMapLocationGoal(char *name, void /* struct bot_goal_s */ *goal);
#endif
int             trap_BotGetLevelItemGoal(int index, char *classname, void /* struct bot_goal_s */ *goal);

#if defined(GLADIATOR)
float           trap_BotAvoidGoalTime(int goalstate, int number);
void            trap_BotSetAvoidGoalTime(int goalstate, int number, float avoidtime);
void            trap_BotInitLevelItems(void);
void            trap_BotUpdateEntityItems(void);
int             trap_BotLoadItemWeights(int goalstate, char *filename);
void            trap_BotFreeItemWeights(int goalstate);
void            trap_BotInterbreedGoalFuzzyLogic(int parent1, int parent2, int child);
void            trap_BotSaveGoalFuzzyLogic(int goalstate, char *filename);
void            trap_BotMutateGoalFuzzyLogic(int goalstate, float range);
int             trap_BotAllocGoalState(int state);
void            trap_BotFreeGoalState(int handle);
#endif

void            trap_BotResetMoveState(int movestate);
void            trap_BotMoveToGoal(void /* struct bot_moveresult_s */ *result, int movestate, void /* struct bot_goal_s */ *goal,
								   int travelflags);
int             trap_BotMoveInDirection(int movestate, vec3_t dir, float speed, int type);
void            trap_BotResetAvoidReach(int movestate);

#if defined(GLADIATOR)
void            trap_BotResetLastAvoidReach(int movestate);
int             trap_BotReachabilityArea(vec3_t origin, int testground);
#endif

int             trap_BotMovementViewTarget(int movestate, void /* struct bot_goal_s */ *goal, int travelflags, float lookahead,
										   vec3_t target);
int             trap_BotPredictVisiblePosition(vec3_t origin, int areanum, void /* struct bot_goal_s */ *goal, int travelflags,
											   vec3_t target);
int             trap_BotAllocMoveState(void);
void            trap_BotFreeMoveState(int handle);
void            trap_BotInitMoveState(int handle, void /* struct bot_initmove_s */ *initmove);
void            trap_BotAddAvoidSpot(int movestate, vec3_t origin, float radius, int type);

#if defined(GLADIATOR)
int             trap_BotChooseBestFightWeapon(int weaponstate, int *inventory);
void            trap_BotGetWeaponInfo(int weaponstate, int weapon, void /* struct weaponinfo_s */ *weaponinfo);
int             trap_BotLoadWeaponWeights(int weaponstate, char *filename);
int             trap_BotAllocWeaponState(void);
void            trap_BotFreeWeaponState(int weaponstate);
void            trap_BotResetWeaponState(int weaponstate);

int             trap_GeneticParentsAndChildSelection(int numranks, float *ranks, int *parent1, int *parent2, int *child);
#endif

#endif							// defined(BRAINWORKS)

#endif							// __G_LOCAL_H
