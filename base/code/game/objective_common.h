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

// objective_common.h -- contains global objective definitions
#ifndef OBJECTIVE_COMMON_H
#define OBJECTIVE_COMMON_H

#include "../../../code/qcommon/q_shared.h"

// Used as event param for EV_GLOBAL_OBJ events
typedef enum
{
	OBJEV_NONE = 0,

	OBJEV_FLAG_RED_TAKEN_FREE,		// Red claimed a free spawnflag "Spawnpoint claimed!" "They have claimed a spawnpoint!"
	OBJEV_FLAG_BLUE_TAKEN_FREE,
	OBJEV_FLAG_RED_TAKEN_BLUE,		// Red claimed blue's spawnflag "Spawnpoint taken!" "Spawnpoint lost!"
	OBJEV_FLAG_BLUE_TAKEN_RED,
	OBJEV_FLAG_RED_SECURED,			// Red has permently secured a spawnflag "Spawnpoint secured!" "They have secured a spawnpoint!"
	OBJEV_FLAG_BLUE_SECURED,

	OBJEV_EXPLOSIVE_RED_VULN,		// Red can attack explosive. "The barricade is vulnerable!" "Our barricade is vulnerable!"
	OBJEV_EXPLOSIVE_BLUE_VULN,
	OBJEV_EXPLOSIVE_RED_WEAK,		// Red damaged explosive. "Barricade is damaged!" "Our barricade is damaged!"
	OBJEV_EXPLOSIVE_BLUE_WEAK,
	OBJEV_EXPLOSIVE_RED_KILL,		// Red destroyed explosive. "Barricade destrored!" "Barricade destroyed!"
	OBJEV_EXPLOSIVE_BLUE_KILL,

	OBJEV_SHIELD_RED_WEAK,			// Red weakened a shield. "Shield is weak." "Our ward is weakening."
	OBJEV_SHIELD_BLUE_WEAK,
	OBJEV_SHIELD_RED_KILL,			// Red killed a shield. "Shield dispelled!" "They have dispelled our ward."
	OBJEV_SHIELD_BLUE_KILL,

	OBJEV_ITEM_RED_STOLEN,			// Red team stole the item. "Objective taken!" "They have taken the objective!"
	OBJEV_ITEM_BLUE_STOLEN,
	OBJEV_ITEM_RED_DROPPED,			// Red team dropped the item. "Objective dropped!" "They have dropped the objective!"
	OBJEV_ITEM_BLUE_DROPPED,
	OBJEV_ITEM_TIME_RETURNED,		// Item was left on the ground for to long so it was returned. "Objective Returned!"
	OBJEV_ITEM_DROP_RETURNED,		// Item fell onto a nodrop area so it was returned.
	OBJEV_ITEM_RED_RETURNED,		// Item was stolen by blue, returned by red player. "Objective Returned!" "They have returned the objective!"
	OBJEV_ITEM_BLUE_RETURNED,
	OBJEV_ITEM_RED_CAPTURED,		// Red team captured the item. "Objective captured!" "Objective lost!"
	OBJEV_ITEM_BLUE_CAPTURED,

	OBJEV_TIMELIMIT_30SEC,			// 30 Sec left "30 seconds - now or never!" "30 seconds - hold them off!"
	OBJEV_TIMELIMIT_2MIN,			// 2 Min left "2 minutes remaining"
	OBJEV_TIMELIMIT_5MIN,			// 5 Min left "5 minutes remaining"

	OBJEV_ROUND_RED_ADVANCE,		// Red team (attacking) reach next stage. "Push forward!" "Fall back!"
	OBJEV_ROUND_BLUE_ADVANCE,

	// End of round 1
	OBJEV_ROUND_RED_SET_TIME,		// Red team (attacking) set a time. "Attack successfull!" "Defence failed!"
	OBJEV_ROUND_BLUE_SET_TIME,
	OBJEV_ROUND_RED_HELD,			// Red team (defending) full held. "Defence successfull!" "Attack failed!"
	OBJEV_ROUND_BLUE_HELD,
	// End of round 2
	OBJEV_ROUND_RED_DRAW,			// Game Drawn (double full hold) "Game drawn."
	OBJEV_ROUND_RED_WIN,			// Red won "Victory!" "Defeat!"
	OBJEV_ROUND_BLUE_WIN
} objective_events;

#endif							// OBJECTIVE_COMMON_H
