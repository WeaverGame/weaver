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

// spell_shared.h -- misc weaver functions, used by both game and cgame

weaver_threadsMap_t *ThreadsTree(int threads[MAX_THREADS]);
int             ThreadsToWeaveID(int group, int threads[MAX_THREADS]);

void            WeaveProtectScales(int weavenum, float *airprotect, float *fireprotect, float *earthprotect, float *waterprotect);
int             WeaveCastTime(int weaveID);

qboolean        WeaveHoldable(int weaveID);
int             WeaveHoldPower(int weaveID);
weaver_tiers    WeaveTier(int weaveID);
int             WeaveCharges(int weaveID);
weaver_powers   WeavePrimaryPower(int weaveID);
weaver_group    WeaveADGroup(int weaveID);
const char     *WeaveName(int weaveID);
const char     *WeaveNameP(int weaveID);
int             WeaveEffectType(int weaveID);
int             WeaveCastDelay(int weaveID);

void            LoadWeaveInfo(weaver_weaveInfo_t *info, int weaveID);
