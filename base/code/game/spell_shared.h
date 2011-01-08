/*
===========================================================================
This file is part of the weaver game.

It has misc weaver functions, used by both game and cgame.
===========================================================================
*/

weaver_threadsMap_t *ThreadsTree(int threads[MAX_THREADS]);
int             ThreadsToWeaveID(int group, int threads[MAX_THREADS]);

void            WeaveProtectScales(int weavenum, float *airprotect, float *fireprotect, float *earthprotect, float *waterprotect);

qboolean        WeaveHoldable(int weaveID);
int             WeaveHoldPower(int weaveID);
weaver_tiers    WeaveTier(int weaveID);
int             WeaveCharges(int weaveID);
weaver_powers   WeavePrimaryPower(int weaveID);
weaver_group    WeaveADGroup(int weaveID);
char           *WeaveName(int weaveID);
char           *WeaveNameP(int weaveID);
int             WeaveEffectType(int weaveID);
int             WeaveCastDelay(int weaveID);

void            LoadWeaveInfo(weaver_weaveInfo_t *info, int weaveID);
