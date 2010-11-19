/*
===========================================================================
This file is part of the weaver game.

It tracks game statistics
===========================================================================
*/

//
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
				break;
			case SVET_PP:
				break;
			case SVET_PPW:
				break;
			default:
				break;
		}

		Com_Printf("\n");
	}
}
