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

// cg_spell_ents.c -- misc weaver entities which require more client code
#include "cg_local.h"

void CG_ShieldInfo(centity_t * cent)
{
	refEntity_t     ent;
	entityState_t  *s1;
	vec3_t          normalaxis[3];
	float           scale;

	s1 = &cent->currentState;

	// calculate the axis
	VectorCopy(s1->angles, cent->lerpAngles);

	scale = s1->frame/1000.0f;

	// surface normal axis
	VectorNormalize2(cent->currentState.angles, normalaxis[2]);
	PerpendicularVector(normalaxis[1], normalaxis[2]);
	CrossProduct(normalaxis[1], normalaxis[2], normalaxis[0]);

	//Com_Printf("ShieldInfo Normal=%f %f %f\n", normalaxis[2][0], normalaxis[2][1], normalaxis[2][2]);
	
	// create the render entity
	memset(&ent, 0, sizeof(ent));

	VectorCopy(cent->lerpOrigin, ent.origin);

	VectorScale(normalaxis[0], scale, ent.axis[0]);
	VectorScale(normalaxis[1], scale, ent.axis[1]);
	VectorScale(normalaxis[2], scale, ent.axis[2]);

	// flicker between two skins
	ent.skinNum = cg.clientFrame;
	ent.hModel = cgs.media.weaverShieldInfo;
	ent.renderfx = RF_NOSHADOW;

	ent.nonNormalizedAxes = qtrue;
	//AxisCopy(normalaxis, ent.axis);

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);
}
