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
// g_main.c

#include "g_local.h"
#include "g_lua.h"
#include "g_spell_util.h"

level_locals_t  level;

typedef struct
{
	vmCvar_t       *vmCvar;
	char           *cvarName;
	char           *defaultString;
	int             cvarFlags;
	int             modificationCount;	// for tracking changes
	qboolean        trackChange;	// track this variable, and announce if changed
	qboolean        teamShader;	// track and if changed, update shader state
} cvarTable_t;

gentity_t       g_entities[MAX_GENTITIES];
gclient_t       g_clients[MAX_CLIENTS];

vmCvar_t        g_gametype;
vmCvar_t        g_dmflags;
vmCvar_t        g_fraglimit;
vmCvar_t        g_timelimit;
vmCvar_t        g_capturelimit;
vmCvar_t        g_friendlyFire;
vmCvar_t        g_password;
vmCvar_t        g_needpass;
vmCvar_t        g_maxclients;
vmCvar_t        g_maxGameClients;
vmCvar_t        g_dedicated;
vmCvar_t        g_speed;
vmCvar_t        g_gravity;
vmCvar_t        g_cheats;
vmCvar_t        g_knockback;
vmCvar_t        g_knockbackZ;
vmCvar_t        g_quadfactor;
vmCvar_t        g_forcerespawn;
vmCvar_t        g_inactivity;
vmCvar_t        g_debugMove;
vmCvar_t        g_debugDamage;
vmCvar_t        g_debugAlloc;
vmCvar_t        g_debugLua;
vmCvar_t        g_weaponRespawn;
vmCvar_t        g_weaponTeamRespawn;
vmCvar_t        g_motd;
vmCvar_t        g_synchronousClients;
vmCvar_t        g_warmup;
vmCvar_t        g_doWarmup;
vmCvar_t        g_readyPercent;
vmCvar_t        g_minPlayers;
vmCvar_t        g_restarted;
vmCvar_t        g_logFile;
vmCvar_t        g_logFileSync;
vmCvar_t        g_blood;
vmCvar_t        g_podiumDist;
vmCvar_t        g_podiumDrop;
vmCvar_t        g_allowVote;
vmCvar_t        g_teamAutoJoin;
vmCvar_t        g_teamForceBalance;
vmCvar_t        g_banIPs;
vmCvar_t        g_filterBan;
vmCvar_t        g_smoothClients;
vmCvar_t        g_rankings;
vmCvar_t        g_listEntity;

vmCvar_t        g_debugWeaving;
vmCvar_t        g_debugEntities;

vmCvar_t        g_obeliskHealth;
vmCvar_t        g_obeliskRegenPeriod;
vmCvar_t        g_obeliskRegenAmount;
vmCvar_t        g_obeliskRespawnDelay;
vmCvar_t        g_cubeTimeout;
vmCvar_t        g_enableDust;
vmCvar_t        g_enableBreath;

#ifdef MISSIONPACK
vmCvar_t        g_proxMineTimeout;
#endif

//unlagged - server options
vmCvar_t        g_delagHitscan;
vmCvar_t        g_unlaggedVersion;
vmCvar_t        g_truePing;
vmCvar_t        g_lightningDamage;
vmCvar_t        sv_fps;

//unlagged - server options

vmCvar_t        g_rocketAcceleration;
vmCvar_t        g_rocketVelocity;

vmCvar_t        g_teamSpawnWaves;
vmCvar_t        g_teamSpawnBlue;
vmCvar_t        g_teamSpawnRed;

vmCvar_t        g_currentRound;
vmCvar_t        g_nextTimeLimit;
vmCvar_t        g_swTeamSwitching;
vmCvar_t        g_swTeamToClan;
vmCvar_t        g_swMaps;
vmCvar_t        g_swMap;
vmCvar_t        g_scoreA;
vmCvar_t        g_scoreB;

vmCvar_t        g_woundedHealth;

// these cvars are shared accross both games
vmCvar_t        pm_airControl;
vmCvar_t        pm_fastWeaponSwitches;
vmCvar_t        pm_fixedPmove;
vmCvar_t        pm_fixedPmoveFPS;

vmCvar_t        lua_modules;
vmCvar_t        lua_allowedModules;

#if defined(ACEBOT)
vmCvar_t        ace_debug;
vmCvar_t        ace_showNodes;
vmCvar_t        ace_showLinks;
vmCvar_t        ace_showPath;
vmCvar_t        ace_pickLongRangeGoal;
vmCvar_t        ace_pickShortRangeGoal;
vmCvar_t        ace_attackEnemies;
vmCvar_t        ace_spSkill;
vmCvar_t        ace_botsFile;
#endif


static cvarTable_t gameCvarTable[] = {
	// don't override the cheat state set by the system
	{&g_cheats, "sv_cheats", "", 0, 0, qfalse},

	// noset vars
	{NULL, "gamename", GAMEVERSION, CVAR_SERVERINFO | CVAR_ROM, 0, qfalse},
	{NULL, "gamedate", __DATE__, CVAR_ROM, 0, qfalse},
	{&g_restarted, "g_restarted", "0", CVAR_ROM, 0, qfalse},
	{NULL, "sv_mapname", "", CVAR_SERVERINFO | CVAR_ROM, 0, qfalse},

	// latched vars
	{&g_gametype, "g_gametype", "8", CVAR_SERVERINFO | CVAR_USERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, qfalse},

	{&g_maxclients, "sv_maxclients", "8", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, qfalse},
	{&g_maxGameClients, "g_maxGameClients", "0", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, qfalse},

	// change anytime vars
	{&g_dmflags, "dmflags", "0", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, qtrue},
	{&g_fraglimit, "fraglimit", "20", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue},
	{&g_timelimit, "timelimit", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue},
	{&g_capturelimit, "capturelimit", "8", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue},

	{&g_synchronousClients, "g_synchronousClients", "0", CVAR_SYSTEMINFO, 0, qfalse},

	{&g_friendlyFire, "g_friendlyFire", "0", CVAR_ARCHIVE, 0, qtrue},

	{&g_teamAutoJoin, "g_teamAutoJoin", "0", CVAR_ARCHIVE},
	{&g_teamForceBalance, "g_teamForceBalance", "0", CVAR_ARCHIVE},

	{&g_warmup, "g_warmup", "20", CVAR_ARCHIVE, 0, qtrue},
	{&g_doWarmup, "g_doWarmup", "1", 0, 0, qtrue},
	{&g_readyPercent, "g_readyPercent", "100", 0, 0, qtrue},
	{&g_minPlayers, "g_minPlayers", "1", 0, 0, qtrue},
	{&g_logFile, "g_logFile", "games.log", CVAR_ARCHIVE, 0, qfalse},
	{&g_logFileSync, "g_logFileSync", "0", CVAR_ARCHIVE, 0, qfalse},

	{&g_password, "g_password", "", CVAR_USERINFO, 0, qfalse},

	{&g_banIPs, "g_banIPs", "", CVAR_ARCHIVE, 0, qfalse},
	{&g_filterBan, "g_filterBan", "1", CVAR_ARCHIVE, 0, qfalse},

	{&g_needpass, "g_needpass", "0", CVAR_SERVERINFO | CVAR_ROM, 0, qfalse},

	{&g_dedicated, "dedicated", "0", 0, 0, qfalse},

	{&g_speed, "g_speed", "300", 0, 0, qtrue},
	{&g_gravity, "g_gravity", DEFAULT_GRAVITY_STRING, CVAR_SYSTEMINFO, 0, qtrue},
	{&g_knockback, "g_knockback", "1000", 0, 0, qtrue},
	{&g_knockbackZ, "g_knockbackZ", "40", 0, 0, qtrue},
	{&g_quadfactor, "g_quadfactor", "4", 0, 0, qtrue},
	{&g_weaponRespawn, "g_weaponrespawn", "5", 0, 0, qtrue},
	{&g_weaponTeamRespawn, "g_weaponTeamRespawn", "30", 0, 0, qtrue},
	{&g_forcerespawn, "g_forcerespawn", "20", 0, 0, qtrue},
	{&g_inactivity, "g_inactivity", "0", 0, 0, qtrue},
	{&g_debugDamage, "g_debugDamage", "0", 0, 0, qfalse},
	{&g_debugAlloc, "g_debugAlloc", "0", 0, 0, qfalse},
	{&g_debugLua, "g_debugLua", "0", 0, 0, qfalse},
	{&g_motd, "g_motd", "", 0, 0, qfalse},
	{&g_blood, "com_blood", "1", 0, 0, qfalse},

	{&g_debugWeaving, "g_debugWeaving", "0", 0, 0, qfalse},
	{&g_debugEntities, "g_debugEntities", "0", 0, 0, qfalse},

	{&g_podiumDist, "g_podiumDist", "80", 0, 0, qfalse},
	{&g_podiumDrop, "g_podiumDrop", "70", 0, 0, qfalse},

	{&g_allowVote, "g_allowVote", "1", CVAR_ARCHIVE, 0, qfalse},
	{&g_listEntity, "g_listEntity", "0", 0, 0, qfalse},

	{&g_obeliskHealth, "g_obeliskHealth", "2500", 0, 0, qfalse},
	{&g_obeliskRegenPeriod, "g_obeliskRegenPeriod", "1", 0, 0, qfalse},
	{&g_obeliskRegenAmount, "g_obeliskRegenAmount", "15", 0, 0, qfalse},
	{&g_obeliskRespawnDelay, "g_obeliskRespawnDelay", "10", CVAR_SERVERINFO, 0, qfalse},

	{&g_cubeTimeout, "g_cubeTimeout", "30", 0, 0, qfalse},

	{&g_enableDust, "g_enableDust", "0", CVAR_SERVERINFO, 0, qtrue, qfalse},
	{&g_enableBreath, "g_enableBreath", "0", CVAR_SERVERINFO, 0, qtrue, qfalse},

#ifdef MISSIONPACK
	{&g_proxMineTimeout, "g_proxMineTimeout", "20000", 0, 0, qfalse},
#endif

	{&g_smoothClients, "g_smoothClients", "1", 0, 0, qfalse},

	{&g_rocketAcceleration, "g_rocketAcceleration", "0", 0, 0, qfalse},
	{&g_rocketVelocity, "g_rocketVelocity", "900", 0, 0, qfalse},

	{&g_smoothClients, "g_smoothClients", "1", 0, 0, qfalse},
	{&g_rankings, "g_rankings", "0", 0, 0, qfalse},

	{&g_teamSpawnWaves, "g_teamSpawnWaves", "1", CVAR_SERVERINFO | CVAR_USERINFO, 0, qfalse},
	{&g_teamSpawnBlue, "g_teamSpawnBlue", "15", CVAR_SERVERINFO | CVAR_USERINFO, 0, qfalse},
	{&g_teamSpawnRed, "g_teamSpawnRed", "15", CVAR_SERVERINFO | CVAR_USERINFO, 0, qfalse},

	{&g_currentRound, "g_currentRound", "0", CVAR_SERVERINFO | CVAR_USERINFO | CVAR_NORESTART, 0, qfalse},
	{&g_nextTimeLimit, "g_nextTimeLimit", "0", CVAR_SERVERINFO | CVAR_USERINFO | CVAR_NORESTART, 0, qfalse},
	{&g_swTeamSwitching, "g_swTeamSwitching", "0", CVAR_SERVERINFO | CVAR_USERINFO | CVAR_NORESTART, 0, qfalse},
	{&g_swTeamToClan, "g_swTeamToClan", "0", CVAR_SERVERINFO | CVAR_USERINFO | CVAR_NORESTART, 0, qfalse},
	{&g_swMaps, "g_swMaps", "", CVAR_SERVERINFO | CVAR_USERINFO | CVAR_NORESTART, 0, qfalse},
	{&g_swMap, "g_swMap", "0", CVAR_SERVERINFO | CVAR_USERINFO | CVAR_NORESTART, 0, qfalse},
	{&g_scoreA, "g_scoreA", "0", CVAR_SERVERINFO | CVAR_USERINFO | CVAR_NORESTART, 0, qfalse},
	{&g_scoreB, "g_scoreB", "0", CVAR_SERVERINFO | CVAR_USERINFO | CVAR_NORESTART, 0, qfalse},

	{&g_woundedHealth, "g_woundedHealth", "-50", CVAR_SERVERINFO | CVAR_USERINFO, 0, qfalse},

	// these cvars are shared accross both games
	{&pm_airControl, "pm_airControl", "0", CVAR_SYSTEMINFO, 0, qfalse},
	{&pm_fastWeaponSwitches, "pm_fastWeaponSwitches", "0", CVAR_SYSTEMINFO, 0, qfalse},
	{&pm_fixedPmove, "pm_fixedPmove", "0", CVAR_SYSTEMINFO, 0, qfalse},
	{&pm_fixedPmoveFPS, "pm_fixedPmoveFPS", "125", CVAR_SYSTEMINFO, 0, qfalse},

	{&lua_allowedModules, "lua_allowedModules", "", 0, 0, qfalse},
	{&lua_modules, "lua_modules", "", 0, 0, qfalse},


//unlagged - server options
	{&g_delagHitscan, "g_delagHitscan", "1", CVAR_ARCHIVE | CVAR_SERVERINFO, 0, qtrue},
	{&g_unlaggedVersion, "g_unlaggedVersion", "2.0", CVAR_ROM | CVAR_SERVERINFO, 0, qfalse},
	{&g_truePing, "g_truePing", "1", CVAR_ARCHIVE, 0, qtrue},
	{&g_lightningDamage, "g_lightningDamage", "8", 0, 0, qtrue},
	// it's CVAR_SYSTEMINFO so the client's sv_fps will be automagically set to its value
	{&sv_fps, "sv_fps", "20", CVAR_SYSTEMINFO | CVAR_ARCHIVE, 0, qfalse},
//unlagged - server options

#if defined(ACEBOT)
	{&ace_debug, "ace_debug", "0", 0, 0, qfalse},
	{&ace_showNodes, "ace_showNodes", "0", 0, 0, qfalse},
	{&ace_showLinks, "ace_showLinks", "0", 0, 0, qfalse},
	{&ace_showPath, "ace_showPath", "0", 0, 0, qfalse},
	{&ace_pickLongRangeGoal, "ace_pickLongRangeGoal", "1", 0, 0, qfalse},
	{&ace_pickShortRangeGoal, "ace_pickShortRangeGoal", "1", 0, 0, qfalse},
	{&ace_attackEnemies, "ace_attackEnemies", "1", 0, 0, qfalse},
	{&ace_spSkill, "g_spSkill", "3", 0, 0, qfalse},	// FIXME rename
	{&ace_botsFile, "g_botsFile", "3", 0, 0, qfalse},	// FIXME rename
#endif
};

// bk001129 - made static to avoid aliasing
static int      gameCvarTableSize = sizeof(gameCvarTable) / sizeof(gameCvarTable[0]);


void            G_InitGame(int levelTime, int randomSeed, int restart);
void            G_RunFrame(int levelTime);
void            G_ShutdownGame(int restart);
void            CheckExitRules(void);


/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
intptr_t vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9,
				int arg10, int arg11)
{
	switch (command)
	{
		case GAME_INIT:
			G_InitGame(arg0, arg1, arg2);
			return 0;
		case GAME_SHUTDOWN:
			G_ShutdownGame(arg0);
			return 0;
		case GAME_CLIENT_CONNECT:
			return (intptr_t) ClientConnect(arg0, arg1, arg2);
		case GAME_CLIENT_THINK:
			ClientThink(arg0);
			return 0;
		case GAME_CLIENT_USERINFO_CHANGED:
			ClientUserinfoChanged(arg0);
			return 0;
		case GAME_CLIENT_DISCONNECT:
			ClientDisconnect(arg0);
			return 0;
		case GAME_CLIENT_BEGIN:
			ClientBegin(arg0);
			return 0;
		case GAME_CLIENT_COMMAND:
			ClientCommand(arg0);
			return 0;
		case GAME_RUN_FRAME:
			G_RunFrame(arg0);
			return 0;
		case GAME_CONSOLE_COMMAND:
			return ConsoleCommand();
		case BOTAI_START_FRAME:
#if defined(BRAINWORKS)
			return BotAIStartFrame(arg0);
#elif defined(ACEBOT)
			ACEAI_StartFrame(arg0);
			return 0;
#else
			return 0;
#endif
	}

	return -1;
}


void QDECL G_Printf(const char *fmt, ...)
{
	va_list         argptr;
	char            text[1024];

	va_start(argptr, fmt);
	Q_vsnprintf(text, sizeof(text), fmt, argptr);
	va_end(argptr);

#ifdef G_LUA
	// Lua API callbacks
	G_LuaHook_Print(text);
#endif

	trap_Printf(text);
}

/*
 * Prints text to 1 client, ent
 */
void QDECL G_PrintfClient(gentity_t * ent, const char *fmt, ...)
{
	va_list         argptr;
	char            text[1024];

	if(!ent || !ent->client)
	{
		return;
	}

	va_start(argptr, fmt);
	Q_vsnprintf(text, sizeof(text), fmt, argptr);
	va_end(argptr);

	trap_SendServerCommand(ent - g_entities, va("print \"%s\n\"", text));
}

void QDECL G_Error(const char *fmt, ...)
{
	va_list         argptr;
	char            text[1024];

	va_start(argptr, fmt);
	Q_vsnprintf(text, sizeof(text), fmt, argptr);
	va_end(argptr);

#ifdef G_LUA
	G_LuaShutdown();
#endif

	trap_Error(text);
}

/*
================
G_FindTeams

Chain together all entities with a matching team field.
Entity teams are used for item groups and multi-entity mover groups.

All but the first will have the FL_TEAMSLAVE flag set and teammaster field set
All but the last will have the teamchain field set to the next one
================
*/
void G_FindTeams(void)
{
	gentity_t      *e, *e2;
	int             i, j;
	int             c, c2;

	c = 0;
	c2 = 0;
	for(i = 1, e = g_entities + i; i < level.numEntities; i++, e++)
	{
		if(!e->inuse)
			continue;
		if(!e->team)
			continue;
		if(e->flags & FL_TEAMSLAVE)
			continue;
		e->teammaster = e;
		c++;
		c2++;
		for(j = i + 1, e2 = e + 1; j < level.numEntities; j++, e2++)
		{
			if(!e2->inuse)
				continue;
			if(!e2->team)
				continue;
			if(e2->flags & FL_TEAMSLAVE)
				continue;
			if(!strcmp(e->team, e2->team))
			{
				c2++;
				e2->teamchain = e->teamchain;
				e->teamchain = e2;
				e2->teammaster = e;
				e2->flags |= FL_TEAMSLAVE;

				// make sure that targets only point at the master
				if(e2->name)
				{
					e->name = e2->name;
					e2->name = NULL;
				}
			}
		}
	}

	G_Printf("%i teams with %i entities\n", c, c2);
}




/*
=================
G_RegisterCvars
=================
*/
void G_RegisterCvars(void)
{
	int             i;
	cvarTable_t    *cv;

	for(i = 0, cv = gameCvarTable; i < gameCvarTableSize; i++, cv++)
	{
		trap_Cvar_Register(cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags);
		if(cv->vmCvar)
			cv->modificationCount = cv->vmCvar->modificationCount;
	}

	// check some things
	if(g_gametype.integer < 0 || g_gametype.integer >= GT_MAX_GAME_TYPE)
	{
		G_Printf("g_gametype %i is out of range, defaulting to 0\n", g_gametype.integer);
		trap_Cvar_Set("g_gametype", "0");
	}

	if(pm_fixedPmoveFPS.integer < 60)
		trap_Cvar_Set("pm_fixedPmoveFPS", "60");
	else if(pm_fixedPmoveFPS.integer > 333)
		trap_Cvar_Set("pm_fixedPmoveFPS", "333");

	level.warmupModificationCount = g_warmup.modificationCount;
}

/*
=================
G_UpdateCvars
=================
*/
void G_UpdateCvars(void)
{
	int             i;
	cvarTable_t    *cv;

	for(i = 0, cv = gameCvarTable; i < gameCvarTableSize; i++, cv++)
	{
		if(cv->vmCvar)
		{
			trap_Cvar_Update(cv->vmCvar);

			if(cv->modificationCount != cv->vmCvar->modificationCount)
			{
				cv->modificationCount = cv->vmCvar->modificationCount;

				if(cv->trackChange)
				{
					trap_SendServerCommand(-1, va("print \"Server: %s changed to %s\n\"", cv->cvarName, cv->vmCvar->string));
				}
			}
		}
	}
}


/*
============
G_InitSWRound

============
*/
void G_InitSWRound(void)
{
	char            cs[MAX_STRING_CHARS];

	trap_GetConfigstring(CS_SWINFO, cs, sizeof(cs));
	Info_SetValueForKey(cs, "winner", "0");
	Info_SetValueForKey(cs, "defender", "1");
	trap_SetConfigstring(CS_SWINFO, cs);
}


/*
============
G_InitGame

============
*/
void G_InitGame(int levelTime, int randomSeed, int restart)
{
	int             i;

	G_Printf("------- Game Initialization -------\n");
	G_Printf("gamename: %s\n", GAMEVERSION);
	G_Printf("gamedate: %s\n", __DATE__);

	srand(randomSeed);

	G_RegisterCvars();

	G_ProcessIPBans();

	G_InitMemory();

	// set some level globals
	memset(&level, 0, sizeof(level));
	level.time = levelTime;
	level.startTime = levelTime;
	level.teamSwapped = qfalse;

	if(g_gametype.integer != GT_SINGLE_PLAYER && g_logFile.string[0])
	{
		if(g_logFileSync.integer)
		{
			trap_FS_FOpenFile(g_logFile.string, &level.logFile, FS_APPEND_SYNC);
		}
		else
		{
			trap_FS_FOpenFile(g_logFile.string, &level.logFile, FS_APPEND);
		}
		if(!level.logFile)
		{
			G_Printf("WARNING: Couldn't open logfile: %s\n", g_logFile.string);
		}
		else
		{
			char            serverinfo[MAX_INFO_STRING];

			trap_GetServerinfo(serverinfo, sizeof(serverinfo));

			G_LogPrintf("------------------------------------------------------------\n");
			G_LogPrintf("InitGame: %s\n", serverinfo);
		}
	}
	else
	{
		G_Printf("Not logging to disk.\n");
	}

#ifdef G_LUA
	G_LuaInit();
#endif

	G_InitWorldSession();

	// initialize all entities for this game
	memset(g_entities, 0, MAX_GENTITIES * sizeof(g_entities[0]));
	level.gentities = g_entities;

	// initialize all clients for this game
	level.maxclients = g_maxclients.integer;
	memset(g_clients, 0, MAX_CLIENTS * sizeof(g_clients[0]));
	level.clients = g_clients;

	// set client fields on player ents
	for(i = 0; i < level.maxclients; i++)
	{
		g_entities[i].client = level.clients + i;
	}

	// always leave room for the max number of clients,
	// even if they aren't all used, so numbers inside that
	// range are NEVER anything but clients
	level.numEntities = MAX_CLIENTS;

	// let the server system know where the entites are
	trap_LocateGameData(level.gentities, level.numEntities, sizeof(gentity_t), &level.clients[0].ps, sizeof(level.clients[0]));

	// reserve some spots for dead player bodies
	InitBodyQue();

	ClearRegisteredItems();

	// parse the key/value pairs and spawn gentities
	G_SpawnEntitiesFromString();

	// general initialization
	G_FindTeams();

	// make sure we have flags for CTF, etc
	if(g_gametype.integer >= GT_TEAM)
	{
		G_CheckTeamItems();
	}

	// initialize spawn times
	if(g_teamSpawnWaves.integer >= 1)
	{
		// Initialize spawn periods
		if(g_teamSpawnRed.integer < 0)
		{
			// default to 15 seconds
			g_teamSpawnRed.integer = 15;
		}
		level.teamSpawnPeriodRed = g_teamSpawnRed.integer * 1000;

		if(g_teamSpawnBlue.integer < 0)
		{
			// default to 15 seconds
			g_teamSpawnBlue.integer = 15;
		}
		level.teamSpawnPeriodBlue = g_teamSpawnBlue.integer * 1000;

		// Initialize random phase. Subtract period to ensure level.time > level.teamSpawnPreviousTime
		level.teamSpawnPreviousTimeRed = (level.teamSpawnPeriodRed * random()) - level.teamSpawnPeriodRed;
		level.teamSpawnPreviousTimeBlue = (level.teamSpawnPeriodBlue * random()) - level.teamSpawnPeriodBlue;
	}

	SaveRegisteredItems();

	G_Printf("-----------------------------------\n");

	if(g_gametype.integer == GT_SINGLE_PLAYER || trap_Cvar_VariableIntegerValue("com_buildScript"))
	{
		G_ModelIndex(SP_PODIUM_MODEL);
		G_SoundIndex("sound/player/gurp1.ogg");
		G_SoundIndex("sound/player/gurp2.ogg");
	}

#if defined(BRAINWORKS)
	if(trap_Cvar_VariableIntegerValue("bot_enable"))
	{
		BotAISetup(restart);
		BotAILoadMap(restart);
		G_InitBots(restart);
	}
#elif defined(ACEBOT)
	ACEND_InitNodes();
	ACEND_LoadNodes();
	ACESP_InitBots(restart);
#endif

	if(g_gametype.integer == GT_OBJECTIVE_SW)
	{
		G_InitSWRound();
	}

	G_StatInit();

#ifdef G_LUA
	// Lua API callbacks
	G_LuaHook_InitGame(levelTime, randomSeed, restart);
#endif
}



/*
=================
G_ShutdownGame
=================
*/
void G_ShutdownGame(int restart)
{
	G_Printf("==== ShutdownGame ====\n");

#ifdef G_LUA
	// quad - Lua API
	G_LuaHook_ShutdownGame(restart);
	G_LuaShutdown();
#endif

	if(level.logFile)
	{
		G_LogPrintf("ShutdownGame:\n");
		G_LogPrintf("------------------------------------------------------------\n");
		trap_FS_FCloseFile(level.logFile);
	}

	// write all the client session data so we can get it back
	G_WriteSessionData();

#if defined(BRAINWORKS)
	if(trap_Cvar_VariableIntegerValue("bot_enable"))
	{
		BotAIShutdown(restart);
	}
#endif

}

//===================================================================

void QDECL Com_Error(int level, const char *error, ...)
{
	va_list         argptr;
	char            text[1024];

	va_start(argptr, error);
	Q_vsnprintf(text, sizeof(text), error, argptr);
	va_end(argptr);

	G_Error("%s", text);
}

void QDECL Com_Printf(const char *msg, ...)
{
	va_list         argptr;
	char            text[1024];

	va_start(argptr, msg);
	Q_vsnprintf(text, sizeof(text), msg, argptr);
	va_end(argptr);

	G_Printf("%s", text);
}

/*
========================================================================

PLAYER COUNTING / SCORE SORTING

========================================================================
*/

/*
=============
AddTournamentPlayer

If there are less than two tournament players, put a
spectator in the game and restart
=============
*/
void AddTournamentPlayer(void)
{
	int             i;
	gclient_t      *client;
	gclient_t      *nextInLine;

	if(level.numPlayingClients >= 2)
	{
		return;
	}

	// never change during intermission
	if(level.intermissiontime)
	{
		return;
	}

	nextInLine = NULL;

	for(i = 0; i < level.maxclients; i++)
	{
		client = &level.clients[i];
		if(client->pers.connected != CON_CONNECTED)
		{
			continue;
		}
		if(client->sess.sessionTeam != TEAM_SPECTATOR)
		{
			continue;
		}
		// never select the dedicated follow or scoreboard clients
		if(client->sess.spectatorState == SPECTATOR_SCOREBOARD || client->sess.spectatorClient < 0)
		{
			continue;
		}

		if(!nextInLine || client->sess.spectatorTime < nextInLine->sess.spectatorTime)
		{
			nextInLine = client;
		}
	}

	if(!nextInLine)
	{
		return;
	}

	level.warmupTime = -1;

	// set them to free-for-all team
	SetTeam(&g_entities[nextInLine - level.clients], "f");
}

/*
=======================
RemoveTournamentLoser

Make the loser a spectator at the back of the line
=======================
*/
void RemoveTournamentLoser(void)
{
	int             clientNum;

	if(level.numPlayingClients != 2)
	{
		return;
	}

	clientNum = level.sortedClients[1];

	if(level.clients[clientNum].pers.connected != CON_CONNECTED)
	{
		return;
	}

	// make them a spectator
	SetTeam(&g_entities[clientNum], "s");
}

/*
=======================
RemoveTournamentWinner
=======================
*/
void RemoveTournamentWinner(void)
{
	int             clientNum;

	if(level.numPlayingClients != 2)
	{
		return;
	}

	clientNum = level.sortedClients[0];

	if(level.clients[clientNum].pers.connected != CON_CONNECTED)
	{
		return;
	}

	// make them a spectator
	SetTeam(&g_entities[clientNum], "s");
}

/*
=======================
AdjustTournamentScores
=======================
*/
void AdjustTournamentScores(void)
{
	int             clientNum;

	clientNum = level.sortedClients[0];
	if(level.clients[clientNum].pers.connected == CON_CONNECTED)
	{
		level.clients[clientNum].sess.wins++;
		ClientUserinfoChanged(clientNum);
	}

	clientNum = level.sortedClients[1];
	if(level.clients[clientNum].pers.connected == CON_CONNECTED)
	{
		level.clients[clientNum].sess.losses++;
		ClientUserinfoChanged(clientNum);
	}

}

/*
=============
SortRanks

=============
*/
int QDECL SortRanks(const void *a, const void *b)
{
	gclient_t      *ca, *cb;

	ca = &level.clients[*(int *)a];
	cb = &level.clients[*(int *)b];

	// sort special clients last
	if(ca->sess.spectatorState == SPECTATOR_SCOREBOARD || ca->sess.spectatorClient < 0)
	{
		return 1;
	}
	if(cb->sess.spectatorState == SPECTATOR_SCOREBOARD || cb->sess.spectatorClient < 0)
	{
		return -1;
	}

	// then connecting clients
	if(ca->pers.connected == CON_CONNECTING)
	{
		return 1;
	}
	if(cb->pers.connected == CON_CONNECTING)
	{
		return -1;
	}


	// then spectators
	if(ca->sess.sessionTeam == TEAM_SPECTATOR && cb->sess.sessionTeam == TEAM_SPECTATOR)
	{
		if(ca->sess.spectatorTime < cb->sess.spectatorTime)
		{
			return -1;
		}
		if(ca->sess.spectatorTime > cb->sess.spectatorTime)
		{
			return 1;
		}
		return 0;
	}
	if(ca->sess.sessionTeam == TEAM_SPECTATOR)
	{
		return 1;
	}
	if(cb->sess.sessionTeam == TEAM_SPECTATOR)
	{
		return -1;
	}

	// then sort by score
	if(ca->ps.persistant[PERS_SCORE] > cb->ps.persistant[PERS_SCORE])
	{
		return -1;
	}
	if(ca->ps.persistant[PERS_SCORE] < cb->ps.persistant[PERS_SCORE])
	{
		return 1;
	}
	return 0;
}

/*
============
CalculateRanks

Recalculates the score ranks of all players
This will be called on every client connect, begin, disconnect, death,
and team change.
============
*/
void CalculateRanks(void)
{
	int             i;
	int             rank;
	int             score;
	int             newScore;
	gclient_t      *cl;

	level.follow1 = -1;
	level.follow2 = -1;
	level.numConnectedClients = 0;
	level.numNonSpectatorClients = 0;
	level.numPlayingClients = 0;
	level.numVotingClients = 0;	// don't count bots
	for(i = 0; i < TEAM_NUM_TEAMS; i++)
	{
		level.numteamVotingClients[i] = 0;
	}
	for(i = 0; i < level.maxclients; i++)
	{
		if(level.clients[i].pers.connected != CON_DISCONNECTED)
		{
			level.sortedClients[level.numConnectedClients] = i;
			level.numConnectedClients++;

			if(level.clients[i].sess.sessionTeam != TEAM_SPECTATOR)
			{
				level.numNonSpectatorClients++;

				// decide if this should be auto-followed
				if(level.clients[i].pers.connected == CON_CONNECTED)
				{
					level.numPlayingClients++;
					if(!(g_entities[i].r.svFlags & SVF_BOT))
					{
						level.numVotingClients++;
						if(level.clients[i].sess.sessionTeam == TEAM_RED)
							level.numteamVotingClients[0]++;
						else if(level.clients[i].sess.sessionTeam == TEAM_BLUE)
							level.numteamVotingClients[1]++;
					}
					if(level.follow1 == -1)
					{
						level.follow1 = i;
					}
					else if(level.follow2 == -1)
					{
						level.follow2 = i;
					}
				}
			}
		}
	}

	qsort(level.sortedClients, level.numConnectedClients, sizeof(level.sortedClients[0]), SortRanks);

	// set the rank value for all clients that are connected and not spectators
	if(g_gametype.integer >= GT_TEAM)
	{
		// in team games, rank is just the order of the teams, 0=red, 1=blue, 2=tied
		for(i = 0; i < level.numConnectedClients; i++)
		{
			cl = &level.clients[level.sortedClients[i]];
			if(level.teamScores[TEAM_RED] == level.teamScores[TEAM_BLUE])
			{
				cl->ps.persistant[PERS_RANK] = 2;
			}
			else if(level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE])
			{
				cl->ps.persistant[PERS_RANK] = 0;
			}
			else
			{
				cl->ps.persistant[PERS_RANK] = 1;
			}
		}
	}
	else
	{
		rank = -1;
		score = 0;
		for(i = 0; i < level.numPlayingClients; i++)
		{
			cl = &level.clients[level.sortedClients[i]];
			newScore = cl->ps.persistant[PERS_SCORE];
			if(i == 0 || newScore != score)
			{
				rank = i;
				// assume we aren't tied until the next client is checked
				level.clients[level.sortedClients[i]].ps.persistant[PERS_RANK] = rank;
			}
			else
			{
				// we are tied with the previous client
				level.clients[level.sortedClients[i - 1]].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
				level.clients[level.sortedClients[i]].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
			}
			score = newScore;
			if(g_gametype.integer == GT_SINGLE_PLAYER && level.numPlayingClients == 1)
			{
				level.clients[level.sortedClients[i]].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
			}
		}
	}

	// set the CS_SCORES1/2 configstrings, which will be visible to everyone
	if(g_gametype.integer >= GT_TEAM)
	{
		trap_SetConfigstring(CS_SCORES1, va("%i", level.teamScores[TEAM_RED]));
		trap_SetConfigstring(CS_SCORES2, va("%i", level.teamScores[TEAM_BLUE]));
	}
	else
	{
		if(level.numConnectedClients == 0)
		{
			trap_SetConfigstring(CS_SCORES1, va("%i", SCORE_NOT_PRESENT));
			trap_SetConfigstring(CS_SCORES2, va("%i", SCORE_NOT_PRESENT));
		}
		else if(level.numConnectedClients == 1)
		{
			trap_SetConfigstring(CS_SCORES1, va("%i", level.clients[level.sortedClients[0]].ps.persistant[PERS_SCORE]));
			trap_SetConfigstring(CS_SCORES2, va("%i", SCORE_NOT_PRESENT));
		}
		else
		{
			trap_SetConfigstring(CS_SCORES1, va("%i", level.clients[level.sortedClients[0]].ps.persistant[PERS_SCORE]));
			trap_SetConfigstring(CS_SCORES2, va("%i", level.clients[level.sortedClients[1]].ps.persistant[PERS_SCORE]));
		}
	}

	// see if it is time to end the level
	CheckExitRules();

	// if we are at the intermission, send the new info to everyone
	if(level.intermissiontime)
	{
		SendScoreboardMessageToAllClients();
	}
}


/*
========================================================================

MAP CHANGING

========================================================================
*/

/*
========================
SendScoreboardMessageToAllClients

Do this at BeginIntermission time and whenever ranks are recalculated
due to enters/exits/forced team changes
========================
*/
void SendScoreboardMessageToAllClients(void)
{
	int             i;

	for(i = 0; i < level.maxclients; i++)
	{
		if(level.clients[i].pers.connected == CON_CONNECTED)
		{
			DeathmatchScoreboardMessage(g_entities + i);
		}
	}
}

/*
========================
MoveClientToIntermission

When the intermission starts, this will be called for all players.
If a new client connects, this will be called after the spawn function.
========================
*/
void MoveClientToIntermission(gentity_t * ent)
{
	// take out of follow mode if needed
	if(ent->client->sess.spectatorState == SPECTATOR_FOLLOW)
	{
		StopFollowing(ent);
	}

	// move to the spot
	VectorCopy(level.intermission_origin, ent->s.origin);
	VectorCopy(level.intermission_origin, ent->client->ps.origin);
	VectorCopy(level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pm_type = PM_INTERMISSION;

	// clean up powerup info
	memset(ent->client->ps.powerups, 0, sizeof(ent->client->ps.powerups));

	ent->client->ps.eFlags = 0;
	ent->s.eFlags = 0;
	ent->s.eType = ET_GENERAL;
	ent->s.modelindex = 0;
	ent->s.loopSound = 0;
	ent->s.event = 0;
	ent->r.contents = 0;
}

/*
==================
FindIntermissionPoint

This is also used for spectator spawns
==================
*/
void FindIntermissionPoint(void)
{
	gentity_t      *ent, *target;
	vec3_t          dir;

	// find the intermission spot
	ent = G_Find(NULL, FOFS(classname), "info_player_intermission");
	if(!ent)
	{
		// the map creator forgot to put in an intermission point...
		SelectSpawnPoint(vec3_origin, level.intermission_origin, level.intermission_angle);
	}
	else
	{
		VectorCopy(ent->s.origin, level.intermission_origin);
		VectorCopy(ent->s.angles, level.intermission_angle);

		// if it has a target, look towards it
		if(ent->target)
		{
			target = G_PickTarget(ent->target);
			if(target)
			{
				VectorSubtract(target->s.origin, level.intermission_origin, dir);
				VectorToAngles(dir, level.intermission_angle);
			}
		}
	}
}

/*
==================
BeginIntermission
==================
*/
void BeginIntermission(void)
{
	int             i;
	gentity_t      *client;

	if(level.intermissiontime)
	{
		return;					// already active
	}

	// if in tournament mode, change the wins / losses
	if(g_gametype.integer == GT_TOURNAMENT)
	{
		AdjustTournamentScores();
	}

	level.intermissiontime = level.time;
	FindIntermissionPoint();

	// if single player game
	if(g_gametype.integer == GT_SINGLE_PLAYER)
	{
		UpdateTournamentInfo();
		//SpawnModelsOnVictoryPads();
	}

	// move all clients to the intermission point
	for(i = 0; i < level.maxclients; i++)
	{
		client = g_entities + i;
		if(!client->inuse)
			continue;

		// respawn if dead
		if(client->health <= 0)
		{
			respawn(client);
		}

		MoveClientToIntermission(client);
	}

	// send the current scoring to all clients
	SendScoreboardMessageToAllClients();

}


/*
=============
ExitLevel

When the intermission has been exited, the server is either killed
or moved to a new level based on the "nextmap" cvar

=============
*/
void ExitLevel(void)
{
	int             i;
	gclient_t      *cl;
	char            nextmap[MAX_STRING_CHARS];
	char            d1[MAX_STRING_CHARS];

#if defined(BRAINWORKS)
	//bot interbreeding
	BotInterbreedEndMatch();
#endif

	// if we are running a tournement map, kick the loser to spectator status,
	// which will automatically grab the next spectator and restart
	if(g_gametype.integer == GT_TOURNAMENT)
	{
		if(!level.restarted)
		{
			RemoveTournamentLoser();
			trap_SendConsoleCommand(EXEC_APPEND, "map_restart 0\n");
			level.restarted = qtrue;
			level.changemap = NULL;
			level.intermissiontime = 0;
		}
		return;
	}

	if(g_gametype.integer == GT_OBJECTIVE_SW)
	{
		trap_Cvar_Set("g_currentRound", va("%i", !g_currentRound.integer));
	}

	trap_Cvar_VariableStringBuffer("nextmap", nextmap, sizeof(nextmap));
	trap_Cvar_VariableStringBuffer("d1", d1, sizeof(d1));

	if(!Q_stricmp(nextmap, "map_restart 0") && Q_stricmp(d1, ""))
	{
		trap_Cvar_Set("nextmap", "vstr d2");
		trap_SendConsoleCommand(EXEC_APPEND, "vstr d1\n");
	}
	else
	{
		trap_SendConsoleCommand(EXEC_APPEND, "vstr nextmap\n");
	}

	level.changemap = NULL;
	level.intermissiontime = 0;

	// reset all the scores so we don't enter the intermission again
	level.teamScores[TEAM_RED] = 0;
	level.teamScores[TEAM_BLUE] = 0;
	for(i = 0; i < g_maxclients.integer; i++)
	{
		cl = level.clients + i;
		if(cl->pers.connected != CON_CONNECTED)
		{
			continue;
		}
		cl->ps.persistant[PERS_SCORE] = 0;
	}

	// we need to do this here before changing to CON_CONNECTING
	G_WriteSessionData();

	// change all client states to connecting, so the early players into the
	// next level will know the others aren't done reconnecting
	for(i = 0; i < g_maxclients.integer; i++)
	{
		if(level.clients[i].pers.connected == CON_CONNECTED)
		{
			level.clients[i].pers.connected = CON_CONNECTING;
		}
	}

}

/*
=================
G_LogPrintf

Print to the logfile with a time stamp if it is open
=================
*/
void QDECL G_LogPrintf(const char *fmt, ...)
{
	va_list         argptr;
	char            string[1024];
	int             min, tens, sec;

	sec = level.time / 1000;

	min = sec / 60;
	sec -= min * 60;
	tens = sec / 10;
	sec -= tens * 10;

	Com_sprintf(string, sizeof(string), "%3i:%i%i ", min, tens, sec);

	va_start(argptr, fmt);
	Q_vsnprintf(string + 7, sizeof(string) - 7, fmt, argptr);
	va_end(argptr);

	if(g_dedicated.integer)
	{
		G_Printf("%s", string + 7);
	}

	if(!level.logFile)
	{
		return;
	}

	trap_FS_Write(string, strlen(string), level.logFile);
}

void G_SWMapChange(int index)
{
	char            mapName[MAX_CVAR_VALUE_STRING];
	int             mapCount, mapNameLength;
	int             i;

	// Change map if g_swMaps has another map
	if(strlen(g_swMaps.string) > 0)
	{
		//G_Printf("SW Nextmap: START map: %d\n", index);
		mapCount = 0;
		mapNameLength = 0;
		for(i = 0; i < sizeof(g_swMaps.string); i++)
		{
			//G_Printf("SW Nextmap: %d %c\n", i, g_swMaps.string[i]);
			if(g_swMaps.string[i] == ',')
			{
				mapName[mapNameLength] = '\0';
				mapCount++;
				continue;
			}
			if(mapCount == index)
			{
				mapName[mapNameLength] = g_swMaps.string[i];
				mapNameLength++;
			}
			if(g_swMaps.string[i] == '\0')
			{
				mapName[mapNameLength] = '\0';
				break;
			}
			if(mapCount > g_swMap.integer + 1)
			{
				mapName[mapNameLength] = '\0';
				break;
			}
		}
		G_Printf("SW Nextmap: '%s'\n", mapName);
		if(strlen(mapName) > 0)
		{
			trap_Cvar_Set("nextmap", va("map %s", mapName));
		}
	}
}

/*
================
LogExit

Append information about this game to the log file
================
*/
void LogExit(const char *string)
{
	int             i, numSorted;
	gclient_t      *cl;
	char            cs[MAX_STRING_CHARS];
	int             winner, defender;

#if 0
	qboolean        won;
#endif

	G_LogPrintf("Exit: %s\n", string);

	level.intermissionQueued = level.time;

	// this will keep the clients from playing any voice sounds
	// that will get cut off when the queued intermission starts
	trap_SetConfigstring(CS_INTERMISSION, "1");

	// don't send more than 32 scores (FIXME?)
	numSorted = level.numConnectedClients;
	if(numSorted > 32)
	{
		numSorted = 32;
	}

	if(g_gametype.integer >= GT_TEAM)
	{
		G_LogPrintf("red:%i  blue:%i\n", level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE]);
	}

	if(g_gametype.integer == GT_OBJECTIVE_SW)
	{
		trap_GetConfigstring(CS_SWINFO, cs, sizeof(cs));
		defender = atoi(Info_ValueForKey(cs, "defender"));
		winner = atoi(Info_ValueForKey(cs, "winner"));

		if((winner == TEAM_FREE) && g_currentRound.integer && (g_nextTimeLimit.value > 0))
		{
			//Round 2
			//Time was set last round (g_nextTimeLimit > 0)
			//Time was not beaten this round (winner == TEAM_FREE)
			winner = defender;
			Info_SetValueForKey(cs, "winner", va("%i", defender));
			trap_SetConfigstring(CS_SWINFO, cs);
		}

		G_LogPrintf("sw-round:%i  defender:%i  winner:%i\n", g_currentRound.integer + 1, defender, winner);

		trap_SendServerCommand(-1,
							   va("print \"sw-round:%i  defender:%i  winner:%i  time:%f\n\"", g_currentRound.integer + 1,
								  defender, winner, (level.time - level.startTime) / 60000.f));

		if(!g_currentRound.integer)
		{
			//Stopwatch round 1
			if(winner == defender || winner == TEAM_FREE)
			{
				//Defenders held
				trap_Cvar_Set("g_nextTimeLimit", va("%f", 0.0f));
			}
			else
			{
				//Attackers won
				trap_Cvar_Set("g_nextTimeLimit", va("%f", (level.time - level.startTime) / 60000.f));
			}

			trap_Cvar_Set("nextmap", "map_restart 0");
		}
		else
		{
			//Stopwatch round 2

			//Adjust scores
			if(winner == TEAM_FREE)
			{
				trap_SendServerCommand(-1, va("print \"sw-round:%i  Round was a draw!\n\"", g_currentRound.integer + 1));
			}
			else
			{
				if((winner == TEAM_RED) ^ (g_swTeamToClan.integer))
				{
					trap_Cvar_Set("g_scoreA", va("%i", g_scoreA.integer + 1));
				}
				if((winner == TEAM_BLUE) ^ (g_swTeamToClan.integer))
				{
					trap_Cvar_Set("g_scoreB", va("%i", g_scoreB.integer + 1));
				}
			}

			G_SWMapChange(g_swMap.integer + 1);

			trap_Cvar_Set("g_nextTimeLimit", "0");
			trap_Cvar_Set("g_swMap", va("%i", g_swMap.integer + 1));
		}
	}

	for(i = 0; i < numSorted; i++)
	{
		int             ping;

		cl = &level.clients[level.sortedClients[i]];

		if(cl->sess.sessionTeam == TEAM_SPECTATOR)
		{
			continue;
		}
		if(cl->pers.connected == CON_CONNECTING)
		{
			continue;
		}

		ping = cl->ps.ping < 999 ? cl->ps.ping : 999;

		G_LogPrintf("score: %i  ping: %i  client: %i %s\n", cl->ps.persistant[PERS_SCORE], ping, level.sortedClients[i],
					cl->pers.netname);

#ifdef MISSIONPACK
		if(g_singlePlayer.integer && g_gametype.integer == GT_TOURNAMENT)
		{
			if(g_entities[cl - level.clients].r.svFlags & SVF_BOT && cl->ps.persistant[PERS_RANK] == 0)
			{
				won = qfalse;
			}
		}
#endif

#if 0
		// give everyone a UT 3 style level exit with an orbital camera
		won = qfalse;
		if(g_gametype.integer == GT_FFA || g_gametype.integer == GT_TOURNAMENT || g_gametype.integer == GT_SINGLE_PLAYER)
		{
			if(cl->ps.persistant[PERS_RANK] == 0)
			{
				won = qtrue;
			}
		}
		else if(g_gametype.integer >= GT_TEAM)
		{
			if(cl->sess.sessionTeam == TEAM_RED)
			{
				won = level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE];
			}
			else if(cl->sess.sessionTeam == TEAM_BLUE)
			{
				won = level.teamScores[TEAM_RED] < level.teamScores[TEAM_BLUE];
			}
		}

		if(!(g_entities[cl - level.clients].r.svFlags & SVF_BOT))
		{
			trap_SendServerCommand(&g_entities[cl - level.clients] - g_entities, va("%s", (won) ? "spWin\n" : "spLose\n"));
		}
#endif
	}
}


/*
=================
CheckIntermissionExit

The level will stay at the intermission for a minimum of 5 seconds
If all players wish to continue, the level will then exit.
If one or more players have not acknowledged the continue, the game will
wait 10 seconds before going on.
=================
*/
void CheckIntermissionExit(void)
{
	int             ready, notReady, playerCount;
	int             i;
	gclient_t      *cl;
	int             readyMask;

	if(g_gametype.integer == GT_SINGLE_PLAYER)
	{
		return;
	}

	// see which players are ready
	ready = 0;
	notReady = 0;
	readyMask = 0;
	playerCount = 0;
	for(i = 0; i < g_maxclients.integer; i++)
	{
		cl = level.clients + i;
		if(cl->pers.connected != CON_CONNECTED)
		{
			continue;
		}
		if(g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT)
		{
			continue;
		}

		playerCount++;
		if(cl->readyToExit)
		{
			ready++;
			if(i < 16)
			{
				readyMask |= 1 << i;
			}
		}
		else
		{
			notReady++;
		}
	}

	// copy the readyMask to each player's stats so
	// it can be displayed on the scoreboard
	for(i = 0; i < g_maxclients.integer; i++)
	{
		cl = level.clients + i;
		if(cl->pers.connected != CON_CONNECTED)
		{
			continue;
		}
		cl->ps.stats[STAT_CLIENTS_READY] = readyMask;
	}

	// never exit in less than five seconds
	if(level.time < level.intermissiontime + 5000)
	{
		return;
	}

	// only test ready status when there are real players present
	if(playerCount > 0)
	{
		// if nobody wants to go, clear timer
		if(!ready)
		{
			level.readyToExit = qfalse;
			return;
		}

		// if everyone wants to go, go now
		if(!notReady)
		{
			ExitLevel();
			return;
		}
	}

	// the first person to ready starts the ten second timeout
	if(!level.readyToExit)
	{
		level.readyToExit = qtrue;
		level.exitTime = level.time;
	}

	// if we have waited ten seconds since at least one player
	// wanted to exit, go ahead
	if(level.time < level.exitTime + 10000)
	{
		return;
	}

	ExitLevel();
}

/*
=============
ScoreIsTied
=============
*/
qboolean ScoreIsTied(void)
{
	int             a, b;

	if(level.numPlayingClients < 2)
	{
		return qfalse;
	}

	if(g_gametype.integer == GT_OBJECTIVE || g_gametype.integer == GT_OBJECTIVE_SW)
	{
		return qfalse;
	}

	if(g_gametype.integer >= GT_TEAM)
	{
		return level.teamScores[TEAM_RED] == level.teamScores[TEAM_BLUE];
	}

	a = level.clients[level.sortedClients[0]].ps.persistant[PERS_SCORE];
	b = level.clients[level.sortedClients[1]].ps.persistant[PERS_SCORE];

	return a == b;
}

/*
=================
CheckExitRules

There will be a delay between the time the exit is qualified for
and the time everyone is moved to the intermission spot, so you
can see the last frag.
=================
*/
void CheckExitRules(void)
{
	int             i;
	gclient_t      *cl;

	// if at the intermission, wait for all non-bots to
	// signal ready, then go to next level
	if(level.intermissiontime)
	{
		CheckIntermissionExit();
		return;
	}

	if(level.intermissionQueued)
	{
		int             time = (g_gametype.integer == GT_SINGLE_PLAYER) ? SP_INTERMISSION_DELAY_TIME : INTERMISSION_DELAY_TIME;

		if(level.time - level.intermissionQueued >= time)
		{
			level.intermissionQueued = 0;
			BeginIntermission();
		}
		return;
	}

	// check for sudden death
	if(ScoreIsTied())
	{
		// always wait for sudden death
		return;
	}

	if(g_timelimit.value && !level.warmupTime)
	{
		if((level.time - level.startTime) >= (g_timelimit.value * 60000.0f))
		{
			trap_SendServerCommand(-1, "print \"Timelimit hit.\n\"");
			LogExit("Timelimit hit.");
			return;
		}
	}

	if(level.numPlayingClients < 2)
	{
		return;
	}

	if(g_gametype.integer < GT_CTF && g_fraglimit.integer)
	{
		if(level.teamScores[TEAM_RED] >= g_fraglimit.integer)
		{
			trap_SendServerCommand(-1, "print \"Red hit the fraglimit.\n\"");
			LogExit("Fraglimit hit.");
			return;
		}

		if(level.teamScores[TEAM_BLUE] >= g_fraglimit.integer)
		{
			trap_SendServerCommand(-1, "print \"Blue hit the fraglimit.\n\"");
			LogExit("Fraglimit hit.");
			return;
		}

		for(i = 0; i < g_maxclients.integer; i++)
		{
			cl = level.clients + i;
			if(cl->pers.connected != CON_CONNECTED)
			{
				continue;
			}
			if(cl->sess.sessionTeam != TEAM_FREE)
			{
				continue;
			}

			if(cl->ps.persistant[PERS_SCORE] >= g_fraglimit.integer)
			{
				LogExit("Fraglimit hit.");
				trap_SendServerCommand(-1, va("print \"%s" S_COLOR_WHITE " hit the fraglimit.\n\"", cl->pers.netname));
				return;
			}
		}
	}

	if(g_gametype.integer >= GT_CTF && g_capturelimit.integer)
	{

		if(level.teamScores[TEAM_RED] >= g_capturelimit.integer)
		{
			trap_SendServerCommand(-1, "print \"Red hit the capturelimit.\n\"");
			LogExit("Capturelimit hit.");
			return;
		}

		if(level.teamScores[TEAM_BLUE] >= g_capturelimit.integer)
		{
			trap_SendServerCommand(-1, "print \"Blue hit the capturelimit.\n\"");
			LogExit("Capturelimit hit.");
			return;
		}
	}
}

qboolean GameIsInWarmup(void)
{
	char            buff[MAX_STRING_CHARS];

	trap_GetConfigstring(CS_WARMUP, buff, sizeof(buff));

	if(Q_stricmp(buff, "") == 0)
	{
		return qfalse;
	}
	else
	{
		return qtrue;
	}
}

/*
========================================================================

FUNCTIONS CALLED EVERY FRAME

========================================================================
*/


/*
=============
CheckTournament

Once a frame, check for changes in tournement player state
=============
*/
void CheckTournament(void)
{
	// check because we run 3 game frames before calling Connect and/or ClientBegin
	// for clients on a map_restart
	if(level.numPlayingClients == 0)
	{
		return;
	}

	if(g_gametype.integer == GT_TOURNAMENT)
	{

		// pull in a spectator if needed
		if(level.numPlayingClients < 2)
		{
			AddTournamentPlayer();
		}

		// if we don't have two players, go back to "waiting for players"
		if(level.numPlayingClients != 2)
		{
			if(level.warmupTime != -1)
			{
				level.warmupTime = -1;
				trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
				G_LogPrintf("Warmup:\n");
			}
			return;
		}

		if(level.warmupTime == 0)
		{
			return;
		}

		// if the warmup is changed at the console, restart it
		if(g_warmup.modificationCount != level.warmupModificationCount)
		{
			level.warmupModificationCount = g_warmup.modificationCount;
			level.warmupTime = -1;
		}

		// if all players have arrived, start the countdown
		if(level.warmupTime < 0)
		{
			if(level.numPlayingClients == 2)
			{
				// fudge by -1 to account for extra delays
				if(g_warmup.integer > 1)
				{
					level.warmupTime = level.time + (g_warmup.integer - 1) * 1000;
				}
				else
				{
					level.warmupTime = 0;
				}

				trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
			}
			return;
		}

		// if the warmup time has counted down, restart
		if(level.time > level.warmupTime)
		{
			level.warmupTime += 10000;
			trap_Cvar_Set("g_restarted", "1");
			trap_SendConsoleCommand(EXEC_APPEND, "map_restart 0\n");
			level.restarted = qtrue;
			return;
		}
	}
	else if(g_gametype.integer >= GT_TEAM && level.warmupTime != 0)
	{
		int             i;
		int             countRed = 0;
		int             readyCountRed = 0;
		int             countBlue = 0;
		int             readyCountBlue = 0;
		qboolean        notEnough = qfalse;

		for(i = 0; i < level.maxclients; i++)
		{
			if(level.clients[i].pers.connected == CON_DISCONNECTED)
			{
				continue;
			}
			if(level.clients[i].sess.sessionTeam == TEAM_RED)
			{
				countRed++;
				readyCountRed += (level.clients[i].pers.ready == qtrue);
			}
			else if(level.clients[i].sess.sessionTeam == TEAM_BLUE)
			{
				countBlue++;
				readyCountBlue += (level.clients[i].pers.ready == qtrue);
			}
		}

		if(g_readyPercent.integer > 0)
		{
			if(countRed == 0)
			{
				notEnough = qtrue;
			}
			else if((100 * readyCountRed / countRed) < g_readyPercent.integer)
			{
				notEnough = qtrue;
			}
			if(countBlue == 0)
			{
				notEnough = qtrue;
			}
			else if((100 * readyCountBlue / countBlue) < g_readyPercent.integer)
			{
				notEnough = qtrue;
			}
		}

		if(g_minPlayers.integer > 0)
		{
			if((countRed + countBlue) < g_minPlayers.integer)
			{
				notEnough = qtrue;
			}
		}

		if(notEnough && !level.warmupForcedStart)
		{
			if(level.warmupTime != -1)
			{
				level.warmupTime = -1;
				trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
				G_LogPrintf("Warmup:\n");
			}
			return;				// still waiting for team members
		}

		if(level.warmupTime == 0)
		{
			return;
		}

		// if the warmup is changed at the console, restart it
		if(g_warmup.modificationCount != level.warmupModificationCount)
		{
			level.warmupModificationCount = g_warmup.modificationCount;
			level.warmupTime = -1;
		}

		// if all players have arrived, start the countdown
		if(level.warmupTime < 0)
		{
			// fudge by -1 to account for extra delays
			level.warmupTime = level.time + (g_warmup.integer - 1) * 1000;
			trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
			return;
		}

		// if the warmup time has counted down, restart
		if(level.time > level.warmupTime)
		{
			level.warmupTime += 10000;
			trap_Cvar_Set("g_restarted", "1");
			trap_SendConsoleCommand(EXEC_APPEND, "map_restart 0\n");
			level.restarted = qtrue;
			level.warmupForcedStart = qfalse;
			return;
		}
	}
	else if(g_gametype.integer != GT_SINGLE_PLAYER && level.warmupTime != 0)
	{
		int             counts[TEAM_NUM_TEAMS];
		qboolean        notEnough = qfalse;

		if(g_gametype.integer > GT_TEAM)
		{
			counts[TEAM_BLUE] = TeamCount(-1, TEAM_BLUE);
			counts[TEAM_RED] = TeamCount(-1, TEAM_RED);

			if(counts[TEAM_RED] < 1 || counts[TEAM_BLUE] < 1)
			{
				notEnough = qtrue;
			}
		}
		else if(level.numPlayingClients < 2)
		{
			notEnough = qtrue;
		}

		if(notEnough)
		{
			if(level.warmupTime != -1)
			{
				level.warmupTime = -1;
				trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
				G_LogPrintf("Warmup:\n");
			}
			return;				// still waiting for team members
		}

		if(level.warmupTime == 0)
		{
			return;
		}

		// if the warmup is changed at the console, restart it
		if(g_warmup.modificationCount != level.warmupModificationCount)
		{
			level.warmupModificationCount = g_warmup.modificationCount;
			level.warmupTime = -1;
		}

		// if all players have arrived, start the countdown
		if(level.warmupTime < 0)
		{
			// fudge by -1 to account for extra delays
			level.warmupTime = level.time + (g_warmup.integer - 1) * 1000;
			trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
			return;
		}

		// if the warmup time has counted down, restart
		if(level.time > level.warmupTime)
		{
			level.warmupTime += 10000;
			trap_Cvar_Set("g_restarted", "1");
			trap_SendConsoleCommand(EXEC_APPEND, "map_restart 0\n");
			level.restarted = qtrue;
			return;
		}
	}
}


/*
==================
CheckVote
==================
*/
void CheckVote(void)
{
	if(level.voteExecuteTime && level.voteExecuteTime < level.time)
	{
		level.voteExecuteTime = 0;
		trap_SendConsoleCommand(EXEC_APPEND, va("%s\n", level.voteString));
	}
	if(!level.voteTime)
	{
		return;
	}
	if(level.time - level.voteTime >= VOTE_TIME)
	{
		trap_SendServerCommand(-1, "print \"Vote failed.\n\"");
	}
	else
	{
		// ATVI Q3 1.32 Patch #9, WNF
		if(level.voteYes > level.numVotingClients / 2)
		{
			// execute the command, then remove the vote
			trap_SendServerCommand(-1, "print \"Vote passed.\n\"");
			level.voteExecuteTime = level.time + 3000;
		}
		else if(level.voteNo >= level.numVotingClients / 2)
		{
			// same behavior as a timeout
			trap_SendServerCommand(-1, "print \"Vote failed.\n\"");
		}
		else
		{
			// still waiting for a majority
			return;
		}
	}
	level.voteTime = 0;
	trap_SetConfigstring(CS_VOTE_TIME, "");

}

/*
==================
PrintTeam
==================
*/
void PrintTeam(int team, char *message)
{
	int             i;

	for(i = 0; i < level.maxclients; i++)
	{
		if(level.clients[i].sess.sessionTeam != team)
			continue;
		trap_SendServerCommand(i, message);
	}
}

/*
==================
SetLeader
==================
*/
void SetLeader(int team, int client)
{
	int             i;

	if(level.clients[client].pers.connected == CON_DISCONNECTED)
	{
		PrintTeam(team, va("print \"%s is not connected\n\"", level.clients[client].pers.netname));
		return;
	}
	if(level.clients[client].sess.sessionTeam != team)
	{
		PrintTeam(team, va("print \"%s is not on the team anymore\n\"", level.clients[client].pers.netname));
		return;
	}
	for(i = 0; i < level.maxclients; i++)
	{
		if(level.clients[i].sess.sessionTeam != team)
			continue;
		if(level.clients[i].sess.teamLeader)
		{
			level.clients[i].sess.teamLeader = qfalse;
			ClientUserinfoChanged(i);
		}
	}
	level.clients[client].sess.teamLeader = qtrue;
	ClientUserinfoChanged(client);
	PrintTeam(team, va("print \"%s is the new team leader\n\"", level.clients[client].pers.netname));
}

/*
==================
CheckTeamLeader
==================
*/
void CheckTeamLeader(int team)
{
	int             i;

	for(i = 0; i < level.maxclients; i++)
	{
		if(level.clients[i].sess.sessionTeam != team)
			continue;
		if(level.clients[i].sess.teamLeader)
			break;
	}
	if(i >= level.maxclients)
	{
		for(i = 0; i < level.maxclients; i++)
		{
			if(level.clients[i].sess.sessionTeam != team)
				continue;
			if(!(g_entities[i].r.svFlags & SVF_BOT))
			{
				level.clients[i].sess.teamLeader = qtrue;
				break;
			}
		}
		for(i = 0; i < level.maxclients; i++)
		{
			if(level.clients[i].sess.sessionTeam != team)
				continue;
			level.clients[i].sess.teamLeader = qtrue;
			break;
		}
	}
}

/*
==================
CheckTeamVote
==================
*/
void CheckTeamVote(int team)
{
	int             cs_offset;

	if(team == TEAM_RED)
		cs_offset = 0;
	else if(team == TEAM_BLUE)
		cs_offset = 1;
	else
		return;

	if(!level.teamVoteTime[cs_offset])
	{
		return;
	}
	if(level.time - level.teamVoteTime[cs_offset] >= VOTE_TIME)
	{
		trap_SendServerCommand(-1, "print \"Team vote failed.\n\"");
	}
	else
	{
		if(level.teamVoteYes[cs_offset] > level.numteamVotingClients[cs_offset] / 2)
		{
			// execute the command, then remove the vote
			trap_SendServerCommand(-1, "print \"Team vote passed.\n\"");
			//
			if(!Q_strncmp("leader", level.teamVoteString[cs_offset], 6))
			{
				//set the team leader
				SetLeader(team, atoi(level.teamVoteString[cs_offset] + 7));
			}
			else
			{
				trap_SendConsoleCommand(EXEC_APPEND, va("%s\n", level.teamVoteString[cs_offset]));
			}
		}
		else if(level.teamVoteNo[cs_offset] >= level.numteamVotingClients[cs_offset] / 2)
		{
			// same behavior as a timeout
			trap_SendServerCommand(-1, "print \"Team vote failed.\n\"");
		}
		else
		{
			// still waiting for a majority
			return;
		}
	}
	level.teamVoteTime[cs_offset] = 0;
	trap_SetConfigstring(CS_TEAMVOTE_TIME + cs_offset, "");

}


/*
==================
CheckCvars
==================
*/
void CheckCvars(void)
{
	static int      lastMod = -1;

	if(g_password.modificationCount != lastMod)
	{
		lastMod = g_password.modificationCount;
		if(*g_password.string && Q_stricmp(g_password.string, "none"))
		{
			trap_Cvar_Set("g_needpass", "1");
		}
		else
		{
			trap_Cvar_Set("g_needpass", "0");
		}
	}
}

/*
=============
G_RunThink

Runs thinking code for this frame if necessary
=============
*/
void G_RunThink(gentity_t * ent)
{
	float           thinktime;

	thinktime = ent->nextthink;
	if(thinktime <= 0)
	{
		return;
	}
	if(thinktime > level.time)
	{
		return;
	}

	ent->nextthink = 0;
	if(!ent->think)
	{
		G_Error("NULL ent->think");
	}

#ifdef G_LUA
	// Lua API callbacks
	if(ent->luaThink && !ent->client)
	{
		G_LuaHook_EntityThink(ent->luaThink, ent->s.number);
	}
#endif

	ent->think(ent);
}

/*
================
G_RunFrame

Advances the non-player objects in the world
================
*/
void G_RunFrame(int levelTime)
{
	int             i;
	gentity_t      *ent;
	int             msec;
	int             start, end;
	int             spawnChange;

	//G_Printf("G_RunFrame()\n");

	// if we are waiting for the level to restart, do nothing
	if(level.restarted)
	{
		return;
	}

	level.framenum++;
	level.previousTime = level.time;
	level.time = levelTime;
	msec = level.time - level.previousTime;

	// get any cvar changes
	G_UpdateCvars();


	// spawns within G_RunClient(). 
	// Check if this frame involves spawning any players.
	if(g_teamSpawnWaves.integer >= 1)
	{
		if(level.time > (level.teamSpawnPreviousTimeRed + level.teamSpawnPeriodRed))
		{
			spawnChange = ((level.time - level.teamSpawnPreviousTimeRed) / level.teamSpawnPeriodRed) * level.teamSpawnPeriodRed;
			Com_Printf("Red Spawn! time=%d period=%d add=%d last=%d next=%d\n",
					   level.time, level.teamSpawnPeriodRed, spawnChange, level.teamSpawnPreviousTimeRed,
					   level.teamSpawnPreviousTimeRed + spawnChange);
			level.teamSpawnPreviousTimeRed += spawnChange;
			level.teamSpawningRed = 1;
		}
		else
		{
			level.teamSpawningRed = 0;
		}
		if(level.time > (level.teamSpawnPreviousTimeBlue + level.teamSpawnPeriodBlue))
		{
			spawnChange = ((level.time - level.teamSpawnPreviousTimeBlue) / level.teamSpawnPeriodBlue) * level.teamSpawnPeriodBlue;
			Com_Printf("Blue Spawn! time=%d period=%d add=%d last=%d next=%d\n", 
					   level.time, level.teamSpawnPeriodBlue, spawnChange, level.teamSpawnPreviousTimeRed,
					   level.teamSpawnPreviousTimeBlue + spawnChange);
			level.teamSpawnPreviousTimeBlue += spawnChange;
			level.teamSpawningBlue = 1;
		}
		else
		{
			level.teamSpawningBlue = 0;
		}
	}

	//
	// go through all allocated objects
	//
	if(g_debugEntities.integer)
	{
		Com_Printf("Entities {num, class, etype, eflags}: ");
	}
	start = trap_Milliseconds();
	ent = &g_entities[0];
	for(i = 0; i < level.numEntities; i++, ent++)
	{
		if(!ent->inuse)
		{
			continue;
		}

		if(g_debugEntities.integer)
		{
			Com_Printf("{%d, %s, %d, %d},", i, ent->classname, ent->s.eType, ent->s.eFlags);
		}

		// clear events that are too old
		if(level.time - ent->eventTime > EVENT_VALID_MSEC)
		{
			if(ent->s.event)
			{
				ent->s.event = 0;	// &= EV_EVENT_BITS;
				if(ent->client)
				{
					ent->client->ps.externalEvent = 0;
					// predicted events should never be set to zero
					//ent->client->ps.events[0] = 0;
					//ent->client->ps.events[1] = 0;
				}
			}
			if(ent->freeAfterEvent)
			{
				// tempEntities or dropped items completely go away after their event
				G_FreeEntity(ent);
				continue;
			}
			else if(ent->unlinkAfterEvent)
			{
				// items that will respawn will hide themselves after their pickup event
				ent->unlinkAfterEvent = qfalse;
				trap_UnlinkEntity(ent);
			}
		}

		// temporary entities don't think
		if(ent->freeAfterEvent)
		{
			continue;
		}

		if(!ent->r.linked && ent->neverFree)
		{
			continue;
		}

//unlagged - backward reconciliation #2
		// we'll run missiles separately to save CPU in backward reconciliation
/*
		if(ent->s.eType == ET_PROJECTILE || ent->s.eType == ET_PROJECTILE2)
		{
			G_RunMissile(ent);
			continue;
		}
*/
//unlagged - backward reconciliation #2

		if(ent->s.eType == ET_ITEM || ent->physicsObject)
		{
			G_RunItem(ent);
			continue;
		}

		if(ent->s.eType == ET_MOVER)
		{
			G_RunMover(ent);
			continue;
		}

		if(i < MAX_CLIENTS)
		{
			G_RunClient(ent);
			//WEAVER
			//reset the power count for this client for this frame
			ClientPowerHeldRelease(&g_clients[i]);
			continue;
		}

		//WEAVER
		//count power for each client
		if(ent->s.eType == ET_WEAVE_HELD)
		{
			ClientPowerHeldConsume(&g_clients[ent->s.otherEntityNum2], G_HeldWeave_GetPower(ent));
		}

		//WEAVER
		if(ent->s.eType == ET_WEAVE_MISSILE || ent->s.eType == ET_WEAVE_EFFECT)
		{
			G_RunWeaveEffect(ent);
		}

		//WEAVER
		if(ent->s.eType == ET_WEAVE_THREADS)
		{
			continue;
		}

		G_RunThink(ent);
	}

//unlagged - backward reconciliation #2
	// NOW run the missiles, with all players backward-reconciled
	// to the positions they were in exactly 50ms ago, at the end
	// of the last server frame
	G_TimeShiftAllClients(level.previousTime, NULL);

	ent = &g_entities[0];
	for(i = 0; i < level.numEntities; i++, ent++)
	{
		if(!ent->inuse)
		{
			continue;
		}

		// temporary entities don't think
		if(ent->freeAfterEvent)
		{
			continue;
		}

		if(ent->s.eType == ET_PROJECTILE || ent->s.eType == ET_PROJECTILE2)
		{
			G_RunMissile(ent);
		}
	}

	G_UnTimeShiftAllClients(NULL);
//unlagged - backward reconciliation #2

	//WEAVER do threads think after entities, 
	//since threads need heldWeaves to have throught first
	ThreadsThink();

	end = trap_Milliseconds();

	start = trap_Milliseconds();

	// perform final fixups on the players
	ent = &g_entities[0];
	for(i = 0; i < level.maxclients; i++, ent++)
	{
		if(ent->inuse)
		{
			ClientEndFrame(ent);
		}
	}
	end = trap_Milliseconds();

	// see if it is time to do a tournement restart
	CheckTournament();

	// see if it is time to end the level
	CheckExitRules();

	// update to team status?
	CheckTeamStatus();

	// cancel vote if timed out
	CheckVote();

	// check team votes
	CheckTeamVote(TEAM_RED);
	CheckTeamVote(TEAM_BLUE);

	// for tracking changes
	CheckCvars();

	if(g_listEntity.integer)
	{
		for(i = 0; i < MAX_GENTITIES; i++)
		{
			G_Printf("%4i: %s\n", i, g_entities[i].classname);
		}
		trap_Cvar_Set("g_listEntity", "0");
	}

	if(g_debugEntities.integer)
	{
		Com_Printf(" --end of Frame\n");
	}

#ifdef G_LUA
	G_LuaHook_RunFrame(levelTime);
#endif

//unlagged - backward reconciliation #4
	// record the time at the end of this frame - it should be about
	// the time the next frame begins - when the server starts
	// accepting commands from connected clients
	level.frameStartTime = trap_Milliseconds();
//unlagged - backward reconciliation #4
}
