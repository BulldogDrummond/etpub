#include "g_local.h"
#include "../ui/menudef.h"


/*
=======================================================================

  SESSION DATA

Session data is the only data that stays persistant across level loads
and tournament restarts.
=======================================================================
*/

/*
================
G_WriteClientSessionData

Called on game shutdown
================
*/
void G_WriteClientSessionData( gclient_t *client, qboolean restart )
{
	int mvc = G_smvGenerateClientList(g_entities + (client - level.clients));
	const char	*s;

	// OSP -- stats reset check
	// tjw: stopped saving weapon stats between rounds so always
	//      reset stats.
	//if(level.fResetStats) G_deleteStats(client - level.clients);
	G_deleteStats(client - level.clients);

	s = va("%i %i %i %i %i %i %i %i %i %i %i %i %i %i %f %f %f %f %i %i %i %i %i %i %i %i %i %i %i %i %i %s %s %s %u %d %i %i",
		client->sess.sessionTeam,
		client->sess.spectatorTime,
		client->sess.spectatorState,
		client->sess.spectatorClient,
		client->sess.playerType,			// DHM - Nerve
		client->sess.playerWeapon,			// DHM - Nerve
		client->sess.playerWeapon2,
		client->sess.latchPlayerType,		// DHM - Nerve
		client->sess.latchPlayerWeapon,		// DHM - Nerve
		client->sess.latchPlayerWeapon2,

		// OSP
		client->sess.coach_team,
		client->sess.deaths,
		client->sess.game_points,
		client->sess.kills,
		client->sess.overall_killrating,
		client->sess.overall_killvariance,
		client->sess.rating,
		client->sess.rating_variance,
		client->sess.referee,
		client->sess.spec_invite,
		client->sess.spec_team,
		client->sess.suicides,
		client->sess.team_kills,
		(mvc & 0xFFFF),
		((mvc >> 16) & 0xFFFF)
		// Damage and rounds played rolled in with weapon stats (below)
		// OSP

		, 
//		client->sess.experience,
		client->sess.auto_unmute_time,
		client->sess.ignoreClients[0],
		client->sess.ignoreClients[1],
		client->pers.enterTime,
		restart ? client->sess.spawnObjectiveIndex : 0,
		client->sess.ATB_count,
		// Dens: Needs to be saved to prevent spoofing
		// quad: I think this solves ticket #5, will need to test it at large now
		//       but at least ETTV clients don't get kicked anymore.
		client->sess.guid &&
			( !client->sess.guid || !Q_stricmp( client->sess.guid, "" ) ) ?
				client->sess.guid : "NOGUID",
		client->sess.ip &&
			( !client->sess.ip || !Q_stricmp( client->sess.ip, "" ) ) ?
				client->sess.ip : "NOIP",
		client->sess.mac &&
			( !client->sess.mac || !Q_stricmp( client->sess.mac, "" ) ) ?
				client->sess.mac : "NOMAC",
		client->sess.uci, //mcwf GeoIP
		client->sess.need_greeting,
		// quad: shoutcaster and ettv
		client->sess.shoutcaster,
		client->sess.ettv
		);

	trap_Cvar_Set( va( "session%i", client - level.clients ), s );

	// Arnout: store the clients stats (7) and medals (7)
	// addition: but only if it isn't a forced map_restart (done by someone on the console)
	// tjw: go ahead and write them if the server has g_XPSave with the 2 flag
	if( !(restart && !level.warmupTime) || (g_XPSave.integer & XPSF_NR_MAPRESET)) {
		s = va( "%.2f %.2f %.2f %.2f %.2f %.2f %.2f %i %i %i %i %i %i %i",
			client->sess.skillpoints[0],
			client->sess.skillpoints[1],
			client->sess.skillpoints[2],
			client->sess.skillpoints[3],
			client->sess.skillpoints[4],
			client->sess.skillpoints[5],
			client->sess.skillpoints[6],
			client->sess.medals[0],
			client->sess.medals[1],
			client->sess.medals[2],
			client->sess.medals[3],
			client->sess.medals[4],
			client->sess.medals[5],
			client->sess.medals[6]
			);

		trap_Cvar_Set( va( "sessionstats%i", client - level.clients ), s );
	}

	// tjw: don't bother saving weapon stats between rounds
	/*
	// OSP -- save weapon stats too
	if(!level.fResetStats)
		trap_Cvar_Set(va("wstats%i", client - level.clients), G_createStats(&g_entities[client - level.clients]));
	// OSP
	*/
}


/*
================
G_ClientSwap

Client swap handling
================
*/
void G_ClientSwap(gclient_t *client)
{
	int flags = 0;

	if(client->sess.sessionTeam == TEAM_AXIS) client->sess.sessionTeam = TEAM_ALLIES;
	else if(client->sess.sessionTeam == TEAM_ALLIES) client->sess.sessionTeam = TEAM_AXIS;

	// Swap spec invites as well
	if(client->sess.spec_invite & TEAM_AXIS) flags |= TEAM_ALLIES;
	if(client->sess.spec_invite & TEAM_ALLIES) flags |= TEAM_AXIS;

	client->sess.spec_invite = flags;

	// Swap spec follows as well
	flags = 0;
	if(client->sess.spec_team & TEAM_AXIS) flags |= TEAM_ALLIES;
	if(client->sess.spec_team & TEAM_ALLIES) flags |= TEAM_AXIS;

	client->sess.spec_team = flags;
}


void G_CalcRank( gclient_t* client ) {
	int i, highestskill = 0;

	for( i = 0; i < SK_NUM_SKILLS; i++ ) {
		G_SetPlayerSkill( client, i );
		if( client->sess.skill[i] > highestskill ) {
			highestskill = client->sess.skill[i];
		}
	}

	// set rank
	client->sess.rank = highestskill;

	if( client->sess.rank >=4 ) {
		int cnt = 0;

		// Gordon: count the number of maxed out skills
		for( i = 0; i < SK_NUM_SKILLS; i++ ) {
			if( client->sess.skill[ i ] >= 4 ) {
				cnt++;
			}
		}

		client->sess.rank = cnt + 3;
		if( client->sess.rank > 10 ) {
			client->sess.rank = 10;
		}
	}
}

/*
================
G_ReadSessionData

Called on a reconnect
================
*/
void G_ReadSessionData( gclient_t *client )
{
	int mvc_l, mvc_h, need_greeting;
	char s[MAX_STRING_CHARS];
	qboolean test;
	qboolean load = qfalse;

	trap_Cvar_VariableStringBuffer( va( "session%i", client - level.clients ), s, sizeof(s) );

	sscanf( s, "%i %i %i %i %i %i %i %i %i %i %i %i %i %i %f %f %f %f %i %i %i %i %i %i %i %i %i %i %i %i %i %s %s %s %u %d %i %i", //mcwf GeoIP
		(int *)&client->sess.sessionTeam,
		&client->sess.spectatorTime,
		(int *)&client->sess.spectatorState,
		&client->sess.spectatorClient,
		&client->sess.playerType,			// DHM - Nerve
		&client->sess.playerWeapon,			// DHM - Nerve
		&client->sess.playerWeapon2,
		&client->sess.latchPlayerType,		// DHM - Nerve
		&client->sess.latchPlayerWeapon,	// DHM - Nerve
		&client->sess.latchPlayerWeapon2,

		// OSP
		&client->sess.coach_team,
		&client->sess.deaths,
		&client->sess.game_points,
		&client->sess.kills,
		&client->sess.overall_killrating,
		&client->sess.overall_killvariance,
		&client->sess.rating,
		&client->sess.rating_variance,
		&client->sess.referee,
		&client->sess.spec_invite,
		&client->sess.spec_team,
		&client->sess.suicides,
		&client->sess.team_kills,
		&mvc_l,
		&mvc_h
		// Damage and round count rolled in with weapon stats (below)
		// OSP

		, 
//		&client->sess.experience,
		(int *)&client->sess.auto_unmute_time,
		&client->sess.ignoreClients[0],
		&client->sess.ignoreClients[1],
		&client->pers.enterTime,
		&client->sess.spawnObjectiveIndex,
		&client->sess.ATB_count,
		// Dens: Needed to prevent spoofing
		client->sess.guid,
		client->sess.ip,
		client->sess.mac,
		&client->sess.uci, //mcwf GeoIP
		&need_greeting,
		// quad: shoutcaster and ettv
		&client->sess.shoutcaster,
		&client->sess.ettv
		);

	client->sess.need_greeting = (need_greeting == 1) ? qtrue : qfalse;

	// OSP -- reinstate MV clients
	client->pers.mvReferenceList = (mvc_h << 16) | mvc_l;
	// OSP

	// tjw: don't bother storing weapon stats between rounds
	/*
	// OSP -- pull and parse weapon stats
	*s = 0;
	trap_Cvar_VariableStringBuffer(va("wstats%i", client - level.clients), s, sizeof(s));
	if(*s) {
		G_parseStats(s);
		if(g_gamestate.integer == GS_PLAYING) client->sess.rounds++;
	}
	// OSP
	*/

	// Arnout: likely there are more cases in which we don't want this
	// tjw: sorry this hurts my brain
	/*
	if( g_gametype.integer != GT_SINGLE_PLAYER &&
		g_gametype.integer != GT_COOP &&
		g_gametype.integer != GT_WOLF &&
		g_gametype.integer != GT_WOLF_STOPWATCH &&
		!(g_gametype.integer == GT_WOLF_CAMPAIGN && ( g_campaigns[level.currentCampaign].current == 0  || level.newCampaign ) ) &&
		!(g_gametype.integer == GT_WOLF_LMS && g_currentRound.integer == 0 ) ) {
	*/
	load = qfalse;
	if(g_gametype.integer == GT_WOLF_CAMPAIGN) {
		// tjw: only load scores back in if this map isn't starting 
		// a new campaign
		load = !(g_campaigns[level.currentCampaign].current == 0 
			|| level.newCampaign);
	}
	if(g_gametype.integer == GT_WOLF_LMS) {
		// tjw: only load scores back if this isn't the first round
		load = (g_currentRound.integer != 0);
	}

	switch(g_gametype.integer) {
	case GT_WOLF_CAMPAIGN:
	case GT_WOLF_LMS:
	case GT_WOLF_STOPWATCH:
	case GT_WOLF:
	case GT_WOLF_MAPVOTE:
		if(g_XPSave.integer & XPSF_NR_EVER)
			load = qtrue;
		if((g_XPSave.integer & XPSF_NR_MAPRESET) && g_reset.integer) 
			load = qtrue;
	}

	if(load) {
		trap_Cvar_VariableStringBuffer( va( "sessionstats%i", client - level.clients ), s, sizeof(s) );

		// Arnout: read the clients stats (7) and medals (7)
		sscanf( s, "%f %f %f %f %f %f %f %i %i %i %i %i %i %i",
			&client->sess.skillpoints[0],
			&client->sess.skillpoints[1],
			&client->sess.skillpoints[2],
			&client->sess.skillpoints[3],
			&client->sess.skillpoints[4],
			&client->sess.skillpoints[5],
			&client->sess.skillpoints[6],
			&client->sess.medals[0],
			&client->sess.medals[1],
			&client->sess.medals[2],
			&client->sess.medals[3],
			&client->sess.medals[4],
			&client->sess.medals[5],
			&client->sess.medals[6]
			);

	}

	G_CalcRank( client );

	test = (g_altStopwatchMode.integer != 0 || g_currentRound.integer == 1);

	if(g_gametype.integer == GT_WOLF_STOPWATCH && g_gamestate.integer != GS_PLAYING && test) {
		G_ClientSwap(client);
	}

	if ( g_swapteams.integer ) {
		trap_Cvar_Set( "g_swapteams", "0" );
		G_ClientSwap(client);
	}

	{
		int j;

		client->sess.startxptotal = 0;
		for( j = 0; j < SK_NUM_SKILLS; j++ ) {
			client->sess.startskillpoints[j] = client->sess.skillpoints[j];
			client->sess.startxptotal += client->sess.skillpoints[j];
		}
	}
}


/*
================
G_InitSessionData

Called on a first-time connect
================
*/
void G_InitSessionData( gclient_t *client, char *userinfo ) {
	clientSession_t	*sess;
//	const char		*value;

	sess = &client->sess;

	// pheno: set default values for guid, ip and mac address
	Q_strncpyz( sess->guid, "NOGUID", sizeof( sess->guid ) );
	Q_strncpyz( sess->ip, "NOIP", sizeof( sess->ip ) );
	Q_strncpyz( sess->mac, "NOMAC", sizeof( sess->mac ) );

	// initial team determination
	sess->sessionTeam = TEAM_SPECTATOR;

	sess->spectatorState = SPECTATOR_FREE;
	sess->spectatorTime = level.time;

	// DHM - Nerve
	sess->latchPlayerType = sess->playerType = 0;
	sess->latchPlayerWeapon = sess->playerWeapon = 0;
	sess->latchPlayerWeapon2 = sess->playerWeapon2 = 0;

	sess->spawnObjectiveIndex = 0;
	// dhm - end

	memset( sess->ignoreClients, 0, sizeof(sess->ignoreClients) );
//	sess->experience = 0;
	sess->auto_unmute_time = 0;
	memset( sess->skill, 0, sizeof(sess->skill) );
	memset( sess->skillpoints, 0, sizeof(sess->skillpoints) );
	// CHRUKER: b017 - startskillpoints didn't get reset
	memset( sess->startskillpoints, 0, sizeof(sess->startskillpoints) );
	memset( sess->medals, 0, sizeof(sess->medals) );
	sess->rank = 0;
	// CHRUKER: b017 - startxptotal didn't get reset
	sess->startxptotal = 0;


	// OSP
	sess->coach_team = 0;
	sess->referee = (client->pers.localClient) ? RL_REFEREE : RL_NONE;
	sess->spec_invite = 0;
	sess->spec_team = 0;
	G_deleteStats(client - level.clients);
	// OSP
	
	// josh:
	sess->overall_killrating = 0.0f;
	sess->overall_killvariance = SIGMA2_DELTA;
	//  rating = player rating now
	sess->rating = 0.0;
	sess->rating_variance = SIGMA2_THETA;

	sess->uci = 0;//mcwf GeoIP
	sess->need_greeting = qtrue; // redeye - moved greeting message to ClientBegin
	
	// quad - shoutcaster & ettv
	sess->ettv = ( atoi( Info_ValueForKey( userinfo, "protocol" ) ) == 284 );
	// pheno: grant shoutcaster status to ettv slave
	sess->shoutcaster = ( sess->ettv &&
		( g_ettvFlags.integer & ETTV_SHOUTCASTER ) );

	G_WriteClientSessionData( client, qfalse );
}


/*
==================
G_InitWorldSession

==================
*/
void G_InitWorldSession( void ) {
	char	s[MAX_STRING_CHARS];
	int		gt;
	int		i, j;

	trap_Cvar_VariableStringBuffer( "session", s, sizeof(s) );
	gt = atoi( s );
	
	// if the gametype changed since the last session, don't use any
	// client sessions
	if ( g_gametype.integer != gt ) {
		level.newSession = qtrue;
		level.fResetStats = qtrue;
		G_Printf( "Gametype changed, clearing session data.\n" );

	} else {
		char *tmp = s;
		qboolean test = (g_altStopwatchMode.integer != 0 || g_currentRound.integer == 1);


#define GETVAL(x) if((tmp = strchr(tmp, ' ')) == NULL) return; x = atoi(++tmp);

		// Get team lock stuff
		GETVAL(gt);
		teamInfo[TEAM_AXIS].spec_lock = (gt & TEAM_AXIS) ? qtrue : qfalse;
		teamInfo[TEAM_ALLIES].spec_lock = (gt & TEAM_ALLIES) ? qtrue : qfalse;
		GETVAL(level.lastBanner);
		GETVAL(level.mapsSinceLastXPReset);

		// See if we need to clear player stats
		// FIXME: deal with the multi-map missions
		if(g_gametype.integer != GT_WOLF_CAMPAIGN) {
			if((tmp = strchr(va("%s", tmp), ' ')) != NULL) {
				tmp++;
				trap_GetServerinfo(s, sizeof(s));
				if(Q_stricmp(tmp, Info_ValueForKey(s, "mapname"))) {
					level.fResetStats = qtrue;
					G_Printf("Map changed, clearing player stats.\n");
				}
			}
		}

		// OSP - have to make sure spec locks follow the right teams
		if(g_gametype.integer == GT_WOLF_STOPWATCH && g_gamestate.integer != GS_PLAYING && test) {
			G_swapTeamLocks();
		}

		if(g_swapteams.integer) {
			G_swapTeamLocks();
		}
	}

	for( i = 0; i < MAX_FIRETEAMS; i++ ) {
		char *p, *c;

		trap_Cvar_VariableStringBuffer( va("fireteam%i", i), s, sizeof(s) );

/*		p = Info_ValueForKey( s, "n" );

		if(p && *p) {
			Q_strncpyz( level.fireTeams[i].name, p, 32 );
			level.fireTeams[i].inuse = qtrue;
		} else {
			*level.fireTeams[i].name = '\0';
			level.fireTeams[i].inuse = qfalse;
		}*/

		p = Info_ValueForKey( s, "id" );
		j = atoi(p);
		if(!*p || j == -1) {
			level.fireTeams[i].inuse = qfalse;
		} else {
			level.fireTeams[i].inuse = qtrue;
		}
		level.fireTeams[i].ident = j + 1;

		p = Info_ValueForKey( s, "p" );
		level.fireTeams[i].priv = !atoi( p ) ? qfalse : qtrue;

		p = Info_ValueForKey( s, "i" );

		j = 0;
		if(p && *p) {
			c = p;
			for(c = strchr(c, ' ')+1; c && *c; ) {
				char str[8];
				char* l = strchr(c, ' ');
				if(!l) {
					break;
				}
				Q_strncpyz( str, c, l - c + 1 );
				str[l - c] = '\0';
				level.fireTeams[i].joinOrder[j++] = atoi(str);
				c = l + 1;
			}
		}

		for( ;j < MAX_CLIENTS; j++ ) {
			level.fireTeams[i].joinOrder[j] = -1;
		}
		G_UpdateFireteamConfigString(&level.fireTeams[i]);
	}
}

/*
==================
G_WriteSessionData

==================
*/
void G_WriteSessionData( qboolean restart ) {
	int i;
	char strServerInfo[MAX_INFO_STRING];
	int		j;




	trap_GetServerinfo(strServerInfo, sizeof(strServerInfo));
	trap_Cvar_Set("session", va("%i %i %d %d %s", g_gametype.integer,
											(teamInfo[TEAM_AXIS].spec_lock * TEAM_AXIS | teamInfo[TEAM_ALLIES].spec_lock * TEAM_ALLIES),
											level.lastBanner,
											level.mapsSinceLastXPReset,
											Info_ValueForKey(strServerInfo, "mapname")));

	// Keep stats for all players in sync
	for(i=0; !level.fResetStats && i<level.numConnectedClients; i++ ) {
		if((g_gamestate.integer == GS_WARMUP_COUNTDOWN &&
		  ((g_gametype.integer == GT_WOLF_STOPWATCH && level.clients[level.sortedClients[i]].sess.rounds >= 2) ||
		   (g_gametype.integer != GT_WOLF_STOPWATCH && level.clients[level.sortedClients[i]].sess.rounds >= 1))))
			   level.fResetStats = qtrue;
	}

	for(i=0; i<level.numConnectedClients; i++ ) {
		if(level.clients[level.sortedClients[i]].pers.connected == CON_CONNECTED) {
			G_WriteClientSessionData(&level.clients[level.sortedClients[i]], restart);
		// For slow connecters and a short warmup
		} else if(level.fResetStats) {
			G_deleteStats(level.sortedClients[i]);
		}
	}

	for( i = 0; i < MAX_FIRETEAMS; i++ ) {
		char buffer[MAX_STRING_CHARS];
		if(!level.fireTeams[i].inuse) {
			Com_sprintf(buffer, MAX_STRING_CHARS, "\\id\\-1");
		} else {
			char buffer2[MAX_STRING_CHARS];

			*buffer2 = '\0';
			for( j = 0; j < MAX_CLIENTS; j++ ) {
				char p[8];
				Com_sprintf(p, 8, " %i", level.fireTeams[i].joinOrder[j]);
				Q_strcat(buffer2, MAX_STRING_CHARS, p);
			}
	//		Com_sprintf(buffer, MAX_STRING_CHARS, "\\n\\%s\\i\\%s", level.fireTeams[i].name, buffer2);
			Com_sprintf(buffer, MAX_STRING_CHARS, "\\id\\%i\\i\\%s\\p\\%i", level.fireTeams[i].ident - 1, buffer2, level.fireTeams[i].priv ? 1 : 0 );
		}

		trap_Cvar_Set(va("fireteam%i", i), buffer);
	}
}
