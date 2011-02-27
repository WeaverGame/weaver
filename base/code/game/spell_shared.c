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

// spell_shared.c -- misc weaver functions, used by both game and cgame
#include "../../../code/qcommon/q_shared.h"
#include "../game/spell_common.h"

/*
=================
Weave Map

Tree structure

Node
* Array of pointers to node for each power
* Agressive weave ID of current combination
* Defensive weave ID of current combination
=================
*/

//WVP_NONE, WVP_AIR, WVP_AIRFIRE, WVP_FIRE, WVP_EARTHFIRE, WVP_EARTH, WVP_EARTHWATER, WVP_WATER, WVP_AIRWATER, WVP_SPIRIT

//Air 4
weaver_threadsMap_t wm_Air_Spirit_Airwater_Airfire = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_AIR_BINDPLAYER, WVW_NONE};
//Air 3
weaver_threadsMap_t wm_Air_Spirit_Air = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_AIR_GRABPLAYER, WVW_NONE};
weaver_threadsMap_t wm_Air_Spirit_Airwater = {{NULL, NULL, &wm_Air_Spirit_Airwater_Airfire, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_NONE, WVW_NONE};
//Air 2
//Collision WVW_D_AIR_GRAB, temporarily removed
weaver_threadsMap_t wm_Air_Air = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_AIR_BLAST, WVW_NONE};
weaver_threadsMap_t wm_Air_Spirit = {{NULL, &wm_Air_Spirit_Air, NULL, NULL, NULL, NULL, NULL, NULL, &wm_Air_Spirit_Airwater, NULL}, WVW_D_AIR_PROTECT, WVW_NONE}; //D
//Air 1
weaver_threadsMap_t wm_Air = {{NULL, &wm_Air_Air, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &wm_Air_Spirit}, WVW_NONE, WVW_NONE};

//Airfire 4
weaver_threadsMap_t wm_Airfire_Fire_Air_Airfire = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_AIRFIRE_LIGHTNING, WVW_NONE};
//Airfire 3
weaver_threadsMap_t wm_Airfire_Fire_Air = {{NULL, NULL, &wm_Airfire_Fire_Air_Airfire, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_NONE, WVW_NONE};
//Airfire 2
weaver_threadsMap_t wm_Airfire_Fire = {{NULL, &wm_Airfire_Fire_Air, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_AIRFIRE_SWORD, WVW_NONE};
weaver_threadsMap_t wm_Airfire_Spirit = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_D_AIRFIRE_LIGHT, WVW_NONE}; //D
//Airfire 1
weaver_threadsMap_t wm_Airfire = {{NULL, NULL, NULL, &wm_Airfire_Fire, NULL, NULL, NULL, NULL, NULL, &wm_Airfire_Spirit}, WVW_NONE, WVW_NONE};

//Fire 4
weaver_threadsMap_t wm_Fire_Earthfire_Airfire_Fire = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_FIRE_MULTIDARTS, WVW_NONE};
//Fire 3
weaver_threadsMap_t wm_Fire_Earthfire_Airfire = {{NULL, NULL, NULL, &wm_Fire_Earthfire_Airfire_Fire, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_FIRE_BALL, WVW_NONE};
//Fire 2
weaver_threadsMap_t wm_Fire_Earthfire = {{NULL, NULL, &wm_Fire_Earthfire_Airfire, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_FIRE_DARTS, WVW_NONE};
weaver_threadsMap_t wm_Fire_Spirit = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_D_FIRE_PROTECT, WVW_NONE}; //D
//Fire 1
weaver_threadsMap_t wm_Fire = {{NULL, NULL, NULL, NULL, &wm_Fire_Earthfire, NULL, NULL, NULL, NULL, &wm_Fire_Spirit}, WVW_NONE, WVW_NONE};

//Earthfire 3
weaver_threadsMap_t wm_Earthfire_Fire_Earthfire = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_D_EARTHFIRE_EXPLOSIVE_M, WVW_NONE}; //D
//Earthfire 2
weaver_threadsMap_t wm_Earthfire_Airfire = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_EARTHFIRE_IGNITE, WVW_NONE};
weaver_threadsMap_t wm_Earthfire_Fire = {{NULL, NULL, NULL, NULL, &wm_Earthfire_Fire_Earthfire, NULL, NULL, NULL, NULL, NULL}, WVW_D_EARTHFIRE_EXPLOSIVE_S, WVW_NONE}; //D
//Earthfire 1
weaver_threadsMap_t wm_Earthfire = {{NULL, NULL, &wm_Earthfire_Airfire, &wm_Earthfire_Fire, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_NONE, WVW_NONE};

//Earth 4
weaver_threadsMap_t wm_Earth_Fire_Earthfire_Earth = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_EARTH_QUAKE_L, WVW_NONE};
//Earth 3
weaver_threadsMap_t wm_Earth_Fire_Earthfire = {{NULL, NULL, NULL, NULL, NULL, &wm_Earth_Fire_Earthfire_Earth, NULL, NULL, NULL, NULL}, WVW_A_EARTH_QUAKE_M, WVW_NONE};
//Earth 2
weaver_threadsMap_t wm_Earth_Airfire = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_D_EARTH_UNLOCK, WVW_NONE}; //D
weaver_threadsMap_t wm_Earth_Fire = {{NULL, NULL, NULL, NULL, &wm_Earth_Fire_Earthfire, NULL, NULL, NULL, NULL, NULL}, WVW_A_EARTH_QUAKE_S, WVW_NONE};
weaver_threadsMap_t wm_Earth_Spirit = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_D_EARTH_PROTECT, WVW_NONE}; //D
//Earth 1
weaver_threadsMap_t wm_Earth = {{NULL, NULL, &wm_Earth_Airfire, &wm_Earth_Fire, NULL, NULL, NULL, NULL, NULL, &wm_Earth_Spirit}, WVW_NONE, WVW_NONE};

//Earthwater 3
weaver_threadsMap_t wm_Earthwater_Spirit_Water = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_EARTHWATER_POISON, WVW_NONE};
//Earthwater 2
weaver_threadsMap_t wm_Earthwater_Spirit = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, &wm_Earthwater_Spirit_Water, NULL, NULL}, WVW_A_EARTHWATER_SLOW, WVW_NONE};
//Earthwater 1
weaver_threadsMap_t wm_Earthwater = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &wm_Earthwater_Spirit}, WVW_NONE, WVW_NONE};

//Water 4
weaver_threadsMap_t wm_Water_Fire_Airwater_Water = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_WATER_ICESHARDS_M, WVW_NONE};
//Water 3
weaver_threadsMap_t wm_Water_Fire_Airwater = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, &wm_Water_Fire_Airwater_Water, NULL, NULL}, WVW_A_WATER_ICESHARDS_S, WVW_NONE};
weaver_threadsMap_t wm_Water_Spirit_Water = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_D_WATER_HEAL_M, WVW_NONE}; //D
//Water 2
weaver_threadsMap_t wm_Water_Fire = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &wm_Water_Fire_Airwater, NULL}, WVW_NONE, WVW_NONE};
weaver_threadsMap_t wm_Water_Airwater = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_D_WATER_HEAL_S, WVW_NONE}; //D
weaver_threadsMap_t wm_Water_Spirit = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, &wm_Water_Spirit_Water, NULL, NULL}, WVW_D_WATER_PROTECT, WVW_NONE}; //D
//Water 1
weaver_threadsMap_t wm_Water = {{NULL, NULL, NULL, &wm_Water_Fire, NULL, NULL, NULL, NULL, &wm_Water_Airwater, &wm_Water_Spirit}, WVW_NONE, WVW_NONE};

//Airwater 4
weaver_threadsMap_t wm_Airwater_Air_Spirit_Airwater = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_AIRWATER_RIP, WVW_NONE};
//Airwater 3
weaver_threadsMap_t wm_Airwater_Air_Spirit = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &wm_Airwater_Air_Spirit_Airwater, NULL}, WVW_NONE, WVW_NONE};
weaver_threadsMap_t wm_Airwater_Airfire_Water = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_D_AIRWATER_FOG, WVW_NONE}; //D
weaver_threadsMap_t wm_Airwater_Spirit_Airfire = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_AIRWATER_DARTS_M, WVW_NONE};
//Airwater 2
weaver_threadsMap_t wm_Airwater_Air = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &wm_Airwater_Air_Spirit}, WVW_NONE, WVW_NONE};
weaver_threadsMap_t wm_Airwater_Airfire = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, &wm_Airwater_Airfire_Water, NULL, NULL}, WVW_NONE, WVW_NONE};
weaver_threadsMap_t wm_Airwater_Spirit = {{NULL, NULL, &wm_Airwater_Spirit_Airfire, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_AIRWATER_DARTS_S, WVW_NONE};
//Airwater 1
weaver_threadsMap_t wm_Airwater = {{NULL, &wm_Airwater_Air, &wm_Airwater_Airfire, NULL, NULL, NULL, NULL, NULL, NULL, &wm_Airwater_Spirit}, WVW_NONE, WVW_NONE};

//Spirit 5
weaver_threadsMap_t wm_Spirit_Spirit_Air_Spirit_Airfire = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_NONE, WVW_NONE};
//Spirit 4
weaver_threadsMap_t wm_Spirit_Spirit_Air_Spirit = {{NULL, NULL, &wm_Spirit_Spirit_Air_Spirit_Airfire, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_NONE, WVW_NONE};
//Spirit 3
weaver_threadsMap_t wm_Spirit_Airfire_Earthwater = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_NONE, WVW_NONE};
weaver_threadsMap_t wm_Spirit_Spirit_Air = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &wm_Spirit_Spirit_Air_Spirit}, WVW_A_SPIRIT_SLICE_M, WVW_NONE};
weaver_threadsMap_t wm_Spirit_Spirit_Earth = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_SPIRIT_SHIELD, WVW_NONE};
//Spirit 2
weaver_threadsMap_t wm_Spirit_Air = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_SPIRIT_SLICE_S, WVW_NONE};
weaver_threadsMap_t wm_Spirit_Airfire = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_NONE, WVW_NONE};
weaver_threadsMap_t wm_Spirit_Water = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_D_SPIRIT_STAMINA, WVW_NONE}; //D
weaver_threadsMap_t wm_Spirit_Spirit = {{NULL, &wm_Spirit_Spirit_Air, NULL, NULL, NULL, &wm_Spirit_Spirit_Earth, NULL, NULL, NULL, NULL}, WVW_D_SPIRIT_LINK, WVW_NONE}; //D
//Spirit 1
weaver_threadsMap_t wm_Spirit = {{NULL, &wm_Spirit_Air, &wm_Spirit_Airfire, NULL, NULL, NULL, NULL, &wm_Spirit_Water, NULL, &wm_Spirit_Spirit}, WVW_NONE, WVW_NONE};

//Base 0
weaver_threadsMap_t wm = {{NULL, &wm_Air, &wm_Airfire, &wm_Fire, &wm_Earthfire, &wm_Earth, &wm_Earthwater, &wm_Water, &wm_Airwater, &wm_Spirit}, WVW_NONE, WVW_NONE};
weaver_threadsMap_t wm_NULL = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_NONE, WVW_NONE};

/*
=================
ThreadsTree

Weaves tree.
=================
*/
weaver_threadsMap_t *ThreadsTree(int threads[MAX_THREADS])
{
	weaver_threadsMap_t *current;
	int             i;

	current = &wm;

	// Progress currently selected weave down tree 
	// according to selected elements.
	for(i = 0; i < MAX_THREADS; i++)
	{
		// no element selected, cannot progress another level.
		if(threads[i] == WVP_NONE)
		{
			break;
		}
		// check if next selection is valid 
		if(current->next[threads[i]] == NULL)
		{
			// invalid, dead end
			current = &wm_NULL;
			break;
		}
		else
		{
			// valid, progress down tree
			current = current->next[threads[i]];
		}
	}

	return current;
}

/*
=================
ThreadsAgressive

Agressive weaves tree.
=================
*/
int ThreadsAgressive(int threads[MAX_THREADS])
{
	//Com_Printf("ThreadsAgressive (new) Weave=%d\n", ThreadsTree(threads)->weaveA);
	return ThreadsTree(threads)->weaveA;
}

/*
=================
ThreadsAgressive

Agressive weaves tree.
=================
*/
int ThreadsDefensive(int threads[MAX_THREADS])
{
	//Com_Printf("ThreadsDefensive (new) Weave=%d\n", ThreadsTree(threads)->weaveD);
	return ThreadsTree(threads)->weaveA; //D
}

/*
=================
ThreadsToWeaveID

Calls the appropriate thread tree.
=================
*/
int ThreadsToWeaveID(int group, int threads[MAX_THREADS])
{
	switch (group)
	{
		case WVG_AGRESSIVE:
			return ThreadsAgressive(threads);
		case WVG_DEFENSIVE:
			return ThreadsDefensive(threads);
		default:
			//invalid group WTF!
			return -1;
	}
}

/*
=================
WeaveProtectScales

Sets scale of protection for a weave's damage
=================
*/
void WeaveProtectScales(int weavenum, float *airprotect, float *fireprotect, float *earthprotect, float *waterprotect)
{
	*airprotect = 0.0f;
	*fireprotect = 0.0f;
	*earthprotect = 0.0f;
	*waterprotect = 0.0f;

	switch (weavenum)
	{
		case WVW_A_AIRFIRE_LIGHTNING:
		case WVW_A_AIRFIRE_SWORD:
			*airprotect = 1.4f;
			break;
		case WVW_D_AIR_PROTECT:
		case WVW_D_FIRE_PROTECT:
		case WVW_D_AIRFIRE_LIGHT:
		case WVW_D_AIRWATER_FOG:
		case WVW_D_SPIRIT_TRAVEL:
			break;
		case WVW_D_AIRFIRE_WALL:
			*fireprotect = 1.2f;
			break;
		case WVW_D_EARTHFIRE_EXPLOSIVE_S:
		case WVW_D_EARTHFIRE_EXPLOSIVE_M:
			*airprotect = 0.4f;
			*fireprotect = 0.8f;
			break;
		case WVW_D_SPIRIT_LINK:
			break;
		case WVW_A_FIRE_BLOSSOMS:
			*fireprotect = 1.1f;
			break;
		case WVW_A_SPIRIT_DEATHGATE:
			break;
		case WVW_D_EARTH_UNLOCK:
		case WVW_A_EARTHFIRE_IGNITE:
		case WVW_D_WATER_HEAL_S:
		case WVW_D_WATER_CURE:
		case WVW_D_WATER_HEAL_M:
		case WVW_D_SPIRIT_STAMINA:
		case WVW_D_AIRFIRE_INVIS:
		case WVW_A_SPIRIT_STILL:
			break;
		case WVW_D_AIR_GRAB:
		case WVW_A_AIR_GRABPLAYER:
		case WVW_A_AIR_BINDPLAYER:
		case WVW_A_SPIRIT_SHIELD:
		case WVW_A_SPIRIT_SLICE_S:
		case WVW_A_SPIRIT_SLICE_M:
		case WVW_A_SPIRIT_SLICE_L:
		case WVW_A_SPIRIT_BALEFIRE:
			break;
		case WVW_A_AIR_BLAST:
			*airprotect = 1.4f;
			break;
		case WVW_A_EARTH_QUAKE_S:
		case WVW_A_EARTH_QUAKE_M:
		case WVW_A_EARTH_QUAKE_L:
		case WVW_A_FIRE_BALL:
			*fireprotect = 1.2f;
			break;
		case WVW_A_FIRE_DARTS:
			*fireprotect = 1.2f;
			break;
		case WVW_A_FIRE_MULTIDARTS:
			*fireprotect = 1.2f;
			break;
		case WVW_A_AIRWATER_DARTS_S:
			*waterprotect = 1.0f;
			*airprotect = 0.5f;
			break;
		case WVW_A_AIRWATER_DARTS_M:
			*waterprotect = 1.0f;
			*airprotect = 0.5f;
			break;
		case WVW_A_EARTHWATER_SLOW:
			*waterprotect = 1.1f;
			break;
		case WVW_A_EARTHWATER_POISON:
			*waterprotect = 1.1f;
			break;
		case WVW_A_WATER_ICESHARDS_S:
			*waterprotect = 1.2f;
			*airprotect = 0.45f;
			*fireprotect = 0.18f;
			break;
		case WVW_A_WATER_ICESHARDS_M:
			*waterprotect = 1.2f;
			*airprotect = 0.45f;
			*fireprotect = 0.18f;
			break;
		case WVW_A_AIRWATER_RIP:
			*waterprotect = 1.2f;
			*airprotect = 0.6f;
			break;
			//Fail
		case WVW_NONE:
		case -1:
		default:
			break;
	}
}

#define WEAVEINFO(id,holdable,holdPowerCharge,holdMaxTime,castCharges,castDelay,primaryPower,effectType,tier,group,name,nameP) case (id): return (holdable);
qboolean WeaveHoldable(int weaveID)
{
	switch(weaveID)
	{
#include "spell_info.def"
		case WVW_NONE:
		case -1:
		default:
			return qfalse;
	}
}

#define WEAVEINFO(id,holdable,holdPowerCharge,holdMaxTime,castCharges,castDelay,primaryPower,effectType,tier,group,name,nameP) case (id): return (holdPowerCharge);
int WeaveHoldPower(int weaveID)
{
	switch(weaveID)
	{
#include "spell_info.def"
		case WVW_NONE:
		case -1:
		default:
			return 0;
	}
}

#define WEAVEINFO(id,holdable,holdPowerCharge,holdMaxTime,castCharges,castDelay,primaryPower,effectType,tier,group,name,nameP) case (id): return (tier);
weaver_tiers WeaveTier(int weaveID)
{
	switch(weaveID)
	{
#include "spell_info.def"
		case WVW_NONE:
		case -1:
		default:
			return WTIER_NONE;
	}
}

#define WEAVEINFO(id,holdable,holdPowerCharge,holdMaxTime,castCharges,castDelay,primaryPower,effectType,tier,group,name,nameP) case (id): return (primaryPower);
weaver_powers WeavePrimaryPower(int weaveID)
{
	switch(weaveID)
	{
#include "spell_info.def"
		case WVW_NONE:
		case -1:
		default:
			return WVP_NONE;
	}
}

#define WEAVEINFO(id,holdable,holdPowerCharge,holdMaxTime,castCharges,castDelay,primaryPower,effectType,tier,group,name,nameP) case (id): return (name);
char *WeaveName(int weaveID)
{
	switch(weaveID)
	{
#include "spell_info.def"
		case WVW_NONE:
		case -1:
		default:
			return "";
	}
}

#define WEAVEINFO(id,holdable,holdPowerCharge,holdMaxTime,castCharges,castDelay,primaryPower,effectType,tier,group,name,nameP) case (id): return (nameP);
char *WeaveNameP(int weaveID)
{
	switch(weaveID)
	{
#include "spell_info.def"
		case WVW_NONE:
		case -1:
		default:
			return "";
	}
}

#define WEAVEINFO(id,holdable,holdPowerCharge,holdMaxTime,castCharges,castDelay,primaryPower,effectType,tier,group,name,nameP) case (id): return (effectType);
int WeaveEffectType(int weaveID)
{
	switch(weaveID)
	{
#include "spell_info.def"
		case WVW_NONE:
		case -1:
		default:
			return 0;
	}
}

#define WEAVEINFO(id,holdable,holdPowerCharge,holdMaxTime,castCharges,castDelay,primaryPower,effectType,tier,group,name,nameP) case (id): return (holdMaxTime);
int WeaveHoldMaxTime(int weaveID)
{
	switch(weaveID)
	{
#include "spell_info.def"
		case WVW_NONE:
		case -1:
		default:
			return 0;
	}
}

#define WEAVEINFO(id,holdable,holdPowerCharge,holdMaxTime,castCharges,castDelay,primaryPower,effectType,tier,group,name,nameP) case (id): return (castDelay);
int WeaveCastDelay(int weaveID)
{
	switch(weaveID)
	{
#include "spell_info.def"
		case WVW_NONE:
		case -1:
		default:
			return 0;
	}
}

#define WEAVEINFO(id,holdable,holdPowerCharge,holdMaxTime,castCharges,castDelay,primaryPower,effectType,tier,group,name,nameP) case (id): return (group);
weaver_group WeaveADGroup(int weaveID)
{
	switch(weaveID)
	{
#include "spell_info.def"
		case WVW_NONE:
		case -1:
		default:
			return WVG_AGRESSIVE; // whatever
	}
}

#define WEAVEINFO(id,holdable,holdPowerCharge,holdMaxTime,castCharges,castDelay,primaryPower,effectType,tier,group,name,nameP) case (id): return (castCharges);
int WeaveCharges(int weaveID)
{
	switch(weaveID)
	{
#include "spell_info.def"
		case WVW_NONE:
		case -1:
		default:
			return 1;
	}
}

void LoadWeaveInfo(weaver_weaveInfo_t *info, int weaveID)
{
	info->holdable = WeaveHoldable(weaveID);
	info->holdPowerCharge = WeaveHoldPower(weaveID);
	info->holdMaxTime = WeaveHoldMaxTime(weaveID);
	info->castCharges = WeaveCharges(weaveID);
	info->castDelay = WeaveCastDelay(weaveID);
	info->primaryPower = WeavePrimaryPower(weaveID);
	info->effectType = WeaveEffectType(weaveID);
	info->tier = WeaveTier(weaveID);
	info->group = WeaveADGroup(weaveID);
	info->name = WeaveName(weaveID);
	info->nameP = WeaveNameP(weaveID);
}