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

// spell_common.h -- contains global weaver definitions
#ifndef SPELL_COMMON_H
#define SPELL_COMMON_H

#include <q_shared.h>

//Game
#define MAX_THREADS         8
#define HELD_MAX            4

#define MIN_WEAPON_WEAVE    (MAX_WEAPONS - HELD_MAX)
#define MAX_WEAPON_WEAVE    (MAX_WEAPONS - 1)

#define FULL_POWER          1000.0
#define POWER_PER_THREAD    175

//Entities
#define HELD_CLASSNAME      "weaveheld"
#define EFFECT_CLASSNAME    "weave_effect"
#define LINK_CLASSNAME      "weave_link"
#define THREAD_CLASSNAME    "threads"

//Weaver disc
#define WEAVER_CENTER       25
#define WEAVER_POWERDIV     360/8
#define NUM_WEAVERDISCS     3

//HUD
#define HUD_HEALTH_HEIGHT   90
#define HUD_HEALTH_WIDTH    12
#define HUD_PROTECT_HEIGHT  90
#define HUD_PROTECT_WIDTH   9
#define HUD_POWER_HEIGHT    90
#define HUD_POWER_WIDTH     12
#define HUD_STATUS_HEIGHT   32
#define HUD_STATUS_WIDTH    32

//Times
#define TIME_THREE_SEC      3000
#define TIME_TWENTY_SEC     20000
#define TIME_THIRTY_SEC     30000
#define TIME_ONE_MIN        60000
#define TIME_TWO_MIN        120000
#define TIME_PROJECTILE_EXPIRE 15000

//Knockback
#define KNOCKBACK_PLAYER_MASS 200
#define KNOCKBACK_TIME_MIN 50
#define KNOCKBACK_TIME_MAX 150

//Weaves
#define WEAVE_AIRBLAST_START 50
#define WEAVE_AIRBLAST_TIME 750
#define WEAVE_AIRBLAST_DAMAGE 10
#define WEAVE_AIRBLAST_SPLASH 50
#define WEAVE_AIRBLAST_SPLASH_DAMAGE 10
#define WEAVE_AIRBLAST_SPEED 850
#define WEAVE_AIRBLAST_KNOCK 320.0f

#define WEAVE_FIREDARTS_START 50
#define WEAVE_FIREDARTS_DAMAGE 7
#define WEAVE_FIREDARTS_SPREAD 50
#define WEAVE_FIREDARTS_SPEED 2600

#define WEAVE_FIREBALL_START 50
#define WEAVE_FIREBALL_DAMAGE 70
#define WEAVE_FIREBALL_SPLASH 200
#define WEAVE_FIREBALL_SPLASH_DAMAGE 60
#define WEAVE_FIREBALL_SPEED 1300

#define WEAVE_EARTHQUAKE_START 50
#define WEAVE_EARTHQUAKE_GRAVITY 700.0f

#define WEAVE_EARTHQUAKES_PROJDAMAGE 50
#define WEAVE_EARTHQUAKES_PROJSPLASH 50
#define WEAVE_EARTHQUAKES_PROJSPLASH_DAMAGE 50
#define WEAVE_EARTHQUAKES_PROJSPEED 700
#define WEAVE_EARTHQUAKES_DPS 1
#define WEAVE_EARTHQUAKES_RADIUS 250
#define WEAVE_EARTHQUAKES_DURATION 7500

#define WEAVE_EARTHQUAKEM_PROJDAMAGE 80
#define WEAVE_EARTHQUAKEM_PROJSPLASH 80
#define WEAVE_EARTHQUAKEM_PROJSPLASH_DAMAGE 80
#define WEAVE_EARTHQUAKEM_PROJSPEED 800
#define WEAVE_EARTHQUAKEM_DPS 2
#define WEAVE_EARTHQUAKEM_RADIUS 300
#define WEAVE_EARTHQUAKEM_DURATION 9000

#define WEAVE_EARTHQUAKEL_PROJDAMAGE 100
#define WEAVE_EARTHQUAKEL_PROJSPLASH 100
#define WEAVE_EARTHQUAKEL_PROJSPLASH_DAMAGE 100
#define WEAVE_EARTHQUAKEL_PROJSPEED 900
#define WEAVE_EARTHQUAKEL_DPS 4
#define WEAVE_EARTHQUAKEL_RADIUS 350
#define WEAVE_EARTHQUAKEL_DURATION 12000

#define WEAVE_SLICE_START 50
#define WEAVE_SLICE_SPEED 950
#define WEAVE_SLICE_SPREAD 86

#define WEAVE_SLICES_TIER 2
#define WEAVE_SLICEM_TIER 4
#define WEAVE_SLICEL_TIER 6

#define WEAVE_SHIELD_TIME 30000
#define WEAVE_SHIELD_CAST_RANGE 8192
#define WEAVE_SHIELD_PULSE_TIME 150

#define WEAVE_HEAL_CAST_RANGE 200
#define WEAVE_HEAL_HEALTH 3
#define WEAVE_HEAL_TIME 8000
#define WEAVE_HEALM_DELAY 1
#define WEAVE_HEALS_DELAY 3
#define WEAVE_HEAL_PULSE_TIME 500

#define WEAVE_GRABPLAYER_TIME 12500
#define WEAVE_GRABPLAYER_RANGE 200

#define WEAVE_PROTECTFIRE_TIME 45000
#define WEAVE_PROTECTFIRE 100

#define WEAVE_PROTECTAIR_TIME 45000
#define WEAVE_PROTECTAIR 100

#define WEAVE_PROTECTEARTH_TIME 45000
#define WEAVE_PROTECTEARTH 100

#define WEAVE_PROTECTWATER_TIME 45000
#define WEAVE_PROTECTWATER 100

#define WEAVE_ICESHARDS_START 50
#define WEAVE_ICESHARDS_DAMAGE 14
#define WEAVE_ICESHARDS_SPLASH 20
#define WEAVE_ICESHARDS_SPLASH_DAMAGE 5
#define WEAVE_ICESHARDS_SEPARATION 15
#define WEAVE_ICESHARDS_SPREAD 10
#define WEAVE_ICESHARDS_COUNT 2
#define WEAVE_ICESHARDS_SPEED 1200
#define WEAVE_ICESHARDS_ACCELERATION -25
#define TIME_ICESHARD_THINK 400
#define TIME_ICESHARD_COUNT 20
#define WEAVE_ICESHARDS_STOP_SPEED 1000
#define WEAVE_ICESHARDS_BOUNCE_SCALE 0.975
#define WEAVE_ICESHARDS_BOUNCE_DMGSCALE 0.8

#define WEAVE_ICEMULTISHARDS_COUNT 3

#define WEAVE_LIGHTSOURCE_TIME 45000

#define WEAVE_SLOWPOISON_SLOWFACTOR 0.36
#define WEAVE_SLOWPOISON_TICKS 20
#define WEAVE_SLOWPOISON_DURATION 8000
#define WEAVE_SLOWPOISON_PROJDAMAGE 10
#define WEAVE_SLOWPOISON_PROJSPLASH 0
#define WEAVE_SLOWPOISON_PROJSPLASH_DAMAGE 0
#define WEAVE_SLOWPOISON_PROJSPEED 1600
#define WEAVE_SLOWPOISON_DPS 6
#define WEAVE_SLOWPOISON_START 50

#define WEAVE_POISON_TICKS 100
#define WEAVE_POISON_DURATION 40000
#define WEAVE_POISON_PROJDAMAGE 8
#define WEAVE_POISON_PROJSPLASH 0
#define WEAVE_POISON_PROJSPLASH_DAMAGE 0
#define WEAVE_POISON_PROJSPEED 1600
#define WEAVE_POISON_MINDMG 5
#define WEAVE_POISON_MAXDMG 26
#define WEAVE_POISON_DMGINCREMENT 3
#define WEAVE_POISON_START 50

#define WEAVE_CURE_CAST_RANGE 75
#define WEAVE_CURE_HEALTH 10

#define WEAVE_RIP_DAMAGE 70
#define WEAVE_RIP_CAST_RANGE 250

#define WEAVE_STAMINA_CAST_RANGE 200
#define WEAVE_STAMINA_TIME 8500
#define WEAVE_STAMINA_PULSE_TIME 100

#define WEAVE_LIGHTNING_CAST_RANGE 2000
#define WEAVE_LIGHTNING_DAMAGE 110
#define WEAVE_LIGHTNING_SPLASH_DAMAGE 100
#define WEAVE_LIGHTNING_RADIUS 250
#define WEAVE_LIGHTNING_DELAY 3000
#define WEAVE_LIGHTNING_PERIOD 2500
#define WEAVE_LIGHTNING_TIME 18500
#define WEAVE_LIGHTNING_SKYHEIGHT 8192
#define WEAVE_LIGHTNING_OFFSET 16.0f

#define WEAVE_EXPLOSIVE_CAST_RANGE 200
#define WEAVE_EXPLOSIVE_S_SPLASH_DAMAGE 50
#define WEAVE_EXPLOSIVE_M_SPLASH_DAMAGE 65
#define WEAVE_EXPLOSIVE_S_RADIUS 350
#define WEAVE_EXPLOSIVE_M_RADIUS 400
#define WEAVE_EXPLOSIVE_OFFSET 10.0f

#define WEAVE_LINKTHINK_TIME 200

typedef enum
{
	WST_HELD,
	WST_INPROCESS,
	WST_EXPIRED,
	WST_RELEASED,
	WST_INPROCESSRELEASED,
} weaver_weavestates;

typedef enum
{
	WVG_AGRESSIVE,
	WVG_DEFENSIVE
} weaver_group;

typedef enum
{
	WVP_NONE,
	WVP_AIR,
	WVP_AIRFIRE,
	WVP_FIRE,
	WVP_EARTHFIRE,
	WVP_EARTH,
	WVP_EARTHWATER,
	WVP_WATER,
	WVP_AIRWATER,
	WVP_SPIRIT,
	WVP_NUMBER
} weaver_powers;

typedef enum
{
	WVT_NONE = 0,
	WVT_SELF = BIT(0),
	WVT_PLAYER_ALLY = BIT(1),
	WVT_PLAYER_ENEMY = BIT(2),
	WVT_AI_ALLY = BIT(3),
	WVT_AI_ENEMY = BIT(4),
	WVT_SHOT = BIT(5),
	WVT_ENTITY = BIT(6),
	WVT_SURFACE = BIT(7),
	WVT_OBJECT = BIT(8),
	WVT_SPECIAL = BIT(9)
} weaver_effectType;

#define WVT_PLAYER_OTHER (WVT_PLAYER_ENEMY | WVT_PLAYER_ALLY)
#define WVT_AI_OTHER (WVT_AI_ALLY | WVT_AI_ENEMY)

#define WVT_PL_AI_ALLY (WVT_PLAYER_ALLY | WVT_AI_ALLY)
#define WVT_PL_AI_ENEMY (WVT_PLAYER_ALLY | WVT_AI_ENEMY)
#define WVT_PL_AI_OTHER (WVT_PLAYER_OTHER | WVT_AI_OTHER)

typedef enum
{
	WTIER_NONE = 0,
	WTIER_1 = 1,
	WTIER_2 = 2,
	WTIER_3 = 3,
	WTIER_4 = 3,
	WTIER_5 = 5,
	WTIER_6 = 6
} weaver_tiers;

#define WEAVEINFO(id,holdable,holdPowerCharge,holdMaxTime,castCharges,castDelay,shotDelay,primaryPower,effectType,tier,group,name,nameP) id,
typedef enum
{
	WVW_NONE,
#include "spell_info.def"
	WVW_NUM_WEAVES
} weaver_weaves;

typedef struct weaver_weaveInfo_s
{
	qboolean        holdable;			// qfalse if the spell must be cast instantly
	int             holdPowerCharge;	// amount of power used while holding it
	int             holdMaxTime;		// maximum amount of time it can be held
	int             castCharges;		// number of charges available to cast
	int             castDelay;			// minmum time between casting charges
	weaver_powers   primaryPower;		// primary power
	weaver_effectType effectType;		// effect type
	weaver_tiers    tier;				// tier
	weaver_group    group;				// aggressive or defensive
	const char     *name;				// internal name, used for loading resources. [a-z]+
	const char     *nameP;				// presentable name, used for showing players. [a-zA-Z ]*
} weaver_weaveInfo_t;

typedef struct weaver_threadsMap_s weaver_threadsMap_t;

const char *WeaveGroupNames[2];
const char *WeavePowerNames[10];

struct weaver_threadsMap_s
{
	weaver_threadsMap_t *next[WVP_NUMBER];
	weaver_weaves   weaveA;
	weaver_weaves   weaveD;
};

#endif							// SPELL_COMMON_H
