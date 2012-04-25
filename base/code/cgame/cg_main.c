/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
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

// cg_main.c -- initialization and primary entry point for cgame
#include "cg_local.h"


int             forceModelModificationCount = -1;
int             forceBrightSkinsModificationCount = -1;

void            CG_Init(int serverMessageNum, int serverCommandSequence, int clientNum);
void            CG_Shutdown(void);


/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
intptr_t vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9,
				int arg10, int arg11)
{

	switch (command)
	{
		case CG_INIT:
			CG_Init(arg0, arg1, arg2);
			return 0;

		case CG_SHUTDOWN:
			CG_Shutdown();
			return 0;

		case CG_CONSOLE_COMMAND:
			return CG_ConsoleCommand();

		case CG_DRAW_ACTIVE_FRAME:
			CG_DrawActiveFrame(arg0, arg1, arg2);
			return 0;

		case CG_CROSSHAIR_PLAYER:
			return CG_CrosshairPlayer();

		case CG_LAST_ATTACKER:
			return CG_LastAttacker();

		case CG_KEY_EVENT:
			CG_KeyEvent(arg0, arg1);
			return 0;

		case CG_MOUSE_EVENT:
			CG_MouseEvent(arg0, arg1);
			return 0;

		case CG_EVENT_HANDLING:
			CG_EventHandling(arg0);
			return 0;

		default:
			CG_Error("vmMain: unknown command %i", command);
			break;
	}
	return -1;
}


cg_t            cg;
cgs_t           cgs;
centity_t       cg_entities[MAX_GENTITIES];
weaponInfo_t    cg_weapons[MAX_WEAPONS];
weaver_weaveCGInfo cg_weaves[WVW_NUM_WEAVES];
itemInfo_t      cg_items[MAX_ITEMS];


vmCvar_t        cg_railTrailTime;
vmCvar_t        cg_centertime;
vmCvar_t        cg_runpitch;
vmCvar_t        cg_runroll;
vmCvar_t        cg_bobup;
vmCvar_t        cg_bobpitch;
vmCvar_t        cg_bobroll;
vmCvar_t        cg_swingSpeed;
vmCvar_t        cg_shadows;
vmCvar_t        cg_precomputedLighting;
vmCvar_t        cg_gibs;
vmCvar_t        cg_drawTimer;
vmCvar_t        cg_drawFPS;
vmCvar_t        cg_drawSnapshot;
vmCvar_t        cg_draw3dIcons;
vmCvar_t        cg_drawIcons;
vmCvar_t        cg_drawAmmoWarning;
vmCvar_t        cg_drawRewards;

vmCvar_t        cg_hudRed;
vmCvar_t        cg_hudGreen;
vmCvar_t        cg_hudBlue;
vmCvar_t        cg_hudAlpha;

vmCvar_t        cg_drawCrosshair;
vmCvar_t        cg_drawCrosshairNames;
vmCvar_t        cg_crosshairSize;
vmCvar_t        cg_crosshairX;
vmCvar_t        cg_crosshairY;
vmCvar_t        cg_crosshairHealth;
vmCvar_t        cg_crosshairPulse;

vmCvar_t        cg_draw2D;
vmCvar_t        cg_debugHUD;
vmCvar_t        cg_drawStatus;
vmCvar_t        cg_drawStatusLines;
vmCvar_t        cg_drawSideBar;
vmCvar_t        cg_drawPickupItem;
vmCvar_t        cg_drawWeaponSelect;
vmCvar_t        cg_animSpeed;
vmCvar_t        cg_animBlend;
vmCvar_t        cg_debugPlayerAnim;
vmCvar_t        cg_debugWeaponAnim;
vmCvar_t        cg_debugPosition;
vmCvar_t        cg_debugEvents;
vmCvar_t        cg_errorDecay;
vmCvar_t        cg_nopredict;
vmCvar_t        cg_noPlayerAnims;
vmCvar_t        cg_showmiss;
vmCvar_t        cg_footsteps;
vmCvar_t        cg_addMarks;
vmCvar_t        cg_brassTime;
vmCvar_t        cg_viewsize;
vmCvar_t        cg_drawGun;
vmCvar_t        cg_gun_frame;
vmCvar_t        cg_gunX;
vmCvar_t        cg_gunY;
vmCvar_t        cg_gunZ;
vmCvar_t        cg_tracerChance;
vmCvar_t        cg_tracerWidth;
vmCvar_t        cg_tracerLength;
vmCvar_t        cg_autoswitch;
vmCvar_t        cg_ignore;
vmCvar_t        cg_simpleItems;
vmCvar_t        cg_fov;
vmCvar_t        cg_zoomFov;
vmCvar_t        cg_thirdPerson;
vmCvar_t        cg_thirdPersonRange;
vmCvar_t        cg_thirdPersonAngle;
vmCvar_t        cg_stereoSeparation;
vmCvar_t        cg_lagometer;
vmCvar_t        cg_drawAttacker;
vmCvar_t        cg_synchronousClients;
vmCvar_t        cg_notifyTime;
vmCvar_t        cg_notifyHeight;
vmCvar_t        cg_chatTime;
vmCvar_t        cg_chatHeight;
vmCvar_t        cg_teamChatTime;
vmCvar_t        cg_teamChatHeight;
vmCvar_t        cg_stats;
vmCvar_t        cg_buildScript;
vmCvar_t        cg_forceModel;
vmCvar_t        cg_forceBrightSkins;
vmCvar_t        cg_blood;
vmCvar_t        cg_predictItems;
vmCvar_t        cg_deferPlayers;
vmCvar_t        cg_drawTeamOverlay;
vmCvar_t        cg_teamOverlayUserinfo;
vmCvar_t        cg_drawFriend;
vmCvar_t        cg_teamChatsOnly;
vmCvar_t        cg_noVoiceChats;
vmCvar_t        cg_noVoiceText;
vmCvar_t        cg_hudFiles;
vmCvar_t        cg_scorePlum;

//unlagged - smooth clients #2
// this is done server-side now
//vmCvar_t  cg_smoothClients;
//unlagged - smooth clients #2
vmCvar_t        cg_pmove_msec;
vmCvar_t        cg_cameraMode;
vmCvar_t        cg_cameraOrbit;
vmCvar_t        cg_cameraOrbitDelay;
vmCvar_t        cg_timescaleFadeEnd;
vmCvar_t        cg_timescaleFadeSpeed;
vmCvar_t        cg_timescale;
vmCvar_t        cg_noTaunt;
vmCvar_t        cg_noProjectileTrail;
vmCvar_t        cg_railType;
vmCvar_t        cg_trueLightning;

vmCvar_t        cg_particles;
vmCvar_t        cg_particleCollision;

// these cvars are shared accross both games
vmCvar_t        pm_airControl;
vmCvar_t        pm_fastWeaponSwitches;
vmCvar_t        pm_fixedPmove;
vmCvar_t        pm_fixedPmoveFPS;

vmCvar_t        cg_gravityZ;

//weaver cvars
vmCvar_t        cg_drawWeaverDisc;
vmCvar_t        cg_drawWeaverThread;
vmCvar_t        cg_weaverTutorial;
vmCvar_t        cg_weaverTutorialMaxElement;
vmCvar_t        cg_weaverTutorialMaxDepth;
vmCvar_t        cg_tutorial;
vmCvar_t        cg_weaverDiscSize;
vmCvar_t        cg_weaverDiscX;
vmCvar_t        cg_weaverDiscY;
vmCvar_t        cg_weaverSenseRadius;
vmCvar_t        cg_weaverDiscMoveScale;
vmCvar_t        cg_weaverThreadMoveScale;
vmCvar_t        cg_weaverSpellSwitch;

vmCvar_t        cg_currentSelectedPlayer;
vmCvar_t        cg_currentSelectedPlayerName;
vmCvar_t        cg_singlePlayer;
vmCvar_t        cg_singlePlayerActive;
vmCvar_t        cg_enableDust;
vmCvar_t        cg_enableBreath;
vmCvar_t        cg_obeliskRespawnDelay;

vmCvar_t        cg_wallWalkSmoothTime;
vmCvar_t        cg_drawPlayerCollision;

//unlagged - client options
vmCvar_t        cg_delag;
vmCvar_t        cg_debugDelag;
vmCvar_t        cg_drawBBox;
vmCvar_t        cg_cmdTimeNudge;
vmCvar_t        sv_fps;
vmCvar_t        cg_projectileNudge;
vmCvar_t        cg_optimizePrediction;
vmCvar_t        cl_timeNudge;
vmCvar_t        cg_latentSnaps;
vmCvar_t        cg_latentCmds;
vmCvar_t        cg_plOut;
//unlagged - client options

typedef struct
{
	vmCvar_t       *vmCvar;
	char           *cvarName;
	char           *defaultString;
	int             cvarFlags;
} cvarTable_t;

static cvarTable_t cvarTable[] = {	// bk001129
	{&cg_ignore, "cg_ignore", "0", 0},	// used for debugging
	{&cg_autoswitch, "cg_autoswitch", "1", CVAR_ARCHIVE},
	{&cg_drawGun, "cg_drawGun", "1", CVAR_ARCHIVE},
	{&cg_zoomFov, "cg_zoomfov", "22.5", CVAR_ARCHIVE},
	{&cg_fov, "cg_fov", "90", CVAR_ARCHIVE},
	{&cg_viewsize, "cg_viewsize", "100", CVAR_ARCHIVE},
	{&cg_stereoSeparation, "cg_stereoSeparation", "0.4", CVAR_ARCHIVE},
	{&cg_shadows, "cg_shadows", "1", 0},
	{&cg_precomputedLighting, "r_precomputedLighting", "0", 0},
	{&cg_gibs, "cg_gibs", "1", CVAR_ARCHIVE},
	{&cg_draw2D, "cg_draw2D", "1", CVAR_ARCHIVE},
	{&cg_debugHUD, "cg_debugHUD", "0", CVAR_ARCHIVE},
	{&cg_drawStatus, "cg_drawStatus", "3", CVAR_CHEAT},
	{&cg_drawStatusLines, "cg_drawStatusLines", "1", CVAR_ARCHIVE},
	{&cg_drawSideBar, "cg_drawSideBar", "0", CVAR_ARCHIVE},
	{&cg_drawPickupItem, "cg_drawPickupItem", "1", CVAR_ARCHIVE},
	{&cg_drawWeaponSelect, "cg_drawWeaponSelect", "1", CVAR_ARCHIVE},
	{&cg_drawTimer, "cg_drawTimer", "1", CVAR_ARCHIVE},
	{&cg_drawFPS, "cg_drawFPS", "0", CVAR_ARCHIVE},
	{&cg_drawSnapshot, "cg_drawSnapshot", "0", CVAR_ARCHIVE},
	{&cg_draw3dIcons, "cg_draw3dIcons", "0", CVAR_ARCHIVE},
	{&cg_drawIcons, "cg_drawIcons", "1", CVAR_ARCHIVE},
	{&cg_drawAmmoWarning, "cg_drawAmmoWarning", "1", CVAR_ARCHIVE},
	{&cg_drawAttacker, "cg_drawAttacker", "1", CVAR_ARCHIVE},
	{&cg_drawRewards, "cg_drawRewards", "1", CVAR_ARCHIVE},

	//hud stuff

	{&cg_hudRed, "cg_hudRed", "0.6", CVAR_ARCHIVE},
	{&cg_hudGreen, "cg_hudGreen", "0.9", CVAR_ARCHIVE},
	{&cg_hudBlue, "cg_hudBlue", "1.0", CVAR_ARCHIVE},
	{&cg_hudAlpha, "cg_hudAlpha", "1.0", CVAR_ARCHIVE},

	// generic crosshair stuff
	{&cg_drawCrosshair, "cg_drawCrosshair", "1", CVAR_ARCHIVE},
	{&cg_drawCrosshairNames, "cg_crosshairNames", "1", CVAR_ARCHIVE},
	{&cg_crosshairSize, "cg_crosshairSize", "24", CVAR_ARCHIVE},
	{&cg_crosshairHealth, "cg_crosshairHealth", "1", CVAR_ARCHIVE},
	{&cg_crosshairX, "cg_crosshairX", "0", CVAR_ARCHIVE},
	{&cg_crosshairY, "cg_crosshairY", "0", CVAR_ARCHIVE},
	{&cg_crosshairPulse, "cg_crosshairPulse", "1", CVAR_ARCHIVE},	// pulse crosshair when picking up items

	{&cg_brassTime, "cg_brassTime", "2500", CVAR_ARCHIVE},
	{&cg_simpleItems, "cg_simpleItems", "0", CVAR_ARCHIVE},
	{&cg_addMarks, "cg_marks", "1", CVAR_ARCHIVE},
	{&cg_lagometer, "cg_lagometer", "1", CVAR_ARCHIVE},
	{&cg_railTrailTime, "cg_railTrailTime", "1400", CVAR_ARCHIVE},
	{&cg_gunX, "cg_gunX", "0", CVAR_CHEAT},
	{&cg_gunY, "cg_gunY", "0", CVAR_CHEAT},
	{&cg_gunZ, "cg_gunZ", "0", CVAR_CHEAT},
	{&cg_centertime, "cg_centertime", "3", CVAR_CHEAT},
	{&cg_runpitch, "cg_runpitch", "0.002", CVAR_ARCHIVE},
	{&cg_runroll, "cg_runroll", "0.005", CVAR_ARCHIVE},
	{&cg_bobup, "cg_bobup", "0.005", CVAR_CHEAT},
	{&cg_bobpitch, "cg_bobpitch", "0.002", CVAR_ARCHIVE},
	{&cg_bobroll, "cg_bobroll", "0.002", CVAR_ARCHIVE},
	{&cg_swingSpeed, "cg_swingSpeed", "0.3", CVAR_CHEAT},
	{&cg_animSpeed, "cg_animspeed", "1", CVAR_CHEAT},
	{&cg_animBlend, "cg_animblend", "5.0", CVAR_ARCHIVE},
	{&cg_debugPlayerAnim, "cg_debugPlayerAnim", "0", CVAR_CHEAT},
	{&cg_debugWeaponAnim, "cg_debugWeaponAnim", "0", CVAR_CHEAT},
	{&cg_debugPosition, "cg_debugposition", "0", CVAR_CHEAT},
	{&cg_debugEvents, "cg_debugevents", "0", CVAR_CHEAT},
	{&cg_errorDecay, "cg_errordecay", "100", 0},
	{&cg_nopredict, "cg_nopredict", "0", 0},
	{&cg_noPlayerAnims, "cg_noplayeranims", "0", CVAR_CHEAT},
	{&cg_showmiss, "cg_showmiss", "0", 0},
	{&cg_footsteps, "cg_footsteps", "1", CVAR_CHEAT},
	{&cg_tracerChance, "cg_tracerchance", "0.4", CVAR_CHEAT},
	{&cg_tracerWidth, "cg_tracerwidth", "1", CVAR_CHEAT},
	{&cg_tracerLength, "cg_tracerlength", "100", CVAR_CHEAT},
	{&cg_thirdPersonRange, "cg_thirdPersonRange", "40", CVAR_CHEAT},
	{&cg_thirdPersonAngle, "cg_thirdPersonAngle", "0", CVAR_CHEAT},
	{&cg_thirdPerson, "cg_thirdPerson", "0", 0},
	{&cg_notifyTime, "cg_notifyTime", "10000", CVAR_ARCHIVE},
	{&cg_notifyHeight, "cg_notifyHeight", "10", CVAR_ARCHIVE},
	{&cg_chatTime, "cg_chatTime", "10000", CVAR_ARCHIVE},
	{&cg_chatHeight, "cg_chatHeight", "10", CVAR_ARCHIVE},
	{&cg_teamChatTime, "cg_teamChatTime", "10000", CVAR_ARCHIVE},
	{&cg_teamChatHeight, "cg_teamChatHeight", "10", CVAR_ARCHIVE},
	{&cg_forceModel, "cg_forceModel", "0", CVAR_ARCHIVE},
	{&cg_forceBrightSkins, "cg_forceBrightSkins", "0", CVAR_ARCHIVE},
	{&cg_predictItems, "cg_predictItems", "1", CVAR_ARCHIVE},
	{&cg_deferPlayers, "cg_deferPlayers", "0", CVAR_CHEAT},
	{&cg_drawTeamOverlay, "cg_drawTeamOverlay", "0", CVAR_ARCHIVE},
	{&cg_teamOverlayUserinfo, "teamoverlay", "0", CVAR_ROM | CVAR_USERINFO},
	{&cg_stats, "cg_stats", "0", 0},
	{&cg_drawFriend, "cg_drawFriend", "1", CVAR_ARCHIVE},
	{&cg_teamChatsOnly, "cg_teamChatsOnly", "0", CVAR_ARCHIVE},
	{&cg_noVoiceChats, "cg_noVoiceChats", "0", CVAR_ARCHIVE},
	{&cg_noVoiceText, "cg_noVoiceText", "0", CVAR_ARCHIVE},

	// the following variables are created in other parts of the system,
	// but we also reference them here
	{&cg_buildScript, "com_buildScript", "0", 0},	// force loading of all possible data amd error on failures
	{&cg_blood, "com_blood", "1", CVAR_ARCHIVE},
	{&cg_synchronousClients, "g_synchronousClients", "0", 0},	// communicated by systeminfo

	{&cg_currentSelectedPlayer, "cg_currentSelectedPlayer", "0", CVAR_ARCHIVE},
	{&cg_currentSelectedPlayerName, "cg_currentSelectedPlayerName", "", CVAR_ARCHIVE},

	{&cg_singlePlayer, "ui_singlePlayerActive", "0", CVAR_USERINFO},
	{&cg_singlePlayerActive, "ui_singlePlayerActive", "0", CVAR_USERINFO},

	{&cg_enableDust, "g_enableDust", "0", CVAR_SERVERINFO},
	{&cg_enableBreath, "g_enableBreath", "0", CVAR_SERVERINFO},
	{&cg_obeliskRespawnDelay, "g_obeliskRespawnDelay", "10", CVAR_SERVERINFO},

	{&cg_cameraOrbit, "cg_cameraOrbit", "0", CVAR_CHEAT},
	{&cg_cameraOrbitDelay, "cg_cameraOrbitDelay", "50", CVAR_ARCHIVE},
	{&cg_timescaleFadeEnd, "cg_timescaleFadeEnd", "1", 0},
	{&cg_timescaleFadeSpeed, "cg_timescaleFadeSpeed", "0", 0},
	{&cg_timescale, "timescale", "1", 0},
	{&cg_scorePlum, "cg_scorePlums", "1", CVAR_USERINFO | CVAR_ARCHIVE},
//unlagged - smooth clients #2
// this is done server-side now
//  { &cg_smoothClients, "cg_smoothClients", "0", CVAR_USERINFO | CVAR_ARCHIVE},
//unlagged - smooth clients #2
	{&cg_cameraMode, "com_cameraMode", "0", CVAR_CHEAT},

	{&cg_noTaunt, "cg_noTaunt", "0", CVAR_ARCHIVE},
	{&cg_noProjectileTrail, "cg_noProjectileTrail", "0", CVAR_ARCHIVE},

	{&cg_railType, "cg_railType", "2", CVAR_CHEAT},
	{&cg_trueLightning, "cg_trueLightning", "0.0", CVAR_ARCHIVE},
	{&cg_particles, "cg_particles", "1", CVAR_ARCHIVE},
	{&cg_particleCollision, "cg_particleCollision", "0", CVAR_ARCHIVE},

	// these cvars are shared accross both games
	{&pm_airControl, "pm_airControl", "0", 0},
	{&pm_fastWeaponSwitches, "pm_fastWeaponSwitches", "0", 0},
	{&pm_fixedPmove, "pm_fixedPmove", "0", 0},
	{&pm_fixedPmoveFPS, "pm_fixedPmoveFPS", "125", 0},

	{&cg_gravityZ, "g_gravityZ", "0", 0},	// communicated by systeminfo

//unlagged - client options
	{&cg_delag, "cg_delag", "1", CVAR_ARCHIVE | CVAR_USERINFO},
	{&cg_debugDelag, "cg_debugDelag", "0", CVAR_USERINFO | CVAR_CHEAT},
	{&cg_drawBBox, "cg_drawBBox", "0", CVAR_CHEAT},
	{&cg_cmdTimeNudge, "cg_cmdTimeNudge", "0", CVAR_ARCHIVE | CVAR_USERINFO},
	// this will be automagically copied from the server
	{&sv_fps, "sv_fps", "20", 0},
	{&cg_projectileNudge, "cg_projectileNudge", "0", CVAR_ARCHIVE},
	{&cg_optimizePrediction, "cg_optimizePrediction", "1", CVAR_ARCHIVE},
	{&cl_timeNudge, "cl_timeNudge", "0", CVAR_ARCHIVE},
	{&cg_latentSnaps, "cg_latentSnaps", "0", CVAR_USERINFO | CVAR_CHEAT},
	{&cg_latentCmds, "cg_latentCmds", "0", CVAR_USERINFO | CVAR_CHEAT},
	{&cg_plOut, "cg_plOut", "0", CVAR_USERINFO | CVAR_CHEAT},
//unlagged - client options


	{&cg_wallWalkSmoothTime, "cg_wallWalkSmoothTime", "300", CVAR_ARCHIVE},

	// weaver cvars
	{&cg_drawWeaverDisc, "cg_drawWeaverDisc", "1", CVAR_ARCHIVE},
	{&cg_drawWeaverThread, "cg_drawWeaverThread", "1", CVAR_ARCHIVE},
	{&cg_weaverTutorial, "cg_weaverTutorial", "2", CVAR_ARCHIVE},
	{&cg_weaverTutorialMaxElement, "cg_weaverTutorialMaxElement", "8", CVAR_ARCHIVE},
	{&cg_weaverTutorialMaxDepth, "cg_weaverTutorialMaxDepth", "10", CVAR_ARCHIVE},
	{&cg_tutorial, "cg_tutorial", "1", CVAR_ARCHIVE},
	{&cg_weaverDiscSize, "cg_weaverDiscSize", "128", CVAR_ARCHIVE},
	{&cg_weaverDiscX, "cg_weaverDiscX", "0", CVAR_ARCHIVE},
	{&cg_weaverDiscY, "cg_weaverDiscY", "0", CVAR_ARCHIVE},
	{&cg_weaverSenseRadius, "cg_weaverSenseRadius", "180", CVAR_ARCHIVE},
	{&cg_weaverDiscMoveScale, "cg_weaverDiscMoveScale", "0", CVAR_ARCHIVE},
	{&cg_weaverThreadMoveScale, "cg_weaverThreadMoveScale", "1", CVAR_ARCHIVE},
	{&cg_weaverSpellSwitch, "cg_weaverSpellSwitch", "1", CVAR_ARCHIVE},
};

static int      cvarTableSize = sizeof(cvarTable) / sizeof(cvarTable[0]);

/*
=================
CG_RegisterCvars
=================
*/
void CG_RegisterCvars(void)
{
	int             i;
	cvarTable_t    *cv;
	char            var[MAX_TOKEN_CHARS];

	for(i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++)
	{
		trap_Cvar_Register(cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags);
	}

	// see if we are also running the server on this machine
	trap_Cvar_VariableStringBuffer("sv_running", var, sizeof(var));
	cgs.localServer = atoi(var);

	forceModelModificationCount = cg_forceModel.modificationCount;
	forceBrightSkinsModificationCount = cg_forceBrightSkins.modificationCount;

	trap_Cvar_Register(NULL, "model", DEFAULT_MODEL, CVAR_USERINFO | CVAR_ARCHIVE);
}

/*
===================
CG_ForceModelChange
===================
*/
static void CG_ForceModelChange(void)
{
	int             i;

	for(i = 0; i < MAX_CLIENTS; i++)
	{
		const char     *clientInfo;

		clientInfo = CG_ConfigString(CS_PLAYERS + i);
		if(!clientInfo[0])
		{
			continue;
		}
		CG_NewClientInfo(i);
	}
}

/*
=================
CG_UpdateCvars
=================
*/
void CG_UpdateCvars(void)
{
	int             i;
	cvarTable_t    *cv;

	for(i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++)
	{
//unlagged - client options
		// clamp the value between 0 and 999
		// negative values would suck - people could conceivably shoot other
		// players *long* after they had left the area, on purpose
		if(cv->vmCvar == &cg_cmdTimeNudge)
		{
			CG_Cvar_ClampInt(cv->cvarName, cv->vmCvar, 0, 999);
		}
		// cl_timenudge less than -50 or greater than 50 doesn't actually
		// do anything more than -50 or 50 (actually the numbers are probably
		// closer to -30 and 30, but 50 is nice and round-ish)
		// might as well not feed the myth, eh?
		else if(cv->vmCvar == &cl_timeNudge)
		{
			CG_Cvar_ClampInt(cv->cvarName, cv->vmCvar, -50, 50);
		}
		// don't let this go too high - no point
		else if(cv->vmCvar == &cg_latentSnaps)
		{
			CG_Cvar_ClampInt(cv->cvarName, cv->vmCvar, 0, 10);
		}
		// don't let this get too large
		else if(cv->vmCvar == &cg_latentCmds)
		{
			CG_Cvar_ClampInt(cv->cvarName, cv->vmCvar, 0, MAX_LATENT_CMDS - 1);
		}
		// no more than 100% packet loss
		else if(cv->vmCvar == &cg_plOut)
		{
			CG_Cvar_ClampInt(cv->cvarName, cv->vmCvar, 0, 100);
		}
//unlagged - client options
		trap_Cvar_Update(cv->vmCvar);
	}

	// check for modications here

	// If team overlay is on, ask for updates from the server.  If its off,
	// let the server know so we don't receive it
	if(drawTeamOverlayModificationCount != cg_drawTeamOverlay.modificationCount)
	{
		drawTeamOverlayModificationCount = cg_drawTeamOverlay.modificationCount;

		if(cg_drawTeamOverlay.integer > 0)
		{
			trap_Cvar_Set("teamoverlay", "1");
		}
		else
		{
			trap_Cvar_Set("teamoverlay", "0");
		}
	}

	// if force model changed
	if(forceModelModificationCount != cg_forceModel.modificationCount || forceBrightSkinsModificationCount != cg_forceBrightSkins.modificationCount)
	{
		forceModelModificationCount = cg_forceModel.modificationCount;
		forceBrightSkinsModificationCount = cg_forceBrightSkins.modificationCount;

		CG_ForceModelChange();
	}
}

int CG_CrosshairPlayer(void)
{
	if(cg.time > (cg.crosshairClientTime + 1000))
	{
		return -1;
	}
	return cg.crosshairClientNum;
}

int CG_LastAttacker(void)
{
	if(!cg.attackerTime)
	{
		return -1;
	}
	return cg.snap->ps.persistant[PERS_ATTACKER];
}

void QDECL CG_Printf(const char *msg, ...)
{
	va_list         argptr;
	char            text[1024];

	va_start(argptr, msg);
	Q_vsnprintf(text, sizeof(text), msg, argptr);
	va_end(argptr);

	trap_Print(text);
}

void QDECL CG_Error(const char *msg, ...)
{
	va_list         argptr;
	char            text[1024];

	va_start(argptr, msg);
	Q_vsnprintf(text, sizeof(text), msg, argptr);
	va_end(argptr);

#ifdef CG_LUA
	CG_ShutdownLua();
#endif

	trap_Error(text);
}

void QDECL Com_Error(int level, const char *error, ...)
{
	va_list         argptr;
	char            text[1024];

	va_start(argptr, error);
	Q_vsnprintf(text, sizeof(text), error, argptr);
	va_end(argptr);

	CG_Error("%s", text);
}

void QDECL Com_Printf(const char *msg, ...)
{
	va_list         argptr;
	char            text[1024];

	va_start(argptr, msg);
	Q_vsnprintf(text, sizeof(text), msg, argptr);
	va_end(argptr);

	CG_Printf("%s", text);
}

/*
================
CG_Argv
================
*/
const char     *CG_Argv(int arg)
{
	static char     buffer[MAX_STRING_CHARS];

	trap_Argv(arg, buffer, sizeof(buffer));

	return buffer;
}


//========================================================================

/*
=================
CG_RegisterItemSounds

The server says this item is used on this level
=================
*/
static void CG_RegisterItemSounds(int itemNum)
{
	gitem_t        *item;
	char            data[MAX_QPATH];
	char           *s, *start;
	int             len;

	item = &bg_itemlist[itemNum];

	if(item->pickup_sound)
	{
		trap_S_RegisterSound(item->pickup_sound);
	}

	// parse the space seperated precache string for other media
	s = item->sounds;
	if(!s || !s[0])
		return;

	while(*s)
	{
		start = s;
		while(*s && *s != ' ')
		{
			s++;
		}

		len = s - start;
		if(len >= MAX_QPATH || len < 5)
		{
			CG_Error("PrecacheItem: %s has bad precache string", item->classname);
			return;
		}
		memcpy(data, start, len);
		data[len] = 0;
		if(*s)
		{
			s++;
		}

		if(!strcmp(data + len - 3, "wav"))
		{
			trap_S_RegisterSound(data);
		}
	}
}




/*
=================
CG_RegisterSounds

called during a precache command
=================
*/
static void CG_RegisterSounds(void)
{
	int             i;
	char            items[MAX_ITEMS + 1];
	char            name[MAX_QPATH];
	const char     *soundName;

	// voice commands
#ifdef MISSIONPACK
	CG_LoadVoiceChats();
#endif
	CG_LoadingString("Announcements", qfalse);

	cgs.media.oneMinuteSound = trap_S_RegisterSound("sound/feedback/1_minute.ogg");
	cgs.media.fiveMinuteSound = trap_S_RegisterSound("sound/feedback/5_minute.ogg");
	cgs.media.suddenDeathSound = trap_S_RegisterSound("sound/feedback/sudden_death.ogg");
	cgs.media.oneFragSound = trap_S_RegisterSound("sound/feedback/1_frag.ogg");
	cgs.media.twoFragSound = trap_S_RegisterSound("sound/feedback/2_frags.ogg");
	cgs.media.threeFragSound = trap_S_RegisterSound("sound/feedback/3_frags.ogg");
	cgs.media.count3Sound = trap_S_RegisterSound("sound/feedback/three.ogg");
	cgs.media.count2Sound = trap_S_RegisterSound("sound/feedback/two.ogg");
	cgs.media.count1Sound = trap_S_RegisterSound("sound/feedback/one.ogg");
	cgs.media.countFightSound = trap_S_RegisterSound("sound/feedback/fight.ogg");
	cgs.media.countPrepareSound = trap_S_RegisterSound("sound/feedback/prepare.ogg");

	cgs.media.voteNow = trap_S_RegisterSound("sound/feedback/vote_now.ogg");
	cgs.media.votePassed = trap_S_RegisterSound("sound/feedback/vote_passed.ogg");
	cgs.media.voteFailed = trap_S_RegisterSound("sound/feedback/vote_failed.ogg");

	cgs.media.winnerSound = trap_S_RegisterSound("sound/feedback/youwin.ogg");
	cgs.media.loserSound = trap_S_RegisterSound("sound/feedback/youlose.ogg");

	CG_LoadingString("Orders", qfalse);

	if(cgs.gametype >= GT_TEAM || cg_buildScript.integer)
	{
		cgs.media.captureAwardSound = trap_S_RegisterSound("sound/teamplay/flagcapture_yourteam.wav");
		cgs.media.redLeadsSound = trap_S_RegisterSound("sound/feedback/redleads.ogg");
		cgs.media.blueLeadsSound = trap_S_RegisterSound("sound/feedback/blueleads.ogg");
		cgs.media.teamsTiedSound = trap_S_RegisterSound("sound/feedback/teamstied.ogg");
		cgs.media.hitTeamSound = trap_S_RegisterSound("sound/feedback/hit_teammate.wav");

		cgs.media.redScoredSound = trap_S_RegisterSound("sound/teamplay/voc_red_scores.ogg");
		cgs.media.blueScoredSound = trap_S_RegisterSound("sound/teamplay/voc_blue_scores.ogg");

		cgs.media.captureYourTeamSound = trap_S_RegisterSound("sound/teamplay/flagcapture_yourteam.wav");
		cgs.media.captureOpponentSound = trap_S_RegisterSound("sound/teamplay/flagcapture_opponent.wav");

		cgs.media.returnYourTeamSound = trap_S_RegisterSound("sound/teamplay/flagreturn_yourteam.wav");
		cgs.media.returnOpponentSound = trap_S_RegisterSound("sound/teamplay/flagreturn_opponent.wav");

		cgs.media.takenYourTeamSound = trap_S_RegisterSound("sound/teamplay/flagtaken_yourteam.wav");
		cgs.media.takenOpponentSound = trap_S_RegisterSound("sound/teamplay/flagtaken_opponent.wav");

		if(cgs.gametype == GT_CTF || cg_buildScript.integer)
		{
			cgs.media.redFlagReturnedSound = trap_S_RegisterSound("sound/teamplay/voc_red_returned.ogg");
			cgs.media.blueFlagReturnedSound = trap_S_RegisterSound("sound/teamplay/voc_blue_returned.ogg");
			cgs.media.enemyTookYourFlagSound = trap_S_RegisterSound("sound/teamplay/voc_enemy_flag.ogg");
			cgs.media.yourTeamTookEnemyFlagSound = trap_S_RegisterSound("sound/teamplay/voc_team_flag.ogg");
		}

		if(cgs.gametype == GT_1FCTF)
		{
			// FIXME: get a replacement for this sound ?
			cgs.media.neutralFlagReturnedSound = trap_S_RegisterSound("sound/teamplay/flagreturn_opponent.ogg");
			cgs.media.yourTeamTookTheFlagSound = trap_S_RegisterSound("sound/teamplay/voc_team_1flag.ogg");
			cgs.media.enemyTookTheFlagSound = trap_S_RegisterSound("sound/teamplay/voc_enemy_1flag.ogg");
		}

		if(cgs.gametype == GT_1FCTF || cgs.gametype == GT_CTF)
		{
			cgs.media.youHaveFlagSound = trap_S_RegisterSound("sound/teamplay/voc_you_flag.ogg");
			cgs.media.holyShitSound = trap_S_RegisterSound("sound/feedback/holyshit.ogg");
		}

		if(cgs.gametype == GT_OBELISK)
		{
			cgs.media.yourBaseIsUnderAttackSound = trap_S_RegisterSound("sound/teamplay/voc_base_attack.ogg");
		}
	}

	/*
	cgs.media.useNothingSound = trap_S_RegisterSound("sound/items/use_nothing.wav");
	cgs.media.gibSound = trap_S_RegisterSound("sound/player/gibelectro.ogg");
	cgs.media.gibBounce1Sound = trap_S_RegisterSound("sound/player/gibimp1.ogg");
	cgs.media.gibBounce2Sound = trap_S_RegisterSound("sound/player/gibimp2.ogg");
	cgs.media.gibBounce3Sound = trap_S_RegisterSound("sound/player/gibimp3.ogg");

	cgs.media.obeliskHitSound1 = trap_S_RegisterSound("sound/items/obelisk_hit_01.ogg");
	cgs.media.obeliskHitSound2 = trap_S_RegisterSound("sound/items/obelisk_hit_02.ogg");
	cgs.media.obeliskHitSound3 = trap_S_RegisterSound("sound/items/obelisk_hit_03.ogg");
	cgs.media.obeliskRespawnSound = trap_S_RegisterSound("sound/items/obelisk_respawn.ogg");
	*/

	CG_LoadingString("Ambient Sounds", qfalse);

	cgs.media.teleInSound = trap_S_RegisterSound("sound/player/telein.ogg");
	cgs.media.teleOutSound = trap_S_RegisterSound("sound/player/teleout.ogg");
	cgs.media.respawnSound = trap_S_RegisterSound("sound/items/respawn.ogg");

	cgs.media.noAmmoSound = trap_S_RegisterSound("sound/weapons/noammo.ogg");

	cgs.media.talkSound = trap_S_RegisterSound("sound/player/talk.ogg");

	cgs.media.hitSound = trap_S_RegisterSound("sound/feedback/hit.wav");

	CG_LoadingString("Footsteps", qfalse);

	// footstepts ...and other player movement
	cgs.media.watrInSound = trap_S_RegisterSound("sound/player/water_in.ogg");
	cgs.media.watrOutSound = trap_S_RegisterSound("sound/player/water_out.ogg");
	cgs.media.watrUnSound = trap_S_RegisterSound("sound/player/water_under.ogg");

	cgs.media.jumpPadSound = trap_S_RegisterSound("sound/world/jumppad.wav");

	for(i = 0; i < 4; i++)
	{
		Com_sprintf(name, sizeof(name), "sound/player/footsteps/stone%i.ogg", i + 1);
		cgs.media.footsteps[FOOTSTEP_STONE][i] = trap_S_RegisterSound(name);

		Com_sprintf(name, sizeof(name), "sound/player/footsteps/softrug%i.ogg", i + 1);
		cgs.media.footsteps[FOOTSTEP_BOOT][i] = trap_S_RegisterSound(name);

		Com_sprintf(name, sizeof(name), "sound/player/footsteps/flesh%i.ogg", i + 1);
		cgs.media.footsteps[FOOTSTEP_FLESH][i] = trap_S_RegisterSound(name);

		Com_sprintf(name, sizeof(name), "sound/player/footsteps/mech%i.ogg", i + 1);
		cgs.media.footsteps[FOOTSTEP_MECH][i] = trap_S_RegisterSound(name);

		Com_sprintf(name, sizeof(name), "sound/player/footsteps/energy%i.ogg", i + 1);
		cgs.media.footsteps[FOOTSTEP_ENERGY][i] = trap_S_RegisterSound(name);

		Com_sprintf(name, sizeof(name), "sound/player/footsteps/splash%i.ogg", i + 1);
		cgs.media.footsteps[FOOTSTEP_SPLASH][i] = trap_S_RegisterSound(name);

		Com_sprintf(name, sizeof(name), "sound/player/footsteps/metal%i.ogg", i + 1);
		cgs.media.footsteps[FOOTSTEP_METAL][i] = trap_S_RegisterSound(name);

		Com_sprintf(name, sizeof(name), "sound/player/footsteps/wallwalk%i.ogg", i + 1);
		cgs.media.footsteps[FOOTSTEP_WALLWALK][i] = trap_S_RegisterSound(name);
	}

	// only register the items that the server says we need
	// raynorpat: used to be a standard strcpy, but can be exploited via
	// a remote stack overflow. see http://www.milw0rm.com/exploits/1977
	Q_strncpyz(items, CG_ConfigString(CS_ITEMS), sizeof(items));

	/*
	CG_LoadingString("item sounds", qfalse);

	for(i = 1; i < bg_numItems; i++)
	{
//      if ( items[ i ] == '1' || cg_buildScript.integer ) {
		CG_RegisterItemSounds(i);
//      }
	}
	*/

	for(i = 1; i < MAX_SOUNDS; i++)
	{
		soundName = CG_ConfigString(CS_SOUNDS + i);
		if(!soundName[0])
		{
			break;
		}
		if(soundName[0] == '*')
		{
			continue;			// custom sound
		}
		cgs.gameSounds[i] = trap_S_RegisterSound(soundName);
	}

	/*
	cgs.media.flightSound = trap_S_RegisterSound("sound/items/flight.wav");
	cgs.media.medkitSound = trap_S_RegisterSound("sound/items/use_medkit.wav");
	cgs.media.quadSound = trap_S_RegisterSound("sound/items/damage.ogg");
	cgs.media.sfx_railg = trap_S_RegisterSound("sound/weapons/railgun/railgf1a.ogg");
	cgs.media.sfx_rockexp = trap_S_RegisterSound("sound/weapons/rocket/rocklx1a.ogg");
	cgs.media.sfx_plasmaexp = trap_S_RegisterSound("sound/weapons/plasma/plasmx1a.wav");
	cgs.media.hookImpactSound = trap_S_RegisterSound("sound/weapons/gauntlet/slashkut.ogg");
	cgs.media.impactFlesh1Sound = trap_S_RegisterSound("sound/weapons/impactFlesh1.ogg");
	cgs.media.impactFlesh2Sound = trap_S_RegisterSound("sound/weapons/impactFlesh2.ogg");
	cgs.media.impactFlesh3Sound = trap_S_RegisterSound("sound/weapons/impactFlesh3.ogg");
	cgs.media.impactMetal1Sound = trap_S_RegisterSound("sound/weapons/impactMetal1.ogg");
	cgs.media.impactMetal2Sound = trap_S_RegisterSound("sound/weapons/impactMetal2.ogg");
	cgs.media.impactMetal3Sound = trap_S_RegisterSound("sound/weapons/impactMetal3.ogg");
	cgs.media.impactMetal4Sound = trap_S_RegisterSound("sound/weapons/impactMetal4.ogg");
	cgs.media.impactWall1Sound = trap_S_RegisterSound("sound/weapons/impactWall1.ogg");
	cgs.media.impactWall2Sound = trap_S_RegisterSound("sound/weapons/impactWall2.ogg");

	cgs.media.sfx_nghit = trap_S_RegisterSound("sound/weapons/flakcannon/wnalimpd.ogg");
	cgs.media.sfx_nghitflesh = trap_S_RegisterSound("sound/weapons/flakcannon/wnalimpl.ogg");
	cgs.media.sfx_nghitmetal = trap_S_RegisterSound("sound/weapons/flakcannon/wnalimpm.ogg");

	cgs.media.kamikazeExplodeSound = trap_S_RegisterSound("sound/items/kam_explode.wav");
	cgs.media.kamikazeImplodeSound = trap_S_RegisterSound("sound/items/kam_implode.wav");
	cgs.media.kamikazeFarSound = trap_S_RegisterSound("sound/items/kam_explode_far.wav");
	*/

	CG_LoadingString("Misc Sounds", qfalse);

	cgs.media.regenSound = trap_S_RegisterSound("sound/items/regen.wav");
	cgs.media.protectSound = trap_S_RegisterSound("sound/items/protect3.wav");
	cgs.media.n_healthSound = trap_S_RegisterSound("sound/items/n_health.wav");
	cgs.media.hgrenb1aSound = trap_S_RegisterSound("sound/weapons/grenade/hgrenb1a.wav");
	cgs.media.hgrenb2aSound = trap_S_RegisterSound("sound/weapons/grenade/hgrenb2a.wav");

	cgs.media.tracerSound = trap_S_RegisterSound("sound/weapons/machinegun/par_shot_2.ogg");
	cgs.media.selectSound = trap_S_RegisterSound("sound/weapons/change.ogg");
	cgs.media.wearOffSound = trap_S_RegisterSound("sound/items/wearoff.ogg");
}


//===================================================================================


/*
=================
CG_RegisterGraphics

This function may execute for a couple of minutes with a slow disk.
=================
*/
static void CG_RegisterGraphics(void)
{
	int             i, j;
	char            path[MAX_QPATH];
	char            items[MAX_ITEMS + 1];
	static char    *sb_nums[11] = {
		"gfx/2d/numbers/zero_32b",
		"gfx/2d/numbers/one_32b",
		"gfx/2d/numbers/two_32b",
		"gfx/2d/numbers/three_32b",
		"gfx/2d/numbers/four_32b",
		"gfx/2d/numbers/five_32b",
		"gfx/2d/numbers/six_32b",
		"gfx/2d/numbers/seven_32b",
		"gfx/2d/numbers/eight_32b",
		"gfx/2d/numbers/nine_32b",
		"gfx/2d/numbers/minus_32b",
	};

	// clear any references to old media
	memset(&cg.refdef, 0, sizeof(cg.refdef));
	trap_R_ClearScene();

	CG_LoadingString("Map BSP Data", qfalse);

	trap_R_LoadWorldMap(cgs.mapname);

	CG_LoadingString("HUD", qfalse);

	// precache status bar pics
	for(i = 0; i < 11; i++)
	{
		cgs.media.numberShaders[i] = trap_R_RegisterShader(sb_nums[i]);
	}

	/*
	cgs.media.hud_top_team_middle = trap_R_RegisterShaderNoMip("hud/hud_top_team_middle");
	cgs.media.hud_top_team_middle_overlay = trap_R_RegisterShaderNoMip("hud/hud_top_team_middle_overlay");
	cgs.media.hud_top_team_left = trap_R_RegisterShaderNoMip("hud/hud_top_team_left");
	cgs.media.hud_top_team_left_overlay = trap_R_RegisterShaderNoMip("hud/hud_top_team_left_overlay");
	cgs.media.hud_top_ctf_left = trap_R_RegisterShaderNoMip("hud/hud_top_ctf_left");
	cgs.media.hud_top_ctf_right = trap_R_RegisterShaderNoMip("hud/hud_top_ctf_right");
	cgs.media.hud_top_team_right = trap_R_RegisterShaderNoMip("hud/hud_top_team_right");
	cgs.media.hud_top_team_right_overlay = trap_R_RegisterShaderNoMip("hud/hud_top_team_right_overlay");
	cgs.media.hud_top_ffa_middle = trap_R_RegisterShaderNoMip("hud/hud_top_ffa_middle");
	cgs.media.hud_top_ffa_middle_overlay = trap_R_RegisterShaderNoMip("hud/hud_top_ffa_middle_overlay");
	cgs.media.hud_top_ffa_left = trap_R_RegisterShaderNoMip("hud/hud_top_ffa_left");
	cgs.media.hud_top_ffa_left_overlay = trap_R_RegisterShaderNoMip("hud/hud_top_ffa_left_overlay");
	cgs.media.hud_top_ffa_right = trap_R_RegisterShaderNoMip("hud/hud_top_ffa_right");
	cgs.media.hud_top_ffa_right_overlay = trap_R_RegisterShaderNoMip("hud/hud_top_ffa_right_overlay");

	cgs.media.hud_bar_left = trap_R_RegisterShaderNoMip("hud/hud_bar_left");
	cgs.media.hud_bar_left_overlay = trap_R_RegisterShaderNoMip("hud/hud_bar_left_overlay");
	cgs.media.hud_bar_middle_middle = trap_R_RegisterShaderNoMip("hud/hud_bar_middle_middle");
	cgs.media.hud_bar_middle_left_end = trap_R_RegisterShaderNoMip("hud/hud_bar_middle_left_end");
	cgs.media.hud_bar_middle_left_middle = trap_R_RegisterShaderNoMip("hud/hud_bar_middle_left_middle");
	cgs.media.hud_bar_middle_left_right = trap_R_RegisterShaderNoMip("hud/hud_bar_middle_left_right");
	cgs.media.hud_bar_middle_right_left = trap_R_RegisterShaderNoMip("hud/hud_bar_middle_right_left");
	cgs.media.hud_bar_middle_right_middle = trap_R_RegisterShaderNoMip("hud/hud_bar_middle_right_middle");
	cgs.media.hud_bar_middle_right_end = trap_R_RegisterShaderNoMip("hud/hud_bar_middle_right_end");
	cgs.media.hud_bar_middle_overlay = trap_R_RegisterShaderNoMip("hud/hud_bar_middle_overlay");
	cgs.media.hud_bar_right = trap_R_RegisterShaderNoMip("hud/hud_bar_right");
	cgs.media.hud_bar_right_overlay = trap_R_RegisterShaderNoMip("hud/hud_bar_right_overlay");

	cgs.media.hud_icon_health = trap_R_RegisterShaderNoMip("hud/hud_icon_health");
	cgs.media.hud_icon_armor = trap_R_RegisterShaderNoMip("hud/hud_icon_armor");
	*/

	cgs.media.osd_button = trap_R_RegisterShaderNoMip("ui/button");
	cgs.media.osd_button_focus = trap_R_RegisterShaderNoMip("ui/button_focus");

	/*
	cgs.media.backTileShader = trap_R_RegisterShader("gfx/2d/backtile");
	cgs.media.noammoShader = trap_R_RegisterShader("icons/noammo");
	*/

	/*
	cgs.media.sideBarItemShader = trap_R_RegisterShaderNoMip("hud/sidebar_item");
	cgs.media.sideBarItemSelectShader = trap_R_RegisterShaderNoMip("hud/sidebar_item_select");
	cgs.media.sideBarPowerupShader = trap_R_RegisterShaderNoMip("hud/sidebar_powerup");
	*/

	cgs.media.scoreboard_row = trap_R_RegisterShaderNoMip("gfx/scoreboard/row");
	cgs.media.scoreboard_top_red = trap_R_RegisterShaderNoMip("gfx/scoreboard/top_red");
	cgs.media.scoreboard_top_blue = trap_R_RegisterShaderNoMip("gfx/scoreboard/top_blue");
	cgs.media.scoreboard_vs = trap_R_RegisterShaderNoMip("gfx/scoreboard/vs");

	// weaver disc
	for(i = 1; i <= NUM_WEAVERDISCS; i++)
	{
		cgs.media.weaverDiscShader[i] = trap_R_RegisterShaderNoMip(va("gfx/hud/disc%i", i));
	}

	// sense dots
	cgs.media.weaverSenseDot1 = trap_R_RegisterShaderNoMip("gfx/hud/sensedot1");
	cgs.media.weaverSenseDot2 = trap_R_RegisterShaderNoMip("gfx/hud/sensedot2");
	cgs.media.weaverSenseDot3 = trap_R_RegisterShaderNoMip("gfx/hud/sensedot3");

	cgs.media.weaverCorner = trap_R_RegisterShaderNoMip("gfx/hud/corner");
	cgs.media.weaverBarExt = trap_R_RegisterShaderNoMip("gfx/hud/bar_ext");
	cgs.media.weaverBarExt0 = trap_R_RegisterShaderNoMip("gfx/hud/bar_ext0");
	cgs.media.weaverBarEnd = trap_R_RegisterShaderNoMip("gfx/hud/bar_end");
	cgs.media.weaverBarDiv = trap_R_RegisterShaderNoMip("gfx/hud/bar_div");
	
	cgs.media.hpBot = trap_R_RegisterShaderNoMip("gfx/hud/hp_bot");
	cgs.media.stBot = trap_R_RegisterShaderNoMip("gfx/hud/st_bot");

	for(i = 0; i < 3; i++)
	{
		for(j = WVP_NONE + 1; j < WVP_NUMBER; j += 2)
		{
			Com_sprintf(path, MAX_QPATH, "models/threads/%s%d", WeavePowerNames[j], i+1);
			cgs.media.weaverThreads[j][i] = trap_R_RegisterShaderNoMip(path);
		}
	}

	cgs.media.weaverStatus[PW_REGEN] = trap_R_RegisterShaderNoMip("gfx/hud/powerups/healing");
	cgs.media.weaverStatus[PW_SHIELDED] = trap_R_RegisterShaderNoMip("gfx/hud/powerups/shielded");
	cgs.media.weaverStatus[PW_POISONED] = trap_R_RegisterShaderNoMip("gfx/hud/powerups/poisoned1");
	cgs.media.weaverStatus[PW_SLOWPOISONED] = trap_R_RegisterShaderNoMip("gfx/hud/powerups/poisoned2");

	CG_LoadingString("Crosshairs", qfalse);

	for(i = 0; i < NUM_CROSSHAIRS; i++)
	{
		cgs.media.crosshairShader[i] = trap_R_RegisterShader(va("gfx/hud/crosshairs/xh%i", i + 1));
	}

	CG_LoadingString("Icons", qfalse);

	cgs.media.balloonShader = trap_R_RegisterShader("sprites/balloon3");

	if(cgs.gametype >= GT_TEAM || cg_buildScript.integer)
	{
		cgs.media.friendShader = trap_R_RegisterShader("sprites/friend");
		cgs.media.redQuadShader = trap_R_RegisterShader("powerups/blueflag");
		cgs.media.teamStatusBar = trap_R_RegisterShader("gfx/2d/colorbar.tga");
	}

	// only register the items that the server says we need
	// raynorpat: used to be a standard strcpy, but can be exploited via
	// a remote stack overflow. see http://www.milw0rm.com/exploits/1977
	Q_strncpyz(items, CG_ConfigString(CS_ITEMS), sizeof(items));

	
	/*
	memset(cg_items, 0, sizeof(cg_items));
	memset(cg_weapons, 0, sizeof(cg_weapons));
	*/
	/*
	CG_LoadingString("items", qfalse);

	for(i = 1; i < bg_numItems; i++)
	{
		if(items[i] == '1' || cg_buildScript.integer)
			CG_RegisterItemVisuals(i);
	}
	*/

	CG_LoadingString("Effects", qfalse);

	cgs.media.viewBloodShader = trap_R_RegisterShader("viewBloodBlend");

	cgs.media.deferShader = trap_R_RegisterShaderNoMip("gfx/2d/defer.tga");

	cgs.media.smokePuffShader = trap_R_RegisterShader("smokePuff");
	cgs.media.shotgunSmokePuffShader = trap_R_RegisterShader("shotgunSmokePuff");

	cgs.media.nailPuffShader = trap_R_RegisterShader("nailtrail");

	cgs.media.plasmaBallShader = trap_R_RegisterShader("sprites/plasma1");
	cgs.media.bloodTrailShader = trap_R_RegisterShader("particles/blood_trail");
	cgs.media.bloodSpurtShader = trap_R_RegisterShader("particles/blood_spurt");
	cgs.media.bloodSpurt2Shader = trap_R_RegisterShader("particles/blood_spurt");
	cgs.media.bloodSpurt3Shader = trap_R_RegisterShader("particles/blood_spurt");
	cgs.media.lagometerShader = trap_R_RegisterShader("lagometer");
	cgs.media.lagometer_lagShader = trap_R_RegisterShader("lagometer_lag");
	cgs.media.connectionShader = trap_R_RegisterShader("disconnected");

	cgs.media.waterBubbleShader = trap_R_RegisterShader("waterBubble");

	cgs.media.tracerShader = trap_R_RegisterShader("gfx/misc/tracer");
	cgs.media.selectShader = trap_R_RegisterShader("gfx/2d/select");
	cgs.media.weaponSelectShader = trap_R_RegisterShader("gfx/2d/weapon_select");

	cgs.media.sparkShader = trap_R_RegisterShader("particles/glow");

	cgs.media.bloodExplosionShader = trap_R_RegisterShader("bloodExplosion");

	cgs.media.teleportFlareShader = trap_R_RegisterShader("particles/flare2");

	// globe mapping shaders
	cgs.media.shadowProjectedLightShader = trap_R_RegisterShaderLightAttenuation("lights/shadowProjectedLight");

	CG_LoadingString("Particles", qfalse);
	CG_InitParticles();

	CG_LoadingString("Powerups", qfalse);

	// powerup shaders
	cgs.media.quadShader = trap_R_RegisterShader("powerups/quad");
	cgs.media.quadWeaponShader = trap_R_RegisterShader("powerups/quadWeapon");
	cgs.media.battleSuitShader = trap_R_RegisterShader("powerups/battleSuit");
	cgs.media.battleWeaponShader = trap_R_RegisterShader("powerups/battleWeapon");
	cgs.media.invisShader = trap_R_RegisterShader("powerups/invisibility");
	cgs.media.regenShader = trap_R_RegisterShader("powerups/regen");
	cgs.media.hastePuffShader = trap_R_RegisterShader("hasteSmokePuff");

	// effects
	cgs.media.unlinkEffect = trap_R_RegisterShader("player/unlink_effect");

	CG_LoadingString("Decals", qfalse);

	// wall marks
	cgs.media.bulletMarkShader = trap_R_RegisterShader("gfx/damage/bullet_mrk");
	cgs.media.burnMarkShader = trap_R_RegisterShader("burnMark");
	cgs.media.holeMarkShader = trap_R_RegisterShader("gfx/damage/hole_lg_mrk");
	cgs.media.energyMarkShader = trap_R_RegisterShader("gfx/damage/plasma_mrk");
	cgs.media.shadowMarkShader = trap_R_RegisterShader("markShadow");
	cgs.media.wakeMarkShader = trap_R_RegisterShader("wake");
	cgs.media.bloodMarkShader = trap_R_RegisterShader("textures/decals/blood_splat04");
	cgs.media.bloodMark2Shader = trap_R_RegisterShader("textures/decals/blood_splat05");
	cgs.media.bloodMark3Shader = trap_R_RegisterShader("textures/decals/blood_splat06");

	CG_LoadingString("Map Models", qfalse);

	// register the inline models
	cgs.numInlineModels = trap_CM_NumInlineModels();
	for(i = 1; i < cgs.numInlineModels; i++)
	{
		char            name[10];
		vec3_t          mins, maxs;
		int             j;

		Com_sprintf(name, sizeof(name), "*%i", i);
		cgs.inlineDrawModel[i] = trap_R_RegisterModel(name);
		trap_R_ModelBounds(cgs.inlineDrawModel[i], mins, maxs);
		for(j = 0; j < 3; j++)
		{
			cgs.inlineModelMidpoints[i][j] = mins[j] + 0.5 * (maxs[j] - mins[j]);
		}
	}

	// register all the server specified models
	for(i = 1; i < MAX_MODELS; i++)
	{
		const char     *modelName;

		modelName = CG_ConfigString(CS_MODELS + i);
		if(!modelName[0])
		{
			break;
		}
		cgs.gameModels[i] = trap_R_RegisterModel(modelName);
	}

	CG_LoadingString("Debris", qfalse);

	// Debris models
	// Consider moving these loads into the code that initializes a func_explosive, so they are only loaded when required.
	// Also, if theres not enough models, point some of these to others so the code still calls them fine. Eg, Gibs.
	cgs.media.debrisModels[ENTMAT_WOOD][0][0] = trap_R_RegisterModel("models/debris/wood1a.md5mesh");
	cgs.media.debrisModels[ENTMAT_WOOD][0][1] = trap_R_RegisterModel("models/debris/wood1b.md5mesh");
	cgs.media.debrisModels[ENTMAT_WOOD][1][0] = trap_R_RegisterModel("models/debris/wood2a.md5mesh");
	cgs.media.debrisModels[ENTMAT_WOOD][1][1] = trap_R_RegisterModel("models/debris/wood2b.md5mesh");
	cgs.media.debrisModels[ENTMAT_WOOD][2][0] = trap_R_RegisterModel("models/debris/wood3a.md5mesh");
	cgs.media.debrisModels[ENTMAT_WOOD][2][1] = trap_R_RegisterModel("models/debris/wood3b.md5mesh");

	/*
	cgs.media.debrisModels[ENTMAT_GLASS][0][0] = trap_R_RegisterModel("models/debris/glass1a.md5mesh");
	cgs.media.debrisModels[ENTMAT_GLASS][0][1] = trap_R_RegisterModel("models/debris/glass1b.md5mesh");
	cgs.media.debrisModels[ENTMAT_GLASS][1][0] = trap_R_RegisterModel("models/debris/glass2a.md5mesh");
	cgs.media.debrisModels[ENTMAT_GLASS][1][1] = trap_R_RegisterModel("models/debris/glass2b.md5mesh");
	cgs.media.debrisModels[ENTMAT_GLASS][2][0] = trap_R_RegisterModel("models/debris/glass3a.md5mesh");
	cgs.media.debrisModels[ENTMAT_GLASS][2][1] = trap_R_RegisterModel("models/debris/glass3b.md5mesh");

	cgs.media.debrisModels[ENTMAT_METAL][0][0] = trap_R_RegisterModel("models/debris/metal1a.md5mesh");
	cgs.media.debrisModels[ENTMAT_METAL][0][1] = trap_R_RegisterModel("models/debris/metal1b.md5mesh");
	cgs.media.debrisModels[ENTMAT_METAL][1][0] = trap_R_RegisterModel("models/debris/metal2a.md5mesh");
	cgs.media.debrisModels[ENTMAT_METAL][1][1] = trap_R_RegisterModel("models/debris/metal2b.md5mesh");
	cgs.media.debrisModels[ENTMAT_METAL][2][0] = trap_R_RegisterModel("models/debris/metal3a.md5mesh");
	cgs.media.debrisModels[ENTMAT_METAL][2][1] = trap_R_RegisterModel("models/debris/metal3b.md5mesh");

	cgs.media.debrisModels[ENTMAT_GIBS][0][0] = cgs.media.gibIntestine;
	cgs.media.debrisModels[ENTMAT_GIBS][0][1] = cgs.media.gibLeg;
	cgs.media.debrisModels[ENTMAT_GIBS][1][0] = cgs.media.gibBrain;
	cgs.media.debrisModels[ENTMAT_GIBS][1][1] = cgs.media.gibSkull;
	cgs.media.debrisModels[ENTMAT_GIBS][2][0] = cgs.media.gibAbdomen;
	cgs.media.debrisModels[ENTMAT_GIBS][2][1] = cgs.media.gibChest;

	cgs.media.debrisModels[ENTMAT_BODY][0][0] = trap_R_RegisterModel("models/debris/body1a.md5mesh");
	cgs.media.debrisModels[ENTMAT_BODY][0][1] = trap_R_RegisterModel("models/debris/body1b.md5mesh");
	cgs.media.debrisModels[ENTMAT_BODY][1][0] = trap_R_RegisterModel("models/debris/body2a.md5mesh");
	cgs.media.debrisModels[ENTMAT_BODY][1][1] = trap_R_RegisterModel("models/debris/body2b.md5mesh");
	cgs.media.debrisModels[ENTMAT_BODY][2][0] = trap_R_RegisterModel("models/debris/body3a.md5mesh");
	cgs.media.debrisModels[ENTMAT_BODY][2][1] = trap_R_RegisterModel("models/debris/body3b.md5mesh");
	*/

	cgs.media.debrisModels[ENTMAT_BRICK][0][0] = trap_R_RegisterModel("models/debris/brick1a.md5mesh");
	cgs.media.debrisModels[ENTMAT_BRICK][0][1] = trap_R_RegisterModel("models/debris/brick1b.md5mesh");
	cgs.media.debrisModels[ENTMAT_BRICK][1][0] = trap_R_RegisterModel("models/debris/brick2a.md5mesh");
	cgs.media.debrisModels[ENTMAT_BRICK][1][1] = trap_R_RegisterModel("models/debris/brick2b.md5mesh");
	cgs.media.debrisModels[ENTMAT_BRICK][2][0] = trap_R_RegisterModel("models/debris/brick3a.md5mesh");
	cgs.media.debrisModels[ENTMAT_BRICK][2][1] = trap_R_RegisterModel("models/debris/brick3b.md5mesh");

	cgs.media.debrisModels[ENTMAT_STONE][0][0] = trap_R_RegisterModel("models/debris/stone1a.md5mesh");
	cgs.media.debrisModels[ENTMAT_STONE][0][1] = trap_R_RegisterModel("models/debris/stone1b.md5mesh");
	cgs.media.debrisModels[ENTMAT_STONE][1][0] = trap_R_RegisterModel("models/debris/stone2a.md5mesh");
	cgs.media.debrisModels[ENTMAT_STONE][1][1] = trap_R_RegisterModel("models/debris/stone2b.md5mesh");
	cgs.media.debrisModels[ENTMAT_STONE][2][0] = trap_R_RegisterModel("models/debris/stone3a.md5mesh");
	cgs.media.debrisModels[ENTMAT_STONE][2][1] = trap_R_RegisterModel("models/debris/stone3b.md5mesh");

	/*
	cgs.media.debrisModels[ENTMAT_TILES][0][0] = trap_R_RegisterModel("models/debris/tiles1a.md5mesh");
	cgs.media.debrisModels[ENTMAT_TILES][0][1] = trap_R_RegisterModel("models/debris/tiles1b.md5mesh");
	cgs.media.debrisModels[ENTMAT_TILES][1][0] = trap_R_RegisterModel("models/debris/tiles2a.md5mesh");
	cgs.media.debrisModels[ENTMAT_TILES][1][1] = trap_R_RegisterModel("models/debris/tiles2b.md5mesh");
	cgs.media.debrisModels[ENTMAT_TILES][2][0] = trap_R_RegisterModel("models/debris/tiles3a.md5mesh");
	cgs.media.debrisModels[ENTMAT_TILES][2][1] = trap_R_RegisterModel("models/debris/tiles3b.md5mesh");

	cgs.media.debrisModels[ENTMAT_PLASTER][0][0] = trap_R_RegisterModel("models/debris/plaster1a.md5mesh");
	cgs.media.debrisModels[ENTMAT_PLASTER][0][1] = trap_R_RegisterModel("models/debris/plaster1b.md5mesh");
	cgs.media.debrisModels[ENTMAT_PLASTER][1][0] = trap_R_RegisterModel("models/debris/plaster2a.md5mesh");
	cgs.media.debrisModels[ENTMAT_PLASTER][1][1] = trap_R_RegisterModel("models/debris/plaster2b.md5mesh");
	cgs.media.debrisModels[ENTMAT_PLASTER][2][0] = trap_R_RegisterModel("models/debris/plaster3a.md5mesh");
	cgs.media.debrisModels[ENTMAT_PLASTER][2][1] = trap_R_RegisterModel("models/debris/plaster3b.md5mesh");

	cgs.media.debrisModels[ENTMAT_FIBERS][0][0] = trap_R_RegisterModel("models/debris/fibers1a.md5mesh");
	cgs.media.debrisModels[ENTMAT_FIBERS][0][1] = trap_R_RegisterModel("models/debris/fibers1b.md5mesh");
	cgs.media.debrisModels[ENTMAT_FIBERS][1][0] = trap_R_RegisterModel("models/debris/fibers2a.md5mesh");
	cgs.media.debrisModels[ENTMAT_FIBERS][1][1] = trap_R_RegisterModel("models/debris/fibers2b.md5mesh");
	cgs.media.debrisModels[ENTMAT_FIBERS][2][0] = trap_R_RegisterModel("models/debris/fibers3a.md5mesh");
	cgs.media.debrisModels[ENTMAT_FIBERS][2][1] = trap_R_RegisterModel("models/debris/fibers3b.md5mesh");
	*/

	cgs.media.debrisBit = trap_R_RegisterShader("models/debris/debrisbit");
	cgs.media.debrisPlaster = trap_R_RegisterShader("models/debris/plaster1");

	cgs.media.fire = trap_R_RegisterShader("fire1");
	cgs.media.fireLight = trap_R_RegisterShader("fire1/light");
	cgs.media.flames[0] = trap_R_RegisterShader("flames0");
	cgs.media.flames[1] = trap_R_RegisterShader("flames1");
	cgs.media.flames[2] = trap_R_RegisterShader("flames2");

	CG_LoadingString("Misc", qfalse);

	// capture point flags
	cgs.media.capturePointFlag[TEAM_FREE] = trap_R_RegisterModel("models/entities/flag_neut.md5mesh");
	cgs.media.capturePointFlag[TEAM_RED] = trap_R_RegisterModel("models/entities/flag_red.md5mesh");
	cgs.media.capturePointFlag[TEAM_BLUE] = trap_R_RegisterModel("models/entities/flag_blue.md5mesh");
	cgs.media.objItemIcon = trap_R_RegisterShader("sprites/objitemicon");
	cgs.media.spell_frame[0] = trap_R_RegisterShader("gfx/icons/spell_frame/unselect");
	cgs.media.spell_frame[1] = trap_R_RegisterShader("gfx/icons/spell_frame/select");
	cgs.media.spell_frame[2] = trap_R_RegisterShader("gfx/icons/spell_frame/unselect_process");
	cgs.media.spell_frame[3] = trap_R_RegisterShader("gfx/icons/spell_frame/select_process");

	// weaver sword
	cgs.media.swordModel = trap_R_RegisterModel("models/sword/sword.md5mesh");
	cgs.media.weaverShieldInfo = trap_R_RegisterModel("models/entities/shield_info.md5mesh");

	// debug utils
	cgs.media.lightningShader = trap_R_RegisterShader("lightningBolt");
	cgs.media.debugPlayerAABB = trap_R_RegisterShader("debugPlayerAABB");
	cgs.media.debugPlayerAABB_twoSided = trap_R_RegisterShader("debugPlayerAABB_twoSided");
}

/*
===================
CG_RegisterWeaves
===================
*/
static void CG_RegisterWeaves(void)
{
	CG_Printf("^3Loading Weaves...\n");

	CG_RegisterWeave(WVW_NONE);

	CG_RegisterWeave(WVW_D_AIR_PROTECT);
	CG_RegisterWeave(WVW_A_AIR_GRABPLAYER);
	CG_RegisterWeave(WVW_A_AIR_BLAST);

	CG_RegisterWeave(WVW_D_AIRFIRE_LIGHT);
	CG_RegisterWeave(WVW_A_AIRFIRE_LIGHTNING);

	CG_RegisterWeave(WVW_D_FIRE_PROTECT);
	CG_RegisterWeave(WVW_A_FIRE_BALL);
	CG_RegisterWeave(WVW_A_FIRE_DARTS);
	CG_RegisterWeave(WVW_A_FIRE_MULTIDARTS);

	CG_RegisterWeave(WVW_D_EARTH_PROTECT);
	CG_RegisterWeave(WVW_A_EARTH_QUAKE_S);
	CG_RegisterWeave(WVW_A_EARTH_QUAKE_M);
	CG_RegisterWeave(WVW_A_EARTH_QUAKE_L);

	CG_RegisterWeave(WVW_A_EARTHWATER_SLOW);
	CG_RegisterWeave(WVW_A_EARTHWATER_POISON);

	CG_RegisterWeave(WVW_D_EARTHFIRE_EXPLOSIVE_S);
	CG_RegisterWeave(WVW_D_EARTHFIRE_EXPLOSIVE_M);

	CG_RegisterWeave(WVW_D_WATER_PROTECT);
	CG_RegisterWeave(WVW_A_WATER_ICESHARDS_S);
	CG_RegisterWeave(WVW_A_WATER_ICESHARDS_M);
	CG_RegisterWeave(WVW_D_WATER_HEAL_S);
	CG_RegisterWeave(WVW_D_WATER_HEAL_M);
	CG_RegisterWeave(WVW_D_WATER_CURE);

	CG_RegisterWeave(WVW_A_AIRWATER_RIP);
	CG_RegisterWeave(WVW_D_AIRWATER_FOG);
	CG_RegisterWeave(WVW_A_AIRWATER_DARTS_S);
	CG_RegisterWeave(WVW_A_AIRWATER_DARTS_M);

	CG_RegisterWeave(WVW_A_SPIRIT_SLICE_S);
	CG_RegisterWeave(WVW_A_SPIRIT_SLICE_M);
	CG_RegisterWeave(WVW_A_SPIRIT_SLICE_L);
	CG_RegisterWeave(WVW_D_SPIRIT_STAMINA);
	CG_RegisterWeave(WVW_D_SPIRIT_LINK);
	CG_RegisterWeave(WVW_A_SPIRIT_SHIELD);

	CG_Printf("^3Weaves registered.\n");
}

/*
===================
CG_RegisterWeaves
===================
*/
static void CG_RegisterCommandAliases(void)
{
	trap_AddCommandAlias("+thread", "+button7");
	trap_AddCommandAlias("-thread", "-button7");
	trap_AddCommandAlias("+weave", "+button8");
	trap_AddCommandAlias("-weave", "-button8");
	trap_AddCommandAlias("+sprint", "+button9");
	trap_AddCommandAlias("-sprint", "-button9");
}

//===========================================================================


/*
=======================
CG_BuildSpectatorString
=======================
*/
void CG_BuildSpectatorString(void)
{
	int             i;

	cg.spectatorList[0] = 0;
	for(i = 0; i < MAX_CLIENTS; i++)
	{
		if(cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team == TEAM_SPECTATOR)
		{
			Q_strcat(cg.spectatorList, sizeof(cg.spectatorList), va("%s     ", cgs.clientinfo[i].name));
		}
	}
	i = strlen(cg.spectatorList);
	if(i != cg.spectatorLen)
	{
		cg.spectatorLen = i;
		cg.spectatorWidth = -1;
	}
}


/*
===================
CG_RegisterClients
===================
*/
static void CG_RegisterClients(void)
{
	int             i;

	CG_NewClientInfo(cg.clientNum);

	for(i = 0; i < MAX_CLIENTS; i++)
	{
		const char     *clientInfo;

		if(cg.clientNum == i)
		{
			continue;
		}

		clientInfo = CG_ConfigString(CS_PLAYERS + i);
		if(!clientInfo[0])
		{
			continue;
		}

		CG_LoadingString(va("precaching %i", i), qfalse);

		CG_NewClientInfo(i);
	}


	CG_BuildSpectatorString();
}

//===========================================================================

/*
=================
CG_ConfigString
=================
*/
const char     *CG_ConfigString(int index)
{
	if(index < 0 || index >= MAX_CONFIGSTRINGS)
	{
		CG_Error("CG_ConfigString: bad index: %i", index);
	}
	return cgs.gameState.stringData + cgs.gameState.stringOffsets[index];
}

//==================================================================

/*
======================
CG_StartMusic

======================
*/
void CG_StartMusic(void)
{
	char           *s;
	char            parm1[MAX_QPATH], parm2[MAX_QPATH];

	// start the background music
	s = (char *)CG_ConfigString(CS_MUSIC);
	Q_strncpyz(parm1, Com_Parse(&s), sizeof(parm1));
	Q_strncpyz(parm2, Com_Parse(&s), sizeof(parm2));

	trap_S_StartBackgroundTrack(parm1, parm2);
}

/*
=================
CG_Init

Called after every level change or subsystem restart
Will perform callbacks to make the loading info screen update.
=================
*/
void CG_Init(int serverMessageNum, int serverCommandSequence, int clientNum)
{
	const char     *s;

	CG_Printf("------- CGame Initialization -------\n");

	// clear everything
	memset(&cgs, 0, sizeof(cgs));
	memset(&cg, 0, sizeof(cg));
	memset(cg_entities, 0, sizeof(cg_entities));
	memset(cg_weapons, 0, sizeof(cg_weapons));
	memset(cg_items, 0, sizeof(cg_items));

	cg.clientNum = clientNum;

	cg.progress = 0;

	cgs.processedSnapshotNum = serverMessageNum;
	cgs.serverCommandSequence = serverCommandSequence;

	// load a few needed things before we do any screen updates
	cgs.media.charsetShader = trap_R_RegisterShader("gfx/2d/bigchars");
	cgs.media.whiteShader = trap_R_RegisterShader("white");
	cgs.media.charsetProp1 = trap_R_RegisterShaderNoMip("menu/art/font1_prop.tga");
	cgs.media.charsetProp1Glow = trap_R_RegisterShaderNoMip("menu/art/font1_prop_glo.tga");
	cgs.media.charsetProp2 = trap_R_RegisterShaderNoMip("menu/art/font2_prop.tga");

	// otty: register fonts here, otherwise CG_LoadingString wont work
	trap_R_RegisterFont("fonts/FreeSansBold.ttf", 48, &cgs.media.freeSansBoldFont);
	trap_R_RegisterFont("fonts/FreeSans.ttf", 48, &cgs.media.freeSansFont);
	trap_R_RegisterFont("fonts/FreeSerif.ttf", 48, &cgs.media.freeSerifFont);
	trap_R_RegisterFont("fonts/FreeSerifBold.ttf", 48, &cgs.media.freeSerifBoldFont);

	// otty: font for HUD Numbers
	trap_R_RegisterFont("fonts/FreeSans.ttf", 48, &cgs.media.hudNumberFont);


	CG_RegisterCvars();

	CG_RegisterCommandAliases();

	CG_InitConsoleCommands();

#ifdef CG_LUA
	CG_InitLua();
#endif

	//WEAVER
	//cg.weaponSelect = WP_MACHINEGUN;
	cg.weaponSelect = WP_NONE;

	cgs.redflag = cgs.blueflag = -1;	// For compatibily, default to unset for old servers
	cgs.flagStatus = -1;

	// get the rendering configuration from the client system
	trap_GetGlconfig(&cgs.glconfig);
	cgs.screenXSize = cgs.glconfig.vidWidth;
	cgs.screenYSize = cgs.glconfig.vidHeight;
	cgs.screenMinSize = (cgs.screenXSize > cgs.screenYSize) ? cgs.screenYSize : cgs.screenXSize;
	cgs.screenXScale = cgs.glconfig.vidWidth / 640.0;
	cgs.screenYScale = cgs.glconfig.vidHeight / 480.0;
	cgs.screenScale = cgs.glconfig.vidHeight * (1.0f / 480.0f);
	if(cgs.glconfig.vidWidth * 480 > cgs.glconfig.vidHeight * 640)
	{
		// wide screen
		cgs.screenXBias = 0.5f * (cgs.glconfig.vidWidth - (cgs.glconfig.vidHeight * (640.0f / 480.0f)));
		cgs.screenYBias = 0;
	}
	else if(cgs.glconfig.vidWidth * 480 < cgs.glconfig.vidHeight * 640)
	{
		// narrow screen
		cgs.screenXBias = 0;
		cgs.screenYBias = 0.5f * (cgs.glconfig.vidHeight - (cgs.glconfig.vidWidth * (480.0f / 640.0f)));
		cgs.screenScale = cgs.glconfig.vidWidth * (1.0f / 640.0f);
	}
	else
	{
		// no wide screen
		cgs.screenXBias = cgs.screenYBias = 0;
	}

	CG_HudSizesInit();
	CG_ScoreSizesInit();
	if(cgs.screenMinSize < 1000.0f)
	{
		float scale = cgs.screenMinSize / 1000.0f;
		CG_HudSizesScale(scale);
		CG_ScoreSizesScale(scale);
	}

	// get the gamestate from the client system
	trap_GetGameState(&cgs.gameState);


	// check version
	s = CG_ConfigString(CS_GAME_VERSION);
	if(strcmp(s, GAME_VERSION))
	{
		CG_Error("Client/Server game mismatch: %s/%s", GAME_VERSION, s);
	}

	s = CG_ConfigString(CS_LEVEL_START_TIME);
	cgs.levelStartTime = atoi(s);

	CG_ParseSWInfo();

	CG_ParseServerinfo();

	// load the new map
	CG_LoadingString("", qfalse);

	trap_CM_LoadMap(cgs.mapname);

	cg.loading = qtrue;			// force players to load instead of defer

	CG_LoadingString("Sounds", qtrue);

	CG_RegisterSounds();

	CG_LoadingString("Graphics", qtrue);

	CG_RegisterGraphics();

	CG_LoadingString("Weaves", qtrue);

	CG_RegisterWeaves();

	CG_LoadingString("Clients", qtrue);

	CG_RegisterClients();		// if low on memory, some clients will be deferred

	/*
	CG_LoadingString("osd", qtrue);

	CG_RegisterOSD();
	*/

	cg.loading = qfalse;		// future players will be deferred

	CG_LoadingString("Entities", qtrue);

	CG_InitLocalEntities();

	CG_LoadingString("Polys", qtrue);

	CG_InitMarkPolys();


	// Make sure we have update values (scores)
	CG_SetConfigValues();

	CG_LoadingString("Music", qtrue);

	CG_StartMusic();

	//CG_LoadingString("", qfalse);

	// remove the last loading update
	cg.progress = 0;

#ifdef MISSIONPACK
	CG_InitTeamChat();
#endif

	CG_ShaderStateChanged();

	trap_S_ClearLoopingSounds(qtrue);


}

/*
=================
CG_Shutdown

Called before every level change or subsystem restart
=================
*/
void CG_Shutdown(void)
{
	// some mods may need to do cleanup work here,
	// like closing files or archiving session data

	CG_Printf("------- CGame Shutdown -------\n");

#ifdef CG_LUA
	CG_ShutdownLua();
#endif
}


/*
==================
CG_EventHandling
==================
 type 0 - no event handling
      1 - team menu
      2 - hud editor

*/
#ifndef MISSIONPACK
void CG_EventHandling(int type)
{
}

void CG_KeyEvent(int key, qboolean down)
{
}

void CG_MouseEvent(int x, int y)
{
}
#endif
