/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2006 Tim Angus
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>
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
//
// cg_view.c -- setup all the parameters (position, angle, etc)
// for a 3D rendering


#include "cg_local.h"


/*
=============================================================================

  MODEL TESTING

The viewthing and gun positioning tools from Q2 have been integrated and
enhanced into a single model testing facility.

Model viewing can begin with either "testmodel <modelname>" or "testgun <modelname>".

The names must be the full pathname after the basedir, like
"models/weapons/v_launch/tris.md3" or "players/male/tris.md3"

Testmodel will create a fake entity 100 units in front of the current view
position, directly facing the viewer.  It will remain immobile, so you can
move around it to view it from different angles.

Testgun will cause the model to follow the player around and supress the real
view weapon model.  The default frame 0 of most guns is completely off screen,
so you will probably have to cycle a couple frames to see it.

"nextframe", "prevframe", "nextskin", and "prevskin" commands will change the
frame or skin of the testmodel.  These are bound to F5, F6, F7, and F8 in
q3default.cfg.

If a gun is being tested, the "gun_x", "gun_y", and "gun_z" variables will let
you adjust the positioning.

=============================================================================
*/

/*
=================
CG_TestModel_f

Creates an entity in front of the current position, which
can then be moved around
=================
*/
void CG_TestModel_f(void)
{
	vec3_t          angles;

	memset(&cg.testModelEntity, 0, sizeof(cg.testModelEntity));
	if(trap_Argc() < 2)
	{
		CG_Printf("usage: testModel <modelname> [lerp]\n");
		return;
	}

	Q_strncpyz(cg.testModelName, CG_Argv(1), MAX_QPATH);
	cg.testModelEntity.hModel = trap_R_RegisterModel(cg.testModelName);

	if(trap_Argc() == 3)
	{
		cg.testModelEntity.backlerp = 1.0f - atof(CG_Argv(2));
		cg.testModelEntity.frame = 1;
		cg.testModelEntity.oldframe = 0;
	}
	if(!cg.testModelEntity.hModel)
	{
		CG_Printf("Can't register model\n");
		return;
	}

	VectorMA(cg.refdef.vieworg, 100, cg.refdef.viewaxis[0], cg.testModelEntity.origin);

	angles[PITCH] = 0;
	angles[YAW] = 180 + cg.refdefViewAngles[1];
	angles[ROLL] = 0;

	AnglesToAxis(angles, cg.testModelEntity.axis);
	cg.testGun = qfalse;
}

/*
=================
CG_TestGun_f

Replaces the current view weapon with the given model
=================
*/
void CG_TestGun_f(void)
{
	CG_TestModel_f();
	cg.testGun = qtrue;
	cg.testModelEntity.renderfx |= RF_MINLIGHT | RF_DEPTHHACK | RF_FIRST_PERSON;
}


/*
=================
CG_TestAnimation_f
=================
*/
void CG_TestAnimation_f(void)
{
	if(trap_Argc() < 2)
	{
		CG_Printf("usage: testAnimation <animationname>\n");
		return;
	}

	Q_strncpyz(cg.testAnimationName, CG_Argv(1), MAX_QPATH);
	cg.testAnimation = trap_R_RegisterAnimation(cg.testAnimationName);
	//CG_RegisterAnimation

	if(!cg.testAnimation)
	{
		CG_Printf("Can't register animation\n");
		return;
	}

	// check the bones if they match
	if(!trap_R_CheckSkeleton(&cg.testModelEntity.skeleton, cg.testModelEntity.hModel, cg.testAnimation))
	{
		CG_Printf("Can't verify animation\n");
		return;
	}

	// modify bones and set proper local bounds for culling
	if(!trap_R_BuildSkeleton(&cg.testModelEntity.skeleton,
							 cg.testAnimation,
							 cg.testModelEntity.oldframe, cg.testModelEntity.frame, 1.0 - cg.testModelEntity.backlerp, qfalse))
	{
		CG_Printf("Can't build animation\n");
		return;
	}
}


/*
=================
CG_TestBlend_f
=================
*/
void CG_TestBlend_f(void)
{
	if(trap_Argc() < 2)
	{
		CG_Printf("usage: testBlend <animationname> [lerp factor 0.0 - 1.0]\n");
		return;
	}

	if(!cg.testAnimation)
	{
		CG_Printf("Use testAnimation first to set a valid animation\n");
		return;
	}

	Q_strncpyz(cg.testAnimation2Name, CG_Argv(1), MAX_QPATH);
	cg.testAnimation2 = trap_R_RegisterAnimation(cg.testAnimation2Name);

	if(!cg.testAnimation2)
	{
		CG_Printf("Can't register animation2 for blending\n");
		return;
	}

	if(trap_Argc() == 3)
	{
		cg.testModelEntity.backlerp = 1.0f - atof(CG_Argv(2));
	}

	// modify bones and set proper local bounds for culling
	if(!trap_R_BuildSkeleton(&cg.testModelEntity.skeleton,
							 cg.testAnimation,
							 cg.testModelEntity.oldframe, cg.testModelEntity.frame, 1.0 - cg.testModelEntity.backlerp, qfalse))
	{
		CG_Printf("Can't build animation\n");
		return;
	}

	if(!trap_R_BuildSkeleton(&cg.testAnimation2Skeleton,
							 cg.testAnimation2,
							 cg.testModelEntity.oldframe, cg.testModelEntity.frame, 1.0 - cg.testModelEntity.backlerp, qfalse))
	{
		CG_Printf("Can't build animation2\n");
		return;
	}

	// lerp between first and second animation
	if(!trap_R_BlendSkeleton(&cg.testModelEntity.skeleton, &cg.testAnimation2Skeleton, 1.0 - cg.testModelEntity.backlerp))
	{
		CG_Printf("Can't blend animation2\n");
		return;
	}
}

void CG_TestModelNextFrame_f(void)
{
	cg.testModelEntity.frame++;
	CG_Printf("frame %i\n", cg.testModelEntity.frame);

	if(cg.testAnimation)
	{
		if(!trap_R_BuildSkeleton(&cg.testModelEntity.skeleton,
								 cg.testAnimation,
								 cg.testModelEntity.oldframe,
								 cg.testModelEntity.frame, 1.0 - cg.testModelEntity.backlerp, qfalse))
		{
			CG_Printf("Can't build animation\n");
		}
	}

	if(cg.testAnimation2)
	{
		if(!trap_R_BuildSkeleton(&cg.testAnimation2Skeleton,
								 cg.testAnimation2,
								 cg.testModelEntity.oldframe,
								 cg.testModelEntity.frame, 1.0 - cg.testModelEntity.backlerp, qfalse))
		{
			CG_Printf("Can't build animation2\n");
		}

		if(!trap_R_BlendSkeleton(&cg.testModelEntity.skeleton, &cg.testAnimation2Skeleton, 0.5))
		{
			CG_Printf("Can't blend animation2\n");
		}
	}
}

void CG_TestModelPrevFrame_f(void)
{
	cg.testModelEntity.frame--;
	if(cg.testModelEntity.frame < 0)
	{
		cg.testModelEntity.frame = 0;
	}
	CG_Printf("frame %i\n", cg.testModelEntity.frame);

	if(cg.testAnimation)
	{
		if(!trap_R_BuildSkeleton(&cg.testModelEntity.skeleton,
								 cg.testAnimation,
								 cg.testModelEntity.oldframe,
								 cg.testModelEntity.frame, 1.0 - cg.testModelEntity.backlerp, qfalse))
		{
			CG_Printf("Can't build animation\n");
		}
	}

	if(cg.testAnimation2)
	{
		if(!trap_R_BuildSkeleton(&cg.testAnimation2Skeleton,
								 cg.testAnimation2,
								 cg.testModelEntity.oldframe,
								 cg.testModelEntity.frame, 1.0 - cg.testModelEntity.backlerp, qfalse))
		{
			CG_Printf("Can't build animation2\n");
		}

		if(!trap_R_BlendSkeleton(&cg.testModelEntity.skeleton, &cg.testAnimation2Skeleton, 0.5))
		{
			CG_Printf("Can't blend animation2\n");
		}
	}
}

void CG_TestModelIncreaseLerp_f(void)
{
	cg.testModelEntity.backlerp -= 0.1f;
	if(cg.testModelEntity.backlerp < 0)
	{
		cg.testModelEntity.backlerp = 0;
	}
	CG_Printf("lerp %f\n", 1.0f - cg.testModelEntity.backlerp);

	if(cg.testAnimation)
	{
		if(!trap_R_BuildSkeleton(&cg.testModelEntity.skeleton,
								 cg.testAnimation,
								 cg.testModelEntity.oldframe,
								 cg.testModelEntity.frame, 1.0 - cg.testModelEntity.backlerp, qfalse))
		{
			CG_Printf("Can't build animation\n");
		}
	}

	if(cg.testAnimation2)
	{
		if(!trap_R_BuildSkeleton(&cg.testAnimation2Skeleton,
								 cg.testAnimation2,
								 cg.testModelEntity.oldframe,
								 cg.testModelEntity.frame, 1.0 - cg.testModelEntity.backlerp, qfalse))
		{
			CG_Printf("Can't build animation2\n");
		}

		if(!trap_R_BlendSkeleton(&cg.testModelEntity.skeleton, &cg.testAnimation2Skeleton, 1.0 - cg.testModelEntity.backlerp))
		{
			CG_Printf("Can't blend animation2\n");
		}
	}
}

void CG_TestModelDecreaseLerp_f(void)
{
	cg.testModelEntity.backlerp += 0.1f;
	if(cg.testModelEntity.backlerp > 1.0)
	{
		cg.testModelEntity.backlerp = 1;
	}
	CG_Printf("lerp %f\n", 1.0f - cg.testModelEntity.backlerp);

	if(cg.testAnimation)
	{
		if(!trap_R_BuildSkeleton(&cg.testModelEntity.skeleton,
								 cg.testAnimation,
								 cg.testModelEntity.oldframe,
								 cg.testModelEntity.frame, 1.0 - cg.testModelEntity.backlerp, qfalse))
		{
			CG_Printf("Can't build animation\n");
		}
	}

	if(cg.testAnimation2)
	{
		if(!trap_R_BuildSkeleton(&cg.testAnimation2Skeleton,
								 cg.testAnimation2,
								 cg.testModelEntity.oldframe,
								 cg.testModelEntity.frame, 1.0 - cg.testModelEntity.backlerp, qfalse))
		{
			CG_Printf("Can't build animation2\n");
		}

		if(!trap_R_BlendSkeleton(&cg.testModelEntity.skeleton, &cg.testAnimation2Skeleton, 1.0 - cg.testModelEntity.backlerp))
		{
			CG_Printf("Can't blend animation2\n");
		}
	}
}

void CG_TestModelNextSkin_f(void)
{
	cg.testModelEntity.skinNum++;
	CG_Printf("skin %i\n", cg.testModelEntity.skinNum);
}

void CG_TestModelPrevSkin_f(void)
{
	cg.testModelEntity.skinNum--;
	if(cg.testModelEntity.skinNum < 0)
	{
		cg.testModelEntity.skinNum = 0;
	}
	CG_Printf("skin %i\n", cg.testModelEntity.skinNum);
}

static void CG_AddTestModel(void)
{
	int             i;

	// re-register the model, because the level may have changed
	cg.testModelEntity.hModel = trap_R_RegisterModel(cg.testModelName);
	if(!cg.testModelEntity.hModel)
	{
		CG_Printf("Can't register model\n");
		return;
	}

	// if testing a gun, set the origin reletive to the view origin
	if(cg.testGun)
	{
		VectorCopy(cg.refdef.vieworg, cg.testModelEntity.origin);
		VectorCopy(cg.refdef.viewaxis[0], cg.testModelEntity.axis[0]);
		VectorCopy(cg.refdef.viewaxis[1], cg.testModelEntity.axis[1]);
		VectorCopy(cg.refdef.viewaxis[2], cg.testModelEntity.axis[2]);

		// allow the position to be adjusted
		for(i = 0; i < 3; i++)
		{
			cg.testModelEntity.origin[i] += cg.refdef.viewaxis[0][i] * cg_gunX.value;
			cg.testModelEntity.origin[i] += cg.refdef.viewaxis[1][i] * cg_gunY.value;
			cg.testModelEntity.origin[i] += cg.refdef.viewaxis[2][i] * cg_gunZ.value;
		}
	}

#if 0
	if(cg.testAnimation)
	{
		int             max = trap_R_AnimNumFrames(cg.testAnimation);

		cg.testModelEntity.oldframe = cg.testModelEntity.frame;

		cg.testModelEntity.frame++;
		if(cg.testModelEntity.frame >= max)
		{
			cg.testModelEntity.frame = 0;
		}

		if(!trap_R_BuildSkeleton(&cg.testModelEntity.skeleton,
								 cg.testAnimation,
								 cg.testModelEntity.oldframe,
								 cg.testModelEntity.frame, 1.0 - cg.testModelEntity.backlerp, qfalse))
		{
			CG_Printf("Can't build animation\n");
		}
	}
#endif

	if(cg.testModelEntity.skeleton.type == SK_RELATIVE)
	{
		// transform relative bones to absolute ones required for vertex skinning
		CG_TransformSkeleton(&cg.testModelEntity.skeleton, NULL);
	}

	trap_R_AddRefEntityToScene(&cg.testModelEntity);
}


/*
=================
CG_TestOmniLight_f

Creates a omni-directional light in front of the current position, which can then be moved around
=================
*/
void CG_TestOmniLight_f(void)
{
	vec3_t          angles;

	memset(&cg.testLight, 0, sizeof(cg.testLight));
	if(trap_Argc() < 2)
	{
		CG_Printf("usage: testOmniLight <lightShaderName>\n");
		return;
	}

	Q_strncpyz(cg.testLightName, CG_Argv(1), sizeof(cg.testLightName));
	cg.testLight.attenuationShader = trap_R_RegisterShaderLightAttenuation(cg.testLightName);

	if(!cg.testLight.attenuationShader)
	{
		CG_Printf("Can't register attenuation shader\n");
		return;
	}

	cg.testLight.rlType = RL_OMNI;
//  cg.testLight.lightfx = LF_ROTATION;

	VectorMA(cg.refdef.vieworg, 100, cg.refdef.viewaxis[0], cg.testLight.origin);

	cg.testLight.color[0] = 1.0;
	cg.testLight.color[1] = 1.0;
	cg.testLight.color[2] = 1.0;

	cg.testLight.radius[0] = 300;
	cg.testLight.radius[1] = 300;
	cg.testLight.radius[2] = 300;

	angles[PITCH] = cg.refdefViewAngles[PITCH];
	angles[YAW] = cg.refdefViewAngles[YAW];	// + 180;
	angles[ROLL] = 0;

	AnglesToQuat(angles, cg.testLight.rotation);

	cg.testFlashLight = qfalse;
}


/*
=================
CG_TestProjLight_f

Creates a projective light in front of the current position, which can then be moved around
=================
*/
void CG_TestProjLight_f(void)
{
	float           fov_x;

	memset(&cg.testLight, 0, sizeof(cg.testLight));
	if(trap_Argc() < 2)
	{
		CG_Printf("usage: testProjLight <lightShaderName>\n");
		return;
	}

	Q_strncpyz(cg.testLightName, CG_Argv(1), sizeof(cg.testLightName));
	cg.testLight.attenuationShader = trap_R_RegisterShaderLightAttenuation(cg.testLightName);

	if(!cg.testLight.attenuationShader)
	{
		CG_Printf("Can't register attenuation shader\n");
		return;
	}

	cg.testLight.rlType = RL_PROJ;
//  cg.testLight.lightfx = LF_ROTATION;

	VectorCopy(cg.refdef.vieworg, cg.testLight.origin);

	cg.testLight.color[0] = 1.0;
	cg.testLight.color[1] = 1.0;
	cg.testLight.color[2] = 1.0;

	QuatClear(cg.testLight.rotation);

	fov_x = tanf(DEG2RAD(cg.refdef.fov_x * 0.5f));
	VectorCopy(cg.refdef.viewaxis[0], cg.testLight.projTarget);
	VectorScale(cg.refdef.viewaxis[1], -fov_x, cg.testLight.projRight);
	VectorScale(cg.refdef.viewaxis[2], fov_x, cg.testLight.projUp);
	VectorScale(cg.refdef.viewaxis[0], 10, cg.testLight.projStart);
	VectorScale(cg.refdef.viewaxis[0], 1000, cg.testLight.projEnd);

	cg.testFlashLight = qfalse;
}

/*
=================
CG_TestFlashLight_f
=================
*/
void CG_TestFlashLight_f(void)
{
	if(trap_Argc() < 2)
	{
		CG_Printf("usage: testFlashLight <lightShaderName>\n");
		return;
	}

	CG_TestProjLight_f();
	cg.testFlashLight = qtrue;
}


static void CG_AddTestLight(void)
{
	float           fov_x;

	// re-register the model, because the level may have changed
	cg.testLight.attenuationShader = trap_R_RegisterShaderLightAttenuation(cg.testLightName);
	if(!cg.testLight.attenuationShader)
	{
		CG_Printf("Can't register attenuation shader\n");
		return;
	}

	// if testing a flashlight, set the projection direction reletive to the view direction
	if(cg.testFlashLight)
	{
		VectorCopy(cg.refdef.vieworg, cg.testLight.origin);

		fov_x = tanf(DEG2RAD(cg.refdef.fov_x * 0.5f));
		VectorCopy(cg.refdef.viewaxis[0], cg.testLight.projTarget);
		VectorScale(cg.refdef.viewaxis[1], -fov_x, cg.testLight.projRight);
		VectorScale(cg.refdef.viewaxis[2], fov_x, cg.testLight.projUp);
		VectorScale(cg.refdef.viewaxis[0], 10, cg.testLight.projStart);
		VectorScale(cg.refdef.viewaxis[0], 1000, cg.testLight.projEnd);
	}

	trap_R_AddRefLightToScene(&cg.testLight);
}


/*
=================
CG_TestGib_f
=================
*/
void CG_TestGib_f(void)
{
	vec3_t          origin;

	// raynorpat: spawn the gibs out in front of the testing player :)
	VectorMA(cg.refdef.vieworg, 100, cg.refdef.viewaxis[0], origin);

	CG_GibPlayer(origin);
//  CG_ParticleBloodCloud(cg.testModelEntity.origin, cg.refdef.viewaxis[0]);
//  CG_BloodPool(cgs.media.bloodSpurtShader, cg.testModelEntity.origin);
}


//============================================================================


/*
=================
CG_CalcVrect

Sets the coordinates of the rendered window
=================
*/
static void CG_CalcVrect(void)
{
	int             size;

	// the intermission should allways be full screen
	if(cg.snap->ps.pm_type == PM_INTERMISSION)
	{
		size = 100;
	}
	else
	{
		// bound normal viewsize
		if(cg_viewsize.integer < 30)
		{
			trap_Cvar_Set("cg_viewsize", "30");
			size = 30;
		}
		else if(cg_viewsize.integer > 100)
		{
			trap_Cvar_Set("cg_viewsize", "100");
			size = 100;
		}
		else
		{
			size = cg_viewsize.integer;
		}
	}

	cg.refdef.width = cgs.glconfig.vidWidth * size / 100;
	cg.refdef.width &= ~1;

	cg.refdef.height = cgs.glconfig.vidHeight * size / 100;
	cg.refdef.height &= ~1;

	cg.refdef.x = (cgs.glconfig.vidWidth - cg.refdef.width) / 2;
	cg.refdef.y = (cgs.glconfig.vidHeight - cg.refdef.height) / 2;
}

//==============================================================================


/*
===============
CG_OffsetThirdPersonView

===============
*/
#define	FOCUS_DISTANCE	512
static void CG_OffsetThirdPersonView(void)
{
	vec3_t          forward, right, up;
	vec3_t          view;
	vec3_t          focusAngles;
	trace_t         trace;
	static vec3_t   mins = { -8, -8, -8 };
	static vec3_t   maxs = { 8, 8, 8 };
	vec3_t          focusPoint;
	float           focusDist;
	float           forwardScale, sideScale;
	vec3_t          surfNormal;

	if(cg.predictedPlayerState.pm_flags & PMF_WALLCLIMBING)
	{
		if(cg.predictedPlayerState.pm_flags & PMF_WALLCLIMBINGCEILING)
			VectorSet(surfNormal, 0.0f, 0.0f, -1.0f);
		else
			VectorCopy(cg.predictedPlayerState.grapplePoint, surfNormal);
	}
	else
		VectorSet(surfNormal, 0.0f, 0.0f, 1.0f);

	VectorMA(cg.refdef.vieworg, cg.predictedPlayerState.viewheight, surfNormal, cg.refdef.vieworg);

	VectorCopy(cg.refdefViewAngles, focusAngles);

	// allow wounded to look around
	if(cg.predictedPlayerState.pm_type == PM_WOUNDED)
	{
		// focus angle used to calculate direction of camera
		// view angle between +-180, divide by 5; between +-36; prevents some weird angles
		focusAngles[PITCH] = AngleNormalize180(focusAngles[PITCH])/5.0f;
		// refdefViewAngles used to calculate position of camera on sphere around origin
		cg.refdefViewAngles[PITCH] = 0;
	}
	// if dead, look at killer
	else if(cg.predictedPlayerState.stats[STAT_HEALTH] <= 0)
	{
		focusAngles[YAW] = cg.predictedPlayerState.stats[STAT_DEAD_YAW];
		cg.refdefViewAngles[YAW] = cg.predictedPlayerState.stats[STAT_DEAD_YAW];
	}

	AngleVectors(focusAngles, forward, NULL, NULL);

	VectorMA(cg.refdef.vieworg, FOCUS_DISTANCE, forward, focusPoint);

	VectorCopy(cg.refdef.vieworg, view);

	VectorMA(view, 12, surfNormal, view);

	//cg.refdefViewAngles[PITCH] *= 0.5;

	AngleVectors(cg.refdefViewAngles, forward, right, up);

	forwardScale = cos(cg_thirdPersonAngle.value / 180 * M_PI);
	sideScale = sin(cg_thirdPersonAngle.value / 180 * M_PI);
	VectorMA(view, -cg_thirdPersonRange.value * forwardScale, forward, view);
	VectorMA(view, -cg_thirdPersonRange.value * sideScale, right, view);

	// trace a ray from the origin to the viewpoint to make sure the view isn't
	// in a solid block.  Use an 8 by 8 block to prevent the view from near clipping anything

	if(!cg_cameraMode.integer)
	{
		CG_Trace(&trace, cg.refdef.vieworg, mins, maxs, view, cg.predictedPlayerState.clientNum, MASK_SOLID);

		if(trace.fraction != 1.0)
		{
			VectorCopy(trace.endpos, view);
			view[2] += (1.0 - trace.fraction) * 32;
			// try another trace to this position, because a tunnel may have the ceiling
			// close enogh that this is poking out

			CG_Trace(&trace, cg.refdef.vieworg, mins, maxs, view, cg.predictedPlayerState.clientNum, MASK_SOLID);
			VectorCopy(trace.endpos, view);
		}
	}

	VectorCopy(view, cg.refdef.vieworg);

	// select pitch to look at focus point from vieword
	VectorSubtract(focusPoint, cg.refdef.vieworg, focusPoint);
#if 0
	if(cg.predictedPlayerState.pm_flags & PMF_WALLCLIMBING)
	{
		focusDist = VectorLength(focusPoint);
	}
	else
#endif
	{
		focusDist = sqrt(focusPoint[0] * focusPoint[0] + focusPoint[1] * focusPoint[1]);
	}
	if(focusDist < 1)
	{
		focusDist = 1;			// should never happen
	}
	cg.refdefViewAngles[PITCH] = -180 / M_PI * atan2(focusPoint[2], focusDist);
	cg.refdefViewAngles[YAW] -= cg_thirdPersonAngle.value;
}


// this causes a compiler bug on mac MrC compiler
static void CG_StepOffset(void)
{
	float           steptime;
	int             timeDelta;
	vec3_t          normal;
	playerState_t  *ps = &cg.predictedPlayerState;

	if(ps->pm_flags & PMF_WALLCLIMBING)
	{
		if(ps->pm_flags & PMF_WALLCLIMBINGCEILING)
			VectorSet(normal, 0.0f, 0.0f, -1.0f);
		else
			VectorCopy(ps->grapplePoint, normal);
	}
	else
		VectorSet(normal, 0.0f, 0.0f, 1.0f);

	steptime = STEP_TIME;

	// smooth out stair climbing
	timeDelta = cg.time - cg.stepTime;
	if(timeDelta < steptime)
	{
		float           stepChange = cg.stepChange * (steptime - timeDelta) / steptime;

		if(ps->pm_flags & PMF_WALLCLIMBING)
			VectorMA(cg.refdef.vieworg, -stepChange, normal, cg.refdef.vieworg);
		else
			cg.refdef.vieworg[2] -= stepChange;
	}
}

/*
===============
CG_OffsetFirstPersonView
===============
*/
static void CG_OffsetFirstPersonView(void)
{
	float          *origin;
	float          *angles;
	float           bob;
	float           ratio;
	float           delta;
	float           speed;
	float           f;
	vec3_t          predictedVelocity;
	int             timeDelta;
	vec3_t          normal;
	playerState_t  *ps = &cg.predictedPlayerState;

	if(ps->pm_flags & PMF_WALLCLIMBING)
	{
		if(ps->pm_flags & PMF_WALLCLIMBINGCEILING)
			VectorSet(normal, 0.0f, 0.0f, -1.0f);
		else
			VectorCopy(ps->grapplePoint, normal);
	}
	else
		VectorSet(normal, 0.0f, 0.0f, 1.0f);


	if(cg.snap->ps.pm_type == PM_INTERMISSION)
	{
		return;
	}

	origin = cg.refdef.vieworg;
	angles = cg.refdefViewAngles;

	// if dead, fix the angle and don't add any kick
	if(cg.snap->ps.stats[STAT_HEALTH] <= 0)
	{
		angles[ROLL] = 40;
		angles[PITCH] = -15;
		angles[YAW] = cg.snap->ps.stats[STAT_DEAD_YAW];
		origin[2] += cg.predictedPlayerState.viewheight;
		return;
	}

	// add angles based on weapon kick
	VectorAdd(angles, cg.kick_angles, angles);

	// add angles based on damage kick
	if(cg.damageTime)
	{
		ratio = cg.time - cg.damageTime;
		if(ratio < DAMAGE_DEFLECT_TIME)
		{
			ratio /= DAMAGE_DEFLECT_TIME;
			angles[PITCH] += ratio * cg.v_dmg_pitch;
			angles[ROLL] += ratio * cg.v_dmg_roll;
		}
		else
		{
			ratio = 1.0 - (ratio - DAMAGE_DEFLECT_TIME) / DAMAGE_RETURN_TIME;
			if(ratio > 0)
			{
				angles[PITCH] += ratio * cg.v_dmg_pitch;
				angles[ROLL] += ratio * cg.v_dmg_roll;
			}
		}
	}

	// add pitch based on fall kick
#if 0
	ratio = (cg.time - cg.landTime) / FALL_TIME;
	if(ratio < 0)
		ratio = 0;
	angles[PITCH] += ratio * cg.fall_value;
#endif

	// add angles based on velocity
	VectorCopy(cg.predictedPlayerState.velocity, predictedVelocity);

	delta = DotProduct(predictedVelocity, cg.refdef.viewaxis[0]);
	angles[PITCH] += delta * cg_runpitch.value;

	delta = DotProduct(predictedVelocity, cg.refdef.viewaxis[1]);
	angles[ROLL] -= delta * cg_runroll.value;

	// add angles based on bob

	// make sure the bob is visible even at low speeds
	speed = cg.xyspeed > 200 ? cg.xyspeed : 200;

	delta = cg.bobfracsin * cg_bobpitch.value * speed;
	if(cg.predictedPlayerState.pm_flags & PMF_DUCKED)
		delta *= 3;				// crouching
	angles[PITCH] += delta;
	delta = cg.bobfracsin * cg_bobroll.value * speed;
	if(cg.predictedPlayerState.pm_flags & PMF_DUCKED)
		delta *= 3;				// crouching accentuates roll
	if(cg.bobcycle & 1)
		delta = -delta;
	angles[ROLL] += delta;

//===================================

	// add view height
	// TA: when wall climbing the viewheight is not straight up
	if(cg.predictedPlayerState.pm_flags & PMF_WALLCLIMBING)
		VectorMA(origin, ps->viewheight, normal, origin);
	else
		origin[2] += cg.predictedPlayerState.viewheight;

	// smooth out duck height changes
	timeDelta = cg.time - cg.duckTime;
	if(timeDelta < DUCK_TIME)
	{
		cg.refdef.vieworg[2] -= cg.duckChange * (DUCK_TIME - timeDelta) / DUCK_TIME;
	}

	// add bob height
	bob = cg.bobfracsin * cg.xyspeed * cg_bobup.value;

	if(bob > 6)
	{
		bob = 6;
	}

	// TA: likewise for bob
	if(cg.predictedPlayerState.pm_flags & PMF_WALLCLIMBING)
		VectorMA(origin, bob, normal, origin);
	else
		origin[2] += bob;


	// add fall height
	delta = cg.time - cg.landTime;

	if(delta < LAND_DEFLECT_TIME)
	{
		f = delta / LAND_DEFLECT_TIME;
		cg.refdef.vieworg[2] += cg.landChange * f;
	}
	else if(delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME)
	{
		delta -= LAND_DEFLECT_TIME;
		f = 1.0 - (delta / LAND_RETURN_TIME);
		cg.refdef.vieworg[2] += cg.landChange * f;
	}

	// add step offset
	CG_StepOffset();

	// add kick offset

	VectorAdd(origin, cg.kick_origin, origin);

	// pivot the eye based on a neck length
#if 0
	{
#define	NECK_LENGTH		8
		vec3_t          forward, up;

		cg.refdef.vieworg[2] -= NECK_LENGTH;
		AngleVectors(cg.refdefViewAngles, forward, NULL, up);
		VectorMA(cg.refdef.vieworg, 3, forward, cg.refdef.vieworg);
		VectorMA(cg.refdef.vieworg, NECK_LENGTH, up, cg.refdef.vieworg);
	}
#endif
}

//======================================================================

void CG_ZoomDown_f(void)
{

	if(cg.osd.input)
	{
		CG_OSDInput();
		return;
	}

	if(cg.zoomed)
	{
		return;
	}
	cg.zoomed = qtrue;
	cg.zoomTime = cg.time;
}

void CG_ZoomUp_f(void)
{
	if(!cg.zoomed)
	{
		return;
	}
	cg.zoomed = qfalse;
	cg.zoomTime = cg.time;
}


/*
====================
CG_CalcFov

Fixed fov at intermissions, otherwise account for fov variable and zooms.
====================
*/
#define	WAVE_AMPLITUDE	1
#define	WAVE_FREQUENCY	0.4

static int CG_CalcFov(void)
{
	float           x;
	float           phase;
	float           v;
	int             contents;
	float           fov_x, fov_y;
	float           zoomFov;
	float           f;
	int             inwater;

	if(cg.predictedPlayerState.pm_type == PM_INTERMISSION)
	{
		// if in intermission, use a fixed value
		fov_x = 90;
	}
	else if(cg.predictedPlayerState.pm_type == PM_SPECTATOR)
	{
		// if a spectator, use a fixed value
		fov_x = 90;
	}
	else
	{
		// user selectable
		if(cgs.dmflags & DF_FIXED_FOV)
		{
			// dmflag to prevent wide fov for all clients
			fov_x = 90;
		}
		else
		{
			fov_x = cg_fov.value;
			if(fov_x < 1)
			{
				fov_x = 1;
			}
			else if(fov_x > 160)
			{
				fov_x = 160;
			}
		}

		// account for zooms
		zoomFov = cg_zoomFov.value;
		if(zoomFov < 1)
		{
			zoomFov = 1;
		}
		else if(zoomFov > 160)
		{
			zoomFov = 160;
		}

		if(cg.zoomed)
		{
			f = (cg.time - cg.zoomTime) / (float)ZOOM_TIME;
			if(f > 1.0)
			{
				fov_x = zoomFov;
			}
			else
			{
				fov_x = fov_x + f * (zoomFov - fov_x);
			}
		}
		else
		{
			f = (cg.time - cg.zoomTime) / (float)ZOOM_TIME;
			if(f > 1.0)
			{
				fov_x = fov_x;
			}
			else
			{
				fov_x = zoomFov + f * (fov_x - zoomFov);
			}
		}
	}

	x = cg.refdef.width / tan(fov_x / 360 * M_PI);
	fov_y = atan2(cg.refdef.height, x);
	fov_y = fov_y * 360 / M_PI;

	// warp if underwater
	contents = CG_PointContents(cg.refdef.vieworg, -1);

	if(contents & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
	{
		phase = cg.time / 1000.0 * WAVE_FREQUENCY * M_PI * 2;
		v = WAVE_AMPLITUDE * sin(phase);
		fov_x += v;
		fov_y -= v;
		inwater = qtrue;
	}
	else
	{
		inwater = qfalse;
	}


	// set it
	cg.refdef.fov_x = fov_x;
	cg.refdef.fov_y = fov_y;

	if(!cg.zoomed)
	{
		cg.zoomSensitivity = 1;
	}
	else
	{
		cg.zoomSensitivity = cg.refdef.fov_y / 75.0;
	}

	return inwater;
}



/*
===============
CG_DamageBlendBlob
===============
*/
static void CG_DamageBlendBlob(void)
{
#if 0
	int             t;
	int             maxTime;
	refEntity_t     ent;

	if(!cg.damageValue)
	{
		return;
	}

	//if (cg.cameraMode) {
	//  return;
	//}

	// ragePro systems can't fade blends, so don't obscure the screen
	if(cgs.glconfig.hardwareType == GLHW_RAGEPRO)
	{
		return;
	}

	maxTime = DAMAGE_TIME;
	t = cg.time - cg.damageTime;
	if(t <= 0 || t >= maxTime)
	{
		return;
	}


	memset(&ent, 0, sizeof(ent));
	ent.reType = RT_SPRITE;
	ent.renderfx = RF_FIRST_PERSON;

	VectorMA(cg.refdef.vieworg, 8, cg.refdef.viewaxis[0], ent.origin);
	VectorMA(ent.origin, cg.damageX * -8, cg.refdef.viewaxis[1], ent.origin);
	VectorMA(ent.origin, cg.damageY * 8, cg.refdef.viewaxis[2], ent.origin);

	ent.radius = cg.damageValue * 3;
	ent.customShader = cgs.media.viewBloodShader;
	ent.shaderRGBA[0] = 255;
	ent.shaderRGBA[1] = 255;
	ent.shaderRGBA[2] = 255;
	ent.shaderRGBA[3] = 200 * (1.0 - ((float)t / maxTime));
	trap_R_AddRefEntityToScene(&ent);
#endif
}

#define NORMAL_HEIGHT 64.0f
#define NORMAL_WIDTH  6.0f

/*
===============
CG_DrawSurfNormal

Draws a vector against
the surface player is looking at
===============
*/
/*static void CG_DrawSurfNormal(void)
{
	trace_t         tr;
	vec3_t          end, temp;
	polyVert_t      normal[4];
	vec4_t          color = { 0.0f, 255.0f, 0.0f, 128.0f };

	VectorMA(cg.refdef.vieworg, 8192, cg.refdef.viewaxis[0], end);

	CG_Trace(&tr, cg.refdef.vieworg, NULL, NULL, end, cg.predictedPlayerState.clientNum, MASK_SOLID);

	VectorCopy(tr.endpos, normal[0].xyz);
	normal[0].st[0] = 0;
	normal[0].st[1] = 0;
	Vector4Copy(color, normal[0].modulate);

	VectorMA(tr.endpos, NORMAL_WIDTH, cg.refdef.viewaxis[1], temp);
	VectorCopy(temp, normal[1].xyz);
	normal[1].st[0] = 0;
	normal[1].st[1] = 1;
	Vector4Copy(color, normal[1].modulate);

	VectorMA(tr.endpos, NORMAL_HEIGHT, tr.plane.normal, temp);
	VectorMA(temp, NORMAL_WIDTH, cg.refdef.viewaxis[1], temp);
	VectorCopy(temp, normal[2].xyz);
	normal[2].st[0] = 1;
	normal[2].st[1] = 1;
	Vector4Copy(color, normal[2].modulate);

	VectorMA(tr.endpos, NORMAL_HEIGHT, tr.plane.normal, temp);
	VectorCopy(temp, normal[3].xyz);
	normal[3].st[0] = 1;
	normal[3].st[1] = 0;
	Vector4Copy(color, normal[3].modulate);

	trap_R_AddPolyToScene(cgs.media.debugPlayerAABB_twoSided, 4, normal);
}*/

/*
===============
CG_addSmoothOp
===============
*/
void CG_addSmoothOp(vec3_t rotAxis, float rotAngle, float timeMod)
{
	int             i;

	//iterate through smooth array
	for(i = 0; i < MAXSMOOTHS; i++)
	{
		//found an unused index in the smooth array
		if(cg.sList[i].time + cg_wallWalkSmoothTime.integer < cg.time)
		{
			//copy to array and stop
			VectorCopy(rotAxis, cg.sList[i].rotAxis);
			cg.sList[i].rotAngle = rotAngle;
			cg.sList[i].time = cg.time;
			cg.sList[i].timeMod = timeMod;
			return;
		}
	}

	//no free indices in the smooth array
}

/*
===============
CG_smoothWWTransitions
===============
*/
static void CG_smoothWWTransitions(playerState_t * ps, const vec3_t in, vec3_t out)
{
	vec3_t          surfNormal, rotAxis, temp;
	vec3_t          refNormal = { 0.0f, 0.0f, 1.0f };
	vec3_t          ceilingNormal = { 0.0f, 0.0f, -1.0f };
	int             i;
	float           stLocal, sFraction, rotAngle;
	float           smoothTime, timeMod;
	qboolean        performed = qfalse;
	vec3_t          inAxis[3], lastAxis[3], outAxis[3];

	if(cg.snap->ps.pm_flags & PMF_FOLLOW)
	{
		VectorCopy(in, out);
		return;
	}

	//set surfNormal
	if(!(ps->pm_flags & PMF_WALLCLIMBINGCEILING))
		VectorCopy(ps->grapplePoint, surfNormal);
	else
		VectorCopy(ceilingNormal, surfNormal);

	AnglesToAxis(in, inAxis);

	//if we are moving from one surface to another smooth the transition
	if(!VectorCompare(surfNormal, cg.lastNormal))
	{
		//if we moving from the ceiling to the floor special case
		//( x product of colinear vectors is undefined)
		if(VectorCompare(ceilingNormal, cg.lastNormal) && VectorCompare(refNormal, surfNormal))
		{
			AngleVectors(in, temp, NULL, NULL);
			ProjectPointOnPlane(rotAxis, temp, refNormal);
			VectorNormalize(rotAxis);
			rotAngle = 180.0f;
			timeMod = 1.5f;
		}
		else
		{
			AnglesToAxis(cg.lastVangles, lastAxis);
			rotAngle = DotProduct(inAxis[0], lastAxis[0]) +
				DotProduct(inAxis[1], lastAxis[1]) + DotProduct(inAxis[2], lastAxis[2]);

			rotAngle = RAD2DEG(acos((rotAngle - 1.0f) / 2.0f));

			CrossProduct(lastAxis[0], inAxis[0], temp);
			VectorCopy(temp, rotAxis);
			CrossProduct(lastAxis[1], inAxis[1], temp);
			VectorAdd(rotAxis, temp, rotAxis);
			CrossProduct(lastAxis[2], inAxis[2], temp);
			VectorAdd(rotAxis, temp, rotAxis);

			VectorNormalize(rotAxis);

			timeMod = 1.0f;
		}

		//add the op
		CG_addSmoothOp(rotAxis, rotAngle, timeMod);
	}

	//iterate through ops
	for(i = MAXSMOOTHS - 1; i >= 0; i--)
	{
		smoothTime = (int)(cg_wallWalkSmoothTime.integer * cg.sList[i].timeMod);

		//if this op has time remaining, perform it
		if(cg.time < cg.sList[i].time + smoothTime)
		{
			stLocal = 1.0f - (((cg.sList[i].time + smoothTime) - cg.time) / smoothTime);
			sFraction = -(cos(stLocal * M_PI) + 1.0f) / 2.0f;

			RotatePointAroundVector(outAxis[0], cg.sList[i].rotAxis, inAxis[0], sFraction * cg.sList[i].rotAngle);
			RotatePointAroundVector(outAxis[1], cg.sList[i].rotAxis, inAxis[1], sFraction * cg.sList[i].rotAngle);
			RotatePointAroundVector(outAxis[2], cg.sList[i].rotAxis, inAxis[2], sFraction * cg.sList[i].rotAngle);

			AxisCopy(outAxis, inAxis);
			performed = qtrue;
		}
	}

	//if we performed any ops then return the smoothed angles
	//otherwise simply return the in angles
	if(performed)
		AxisToAngles(outAxis, out);
	else
		VectorCopy(in, out);

	//copy the current normal to the lastNormal
	VectorCopy(in, cg.lastVangles);
	VectorCopy(surfNormal, cg.lastNormal);
}

/*
===============
CG_smoothWJTransitions
===============
*/
/*static void CG_smoothWJTransitions(playerState_t * ps, const vec3_t in, vec3_t out)
{
	int             i;
	float           stLocal, sFraction;
	qboolean        performed = qfalse;
	vec3_t          inAxis[3], outAxis[3];

	if(cg.snap->ps.pm_flags & PMF_FOLLOW)
	{
		VectorCopy(in, out);
		return;
	}

	AnglesToAxis(in, inAxis);

	//iterate through ops
	for(i = MAXSMOOTHS - 1; i >= 0; i--)
	{
		//if this op has time remaining, perform it
		if(cg.time < cg.sList[i].time + cg_wallWalkSmoothTime.integer)
		{
			stLocal = ((cg.sList[i].time + cg_wallWalkSmoothTime.integer) - cg.time) / cg_wallWalkSmoothTime.integer;
			sFraction = 1.0f - ((cos(stLocal * M_PI * 2.0f) + 1.0f) / 2.0f);

			RotatePointAroundVector(outAxis[0], cg.sList[i].rotAxis, inAxis[0], sFraction * cg.sList[i].rotAngle);
			RotatePointAroundVector(outAxis[1], cg.sList[i].rotAxis, inAxis[1], sFraction * cg.sList[i].rotAngle);
			RotatePointAroundVector(outAxis[2], cg.sList[i].rotAxis, inAxis[2], sFraction * cg.sList[i].rotAngle);

			AxisCopy(outAxis, inAxis);
			performed = qtrue;
		}
	}

	//if we performed any ops then return the smoothed angles
	//otherwise simply return the in angles
	if(performed)
		AxisToAngles(outAxis, out);
	else
		VectorCopy(in, out);
}*/


/*
===============
CG_CalcViewValues

Sets cg.refdef view values
===============
*/
static int CG_CalcViewValues(void)
{
	playerState_t  *ps;

	memset(&cg.refdef, 0, sizeof(cg.refdef));

	// calculate size of 3D view
	CG_CalcVrect();

	ps = &cg.predictedPlayerState;
/*
	if (cg.cameraMode) {
		vec3_t origin, angles;
		if (trap_getCameraInfo(cg.time, &origin, &angles)) {
			VectorCopy(origin, cg.refdef.vieworg);
			angles[ROLL] = 0;
			VectorCopy(angles, cg.refdefViewAngles);
			AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );
			return CG_CalcFov();
		} else {
			cg.cameraMode = qfalse;
		}
	}
*/

#if defined(USE_JAVA)
	{
		VectorCopy(ps->origin, cg.refdef.vieworg);
		VectorCopy(ps->viewangles, cg.refdefViewAngles);
		AnglesToAxis(cg.refdefViewAngles, cg.refdef.viewaxis);

		return CG_CalcFov();
	}
#endif


	// intermission view
	if(ps->pm_type == PM_INTERMISSION)
	{
		VectorCopy(ps->origin, cg.refdef.vieworg);
		VectorCopy(ps->viewangles, cg.refdefViewAngles);
		AnglesToAxis(cg.refdefViewAngles, cg.refdef.viewaxis);

		return CG_CalcFov();
	}

	cg.bobcycle = (ps->bobCycle & 128) >> 7;
	cg.bobfracsin = fabs(sin((ps->bobCycle & 127) / 127.0 * M_PI));
	cg.xyspeed = sqrt(ps->velocity[0] * ps->velocity[0] + ps->velocity[1] * ps->velocity[1]);

	VectorCopy(ps->origin, cg.refdef.vieworg);

	//if(BG_ClassHasAbility(ps->stats[STAT_PCLASS], SCA_WALLCLIMBER))
	{
		CG_smoothWWTransitions(ps, ps->viewangles, cg.refdefViewAngles);
	}
//	else if(BG_ClassHasAbility(ps->stats[STAT_PCLASS], SCA_WALLJUMPER))
//	{
//		CG_smoothWJTransitions(ps, ps->viewangles, cg.refdefViewAngles);
//	}
//	else
//	{
//		VectorCopy(ps->viewangles, cg.refdefViewAngles);
//	}

	// clumsy logic, but it needs to be this way round because the CS propogation
	// delay screws things up otherwise
#if 0
	if(!BG_ClassHasAbility(ps->stats[STAT_PCLASS], SCA_WALLJUMPER))
	{
		if(!(ps->stats[STAT_STATE] & SS_WALLCLIMBING))
			VectorSet(cg.lastNormal, 0.0f, 0.0f, 1.0f);
	}
#endif

	if(cg_cameraOrbit.integer)
	{
		if(cg.time > cg.nextOrbitTime)
		{
			cg.nextOrbitTime = cg.time + cg_cameraOrbitDelay.integer;
			cg_thirdPersonAngle.value += cg_cameraOrbit.value;
		}
	}
	// add error decay
	if(cg_errorDecay.value > 0)
	{
		int             t;
		float           f;

		t = cg.time - cg.predictedErrorTime;
		f = (cg_errorDecay.value - t) / cg_errorDecay.value;

		if(f > 0 && f < 1)
		{
			VectorMA(cg.refdef.vieworg, f, cg.predictedError, cg.refdef.vieworg);
		}
		else
		{
			cg.predictedErrorTime = 0;
		}
	}

	if(cg.renderingThirdPerson)
	{
		// back away from character
		CG_OffsetThirdPersonView();
	}
	else
	{
		// offset for local bobbing and kicks
		CG_OffsetFirstPersonView();
	}

	// position eye reletive to origin
	AnglesToAxis(cg.refdefViewAngles, cg.refdef.viewaxis);

	if(cg.hyperspace)
	{
		cg.refdef.rdflags |= RDF_NOWORLDMODEL | RDF_HYPERSPACE;
	}

	// field of view
	return CG_CalcFov();
}


/*
=====================
CG_PowerupTimerSounds
=====================
*/
static void CG_PowerupTimerSounds(void)
{
	int             i;
	int             t;

	// powerup timers going away
	for(i = 0; i < MAX_POWERUPS; i++)
	{
		t = cg.snap->ps.powerups[i];
		if(t <= cg.time)
		{
			continue;
		}
		if(t - cg.time >= POWERUP_BLINKS * POWERUP_BLINK_TIME)
		{
			continue;
		}
		if((t - cg.time) / POWERUP_BLINK_TIME != (t - cg.oldTime) / POWERUP_BLINK_TIME)
		{
			trap_S_StartSound(NULL, cg.snap->ps.clientNum, CHAN_ITEM, cgs.media.wearOffSound);
		}
	}
}

/*
=====================
CG_AddBufferedSound
=====================
*/
void CG_AddBufferedSound(sfxHandle_t sfx)
{
	if(!sfx)
		return;
	cg.soundBuffer[cg.soundBufferIn] = sfx;
	cg.soundBufferIn = (cg.soundBufferIn + 1) % MAX_SOUNDBUFFER;
	if(cg.soundBufferIn == cg.soundBufferOut)
	{
		cg.soundBufferOut++;
	}
}

/*
=====================
CG_PlayBufferedSounds
=====================
*/
static void CG_PlayBufferedSounds(void)
{
	if(cg.soundTime < cg.time)
	{
		if(cg.soundBufferOut != cg.soundBufferIn && cg.soundBuffer[cg.soundBufferOut])
		{
			trap_S_StartLocalSound(cg.soundBuffer[cg.soundBufferOut], CHAN_ANNOUNCER);
			cg.soundBuffer[cg.soundBufferOut] = 0;
			cg.soundBufferOut = (cg.soundBufferOut + 1) % MAX_SOUNDBUFFER;
			cg.soundTime = cg.time + 750;
		}
	}
}

//=========================================================================

/*
=================
CG_DrawActiveFrame

Generates and draws a game scene and status information at the given time.
=================
*/
void CG_DrawActiveFrame(int serverTime, stereoFrame_t stereoView, qboolean demoPlayback)
{
	int             inwater;

	cg.time = serverTime;

//unlagged - lag simulation #1
	// adjust the clock to reflect latent snaps
	cg.time -= cg_latentSnaps.integer * (1000 / sv_fps.integer);
//unlagged - lag simulation #1

	cg.demoPlayback = demoPlayback;

	// update cvars
	CG_UpdateCvars();

	// if we are only updating the screen as a loading
	// pacifier, don't even try to read snapshots
	if(cg.progress > 0)
	{
		CG_DrawInformation();
		return;
	}

	// any looped sounds will be respecified as entities
	// are added to the render list
	trap_S_ClearLoopingSounds(qfalse);

	// clear all the render lists
	trap_R_ClearScene();

	// set up cg.snap and possibly cg.nextSnap
	CG_ProcessSnapshots();

	// if we haven't received any snapshots yet, all
	// we can draw is the information screen
	if(!cg.snap || (cg.snap->snapFlags & SNAPFLAG_NOT_ACTIVE))
	{
		CG_DrawInformation();
		return;
	}

	// let the client system know what our weapon and zoom settings are
	trap_SetUserCmdValue(cg.weaponSelect, cg.zoomSensitivity);

	// this counter will be bumped for every valid scene we generate
	cg.clientFrame++;

	// update cg.predictedPlayerState
	CG_PredictPlayerState();

	// decide on third person view
	cg.renderingThirdPerson = cg_thirdPerson.integer || (cg.snap->ps.stats[STAT_HEALTH] <= 0);

	// build cg.refdef
	inwater = CG_CalcViewValues();
	if(inwater)
	{
		cg.refdef.rdflags |= RDF_UNDERWATER;
	}

	// first person blend blobs, done after AnglesToAxis
	if(!cg.renderingThirdPerson)
	{
		CG_DamageBlendBlob();
	}

	// build the render lists
	if(!cg.hyperspace)
	{
		CG_AddPacketEntities();	// adter calcViewValues, so predicted player state is correct
		CG_AddMarks();
		CG_AddParticles();
		CG_AddLocalEntities();
	}
	CG_AddViewWeapon(&cg.predictedPlayerState);

	// add buffered sounds
	CG_PlayBufferedSounds();

	// play buffered voice chats
	CG_PlayBufferedVoiceChats();

	// finish up the rest of the refdef
	if(cg.testModelEntity.hModel)
	{
		CG_AddTestModel();
	}

	// Tr3B - test light to preview Doom3 style light attenuation shaders
	if(cg.testLight.attenuationShader)
	{
		CG_AddTestLight();
	}

	cg.refdef.time = cg.time;
	memcpy(cg.refdef.areamask, cg.snap->areamask, sizeof(cg.refdef.areamask));

	// warning sounds when powerup is wearing off
	CG_PowerupTimerSounds();

	// update audio positions
	trap_S_Respatialize(cg.snap->ps.clientNum, cg.refdef.vieworg, cg.refdef.viewaxis, inwater);

	// make sure the lagometerSample and frame timing isn't done twice when in stereo
	if(stereoView != STEREO_RIGHT)
	{
		cg.frametime = cg.time - cg.oldTime;

		if(cg.frametime < 0)
		{
			cg.frametime = 0;
		}
		cg.oldTime = cg.time;
		CG_AddLagometerFrameInfo();
	}

	if(cg_timescale.value != cg_timescaleFadeEnd.value)
	{
		if(cg_timescale.value < cg_timescaleFadeEnd.value)
		{
			cg_timescale.value += cg_timescaleFadeSpeed.value * ((float)cg.frametime) / 1000;
			if(cg_timescale.value > cg_timescaleFadeEnd.value)
				cg_timescale.value = cg_timescaleFadeEnd.value;
		}
		else
		{
			cg_timescale.value -= cg_timescaleFadeSpeed.value * ((float)cg.frametime) / 1000;
			if(cg_timescale.value < cg_timescaleFadeEnd.value)
				cg_timescale.value = cg_timescaleFadeEnd.value;
		}

		if(cg_timescaleFadeSpeed.value)
		{
			trap_Cvar_Set("timescale", va("%f", cg_timescale.value));
		}
	}

	// actually issue the rendering calls
	CG_DrawActive(stereoView);

	if(cg_stats.integer)
	{
		CG_Printf("cg.clientFrame:%i\n", cg.clientFrame);
	}
}
