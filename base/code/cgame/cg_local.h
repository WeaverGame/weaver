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

// cg_local.h
#include <q_shared.h>
#include <tr_types.h>
#include <bg_public.h>

#include "cg_public.h"
#include "../game/spell_common.h"
#include "../game/spell_shared.h"
#include "../game/objective_common.h"

// The entire cgame module is unloaded and reloaded on each level change,
// so there is NO persistant data between levels on the client side.
// If you absolutely need something stored, it can either be kept
// by the server in the server stored userinfos, or stashed in a cvar.

#define CG_FONT_THRESHOLD	0.1

#define	POWERUP_BLINKS		5

#define	POWERUP_BLINK_TIME	1000
#define	FADE_TIME			200
#define	PULSE_TIME			200
#define	DAMAGE_DEFLECT_TIME	100
#define	DAMAGE_RETURN_TIME	400
#define DAMAGE_TIME			500
#define	LAND_DEFLECT_TIME	150
#define	LAND_RETURN_TIME	300
#define	STEP_TIME			200
#define	DUCK_TIME			100
#define	PAIN_TWITCH_TIME	200
#define	WEAPON_SELECT_TIME	1400
#define	ITEM_SCALEUP_TIME	1000
#define	ZOOM_TIME			150
#define	ITEM_BLOB_TIME		200
#define	MUZZLE_FLASH_TIME	20
#define	SINK_TIME			1000	// time for fragments to sink into ground before going away
#define	ATTACKER_HEAD_TIME	10000
#define	REWARD_TIME			3000

#define	PULSE_SCALE			1.5	// amount to scale up the icons when activating

#define	MAX_STEP_CHANGE		32

#define	MAX_VERTS_ON_POLY	10
#define	MAX_MARK_POLYS		256

#define STAT_MINUS			10	// num frame for '-' stats digit

#define	ICON_SIZE			32
#define	CHAR_WIDTH			20
#define	CHAR_HEIGHT			20
#define CHAR_SPACE			16

#define	TEXT_ICON_SPACE		4

#define	NOTIFY_WIDTH		80
#define NOTIFY_HEIGHT		10

#define	CHAT_WIDTH			80
#define CHAT_HEIGHT			10

#define	TEAMCHAT_WIDTH		80
#define TEAMCHAT_HEIGHT		10

// very large characters
#define	GIANT_WIDTH			32
#define	GIANT_HEIGHT		48

#define	NUM_CROSSHAIRS		2

#define TEAM_OVERLAY_MAXNAME_WIDTH	12
#define TEAM_OVERLAY_MAXLOCATION_WIDTH	16

#define DEFAULT_REDTEAM_NAME		"Stroggs"
#define DEFAULT_BLUETEAM_NAME		"Pagans"

typedef enum
{
	FOOTSTEP_STONE,
	FOOTSTEP_BOOT,
	FOOTSTEP_FLESH,
	FOOTSTEP_MECH,
	FOOTSTEP_ENERGY,
	FOOTSTEP_METAL,
	FOOTSTEP_SPLASH,
	FOOTSTEP_WALLWALK,

	FOOTSTEP_TOTAL
} footstep_t;

typedef enum
{
	IMPACTSOUND_DEFAULT,
	IMPACTSOUND_METAL,
	IMPACTSOUND_FLESH
} impactSound_t;

//=================================================

// player entities need to track more information
// than any other type of entity.

// note that not every player entity is a client entity,
// because corpses after respawn are outside the normal
// client numbering range

// when changing animation, set animationStartTime to frameTime + lerping time
// The current lerp will finish out, then it will lerp to the new animation
typedef struct
{
	refSkeleton_t   oldSkeleton;
	int             oldFrame;
	int             oldFrameTime;	// time when ->oldFrame was exactly on

	refSkeleton_t   skeleton;
	int             frame;
	int             frameTime;	// time when ->frame will be exactly on

	float           backlerp;

	float           yawAngle;
	qboolean        yawing;
	float           pitchAngle;
	qboolean        pitching;

	int             animationNumber;	// may include ANIM_TOGGLEBIT
	animation_t    *animation;
	int             animationStartTime;	// time when the first frame of the animation will be exact
	float           animationScale;

	// added for smooth blending between animations on change

	int             old_animationNumber;	// may include ANIM_TOGGLEBIT
	animation_t    *old_animation;

	float           blendlerp;
	float           blendtime;

	int             weaponNumber;
	int             old_weaponNumber;
} lerpFrame_t;

// debugging values:

int             debug_anim_current;
int             debug_anim_old;
float           debug_anim_blend;


// player entities need to track more information
// than any other type of entity.

// note that not every player entity is a client entity,
// because corpses after respawn are outside the normal
// client numbering range

#define   MAX_PLAYER_WEAVE_EFFECTS 1

//TA: smoothing of view and model for WW transitions
#define   MAXSMOOTHS          32

typedef struct
{
	float           time;
	float           timeMod;

	vec3_t          rotAxis;
	float           rotAngle;
} smooth_t;

typedef struct
{
	lerpFrame_t     legs, torso, flag, gun;
	int             painTime;
	int             painDirection;	// flip from 0 to 1
	int             lightningFiring;

	// railgun trail spawning
	vec3_t          railgunImpact;
	qboolean        railgunFlash;

	// machinegun spinning
	float           barrelAngle;
	int             barrelTime;
	qboolean        barrelSpinning;

	// death effect
	int             deathTime;
	float           deathScale;

	// wallwalk
	vec3_t          lastNormal;
	vec3_t          lastAxis[3];
	smooth_t        sList[MAXSMOOTHS];

	// objectives
	int             objEnt;

	// casting effects
	int             threads[MAX_THREADS];
	int             threadsLight;
	vec3_t          threadsColor;

	// held weaves
	int             heldWeaveEnt[HELD_MAX];

	// weaves on player
	qboolean        protectWeaveOn[4];
	int             protectWeaveEnt[4];
	int             weaveEffectEnt[MAX_PLAYER_WEAVE_EFFECTS];
} playerEntity_t;

//=================================================



// centity_t have a direct corespondence with gentity_t in the game, but
// only the entityState_t is directly communicated to the cgame
typedef struct centity_s
{
	entityState_t   currentState;	// from cg.frame
	entityState_t   nextState;	// from cg.nextFrame, if available
	qboolean        interpolate;	// true if next is valid to interpolate to
	qboolean        currentValid;	// true if cg.frame holds this entity

	int             muzzleFlashTime;	// move to playerEntity?
	int             previousEvent;
	int             teleportFlag;

	int             trailTime;	// so missile trails can handle dropped initial packets
	int             dustTrailTime;
	int             miscTime;

	int             snapShotTime;	// last time this entity was found in a snapshot

	playerEntity_t  pe;

	int             errorTime;	// decay the error from this time
	vec3_t          errorOrigin;
	vec3_t          errorAngles;

	qboolean        extrapolated;	// false if origin / angles is an interpolation
	vec3_t          rawOrigin;
	vec3_t          rawAngles;

	vec3_t          beamEnd;

	// exact interpolated position of entity on this frame
	vec3_t          lerpOrigin;
	vec3_t          lerpAngles;

} centity_t;


//======================================================================

// local entities are created as a result of events or predicted actions,
// and live independantly from all server transmitted entities

typedef struct markPoly_s
{
	struct markPoly_s *prevMark, *nextMark;
	int             time;
	qhandle_t       markShader;
	qboolean        alphaFade;	// fade alpha instead of rgb
	float           color[4];
	poly_t          poly;
	polyVert_t      verts[MAX_VERTS_ON_POLY];
} markPoly_t;


typedef enum
{
	LE_MARK,
	LE_EXPLOSION,
	LE_SPRITE_EXPLOSION,
	LE_FRAGMENT,
	LE_MOVE_SCALE_FADE,
	LE_FALL_SCALE_FADE,
	LE_FADE_RGB,
	LE_SCALE_FADE,
	LE_SCOREPLUM,
	LE_KAMIKAZE,
	LE_RAILEXPLOSION,
	LE_FIRE,
	LE_MOVING_REFENT,
	LE_SHOWREFENTITY
} leType_t;

typedef enum
{
	LEF_PUFF_DONT_SCALE = 0x0001,	// do not scale size over time
	LEF_TUMBLE = 0x0002,		// tumble over time, used for ejecting shells
	LEF_SOUND1 = 0x0004,		// sound 1 for kamikaze
	LEF_SOUND2 = 0x0008			// sound 2 for kamikaze
} leFlag_t;

typedef enum
{
	LEMT_NONE,
	LEMT_BURN,
	LEMT_BLOOD
} leMarkType_t;					// fragment local entities can leave marks on walls

typedef enum
{
	LEBS_NONE,
	LEBS_BLOOD,
	LEBS_BRASS
} leBounceSoundType_t;			// fragment local entities can make sounds on impacts

typedef struct localEntity_s
{
	struct localEntity_s *prev, *next;
	leType_t        leType;
	int             leFlags;

	int             startTime;
	int             endTime;
	int             fadeInTime;

	float           lifeRate;	// 1.0 / (endTime - startTime)

	trajectory_t    pos;
	trajectory_t    angles;

	// Tr3B - added from http://www.icculus.org/~phaethon/q3a/misc/quats.html
	quat_t          quatOrient;
	quat_t          quatRot;
	vec3_t          rotAxis;
	float           angVel;

	float           bounceFactor;	// 0.0 = no bounce, 1.0 = perfect

	float           color[4];

	float           radius;

	float           length;

	float           light;
	vec3_t          lightColor;

	leMarkType_t    leMarkType;	// mark to leave on fragment impact
	leBounceSoundType_t leBounceSoundType;

	refEntity_t     refEntity;
} localEntity_t;

//======================================================================


typedef struct
{
	int             client;
	int             score;
	int             ping;
	int             time;
	int             scoreFlags;
	int             powerUps;
	int             accuracy;
	int             impressiveCount;
	int             excellentCount;
	int             guantletCount;
	int             defendCount;
	int             assistCount;
	int             captures;
	qboolean        perfect;
	int             team;
} score_t;

// each client has an associated clientInfo_t
// that contains media references necessary to present the
// client model and other color coded effects
// this is regenerated each time a client's configstring changes,
// usually as a result of a userinfo (name, model, etc) change
#define	MAX_CUSTOM_SOUNDS	32

typedef struct
{
	qboolean        infoValid;

	char            name[MAX_QPATH];
	team_t          team;

	int             botSkill;	// 0 = not bot, 1-5 = bot

	vec3_t          color1;
	vec3_t          color2;

	int             score;		// updated by score servercmds
	int             location;	// location index for team mode
	int             health;		// you only get this info about your teammates
	int             armor;
	int             curWeapon;

	int             handicap;
	int             wins, losses;	// in tourney mode

	int             teamTask;	// task in teamplay (offence/defence)
	qboolean        teamLeader;	// true when this is a team leader

	int             powerups;	// so can display quad/flag status

	int             medkitUsageTime;
	int             invulnerabilityStartTime;
	int             invulnerabilityStopTime;

	int             breathPuffTime;

	// when clientinfo is changed, the loading of models/skins/sounds
	// can be deferred until you are dead, to prevent hitches in
	// gameplay
	char            modelName[MAX_QPATH];
	char            skinName[MAX_QPATH];
	char            redTeam[MAX_TEAMNAME];
	char            blueTeam[MAX_TEAMNAME];
	qboolean        deferred;

	qboolean        newAnims;	// true if using the new mission pack animations
	qboolean        fixedlegs;	// true if legs yaw is always the same as torso yaw
	qboolean        fixedtorso;	// true if torso never changes yaw

	vec3_t          headOffset;	// move head in icon views
	footstep_t      footsteps;
	gender_t        gender;		// from model

	// Tr3B: don't forget to add these values to CG_CopyClientInfoModel !
	char            firstTorsoBoneName[MAX_QPATH];
	char            lastTorsoBoneName[MAX_QPATH];

	char            torsoControlBoneName[MAX_QPATH];
	char            neckControlBoneName[MAX_QPATH];

	vec3_t          modelScale;

	qhandle_t       bodyModel;
	qhandle_t       bodySkin;

	qhandle_t       modelIcon;

	animation_t     animations[MAX_PLAYER_ANIMATIONS];

	sfxHandle_t     sounds[MAX_CUSTOM_SOUNDS];
} clientInfo_t;


// each WP_* weapon enum has an associated weaponInfo_t
// that contains media references necessary to present the
// weapon and its effects
typedef struct weaponInfo_s
{
	qboolean        registered;
	gitem_t        *item;

	qhandle_t       handsModel;	// the hands don't actually draw, they just position the weapon
	qhandle_t       weaponModel;
	qhandle_t       barrelModel;
	qhandle_t       flashModel;

	qhandle_t       viewModel;
	animation_t     viewModel_animations[MAX_WEAPON_STATES];

	vec3_t          weaponMidpoint;	// so it will rotate centered instead of by tag

	float           flashLight;
	vec3_t          flashLightColor;
	sfxHandle_t     flashSound[10];	// fast firing weapons randomly choose

	float           flashLight2;
	vec3_t          flashLightColor2;
	sfxHandle_t     flashSound2[10];	// fast firing weapons randomly choose

	qhandle_t       weaponIcon;
	qhandle_t       ammoIcon;

	qhandle_t       ammoModel;

	qhandle_t       projectileModel;
	qhandle_t       projectileSkin;
	sfxHandle_t     projectileSound;
	void            (*projectileTrailFunc) (centity_t *, const struct weaponInfo_s * wi);
	float           projectileLight;
	vec3_t          projectileLightColor;
	int             projectileRenderfx;

	qhandle_t       projectileModel2;
	sfxHandle_t     projectileSound2;
	void            (*projectileTrailFunc2) (centity_t *, const struct weaponInfo_s * wi);
	float           projectileLight2;
	vec3_t          projectileLightColor2;
	int             projectileRenderfx2;

	void            (*ejectBrassFunc) (centity_t *);
	void            (*ejectBrassFunc2) (centity_t *);

	float           trailRadius;
	float           wiTrailTime;

	float           trailRadius2;
	float           wiTrailTime2;

	sfxHandle_t     readySound;
	sfxHandle_t     firingSound;
	qboolean        loopFireSound;
} weaponInfo_t;

#define MAX_WEAVE_MISSILE_MODELS 4
#define MAX_WEAVE_INSTANCE_MODELS 4

//Client weave info
typedef struct weaver_weaveCGInfo_s
{
	qboolean        registered;

	//weave info
	weaver_weaveInfo_t info;

	//misc resources
	qhandle_t       icon;
	sfxHandle_t     firingSound;

	//missiles
	int             missileModelCount;
	qhandle_t       missileModel[MAX_WEAVE_MISSILE_MODELS];
	float           missileLight;
	vec3_t          missileLightColor;
	sfxHandle_t     missileSound;
	int             missileRenderfx;

	//void            (*missileTrailFunc) ( centity_t * );
	qhandle_t       missileTrailShader[2];
	int             missileTrailStep;
	float           missileTrailRadius;
	//float         missileTrailRGB[3];
	float           missileTrailAlpha;
	int             missileTrailDuration;

	qhandle_t       exploModel;
	float           exploRadius;
	qhandle_t       exploMark;
	float           exploMarkRadius;
	qboolean        exploMarkAlphaFade;
	qhandle_t       exploSprite;
	sfxHandle_t     exploSfx;
	float           exploLight;
	vec3_t          exploLightColor;
	int             exploDuration;
	int             exploSparksDuration;
	int             exploSparksX;
	int             exploSparksY;
	int             exploSparksSpeed;
	int             exploSparksVelScale;
	int             exploSparksOffset;

	//instance
	qhandle_t       instanceModel[MAX_WEAVE_INSTANCE_MODELS];
	float           instanceLight;
	vec3_t          instanceLightColor;
	qhandle_t       instanceLightShader;
	int             instanceRenderfx;
	sfxHandle_t     instanceSound;
	qhandle_t       instanceShader[2];
} weaver_weaveCGInfo;

// each IT_* item has an associated itemInfo_t
// that constains media references necessary to present the
// item and its effects
typedef struct
{
	qboolean        registered;
	qhandle_t       models[MAX_ITEM_MODELS];
	qhandle_t       skins[MAX_ITEM_MODELS];
	lerpFrame_t     lerpFrame;
	qhandle_t       icon;
} itemInfo_t;


typedef struct
{
	int             itemNum;
} powerupInfo_t;


#define MAX_SKULLTRAIL		10

typedef struct
{
	vec3_t          positions[MAX_SKULLTRAIL];
	int             numpositions;
} skulltrail_t;


#define MAX_REWARDSTACK		10
#define MAX_SOUNDBUFFER		20

//======================================================================

typedef enum
{
	P_NONE,
	P_WEATHER,
	P_WEATHER_TURBULENT,
	P_WEATHER_FLURRY,
	P_FLAT,
	P_FLAT_SCALEUP,
	P_FLAT_SCALEUP_FADE,
	P_SMOKE,
	P_SMOKE_IMPACT,
	P_BLOOD,
	P_BUBBLE,
	P_BUBBLE_TURBULENT,
	P_SPRITE,
	P_SPARK,
	P_LIGHTSPARK
} particleType_t;

// particle flags
enum
{
	PF_UNDERWATER = BIT(0),
	PF_AIRONLY = BIT(1),
};

typedef struct particle_s
{
	particleType_t  type;
	int             flags;

	qhandle_t       pshader;

	float           time;
	float           endTime;

	vec3_t          org;
	vec3_t          oldOrg;

	vec3_t          vel;
	vec3_t          accel;

	vec4_t          color;
	vec4_t          colorVel;

	float           width;
	float           height;

	float           endWidth;
	float           endHeight;

	float           start;
	float           end;

	float           startfade;
	qboolean        rotate;
	int             snum;

	qboolean        link;

	int             roll;

	int             accumroll;

	float           bounceFactor;	// 0.0 = no bounce, 1.0 = perfect

	struct particle_s *next;
} cparticle_t;

//======================================================================

// all cg.stepTime, cg.duckTime, cg.landTime, etc are set to cg.time when the action
// occurs, and they will have visible effects for #define STEP_TIME or whatever msec after

#define MAX_PREDICTED_EVENTS	16

//unlagged - optimized prediction
#define NUM_SAVED_STATES (CMD_BACKUP + 2)
//unlagged - optimized prediction

// Loading screen constants
#define CG_INFO_LOGO_WIDTH 256
#define CG_INFO_LOADTILE_WIDTH 20
#define CG_INFO_BLACKBAR_H 64
#define CG_INFO_LEFT_X 20
#define CG_INFO_RIGHT_X 260
#define CG_INFO_DOT_W 12
#define CG_INFO_SIDE_GRADIENT_WIDTH 256

#define NUM_PROGRESS 30

typedef struct
{
	char            info[100];
	qboolean        strong;
} progressInfo_t;



#define MAX_OSD_GROUPS 8
#define MAX_OSD_GROUP_ENTRYS 8



typedef struct
{

	int             id;

	char            caption[64];
	char            parm[32];

	void            (*func) (char *s);

	float           angle;
	float           scale;
	vec3_t          dir;
	vec3_t          endpos;

} osd_entry_t;

typedef struct
{

	char            name[32];
	osd_entry_t     entrys[MAX_OSD_GROUP_ENTRYS];
	int             numEntrys;

	float           alpha, wish_alpha;

	vec3_t          start;
	float           radius;


} osd_group_t;


osd_group_t     osdGroups[MAX_OSD_GROUPS];
int             numOSDGroups;


typedef struct osd_s
{

	qboolean        input;

	osd_entry_t    *curEntry;
	osd_group_t    *curGroup;

	float           offset;
} osd_t;


typedef struct
{
	int             clientFrame;	// incremented each frame

	int             clientNum;

	qboolean        demoPlayback;
	qboolean        levelShot;	// taking a level menu screenshot
	int             deferredPlayerLoading;
	qboolean        loading;	// don't defer players at initial startup
	qboolean        intermissionStarted;	// don't play voice rewards, because game will end shortly

	// there are only one or two snapshot_t that are relevent at a time
	int             latestSnapshotNum;	// the number of snapshots the client system has received
	int             latestSnapshotTime;	// the time from latestSnapshotNum, so we don't need to read the snapshot yet

	snapshot_t     *snap;		// cg.snap->serverTime <= cg.time
	snapshot_t     *nextSnap;	// cg.nextSnap->serverTime > cg.time, or NULL
	snapshot_t      activeSnapshots[2];

	float           frameInterpolation;	// (float)( cg.time - cg.frame->serverTime ) / (cg.nextFrame->serverTime - cg.frame->serverTime)

	qboolean        thisFrameTeleport;
	qboolean        nextFrameTeleport;

	int             frametime;	// cg.time - cg.oldTime

	int             time;		// this is the time value that the client
	// is rendering at.
	int             oldTime;	// time at last frame, used for missile trails and prediction checking

	int             physicsTime;	// either cg.snap->time or cg.nextSnap->time

	int             timelimitWarnings;	// 5 min, 1 min, overtime
	int             fraglimitWarnings;

	qboolean        mapRestart;	// set on a map restart to set back the weapon

	qboolean        renderingThirdPerson;	// during deaths, chasecams, etc

	// prediction state
	qboolean        hyperspace;	// true if prediction has hit a trigger_teleport
	playerState_t   predictedPlayerState;
	centity_t       predictedPlayerEntity;
	qboolean        validPPS;	// clear until the first call to CG_PredictPlayerState
	int             predictedErrorTime;
	vec3_t          predictedError;

	int             eventSequence;
	int             predictableEvents[MAX_PREDICTED_EVENTS];

	float           stepChange;	// for stair up smoothing
	int             stepTime;

	float           duckChange;	// for duck viewheight smoothing
	int             duckTime;

	float           landChange;	// for landing hard
	int             landTime;

	// input state sent to server
	int             weaponSelect;

	// auto rotating items
	vec3_t          autoAngles;
	vec3_t          autoAxis[3];
	vec3_t          autoAnglesFast;
	vec3_t          autoAxisFast[3];

	// view rendering
	refdef_t        refdef;
	vec3_t          refdefViewAngles;	// will be converted to refdef.viewaxis

	// zoom key
	qboolean        zoomed;
	int             zoomTime;
	float           zoomSensitivity;

	// information screen text during loading
	progressInfo_t  progressInfo[NUM_PROGRESS];
	int             progress;

	// scoreboard
	int             scoresRequestTime;
	int             numScores;
	int             selectedScore;
	int             teamScores[2];
	score_t         scores[MAX_CLIENTS];
	qboolean        showScores;
	qboolean        scoreBoardShowing;
	int             scoreFadeTime;
	char            killerName[MAX_NAME_LENGTH];
	char            spectatorList[MAX_STRING_CHARS];	// list of names
	int             spectatorLen;	// length of list
	float           spectatorWidth;	// width in device units
	int             spectatorTime;	// next time to offset
	int             spectatorPaintX;	// current paint x
	int             spectatorPaintX2;	// current paint x
	int             spectatorOffset;	// current offset from start
	int             spectatorPaintLen;	// current offset from start

	// skull trails
	skulltrail_t    skulltrails[MAX_CLIENTS];

	// centerprinting
	int             centerPrintTime;
	int             centerPrintCharWidth;
	int             centerPrintY;
	char            centerPrint[1024];
	int             centerPrintLines;

	// low ammo warning state
	int             lowAmmoWarning;	// 1 = low, 2 = empty

	// kill timers for carnage reward
	int             lastKillTime;

	// crosshair client ID
	int             crosshairClientNum;
	int             crosshairClientTime;

	// powerup active flashing
	int             powerupActive;
	int             powerupTime;

	// attacking player
	int             attackerTime;
	int             voiceTime;

	// reward medals
	int             rewardStack;
	int             rewardTime;
	int             rewardCount[MAX_REWARDSTACK];
	qhandle_t       rewardShader[MAX_REWARDSTACK];
	qhandle_t       rewardSound[MAX_REWARDSTACK];

	// sound buffer mainly for announcer sounds
	int             soundBufferIn;
	int             soundBufferOut;
	int             soundTime;
	qhandle_t       soundBuffer[MAX_SOUNDBUFFER];

	// for voice chat buffer
	int             voiceChatTime;
	int             voiceChatBufferIn;
	int             voiceChatBufferOut;

	// warmup countdown
	int             warmup;
	int             warmupCount;

	//==========================

	int             itemPickup;
	int             itemPickupTime;
	int             itemPickupBlendTime;	// the pulse around the crosshair is timed seperately

	int             weaponSelectTime;
	int             weaponAnimation;
	int             weaponAnimationTime;

	// blend blobs
	float           damageTime;
	float           damageX, damageY, damageValue;

	// status bar head
	float           headYaw;
	float           headEndPitch;
	float           headEndYaw;
	int             headEndTime;
	float           headStartPitch;
	float           headStartYaw;
	int             headStartTime;

	// view movement
	float           v_dmg_time;
	float           v_dmg_pitch;
	float           v_dmg_roll;

	vec3_t          kick_angles;	// weapon kicks
	vec3_t          kick_origin;

	// temp working variables for player view
	float           bobfracsin;
	int             bobcycle;
	float           xyspeed;
	int             nextOrbitTime;

	//qboolean cameraMode;      // if rendering from a loaded camera


	// development tool
	refEntity_t     testModelEntity;
	char            testModelName[MAX_QPATH];
	qboolean        testGun;

	// this will only change the skeleton of testModelEntity
	char            testAnimationName[MAX_QPATH];
	qhandle_t       testAnimation;

	char            testAnimation2Name[MAX_QPATH];
	qhandle_t       testAnimation2;
	refSkeleton_t   testAnimation2Skeleton;

	// play with doom3 style light materials
	refLight_t      testLight;
	char            testLightName[MAX_QPATH];
	qboolean        testFlashLight;

	int             teamSpawnPrevious;
	int             teamSpawnPeriod;

//unlagged - optimized prediction
	int             lastPredictedCommand;
	int             lastServerTime;
	playerState_t   savedPmoveStates[NUM_SAVED_STATES];
	int             stateHead, stateTail;
//unlagged - optimized prediction

	// hud variables

	float           bar_offset;	// offset calculation for middle bar
	int             bar_count;	//number of items displayed in the bar
	int             scoreboard_offset;	// scoreboard scrolling

	// OSD
	osd_t           osd;

	// wallwalk
	vec3_t          lastNormal;	//TA: view smoothage
	vec3_t          lastVangles;	//TA: view smoothage
	smooth_t        sList[MAXSMOOTHS];	//TA: WW smoothing

	int             spawnTime;	//TA: fovwarp

} cg_t;


// all of the model, shader, and sound references that are
// loaded at gamestate time are stored in cgMedia_t
// Other media that can be tied to clients, weapons, or items are
// stored in the clientInfo_t, itemInfo_t, weaponInfo_t, and powerupInfo_t
typedef struct
{
	qhandle_t       charsetShader;
	qhandle_t       charsetProp1;
	qhandle_t       charsetProp1Glow;
	qhandle_t       charsetProp2;
	qhandle_t       whiteShader;

	qhandle_t       flagModel;
	animation_t     flagAnimations[MAX_FLAG_ANIMATIONS];
	qhandle_t       redFlagSkin;
	qhandle_t       blueFlagSkin;
	qhandle_t       neutralFlagSkin;

	qhandle_t       redFlagShader[3];
	qhandle_t       blueFlagShader[3];
	qhandle_t       flagShader[4];

	qhandle_t       redFlagBaseModel;
	qhandle_t       blueFlagBaseModel;
	qhandle_t       neutralFlagBaseModel;

	qhandle_t       overloadBaseModel;
	qhandle_t       overloadTargetModel;
	qhandle_t       overloadLightsModel;
	qhandle_t       overloadEnergyModel;

	qhandle_t       harvesterModel;
	qhandle_t       harvesterAnimation;
	qhandle_t       harvesterRedSkin;
	qhandle_t       harvesterBlueSkin;
	qhandle_t       harvesterNeutralModel;

	qhandle_t       harvesterSkullModel;
	qhandle_t       redSkullSkin;
	qhandle_t       blueSkullSkin;
	qhandle_t       redSkullIcon;
	qhandle_t       blueSkullIcon;

	qhandle_t       armorModel;
	qhandle_t       armorIcon;

	qhandle_t       teamStatusBar;

	qhandle_t       deferShader;

	// func_explosive debris
	qhandle_t       debrisModels[ENTMAT_NUMBER][3][2];
	qhandle_t       debrisBit;
	qhandle_t       debrisPlaster;

	qhandle_t       fire;
	qhandle_t       fireLight;
	qhandle_t       flames[3];

	// gib explosions
	qhandle_t       gibAbdomen;
	qhandle_t       gibArm;
	qhandle_t       gibChest;
	qhandle_t       gibFist;
	qhandle_t       gibFoot;
	qhandle_t       gibForearm;
	qhandle_t       gibIntestine;
	qhandle_t       gibLeg;
	qhandle_t       gibSkull;
	qhandle_t       gibBrain;

	qhandle_t       smoke2;

	qhandle_t       machinegunBrassModel;
	qhandle_t       shotgunBrassModel;

	qhandle_t       railRingsShader;
	qhandle_t       railRings2Shader;
	qhandle_t       railCoreShader;

	qhandle_t       lightningShader;

	qhandle_t       friendShader;

	qhandle_t       balloonShader;
	qhandle_t       connectionShader;

	qhandle_t       weaponSelectShader;
	qhandle_t       selectShader;
	qhandle_t       viewBloodShader;
	qhandle_t       tracerShader;

	qhandle_t       crosshairShader[NUM_CROSSHAIRS];


	//new hud shader
	qhandle_t       hud_top_team_middle;
	qhandle_t       hud_top_team_middle_overlay;
	qhandle_t       hud_top_team_left;
	qhandle_t       hud_top_team_left_overlay;
	qhandle_t       hud_top_ctf_left;
	qhandle_t       hud_top_ctf_right;
	qhandle_t       hud_top_team_right;
	qhandle_t       hud_top_team_right_overlay;
	qhandle_t       hud_top_ffa_middle;
	qhandle_t       hud_top_ffa_middle_overlay;
	qhandle_t       hud_top_ffa_left;
	qhandle_t       hud_top_ffa_left_overlay;
	qhandle_t       hud_top_ffa_right;
	qhandle_t       hud_top_ffa_right_overlay;

	qhandle_t       hud_bar_left;
	qhandle_t       hud_bar_left_overlay;
	qhandle_t       hud_bar_middle_middle;
	qhandle_t       hud_bar_middle_left_end;
	qhandle_t       hud_bar_middle_left_middle;
	qhandle_t       hud_bar_middle_left_right;
	qhandle_t       hud_bar_middle_right_left;
	qhandle_t       hud_bar_middle_right_middle;
	qhandle_t       hud_bar_middle_right_end;
	qhandle_t       hud_bar_middle_overlay;
	qhandle_t       hud_bar_right;
	qhandle_t       hud_bar_right_overlay;

	qhandle_t       hud_icon_health;
	qhandle_t       hud_icon_armor;

	qhandle_t       osd_button;
	qhandle_t       osd_button_focus;

	//scoreboard stuff
	qhandle_t       scoreboard_row;
	qhandle_t       scoreboard_top_red;
	qhandle_t       scoreboard_top_blue;
	qhandle_t       scoreboard_vs;

	qhandle_t       lagometerShader;
	qhandle_t       lagometer_lagShader;
	qhandle_t       backTileShader;
	qhandle_t       noammoShader;

	qhandle_t       sideBarItemShader;
	qhandle_t       sideBarItemSelectShader;
	qhandle_t       sideBarPowerupShader;

	qhandle_t       sparkShader;

	qhandle_t       smokePuffShader;
	qhandle_t       shotgunSmokePuffShader;
	qhandle_t       nailPuffShader;
	qhandle_t       plasmaBallShader;
	qhandle_t       waterBubbleShader;

	qhandle_t       bloodTrailShader;
	qhandle_t       bloodSpurtShader;
	qhandle_t       bloodSpurt2Shader;
	qhandle_t       bloodSpurt3Shader;

	// globe mapping shaders
	qhandle_t       shadowProjectedLightShader;

	qhandle_t       numberShaders[11];

	qhandle_t       shadowMarkShader;

	// wall mark shaders
	qhandle_t       wakeMarkShader;
	qhandle_t       bloodMarkShader;
	qhandle_t       bloodMark2Shader;
	qhandle_t       bloodMark3Shader;
	qhandle_t       bulletMarkShader;
	qhandle_t       burnMarkShader;
	qhandle_t       holeMarkShader;
	qhandle_t       energyMarkShader;

	// powerup shaders
	qhandle_t       quadShader;
	qhandle_t       redQuadShader;
	qhandle_t       quadWeaponShader;
	qhandle_t       invisShader;
	qhandle_t       regenShader;
	qhandle_t       battleSuitShader;
	qhandle_t       battleWeaponShader;
	qhandle_t       hastePuffShader;
	qhandle_t       redKamikazeShader;
	qhandle_t       blueKamikazeShader;

	//effect shaders
	qhandle_t       unlinkEffect;

	// weapon effect models
	qhandle_t       dishFlashModel;

	// weapon effect shaders
	qhandle_t       rocketExplosionShader;
	qhandle_t       grenadeExplosionShader;
	qhandle_t       bfgExplosionShader;
	qhandle_t       bloodExplosionShader;

	// special effects models
	qhandle_t       teleportFlareShader;
	qhandle_t       kamikazeEffectModel;
	qhandle_t       kamikazeShockWave;
	qhandle_t       kamikazeHeadModel;
	qhandle_t       kamikazeHeadTrail;
	qhandle_t       invulnerabilityPowerupModel;

	// medals shown during gameplay
	qhandle_t       medalImpressive;
	qhandle_t       medalExcellent;
	qhandle_t       medalGauntlet;
	qhandle_t       medalDefend;
	qhandle_t       medalAssist;
	qhandle_t       medalCapture;
	qhandle_t       medalTelefrag;

	// Tr3B: new truetype fonts
	fontInfo_t      freeSansBoldFont;
	fontInfo_t      freeSerifBoldFont;
	fontInfo_t      freeSerifFont;
	fontInfo_t      freeSansFont;

	// otty: new font for HUD
	fontInfo_t      hudNumberFont;

	// sounds
	sfxHandle_t     quadSound;
	sfxHandle_t     tracerSound;
	sfxHandle_t     selectSound;
	sfxHandle_t     useNothingSound;
	sfxHandle_t     wearOffSound;
	sfxHandle_t     footsteps[FOOTSTEP_TOTAL][4];
	sfxHandle_t     sfx_lghit1;
	sfxHandle_t     sfx_lghit2;
	sfxHandle_t     sfx_lghit3;
	sfxHandle_t     sfx_railg;
	sfxHandle_t     sfx_rockexp;
	sfxHandle_t     sfx_plasmaexp;
	sfxHandle_t     hookImpactSound;
	sfxHandle_t     impactFlesh1Sound;
	sfxHandle_t     impactFlesh2Sound;
	sfxHandle_t     impactFlesh3Sound;
	sfxHandle_t     impactMetal1Sound;
	sfxHandle_t     impactMetal2Sound;
	sfxHandle_t     impactMetal3Sound;
	sfxHandle_t     impactMetal4Sound;
	sfxHandle_t     impactWall1Sound;
	sfxHandle_t     impactWall2Sound;
	sfxHandle_t     sfx_nghit;
	sfxHandle_t     sfx_nghitflesh;
	sfxHandle_t     sfx_nghitmetal;

	sfxHandle_t     kamikazeExplodeSound;
	sfxHandle_t     kamikazeImplodeSound;
	sfxHandle_t     kamikazeFarSound;

	sfxHandle_t     obeliskHitSound1;
	sfxHandle_t     obeliskHitSound2;
	sfxHandle_t     obeliskHitSound3;
	sfxHandle_t     obeliskRespawnSound;

	sfxHandle_t     winnerSound;
	sfxHandle_t     loserSound;

	sfxHandle_t     gibSound;
	sfxHandle_t     gibBounce1Sound;
	sfxHandle_t     gibBounce2Sound;
	sfxHandle_t     gibBounce3Sound;
	sfxHandle_t     teleInSound;
	sfxHandle_t     teleOutSound;
	sfxHandle_t     noAmmoSound;
	sfxHandle_t     respawnSound;
	sfxHandle_t     talkSound;
	sfxHandle_t     fallSound;
	sfxHandle_t     jumpPadSound;

	sfxHandle_t     oneMinuteSound;
	sfxHandle_t     fiveMinuteSound;
	sfxHandle_t     suddenDeathSound;

	sfxHandle_t     threeFragSound;
	sfxHandle_t     twoFragSound;
	sfxHandle_t     oneFragSound;

	sfxHandle_t     hitSound;
	sfxHandle_t     hitSoundHighArmor;
	sfxHandle_t     hitSoundLowArmor;
	sfxHandle_t     hitTeamSound;
	sfxHandle_t     impressiveSound;
	sfxHandle_t     excellentSound;
	sfxHandle_t     deniedSound;
	sfxHandle_t     humiliationSound;
	sfxHandle_t     assistSound;
	sfxHandle_t     defendSound;
	sfxHandle_t     telefragSound;

	sfxHandle_t     takenLeadSound;
	sfxHandle_t     tiedLeadSound;
	sfxHandle_t     lostLeadSound;

	sfxHandle_t     voteNow;
	sfxHandle_t     votePassed;
	sfxHandle_t     voteFailed;

	sfxHandle_t     watrInSound;
	sfxHandle_t     watrOutSound;
	sfxHandle_t     watrUnSound;

	sfxHandle_t     flightSound;
	sfxHandle_t     medkitSound;

	sfxHandle_t     weaponHoverSound;

	// teamplay sounds
	sfxHandle_t     captureAwardSound;
	sfxHandle_t     redScoredSound;
	sfxHandle_t     blueScoredSound;
	sfxHandle_t     redLeadsSound;
	sfxHandle_t     blueLeadsSound;
	sfxHandle_t     teamsTiedSound;

	sfxHandle_t     captureYourTeamSound;
	sfxHandle_t     captureOpponentSound;
	sfxHandle_t     returnYourTeamSound;
	sfxHandle_t     returnOpponentSound;
	sfxHandle_t     takenYourTeamSound;
	sfxHandle_t     takenOpponentSound;

	sfxHandle_t     redFlagReturnedSound;
	sfxHandle_t     blueFlagReturnedSound;
	sfxHandle_t     neutralFlagReturnedSound;
	sfxHandle_t     enemyTookYourFlagSound;
	sfxHandle_t     enemyTookTheFlagSound;
	sfxHandle_t     yourTeamTookEnemyFlagSound;
	sfxHandle_t     yourTeamTookTheFlagSound;
	sfxHandle_t     youHaveFlagSound;
	sfxHandle_t     yourBaseIsUnderAttackSound;
	sfxHandle_t     holyShitSound;

	// tournament sounds
	sfxHandle_t     count3Sound;
	sfxHandle_t     count2Sound;
	sfxHandle_t     count1Sound;
	sfxHandle_t     countFightSound;
	sfxHandle_t     countPrepareSound;

	qhandle_t       cursor;
	qhandle_t       selectCursor;
	qhandle_t       sizeCursor;

	sfxHandle_t     regenSound;
	sfxHandle_t     protectSound;
	sfxHandle_t     n_healthSound;
	sfxHandle_t     hgrenb1aSound;
	sfxHandle_t     hgrenb2aSound;

	// debug utils
	qhandle_t       debugPlayerAABB;
	qhandle_t       debugPlayerAABB_twoSided;

	// weaver resorces
	qhandle_t       capturePointFlag[3];
	qhandle_t       objItemIcon;

	qhandle_t       spell_frame[4];

	// weaver sword
	qhandle_t       swordModel;

	// weaver disc
	qhandle_t       weaverDiscShader[NUM_WEAVERDISCS + 1];

	// weaver sense dots
	qhandle_t       weaverSenseDot1;
	qhandle_t       weaverSenseDot2;
	qhandle_t       weaverSenseDot3;

	qhandle_t       weaverShieldInfo;

	qhandle_t       weaverThreads[WVP_NUMBER][3];

	qhandle_t       weaverStatus[PW_NUM_POWERUPS];

	qhandle_t       weaverCorner;
	qhandle_t       weaverBarEnd;
	qhandle_t       weaverBarExt;
	qhandle_t       weaverBarExt0;
	qhandle_t       weaverBarDiv;

	qhandle_t       hpBot;
	qhandle_t       stBot;

} cgMedia_t;

typedef enum {
	CHAT_MODE_ALL,
	CHAT_MODE_TEAM,
	CHAT_MODE_TELL,
	CHAT_MODE_NUM
} chat_mode_t;

typedef enum {
	NOTIFY_MODE_CONNET,
	NOTIFY_MODE_JOINTEAM,
	NOTIFY_MODE_OBITUARY,
	NOTIFY_MODE_DISCONNECT,
	NOTIFY_MODE_OBJ_RED, // Red Success
	NOTIFY_MODE_OBJ_BLUE, // Blue Success
	NOTIFY_MODE_OBJ_OTHER,
	NOTIFY_MODE_NUM
} notify_mode_t;

// The client game static (cgs) structure hold everything
// loaded or calculated from the gamestate.  It will NOT
// be cleared when a tournement restart is done, allowing
// all clients to begin playing instantly
typedef struct
{
	gameState_t     gameState;	// gamestate from server
	glconfig_t      glconfig;	// rendering configuration
	float           screenScale;	// derived from glconfig
	float           screenXBias;
	float           screenYBias;
	float           screenXScale;
	float           screenYScale;
	float           screenXSize;	// screen width
	float           screenYSize;	// screen height
	float           screenMinSize;	// screen minimum of width/height

	int             serverCommandSequence;	// reliable command stream counter
	int             processedSnapshotNum;	// the number of snapshots cgame has requested

	qboolean        localServer;	// detected on startup by checking sv_running

	// parsed from serverinfo
	gametype_t      gametype;
	int             dmflags;
	int             teamflags;
	int             fraglimit;
	int             capturelimit;
	float           timelimit;
	int             maxclients;
	char            mapname[MAX_QPATH];
	char            redTeam[MAX_QPATH];
	char            blueTeam[MAX_QPATH];
	int             currentRound;
	int             defender;
	int             winner;

	int             voteTime;
	int             voteYes;
	int             voteNo;
	qboolean        voteModified;	// beep whenever changed
	char            voteString[MAX_STRING_TOKENS];

	int             teamVoteTime[2];
	int             teamVoteYes[2];
	int             teamVoteNo[2];
	qboolean        teamVoteModified[2];	// beep whenever changed
	char            teamVoteString[2][MAX_STRING_TOKENS];

	int             levelStartTime;

	int             scores1, scores2;	// from configstrings
	int             redflag, blueflag;	// flag status from configstrings
	int             flagStatus;

	qboolean        newHud;

	//
	// locally derived information from gamestate
	//
	qhandle_t       gameModels[MAX_MODELS];
	sfxHandle_t     gameSounds[MAX_SOUNDS];

	int             numInlineModels;
	qhandle_t       inlineDrawModel[MAX_MODELS];
	vec3_t          inlineModelMidpoints[MAX_MODELS];

	clientInfo_t    clientinfo[MAX_CLIENTS];

	// notify width is *3 because of embedded color codes
	char            notifyMsgs[NOTIFY_HEIGHT][NOTIFY_WIDTH * 3 + 1];
	int             notifyMsgTimes[NOTIFY_HEIGHT];
	int             notifyPos;
	int             notifyLastPos;
	notify_mode_t     notifyModes[NOTIFY_HEIGHT];

	// chat width is *3 because of embedded color codes
	char            chatMsgs[CHAT_HEIGHT][CHAT_WIDTH * 3 + 1];
	int             chatMsgTimes[CHAT_HEIGHT];
	int             chatPos;
	int             chatLastPos;
	chat_mode_t     chatModes[CHAT_HEIGHT];

#if 0
	// teamchat width is *3 because of embedded color codes
	char            teamChatMsgs[TEAMCHAT_HEIGHT][TEAMCHAT_WIDTH * 3 + 1];
	int             teamChatMsgTimes[TEAMCHAT_HEIGHT];
	int             teamChatPos;
	int             teamLastChatPos;
#endif

	int             cursorX;
	int             cursorY;
	qboolean        eventHandling;
	qboolean        mouseCaptured;
	qboolean        sizingHud;
	void           *capturedItem;
	qhandle_t       activeCursor;

	// orders
	int             currentOrder;
	qboolean        orderPending;
	int             orderTime;
	int             currentVoiceClient;
	int             acceptOrderTime;
	int             acceptTask;
	int             acceptLeader;
	char            acceptVoice[MAX_NAME_LENGTH];

	// media
	cgMedia_t       media;

//unlagged - client options
	// this will be set to the server's g_delagHitscan
	int             delagHitscan;
//unlagged - client options
} cgs_t;

//==============================================================================

extern cgs_t    cgs;
extern cg_t     cg;
extern centity_t cg_entities[MAX_GENTITIES];
extern weaponInfo_t cg_weapons[MAX_WEAPONS];
extern weaver_weaveCGInfo cg_weaves[WVW_NUM_WEAVES];
extern itemInfo_t cg_items[MAX_ITEMS];
extern markPoly_t cg_markPolys[MAX_MARK_POLYS];

extern vmCvar_t cg_centertime;
extern vmCvar_t cg_runpitch;
extern vmCvar_t cg_runroll;
extern vmCvar_t cg_bobup;
extern vmCvar_t cg_bobpitch;
extern vmCvar_t cg_bobroll;
extern vmCvar_t cg_swingSpeed;
extern vmCvar_t cg_shadows;
extern vmCvar_t cg_precomputedLighting;
extern vmCvar_t cg_gibs;
extern vmCvar_t cg_drawTimer;
extern vmCvar_t cg_drawFPS;
extern vmCvar_t cg_drawSnapshot;
extern vmCvar_t cg_draw3dIcons;
extern vmCvar_t cg_drawIcons;
extern vmCvar_t cg_drawAmmoWarning;
extern vmCvar_t cg_drawCrosshair;

extern vmCvar_t cg_hudRed;
extern vmCvar_t cg_hudGreen;
extern vmCvar_t cg_hudBlue;
extern vmCvar_t cg_hudAlpha;

extern vmCvar_t cg_drawCrosshairNames;
extern vmCvar_t cg_drawRewards;
extern vmCvar_t cg_drawTeamOverlay;
extern vmCvar_t cg_teamOverlayUserinfo;
extern vmCvar_t cg_crosshairX;
extern vmCvar_t cg_crosshairY;
extern vmCvar_t cg_crosshairSize;
extern vmCvar_t cg_crosshairHealth;

extern vmCvar_t cg_crosshairDot;
extern vmCvar_t cg_crosshairCircle;
extern vmCvar_t cg_crosshairCross;
extern vmCvar_t cg_crosshairPulse;

extern vmCvar_t cg_drawStatus;
extern vmCvar_t cg_drawStatusLines;
extern vmCvar_t cg_drawSideBar;
extern vmCvar_t cg_drawPickupItem;
extern vmCvar_t cg_drawWeaponSelect;
extern vmCvar_t cg_draw2D;
extern vmCvar_t cg_debugHUD;
extern vmCvar_t cg_animSpeed;
extern vmCvar_t cg_animBlend;
extern vmCvar_t cg_debugPlayerAnim;
extern vmCvar_t cg_debugWeaponAnim;
extern vmCvar_t cg_debugPosition;
extern vmCvar_t cg_debugEvents;
extern vmCvar_t cg_railTrailTime;
extern vmCvar_t cg_errorDecay;
extern vmCvar_t cg_nopredict;
extern vmCvar_t cg_noPlayerAnims;
extern vmCvar_t cg_showmiss;
extern vmCvar_t cg_footsteps;
extern vmCvar_t cg_addMarks;
extern vmCvar_t cg_brassTime;
extern vmCvar_t cg_gun_frame;
extern vmCvar_t cg_gunX;
extern vmCvar_t cg_gunY;
extern vmCvar_t cg_gunZ;
extern vmCvar_t cg_drawGun;
extern vmCvar_t cg_viewsize;
extern vmCvar_t cg_tracerChance;
extern vmCvar_t cg_tracerWidth;
extern vmCvar_t cg_tracerLength;
extern vmCvar_t cg_autoswitch;
extern vmCvar_t cg_ignore;
extern vmCvar_t cg_simpleItems;
extern vmCvar_t cg_fov;
extern vmCvar_t cg_zoomFov;
extern vmCvar_t cg_thirdPersonRange;
extern vmCvar_t cg_thirdPersonAngle;
extern vmCvar_t cg_thirdPerson;
extern vmCvar_t cg_stereoSeparation;
extern vmCvar_t cg_lagometer;
extern vmCvar_t cg_drawAttacker;
extern vmCvar_t cg_synchronousClients;
extern vmCvar_t cg_notifyTime;
extern vmCvar_t cg_notifyHeight;
extern vmCvar_t cg_chatTime;
extern vmCvar_t cg_chatHeight;
extern vmCvar_t cg_teamChatTime;
extern vmCvar_t cg_teamChatHeight;
extern vmCvar_t cg_stats;
extern vmCvar_t cg_forceModel;
extern vmCvar_t cg_forceBrightSkins;
extern vmCvar_t cg_buildScript;
extern vmCvar_t cg_blood;
extern vmCvar_t cg_predictItems;
extern vmCvar_t cg_deferPlayers;
extern vmCvar_t cg_drawFriend;
extern vmCvar_t cg_teamChatsOnly;
extern vmCvar_t cg_noVoiceChats;
extern vmCvar_t cg_noVoiceText;
extern vmCvar_t cg_scorePlum;

//unlagged - smooth clients #2
// this is done server-side now
//extern    vmCvar_t        cg_smoothClients;
//unlagged - smooth clients #2
extern vmCvar_t cg_cameraOrbit;
extern vmCvar_t cg_cameraOrbitDelay;
extern vmCvar_t cg_timescaleFadeEnd;
extern vmCvar_t cg_timescaleFadeSpeed;
extern vmCvar_t cg_timescale;
extern vmCvar_t cg_cameraMode;
extern vmCvar_t cg_noTaunt;
extern vmCvar_t cg_noProjectileTrail;
extern vmCvar_t cg_railType;
extern vmCvar_t cg_trueLightning;

extern vmCvar_t cg_particles;
extern vmCvar_t cg_particleCollision;

extern vmCvar_t pm_airControl;
extern vmCvar_t pm_fastWeaponSwitches;
extern vmCvar_t pm_fixedPmove;
extern vmCvar_t pm_fixedPmoveFPS;

extern vmCvar_t cg_gravityZ;

//weaver cvars
extern vmCvar_t cg_drawWeaverDisc;
extern vmCvar_t cg_drawWeaverThread;
extern vmCvar_t cg_weaverTutorial;
extern vmCvar_t cg_tutorial;
extern vmCvar_t cg_weaverDiscSize;
extern vmCvar_t cg_weaverDiscX;
extern vmCvar_t cg_weaverDiscY;
extern vmCvar_t cg_weaverSenseRadius;
extern vmCvar_t cg_weaverDiscMoveScale;
extern vmCvar_t cg_weaverThreadMoveScale;
extern vmCvar_t cg_weaverSpellSwitch;

extern vmCvar_t cg_redTeamName;
extern vmCvar_t cg_blueTeamName;
extern vmCvar_t cg_currentSelectedPlayer;
extern vmCvar_t cg_currentSelectedPlayerName;
extern vmCvar_t cg_singlePlayer;
extern vmCvar_t cg_singlePlayerActive;
extern vmCvar_t cg_enableDust;
extern vmCvar_t cg_enableBreath;
extern vmCvar_t cg_obeliskRespawnDelay;

//TeamColors
extern vec4_t   redTeamColor;
extern vec4_t   blueTeamColor;
extern vec4_t   baseTeamColor;

extern vmCvar_t cg_wallWalkSmoothTime;
extern vmCvar_t cg_drawPlayerCollision;

//unlagged - client options
extern vmCvar_t cg_delag;
extern vmCvar_t cg_debugDelag;
extern vmCvar_t cg_drawBBox;
extern vmCvar_t cg_cmdTimeNudge;
extern vmCvar_t sv_fps;
extern vmCvar_t cg_projectileNudge;
extern vmCvar_t cg_optimizePrediction;
extern vmCvar_t cl_timeNudge;
extern vmCvar_t cg_latentSnaps;
extern vmCvar_t cg_latentCmds;
extern vmCvar_t cg_plOut;
//unlagged - client options

//unlagged - cg_unlagged.c
void            CG_PredictWeaponEffects(centity_t * cent);
void            CG_AddBoundingBox(centity_t * cent);
qboolean        CG_Cvar_ClampInt(const char *name, vmCvar_t * vmCvar, int min, int max);

//unlagged - cg_unlagged.c

//
// cg_main.c
//
const char     *CG_ConfigString(int index);
const char     *CG_Argv(int arg);

void QDECL      CG_Printf(const char *msg, ...);
void QDECL      CG_Error(const char *msg, ...);

void            CG_StartMusic(void);

void            CG_UpdateCvars(void);

int             CG_CrosshairPlayer(void);
int             CG_LastAttacker(void);
void            CG_LoadMenus(const char *menuFile);
void            CG_KeyEvent(int key, qboolean down);
void            CG_MouseEvent(int x, int y);
void            CG_EventHandling(int type);
void            CG_RankRunFrame(void);
void            CG_SetScoreSelection(void *menu);
score_t        *CG_GetSelectedScore();
void            CG_BuildSpectatorString();


//
// cg_animation.c
//
qboolean        CG_RegisterAnimation(animation_t * anim, const char *filename,
									 qboolean loop, qboolean reversed, qboolean clearOrigin);

void            CG_SetLerpFrameAnimation(lerpFrame_t * lf, animation_t * anims, int animsNum, int newAnimation);
void            CG_RunLerpFrame(lerpFrame_t * lf, animation_t * anims, int animsNum, int newAnimation, float speedScale);


//
// cg_view.c
//
void            CG_TestModel_f(void);
void            CG_TestGun_f(void);
void            CG_TestModelNextFrame_f(void);
void            CG_TestModelPrevFrame_f(void);
void            CG_TestModelIncreaseLerp_f(void);
void            CG_TestModelDecreaseLerp_f(void);
void            CG_TestModelNextSkin_f(void);
void            CG_TestModelPrevSkin_f(void);
void            CG_TestAnimation_f(void);
void            CG_TestBlend_f(void);
void            CG_TestOmniLight_f(void);
void            CG_TestProjLight_f(void);
void            CG_TestFlashLight_f(void);
void            CG_TestGib_f(void);
void            CG_ZoomDown_f(void);
void            CG_ZoomUp_f(void);
void            CG_AddBufferedSound(sfxHandle_t sfx);

void            CG_DrawActiveFrame(int serverTime, stereoFrame_t stereoView, qboolean demoPlayback);


//
// cg_drawtools.c
//
void            CG_AdjustFrom640(float *x, float *y, float *w, float *h);
void            CG_FillRect(float x, float y, float width, float height, const float *color);
void            CG_DrawPic(float x, float y, float width, float height, qhandle_t hShader);
void            CG_DrawString(float x, float y, const char *string, float charWidth, float charHeight, const float *modulate);


void            CG_DrawStringExt(int x, int y, const char *string, const float *setColor,
								 qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars);
void            CG_DrawBigString(int x, int y, const char *s, float alpha);
void            CG_DrawBigStringColor(int x, int y, const char *s, vec4_t color);
void            CG_DrawSmallString(int x, int y, const char *s, float alpha);
void            CG_DrawSmallStringColor(int x, int y, const char *s, vec4_t color);

int             CG_DrawStrlen(const char *str);

float          *CG_FadeColor(int startMsec, int totalMsec);
float          *CG_TeamColor(int team);
void            CG_TileClear(void);
void            CG_ColorForHealth(vec4_t hcolor);
void            CG_GetColorForHealth(int health, int armor, vec4_t hcolor);

void            UI_DrawProportionalString(int x, int y, const char *str, int style, vec4_t color);
void            CG_DrawRect(float x, float y, float width, float height, float size, const float *color);
void            CG_DrawSides(float x, float y, float w, float h, float size);
void            CG_DrawTopBottom(float x, float y, float w, float h, float size);


//
// cg_draw.c, cg_newDraw.c
//
extern int      sortedTeamPlayers[TEAM_MAXOVERLAY];
extern int      numSortedTeamPlayers;
extern int      drawTeamOverlayModificationCount;
extern char     systemChat[256];
extern char     teamChat1[256];
extern char     teamChat2[256];

void            CG_DrawHudString(int x, int y, char *s, float size, int style, const vec4_t color);

int             CG_Text_Width(const char *text, float scale, int limit, const fontInfo_t * font);
int             CG_Text_Height(const char *text, float scale, int limit, const fontInfo_t * font);
void            CG_Text_PaintChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2,
								  qhandle_t hShader);
void            CG_Text_Paint(float x, float y, float scale, const vec4_t color, const char *text, float adjust, int limit,
							  int style, const fontInfo_t * font);
void            CG_Text_PaintAligned(int x, int y, const char *s, float scale, int style, const vec4_t color,
									 const fontInfo_t * font);
void            CG_Text_PaintAlignedBlock(int x, int y, char *s, float scale, int style, const vec4_t color,
									 const fontInfo_t * font);

void            CG_AddLagometerFrameInfo(void);
void            CG_AddLagometerSnapshotInfo(snapshot_t * snap);
void            CG_CenterPrint(const char *str, int y, int charWidth);
void            CG_DrawHead(float x, float y, float w, float h, int clientNum, vec3_t headAngles);
void            CG_DrawActive(stereoFrame_t stereoView);
void            CG_DrawFlagModel(float x, float y, float w, float h, int team, qboolean force2D);
void            CG_DrawTeamBackground(int x, int y, int w, int h, float alpha, int team);
void            CG_OwnerDraw(float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags,
							 int align, float special, float scale, vec4_t color, qhandle_t shader, int textStyle);
void            CG_SelectPrevPlayer(void);
void            CG_SelectNextPlayer(void);
void            CG_ScanForCrosshairEntity(void);
float           CG_GetValue(int ownerDraw);
qboolean        CG_OwnerDrawVisible(int flags);
void            CG_RunMenuScript(char **args);
void            CG_ShowResponseHead(void);
void            CG_SetPrintString(int type, const char *p);
void            CG_InitTeamChat(void);
void            CG_GetTeamColor(vec4_t * color);
const char     *CG_GetGameStatusText(void);
const char     *CG_GetKillerText(void);
void            CG_Draw3DModel(float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles);
void            CG_Draw3DWeaponModel(float x, float y, float w, float h, qhandle_t weaponModel, qhandle_t barrelModel,
									 qhandle_t skin, vec3_t origin, vec3_t angles);
void            CG_CheckOrderPending(void);
const char     *CG_GameTypeString(void);
qboolean        CG_YourTeamHasFlag(void);
qboolean        CG_OtherTeamHasFlag(void);
qhandle_t       CG_StatusHandle(int task);



//
// cg_players.c
//
void            CG_Player(centity_t * cent);
void            CG_ResetPlayerEntity(centity_t * cent);
void            CG_AddRefEntityWithPowerups(refEntity_t * ent, entityState_t * state, int team);
void            CG_NewClientInfo(int clientNum);
sfxHandle_t     CG_CustomSound(int clientNum, const char *soundName);
void            CG_DrawPlayerCollision(centity_t * cent, const vec3_t bodyOrigin, const matrix_t bodyRotation);

//
// cg_predict.c
//
void            CG_BuildSolidList(void);
int             CG_PointContents(const vec3_t point, int passEntityNum);
void            CG_Trace(trace_t * result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
						 int skipNumber, int mask);
void            CG_CapTrace(trace_t * result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
							int skipNumber, int mask);
void            CG_BiSphereTrace(trace_t * result, const vec3_t start, const vec3_t end,
								 const float startRadius, const float endRadius, int skipNumber, int mask);
void            CG_PredictPlayerState(void);
void            CG_LoadDeferredPlayers(void);

//WEAVER
// cg_spell_effects.c
//
void            CG_WeaveEffect(centity_t * cent);
void            CG_WeaveEffect_OnPlayer(centity_t * cent, centity_t * player, playerState_t * ps, refEntity_t * body);
void            CG_WeaveCast(centity_t * cent);
void            CG_WeaveMissile_Trail(centity_t * ent);
void            CG_WeaveEffect_Link(centity_t * cent);
void            CG_WeaveMissileHitPlayer(int weapon, vec3_t origin, vec3_t dir, int entityNum);
void            CG_WeaveMissileHitWall(int weapon, int clientNum, vec3_t origin, vec3_t dir, int entityNum);

// cg_spell_ents.c
void            CG_ShieldInfo(centity_t * cent);
qboolean        CG_ShieldTeamAttacking(centity_t *ent, team_t team);
centity_t      *CG_ObjItem(playerEntity_t * pe, int clientNum);

// cg_spell_weaves.c
void            CG_ShowThreads(centity_t * cent);
qboolean        CG_PlayerIsCasting(playerState_t * ps);
qboolean        CG_PlayerIsThreading(playerState_t * ps);
void            CG_AddPlayerThreads(centity_t * player, playerState_t * ps, refEntity_t * parent);
void            CG_AddPlayerHeldWeave(centity_t * player, playerState_t * ps, refEntity_t * parent);
void            CG_AddPlayerProtects(centity_t * player, playerState_t * ps, refEntity_t * body);
void            CG_AddPlayerWeaveEffects(centity_t * player, playerState_t * ps, refEntity_t * body);
void            CG_HeldWeave(centity_t * cent);
void            CG_WeaveSelect_f(void);
void            CG_WeaveNext_f(void);
void            CG_WeavePrev_f(void);
void            CG_WeaveClearedChange(void);
void            CG_RegisterWeave(int weaveNum);

// cg_spell_sense.c
void            CG_UpdateWeaveSense(int clientnum, vec3_t origin, int power);
void            CG_DrawWeaveSense(void);

// cg_spell_hud.c
void            CG_HudSizeRecalc(void);
void            CG_HudSizesInit(void);
void            CG_HudSizesScale(float f);
void            CG_DrawWeaverHUD(void);
void            CG_DrawWeaverChat(void);
void            CG_DrawWeaverNotify(void);
void            CG_DrawWeaverCrosshairNames(void);

// cg_tutorial.c
char           *CG_TutorialText( void );

//
// cg_events.c
//
void            CG_AddToNotify(notify_mode_t mode, const char *str);
void            CG_CheckEvents(centity_t * cent);
const char     *CG_PlaceString(int rank);
void            CG_EntityEvent(centity_t * cent, vec3_t position);
void            CG_PainEvent(centity_t * cent, int health);


//
// cg_ents.c
//
void            CG_SetEntitySoundPosition(centity_t * cent);
void            CG_AddPacketEntities(void);
void            CG_Beam(centity_t * cent);

float           CG_DrawLineSegment(const vec3_t start, const vec3_t end,
								   float totalLength, float segmentSize, float scrollspeed, qhandle_t shader);
void            CG_CurvedLine(const vec3_t start, const vec3_t end, const vec3_t startDir,
							  qhandle_t shader, float segmentLen, float scrollSpeed);

void            CG_AdjustPositionForMover(const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out);

void            CG_PositionEntityOnTag(refEntity_t * entity, const refEntity_t * parent, qhandle_t parentModel, char *tagName);
void            CG_PositionRotatedEntityOnTag(refEntity_t * entity, const refEntity_t * parent,
											  qhandle_t parentModel, char *tagName);

qboolean        CG_PositionRotatedEntityOnBone(refEntity_t * entity, const refEntity_t * parent,
											   qhandle_t parentModel, char *tagName);

void            CG_TransformSkeleton(refSkeleton_t * skel, const vec3_t scale);
int             CG_UniqueNoShadowID(void);


//
// cg_weapons.c
//
void            CG_NextWeapon_f(void);
void            CG_PrevWeapon_f(void);
void            CG_Weapon_f(void);

void            CG_PositionEntityOnWeaponTag(refEntity_t *gun, const refEntity_t * parent, qhandle_t parentModel, playerState_t * ps, centity_t * cent);

void            CG_RegisterWeapon(int weaponNum);
void            CG_RegisterItemVisuals(int itemNum);

void            CG_FireWeapon(centity_t * cent);
void            CG_FireWeapon2(centity_t * cent);
void            CG_MissileHitWall(int weapon, int entityType, int clientNum, vec3_t origin, vec3_t dir, impactSound_t soundType);
void            CG_MissileHitPlayer(int weapon, int entityType, vec3_t origin, vec3_t dir, int entityNum);
void            CG_ShotgunFire(entityState_t * es);
void            CG_Bullet(vec3_t origin, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum);

void            CG_RailTrail(clientInfo_t * ci, vec3_t start, vec3_t end);
void            CG_GrappleTrail(centity_t * ent, const weaponInfo_t * wi);
void            CG_AddViewWeapon(playerState_t * ps);
void            CG_AddPlayerWeapon(refEntity_t * parent, playerState_t * ps, centity_t * cent, int team);
void            CG_DrawWeaponSelect(void);

void            CG_OutOfAmmoChange(void);	// should this be in pmove?

//
// cg_sword.c
//
void            CG_AddPlayerSword(refEntity_t * parent, playerState_t * ps, centity_t * cent, int team);

//
// cg_marks.c
//
void            CG_InitMarkPolys(void);
void            CG_AddMarks(void);
void            CG_ImpactMark(qhandle_t markShader,
							  const vec3_t origin, const vec3_t dir,
							  float orientation,
							  float r, float g, float b, float a, qboolean alphaFade, float radius, qboolean temporary);

//
// cg_localents.c
//
void            CG_InitLocalEntities(void);
localEntity_t  *CG_AllocLocalEntity(void);
void            CG_AddLocalEntities(void);

//
// cg_effects.c
//
localEntity_t  *CG_SmokePuff(const vec3_t p,
							 const vec3_t vel,
							 float radius,
							 float r, float g, float b, float a,
							 float duration, int startTime, int fadeInTime, int leFlags, qhandle_t hShader);
void            CG_BubbleTrail(vec3_t start, vec3_t end, float spacing);

void            CG_KamikazeEffect(vec3_t org);
void            CG_RailExplode(vec3_t org);

void            CG_ObeliskExplode(vec3_t org, int entityNum);
void            CG_ObeliskPain(vec3_t org);
void            CG_ExplosiveModel(vec3_t origin, vec3_t mins, vec3_t maxs, qhandle_t model, vec3_t matrix[3]);
void            CG_ExplosiveRubble(vec3_t origin, vec3_t mins, vec3_t maxs, qhandle_t model);

void            CG_LightningBoltBeam(vec3_t start, vec3_t end);

void            CG_ScorePlum(int client, vec3_t org, int score);

void            CG_GibPlayer(vec3_t playerOrigin);

void            CG_Bleed(vec3_t origin, int entityNum);

void            CG_ExplosiveExplode(centity_t * cent);

void            CG_FireEffect(vec3_t org, vec3_t mins, vec3_t maxs, float flameSize, int particles, float intensity);

void            CG_Fire(centity_t * cent);
void            CG_AddFire(localEntity_t * le);

localEntity_t  *CG_MakeExplosion(vec3_t origin, vec3_t dir, qhandle_t hModel, qhandle_t shader, int msec, qboolean isSprite);

//
// cg_snapshot.c
//
void            CG_ProcessSnapshots(void);

//unlagged - early transitioning
void            CG_TransitionEntity(centity_t * cent);

//unlagged - early transitioning

//
// cg_info.c
//
void            CG_LoadingString(const char *s, qboolean strong);
void            CG_DrawInformation(void);

//
// cg_scoreboard.c
//
void            CG_DrawOldTourneyScoreboard(void);
qboolean        CG_DrawScoreboardNew(void);
void            CG_ScoreSizesRecalc(void);
void            CG_ScoreSizesInit(void);
void            CG_ScoreSizesScale(float f);

//
// cg_consolecmds.c
//
qboolean        CG_ConsoleCommand(void);
void            CG_InitConsoleCommands(void);

//
// cg_servercmds.c
//
void            CG_ExecuteNewServerCommands(int latestSequence);
void            CG_ParseServerinfo(void);
void            CG_ParseSWInfo(void);
void            CG_SetConfigValues(void);
void            CG_LoadVoiceChats(void);
void            CG_ShaderStateChanged(void);
void            CG_VoiceChatLocal(int mode, qboolean voiceOnly, int clientNum, int color, const char *cmd);
void            CG_PlayBufferedVoiceChats(void);

//
// cg_playerstate.c
//
void            CG_Respawn(void);
void            CG_TransitionPlayerState(playerState_t * ps, playerState_t * ops);
void            CG_CheckChangedPredictableEvents(playerState_t * ps);


#ifdef CG_LUA
//
// cg_lua.c
//
#include <lua.h>
void            CG_InitLua(void);
void            CG_ShutdownLua(void);
void            CG_LoadLuaScript(const char *filename);
void            CG_RunLuaFunction(const char *func, const char *sig, ...);
void            CG_DumpLuaStack(void);
void            CG_RestartLua_f(void);

//
// lua_cgame.c
//
int             luaopen_cgame(lua_State * L);


//
// lua_particle.c
//
typedef struct
{
	cparticle_t    *p;
} lua_Particle;

int             luaopen_particle(lua_State * L);
void            lua_pushparticle(lua_State * L, cparticle_t * p);
lua_Particle   *lua_getparticle(lua_State * L, int argNum);

//
// lua_qmath.c
//
int             luaopen_qmath(lua_State * L);

//
// lua_vector.c
//
int             luaopen_vector(lua_State * L);
void            lua_pushvector(lua_State * L, vec3_t v);
vec_t          *lua_getvector(lua_State * L, int argNum);
#endif


//
// cg_particles.c
//
void            CG_InitParticles(void);
cparticle_t    *CG_AllocParticle(void);
void            CG_FreeParticle(cparticle_t * p);
void            CG_AddParticles(void);
void            CG_ParticleSnow(qhandle_t pshader, vec3_t origin, vec3_t origin2, int turb, float range, int snum);
void            CG_ParticleSmoke(qhandle_t pshader, centity_t * cent);
void            CG_AddParticleShrapnel(localEntity_t * le);
void            CG_ParticleSnowFlurry(qhandle_t pshader, centity_t * cent);
void            CG_ParticleImpactSmokePuff(qhandle_t pshader, vec3_t origin);
void            CG_ParticleBlood(vec3_t org, vec3_t dir, int count);
void            CG_Particle_Bleed(qhandle_t pshader, vec3_t start, vec3_t dir, int fleshEntityNum, int duration);
void            CG_BloodPool(qhandle_t pshader, vec3_t origin);
void            CG_ParticleBulletDebris(vec3_t org, vec3_t vel, int duration);
void            CG_ParticleDirtBulletDebris_Core(vec3_t org, vec3_t vel, int duration, float width, float height, float alpha,
												 qhandle_t shader);
void            CG_ParticleBloodCloud(vec3_t origin, vec3_t dir);
void            CG_ParticleSparks(vec3_t org, vec3_t vel, int duration, float x, float y, float speed);
void            CG_ParticleSparks2(vec3_t org, vec3_t dir, int count);
void            CG_ParticleRick(vec3_t org, vec3_t dir);
void            CG_ParticleRailRick(vec3_t org, vec3_t dir, vec3_t clientColor);
void            CG_ParticleHealStream(const vec3_t start, const vec3_t end, float size, qhandle_t shader);
void            CG_ParticleDust(centity_t * cent, vec3_t origin, vec3_t dir);
void            CG_ParticleMisc(qhandle_t pshader, vec3_t origin, int size, int duration, float alpha);
void            CG_ParticleTeleportEffect(const vec3_t origin);
void            CG_ParticleGibEffect(const vec3_t origin);
int             CG_NewParticleArea(int num);
void            CG_TestParticles_f(void);

void            CG_SwingAngles(float destination, float swingTolerance, float clampTolerance,
							   float speed, float *angle, qboolean * swinging);
void            CG_AddPainTwitch(centity_t * cent, vec3_t torsoAngles);

void            CG_PlayerTokens(centity_t * cent, int renderfx);
void            CG_BreathPuffs(centity_t * cent, const vec3_t headOrigin, const vec3_t headDirection);

void            CG_PlayerSprites(centity_t * cent);
void            CG_PlayerSplash(centity_t * cent);
qboolean        CG_PlayerShadow(centity_t * cent, float *shadowPlane, int noShadowID);
qboolean        CG_FindClientModelFile(char *filename, int length, clientInfo_t * ci, const char *modelName,
									   const char *skinName, const char *base, const char *ext);


//
// cg_osd.c
//

void            CG_RegisterOSD(void);
void            CG_OSDUp_f(void);
void            CG_OSDDown_f(void);
void            CG_DrawOSD(void);
void            CG_OSDNext_f(void);
void            CG_OSDPrev_f(void);
void            CG_OSDInput(void);


//
// cg_animation.c
//
void            CG_RunLerpFrame(lerpFrame_t * lf, animation_t * anims, int animsNum, int newAnimation, float speedScale);



//===============================================

//
// system traps
// These functions are how the cgame communicates with the main game system
//

// print message on the local console
void            trap_Print(const char *fmt);

// abort the game
void            trap_Error(const char *fmt);

// milliseconds should only be used for performance tuning, never
// for anything game related.  Get time from the CG_DrawActiveFrame parameter
int             trap_Milliseconds(void);

// console variable interaction
void            trap_Cvar_Register(vmCvar_t * vmCvar, const char *varName, const char *defaultValue, int flags);
void            trap_Cvar_Update(vmCvar_t * vmCvar);
void            trap_Cvar_Set(const char *var_name, const char *value);
void            trap_Cvar_VariableStringBuffer(const char *var_name, char *buffer, int bufsize);

// ServerCommand and ConsoleCommand parameter access
int             trap_Argc(void);
void            trap_Argv(int n, char *buffer, int bufferLength);
void            trap_Args(char *buffer, int bufferLength);

// filesystem access
// returns length of file
int             trap_FS_FOpenFile(const char *qpath, fileHandle_t * f, fsMode_t mode);
void            trap_FS_Read(void *buffer, int len, fileHandle_t f);
void            trap_FS_Write(const void *buffer, int len, fileHandle_t f);
void            trap_FS_FCloseFile(fileHandle_t f);
int             trap_FS_Seek(fileHandle_t f, long offset, int origin);	// fsOrigin_t
int             trap_FS_GetFileList(const char *path, const char *extension, char *listbuf, int bufsize);

// add commands to the local console as if they were typed in
// for map changing, etc.  The command is not executed immediately,
// but will be executed in order the next time console commands
// are processed
void            trap_SendConsoleCommand(const char *text);

// register a command name so the console can perform command completion.
// FIXME: replace this with a normal console command "defineCommand"?
void            trap_AddCommand(const char *cmdName);

// register a command name as an alias for an existing engine command.
// this allows gamecode to register aliases for binds like "+button6".
void            trap_AddCommandAlias(const char *cmdName, const char *cmdOther);

// send a string to the server over the network
void            trap_SendClientCommand(const char *s);

// force a screen update, only used during gamestate load
void            trap_UpdateScreen(void);

// model collision
void            trap_CM_LoadMap(const char *mapname);
int             trap_CM_NumInlineModels(void);
clipHandle_t    trap_CM_InlineModel(int index);	// 0 = world, 1+ = bmodels
clipHandle_t    trap_CM_TempBoxModel(const vec3_t mins, const vec3_t maxs);
int             trap_CM_PointContents(const vec3_t p, clipHandle_t model);
int             trap_CM_TransformedPointContents(const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles);
void            trap_CM_BoxTrace(trace_t * results, const vec3_t start, const vec3_t end,
								 const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask);
void            trap_CM_TransformedBoxTrace(trace_t * results, const vec3_t start, const vec3_t end,
											const vec3_t mins, const vec3_t maxs,
											clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles);
void            trap_CM_CapsuleTrace(trace_t * results, const vec3_t start, const vec3_t end,
									 const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask);
void            trap_CM_TransformedCapsuleTrace(trace_t * results, const vec3_t start, const vec3_t end,
												const vec3_t mins, const vec3_t maxs,
												clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles);
void            trap_CM_BiSphereTrace(trace_t * results, const vec3_t start,
									  const vec3_t end, float startRad, float endRad, clipHandle_t model, int mask);
void            trap_CM_TransformedBiSphereTrace(trace_t * results, const vec3_t start,
												 const vec3_t end, float startRad, float endRad,
												 clipHandle_t model, int mask, const vec3_t origin);

// Returns the projection of a polygon onto the solid brushes in the world
int             trap_CM_MarkFragments(int numPoints, const vec3_t * points,
									  const vec3_t projection,
									  int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t * fragmentBuffer);

// normal sounds will have their volume dynamically changed as their entity
// moves and the listener moves
void            trap_S_StartSound(vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx);
void            trap_S_StopLoopingSound(int entnum);

// a local sound is always played full volume
void            trap_S_StartLocalSound(sfxHandle_t sfx, int channelNum);
void            trap_S_ClearLoopingSounds(qboolean killall);
void            trap_S_AddLoopingSound(int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx);
void            trap_S_AddRealLoopingSound(int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx);
void            trap_S_UpdateEntityPosition(int entityNum, const vec3_t origin);

// respatialize recalculates the volumes of sound as they should be heard by the
// given entityNum and position
void            trap_S_Respatialize(int entityNum, const vec3_t origin, vec3_t axis[3], int inwater);
sfxHandle_t     trap_S_RegisterSound(const char *sample);	// returns buzz if not found
void            trap_S_StartBackgroundTrack(const char *intro, const char *loop);	// empty name stops music
void            trap_S_StopBackgroundTrack(void);


void            trap_R_LoadWorldMap(const char *mapname);

// all media should be registered during level startup to prevent
// hitches during gameplay
qhandle_t       trap_R_RegisterModel(const char *name);	// returns rgb axis if not found
qhandle_t       trap_R_RegisterAnimation(const char *name);
qhandle_t       trap_R_RegisterSkin(const char *name);	// returns all white if not found
qhandle_t       trap_R_RegisterShader(const char *name);	// returns all white if not found
qhandle_t       trap_R_RegisterShaderNoMip(const char *name);	// returns all white if not found
qhandle_t       trap_R_RegisterShaderLightAttenuation(const char *name);

// a scene is built up by calls to R_ClearScene and the various R_Add functions.
// Nothing is drawn until R_RenderScene is called.
void            trap_R_ClearScene(void);
void            trap_R_AddRefEntityToScene(const refEntity_t * ent);
void            trap_R_AddRefLightToScene(const refLight_t * light);

// polys are intended for simple wall marks, not really for doing
// significant construction
void            trap_R_AddPolyToScene(qhandle_t hShader, int numVerts, const polyVert_t * verts);
void            trap_R_AddPolysToScene(qhandle_t hShader, int numVerts, const polyVert_t * verts, int numPolys);
void            trap_R_AddLightToScene(const vec3_t org, float intensity, float r, float g, float b);
int             trap_R_LightForPoint(vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir);
void            trap_R_RenderScene(const refdef_t * fd);
void            trap_R_SetColor(const float *rgba);	// NULL = 1,1,1,1
void            trap_R_DrawStretchPic(float x, float y, float w, float h,
									  float s1, float t1, float s2, float t2, qhandle_t hShader);
void            trap_R_ModelBounds(clipHandle_t model, vec3_t mins, vec3_t maxs);
int             trap_R_LerpTag(orientation_t * tag, clipHandle_t mod, int startFrame, int endFrame,
							   float frac, const char *tagName);
int             trap_R_CheckSkeleton(refSkeleton_t * skel, qhandle_t hModel, qhandle_t hAnim);
int             trap_R_BuildSkeleton(refSkeleton_t * skel, qhandle_t anim, int startFrame, int endFrame, float frac,
									 qboolean clearOrigin);
int             trap_R_BlendSkeleton(refSkeleton_t * skel, const refSkeleton_t * blend, float frac);
int             trap_R_BoneIndex(qhandle_t hModel, const char *boneName);
int             trap_R_AnimNumFrames(qhandle_t hAnim);
int             trap_R_AnimFrameRate(qhandle_t hAnim);

void            trap_R_RemapShader(const char *oldShader, const char *newShader, const char *timeOffset);

qboolean        trap_R_inPVS(const vec3_t p1, const vec3_t p2);

// The glconfig_t will not change during the life of a cgame.
// If it needs to change, the entire cgame will be restarted, because
// all the qhandle_t are then invalid.
void            trap_GetGlconfig(glconfig_t * glconfig);

// the gamestate should be grabbed at startup, and whenever a
// configstring changes
void            trap_GetGameState(gameState_t * gamestate);

// cgame will poll each frame to see if a newer snapshot has arrived
// that it is interested in.  The time is returned seperately so that
// snapshot latency can be calculated.
void            trap_GetCurrentSnapshotNumber(int *snapshotNumber, int *serverTime);

// a snapshot get can fail if the snapshot (or the entties it holds) is so
// old that it has fallen out of the client system queue
qboolean        trap_GetSnapshot(int snapshotNumber, snapshot_t * snapshot);

// retrieve a text command from the server stream
// the current snapshot will hold the number of the most recent command
// qfalse can be returned if the client system handled the command
// argc() / argv() can be used to examine the parameters of the command
qboolean        trap_GetServerCommand(int serverCommandNumber);

// returns the most recent command number that can be passed to GetUserCmd
// this will always be at least one higher than the number in the current
// snapshot, and it may be quite a few higher if it is a fast computer on
// a lagged connection
int             trap_GetCurrentCmdNumber(void);

qboolean        trap_GetUserCmd(int cmdNumber, usercmd_t * ucmd);

// used for the weapon select and zoom
void            trap_SetUserCmdValue(int stateValue, float sensitivityScale);

// aids for VM testing
void            testPrintInt(char *string, int i);
void            testPrintFloat(char *string, float f);

int             trap_MemoryRemaining(void);
void            trap_R_RegisterFont(const char *fontName, int pointSize, fontInfo_t * font);
qboolean        trap_Key_IsDown(int keynum);
int             trap_Key_GetCatcher(void);
void            trap_Key_SetCatcher(int catcher);
int             trap_Key_GetKey(const char *binding);


typedef enum
{
	SYSTEM_PRINT,
	CHAT_PRINT,
	TEAMCHAT_PRINT
} q3print_t;


int             trap_CIN_PlayCinematic(const char *arg0, int xpos, int ypos, int width, int height, int bits);
e_status        trap_CIN_StopCinematic(int handle);
e_status        trap_CIN_RunCinematic(int handle);
void            trap_CIN_DrawCinematic(int handle);
void            trap_CIN_SetExtents(int handle, int x, int y, int w, int h);

qboolean        trap_loadCamera(const char *name);
void            trap_startCamera(int time);
qboolean        trap_getCameraInfo(int time, vec3_t * origin, vec3_t * angles);

qboolean        trap_GetEntityToken(char *buffer, int bufferSize);

int             trap_RealTime(qtime_t * qtime);

void            trap_Key_KeynumToStringBuf(int keynum, char *buf, int buflen);
void            trap_Key_GetBindingBuf(int keynum, char *buf, int buflen);
void            trap_Key_SetBinding(int keynum, const char *binding);

