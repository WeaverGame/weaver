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

// cg_spell_util.h -- misc weaver utilities, ie. a little abstraction.
#ifndef __CG_SPELL_UTIL_H
#define __CG_SPELL_UTIL_H

static ID_INLINE void CG_HeldWeave_SetState(centity_t *held, weaver_weavestates s)
{
	held->currentState.generic1 = s;
}

static ID_INLINE weaver_weavestates CG_HeldWeave_GetState(centity_t *held)
{
	return held->currentState.generic1;
}

static ID_INLINE void CG_HeldWeave_SetPower(centity_t *held, int p)
{
	held->currentState.frame = p;
}

static ID_INLINE int CG_HeldWeave_GetPower(centity_t *held)
{
	return held->currentState.frame;
}

static ID_INLINE void CG_HeldWeave_SetCharges(centity_t *held, int c)
{
	held->currentState.torsoAnim = c;
}

static ID_INLINE int CG_HeldWeave_GetCharges(centity_t *held)
{
	return held->currentState.torsoAnim;
}

#endif							// __CG_SPELL_UTIL_H

