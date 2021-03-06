/*
===========================================================================
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
// lua_game.c -- qagame library for Lua

#include "g_lua.h"

#if(defined(G_LUA))

static int game_Print(lua_State * L)
{
	int             i;
	char            buf[MAX_STRING_CHARS];
	int             n = lua_gettop(L);	// number of arguments

	memset(buf, 0, sizeof(buf));

	lua_getglobal(L, "tostring");

	DEBUG_LUA("game_Print: start: ");

	for(i = 1; i <= n; i++)
	{
		const char     *s;

		lua_pushvalue(L, -1);	// function to be called
		lua_pushvalue(L, i);	// value to print
		lua_call(L, 1, 1);
		s = lua_tostring(L, -1);	// get result

		if(s == NULL)
		{
			DEBUG_LUA("game_Print: return: no string");
			return luaL_error(L, "`tostring' must return a string to `print'");
		}

		Q_strcat(buf, sizeof(buf), s);

		lua_pop(L, 1);			// pop result
	}

	G_Printf("%s\n", buf);

	DEBUG_LUA("game_Print: return: printed string");
	return 0;
}

static int game_Broadcast(lua_State * L)
{
	int             i;
	char            buf[MAX_STRING_CHARS];
	int             n = lua_gettop(L);	// number of arguments

	memset(buf, 0, sizeof(buf));

	lua_getglobal(L, "tostring");

	DEBUG_LUA("game_Broadcast: start: ");

	for(i = 1; i <= n; i++)
	{
		const char     *s;

		lua_pushvalue(L, -1);	// function to be called
		lua_pushvalue(L, i);	// value to print
		lua_call(L, 1, 1);
		s = lua_tostring(L, -1);	// get result

		if(s == NULL)
		{
			DEBUG_LUA("game_Broadcast: return: no string");
			return luaL_error(L, "`tostring' must return a string to `print'");
		}

		Q_strcat(buf, sizeof(buf), s);

		lua_pop(L, 1);			// pop result
	}

	trap_SendServerCommand(-1, va("cp \"" S_COLOR_WHITE "%s\n\"", buf));

	DEBUG_LUA("game_Broadcast: return: broadcasted string");
	return 0;
}

// game.ObjectiveAnnounce(sucess_team, message)
static int game_ObjectiveAnnounce(lua_State * L)
{
	int             team;
	const char     *msg;

	team = luaL_checkint(L, 1);
	msg = luaL_checkstring(L, 2);

	DEBUG_LUA("game_ObjectiveAnnounce: start: team=%d msg=%s", team, msg);

	G_ObjectiveAnnounce(team, msg);

	DEBUG_LUA("game_ObjectiveAnnounce: return: sent");
	return 0;
}

// game.SpawnGroup(team, group, status)
static int game_SpawnGroup(lua_State * L)
{
	gentity_t      *next;
	int             team;
	int             spawnGroup;
	int             status;

	team = luaL_checkint(L, 1);
	spawnGroup = luaL_checkint(L, 2);
	status = luaL_checkint(L, 3);

	DEBUG_LUA("game_SpawnGroup: start: team=%d group=%d status=%d", team, spawnGroup, status);

	next = NULL;
	do
	{
		switch(team)
		{
			case TEAM_RED:
				next = G_Find(next, FOFS(classname), "team_ctf_redspawn");
				break;
			case TEAM_BLUE:
				next = G_Find(next, FOFS(classname), "team_ctf_bluespawn");
				break;
			default:
				DEBUG_LUA("game_SpawnGroup: return: warning - team should be red (1) or blue (2)");
				return 0;
		}
		if(!next)
		{
			DEBUG_LUA("game_SpawnGroup: return: next spawn not found");
			return 0;
		}
		if(next->group == spawnGroup)
		{
			switch(status)
			{
				case 0:
					trap_UnlinkEntity(next);
					break;
				case 1:
					trap_LinkEntity(next);
					break;
				default:
					DEBUG_LUA("game_SpawnGroup: return: warning - status should be disable (0) or enable (1)");
					return 0;
			}
		}
	} while(!Q_stricmp(next->classname, "team_ctf_redspawn") || !Q_stricmp(next->classname, "team_ctf_bluespawn"));

	DEBUG_LUA("game_SpawnGroup: return: no more spawns");
	return 0;
}

// game.EndRound()
static int game_EndRound(lua_State * L)
{
	DEBUG_LUA("game_EndRound: start: round ending");

	if(GameIsInWarmup())
	{
		DEBUG_LUA("game_EndRound: return: in warm up");
		return 0;
	}

	if(level.intermissionQueued)
	{
		DEBUG_LUA("game_EndRound: return: already queued");
		return 0;
	}

	trap_SendServerCommand(-1, "print \"Round Ended.\n\"");
	LogExit("Round Ended.");

	DEBUG_LUA("game_EndRound: return: exited");
	return 0;
}

// game.SetDefender(team)
static int game_SetDefender(lua_State * L)
{
	int             team;
	char            cs[MAX_STRING_CHARS];

	team = luaL_checkint(L, 1);

	DEBUG_LUA("game_SetDefender: start: defender=%d", team);

	if(team == TEAM_RED || team == TEAM_BLUE)
	{
		trap_GetConfigstring(CS_SWINFO, cs, sizeof(cs));
		Info_SetValueForKey(cs, "defender", va("%d", team));
		trap_SetConfigstring(CS_SWINFO, cs);
	}
	else
	{
		DEBUG_LUA("game_SetDefender: team invalid: ");
	}

	DEBUG_LUA("game_SetDefender: return: ");
	return 0;
}

// game.SetWinner(team)
static int game_SetWinner(lua_State * L)
{
	int             team;
	char            cs[MAX_STRING_CHARS];

	team = luaL_checkint(L, 1);

	DEBUG_LUA("game_SetWinner: start: winner=%d", team);

	if(team == TEAM_RED || team == TEAM_BLUE || team == TEAM_FREE)
	{
		trap_GetConfigstring(CS_SWINFO, cs, sizeof(cs));
		Info_SetValueForKey(cs, "winner", va("%d", team));
		trap_SetConfigstring(CS_SWINFO, cs);
	}
	else
	{
		DEBUG_LUA("game_SetWinner: team invalid: ");
	}

	DEBUG_LUA("game_SetWinner: return: ");
	return 0;
}

// game.SetTimeLimit(timelimit)
static int game_SetTimeLimit(lua_State * L)
{
	int             timelimit;

	timelimit = luaL_checkint(L, 1);

	DEBUG_LUA("game_SetTimeLimit: start: timelimit=%d min", timelimit);

	if(g_currentRound.integer == 1 && g_nextTimeLimit.value > 0)
	{
		DEBUG_LUA("game_SetTimeLimit: sw round 2: timelimit=%s min", va("%f", g_nextTimeLimit.value));
		trap_Cvar_Set("timelimit", va("%f", g_nextTimeLimit.value));
	}
	else
	{
		DEBUG_LUA("game_SetTimeLimit: sw round 1: timelimit=%d min", timelimit);
		trap_Cvar_Set("timelimit", va("%f", (float)timelimit));
	}

	DEBUG_LUA("game_SetTimeLimit: return: ");
	return 0;
}

// game.Leveltime()
static int game_Leveltime(lua_State * L)
{
	lua_pushinteger(L, level.time);

	DEBUG_LUA("game_Leveltime: start/return: leveltime=%d", level.time);
	return 1;
}

static const luaL_reg gamelib[] = {
	{"Print", game_Print},
	{"Broadcast", game_Broadcast},
	{"ObjectiveAnnounce", game_ObjectiveAnnounce},
	{"EndRound", game_EndRound},
	{"Leveltime", game_Leveltime},
	{"SetDefender", game_SetDefender},
	{"SetWinner", game_SetWinner},
	{"SetTimeLimit", game_SetTimeLimit},
	{"SpawnGroup", game_SpawnGroup},
	{NULL, NULL}
};

int luaopen_game(lua_State * L)
{
	luaL_register(L, "game", gamelib);

	lua_pushliteral(L, "_GAMEVERSION");
	lua_pushliteral(L, GAMEVERSION);

	return 1;
}

#endif
