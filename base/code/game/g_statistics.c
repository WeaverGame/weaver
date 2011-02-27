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

// g_statistics.c -- tracks game statistics
#include "g_local.h"

/* Variables */
statistics_t        game_stats;

/* Initialization */
void G_StatInitPlayer(gentity_t * ent)
{
	if(ent->client == NULL)
	{
		return;
	}

	Com_Memset(&game_stats.p[(ent - g_entities)], 0, sizeof(statistics_p_t));
}

void G_StatInit(void)
{
	Com_Memset(&game_stats, 0, sizeof(statistics_t));
}

/* Using statistics */
statValue_t *StatGetStat(statField_t f, statEntType sET, gentity_t * ent, gentity_t * other, int weapon)
{
	switch(sET)
	{
		case SVET_P:
			if(ent != NULL)
			{
				return &(game_stats.p[ent - g_entities].stat[f - STEL_P_FIRST]);
			}
			Com_Printf("Warning: Statistics call parameters do not match P field %s.\n", statFieldDefs[f].name1);
			break;
		case SVET_PW:
			if(ent != NULL)
			{
				return &(game_stats.p[ent - g_entities].pw[weapon].stat[f - STEL_PW_FIRST]);
			}
			Com_Printf("Warning: Statistics call parameters do not match PW field %s.\n", statFieldDefs[f].name1);
			break;
		case SVET_PP:
			if(ent != NULL && other != NULL)
			{
				return &(game_stats.p[ent - g_entities].pp[other - g_entities].stat[f - STEL_PP_FIRST]);
			}
			Com_Printf("Warning: Statistics call parameters do not match PP field %s.\n", statFieldDefs[f].name1);
			break;
		case SVET_PPW:
			if(ent != NULL && other != NULL)
			{
				return &(game_stats.p[ent - g_entities].pp[other - g_entities].ppw[weapon].stat[f - STEL_PPW_FIRST]);
			}
			Com_Printf("Warning: Statistics call parameters do not match PPW field %s.\n", statFieldDefs[f].name1);
			break;
		default:
			break;
	}
	return NULL;
}

void G_StatCountAdd(statField_t f, gentity_t * ent, gentity_t * other, int weapon, int val)
{
	statEntType et;
	statValue_t *stat;

	et = statFieldDefs[f].entType;
	if(statFieldDefs[f].valType != STVT_COUNTER)
	{
		Com_Printf("Warning: Statistics call for wrong val type, field %s.\n", statFieldDefs[f].name1);
		return;
	}

	stat = StatGetStat(f, et, ent, other, weapon);
	if(stat == NULL)
	{
		return;
	}

	Com_Printf("Stats: %s ent1=%d ent2=%d, weap=%d, inc=%d\n", statFieldDefs[f].name1, ent->s.number, other->s.number, weapon, val);

	stat->c += val;
}

void G_StatTimeStart(statField_t f, gentity_t * ent, gentity_t * other, int weapon)
{
	statEntType et;
	statValue_t *stat;

	et = statFieldDefs[f].entType;
	if(statFieldDefs[f].valType != STVT_TIMER)
	{
		Com_Printf("Warning: Statistics call for wrong val type, field %s.\n", statFieldDefs[f].name1);
		return;
	}

	stat = StatGetStat(f, et, ent, other, weapon);
	if(stat == NULL)
	{
		return;
	}

	if(stat->s != 0)
	{
		Com_Printf("Warning: Statistics call starting timer twice, field %s.\n", statFieldDefs[f].name1);
	}

	stat->s = ((unsigned int)level.time);
}

void G_StatTimeAdd(statField_t f, gentity_t * ent, gentity_t * other, int weapon)
{
	statEntType et;
	statValue_t *stat;

	et = statFieldDefs[f].entType;
	if(statFieldDefs[f].valType != STVT_TIMER)
	{
		Com_Printf("Warning: Statistics call for wrong val type, field %s.\n", statFieldDefs[f].name1);
		return;
	}

	stat = StatGetStat(f, et, ent, other, weapon);
	if(stat == NULL)
	{
		return;
	}

	if(stat->s == 0)
	{
		Com_Printf("Warning: Statistics call adding unstarted timer, field %s.\n", statFieldDefs[f].name1);
	}
	else
	{
		stat->c += ((unsigned int)level.time) - stat->s;
		stat->s = 0;
	}
}

void StatDumpValue(statField_t f, statValue_t * s)
{
	unsigned int sec;

	switch(statFieldDefs[f].valType)
	{
		case STVT_COUNTER:
			Com_Printf(" %4u", s->c);
			break;
		case STVT_TIMER:
			sec = s->c;
			if(s->s != 0)
			{
				sec += ((unsigned int)level.time - s->s);
			}
			sec /= 1000;
			Com_Printf(" %4u", sec);
			break;
		default:
			break;
	}
}

void StatDumpP(statField_t f)
{
	int i;

	for(i = 0; i < MAX_CLIENTS; i++)
	{
		if(g_entities[i].client == NULL)
		{
			break;
		}
		Com_Printf("p%02d: ", i);
		StatDumpValue(f, &(game_stats.p[i].stat[f - STEL_P_FIRST]));
		Com_Printf("\n");
	}
}

void StatDumpPW(statField_t f)
{
	int i, j;

	for(i = 0; i < MAX_CLIENTS; i++)
	{
		if(g_entities[i].client == NULL)
		{
			break;
		}
		Com_Printf("p%02d: ", i);
		for(j = 0; j < STEL_WEAPON_LENGTH; j++)
		{
			StatDumpValue(f, &(game_stats.p[i].pw[j].stat[f - STEL_P_FIRST]));
			Com_Printf(" ");
		}
		Com_Printf("\n");
	}
}

void StatDumpPP(statField_t f)
{
	int i, j;

	for(i = 0; i < MAX_CLIENTS; i++)
	{
		if(g_entities[i].client == NULL)
		{
			break;
		}
		Com_Printf("p%02d: ==\n", i);
		for(j = 0; j < MAX_CLIENTS; j++)
		{
			if(g_entities[j].client == NULL)
			{
				break;
			}
			Com_Printf("p%02d: ", j);
			StatDumpValue(f, &(game_stats.p[i].pp[j].stat[f - STEL_P_FIRST]));
			Com_Printf(" ");
		}
		Com_Printf("\n");
	}
}

void StatDumpPPW(statField_t f)
{
	int i, j, k;

	for(i = 0; i < MAX_CLIENTS; i++)
	{
		if(g_entities[i].client == NULL)
		{
			break;
		}
		Com_Printf("p%02d: ==\n", i);
		for(j = 0; j < MAX_CLIENTS; j++)
		{
			if(g_entities[j].client == NULL)
			{
				break;
			}
			Com_Printf("p%02d: ", j);
			for(k = 0; k < STEL_WEAPON_LENGTH; k++)
			{
				StatDumpValue(f, &(game_stats.p[i].pp[j].ppw[k].stat[f - STEL_P_FIRST]));
				Com_Printf(" ");
			}
			Com_Printf("\n");
		}
		Com_Printf("\n");
	}
}

void G_StatDump(void)
{
	int i;

	for(i = 0; i < STATF_MAX; i++)
	{
		Com_Printf("%s\n========\n", statFieldDefs[i].name1);

		switch(statFieldDefs[i].entType)
		{
			case SVET_P:
				StatDumpP(i);
				break;
			case SVET_PW:
				StatDumpPW(i);
				break;
			case SVET_PP:
				StatDumpPP(i);
				break;
			case SVET_PPW:
				StatDumpPPW(i);
				break;
			default:
				break;
		}

		Com_Printf("\n");
	}
}
