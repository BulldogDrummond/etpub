// cg_scoreboard -- draw the scoreboard on top of the game screen
#include "cg_local.h"


#define	SCOREBOARD_WIDTH	(31*BIGCHAR_WIDTH)

vec4_t clrUiBack = { 0.f, 0.f, 0.f, .6f };
vec4_t clrUiBar = { .16f, .2f, .17f, .8f };

/*
=================
WM_DrawObjectives
=================
*/

#define INFO_PLAYER_WIDTH		134
#define INFO_SCORE_WIDTH		56
#define INFO_XP_WIDTH			36
#define INFO_CLASS_WIDTH		50
#define INFO_LATENCY_WIDTH		40
#define INFO_LIVES_WIDTH		20
#define INFO_TEAM_HEIGHT		24
#define INFO_BORDER				2
#define INFO_LINE_HEIGHT		30
#define INFO_TOTAL_WIDTH		(INFO_PLAYER_WIDTH + INFO_CLASS_WIDTH + INFO_SCORE_WIDTH + INFO_LATENCY_WIDTH)

//mcwf GeoIP
qboolean cf_draw(float x, float y, float fade, int clientNum) {

        float alpha[4];
        float flag_step = 32;
        unsigned int flag_sd = 512;
        unsigned int client_flag = atoi(Info_ValueForKey(CG_ConfigString(clientNum + CS_PLAYERS),"uci"));


        if (client_flag < 255) {
        float x1 = (float)((client_flag * (unsigned int)flag_step) % flag_sd);
        float y1 = (float)(floor((client_flag * flag_step) / flag_sd) * flag_step);
        float x2 = x1 + flag_step;
        float y2 = y1 + flag_step;
        alpha[0] = alpha[1] = alpha[2] = 1.0; alpha[3] = fade;

        trap_R_SetColor(alpha);
        CG_DrawPicST(x, y, flag_step, flag_step, x1/flag_sd, y1/flag_sd, x2/flag_sd , y2/flag_sd, cgs.media.countryFlags);
        trap_R_SetColor(NULL);
        return qtrue;
        }
        return qfalse;
}
//mcwf GeoIP

int WM_DrawObjectives( int x, int y, int width, float fade ) {
	const char *s, *str;
	int tempy, rows;
	int msec, mins, seconds, tens; // JPW NERVE
	vec4_t tclr =	{ 0.6f,		0.6f,		0.6f,		1.0f };

	if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
		const char *s, *buf, *shader = NULL, *flagshader = NULL, *nameshader = NULL;

		// Moved to CG_DrawIntermission
/*		static int doScreenshot = 0, doDemostop = 0;

		// OSP - End-of-level autoactions
		if(!cg.demoPlayback) {
			if(!cg.latchVictorySound) {
				if(cg_autoAction.integer & AA_SCREENSHOT) {
					doScreenshot = cg.time + 1000;
				}
				if(cg_autoAction.integer & AA_STATSDUMP) {
					CG_dumpStats_f();
				}
				if((cg_autoAction.integer & AA_DEMORECORD) && (cgs.gametype == GT_WOLF_STOPWATCH && cgs.currentRound != 1)) {
					doDemostop = cg.time + 5000;	// stats should show up within 5 seconds
				}
			}
			if(doScreenshot > 0 && doScreenshot < cg.time) {
				CG_autoScreenShot_f();
				doScreenshot = 0;
			}
			if(doDemostop > 0 && doDemostop < cg.time) {
				trap_SendConsoleCommand("stoprecord\n");
				doDemostop = 0;
			}
		}
*/
		rows = 8;
		y += SMALLCHAR_HEIGHT * ( rows - 1 );

		s = CG_ConfigString( CS_MULTI_MAPWINNER );
		buf = Info_ValueForKey( s, "winner" );

		if ( atoi( buf ) == -1 )
			str = "ITS A TIE!";
		else if ( atoi( buf ) ) {
			str = "ALLIES";
//			shader = "ui/assets/portraits/allies_win";
			flagshader = "ui/assets/portraits/allies_win_flag.tga";
			nameshader = "ui/assets/portraits/text_allies.tga";

/*			if ( !cg.latchVictorySound ) {
				cg.latchVictorySound = qtrue;
				trap_S_StartLocalSound( trap_S_RegisterSound( "sound/music/allies_win.wav", qtrue ), CHAN_LOCAL_SOUND );	// FIXME: stream
			}*/
		}
		else {
			str = "AXIS";
//			shader = "ui/assets/portraits/axis_win";
			flagshader = "ui/assets/portraits/axis_win_flag.tga";
			nameshader = "ui/assets/portraits/text_axis.tga";

/*			if ( !cg.latchVictorySound ) {
				cg.latchVictorySound = qtrue;
				trap_S_StartLocalSound( trap_S_RegisterSound( "sound/music/axis_win.wav", qtrue ), CHAN_LOCAL_SOUND );	// FIXME: stream
			}*/
		}

		y += SMALLCHAR_HEIGHT * ( ( rows - 2 ) / 2 );

		if ( flagshader ) {
			CG_DrawPic( 100, 10, 210, 136, trap_R_RegisterShaderNoMip( flagshader ) );
			CG_DrawPic( 325, 10, 210, 136, trap_R_RegisterShaderNoMip( flagshader ) );
		}

		if ( shader )
			CG_DrawPic( 229, 10, 182, 136, trap_R_RegisterShaderNoMip( shader ) );
		if ( nameshader ) {
			CG_DrawPic( 140, 50, 127, 64, trap_R_RegisterShaderNoMip( nameshader ) );
			CG_DrawPic( 365, 50, 127, 64, trap_R_RegisterShaderNoMip( "ui/assets/portraits/text_win.tga" ) );
		}
		return y;
	}
// JPW NERVE -- mission time & reinforce time
	else {
		tempy = y;
		rows = 1;

		CG_FillRect( x-5, y-2, width+5, 21, clrUiBack );
		CG_FillRect( x-5, y-2, width+5, 21, clrUiBar );
		CG_DrawRect_FixedBorder( x-5, y-2, width+5, 21, 1, colorBlack );

		y += SMALLCHAR_HEIGHT * ( rows - 1 );
		if( cgs.timelimit > 0.0f ) {
			msec = ( cgs.timelimit * 60.f * 1000.f ) - ( cg.time - cgs.levelStartTime );

			seconds = msec / 1000;
			mins = seconds / 60;
			seconds -= mins * 60;
			tens = seconds / 10;
			seconds -= tens * 10;
		} else {
			msec = mins = tens = seconds = 0;
		}

		if( cgs.gamestate != GS_PLAYING ) {
			s = va("%s %s", CG_TranslateString("MISSION TIME:"),  CG_TranslateString("WARMUP"));
		} else if ( msec < 0 && cgs.timelimit > 0.0f ) {
			if ( cgs.gamestate == GS_WAITING_FOR_PLAYERS )
				s = va( "%s %s", CG_TranslateString( "MISSION TIME:" ),  CG_TranslateString( "GAME STOPPED" ) );
			else
				s = va( "%s %s", CG_TranslateString( "MISSION TIME:" ),  CG_TranslateString( "SUDDEN DEATH" ) );
		} else {
			s = va( "%s   %2.0f:%i%i", CG_TranslateString( "MISSION TIME:" ), (float)mins, tens, seconds ); // float cast to line up with reinforce time
		}

		CG_Text_Paint_Ext( x, y + 13, 0.25f, 0.25f, tclr, s, 0, 0, 0, &cgs.media.font1 );

		if( cgs.gametype != GT_WOLF_LMS ) {
			if(cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_AXIS || cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_ALLIES) {
				msec = CG_CalculateReinfTime( qfalse ) * 1000;
			}
			else // no team (spectator mode)
				msec = 0;

			if (msec) {
				seconds = msec / 1000;
				mins = seconds / 60;
				seconds -= mins * 60;
				tens = seconds / 10;
				seconds -= tens * 10;

				s = va( "%s %2.0f:%i%i", CG_TranslateString( "REINFORCE TIME:" ), (float)mins, tens, seconds );
				CG_Text_Paint_Ext( 640 - 20 - CG_Text_Width_Ext( s, 0.25f, 0, &cgs.media.font1 ), y + 13, 0.25f, 0.25f, tclr, s, 0, 0, 0, &cgs.media.font1 );
			}
		}

		// NERVE - SMF
		if ( cgs.gametype == GT_WOLF_STOPWATCH ) {
			int w;
			s = va( "%s %i", CG_TranslateString( "STOPWATCH ROUND" ), cgs.currentRound + 1 );

			w = CG_Text_Width_Ext( s, 0.25f, 0, &cgs.media.font1 );

			CG_Text_Paint_Ext( x + 300 - w*0.5f, y + 13, 0.25f, 0.25f, tclr, s, 0, 0, 0, &cgs.media.font1 );
		} else if( cgs.gametype == GT_WOLF_LMS ) {
			int w;
			s = va( "%s %i  %s %i-%i", CG_TranslateString( "ROUND" ), cgs.currentRound + 1, CG_TranslateString( "SCORE" ), cg.teamWonRounds[1], cg.teamWonRounds[0] );
			w = CG_Text_Width_Ext( s, 0.25f, 0, &cgs.media.font1 );

			CG_Text_Paint_Ext( x + 300 - w*0.5f, y + 13, 0.25f, 0.25f, tclr, s, 0, 0, 0, &cgs.media.font1 );
		} else if( cgs.gametype == GT_WOLF_CAMPAIGN ) {
			int w;
			s = va( "MAP %i of %i", cgs.currentCampaignMap + 1, cgs.campaignData.mapCount );
			w = CG_Text_Width_Ext( s, 0.25f, 0, &cgs.media.font1 );

			CG_Text_Paint_Ext( x + 300 - w*0.5f, y + 13, 0.25f, 0.25f, tclr, s, 0, 0, 0, &cgs.media.font1 );
		} else if( cgs.gametype == GT_WOLF_MAPVOTE ) {
			int w;
			s = ( cgs.mapVoteMapY ? 
				va( "MAP %i of %i", cgs.mapVoteMapX + 1, cgs.mapVoteMapY ) : 
				"" );
			w = CG_Text_Width_Ext( s, 0.25f, 0, &cgs.media.font1 );

			CG_Text_Paint_Ext( x + 300 - w*0.5f, y + 13, 0.25f, 0.25f, 
						tclr, s, 0, 0, 0, &cgs.media.font1 );
		}
		
		y += SMALLCHAR_HEIGHT * 2;
	}
// jpw

	return y;
}

static void WM_DrawClientScore( int x, int y, score_t *score, float *color, float fade, qboolean show_lives ) {
	int maxchars, offset;
	int i, j;
	float tempx;
	vec4_t hcolor;
	clientInfo_t *ci;
	char buf[64];

	if ( y + SMALLCHAR_HEIGHT >= 470 )
		return;

	ci = &cgs.clientinfo[score->client];

	if ( score->client == cg.snap->ps.clientNum ) {

		hcolor[3] = fade * 0.3;
		VectorSet( hcolor, .5f, .5f, .2f );			// DARK-RED
		// CHRUKER: b077 - Player highlighting was split into columns
		CG_FillRect( x-5, y, (INFO_PLAYER_WIDTH + INFO_CLASS_WIDTH + 
			INFO_SCORE_WIDTH + INFO_LATENCY_WIDTH + 5),
			SMALLCHAR_HEIGHT - 1, hcolor );
	}

	tempx = x;

	// DHM - Nerve
	VectorSet( hcolor, 1, 1, 1 );
	hcolor[3] = fade;

	// Dens: if we don't show the lives in non-LMS, let the name be a bit longer
	if(show_lives){
		maxchars = 16;
	}else{
		maxchars = 18;
	}
	offset = 0;

	if ( ci->team != TEAM_SPECTATOR ) {
		if(cg_drawClassIcons.integer & CLASSICON_SCOREBOARD){
			maxchars += 3;
		}
		if ( ci->powerups & ( (1 << PW_REDFLAG) | (1 << PW_BLUEFLAG) ) ) {
			// CHRUKER: b078 - Medic, death and objective icons on
			//          the scoreboard are drawn too big
			CG_DrawPic( tempx - 3, y + 1, 14, 14,
				cgs.media.objectiveShader );
			// CHRUKER: b072 - Need to match tempx or else the
			//          other text gets offset
			offset += 14;
			tempx += 14;
			maxchars -= 2;
		}
		
		// foxX: draw ready icon for appropriate players during warmup 
		if ((GS_WARMUP == cgs.gamestate || GS_WARMUP_COUNTDOWN == cgs.gamestate) && ci->clientReady) { 
			CG_DrawPic( tempx - 3, y + 1, 14, 14, 
				cgs.media.hudSprintIcon ); 
				offset += 14; 
			tempx += 14; 
			maxchars -= 2;   
		} 

		// forty - draw no shoot for cov-ops
		if( 
			cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR &&
			ci->team == cgs.clientinfo[cg.clientNum].team &&
			ci->powerups & ( (1 << PW_OPS_DISGUISED) )
		) {
			CG_DrawPic( tempx - 3, y + 1, 14, 14,
				cgs.media.friendShader );
			offset += 14;
			tempx += 14;
			maxchars -= 2;
		}

		// draw the skull icon if out of lives
		if( score->respawnsLeft == -2 || (cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR && ci->team == cgs.clientinfo[cg.clientNum].team && cgs.clientinfo[score->client].health == -1 ) ) {
			// CHRUKER: b078 - Medic, death and objective icons on
			//          the scoreboard are drawn too big
			CG_DrawPic( tempx - 3, y + 1, 14, 14,
				cgs.media.scoreEliminatedShader );
			offset += 14;
			tempx += 14;
			maxchars -= 2;
		} else if( cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR && ci->team == cgs.clientinfo[cg.clientNum].team && cgs.clientinfo[score->client].health == 0 ) {
			// CHRUKER: b078 - Medic, death and objective icons on
			//          the scoreboard are drawn too big
			CG_DrawPic( tempx - 3, y + 1, 14, 14,
				cgs.media.medicIcon );
			offset += 14;
			tempx += 14;
			maxchars -= 2;			
		}
	}

        //mcwf GeoIP
        // draw flag before name
        if ((score->ping != -1) && (score->ping != 999) && (cg_countryFlags.integer)) {
                if(cf_draw(tempx-11, y-8, fade, ci->clientNum)) {
                        offset += 14;
                        tempx += 14;
                        maxchars -= 2;
                }
        }
        //mcwf GeoIP

	// draw name
	CG_DrawStringExt( tempx, y, ci->name, hcolor, qfalse, qfalse, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, maxchars );
	maxchars -= CG_DrawStrlen( ci->name );

	// draw medals
	buf[0] = '\0';
	for( i = 0; i < SK_NUM_SKILLS; i++ ) {
		for( j = 0; j < ci->medals[i]; j++ )
			Q_strcat( buf, sizeof(buf), va( "^%c%c", COLOR_RED + i, skillNames[i][0] ) );
	}
	maxchars--;
	// CHRUKER: b032 - Medals clipped wrong in scoreboard when you're
	//          dead, because CG_DrawStringExt will draw 
	//          everything if maxchars <= 0
	if (maxchars > 0) {
		CG_DrawStringExt( tempx + (BG_drawStrlen(ci->name) *
			SMALLCHAR_WIDTH + SMALLCHAR_WIDTH), y, buf, hcolor,
			qfalse, qfalse, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT,
			maxchars );
	}

	tempx += INFO_PLAYER_WIDTH - offset;

	// Dens: add the extra room here
	if(!show_lives){
		tempx += INFO_LIVES_WIDTH;
	}

	if ( ci->team == TEAM_SPECTATOR ) {
		const char *s;
		
		/*int w, totalwidth;
		totalwidth = INFO_CLASS_WIDTH + INFO_SCORE_WIDTH + INFO_LATENCY_WIDTH - 8;
		// CHRUKER: b031 -  Show connecting people as connecting
		if (score->ping == -1) {
			s = CG_TranslateString( "^3CONNECTING" );
		} else if (score->ping == 999) {
			s = CG_TranslateString( "^3ZOMBIE" );
		} else {
			s = CG_TranslateString( "^3SPECTATOR" );
		}
		w = CG_DrawStrlen( s ) * SMALLCHAR_WIDTH;*/
		
		// quad 
		if (ci->ettv)
			s = CG_TranslateString("^5ETTV"); 
		else if (ci->shoutcaster)
			s = CG_TranslateString("^dSHOUTCASTER");
		else if (score->ping >=0 && score->ping < 999) 
			s = CG_TranslateString("^3SPECT");
		else
			s = "";
		
		CG_DrawSmallString( tempx, y, s, fade );
	}
	// OSP - allow MV clients see the class of its merged client's on the scoreboard
	else if ( cg.snap->ps.persistant[PERS_TEAM] == ci->team || CG_mvMergedClientLocate(score->client) ) {
		if(cg_drawClassIcons.integer & CLASSICON_SCOREBOARD){
			CG_DrawPic(tempx+24, y+1, 14, 14,
				cgs.media.skillPics[BG_ClassSkillForClass( score->playerClass )]);
		}else{
			CG_DrawSmallString( tempx, y, CG_TranslateString( BG_ShortClassnameForNumber( score->playerClass ) ), fade );
		}
	}
	
	//forty - connecting players
	if ( score->ping == -1) {
		const char *s;
		int w, totalwidth;

		totalwidth = INFO_CLASS_WIDTH + INFO_SCORE_WIDTH + INFO_LATENCY_WIDTH - 8;

		s = CG_TranslateString( "^3CONNECTING" );
		w = CG_DrawStrlen( s ) * SMALLCHAR_WIDTH;

		CG_DrawSmallString( tempx + totalwidth - w, y, s, fade );
		return;
	}

	//forty - zombied out players
	if ( score->ping == 999) {
		const char *s;
		int w, totalwidth;

		totalwidth = INFO_CLASS_WIDTH + INFO_SCORE_WIDTH + INFO_LATENCY_WIDTH - 8;

		s = CG_TranslateString( "^3ZOMBIE" );
		w = CG_DrawStrlen( s ) * SMALLCHAR_WIDTH;

		CG_DrawSmallString( tempx + totalwidth - w, y, s, fade );
		return;
	}

	tempx += INFO_CLASS_WIDTH;

	if(cg_scoreboard.integer == SCOREBOARD_PR)
		CG_DrawSmallString( tempx, y,
			va("%.3f", score->playerRating), fade);
	else if(cg_scoreboard.integer == SCOREBOARD_KR)
		CG_DrawSmallString( tempx, y, 
			va( "%.3f", score->killRating ), fade);
	else if (!ci->shoutcaster)
		CG_DrawSmallString( tempx, y, 
			va( "%3i", score->score ), fade);

	if( cg_gameType.integer == GT_WOLF_LMS ) {
		tempx += INFO_SCORE_WIDTH;
	} else {
		tempx += INFO_XP_WIDTH;
	}

	CG_DrawSmallString( tempx, y, va( "%4i", score->ping ), fade );
	tempx += INFO_LATENCY_WIDTH;

	if( cg_gameType.integer != GT_WOLF_LMS && show_lives ) {
		if( score->respawnsLeft >= 0 ) {
			CG_DrawSmallString( tempx, y, va( "%2i", score->respawnsLeft ), fade );
		} else {
			CG_DrawSmallString( tempx, y, " -", fade );
		}
		tempx += INFO_LIVES_WIDTH;
	}
}

const char* WM_TimeToString( float msec ) {
	int mins, seconds, tens;

	seconds = msec / 1000;
	mins = seconds / 60;
	seconds -= mins * 60;
	tens = seconds / 10;
	seconds -= tens * 10;

	return va( "%i:%i%i", mins, tens, seconds );
}

static void WM_DrawClientScore_Small( int x, int y, score_t *score, float *color, float fade, qboolean show_lives ) {
	int maxchars, offset;
	float tempx;
	vec4_t hcolor;
	clientInfo_t *ci;
	 // CHRUKER: b033 - Added to draw medals
	int i, j;
	char buf[64];

	// CHRUKER: b033 - Was using the wrong char height for this
	//          calculation
	if ( y + MINICHAR_HEIGHT >= 470 )
		return;

	ci = &cgs.clientinfo[score->client];

	if ( score->client == cg.snap->ps.clientNum ) {

		hcolor[3] = fade * 0.3;
		VectorSet( hcolor, .5f, .5f, .2f );			// DARK-RED
		// CHRUKER: b077 - Player highlighting was split into columns
		CG_FillRect( x-5, y, (INFO_PLAYER_WIDTH + INFO_CLASS_WIDTH +
			INFO_SCORE_WIDTH + INFO_LATENCY_WIDTH + 5),
			MINICHAR_HEIGHT - 1, hcolor );

	}

	tempx = x;

	// DHM - Nerve
	VectorSet( hcolor, 1, 1, 1 );
	hcolor[3] = fade;

	// CHRUKER: b033 - Corrected to draw medals
	// Dens: if we don't show the lives in non-LMS, let the name be a bit longer
	if(show_lives){
		maxchars = 16;
	}else{
		maxchars = 18;
	}
	offset = 0;

	if ( ci->team != TEAM_SPECTATOR ) {	
		if(cg_drawClassIcons.integer & CLASSICON_SCOREBOARD){
			maxchars += 3;
		}
		if ( ci->powerups & ( (1 << PW_REDFLAG) | (1 << PW_BLUEFLAG) ) ) {
			// CHRUKER: b071 - Objective carrier icon missing on
			//          compact scoreboard
			// CHRUKER: b078 - Medic, death and objective icons 
			//          on the scoreboard are drawn too big
			CG_DrawPic( tempx - 1, y + 1, 10, 10,
				cgs.media.objectiveShader );	
			offset += 10;
			tempx += 10;
			maxchars -= 2;
		}

		// forty - draw no shoot for cov-ops
		if( 
			cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR &&
			ci->team == cgs.clientinfo[cg.clientNum].team &&
			ci->powerups & ( (1 << PW_OPS_DISGUISED) )
		) {
			CG_DrawPic( tempx - 1, y + 1, 10, 10,
				cgs.media.friendShader );
			offset += 10;
			tempx += 10;
			maxchars -= 2;
		}

		// draw the skull icon if out of lives
		if ( score->respawnsLeft == -2 || ( cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR && ci->team == cgs.clientinfo[cg.clientNum].team && cgs.clientinfo[score->client].health == -1 ) ) {
			CG_DrawPic( tempx - 1, y + 1, 10, 10,
				cgs.media.scoreEliminatedShader );
			offset += 10;
			tempx += 10;
			maxchars -= 2;
		} else if( cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR && ci->team == cgs.clientinfo[cg.clientNum].team && cgs.clientinfo[score->client].health == 0 ) {
			CG_DrawPic( tempx - 1, y + 1, 10, 10,
				cgs.media.medicIcon );
			offset += 10;
			tempx += 10;
			maxchars -= 2;
		} 
	}

        //mcwf GeoIP
        // draw flag before name
        if ((score->ping != -1) && (score->ping != 999) && (cg_countryFlags.integer)) {
                if(cf_draw(tempx-11, y-10, fade, ci->clientNum)) {
                        offset += 14;
                        tempx += 14;
                        maxchars -= 2;
                }
        }
        //mcwf GeoIP

	// draw name
	CG_DrawStringExt( tempx, y, ci->name, hcolor, qfalse, qfalse, MINICHAR_WIDTH, MINICHAR_HEIGHT, maxchars );
	
	// CHRUKER: b033 - Added to draw medals
	maxchars -= CG_DrawStrlen( ci->name );

	buf[0] = '\0';
	for( i = 0; i < SK_NUM_SKILLS; i++ ) {
		for( j = 0; j < ci->medals[i]; j++ )
			Q_strcat( buf, sizeof(buf), va( "^%c%c", COLOR_RED + i, skillNames[i][0] ) );
	}
	maxchars--;

	if (maxchars > 0) CG_DrawStringExt( tempx + (BG_drawStrlen(ci->name) * MINICHAR_WIDTH + MINICHAR_WIDTH), y, buf, hcolor, qfalse, qfalse, MINICHAR_WIDTH, MINICHAR_HEIGHT, maxchars );
	// b033

	tempx += INFO_PLAYER_WIDTH - offset;
	// dhm - nerve

	// Dens: add the extra room here
	if(!show_lives){
		tempx += INFO_LIVES_WIDTH;
	}

	if ( ci->team == TEAM_SPECTATOR ) {
		/*const char *s;
		int w, totalwidth;

		totalwidth = INFO_CLASS_WIDTH + INFO_SCORE_WIDTH + INFO_LATENCY_WIDTH - 8;

		// CHRUKER: b031 -  Show connecting people as connecting
		if (score->ping == -1) {
			s = CG_TranslateString( "^3CONNECTING" );
		} else if (score->ping == 999) {
			//forty - zombied out spectators.
			s = CG_TranslateString( "^3ZOMBIE" );
		} else {
			s = CG_TranslateString( "^3SPECTATOR" );
		}
		w = CG_DrawStrlen( s ) * MINICHAR_WIDTH;
		// CHRUKER: b034 - Using the mini char height
		CG_DrawStringExt( tempx + totalwidth - w, y, s, hcolor, qfalse,
			qfalse, MINICHAR_WIDTH, MINICHAR_HEIGHT, 0 );
		return;*/
		CG_DrawStringExt(	tempx, y, "^3SPECT", hcolor, qfalse, qfalse, MINICHAR_WIDTH, MINICHAR_HEIGHT, 0 );
	}
	else if ( cg.snap->ps.persistant[PERS_TEAM] == ci->team ) {
		if(cg_drawClassIcons.integer & CLASSICON_SCOREBOARD){
			CG_DrawPic(tempx+24, y+1, 10, 10,
				cgs.media.skillPics[BG_ClassSkillForClass( score->playerClass )]);
		}else{
			CG_DrawStringExt(	tempx, y, CG_TranslateString( BG_ShortClassnameForNumber( score->playerClass ) ), hcolor, qfalse, qfalse, MINICHAR_WIDTH, MINICHAR_HEIGHT, 0 );
		}
	}
	
	//forty - connecting players
	if ( score->ping == -1) {
		const char *s;
		int w, totalwidth;

		// Dens: we don't want any changes here
		if(!show_lives){
			tempx -= INFO_LIVES_WIDTH;
		}

		totalwidth = INFO_CLASS_WIDTH + INFO_SCORE_WIDTH + INFO_LATENCY_WIDTH - 8;

		s = CG_TranslateString( "^3CONNECTING" );
		w = CG_DrawStrlen( s ) * SMALLCHAR_WIDTH;

		CG_DrawSmallString( tempx + totalwidth - w, y, s, fade );
		return;
	}
	
	//forty - zombied out players
	if ( score->ping == 999) {
		const char *s;
		int w, totalwidth;

		// Dens: we don't want any changes here
		if(!show_lives){
			tempx -= INFO_LIVES_WIDTH;
		}
		
		totalwidth = INFO_CLASS_WIDTH + INFO_SCORE_WIDTH + INFO_LATENCY_WIDTH - 8;
		
		s = CG_TranslateString( "^3ZOMBIE" );
		w = CG_DrawStrlen( s ) * SMALLCHAR_WIDTH;
		
		CG_DrawSmallString( tempx + totalwidth - w, y, s, fade );
		return;
	}

	tempx += INFO_CLASS_WIDTH;

	//CG_DrawStringExt( tempx, y, va( "%3i", score->score ), hcolor, qfalse, qfalse, MINICHAR_WIDTH, MINICHAR_HEIGHT, 0 );
	if(cg_scoreboard.integer == SCOREBOARD_PR)
		CG_DrawStringExt(tempx, y,
			va("%.3f", score->playerRating), 
			hcolor, qfalse, qfalse,
			MINICHAR_WIDTH, MINICHAR_HEIGHT, 0);
	else if(cg_scoreboard.integer == SCOREBOARD_KR)
		CG_DrawStringExt(tempx, y,
			va("%.3f", score->killRating), 
			hcolor, qfalse, qfalse,
			MINICHAR_WIDTH, MINICHAR_HEIGHT, 0);
	else
		CG_DrawStringExt(tempx, y,
			va("%3i", score->score), 
			hcolor, qfalse, qfalse,
			MINICHAR_WIDTH, MINICHAR_HEIGHT, 0);

	if( cg_gameType.integer == GT_WOLF_LMS ) {
		tempx += INFO_SCORE_WIDTH;
	} else {
		tempx += INFO_XP_WIDTH;
	}

	CG_DrawStringExt( tempx, y, va( "%4i", score->ping ), hcolor, qfalse, qfalse, MINICHAR_WIDTH, MINICHAR_HEIGHT, 0 );
	tempx += INFO_LATENCY_WIDTH;

	if( cg_gameType.integer != GT_WOLF_LMS && show_lives ) {
		if( score->respawnsLeft >= 0 ) {
			CG_DrawStringExt( tempx, y, va( "%2i", score->respawnsLeft ), hcolor, qfalse, qfalse, MINICHAR_WIDTH, MINICHAR_HEIGHT, 0 );
		} else {
			CG_DrawStringExt( tempx, y, " -", hcolor, qfalse, qfalse, MINICHAR_WIDTH, MINICHAR_HEIGHT, 0 );
		}
		tempx += INFO_LIVES_WIDTH;
	}
}

static int WM_DrawInfoLine( int x, int y, float fade ) {
	int w, defender, winner;
	const char *s;
	vec4_t tclr =	{ 0.6f,		0.6f,		0.6f,		1.0f };

	if ( cg.snap->ps.pm_type != PM_INTERMISSION ) {
		return y;
	}

	w = 360;
//	CG_DrawPic( 320 - w/2, y, w, INFO_LINE_HEIGHT, trap_R_RegisterShaderNoMip( "ui/assets/mp_line_strip.tga" ) );

	s = CG_ConfigString( CS_MULTI_INFO );
	defender = atoi( Info_ValueForKey( s, "defender" ) );

	s = CG_ConfigString( CS_MULTI_MAPWINNER );
	winner = atoi( Info_ValueForKey( s, "winner" ) );

	if ( cgs.currentRound ) {
		// first round
		s = va( CG_TranslateString( "CLOCK IS NOW SET TO %s!" ), WM_TimeToString( cgs.nextTimeLimit * 60.f * 1000.f ) );
	}
	else {
		// second round
		if ( !defender ) {
			if ( winner != defender )
				s = "ALLIES SUCCESSFULLY BEAT THE CLOCK!";
			else
				s = "ALLIES COULDN'T BEAT THE CLOCK!";
		}
		else {
			if ( winner != defender )
				s = "AXIS SUCCESSFULLY BEAT THE CLOCK!";
			else
				s = "AXIS COULDN'T BEAT THE CLOCK!";
		}

		s = CG_TranslateString( s );
	}

	CG_FillRect( 320 - w/2, y, w, 20, clrUiBar );
	CG_DrawRect_FixedBorder( 320 - w/2, y, w, 20, 1, colorBlack );

	w = CG_Text_Width_Ext( s, 0.25f, 0, &cgs.media.font1 );

	CG_Text_Paint_Ext( 320 - w*0.5f, y + 15, 0.25f, 0.25f, tclr, s, 0, 0, 0, &cgs.media.font1 );
//	CG_DrawSmallString( 320 - w/2, ( y + INFO_LINE_HEIGHT / 2 ) - SMALLCHAR_HEIGHT / 2, s, fade );
	return y + INFO_LINE_HEIGHT + 6;
}

int QDECL CG_SortScoresByPR( const void *a, const void *b ) {
	score_t *score_a = &cg.scores[*((int*)a)];
	score_t *score_b = &cg.scores[*((int*)b)];

	if(score_a->playerRating > score_b->playerRating)
		return -1;
	else if(score_a->playerRating < score_b->playerRating)
		return 1;

	return 0;
}


int QDECL CG_SortScoresByKR( const void *a, const void *b ) {
	score_t *score_a = &cg.scores[*((int*)a)];
	score_t *score_b = &cg.scores[*((int*)b)];

	if(score_a->killRating > score_b->killRating)
		return -1;
	else if(score_a->killRating < score_b->killRating)
		return 1;

	return 0;
}


// CHRUKER: b035 - Added absolute maximum rows
static int WM_TeamScoreboard( int x, int y, team_t team, float fade, int maxrows, int absmaxrows ) {
	vec4_t hcolor;
	float tempx, tempy;
	int height, width;
	int i;
	int count = 0;
	// CHRUKER: b035 - Needed to check if using mini chars
	qboolean use_mini_chars = qfalse, show_lives = qfalse;
	vec4_t tclr =	{ 0.6f,		0.6f,		0.6f,		1.0f };
	char scorename[32] = {"XP"};
	const char *str;
	int sort[MAX_CLIENTS];
	const char *info = CG_ConfigString( CS_SERVERINFO );

	if(cgs.gametype == GT_WOLF_LMS) 
		Q_strncpyz(scorename, "Score", sizeof(scorename));
	
	switch(cg_scoreboard.integer) {
		case SCOREBOARD_PR:
			if(cgs.playerRating)
				Q_strncpyz(scorename, "PRW", sizeof(scorename));
			break;
		case SCOREBOARD_KR:
			if(cgs.killRating)
				Q_strncpyz(scorename, "K/D", sizeof(scorename));
			break;
	}

	str = Info_ValueForKey(info, "g_maxlives");
	if( str && *str ){
		if(atoi( str ) > 0){
			show_lives = qtrue;
		}
	}else{
		show_lives = qtrue;
	}

	str = Info_ValueForKey(info, "g_alliedmaxlives");
	if( str && *str ){
		if(atoi( str ) > 0){
			show_lives = qtrue;
		}
	}

	str = Info_ValueForKey(info, "g_axismaxlives");
	if( str && *str ){
		if(atoi( str ) > 0){
			show_lives = qtrue;
		}
	}


	height = SMALLCHAR_HEIGHT * maxrows;
	width = INFO_PLAYER_WIDTH + INFO_CLASS_WIDTH + INFO_SCORE_WIDTH + INFO_LATENCY_WIDTH;

	CG_FillRect( x-5, y-2, width+5, 21, clrUiBack );
	CG_FillRect( x-5, y-2, width+5, 21, clrUiBar );	
	
	Vector4Set( hcolor, 0, 0, 0, fade );
	CG_DrawRect_FixedBorder( x-5, y-2, width+5, 21, 1, colorBlack );

	// draw header
	if( cg_gameType.integer == GT_WOLF_LMS ) {
		char *s;
		if ( team == TEAM_AXIS ) {
			s = va( "%s [%d] (%d %s)", CG_TranslateString( "AXIS" ), cg.teamScores[0], cg.teamPlayers[team], CG_TranslateString("PLAYERS") );
			s = va( "%s ^3%s", s, cg.teamFirstBlood == TEAM_AXIS ? CG_TranslateString("FIRST BLOOD") : "" );

			CG_Text_Paint_Ext( x, y + 13, 0.25f, 0.25f, tclr, s, 0, 0, 0, &cgs.media.font1 );
		} else if ( team == TEAM_ALLIES ) {
			s = va( "%s [%d] (%d %s)", CG_TranslateString( "ALLIES" ), cg.teamScores[1], cg.teamPlayers[team], CG_TranslateString("PLAYERS") );
			s = va( "%s ^3%s", s, cg.teamFirstBlood == TEAM_ALLIES ? CG_TranslateString("FIRST BLOOD") : "" );

			CG_Text_Paint_Ext( x, y + 13, 0.25f, 0.25f, tclr, s, 0, 0, 0, &cgs.media.font1 );
		}
	} else {
		char *s = NULL;
		if ( team == TEAM_AXIS ) {
			if(cg_scoreboard.integer == SCOREBOARD_PR) {
				s = va("%s [%.1f%%] (%d %s)",
					CG_TranslateString( "AXIS" ),
					cg.axisProb,
					cg.teamPlayers[team],
					CG_TranslateString("PLAYERS"));
			}
			else {
				s = va("%s [%d] (%d %s)",
					CG_TranslateString( "AXIS" ),
					cg.teamScores[0],
					cg.teamPlayers[team],
					CG_TranslateString("PLAYERS"));
			}
		}
		else if ( team == TEAM_ALLIES ) {
			if(cg_scoreboard.integer == SCOREBOARD_PR) {
				s = va("%s [%.1f%%] (%d %s)",
					CG_TranslateString( "ALLIES" ),
					cg.alliesProb,
					cg.teamPlayers[team],
					CG_TranslateString("PLAYERS"));
			}
			else {
				s = va("%s [%d] (%d %s)",
					CG_TranslateString( "ALLIES" ),
					cg.teamScores[1],
					cg.teamPlayers[team],
					CG_TranslateString("PLAYERS"));
			}
		}
		CG_Text_Paint_Ext( x, y + 13, 0.25f, 0.25f, tclr,
			s, 0, 0, 0, &cgs.media.font1 );
	}

	y += SMALLCHAR_HEIGHT + 3;

	tempx = x;

	// CHRUKER: b076 - Adjusted y coordinate, and changed to use DrawBottom
	//          instead of DrawTopBottom
	CG_FillRect( x-5, y, width+5, 18, clrUiBack );
	trap_R_SetColor( colorBlack );
	CG_DrawBottom_NoScale( x-5, y, width+5, 18, 1 );
	trap_R_SetColor( NULL );

	// draw player info headings
	CG_DrawSmallString( tempx, y, CG_TranslateString( "Name" ), fade );
	tempx += INFO_PLAYER_WIDTH;

	if(!show_lives){
		tempx += INFO_LIVES_WIDTH;
	}

	CG_DrawSmallString( tempx, y, CG_TranslateString( "Class" ), fade );
	tempx += INFO_CLASS_WIDTH;

	if( cgs.gametype == GT_WOLF_LMS ) {
		CG_DrawSmallString( tempx, y,
			CG_TranslateString(scorename), fade );
		tempx += INFO_SCORE_WIDTH;
	} else {
		CG_DrawSmallString( tempx + 1 * SMALLCHAR_WIDTH, y, 
			CG_TranslateString(scorename), fade );
		tempx += INFO_XP_WIDTH;
	}

	CG_DrawSmallString( tempx, y, CG_TranslateString( "Ping" ), fade );
	tempx += INFO_LATENCY_WIDTH;

	if( cgs.gametype != GT_WOLF_LMS && show_lives) {
		CG_DrawPicST( tempx + 2, y, INFO_LIVES_WIDTH - 4, 16, 0.f, 0.f, 0.5f, 1.f, team == TEAM_ALLIES ? cgs.media.hudAlliedHelmet : cgs.media.hudAxisHelmet );
		tempx += INFO_LIVES_WIDTH;
	}
	

	// CHRUKER: b076 - The math says char height + 2 * border width
	//          (1 pixel)
	y += SMALLCHAR_HEIGHT + 2;

	cg.teamPlayers[team] = 0; // JPW NERVE
	for ( i = 0; i < cg.numScores; i++ ) {
		if ( team != cgs.clientinfo[ cg.scores[i].client ].team )
			continue;

		cg.teamPlayers[team]++;
	}

	// CHRUKER: b035 - Adjust maxrows
	if ( cg.teamPlayers[team] > maxrows ) {
		if (maxrows == 9) maxrows = absmaxrows;
		if (maxrows == 25) maxrows = absmaxrows;
		use_mini_chars = qtrue;
	}

	// save off y val
	tempy = y;

	// draw color bands
	for ( i = 0; i < maxrows; i++ ) {
		if ( i % 2 == 0 )
			VectorSet( hcolor, (80.f/255.f), (80.f/255.f), (80.f/255.f) );			// LIGHT BLUE
		else
			VectorSet( hcolor, (0.f/255.f), (0.f/255.f), (0.f/255.f) );			// DARK BLUE
		hcolor[3] = fade * 0.3;

		// CHRUKER: b035 - Missing support for mini char height scoreboard background
		if ( use_mini_chars ) {
			// CHRUKER: b076 - Adjusted y height, and changed to
			//          DrawBottom instead of DrawTopBottom
			CG_FillRect( x-5, y, width+5, MINICHAR_HEIGHT, hcolor );
			trap_R_SetColor( colorBlack );
			CG_DrawBottom_NoScale( x-5, y, width+5,
				MINICHAR_HEIGHT, 1 );
			trap_R_SetColor( NULL );

			y += MINICHAR_HEIGHT;
		} else {
			// CHRUKER: b076 - Adjusted y height, and changed to
			//          DrawBottom instead of DrawTopBottom
			
			CG_FillRect( x-5, y, width+5, SMALLCHAR_HEIGHT, hcolor );
			trap_R_SetColor( colorBlack );
			CG_DrawBottom_NoScale( x-5, y, width+5,
				SMALLCHAR_HEIGHT, 1 );
			trap_R_SetColor( NULL );

			y += SMALLCHAR_HEIGHT;
		}
	}

	hcolor[3] = 1;

	y = tempy;

	// draw player info
	VectorSet( hcolor, 1, 1, 1 );
	hcolor[3] = fade;

	// tjw: sort players by which score value is used
	for(i=0; i<cg.numScores; i++)
		sort[i] = i;
	if(cg_scoreboard.integer == SCOREBOARD_PR) {
		qsort(sort, cg.numScores, sizeof(int), CG_SortScoresByPR);
	}
	else if(cg_scoreboard.integer == SCOREBOARD_KR) {
		qsort(sort, cg.numScores, sizeof(int), CG_SortScoresByKR);
	}

	count = 0;
	for( i = 0; i < cg.numScores && count < maxrows; i++ ) {
		if( team != cgs.clientinfo[ cg.scores[sort[i]].client ].team ) {
			continue;
		}
		// CHRUKER: b035 - Using the flag instead
		if(use_mini_chars) {
			WM_DrawClientScore_Small( x, y, &cg.scores[sort[i]],
				hcolor, fade, show_lives );
			y += MINICHAR_HEIGHT;
		} else {
			WM_DrawClientScore( x, y, &cg.scores[sort[i]],
				hcolor, fade, show_lives );
			y += SMALLCHAR_HEIGHT;
		}

		count++;
	}

	// draw spectators
	// CHRUKER: b035 - Missing support for mini char height scoreboard background
	if ( use_mini_chars )
		y += MINICHAR_HEIGHT;
	else
		y += SMALLCHAR_HEIGHT;

	for ( i = 0; i < cg.numScores; i++ ) {
		if ( cgs.clientinfo[ cg.scores[i].client ].team != TEAM_SPECTATOR )
			continue;
		if ( team == TEAM_AXIS && ( i % 2 ) )
			continue;
		if ( team == TEAM_ALLIES && ( ( i + 1 ) % 2 ) )
			continue;
		// CHRUKER: b035 - Missing support for mini char height scoreboard background
		if( use_mini_chars ) {
			WM_DrawClientScore_Small( x, y, &cg.scores[i], hcolor, fade, show_lives );
			y += MINICHAR_HEIGHT;
		} else {
			WM_DrawClientScore( x, y, &cg.scores[i], hcolor, fade, show_lives );
			y += SMALLCHAR_HEIGHT;
		}
	}

	return y;
}
// -NERVE - SMF

/*
=================
CG_DrawScoreboard

Draw the normal in-game scoreboard
=================
*/
qboolean CG_DrawScoreboard( void ) {
	int		x = 0, y = 0, x_right;
	float	fade;
	float	*fadeColor;

	x = 20;
	y = 10;

	x_right = 640 - x - (INFO_TOTAL_WIDTH - 5);

	// don't draw anything if the menu or console is up
	if ( cg_paused.integer ) {
		return qfalse;
	}

	// don't draw scoreboard during death while warmup up
	// OSP - also for pesky scoreboards in demos
	if((cg.warmup || (cg.demoPlayback && cg.snap->ps.pm_type != PM_INTERMISSION)) && !cg.showScores) {
		return qfalse;
	}

	// don't draw if in cameramode
	if( cg.cameraMode ) {
		return qtrue;
	}

	if( cg.showScores || cg.predictedPlayerState.pm_type == PM_INTERMISSION ) {
		fade = 1.0;
		fadeColor = colorWhite;
	} else {
		fadeColor = CG_FadeColor( cg.scoreFadeTime, FADE_TIME );
		
		if( !fadeColor ) {
			// next time scoreboard comes up, don't print killer
			*cg.killerName = 0;
			return qfalse;
		}
 		fade = fadeColor[3];
	}

	y = WM_DrawObjectives( x, y, 640 - 2*x + 5, fade );

	if ( cgs.gametype == GT_WOLF_STOPWATCH && ( cg.snap->ps.pm_type == PM_INTERMISSION ) ) {
		y = WM_DrawInfoLine( x, 155, fade );
		// CHRUKER: b035 - The maxrows has been split into one for
		//          when to use the mini chars and one for when
		//          to stop writing.
		WM_TeamScoreboard( x, y, TEAM_AXIS, fade, 8, 8 );
		x = x_right;
		WM_TeamScoreboard( x, y, TEAM_ALLIES, fade, 8, 8 );
	} else {
		if(cg.snap->ps.pm_type == PM_INTERMISSION) {
			WM_TeamScoreboard( x, y, TEAM_AXIS, fade, 9, 12 );
			x = x_right;
			WM_TeamScoreboard( x, y, TEAM_ALLIES, fade, 9, 12 );
		} else {
			WM_TeamScoreboard( x, y, TEAM_AXIS, fade, 25, 33 );
			x = x_right;
			WM_TeamScoreboard( x, y, TEAM_ALLIES, fade, 25, 33 );
		}
	}

/*	if(!CG_IsSinglePlayer()) {
		qtime_t ct;

		G_showWindowMessages();
		trap_RealTime(&ct);
		s = va("^3%02d:%02d:%02d - %02d %s %d",
							ct.tm_hour, ct.tm_min, ct.tm_sec,
							ct.tm_mday, aMonths[ct.tm_mon], 1900 + ct.tm_year);
		CG_DrawStringExt(444, 12, s, colorWhite, qfalse, qtrue, 8, 8, 0);
	}
*/
	return qtrue;
}
