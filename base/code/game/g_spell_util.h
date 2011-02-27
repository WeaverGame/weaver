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

// g_spell_util.h -- misc weaver utilities, ie. a little abstraction.
#ifndef __G_SPELL_UTIL_H
#define __G_SPELL_UTIL_H

static ID_INLINE void G_HeldWeave_SetState(gentity_t *held, weaver_weavestates s)
{
	held->s.generic1 = s;
}

static ID_INLINE weaver_weavestates G_HeldWeave_GetState(gentity_t *held)
{
	return held->s.generic1;
}

static ID_INLINE void G_HeldWeave_SetPower(gentity_t *held, int p)
{
	held->s.frame = p;
}

static ID_INLINE int G_HeldWeave_GetPower(gentity_t *held)
{
	return held->s.frame;
}

static ID_INLINE void G_HeldWeave_SetCharges(gentity_t *held, int c)
{
	held->s.torsoAnim = c;
}

static ID_INLINE int G_HeldWeave_GetCharges(gentity_t *held)
{
	return held->s.torsoAnim;
}

#endif							// __G_SPELL_UTIL_H