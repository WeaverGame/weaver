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

// cg_spell_casts.c -- spell casting effect functions
#include "cg_local.h"

/*
===============
WeaveCast

generic sound (if available)
===============
*/
void WeaveCast(centity_t * cent)
{
	const weaver_weaveCGInfo *weave;

	weave = &cg_weaves[cent->currentState.eventParm];

	// add sound
	if(weave->firingSound)
	{
		trap_S_StartSound(cent->lerpOrigin, cent->currentState.number, CHAN_WEAPON, weave->firingSound);
	}
}

/*
===============
CG_WeaveCast
===============
*/
void CG_WeaveCast(centity_t * cent)
{
	//CG_RegisterWeave(cent->currentState.weapon);
	switch (cent->currentState.eventParm)
	{
			//Held special
		case WVW_A_AIRFIRE_LIGHTNING:
		case WVW_A_AIRFIRE_SWORD:
		case WVW_D_SPIRIT_TRAVEL:
		case WVW_D_AIRWATER_FOG:
		case WVW_D_AIRFIRE_LIGHT:
		case WVW_D_AIR_PROTECT:
		case WVW_D_FIRE_PROTECT:
		case WVW_D_EARTH_PROTECT:
		case WVW_D_WATER_PROTECT:
		case WVW_D_AIRFIRE_WALL:
		case WVW_D_EARTHFIRE_EXPLOSIVE_S:
		case WVW_D_EARTHFIRE_EXPLOSIVE_M:
		case WVW_D_SPIRIT_LINK:
		case WVW_A_EARTH_ROCKSTORM:
		case WVW_A_SPIRIT_DEATHGATE:
		case WVW_D_EARTH_UNLOCK:
		case WVW_A_EARTHFIRE_IGNITE:
		case WVW_D_WATER_HEAL_S:
		case WVW_D_WATER_CURE:
		case WVW_D_WATER_HEAL_M:
		case WVW_D_SPIRIT_STAMINA:
		case WVW_D_AIRFIRE_INVIS:
		case WVW_A_SPIRIT_STILL:
		case WVW_D_AIR_GRAB:
		case WVW_A_AIR_GRABPLAYER:
		case WVW_A_AIR_BINDPLAYER:
		case WVW_A_SPIRIT_SHIELD:
		case WVW_A_AIRWATER_RIP:
		case WVW_A_EARTH_QUAKE_S:
		case WVW_A_EARTH_QUAKE_M:
		case WVW_A_EARTH_QUAKE_L:
		case WVW_A_FIRE_DARTS:
		case WVW_A_FIRE_MULTIDARTS:
		case WVW_A_SPIRIT_SLICE_S:
		case WVW_A_SPIRIT_SLICE_M:
		case WVW_A_SPIRIT_SLICE_L:
		case WVW_A_SPIRIT_BALEFIRE:
		case WVW_A_AIR_BLAST:
		case WVW_A_FIRE_BALL:
		case WVW_A_AIRWATER_DARTS_S:
		case WVW_A_AIRWATER_DARTS_M:
		case WVW_A_EARTHWATER_SLOW:
		case WVW_A_EARTHWATER_POISON:
		case WVW_A_WATER_ICESHARDS_S:
		case WVW_A_WATER_ICESHARDS_M:
			WeaveCast(cent);
			break;
		case WVW_NONE:
		case -1:
		default:
			break;
	}
}
