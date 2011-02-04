/*
===========================================================================
This file is part of the weaver game.

It has misc weaver utilities, ie. a little abstraction.
===========================================================================
*/

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