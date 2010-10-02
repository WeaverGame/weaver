/*
===========================================================================
This file is part of the weaver game.

It has misc weaver functions.
===========================================================================
*/
#include "../../../code/qcommon/q_shared.h"
#include "g_local.h"

void DEBUGWEAVEING_LVL(char *str, int level)
{
	if(g_debugWeaving.integer >= level)
	{
		Com_Printf("%s\n", str);
	}
}
void DEBUGWEAVEING(char *str)
{
	DEBUGWEAVEING_LVL(str, 1);
}

/*
=================
WeaveProtectCheck

Checks if player's Protects are expended
=================
*/
void WeaveProtectCheck(gclient_t * checkClient)
{
	DEBUGWEAVEING("WeaveProtectCheck: start");
	if(checkClient->ps.stats[STAT_FIREPROTECT] <= 0)
	{
		if(checkClient->protectHeldFire)
		{
			G_ReleaseWeave(checkClient->protectHeldFire->target_ent);
		}
	}
	if(checkClient->ps.stats[STAT_AIRPROTECT] <= 0)
	{
		if(checkClient->protectHeldAir)
		{
			G_ReleaseWeave(checkClient->protectHeldAir->target_ent);
		}
	}
	if(checkClient->ps.stats[STAT_EARTHPROTECT] <= 0)
	{
		if(checkClient->protectHeldEarth)
		{
			G_ReleaseWeave(checkClient->protectHeldEarth->target_ent);
		}
	}
	if(checkClient->ps.stats[STAT_WATERPROTECT] <= 0)
	{
		if(checkClient->protectHeldWater)
		{
			G_ReleaseWeave(checkClient->protectHeldWater->target_ent);
		}
	}
	DEBUGWEAVEING("WeaveProtectCheck: end");
}

int PowerEncode(int threads[MAX_THREADS], int offset, int count)
{
	int             n = 0;
	int             i = 0;

	for(i = 0; i < count; i++)
	{
		//Com_Printf("n=%d thread=%d mult=%d add=%d\n", n, threads[offset+i], (int)pow(WVP_NUMBER, i), (threads[offset+i] * (int)pow(WVP_NUMBER, i)));
		n += (threads[offset + i] * (int)pow(WVP_NUMBER, i));
	}
	return n;
}
