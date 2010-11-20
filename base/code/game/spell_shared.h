/*
===========================================================================
This file is part of the weaver game.

It has misc weaver functions, used by both game and cgame.
===========================================================================
*/

weaver_threadsMap_t *ThreadsTree(int threads[MAX_THREADS]);
int             ThreadsToWeaveID(int group, int threads[MAX_THREADS]);
void            WeaveProtectScales(int weavenum, float *airprotect, float *fireprotect, float *earthprotect, float *waterprotect);
int             WeaveTier(int weaveID);
int             WeaveCharges(int weaveID);
