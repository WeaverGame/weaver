/*
===========================================================================
This file is part of the weaver entity support.

These are misc weaver entities which require more client code
===========================================================================
*/

#include "cg_local.h"

void CG_ShieldInfo(centity_t * cent)
{
	refEntity_t     ent;
	entityState_t  *s1;
	vec3_t          normalaxis[3];
	int             k;

	// create the render entity
	memset(&ent, 0, sizeof(ent));

	s1 = &cent->currentState;

	// calculate the axis
	VectorCopy(s1->angles, cent->lerpAngles);

	// surface normal axis
	VectorNormalize2(cent->currentState.angles, normalaxis[2]);
	PerpendicularVector(normalaxis[1], normalaxis[2]);
	CrossProduct(normalaxis[1], normalaxis[2], normalaxis[0]);

	//Com_Printf("ShieldInfo Normal=%f %f %f\n", normalaxis[2][0], normalaxis[2][1], normalaxis[2][2]);
	AxisCopy(normalaxis, ent.axis);

	// flicker between two skins
	ent.skinNum = cg.clientFrame;
	ent.hModel = cgs.media.weaverShieldInfo;
	ent.renderfx = RF_NOSHADOW;

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);
}
