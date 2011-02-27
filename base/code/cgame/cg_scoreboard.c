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

#include "cg_local.h"

char           *monthStr2[12] = {
	"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
};

// Name, Ready icon, Score, Time, Ping
#define SCOREBOARD_COLS 5
char           *headline_titles[] = { "Player",     "", "Score", "Time", "Ping" };
float           headline_scales[] = { 0.03000f, 0.580f, 0.6200f, 0.760f, 0.900f };
int             headline_styles[] = { UI_LEFT, UI_LEFT, UI_LEFT, UI_LEFT, UI_LEFT };

typedef struct scoreSizes_s {
	float           f;

	int             num_scores;
	int             num_scores_all;
	int             num_scores_red;
	int             num_scores_blue;

	float           sep_x;

	float           row_h;
	float           row_y_offset;
	float           row_text_scale;
	float           row_text_y_offset;

	float           table_y;

	float           top_sep;
	float           top_y;
	float           top_h;
	float           top_text_scale;
	float           w;
	float           h;

	float           title_y;
	float           title_sub_y;

	float           vs_w;
	float           vs_h;

	float           red_x;
	float           blue_x;
	float           ffa_x;

	float           red_col_x[SCOREBOARD_COLS];
	float           blue_col_x[SCOREBOARD_COLS];
	float           ffa_col_x[SCOREBOARD_COLS];

	float           info_timeleft_x;
	float           info_timeleft_y;
} scoreSizes_t;

static scoreSizes_t s;

void CG_ScoreSizesRecalc(void)
{
	int             i;

	s.red_x = (cgs.screenXSize/2) - s.sep_x - s.w;
	s.blue_x = (cgs.screenXSize/2) + s.sep_x;
	s.ffa_x = (cgs.screenXSize/2) - (s.w / 2);

	for(i = 0; i < SCOREBOARD_COLS; i++)
	{
		s.red_col_x[i] = s.red_x + (headline_scales[i] * s.w);
		s.blue_col_x[i] = s.blue_x + (headline_scales[i] * s.w);
		s.ffa_col_x[i] = s.ffa_x + (headline_scales[i] * s.w);
	}

	s.table_y = s.top_y + s.top_h + s.top_sep;

	s.info_timeleft_x = s.blue_x + s.w;
	s.info_timeleft_y = s.top_y - 20.0f;

	s.num_scores_all = cg.numScores;
	s.num_scores_red = cg.teamScores[0];
	s.num_scores_blue = cg.teamScores[1];

	// Player rows
	if(s.num_scores_red >= s.num_scores_blue)
	{
		s.num_scores = s.num_scores_red;
	}
	else
	{
		s.num_scores = s.num_scores_blue;
	}

	if(s.num_scores < 8)
	{
		// Minimum of 8 rows
		s.num_scores = 8;
	}

	// + first (header) row + last (totals) row
	s.num_scores += 2;

	s.h = s.row_h * s.num_scores;

	s.title_y = s.top_y * 0.45;
	s.title_sub_y = s.top_y * 0.72;
}

void CG_ScoreSizesInit(void)
{
	s.f = 1.0f;

	s.sep_x = 16.0f;
	s.row_h = 32.0f;
	s.row_y_offset = 3.0f;
	s.row_text_scale = 0.4f;
	s.row_text_y_offset = 16.0f;

	s.top_sep = 8.0f;
	s.top_y = 130.0f;
	s.top_h = 92.0f;
	s.top_text_scale = 0.45f;
	s.w = 464.0f;

	s.vs_w = 128.0f;
	s.vs_h = 64.0f;

	CG_ScoreSizesRecalc();
}

void CG_ScoreSizesScale(float f)
{
	s.f *= f;

	s.sep_x *= f;
	s.row_h *= f;
	s.row_y_offset *= f;
	s.row_text_scale *= f;
	s.row_text_y_offset *= f;

	s.top_sep *= f;
	s.top_y *= f;
	s.top_h *= f;
	s.top_text_scale *= f;
	s.w *= f;

	s.vs_w *= f;
	s.vs_h *= f;

	CG_ScoreSizesRecalc();
}

/*
=================
CG_DrawScoreboardHeadline
pos is an array of int SCOREBOARD_COLS long.
=================
*/
void CG_DrawScoreboardHeadline(float pos[], int y)
{
	int             i;

	for(i = 0; i < SCOREBOARD_COLS; i++)
	{
		CG_Text_PaintAligned(pos[i], y + s.row_text_y_offset, headline_titles[i], s.row_text_scale, headline_styles[i], colorWhite, &cgs.media.freeSansBoldFont);
	}
}

/*
=================
CG_DrawScoreboardRows

Draws row backgrounds
=================
*/
void CG_DrawScoreboardRows(float x, float y, float *start_y, float *end_y)
{
	int             i;

	trap_R_SetColor(NULL);
	*start_y = y;
	for(i = 0; i < s.num_scores; i++)
	{
		trap_R_DrawStretchPic(x, y, s.w, s.row_h, 0, 0, 1, 1, cgs.media.scoreboard_row);
		y += s.row_h + s.row_y_offset;
	}
	*end_y = y - (s.row_h + s.row_y_offset);
	trap_R_SetColor(NULL);
}

/*
=================
CG_DrawScoreboardTop
pos is an array of int SCOREBOARD_COLS long.
=================
*/
void CG_DrawScoreboardTop(float x, float y, vec4_t bgcolor, qhandle_t topShader)
{
	trap_R_SetColor(bgcolor);
	trap_R_DrawStretchPic(x, y, s.w, s.top_h, 0, 0, 1, 1, topShader);
	trap_R_SetColor(NULL);
}


		//buttom line:
void CG_DrawScoreboardUnderlineNew(void)
{
	const char     *playercount = NULL;
	const char     *gametype = NULL;
	const char     *gamelimit = NULL;
	const char     *matchscore = NULL;
	const char     *time = "";
	const char     *status = "";
	char           *mapname = NULL;
	int             mins, seconds, tens;
	int             msec;

	const char     *info;

	const float     fontsize = 0.4f * s.f;

	info = CG_ConfigString(CS_SERVERINFO);
	mapname = Info_ValueForKey(info, "mapname");

	// Map name
	CG_Text_PaintAligned((cgs.screenXSize/2), s.title_y, mapname, fontsize * 2, UI_CENTER, colorWhite, &cgs.media.freeSansBoldFont);

	// Time left
	if(cgs.timelimit > 0)
	{
		if(cg.warmup == 0)
		{
			msec = (cgs.timelimit * 60.0f * 1000.0f) - (cg.time - cgs.levelStartTime);
			if(msec < 0)
			{
				msec = 0;
			}

			seconds = msec / 1000;
			mins = seconds / 60;
			seconds -= mins * 60;
			tens = seconds / 10;
			seconds -= tens * 10;

			time = va("Time left: %i:%i%i", mins, tens, seconds);
		}
		else
		{
			time = va("Warmup");
		}
		CG_Text_PaintAligned(s.blue_x + s.w, s.top_y - (4.0f * s.f), time, fontsize, UI_RIGHT, colorWhite, &cgs.media.freeSansBoldFont);
	}

	// Player count
	playercount = va("Players: %i/%i", cg.numScores, cgs.maxclients);
	CG_Text_PaintAligned((cgs.screenXSize/2), cgs.screenYSize - (100.0f * s.f), playercount, fontsize, UI_CENTER, colorWhite, &cgs.media.freeSansBoldFont);

	// Game type & limit
	switch(cgs.gametype)
	{
		case GT_FFA:
		case GT_TOURNAMENT:
		case GT_SINGLE_PLAYER:
			gametype = "FFA";
			gamelimit = va("Fraglimit: %i", cgs.fraglimit);
			break;
		case GT_TEAM:
			gametype = "TDM";
			gamelimit = va("Fraglimit: %i", cgs.fraglimit);
			break;
		case GT_CTF:
			gametype = "CTF";
			gamelimit = va("Capturelimit: %i", cgs.capturelimit);
			break;
		case GT_1FCTF:
			gametype = "1 Flag CTF";
			gamelimit = va("Capturelimit: %i", cgs.capturelimit);
			break;
		case GT_OBELISK:
			gametype = "Overload";
			gamelimit = va("Capturelimit: %i", cgs.capturelimit);
			break;
		case GT_HARVESTER:
			gametype = "Harvester";
			gamelimit = va("Capturelimit: %i", cgs.capturelimit);
			break;
		case GT_OBJECTIVE:
			gametype = "Objective";
			break;
		case GT_OBJECTIVE_SW:
			gametype = "SW Objective";
			break;
		default:
			break;
	}
	if(gametype)
	{
		CG_Text_PaintAligned((cgs.screenXSize/2), s.title_sub_y, gametype, fontsize, UI_CENTER, colorWhite, &cgs.media.freeSansBoldFont);
	}
	if(gamelimit)
	{
		CG_Text_PaintAligned((cgs.screenXSize/2) + 20, cgs.screenYSize - (50.0f * s.f), gamelimit, fontsize, UI_CENTER, colorWhite, &cgs.media.freeSansBoldFont);
	}

	// Match score
	if(cgs.gametype == GT_OBJECTIVE_SW)
	{
		msec = (cgs.timelimit * 60.0f * 1000.0f);
		seconds = msec / 1000;
		mins = seconds / 60;
		seconds -= mins * 60;
		tens = seconds / 10;
		seconds -= tens * 10;

		msec = (cgs.timelimit * 60.0f * 1000.0f) - (cg.time - cgs.levelStartTime);

		if(!cgs.currentRound)
		{
			//Stopwatch round 1
			if(cgs.defender == TEAM_RED)
			{
				status = va("Blue attacking %i:%i%i", mins, tens, seconds);
			}
			else if(cgs.defender == TEAM_BLUE)
			{
				status = va("Red attacking %i:%i%i", mins, tens, seconds);
			}
			else
			{
				status = va("Team attacking %i:%i%i", mins, tens, seconds);
			}
		}
		else
		{
			//Stopwatch round 2
			if(cgs.winner == TEAM_FREE)
			{
				if(msec < 0)
				{
					status = va("Game drawn!");
				}
				else if(cgs.defender == TEAM_RED)
				{
					status = va("Blue attacking %i:%i%i", mins, tens, seconds);
				}
				else if(cgs.defender == TEAM_BLUE)
				{
					status = va("Red attacking %i:%i%i", mins, tens, seconds);
				}
				else
				{
					status = va("Team attacking %i:%i%i", mins, tens, seconds);
				}
			}
			else if(cgs.winner == TEAM_RED)
			{
				status = va("Red wins the game!");
			}
			else if(cgs.winner == TEAM_BLUE)
			{
				status = va("Blue wins the game!");
			}
		}
		matchscore = va("Round %i: %s", cgs.currentRound + 1, status);
	}
	else if(cgs.gametype >= GT_TEAM)
	{
		if(cg.teamScores[0] == cg.teamScores[1])
		{
			matchscore = va("Teams are tied at %i", cg.teamScores[0]);
		}
		else if(cg.teamScores[0] >= cg.teamScores[1])
		{
			matchscore = va("Red leads %i to %i", cg.teamScores[0], cg.teamScores[1]);
		}
		else
		{
			matchscore = va("Blue leads %i to %i", cg.teamScores[1], cg.teamScores[0]);
		}
	}
	else
	{
		if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
		{
			matchscore = "spectating...";
		}
		else
		{
			matchscore = va("%s place with %i", CG_PlaceString(cg.snap->ps.persistant[PERS_RANK] + 1), cg.snap->ps.persistant[PERS_SCORE]);
		}
	}
	if(matchscore)
	{
		CG_Text_PaintAligned(cgs.screenXSize / 2, cgs.screenYSize - (75.0f * s.f), matchscore, fontsize, UI_CENTER, colorWhite, &cgs.media.freeSansBoldFont);
	}
}


void CG_DrawScoreboardStat(clientInfo_t * ci, score_t * score, vec4_t color, float pos[], float y)
{
	int             i;
	const char     *str;
	qhandle_t       img;
	vec4_t          colorP;

	// highlight your position
	if(score->client == cg.snap->ps.clientNum)
	{
		VectorCopy4(color, colorP);
		color[0] = 0.2f;
		color[2] = 0.2f;
	}

	if(score->ping == -1)
	{
		// draw "connecting" instead of stats
		str = va("connecting...");
		CG_Text_PaintAligned(pos[0], y + s.row_text_y_offset, str, s.row_text_scale, headline_styles[0], color, &cgs.media.freeSansBoldFont);
	}
	else
	{
		for(i = 0; i < SCOREBOARD_COLS; i++)
		{
			str = NULL;
			img = 0;
			switch(i)
			{
				case 0:
					str = va("%s", ci->name);
					break;
				case 1:
					//TODO: ready icon
					//img = cgs.media.cursor;
					if(cg.snap->ps.stats[STAT_CLIENTS_READY] & (1 << score->client))
					{
						str = "R";
					}
					break;
				case 2:
					str = va("%i", score->score);
					break;
				case 3:
					str = va("%i", score->time);
					break;
				case 4:
					str = va("%i", score->ping);
					break;
				default:
					break;
			}
			if(str)
			{
				CG_Text_PaintAligned(pos[i], y + s.row_text_y_offset, str, s.row_text_scale, headline_styles[i], color, &cgs.media.freeSansBoldFont);
			}
			if(img)
			{
				trap_R_DrawStretchPic(pos[i], y, s.row_h, s.row_h, 0, 0, 1, 1, img);
			}
		}
	}

	if(score->client == cg.snap->ps.clientNum)
	{
		VectorCopy4(colorP, color);
	}
}

qboolean CG_DrawScoreboardNew(void)
{
	qtime_t         tm;
	char            st[1024];
	int             i;
	score_t        *score;
	clientInfo_t   *ci;
	int             count = 0;
	int             red_count = 0;
	int             blue_count = 0;

	if(cgs.gametype == GT_SINGLE_PLAYER && cg.predictedPlayerState.pm_type == PM_INTERMISSION)
	{
		cg.deferredPlayerLoading = 0;
		return qfalse;
	}

	// don't draw scoreboard during death while warmup up
	if(cg.warmup && !cg.showScores)
		return qfalse;

	if(s.num_scores_all != cg.numScores || s.num_scores_red != cg.teamScores[0] || s.num_scores_blue != cg.teamScores[1])
	{
		CG_ScoreSizesRecalc();
	}

	if(cgs.gametype >= GT_TEAM)	// team based scoreboard
	{
		float           red_table_y;
		float           blue_table_y;
		float           red_table_end_y;
		float           blue_table_end_y;

		//red
		CG_DrawScoreboardTop(s.red_x, s.top_y, redTeamColor, cgs.media.scoreboard_top_red);
		CG_DrawScoreboardRows(s.red_x, s.table_y, &red_table_y, &red_table_end_y);
		CG_DrawScoreboardHeadline(s.red_col_x, red_table_y);
		red_table_y += s.row_h + s.row_y_offset;

		//blue
		CG_DrawScoreboardTop(s.blue_x, s.top_y, blueTeamColor, cgs.media.scoreboard_top_blue);
		CG_DrawScoreboardRows(s.blue_x, s.table_y, &blue_table_y, &blue_table_end_y);
		CG_DrawScoreboardHeadline(s.blue_col_x, blue_table_y);
		blue_table_y += s.row_h + s.row_y_offset;

		//display scores
		for(i = 0; i < s.num_scores_all; i++)
		{
			score = &cg.scores[i];
			ci = &cgs.clientinfo[score->client];

			if(ci->team == TEAM_RED)
			{
				CG_DrawScoreboardStat(ci, score, colorWhite, s.red_col_x, red_table_y);
				red_table_y += s.row_h + s.row_y_offset;
				red_count++;
			}
			else if(ci->team == TEAM_BLUE)
			{
				CG_DrawScoreboardStat(ci, score, colorWhite, s.blue_col_x, blue_table_y);
				blue_table_y += s.row_h + s.row_y_offset;
				blue_count++;
			}
			else if(ci->team == TEAM_SPECTATOR && (i & 1) == 0)
			{
				CG_DrawScoreboardStat(ci, score, colorWhite, s.red_col_x, red_table_end_y);
				red_table_end_y -= s.row_h + s.row_y_offset;
			}
			else
			{
				CG_DrawScoreboardStat(ci, score, colorWhite, s.blue_col_x, blue_table_end_y);
				blue_table_end_y -= s.row_h + s.row_y_offset;
			}
		}

		trap_R_DrawStretchPic((cgs.screenXSize - s.vs_w)/2, s.table_y + s.h + (s.vs_h / 2), s.vs_w, s.vs_h, 0, 0, 1, 1, cgs.media.scoreboard_vs);

		CG_DrawScoreboardUnderlineNew();
	}
	else						// FFA Scoreboard
	{
		float           ffa_table_y;
		float           ffa_table_end_y;

		// scoreboard
		CG_DrawScoreboardTop(s.ffa_x, s.top_y, colorWhite, cgs.media.scoreboard_top_blue);
		CG_DrawScoreboardRows(s.ffa_x, s.table_y, &ffa_table_y, &ffa_table_end_y);
		CG_DrawScoreboardHeadline(s.ffa_col_x, ffa_table_y);
		ffa_table_y += s.row_h + s.row_y_offset;

		// buttom line:
		CG_DrawScoreboardUnderlineNew();

		for(i = 0; i < s.num_scores_all; i++)
		{
			score = &cg.scores[i + cg.scoreboard_offset];
			ci = &cgs.clientinfo[score->client];

			if(ci->team != TEAM_SPECTATOR)
			{
				count++;
				CG_DrawScoreboardStat(ci, score, colorWhite, s.ffa_col_x, ffa_table_y);
				ffa_table_y += s.row_h + s.row_y_offset;
			}
			else
			{
				CG_DrawScoreboardStat(ci, score, colorWhite, s.ffa_col_x, ffa_table_end_y);
				ffa_table_end_y -= s.row_h + s.row_y_offset;
			}
		}
	}


	// put the current date and time at the bottom along with version info
	trap_RealTime(&tm);
	Com_sprintf(st, sizeof(st), "%2i:%s%i:%s%i (%i %s %i) " S_COLOR_RED "XreaL v" PRODUCT_VERSION " ", tm.tm_hour,	// 12 hour format
				(tm.tm_min > 9 ? "" : "0"),	// minute padding
				tm.tm_min, (tm.tm_sec > 9 ? "" : "0"),	// second padding
				tm.tm_sec, tm.tm_mday, monthStr2[tm.tm_mon], 1900 + tm.tm_year);


	CG_Text_PaintAligned(cgs.screenXSize/2, cgs.screenYSize - 18, st, 0.45f, UI_CENTER, colorWhite, &cgs.media.freeSansBoldFont);

	return qtrue;
}
