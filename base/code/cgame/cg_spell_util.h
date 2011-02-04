/*
===========================================================================
This file is part of the weaver game.

It has misc weaver utilities, ie. a little abstraction.
===========================================================================
*/

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