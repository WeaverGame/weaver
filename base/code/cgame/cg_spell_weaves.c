/*
===========================================================================

===========================================================================
*/

// cg_weaves.c -- events and effects dealing with weaves
#include "cg_local.h"
//#include "cg_weaver_effects.h"

static const vec3_t heldWeaveOffset[HELD_MAX][HELD_MAX] = {
	{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	{{1, 0, 0}, {-1, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	{{1, 0, 0}, {-0.707f, 0.707f, 0}, {-0.707f, -0.707f, 0}, {0, 0, 0}},
	{{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}}
};

int             threadColorR[WVP_NUMBER] = {
	0,							//none
	239,						//air
	239,						//airfire
	239,						//fire
	185,						//earthfire
	131,						//earth
	65,							//earthwater
	0,							//water
	119,						//airwater
	18
};								//spirit

int             threadColorG[WVP_NUMBER] = {
	0,							//none
	230,						//air
	135,						//airfire
	41,							//fire
	51,							//earthfire
	61,							//earth
	71,							//earthwater
	82,							//water
	156,						//airwater
	173
};								//spirit

int             threadColorB[WVP_NUMBER] = {
	0,							//none
	0,							//air
	20,							//airfire
	41,							//fire
	33,							//earthfire
	26,							//earth
	127,						//earthwater
	231,						//water
	115,						//airwater
	42
};								//spirit

void PowerDecode(playerEntity_t *pe, int n, int offset, int count)
{
	int             i = 0;

	for(i = 0; i < count; i++)
	{
		pe->threads[offset + i] = (n / (int)pow(WVP_NUMBER, i)) % WVP_NUMBER;
	}
}

int PowerColor(int threads[MAX_THREADS], vec3_t * color)
{
	int             i = 0;
	int             total = 0;
	int             outColor[3] = { 0, 0, 0 };

	for(i = 0; i < MAX_THREADS; i++)
	{
		if(threads[i] != WVP_NONE)
		{
			//Weight earlier colors
			outColor[0] <<= 1;
			outColor[1] <<= 1;
			outColor[2] <<= 1;
			total <<= 1;
			//Add this color
			outColor[0] += threadColorR[threads[i]];
			outColor[1] += threadColorG[threads[i]];
			outColor[2] += threadColorB[threads[i]];
			total++;
		}
	}

	(*color)[0] = (float)(outColor[0] / total) / 255.0f;
	(*color)[1] = (float)(outColor[1] / total) / 255.0f;
	(*color)[2] = (float)(outColor[2] / total) / 255.0f;
	return total;
}

/*
=================
CG_ShowThreads

Handles thread entities.
These are the entities which specify the threads to show infront
of other players' models, and on this player's hud.
=================
*/
void CG_ShowThreads(centity_t * cent)
{
	playerEntity_t *pe;

	if(cg.predictedPlayerState.clientNum == cent->currentState.otherEntityNum2)
	{
		//Threads belong to this client.
		pe = &cg.predictedPlayerEntity.pe;
	}
	else
	{
		//Threads belong to another player.
		pe = &cg_entities[cent->currentState.otherEntityNum2].pe;
		//Use info to update weave sense dots
		CG_UpdateWeaveSense(cent->currentState.otherEntityNum2, cent->currentState.origin, cent->currentState.powerups);
	}

	PowerDecode(pe, cent->currentState.constantLight, 0, 8);

	//CG_Printf("ThreadsEnt: client=%d raw=%d threads=%d", cent->currentState.otherEntityNum2, cent->currentState.constantLight, pe->threads[0]);
	//CG_Printf(" dec=%d,%d,%d,%d,%d,%d,%d,%d\n", pe->threads[0], pe->threads[1], pe->threads[2], pe->threads[3], pe->threads[4], pe->threads[5], pe->threads[6], pe->threads[7]);

	if(pe->threads[0] == WVP_NONE)
	{
		return;
	}

	pe->threadsLight = PowerColor(pe->threads, &pe->threadsColor);
}

/*
=================
CG_AddPlayerThreads

Adds thread entities to a player model
=================
*/
void CG_AddPlayerThreads(centity_t * player, playerState_t * ps, refEntity_t * parent)
{
	refEntity_t     ent;
	playerEntity_t *pe;
	int             boneIndex;

	pe = &player->pe;

	//CG_Printf("ThreadsEnt: player=%d cg_ents=%d threads=%d threads=%d\n", player, &cg_entities, player->pe.threads[0], pe->threads[0]);

	if(pe->threads[0] == WVP_NONE)
	{
		return;
	}

	//CG_Printf("ThreadsEnt: player=%d raw=%d dec=%d,%d,%d,%d,%d,%d,%d,%d\n", player->currentState.clientNum,
	//          pe->threads[0], pe->threads[1], pe->threads[2], pe->threads[3], pe->threads[4], pe->threads[5], pe->threads[6], pe->threads[7]);

	//initialize render entity
	memset(&ent, 0, sizeof(ent));

#ifdef XPPM
	if(ps)
	{
		CG_PositionEntityOnTag(&ent, parent, parent->hModel, "tag_weapon");
	}
	else
	{
		boneIndex = trap_R_BoneIndex(parent->hModel, "tag_weapon");
		if(boneIndex >= 0 && boneIndex < pe->torso.skeleton.numBones)
		{
			AxisClear(ent.axis);
			CG_PositionRotatedEntityOnBone(&ent, parent, parent->hModel, "tag_weapon");
		}
		else
		{

			boneIndex = trap_R_BoneIndex(parent->hModel, "MG_ATTACHER");
			if(boneIndex >= 0 && boneIndex < pe->torso.skeleton.numBones)
			{
				// HACK: this is bone specific
				vec3_t          angles;

				angles[PITCH] = -90;
				angles[YAW] = 0;
				angles[ROLL] = -90;

				AnglesToAxis(angles, ent.axis);

				CG_PositionRotatedEntityOnBone(&ent, parent, parent->hModel, "MG_ATTACHER");
			}
		}
	}
#else
	CG_PositionEntityOnTag(&ent, parent, parent->hModel, "tag_weapon");
#endif

	if(player->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson)
	{
		ent.renderfx = RF_THIRD_PERSON;	// only show in mirrors
	}
	else
	{
		ent.renderfx = 0;
	}

	ent.customShader = cgs.media.weaverThreads[pe->threads[0]];
	ent.reType = RT_SPRITE;
	ent.radius = 12;
	ent.rotation = cg.time / 20.0;

	//Com_Printf("color: R=%d G=%d B=%d\n", (int)(color[0]*255), (int)(color[1]*255), (int)(color[2]*255));

	//TODO: use light shader instead of this code
	trap_R_AddLightToScene(ent.origin, 50 + pe->threadsLight + sin(cg.time / 20.0), pe->threadsColor[0], pe->threadsColor[1], pe->threadsColor[2]);

	trap_R_AddRefEntityToScene(&ent);
}

/*
=================
CG_AddPlayerHeldWeave

Adds heldweave entities to a player model
=================
*/
void CG_AddPlayerHeldWeave(centity_t * player, playerState_t * ps, refEntity_t * parent)
{
	refEntity_t     ent[HELD_MAX];
	playerEntity_t *pe;
	int             boneIndex;
	int             i;
	weaver_weaveInfo *weaveInfo;
	centity_t      *heldWeave;
	vec3_t          origin;
	int             rf;
	int             count;

	pe = &player->pe;

	//initialize render entity
	memset(&ent[0], 0, sizeof(ent[0]));

#ifdef XPPM
	if(ps)
	{
		CG_PositionEntityOnTag(&ent[0], parent, parent->hModel, "Head");
	}
	else
	{
		boneIndex = trap_R_BoneIndex(parent->hModel, "Head");
		if(boneIndex >= 0 && boneIndex < pe->torso.skeleton.numBones)
		{
			AxisClear(ent[0].axis);
			CG_PositionRotatedEntityOnBone(&ent[0], parent, parent->hModel, "Head");
		}
		else
		{
			CG_Error("No tag found while adding held weave.");
		}
	}
#else
	CG_PositionEntityOnTag(&ent[0], parent, parent->hModel, "tag_weapon");
#endif

	if(player->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson)
	{
		rf = RF_THIRD_PERSON;	// only show in mirrors
	}
	else
	{
		rf = 0;
	}

	// use the same origin for them all
	AxisClear(ent[0].axis);
	VectorCopy(ent[0].origin, origin);
	origin[2] += 15;

	count = 0;
	for(i = 0; i < HELD_MAX; i++)
	{
		if(pe->heldWeaveEnt[i] == 0)
		{
			continue;
		}

		heldWeave = &cg_entities[pe->heldWeaveEnt[i]];
		if((heldWeave == NULL) || (heldWeave->currentValid < 1))
		{
			// this ent number is no longer a held weave
			pe->heldWeaveEnt[i] = 0;
			continue;
		}
		count++;
	}

	for(i = 0; i < HELD_MAX; i++)
	{
		if(pe->heldWeaveEnt[i] == 0)
		{
			continue;
		}

		heldWeave = &cg_entities[pe->heldWeaveEnt[i]];

		memset(&ent[i], 0, sizeof(ent[i]));

		VectorCopy(origin, ent[i].origin);

		VectorMA(ent[i].origin, 10, heldWeaveOffset[count - 1][i], ent[i].origin);

		weaveInfo = &cg_weaves[heldWeave->currentState.weapon];

		ent[i].customShader = weaveInfo->icon;
		ent[i].reType = RT_SPRITE;
		ent[i].radius = 6;
		ent[i].renderfx = rf;

		trap_R_AddRefEntityToScene(&ent[i]);
	}
}

/*
==================
CG_HeldWeave
//WEAVER
==================
*/
void CG_HeldWeave(centity_t * cent)
{
	playerEntity_t *pe;
	int             slot;

	if(cg.predictedPlayerState.clientNum == cent->currentState.otherEntityNum2)
	{
		//Heldweave belong to this client.
		pe = &cg.predictedPlayerEntity.pe;
	}
	else
	{
		//Heldweave belong to another player.
		pe = &cg_entities[cent->currentState.otherEntityNum2].pe;
	}

	slot = cent->currentState.modelindex2 - (MAX_WEAPONS - HELD_MAX);

	pe->heldWeaveEnt[slot] = cent->currentState.number;
}


/*
=================
CG_RegisterWeapon

The server says this item is used on this level
=================
*/
void CG_RegisterWeave(int weaveNum)
{
	weaver_weaveInfo *weaveInfo;
	int             num;

	//gitem_t        *item, *ammo;
	//char            path[MAX_QPATH];

	weaveInfo = &cg_weaves[weaveNum];

	if(weaveNum == 0)
	{
		return;
	}

	if(weaveInfo->registered)
	{
		return;
	}

	memset(weaveInfo, 0, sizeof(*weaveInfo));
	weaveInfo->registered = qtrue;

	/*
	if(!weaveInfo->handsModel)
	{
		weaveInfo->handsModel = trap_R_RegisterModel("models/weapons/shotgun/shotgun_hand.md3");
	}
	*/

	//Defaults
	//weaveInfo->loopFireSound = qfalse;
	weaveInfo->exploLight = 0;
	weaveInfo->exploMarkAlphaFade = qfalse;
	weaveInfo->exploDuration = 0;
	weaveInfo->missileModelCount = 0;

	weaveInfo->missileTrailStep = 50;

	//Set calculatable stuff
	weaveInfo->castCharges = WeaveCharges(weaveNum);
	weaveInfo->tier = WeaveTier(weaveNum);

	switch (weaveNum)
	{
		case WVW_A_AIR_BLAST:
			//weave info
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 0;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_AIR;
			weaveInfo->effectType = WVT_SHOT;
			weaveInfo->group = WVG_AGRESSIVE;
			weaveInfo->name = "Air Blast";

			//misc resources
			weaveInfo->icon = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/airblast/airblast.md5mesh", qtrue);
			weaveInfo->missileLight = 20;
			MAKERGB(weaveInfo->missileLightColor, 0.3f, 0.3f, 1);
			//weaveInfo->missileRenderfx;
			weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav");

			weaveInfo->missileTrailRadius = 80.0f;
			weaveInfo->missileTrailAlpha = 0.05f;
			weaveInfo->missileTrailDuration = 2000;
			weaveInfo->missileTrailShader[0] = cgs.media.smokePuffShader;

			//weaveInfo->exploModel;
			//weaveInfo->exploMark;
			//weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			//weaveInfo->exploSfx = trap_S_RegisterSound("sound/weapons/rocket/rocklx1a.ogg", qfalse);
			//weaveInfo->exploMark = cgs.media.burnMarkShader;
			//weaveInfo->exploMarkRadius = 64;
			//weaveInfo->exploLight = 0;
			//MAKERGB(weaveInfo->exploLightColor, 0.75f, 0.5f, 0.1f);
			//weaveInfo->exploDuration = 0;

			//instance
			//weaveInfo->instanceModel;
			//weaveInfo->instanceLight;
			//weaveInfo->instanceLightColor;
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_A_FIRE_MULTIDARTS:
			//weave info
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 0;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_FIRE;
			weaveInfo->effectType = WVT_SHOT;
			weaveInfo->group = WVG_AGRESSIVE;
			weaveInfo->name = "Fire Multidarts";

			//misc resources
			weaveInfo->icon = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/firedart/firedart.md5mesh", qtrue);
			//weaveInfo->missileLight = 50;
			//MAKERGB(weaveInfo->missileLightColor, 0.5f, 0.1f, 0.1f);
			//weaveInfo->missileRenderfx;
			//weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav", qfalse);

			//weaveInfo->exploModel;
			//weaveInfo->exploMark;
			//weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			//weaveInfo->exploSfx = trap_S_RegisterSound("sound/weapons/rocket/rocklx1a.ogg", qfalse);
			weaveInfo->exploMark = cgs.media.bulletMarkShader;
			weaveInfo->exploMarkRadius = 8;
			//weaveInfo->exploLight = 100;
			//MAKERGB(weaveInfo->exploLightColor, 0.75f, 0.5f, 0.1f);
			//weaveInfo->exploDuration = 800;
			//weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			//weaveInfo->exploSfx = trap_S_RegisterSound("sound/weapons/rocket/rocklx1a.ogg", qfalse);

			//instance
			//weaveInfo->instanceShader1 = trap_R_RegisterShader("");
			//weaveInfo->instanceModel;
			//weaveInfo->instanceLight;
			//weaveInfo->instanceLightColor;
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_A_FIRE_DARTS:
			//weave info
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 0;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_FIRE;
			weaveInfo->effectType = WVT_SHOT;
			weaveInfo->group = WVG_AGRESSIVE;
			weaveInfo->name = "Fire Darts";

			//misc resources
			weaveInfo->icon = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/firedart/firedart.md5mesh", qtrue);
			//weaveInfo->missileLight = 10;
			//MAKERGB(weaveInfo->missileLightColor, 0.5f, 0.1f, 0.1f);
			//weaveInfo->missileRenderfx;
			//weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav");

			//weaveInfo->exploModel;
			//weaveInfo->exploMark;
			//weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			//weaveInfo->exploSfx = trap_S_RegisterSound("sound/weapons/rocket/rocklx1a.ogg");
			weaveInfo->exploMark = cgs.media.bulletMarkShader;
			weaveInfo->exploMarkRadius = 8;
			//weaveInfo->exploLight = 100;
			//MAKERGB(weaveInfo->exploLightColor, 0.75f, 0.5f, 0.1f);
			//weaveInfo->exploDuration = 800;

			//instance
			//weaveInfo->instanceShader1 = trap_R_RegisterShader("");
			//weaveInfo->instanceModel;
			//weaveInfo->instanceLight;
			//weaveInfo->instanceLightColor;
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_D_AIR_PROTECT:
			//weave info
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 0;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_AIR;
			weaveInfo->effectType = WVT_PLAYER_SELF;
			weaveInfo->group = WVG_DEFENSIVE;
			weaveInfo->name = "Air Protect";

			//misc resources
			weaveInfo->icon = trap_R_RegisterShader("rocketExplosion");
			//weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.ogg");

			//missiles
			//weaveInfo->missileModel;
			//weaveInfo->missileLight;
			//weaveInfo->missileLightColor;
			//weaveInfo->missileRenderfx;
			//weaveInfo->missileSound;

			//instance
			//weaveInfo->instanceModel = trap_R_RegisterModel("models/projectiles/spike/spike.md3", qtrue);
			weaveInfo->instanceLight = 250;
			MAKERGB(weaveInfo->instanceLightColor, 1.0f, 0.8f, 0.0f);
			weaveInfo->instanceShader[0] = trap_R_RegisterShader("models/weaves/protect/protect_air");
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_D_FIRE_PROTECT:
			//weave info
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 0;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_FIRE;
			weaveInfo->effectType = WVT_PLAYER_SELF;
			weaveInfo->group = WVG_DEFENSIVE;
			weaveInfo->name = "Fire Protect";

			//misc resources
			weaveInfo->icon = trap_R_RegisterShader("rocketExplosion");
			//weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.ogg");

			//missiles
			//weaveInfo->missileModel;
			//weaveInfo->missileLight;
			//weaveInfo->missileLightColor;
			//weaveInfo->missileRenderfx;
			//weaveInfo->missileSound;
			//weaveInfo->missileTrailFunc;

			//instance
			//weaveInfo->instanceModel = trap_R_RegisterModel("models/projectiles/spike/spike.md3", qtrue);
			weaveInfo->instanceLight = 250;
			MAKERGB(weaveInfo->instanceLightColor, 1, 0, 0);
			weaveInfo->instanceShader[0] = trap_R_RegisterShader("models/weaves/protect/protect_fire");
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_D_EARTH_PROTECT:
			//weave info
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 0;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_EARTH;
			weaveInfo->effectType = WVT_PLAYER_SELF;
			weaveInfo->group = WVG_DEFENSIVE;
			weaveInfo->name = "Earth Protect";

			//misc resources
			weaveInfo->icon = trap_R_RegisterShader("rocketExplosion");
			//weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.ogg");

			//missiles
			//weaveInfo->missileModel;
			//weaveInfo->missileLight;
			//weaveInfo->missileLightColor;
			//weaveInfo->missileRenderfx;
			//weaveInfo->missileSound;
			//weaveInfo->missileTrailFunc;

			//instance
			//weaveInfo->instanceModel = trap_R_RegisterModel("models/projectiles/spike/spike.md3", qtrue);
			weaveInfo->instanceLight = 250;
			MAKERGB(weaveInfo->instanceLightColor, 1, 0, 0);
			weaveInfo->instanceShader[0] = trap_R_RegisterShader("models/weaves/protect/protect_earth");
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_D_WATER_PROTECT:
			//weave info
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 0;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_WATER;
			weaveInfo->effectType = WVT_PLAYER_SELF;
			weaveInfo->group = WVG_DEFENSIVE;
			weaveInfo->name = "Water Protect";

			//misc resources
			weaveInfo->icon = trap_R_RegisterShader("rocketExplosion");
			//weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.ogg");

			//missiles
			//weaveInfo->missileModel;
			//weaveInfo->missileLight;
			//weaveInfo->missileLightColor;
			//weaveInfo->missileRenderfx;
			//weaveInfo->missileSound;
			//weaveInfo->missileTrailFunc;

			//instance
			//weaveInfo->instanceModel = trap_R_RegisterModel("models/projectiles/spike/spike.md3", qtrue);
			weaveInfo->instanceLight = 250;
			MAKERGB(weaveInfo->instanceLightColor, 1, 0, 0);
			weaveInfo->instanceShader[0] = trap_R_RegisterShader("models/weaves/protect/protect_water");
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_A_FIRE_BALL:
			//weave info
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 0;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_FIRE;
			weaveInfo->effectType = WVT_SHOT;
			weaveInfo->group = WVG_AGRESSIVE;
			weaveInfo->name = "Fireball";

			//misc resources
			weaveInfo->icon = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/fireball/fireball.md5mesh", qtrue);
			weaveInfo->missileLight = 250;
			MAKERGB(weaveInfo->missileLightColor, 0.83f, 0.4f, 0.1f);
			//weaveInfo->missileRenderfx;
			weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav");

			weaveInfo->missileTrailRadius = 20.0f;
			weaveInfo->missileTrailAlpha = 0.1f;
			weaveInfo->missileTrailDuration = 150;
			weaveInfo->missileTrailShader[0] = trap_R_RegisterShader("models/weaves/fireball/trail");
			weaveInfo->missileTrailStep = 80;

			//weaveInfo->exploModel;
			//weaveInfo->exploMark;
			weaveInfo->exploRadius = 100.0f;
			weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->exploSfx = trap_S_RegisterSound("sound/weapons/rocket/rocklx1a.ogg");
			weaveInfo->exploMark = cgs.media.burnMarkShader;
			weaveInfo->exploMarkRadius = 100;
			weaveInfo->exploLight = 350;
			MAKERGB(weaveInfo->exploLightColor, 0.95f, 0.4f, 0.1f);
			weaveInfo->exploDuration = 1000;

			//instance
			//weaveInfo->instanceModel = trap_R_RegisterModel("models/projectiles/spike/spike.md3", qtrue);
			//weaveInfo->instanceLight = 250;
			//MAKERGB(weaveInfo->instanceLightColor, 1, 0, 0);
			//weaveInfo->instanceShader1 = trap_R_RegisterShader("models/weaves/protect_fire/protect_fire");
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_A_EARTH_QUAKE_S:
			//weave info
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 0;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_EARTH;
			weaveInfo->effectType = WVT_SHOT;
			weaveInfo->group = WVG_AGRESSIVE;
			weaveInfo->name = "Earthquake Small";

			//misc resources
			weaveInfo->icon = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/projectiles/missile/missile.md3", qtrue);
			weaveInfo->missileLight = 10;
			MAKERGB(weaveInfo->missileLightColor, 0.5f, 0.1f, 0.1f);
			//weaveInfo->missileRenderfx;
			weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav");
			//weaveInfo->missileTrailFunc;

			//weaveInfo->exploModel;
			//weaveInfo->exploMark;
			//weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			//weaveInfo->exploSfx = trap_S_RegisterSound("sound/weapons/rocket/rocklx1a.ogg", qfalse);
			//weaveInfo->exploMark = cgs.media.bulletMarkShader;
			//weaveInfo->exploMarkRadius = 8;
			//weaveInfo->exploLight = 100;
			//MAKERGB(weaveInfo->exploLightColor, 0.75f, 0.5f, 0.1f);
			//weaveInfo->exploDuration = 800;

			//instance
			weaveInfo->instanceModel[0] = trap_R_RegisterModel("models/weaves/earthquake_s/earthquake_s01.md5mesh", qtrue);
			weaveInfo->instanceModel[1] = trap_R_RegisterModel("models/weaves/earthquake_s/earthquake_s02.md5mesh", qtrue);
			weaveInfo->instanceModel[2] = trap_R_RegisterModel("models/weaves/earthquake_s/earthquake_s03.md5mesh", qtrue);
			weaveInfo->instanceModel[3] = trap_R_RegisterModel("models/weaves/earthquake_s/earthquake_s04.md5mesh", qtrue);
			weaveInfo->instanceLight = 800;
			MAKERGB(weaveInfo->instanceLightColor, 0.6f, 0.3f, 0.1f);
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_A_EARTH_QUAKE_M:
			//weave info
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 630;
			weaveInfo->holdMaxTime = 0;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_EARTH;
			weaveInfo->effectType = WVT_SHOT;
			weaveInfo->group = WVG_AGRESSIVE;
			weaveInfo->name = "Earthquake Medium";

			//misc resources
			weaveInfo->icon = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/projectiles/missile/missile.md3", qtrue);
			weaveInfo->missileLight = 10;
			MAKERGB(weaveInfo->missileLightColor, 0.5f, 0.1f, 0.1f);
			//weaveInfo->missileRenderfx;
			weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav");
			//weaveInfo->missileTrailFunc;

			//weaveInfo->exploModel;
			//weaveInfo->exploMark;
			//weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			//weaveInfo->exploSfx = trap_S_RegisterSound("sound/weapons/rocket/rocklx1a.ogg", qfalse);
			//weaveInfo->exploMark = cgs.media.bulletMarkShader;
			//weaveInfo->exploMarkRadius = 8;
			//weaveInfo->exploLight = 100;
			//MAKERGB(weaveInfo->exploLightColor, 0.75f, 0.5f, 0.1f);
			//weaveInfo->exploDuration = 800;

			//instance
			weaveInfo->instanceModel[0] = trap_R_RegisterModel("models/weaves/earthquake_s/earthquake_s01.md5mesh", qtrue);
			weaveInfo->instanceModel[1] = trap_R_RegisterModel("models/weaves/earthquake_s/earthquake_s02.md5mesh", qtrue);
			weaveInfo->instanceModel[2] = trap_R_RegisterModel("models/weaves/earthquake_s/earthquake_s03.md5mesh", qtrue);
			weaveInfo->instanceModel[3] = trap_R_RegisterModel("models/weaves/earthquake_s/earthquake_s04.md5mesh", qtrue);

			weaveInfo->instanceLight = 800;
			MAKERGB(weaveInfo->instanceLightColor, 0.6f, 0.3f, 0.1f);
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_A_EARTH_QUAKE_L:
			//weave info
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 840;
			weaveInfo->holdMaxTime = 0;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_EARTH;
			weaveInfo->effectType = WVT_SHOT;
			weaveInfo->group = WVG_AGRESSIVE;
			weaveInfo->name = "Earthquake Medium";

			//misc resources
			weaveInfo->icon = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/projectiles/missile/missile.md3", qtrue);
			weaveInfo->missileLight = 10;
			MAKERGB(weaveInfo->missileLightColor, 0.5f, 0.1f, 0.1f);
			//weaveInfo->missileRenderfx;
			weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav");
			//weaveInfo->missileTrailFunc;

			//weaveInfo->exploModel;
			//weaveInfo->exploMark;
			//weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			//weaveInfo->exploSfx = trap_S_RegisterSound("sound/weapons/rocket/rocklx1a.ogg", qfalse);
			//weaveInfo->exploMark = cgs.media.bulletMarkShader;
			//weaveInfo->exploMarkRadius = 8;
			//weaveInfo->exploLight = 100;
			//MAKERGB(weaveInfo->exploLightColor, 0.75f, 0.5f, 0.1f);
			//weaveInfo->exploDuration = 800;

			//instance
			weaveInfo->instanceModel[0] = trap_R_RegisterModel("models/weaves/earthquake_s/earthquake_s01.md5mesh", qtrue);
			weaveInfo->instanceModel[1] = trap_R_RegisterModel("models/weaves/earthquake_s/earthquake_s02.md5mesh", qtrue);
			weaveInfo->instanceModel[2] = trap_R_RegisterModel("models/weaves/earthquake_s/earthquake_s03.md5mesh", qtrue);
			weaveInfo->instanceModel[3] = trap_R_RegisterModel("models/weaves/earthquake_s/earthquake_s04.md5mesh", qtrue);

			weaveInfo->instanceLight = 800;
			MAKERGB(weaveInfo->instanceLightColor, 0.6f, 0.3f, 0.1f);
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_A_SPIRIT_SLICE_S:
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 3000;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_SPIRIT;
			weaveInfo->effectType = WVT_SHOT;
			weaveInfo->group = WVG_AGRESSIVE;
			weaveInfo->name = "Small Slice";

			//misc resources
			weaveInfo->icon = trap_R_RegisterShader("models/weaves/slice/icon_slice1");
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/slice/slice1.md3", qtrue);
			weaveInfo->missileLight = 100;
			MAKERGB(weaveInfo->missileLightColor, 0.6f, 0.6f, 0.6f);
			//weaveInfo->missileRenderfx;
			weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav");
			//weaveInfo->missileTrailFunc;

			//instance
			//weaveInfo->instanceModel;
			//weaveInfo->instanceLight;
			//weaveInfo->instanceLightColor;
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_A_SPIRIT_SLICE_M:
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 3000;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_SPIRIT;
			weaveInfo->effectType = WVT_SHOT;
			weaveInfo->group = WVG_AGRESSIVE;
			weaveInfo->name = "Medium Slice";

			//misc resources
			weaveInfo->icon = trap_R_RegisterShader("models/weaves/slice/icon_slice1");
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/slice/slice1.md3", qtrue);
			weaveInfo->missileLight = 100;
			MAKERGB(weaveInfo->missileLightColor, 0.6f, 0.6f, 0.6f);
			//weaveInfo->missileRenderfx;
			weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav");
			//weaveInfo->missileTrailFunc;

			//instance
			//weaveInfo->instanceModel;
			//weaveInfo->instanceLight;
			//weaveInfo->instanceLightColor;
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_A_SPIRIT_SLICE_L:
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 3000;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_SPIRIT;
			weaveInfo->effectType = WVT_SHOT;
			weaveInfo->group = WVG_AGRESSIVE;
			weaveInfo->name = "Large Slice";

			//misc resources
			weaveInfo->icon = trap_R_RegisterShader("models/weaves/slice/icon_slice1");
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/slice/slice1.md3", qtrue);
			weaveInfo->missileLight = 100;
			MAKERGB(weaveInfo->missileLightColor, 0.6f, 0.6f, 0.6f);
			//weaveInfo->missileRenderfx;
			weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav");
			//weaveInfo->missileTrailFunc;

			//instance
			//weaveInfo->instanceModel;
			//weaveInfo->instanceLight;
			//weaveInfo->instanceLightColor;
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_A_SPIRIT_SHIELD:
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 20000;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_SPIRIT;
			weaveInfo->effectType = WVT_PLAYER_ENEMY;
			weaveInfo->group = WVG_AGRESSIVE;
			weaveInfo->name = "Shield";

			//misc resources
			weaveInfo->icon = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.ogg");

			//missiles
			//weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/slice/slice1.md3", qtrue);
			//weaveInfo->missileLight = 100;
			//MAKERGB(weaveInfo->missileLightColor, 0.6f, 0.6f, 0.6f);
			//weaveInfo->missileRenderfx;
			//weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav", qfalse);
			//weaveInfo->missileTrailFunc;

			//instance
			//weaveInfo->instanceModel;
			//weaveInfo->instanceLight;
			//weaveInfo->instanceLightColor;
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_D_SPIRIT_LINK:
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 3000;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_SPIRIT;
			weaveInfo->effectType = WVT_PLAYER_ALLY;
			weaveInfo->group = WVG_DEFENSIVE;
			weaveInfo->name = "Link";

			//misc resources
			weaveInfo->icon = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.ogg");

			//missiles
			//weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/slice/slice1.md3", qtrue);
			//weaveInfo->missileLight = 100;
			//MAKERGB(weaveInfo->missileLightColor, 0.6f, 0.6f, 0.6f);
			//weaveInfo->missileRenderfx;
			//weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav", qfalse);
			//weaveInfo->missileTrailFunc;

			//instance
			//weaveInfo->instanceModel;
			//weaveInfo->instanceLight;
			MAKERGB(weaveInfo->instanceLightColor, 0.8f, 0.8f, 1.0f);
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			weaveInfo->instanceShader[0] = trap_R_RegisterShader("railCore");
			break;
		case WVW_A_WATER_ICESHARDS_S:
			//weave info
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 0;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_WATER;
			weaveInfo->effectType = WVT_SHOT;
			weaveInfo->group = WVG_AGRESSIVE;
			weaveInfo->name = "Ice Shards";

			//misc resources
			weaveInfo->icon = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/iceshards/iceshard01.md5mesh", qtrue);
			weaveInfo->missileModel[1] = trap_R_RegisterModel("models/weaves/iceshards/iceshard02.md5mesh", qtrue);
			weaveInfo->missileModel[2] = trap_R_RegisterModel("models/weaves/iceshards/iceshard03.md5mesh", qtrue);
			weaveInfo->missileModel[3] = trap_R_RegisterModel("models/weaves/iceshards/iceshard04.md5mesh", qtrue);
			weaveInfo->missileLight = 50;
			MAKERGB(weaveInfo->missileLightColor, 0.1f, 0.1f, 0.3f);
			//weaveInfo->missileRenderfx;
			weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav");
			//weaveInfo->missileTrailFunc;

			//weaveInfo->exploModel;
			//weaveInfo->exploMark;
			//weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			//weaveInfo->exploSfx = trap_S_RegisterSound("sound/weapons/rocket/rocklx1a.ogg", qfalse);
			weaveInfo->exploMark = cgs.media.bulletMarkShader;
			weaveInfo->exploMarkRadius = 8;
			weaveInfo->exploLight = 100;
			MAKERGB(weaveInfo->exploLightColor, 0.1f, 0.25f, 0.75f);
			weaveInfo->exploDuration = 800;

			//instance
			//weaveInfo->instanceModel;
			//weaveInfo->instanceLight;
			//weaveInfo->instanceLightColor;
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_A_WATER_ICESHARDS_M:
			//weave info
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 0;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_WATER;
			weaveInfo->effectType = WVT_SHOT;
			weaveInfo->group = WVG_AGRESSIVE;
			weaveInfo->name = "Multi Ice Shards";

			//misc resources
			weaveInfo->icon = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/weaves/iceshards/iceshard01.md5mesh", qtrue);
			weaveInfo->missileModel[1] = trap_R_RegisterModel("models/weaves/iceshards/iceshard02.md5mesh", qtrue);
			weaveInfo->missileModel[2] = trap_R_RegisterModel("models/weaves/iceshards/iceshard03.md5mesh", qtrue);
			weaveInfo->missileModel[3] = trap_R_RegisterModel("models/weaves/iceshards/iceshard04.md5mesh", qtrue);
			weaveInfo->missileLight = 50;
			MAKERGB(weaveInfo->missileLightColor, 0.1f, 0.1f, 0.3f);
			//weaveInfo->missileRenderfx;
			weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav");
			//weaveInfo->missileTrailFunc;

			//weaveInfo->exploModel;
			//weaveInfo->exploMark;
			//weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			//weaveInfo->exploSfx = trap_S_RegisterSound("sound/weapons/rocket/rocklx1a.ogg", qfalse);
			weaveInfo->exploMark = cgs.media.bulletMarkShader;
			weaveInfo->exploMarkRadius = 8;
			weaveInfo->exploLight = 100;
			MAKERGB(weaveInfo->exploLightColor, 0.1f, 0.25f, 0.75f);
			weaveInfo->exploDuration = 800;

			//instance
			//weaveInfo->instanceModel;
			//weaveInfo->instanceLight;
			//weaveInfo->instanceLightColor;
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;
		case WVW_D_AIRFIRE_LIGHT:
			//weave info
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 0;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_AIRFIRE;
			weaveInfo->effectType = WVT_PLAYER_SELF;
			weaveInfo->group = WVG_DEFENSIVE;
			weaveInfo->name = "Light Source";

			//misc resources
			weaveInfo->icon = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.ogg");

			weaveInfo->instanceShader[0] = trap_R_RegisterShader("lightningBolt");;
			//weaveInfo->instanceModel = trap_R_RegisterModel("models/projectiles/spike/spike.md3", qtrue);
			weaveInfo->instanceLight = 400;
			MAKERGB(weaveInfo->instanceLightColor, 1, 1, 1);

			break;
		case WVW_A_EARTHWATER_SLOW:
			//weave info
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 0;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_EARTHWATER;
			weaveInfo->effectType = WVT_SHOT;
			weaveInfo->group = WVG_AGRESSIVE;
			weaveInfo->name = "Slow Poison";

			//misc resources
			weaveInfo->icon = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/projectiles/spike/spike.md3", qtrue);
			weaveInfo->missileLight = 50;
			MAKERGB(weaveInfo->missileLightColor, 0.0f, 0.0f, 0.0f);
			//weaveInfo->missileRenderfx;
			weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav");
			//weaveInfo->missileTrailFunc;

			//weaveInfo->exploModel;
			//weaveInfo->exploMark;
			//weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			//weaveInfo->exploSfx = trap_S_RegisterSound("sound/weapons/rocket/rocklx1a.ogg", qfalse);
			weaveInfo->exploMark = cgs.media.bulletMarkShader;
			weaveInfo->exploMarkRadius = 8;
			weaveInfo->exploLight = 100;
			MAKERGB(weaveInfo->exploLightColor, 0.5f, 0.0f, 0.0f);
			weaveInfo->exploDuration = 800;

			//instance
			//weaveInfo->instanceModel;
			//weaveInfo->instanceLight;
			//weaveInfo->instanceLightColor;
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;

			break;
		case WVW_A_EARTHWATER_POISON:
			//weave info
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 0;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_EARTHWATER;
			weaveInfo->effectType = WVT_SHOT;
			weaveInfo->group = WVG_AGRESSIVE;
			weaveInfo->name = "Poison";

			//misc resources
			weaveInfo->icon = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->firingSound = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.ogg");

			//missiles
			weaveInfo->missileModel[0] = trap_R_RegisterModel("models/projectiles/spike/spike.md3", qtrue);
			weaveInfo->missileLight = 50;
			MAKERGB(weaveInfo->missileLightColor, 0.0f, 0.0f, 0.0f);
			//weaveInfo->missileRenderfx;
			weaveInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav");
			//weaveInfo->missileTrailFunc;

			//weaveInfo->exploModel;
			//weaveInfo->exploMark;
			//weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			//weaveInfo->exploSfx = trap_S_RegisterSound("sound/weapons/rocket/rocklx1a.ogg", qfalse);
			weaveInfo->exploMark = cgs.media.bulletMarkShader;
			weaveInfo->exploMarkRadius = 8;
			weaveInfo->exploLight = 100;
			MAKERGB(weaveInfo->exploLightColor, 0.5f, 0.0f, 0.0f);
			weaveInfo->exploDuration = 800;

			//instance
			//weaveInfo->instanceModel;
			//weaveInfo->instanceLight;
			//weaveInfo->instanceLightColor;
			//weaveInfo->instanceRenderfx;
			//weaveInfo->instanceSound;
			break;

		case WVW_D_WATER_CURE:
			//weave info
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 0;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_WATER;
			weaveInfo->effectType = WVT_PLAYER_ALLY;
			weaveInfo->group = WVG_DEFENSIVE;
			weaveInfo->name = "Cure";
			break;

		case WVW_A_AIRWATER_RIP:
			//weave.info
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 3000;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_AIRWATER;
			weaveInfo->effectType = WVT_PLAYER_ENEMY;
			weaveInfo->group = WVG_AGRESSIVE;
			weaveInfo->name = "Air Rip";
			break;

		case WVW_A_AIR_GRABPLAYER:
			//weave.info
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 0;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_AIR;
			weaveInfo->effectType = (WVT_PLAYER_ENEMY | WVT_PLAYER_ALLY);
			weaveInfo->group = WVG_AGRESSIVE;
			weaveInfo->name = "Grab Player";
			break;

		case WVW_D_WATER_HEAL_S:
			//weave.info
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 0;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_WATER;
			weaveInfo->effectType = WVT_PLAYER_ALLY;
			weaveInfo->group = WVG_DEFENSIVE;
			weaveInfo->name = "Minor Heal";
			break;

		case WVW_D_WATER_HEAL_M:
			//weave.info
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 0;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_WATER;
			weaveInfo->effectType = WVT_PLAYER_ALLY;
			weaveInfo->group = WVG_DEFENSIVE;
			weaveInfo->name = "Heal";
			break;

		case WVW_D_SPIRIT_STAMINA:
			//weave.info
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 3000;
			weaveInfo->castDelay = 0;
			weaveInfo->primaryPower = WVP_SPIRIT;
			weaveInfo->effectType = WVT_PLAYER_ALLY;
			weaveInfo->group = WVG_DEFENSIVE;
			weaveInfo->name = "Stamina";
			break;

		case WVW_A_AIRFIRE_LIGHTNING:
			//weave.info
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 0;
			weaveInfo->castDelay = 1000;
			weaveInfo->primaryPower = WVP_AIRFIRE;
			weaveInfo->effectType = WVT_SHOT;
			weaveInfo->group = WVG_AGRESSIVE;
			weaveInfo->name = "Lightning";

			weaveInfo->instanceLight = 600;
			MAKERGB(weaveInfo->instanceLightColor, 0.8f, 0.8f, 1.0f);

			//instance
			//qhandle_t       instanceModel[MAX_WEAVE_INSTANCE_MODELS];
			//float           instanceLight;
			//vec3_t          instanceLightColor;
			//int             instanceRenderfx;
			//sfxHandle_t     instanceSound;
			//qhandle_t       instanceShader[1];

			break;

		case WVW_D_EARTHFIRE_EXPLOSIVE_S:
			//weave.info
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 0;
			weaveInfo->castDelay = 1000;
			weaveInfo->primaryPower = WVP_EARTHFIRE;
			weaveInfo->effectType = WVT_SPAWN_ENTITY;
			weaveInfo->group = WVG_DEFENSIVE;
			weaveInfo->name = "Small Explosive";

			weaveInfo->instanceModel[0] = trap_R_RegisterModel("models/projectiles/rocket/rocket.md3", qtrue);
			weaveInfo->instanceLight = 70;
			MAKERGB(weaveInfo->instanceLightColor, 0.5f, 0.1f, 0.2f);
			
			weaveInfo->exploRadius = 65.0f;
			weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->exploSfx = trap_S_RegisterSound("sound/weapons/rocket/rocklx1a.ogg");
			weaveInfo->exploMark = cgs.media.burnMarkShader;
			weaveInfo->exploMarkRadius = 100;
			weaveInfo->exploLight = 350;
			MAKERGB(weaveInfo->exploLightColor, 0.95f, 0.4f, 0.1f);
			weaveInfo->exploDuration = 1000;

			break;

		case WVW_D_EARTHFIRE_EXPLOSIVE_M:
			//weave.info
			weaveInfo->holdable = qtrue;
			weaveInfo->holdPowerCharge = 200;
			weaveInfo->holdMaxTime = 0;
			weaveInfo->castDelay = 1000;
			weaveInfo->primaryPower = WVP_EARTHFIRE;
			weaveInfo->effectType = WVT_SHOT;
			weaveInfo->group = WVG_DEFENSIVE;
			weaveInfo->name = "Medium Explosive";

			weaveInfo->instanceModel[0] = trap_R_RegisterModel("models/projectiles/rocket/rocket.md3", qtrue);
			weaveInfo->instanceLight = 100;
			MAKERGB(weaveInfo->instanceLightColor, 0.8f, 0.1f, 0.2f);
			
			weaveInfo->exploRadius = 70.0f;
			weaveInfo->exploSprite = trap_R_RegisterShader("rocketExplosion");
			weaveInfo->exploSfx = trap_S_RegisterSound("sound/weapons/rocket/rocklx1a.ogg");
			weaveInfo->exploMark = cgs.media.burnMarkShader;
			weaveInfo->exploMarkRadius = 100;
			weaveInfo->exploLight = 350;
			MAKERGB(weaveInfo->exploLightColor, 0.95f, 0.4f, 0.1f);
			weaveInfo->exploDuration = 1000;

			break;

		default:
			break;
	}

	for(num = 0; num < MAX_WEAVE_MISSILE_MODELS; num++)
	{
		if(weaveInfo->missileModel[num])
		{
			weaveInfo->missileModelCount++;
		}
		else
		{
			break;
		}
	}
	Com_Printf("Loading weave %d, missile count %d\n", weaveNum, weaveInfo->missileModelCount);
}