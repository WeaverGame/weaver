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

// g_statistics.h -- data structures and constants for tracking game statistics

/* Statistics ENUM */
#define STATFIELD(e, t, n1, n2) e,
typedef enum {
	#include "g_statfields.def"
	STATF_MAX
} statField_t;

/* Statistics Info */
typedef enum {
	STVT_NULL = 0,
	STVT_COUNTER,
	STVT_TIMER
} statValType;

typedef enum {
	SVET_NULL = 0,
	SVET_P,
	SVET_PW,
	SVET_PP,
	SVET_PPW
} statEntType;

typedef struct statFieldDef_s {
	const statValType valType;
	const statEntType entType;
	const char *name1;
	const char *name2;
} statFieldDef_t;

#define STATFIELD_P(e, t, n1, n2) {t, SVET_P, n1, n2},
#define STATFIELD_PW(e, t, n1, n2) {t, SVET_PW, n1, n2},
#define STATFIELD_PP(e, t, n1, n2) {t, SVET_PP, n1, n2},
#define STATFIELD_PPW(e, t, n1, n2) {t, SVET_PPW, n1, n2},
static const statFieldDef_t statFieldDefs[] = {
	#include "g_statfields.def"
	{STVT_NULL, SVET_NULL, NULL, NULL}
};

/* Statistics Enum Lengths */
#define STATFIELD_P_FIRST(e)	STEL_P_FIRST = (e),
#define STATFIELD_P_LAST(e)		STEL_P_LAST = (e),
#define STATFIELD_PW_FIRST(e)	STEL_PW_FIRST = (e),
#define STATFIELD_PW_LAST(e)	STEL_PW_LAST = (e),
#define STATFIELD_PP_FIRST(e)	STEL_PP_FIRST = (e),
#define STATFIELD_PP_LAST(e)	STEL_PP_LAST = (e),
#define STATFIELD_PPW_FIRST(e)	STEL_PPW_FIRST = (e),
#define STATFIELD_PPW_LAST(e)	STEL_PPW_LAST = (e),
typedef enum {
	#include "g_statfields.def"
	STEL_MAX
} statFieldSize_t;

#define STEL_WEAPON_FIRST	WVW_NONE
#define STEL_WEAPON_LAST	WVW_NUM_WEAVES
#define STEL_WEAPON_LENGTH	(STEL_WEAPON_LAST - STEL_WEAPON_FIRST)

#define STEL_P_LENGTH	((STEL_P_LAST - STEL_P_FIRST) + 1)
#define STEL_PW_LENGTH	((STEL_PW_LAST - STEL_PW_FIRST) + 1)
#define STEL_PP_LENGTH	((STEL_PP_LAST - STEL_PP_FIRST) + 1)
#define STEL_PPW_LENGTH	((STEL_PPW_LAST - STEL_PPW_FIRST) + 1)

typedef struct statValue_s {
	unsigned int c; // Count
	unsigned int s; // Start time for timers
} statValue_t;

typedef struct statistics_ppw_s {
	statValue_t stat[STEL_PPW_LENGTH];
} statistics_ppw_t;

typedef struct statistics_pp_s {
	statValue_t stat[STEL_PP_LENGTH];
	statistics_ppw_t ppw[STEL_WEAPON_LENGTH];
} statistics_pp_t;

typedef struct statistics_pw_s {
	statValue_t stat[STEL_PW_LENGTH];
} statistics_pw_t;

typedef struct statistics_p_s {
	statValue_t stat[STEL_P_LENGTH];
	statistics_pp_t pp[MAX_CLIENTS];
	statistics_pw_t pw[STEL_WEAPON_LENGTH];
} statistics_p_t;

typedef struct statistics_s {
	statistics_p_t p[MAX_CLIENTS];
} statistics_t;
