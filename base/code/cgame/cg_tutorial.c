/*
===========================================================================
Copyright (C) 2000-2009 Darklegion Development
Copyright (C) 2011 Andrew Browne <dersaidin@dersaidin.net>

This file is part of Weaver source code.
This file is based on cg_tutorial.c from Tremulous

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

// cg_tutorial.c -- the tutorial system

#include "cg_local.h"
#include <keycodes.h>

typedef struct
{
	char           *command;
	char           *humanName;
	keyNum_t        keys[ 2 ];
} bind_t;

static bind_t bindings[ ] =
{
	{"+sprint",     "Sprint",          { -1, -1 } },
	{"+speed",      "Run/Walk",        { -1, -1 } },
	{"+moveup",     "Jump",            { -1, -1 } },
	{"+movedown",   "Crouch",          { -1, -1 } },
	{"+attack",     "Attack",          { -1, -1 } },
	{"+thread" ,    "Thread",          { -1, -1 } },
	{"+weave" ,     "Cast a Spell",    { -1, -1 } },
	{"release",     "Release",         { -1, -1 } },
	{"+activate",   "Interact",        { -1, -1 } },
	{"spellprev",   "Previous Spell",  { -1, -1 } },
	{"spellpnext",  "Next Spell",      { -1, -1 } }
};

static const int numBindings = sizeof( bindings ) / sizeof( bind_t );

/*
=================
CG_GetBindings
=================
*/
static void CG_GetBindings( void )
{
  int   i, j, numKeys;
  char  buffer[ MAX_STRING_CHARS ];

  for( i = 0; i < numBindings; i++ )
  {
    bindings[ i ].keys[ 0 ] = bindings[ i ].keys[ 1 ] = K_NONE;
    numKeys = 0;

    for( j = 0; j < K_LAST_KEY; j++ )
    {
      trap_Key_GetBindingBuf( j, buffer, MAX_STRING_CHARS );

      if( buffer[ 0 ] == 0 )
        continue;

      if( !Q_stricmp( buffer, bindings[ i ].command ) )
      {
        bindings[ i ].keys[ numKeys++ ] = j;

        if( numKeys > 1 )
          break;
      }
    }
  }
}

/*
===============
CG_KeyNameForCommand
===============
*/
static const char *CG_KeyNameForCommand( const char *command )
{
  int         i, j;
  static char buffer[ MAX_STRING_CHARS ];
  int         firstKeyLength;

  buffer[ 0 ] = '\0';

  for( i = 0; i < numBindings; i++ )
  {
    if( !Q_stricmp( command, bindings[ i ].command ) )
    {
      if( bindings[ i ].keys[ 0 ] != K_NONE )
      {
        trap_Key_KeynumToStringBuf( bindings[ i ].keys[ 0 ],
            buffer, MAX_STRING_CHARS );
        firstKeyLength = strlen( buffer );

        for( j = 0; j < firstKeyLength; j++ )
          buffer[ j ] = toupper( buffer[ j ] );

        if( bindings[ i ].keys[ 1 ] != K_NONE )
        {
          Q_strcat( buffer, MAX_STRING_CHARS, " or " );
          trap_Key_KeynumToStringBuf( bindings[ i ].keys[ 1 ],
              buffer + strlen( buffer ), MAX_STRING_CHARS - strlen( buffer ) );

          for( j = firstKeyLength + 4; j < strlen( buffer ); j++ )
            buffer[ j ] = toupper( buffer[ j ] );
        }
      }
      else
      {
        Com_sprintf( buffer, MAX_STRING_CHARS, "\"%s\" (unbound)",
          bindings[ i ].humanName );
      }

      return buffer;
    }
  }

  return "";
}

#define MAX_TUTORIAL_TEXT 4096

/*
===============
CG_TutAttackShieldInRange
===============
*/
static entityState_t *CG_TutAttackShieldInRange( playerState_t *ps )
{
	vec3_t          view, point;
	trace_t         trace;
	centity_t      *ent;

	AngleVectors( cg.refdefViewAngles, view, NULL, NULL );
	VectorMA( cg.refdef.vieworg, 64, view, point );
	CG_Trace( &trace, cg.refdef.vieworg, NULL, NULL,
		point, ps->clientNum, MASK_SHOT );

	ent = &cg_entities[ trace.entityNum ];

	if( ent->currentState.eType == ET_SHIELD &&
		CG_ShieldTeamAttacking(ent, ps->persistant[PERS_TEAM]) )
		return &(ent->currentState);
	else
		return NULL;
}

/*
===============
CG_AlienBuilderText
===============
*/
static void CG_TutPlayerCastingText( char *text, playerState_t *ps )
{
	centity_t      *cent;
	weaver_weaveCGInfo *weaveInfo;

	// Check if player is in the process of casting a spell.
	if(CG_PlayerIsCasting(ps))
	{
		// Player is currently casting
		if(CG_PlayerIsThreading(ps))
		{
			Q_strcat( text, MAX_TUTORIAL_TEXT,
				va( "Release %s to add %s to the spell\n",
				CG_KeyNameForCommand( "+thread" ),
				"an element") );
		}
		else
		{
			Q_strcat( text, MAX_TUTORIAL_TEXT,
				va( "Hold down %s and move to an element\n",
				CG_KeyNameForCommand( "+thread" ) ) );
		}
		if( ps->stats[ STAT_POWER ] < POWER_PER_THREAD )
		{
			Q_strcat( text, MAX_TUTORIAL_TEXT,
				"You do not have enough power to add to the spell\n" );
		}
		Q_strcat( text, MAX_TUTORIAL_TEXT,
			va( "Release %s to finish weaving\n",
			CG_KeyNameForCommand( "+weave" ) ) );
	}
	else
	{
		// Player is not currently casting
		Q_strcat( text, MAX_TUTORIAL_TEXT,
			va( "Press %s to begin weaving\n",
			CG_KeyNameForCommand( "+weave" ) ) );

		// Check what spells are available
		if (cg.predictedPlayerState.ammo[cg.weaponSelect] > 0)
		{
			// Player has a spell selected
			cent = &cg_entities[cg.predictedPlayerState.ammo[cg.weaponSelect]];
			weaveInfo = &cg_weaves[cent->currentState.weapon];
			Q_strcat( text, MAX_TUTORIAL_TEXT,
				va( "Press %s to cast %s\n",
				CG_KeyNameForCommand( "+attack" ),
				weaveInfo->info.nameP) );
		}
	}
}

/*
===============
CG_TutPlayerInteractText
===============
*/
static void CG_TutPlayerInteractText( char *text, playerState_t *ps )
{
	/*
	//TODO: this doesn't work.
	entityState_t *es;
	es = CG_TutAttackShieldInRange(ps);
	if(es != NULL)
	{
		Q_strcat( text, MAX_TUTORIAL_TEXT,
			va( "Press and hold %s at the shield to dispell it\n",
			CG_KeyNameForCommand( "+activate" ) ) );
	}
	*/
}

/*
===============
CG_TutPlayerMovementText
===============
*/
static void CG_TutPlayerMovementText( char *text, playerState_t *ps )
{
	if( ps->stats[ STAT_STAMINA ] < (MAX_STAMINA / 10) )
	{
		Q_strcat( text, MAX_TUTORIAL_TEXT,
			"Your stamina is low. Stop sprinting to recover\n" );
	}
	Q_strcat( text, MAX_TUTORIAL_TEXT,
		va( "Press %s and any direction to sprint\n",
		CG_KeyNameForCommand( "+sprint" ) ) );
}

/*
===============
CG_TutSpectatorText
===============
*/
/*
static void CG_TutSpectatorText( char *text, playerState_t *ps )
{
	if( cgs.clientinfo[ cg.clientNum ].team == TEAM_RED || cgs.clientinfo[ cg.clientNum ].team == TEAM_BLUE )
	{
		Q_strcat( text, MAX_TUTORIAL_TEXT,
			va( "Press %s to spawn\n",
			CG_KeyNameForCommand( "+attack" ) ) );
	}
	else
	{
		Q_strcat( text, MAX_TUTORIAL_TEXT,
			va( "Press %s to join a team\n",
			CG_KeyNameForCommand( "+attack" ) ) );
	}

	if( ps->pm_flags & PMF_FOLLOW )
	{
		if( !cg.chaseFollow )
			Q_strcat( text, MAX_TUTORIAL_TEXT,
				va( "Press %s to switch to chase-cam spectator mode\n",
				CG_KeyNameForCommand( "+button2" ) ) );
		else if( cgs.clientinfo[ cg.clientNum ].team == TEAM_NONE )
			Q_strcat( text, MAX_TUTORIAL_TEXT,
				va( "Press %s to return to free spectator mode\n",
				CG_KeyNameForCommand( "+button2" ) ) );
		else
			Q_strcat( text, MAX_TUTORIAL_TEXT,
				va( "Press %s to stop following\n",
				CG_KeyNameForCommand( "+button2" ) ) );
		Q_strcat( text, MAX_TUTORIAL_TEXT,
			va( "Press %s or ",
			CG_KeyNameForCommand( "weapprev" ) ) );
		Q_strcat( text, MAX_TUTORIAL_TEXT,
			va( "%s to change player\n",
			CG_KeyNameForCommand( "weapnext" ) ) );
	}
	else
	{
		Q_strcat( text, MAX_TUTORIAL_TEXT,
			va( "Press %s to follow a player\n",
			CG_KeyNameForCommand( "+button2" ) ) );
	}
}
*/

#define BINDING_REFRESH_INTERVAL 30

/*
===============
CG_TutorialText

Returns context help for the current class/weapon
===============
*/
const char *CG_TutorialText( void )
{
	playerState_t *ps;
	static char   text[ MAX_TUTORIAL_TEXT ];
	static int    refreshBindings = 0;

	if( refreshBindings == 0 )
		CG_GetBindings( );

	refreshBindings = ( refreshBindings + 1 ) % BINDING_REFRESH_INTERVAL;

	text[ 0 ] = '\0';
	ps = &cg.snap->ps;

	if( !cg.intermissionStarted && !cg.demoPlayback )
	{
		/*
		if( ps->persistant[ PERS_SPECSTATE ] != SPECTATOR_NOT ||
			ps->pm_flags & PMF_FOLLOW )
		{
			CG_TutSpectatorText( text, ps );
		}
		else
		*/
		if( ps->persistant[ PERS_TEAM ] == TEAM_SPECTATOR )
		{
		}
		else
		{
			if( ps->stats[ STAT_HEALTH ] > 0 )
			{
				CG_TutPlayerCastingText( text, ps );
				CG_TutPlayerInteractText( text, ps );
				CG_TutPlayerMovementText( text, ps );
			}

			if( ps->persistant[ PERS_TEAM ] == TEAM_RED )
			{
			}
		}
	}

	/*
	else if( !cg.demoPlayback )
	{
		if( !CG_ClientIsReady( ps->clientNum ) )
		{
			Q_strcat( text, MAX_TUTORIAL_TEXT,
				va( "Press %s when ready to continue\n",
				CG_KeyNameForCommand( "+attack" ) ) );
		}
		else
		{
			Q_strcat( text, MAX_TUTORIAL_TEXT, "Waiting for other players to be ready\n" );
		}
	}
	*/

	if( !cg.demoPlayback )
	{
		Q_strcat( text, MAX_TUTORIAL_TEXT, "Press ESC for the menu" );
	}

	return text;
}
