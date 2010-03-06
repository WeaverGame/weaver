/*
===========================================================================
This file is part of the weaver game.

It has misc weaver functions, used by both game and cgame.
===========================================================================
*/
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
weaver_threadsMap_t wm_Air_Spirit_Airwater_Airfire = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_AIR_BINDPLAYER, -1};
//Air 3
weaver_threadsMap_t wm_Air_Spirit_Air = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_AIR_GRABPLAYER, -1};
weaver_threadsMap_t wm_Air_Spirit_Airwater = {{NULL, NULL, &wm_Air_Spirit_Airwater_Airfire, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, -1, -1};
//Air 2
//Collision WVW_D_AIR_GRAB, temporarily removed
weaver_threadsMap_t wm_Air_Air = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_AIR_BLAST, -1};
weaver_threadsMap_t wm_Air_Spirit = {{NULL, &wm_Air_Spirit_Air, NULL, NULL, NULL, NULL, NULL, NULL, &wm_Air_Spirit_Airwater, NULL}, WVW_D_AIR_PROTECT, -1}; //D
//Air 1
weaver_threadsMap_t wm_Air = {{NULL, &wm_Air_Air, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &wm_Air_Spirit}, -1, -1};

//Airfire 4
weaver_threadsMap_t wm_Airfire_Fire_Air_Airfire = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_AIRFIRE_LIGHTNING, -1};
//Airfire 3
weaver_threadsMap_t wm_Airfire_Fire_Air = {{NULL, NULL, &wm_Airfire_Fire_Air_Airfire, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, -1, -1};
//Airfire 2
weaver_threadsMap_t wm_Airfire_Fire = {{NULL, &wm_Airfire_Fire_Air, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_AIRFIRE_SWORD, -1};
weaver_threadsMap_t wm_Airfire_Spirit = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_D_AIRFIRE_LIGHT, -1}; //D
//Airfire 1
weaver_threadsMap_t wm_Airfire = {{NULL, NULL, NULL, &wm_Airfire_Fire, NULL, NULL, NULL, NULL, NULL, &wm_Airfire_Spirit}, -1, -1};

//Fire 4
weaver_threadsMap_t wm_Fire_Earthfire_Airfire_Fire = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_FIRE_MULTIDARTS, -1};
//Fire 3
weaver_threadsMap_t wm_Fire_Earthfire_Airfire = {{NULL, NULL, NULL, &wm_Fire_Earthfire_Airfire_Fire, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_FIRE_BALL, -1};
//Fire 2
weaver_threadsMap_t wm_Fire_Earthfire = {{NULL, NULL, &wm_Fire_Earthfire_Airfire, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_FIRE_DARTS, -1};
weaver_threadsMap_t wm_Fire_Spirit = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_D_FIRE_PROTECT, -1}; //D
//Fire 1
weaver_threadsMap_t wm_Fire = {{NULL, NULL, NULL, NULL, &wm_Fire_Earthfire, NULL, NULL, NULL, NULL, &wm_Fire_Spirit}, -1, -1};

//Earthfire 3
weaver_threadsMap_t wm_Earthfire_Fire_Earthfire = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_D_EARTHFIRE_EXPLOSIVE_M, -1}; //D
//Earthfire 2
weaver_threadsMap_t wm_Earthfire_Airfire = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_EARTHFIRE_IGNITE, -1};
weaver_threadsMap_t wm_Earthfire_Fire = {{NULL, NULL, NULL, NULL, &wm_Earthfire_Fire_Earthfire, NULL, NULL, NULL, NULL, NULL}, WVW_D_EARTHFIRE_EXPLOSIVE_S, -1}; //D
//Earthfire 1
weaver_threadsMap_t wm_Earthfire = {{NULL, NULL, &wm_Earthfire_Airfire, &wm_Earthfire_Fire, NULL, NULL, NULL, NULL, NULL, NULL}, -1, -1};

//Earth 4
weaver_threadsMap_t wm_Earth_Fire_Earthfire_Earth = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_EARTH_QUAKE_L, -1};
//Earth 3
weaver_threadsMap_t wm_Earth_Fire_Earthfire = {{NULL, NULL, NULL, NULL, NULL, &wm_Earth_Fire_Earthfire_Earth, NULL, NULL, NULL, NULL}, WVW_A_EARTH_QUAKE_M, -1};
//Earth 2
weaver_threadsMap_t wm_Earth_Airfire = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_D_EARTH_UNLOCK, -1}; //D
weaver_threadsMap_t wm_Earth_Fire = {{NULL, NULL, NULL, NULL, &wm_Earth_Fire_Earthfire, NULL, NULL, NULL, NULL, NULL}, WVW_A_EARTH_QUAKE_S, -1};
weaver_threadsMap_t wm_Earth_Spirit = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_D_EARTH_PROTECT, -1}; //D
//Earth 1
weaver_threadsMap_t wm_Earth = {{NULL, NULL, &wm_Earth_Airfire, &wm_Earth_Fire, NULL, NULL, NULL, NULL, NULL, &wm_Earth_Spirit}, -1, -1};

//Earthwater 3
weaver_threadsMap_t wm_Earthwater_Spirit_Water = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_EARTHWATER_POISON, -1};
//Earthwater 2
weaver_threadsMap_t wm_Earthwater_Spirit = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, &wm_Earthwater_Spirit_Water, NULL, NULL}, WVW_A_EARTHWATER_SLOW, -1};
//Earthwater 1
weaver_threadsMap_t wm_Earthwater = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &wm_Earthwater_Spirit}, -1, -1};

//Water 4
weaver_threadsMap_t wm_Water_Fire_Airwater_Water = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_WATER_ICESHARDS_M, -1};
//Water 3
weaver_threadsMap_t wm_Water_Fire_Airwater = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, &wm_Water_Fire_Airwater_Water, NULL, NULL}, WVW_A_WATER_ICESHARDS_S, -1};
weaver_threadsMap_t wm_Water_Spirit_Water = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_D_WATER_HEAL_M, -1}; //D
//Water 2
weaver_threadsMap_t wm_Water_Fire = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &wm_Water_Fire_Airwater, NULL}, -1, -1};
weaver_threadsMap_t wm_Water_Airwater = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_D_WATER_HEAL_S, -1}; //D
weaver_threadsMap_t wm_Water_Spirit = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, &wm_Water_Spirit_Water, NULL, NULL}, WVW_D_WATER_PROTECT, -1}; //D
//Water 1
weaver_threadsMap_t wm_Water = {{NULL, NULL, NULL, &wm_Water_Fire, NULL, NULL, NULL, NULL, &wm_Water_Airwater, &wm_Water_Spirit}, -1, -1};

//Airwater 4
weaver_threadsMap_t wm_Airwater_Air_Spirit_Airwater = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_AIRWATER_RIP, -1};
//Airwater 3
weaver_threadsMap_t wm_Airwater_Air_Spirit = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &wm_Airwater_Air_Spirit_Airwater, NULL}, -1, -1};
weaver_threadsMap_t wm_Airwater_Airfire_Water = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_D_AIRWATER_FOG, -1}; //D
weaver_threadsMap_t wm_Airwater_Spirit_Airfire = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_AIRWATER_DARTS_M, -1};
//Airwater 2
weaver_threadsMap_t wm_Airwater_Air = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &wm_Airwater_Air_Spirit}, -1, -1};
weaver_threadsMap_t wm_Airwater_Airfire = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, &wm_Airwater_Airfire_Water, NULL, NULL}, -1, -1};
weaver_threadsMap_t wm_Airwater_Spirit = {{NULL, NULL, &wm_Airwater_Spirit_Airfire, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_AIRWATER_DARTS_S, -1};
//Airwater 1
weaver_threadsMap_t wm_Airwater = {{NULL, &wm_Airwater_Air, &wm_Airwater_Airfire, NULL, NULL, NULL, NULL, NULL, NULL, &wm_Airwater_Spirit}, -1, -1};

//Spirit 5
weaver_threadsMap_t wm_Spirit_Spirit_Air_Spirit_Airfire = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, -1, -1};
//Spirit 4
weaver_threadsMap_t wm_Spirit_Spirit_Air_Spirit = {{NULL, NULL, &wm_Spirit_Spirit_Air_Spirit_Airfire, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, -1, -1};
//Spirit 3
weaver_threadsMap_t wm_Spirit_Airfire_Earthwater = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, -1, -1};
weaver_threadsMap_t wm_Spirit_Spirit_Air = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &wm_Spirit_Spirit_Air_Spirit}, WVW_A_SPIRIT_SLICE_M, -1};
weaver_threadsMap_t wm_Spirit_Spirit_Earth = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_SPIRIT_SHIELD, -1};
//Spirit 2
weaver_threadsMap_t wm_Spirit_Air = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_A_SPIRIT_SLICE_S, -1};
weaver_threadsMap_t wm_Spirit_Airfire = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, -1, -1};
weaver_threadsMap_t wm_Spirit_Water = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, WVW_D_SPIRIT_STAMINA, -1}; //D
weaver_threadsMap_t wm_Spirit_Spirit = {{NULL, &wm_Spirit_Spirit_Air, NULL, NULL, NULL, &wm_Spirit_Spirit_Earth, NULL, NULL, NULL, NULL}, WVW_D_SPIRIT_LINK, -1}; //D
//Spirit 1
weaver_threadsMap_t wm_Spirit = {{NULL, &wm_Spirit_Air, &wm_Spirit_Airfire, NULL, NULL, NULL, NULL, &wm_Spirit_Water, NULL, &wm_Spirit_Spirit}, -1, -1};

//Base 0
weaver_threadsMap_t wm = {{NULL, &wm_Air, &wm_Airfire, &wm_Fire, &wm_Earthfire, &wm_Earth, &wm_Earthwater, &wm_Water, &wm_Airwater, &wm_Spirit}, -1, -1};
weaver_threadsMap_t wm_NULL = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}, -1, -1};

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
WeavePowerName

Returns string name of a powerID
=================
*/
char           *WeavePowerName(int index)
{
	char           *output;

	switch (index)
	{
		case WVP_NONE:
			output = "none";
			break;
		case WVP_AIR:
			output = "air";
			break;
		case WVP_AIRFIRE:
			output = "airfire";
			break;
		case WVP_FIRE:
			output = "fire";
			break;
		case WVP_EARTHFIRE:
			output = "earthfire";
			break;
		case WVP_EARTH:
			output = "earth";
			break;
		case WVP_EARTHWATER:
			output = "earthwater";
			break;
		case WVP_WATER:
			output = "water";
			break;
		case WVP_AIRWATER:
			output = "airwater";
			break;
		case WVP_SPIRIT:
			output = "spirit";
			break;
		default:
			output = "none";
			break;
	}

	return output;
}

/*
=================
WeaveGroupName

Returns name of a groupID
=================
*/
char           *WeaveGroupName(int index)
{
	char           *output;

	switch (index)
	{
		case WVG_AGRESSIVE:
			output = "A";
			break;
		case WVG_DEFENSIVE:
			output = "D";
			break;
		default:
			output = "error";
			break;
	}
	return output;
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
			*airprotect = 1.0f;
			break;
		case WVW_D_AIR_PROTECT:
		case WVW_D_FIRE_PROTECT:
		case WVW_D_AIRFIRE_LIGHT:
		case WVW_D_AIRWATER_FOG:
		case WVW_D_SPIRIT_TRAVEL:
			break;
		case WVW_D_AIRFIRE_WALL:
			*fireprotect = 0.9f;
			break;
		case WVW_D_EARTHFIRE_EXPLOSIVE_S:
		case WVW_D_EARTHFIRE_EXPLOSIVE_M:
			*airprotect = 0.3f;
			*fireprotect = 0.6f;
			break;
		case WVW_D_SPIRIT_LINK:
			break;
		case WVW_A_FIRE_BLOSSOMS:
			*fireprotect = 0.8f;
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
			*airprotect = 1.0f;
			break;
		case WVW_A_EARTH_QUAKE_S:
		case WVW_A_EARTH_QUAKE_M:
		case WVW_A_EARTH_QUAKE_L:
		case WVW_A_FIRE_BALL:
			*fireprotect = 0.9f;
			break;
		case WVW_A_FIRE_DARTS:
			*fireprotect = 0.9f;
			break;
		case WVW_A_FIRE_MULTIDARTS:
			*fireprotect = 0.9f;
			break;
		case WVW_A_AIRWATER_DARTS_S:
			*waterprotect = 0.8f;
			*airprotect = 0.4f;
			break;
		case WVW_A_AIRWATER_DARTS_M:
			*waterprotect = 0.8f;
			*airprotect = 0.4f;
			break;
		case WVW_A_EARTHWATER_SLOW:
			*waterprotect = 0.9f;
			break;
		case WVW_A_EARTHWATER_POISON:
			*waterprotect = 0.9f;
			break;
		case WVW_A_WATER_ICESHARDS_S:
			*waterprotect = 0.9f;
			*airprotect = 0.35f;
			*fireprotect = 0.1f;
			break;
		case WVW_A_WATER_ICESHARDS_M:
			*waterprotect = 0.9f;
			*airprotect = 0.35f;
			*fireprotect = 0.1f;
			break;
		case WVW_A_AIRWATER_RIP:
			*waterprotect = 0.5f;
			*airprotect = 0.5f;
			break;
			//Fail
		case WVW_NONE:
		case -1:
		default:
			break;
	}
}

/*
=================
WeaveTier

Returns the tier of a weave.
=================
*/
int WeaveTier(int weaveID)
{
	switch (weaveID)
	{
		case WVW_A_AIRFIRE_SWORD:
		case WVW_D_AIRFIRE_LIGHT:
		case WVW_D_AIR_PROTECT:
		case WVW_D_FIRE_PROTECT:
		case WVW_D_SPIRIT_LINK:
		case WVW_D_EARTH_UNLOCK:
		case WVW_A_EARTHFIRE_IGNITE:
		case WVW_D_EARTHFIRE_EXPLOSIVE_S:
		case WVW_A_SPIRIT_SLICE_S:
		case WVW_D_WATER_HEAL_S:
		case WVW_D_SPIRIT_STAMINA:
		case WVW_D_AIR_GRAB:
		case WVW_A_AIR_BLAST:
		case WVW_A_EARTH_QUAKE_S:
		case WVW_A_FIRE_DARTS:
		case WVW_A_AIRWATER_DARTS_S:
		case WVW_A_EARTHWATER_SLOW:
			return 1;
		case WVW_D_AIRWATER_FOG:
		case WVW_D_AIRFIRE_WALL:
		case WVW_D_EARTHFIRE_EXPLOSIVE_M:
		case WVW_A_SPIRIT_SLICE_M:
		case WVW_D_WATER_HEAL_M:
		case WVW_D_WATER_CURE:
		case WVW_A_AIR_GRABPLAYER:
		case WVW_A_EARTH_QUAKE_M:
		case WVW_A_FIRE_BALL:
		case WVW_A_AIRWATER_DARTS_M:
		case WVW_A_EARTHWATER_POISON:
		case WVW_A_SPIRIT_SHIELD:
		case WVW_A_WATER_ICESHARDS_S:
			return 2;
		case WVW_D_AIRFIRE_INVIS:
		case WVW_A_AIR_BINDPLAYER:
		case WVW_A_EARTH_QUAKE_L:
		case WVW_A_FIRE_MULTIDARTS:
		case WVW_A_WATER_ICESHARDS_M:
		case WVW_A_AIRWATER_RIP:
			return 3;
		case WVW_A_SPIRIT_SLICE_L:
		case WVW_A_AIRFIRE_LIGHTNING:
			return 4;
		case WVW_D_SPIRIT_TRAVEL:
		case WVW_A_FIRE_BLOSSOMS:
			return 5;
		case WVW_A_SPIRIT_STILL:
		case WVW_A_SPIRIT_BALEFIRE:
		case WVW_A_SPIRIT_DEATHGATE:
			return 6;
			//anything else is an error.
		case WVW_NONE:
		case -1:
		default:
			return 0;
	}
}

/*
=================
WeaveCharges

Returns the maximum number of charges for a given type of weave.
=================
*/
int WeaveCharges(int weaveID)
{
	switch (weaveID)
	{
		case WVW_A_FIRE_MULTIDARTS:
			return 20;
		case WVW_A_FIRE_DARTS:
			return 16;
		case WVW_A_AIRWATER_DARTS_S:
		case WVW_A_AIRWATER_DARTS_M:
		case WVW_A_WATER_ICESHARDS_S:
		case WVW_A_WATER_ICESHARDS_M:
			return 3;
		case WVW_A_FIRE_BALL:
		case WVW_A_AIR_BLAST:
		case WVW_A_SPIRIT_SLICE_S:
		case WVW_A_SPIRIT_SLICE_M:
		case WVW_A_SPIRIT_SLICE_L:
			return 2;
		default:
			return 1;
	}
}
