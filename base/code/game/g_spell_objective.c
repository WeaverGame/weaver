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

// g_spell_objective.c -- objectives
#include "g_local.h"

void G_ObjectiveAnnounce(objective_events objevp, gentity_t * ent, gentity_t * other)
{
	gentity_t      *te;
	te = G_TempEntity(ent->s.pos.trBase, EV_GLOBAL_OBJ);
	te->s.eventParm = objevp;
	te->r.svFlags |= SVF_BROADCAST;
	if(ent != NULL)
	{
		te->s.otherEntityNum = ent->s.number;
	}
	if(other != NULL)
	{
		te->s.otherEntityNum2 = other->s.number;
	}
}
