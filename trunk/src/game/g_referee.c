// G_referee.c: Referee handling
// -------------------------------
// 04 Apr 02
// rhea@OrangeSmoothie.org
//
#include "g_local.h"
#include "../ui/menudef.h"


//
// UGH!  Clean me!!!!
//

// Parses for a referee command.
//	--> ref arg allows for the server console to utilize all referee commands (ent == NULL)
//
qboolean G_refCommandCheck(gentity_t *ent, char *cmd)
{
		 if(!Q_stricmp(cmd, "allready"))	G_refAllReady_cmd(ent);
	else if(!Q_stricmp(cmd, "lock"))		G_refLockTeams_cmd(ent, qtrue);
	else if(!Q_stricmp(cmd, "help"))		G_refHelp_cmd(ent);
	else if(!Q_stricmp(cmd, "pause"))		G_refPause_cmd(ent, qtrue);
	else if(!Q_stricmp(cmd, "putallies"))	G_refPlayerPut_cmd(ent, TEAM_ALLIES);
	else if(!Q_stricmp(cmd, "putaxis"))		G_refPlayerPut_cmd(ent, TEAM_AXIS);
	else if(!Q_stricmp(cmd, "remove"))		G_refRemove_cmd(ent);
	else if(!Q_stricmp(cmd, "speclock"))	G_refSpeclockTeams_cmd(ent, qtrue);
	else if(!Q_stricmp(cmd, "specunlock"))	G_refSpeclockTeams_cmd(ent, qfalse);
	else if(!Q_stricmp(cmd, "unlock"))		G_refLockTeams_cmd(ent, qfalse);
	else if(!Q_stricmp(cmd, "unpause"))		G_refPause_cmd(ent, qfalse);
	else if(!Q_stricmp(cmd, "warmup"))		G_refWarmup_cmd(ent);
	else if(!Q_stricmp(cmd, "warn"))		G_refWarning_cmd(ent);
	// yada - handled by the vote functions now
	//else if(!Q_stricmp(cmd, "mute"))		G_refMute_cmd(ent, qtrue); 
	//else if(!Q_stricmp(cmd, "unmute"))		G_refMute_cmd(ent, qfalse);

	// pheno
	else if( !Q_stricmp( cmd, "makeshoutcaster" ) )
		G_refMakeShoutcaster_cmd( ent );
	else if( !Q_stricmp( cmd, "removeshoutcaster" ) )
		G_refRemoveShoutcaster_cmd( ent );
	else if( !Q_stricmp( cmd, "logout" ) )
		G_refLogout_cmd( ent );

	else return(qfalse);

	return(qtrue);
}


// Lists ref commands.
/*void G_refHelp_cmd(gentity_t *ent)
{
	// List commands only for enabled refs.
	if(ent) {
		// CHRUKER: b068 - This seems like a very redundant check 
		//          since the function is only called from the server
		//          console or through G_ref_cmd (the function 
		//          just below).
		//
		//if(!ent->client->sess.referee) {
		//	CP("cpm \"Sorry, you are not a referee!\n");
		//	return;
		//}
		//

		CP("print \"\n^3Referee commands:^7\n\"");
		CP(    "print \"------------------------------------------\n\"");

		G_voteHelp(ent, qfalse);
		// CHRUKER: b038 - Removed non-existing restart command
		// CHRUKER: b039 - Added <pid> parameter to remove command
		CP("print \"\n^5allready         putallies^7 <pid>  ^5speclock          warmup\n\"");
		CP(  "print \"^5lock             putaxis^7 <pid>    ^5specunlock        warn ^7<pid>\n\"");
		CP(  "print \"^5help             remove^7 <pid>     ^5unlock            mute ^7<pid>\n\"");
		CP(  "print \"^5pause            unpause           unmute ^7<pid>\n\"");

		CP(  "print \"Usage: ^3\\ref <cmd> [params]\n\n\"");

	// Help for the console
	} else {
		G_Printf("\nAdditional console commands:\n");
		G_Printf(  "----------------------------------------------\n");
		G_Printf(  "allready    putallies <pid>     unlock\n");
		G_Printf(  "lock        putaxis <pid>       unpause\n");
		G_Printf(  "help        warmup [value]\n");
		G_Printf(  "pause       speclock            warn <pid>\n");
		G_Printf(  "specunlock  remove <pid>\n\n");

		G_Printf(  "Usage: <cmd> [params]\n\n");
	}
}*/

// yada - new version since sv and game cmd are equal now
void G_refHelp_cmd(gentity_t *ent)
{
	G_refPrintf(ent,"\n^3Referee commands:");
	G_refPrintf(ent,"------------------------------------------");
	G_voteHelp(ent, qfalse);
	// CHRUKER: b038 - Removed non-existing restart command
	// CHRUKER: b039 - Added <pid> parameter to remove command
	G_refPrintf(ent,"------------------------------------------");
	G_refPrintf(ent,"^5allready         putallies^7 <pid>^5  speclock         warmup");
	G_refPrintf(ent,"^5lock             putaxis^7 <pid>^5    specunlock       warn^7 <pid>");
	G_refPrintf(ent,"^5help             remove^7 <pid>^5     unlock           logout");
	G_refPrintf(ent,"^5pause            makeshoutcaster^7 <pid>");
	G_refPrintf(ent,"^5unpause          removeshoutcaster^7 <pid>");
	G_refPrintf(ent,"------------------------------------------");
	G_refPrintf(ent,"Usage: ^3\\ref <cmd> [params]\n");
}


// Request for ref status or lists ref commands.
void G_ref_cmd(gentity_t *ent, unsigned int dwCommand, qboolean fValue)
{
	char arg[MAX_TOKEN_CHARS];

	// forty - in mod flood protection
	// yada - dont check this on console
	if(	ent && ClientIsFlooding(ent, qfalse) ){
		CP("print \"^1Spam Protection: ^7dropping ref\n\"");
		return;
	}
	
	// Roll through ref commands if already a ref
	if(ent == NULL || ent->client->sess.referee) {
		voteInfo_t votedata;

		trap_Argv(1, arg, sizeof(arg));

		memcpy( &votedata, &level.voteInfo, sizeof( voteInfo_t ) );

		if( Cmd_CallVote_f(ent, 0, qtrue) ) {
			memcpy( &level.voteInfo, &votedata, sizeof( voteInfo_t ) );
			return;
		} else {
			memcpy( &level.voteInfo, &votedata, sizeof( voteInfo_t ) );

			if(G_refCommandCheck(ent, arg)) {
				return;
			} else {
				G_refHelp_cmd(ent);
			}
		}
		return;
	}

	if(ent) {
		if(!Q_stricmp(refereePassword.string, "none") || !refereePassword.string[0]) {
			// CHRUKER: b046 - Was using the cpm command, but this is really just for the console.
			CP("print \"Sorry, referee status disabled on this server.\n\"");
			return;
		}

		if(trap_Argc() < 2) {
			// CHRUKER: b046 - Was using the cpm command, but this is really just for the console.
			CP("print \"Usage: ref [password]\n\"");
			return;
		}

		trap_Argv(1, arg, sizeof(arg));

		if(Q_stricmp(arg, refereePassword.string)) {
			// CHRUKER: b046 - Was using the cpm command, but this is really just for the console.
			CP("print \"Invalid referee password!\n\"");
			return;
		}

		ent->client->sess.referee = 1;
		ent->client->sess.spec_invite = TEAM_AXIS | TEAM_ALLIES;
		AP(va("cp \"%s\n^3has become a referee\n\"", ent->client->pers.netname));
		ClientUserinfoChanged( ent-g_entities );
	}
}


// Readies all players in the game.
void G_refAllReady_cmd(gentity_t *ent)
{
	int i;
	gclient_t *cl;

	if( g_gamestate.integer == GS_PLAYING ) {
// rain - #105 - allow allready in intermission
//	|| g_gamestate.integer == GS_INTERMISSION) {
		G_refPrintf(ent, "Match already in progress!");
		return;
	}

	// Ready them all and lock the teams
	for(i=0; i<level.numConnectedClients; i++ ) {
		cl = level.clients + level.sortedClients[i];
		if(cl->sess.sessionTeam != TEAM_SPECTATOR) cl->pers.ready = qtrue;
	}

	// Can we start?
	level.ref_allready = qtrue;
	G_readyMatchState();
}


// Changes team lock status
void G_refLockTeams_cmd(gentity_t *ent, qboolean fLock)
{
	char *status;

	teamInfo[TEAM_AXIS].team_lock = (TeamCount(-1, TEAM_AXIS)) ? fLock : qfalse;
	teamInfo[TEAM_ALLIES].team_lock = (TeamCount(-1, TEAM_ALLIES)) ? fLock : qfalse;

	status = va("Referee has ^3%sLOCKED^7 teams", ((fLock) ? "" : "UN"));

	G_printFull(status, ent);
	G_refPrintf(ent, "You have %sLOCKED teams\n", ((fLock) ? "" : "UN"));

	if( fLock ) {
		level.server_settings |= CV_SVS_LOCKTEAMS;
	} else {
		level.server_settings &= ~CV_SVS_LOCKTEAMS;
	}
	trap_SetConfigstring(CS_SERVERTOGGLES, va("%d", level.server_settings));
}


// Pause/unpause a match.
void G_refPause_cmd(gentity_t *ent, qboolean fPause)
{
	char *status[2] = { "^5UN", "^1" };
	char *referee = (ent) ? "Referee" : "ref";

	if((PAUSE_UNPAUSING >= level.match_pause && !fPause) || (PAUSE_NONE != level.match_pause && fPause)) {
		// CHRUKER: b047 - Remove unneeded \" and linebreak
		G_refPrintf(ent, "The match is already %sPAUSED!", status[fPause]);
		return;
	}

	if(ent && !G_cmdDebounce(ent, ((fPause)?"pause":"unpause"))) return;

	// Trigger the auto-handling of pauses
	if(fPause) {
		level.match_pause = 100 + ((ent) ? (1 + ent - g_entities) : 0);
		G_globalSound("sound/misc/referee.wav");
		G_spawnPrintf(DP_PAUSEINFO, level.time + 15000, NULL);
		AP(va("print \"^3%s ^1PAUSED^3 the match^3!\n", referee));
		// CHRUKER: b047 - Remove unneeded \" and linebreak
		CP(va("cp \"^3Match is ^1PAUSED^3! (^7%s^3)", referee));
		level.server_settings |= CV_SVS_PAUSE;
		trap_SetConfigstring(CS_SERVERTOGGLES, va("%d", level.server_settings));
	} else {
		AP(va("print \"\n^3%s ^5UNPAUSES^3 the match ... resuming in 10 seconds!\n\n\"", referee));
		level.match_pause = PAUSE_UNPAUSING;
		G_globalSound("sound/osp/prepare.wav");
		G_spawnPrintf(DP_UNPAUSING, level.time + 10, NULL);
		return;
	}
}


// Puts a player on a team.
void G_refPlayerPut_cmd(gentity_t *ent, int team_id)
{
	int pid;
	char arg[MAX_TOKEN_CHARS];
	gentity_t *player;

	// Works for teamplayish matches
	if(g_gametype.integer < GT_WOLF) {
		G_refPrintf(ent, "\"put[allies|axis]\" only for team-based games!");
		return;
	}
	
	// yada - yeah right not giving an arg will end up as slot 0...
	// fixme: could maybe also be handled in ClientNumberFromString
	// if(ent&&!*s) return ent-g_entities;
	if(trap_Argc()!=3){
		G_refPrintf(ent,"Usage: \\ref put[allies|axis] <pid>");
		return;
	}

	// Find the player to place.
	trap_Argv(2, arg, sizeof(arg));
	if((pid = ClientNumberFromString(ent, arg)) == -1) return;

	player = g_entities + pid;

	// Can only move to other teams.
	if(player->client->sess.sessionTeam == team_id) {
		// CHRUKER: b047 - Remove unneeded linebreak
		G_refPrintf(ent, "\"%s\" is already on team %s!", player->client->pers.netname, aTeams[team_id]);
		return;
	}

	if(team_maxplayers.integer && TeamCount(-1, team_id) >= team_maxplayers.integer) {
		// CHRUKER: b047 - Remove unneeded linebreak
		G_refPrintf(ent, "Sorry, the %s team is already full!",  aTeams[team_id]);
		return;
	}

	player->client->pers.invite = team_id;
	player->client->pers.ready = qfalse;

	if( team_id == TEAM_AXIS ) {
		SetTeam( player, "red", qtrue, -1, -1, qfalse );
	} else {
		SetTeam( player, "blue", qtrue, -1, -1, qfalse );
	}

	if(g_gamestate.integer == GS_WARMUP || g_gamestate.integer == GS_WARMUP_COUNTDOWN)
		G_readyMatchState();
}


// Removes a player from a team.
void G_refRemove_cmd(gentity_t *ent)
{
	int pid;
	char arg[MAX_TOKEN_CHARS];
	gentity_t *player;

	// Works for teamplayish matches
	if(g_gametype.integer < GT_WOLF) {
		G_refPrintf(ent, "\"remove\" only for team-based games!");
		return;
	}
	
	// yada - yeah right not giving an arg will end up as slot 0...
	// fixme: could maybe also be handled in ClientNumberFromString
	// if(ent&&!*s) return ent-g_entities;
	if(trap_Argc()!=3){
		G_refPrintf(ent,"Usage: \\ref remove <pid>");
		return;
	}

	// Find the player to remove.
	trap_Argv(2, arg, sizeof(arg));
	if((pid = ClientNumberFromString(ent, arg)) == -1) return;

	player = g_entities + pid;

	// Can only remove active players.
	if(player->client->sess.sessionTeam == TEAM_SPECTATOR) {
		G_refPrintf(ent, "You can only remove people in the game!");
		return;
	}

	// Announce the removal
	AP(va("cp \"%s\n^7removed from team %s\n\"", player->client->pers.netname, aTeams[player->client->sess.sessionTeam]));
	CPx(pid, va("print \"^5You've been removed from the %s team\n\"", aTeams[player->client->sess.sessionTeam]));

	SetTeam( player, "s", qtrue, -1, -1, qfalse );

	if(g_gamestate.integer == GS_WARMUP || g_gamestate.integer == GS_WARMUP_COUNTDOWN) {
		G_readyMatchState();
	}
}


// Changes team spectator lock status
void G_refSpeclockTeams_cmd(gentity_t *ent, qboolean fLock)
{
	char *status;

	// Ensure proper locking
	G_updateSpecLock(TEAM_AXIS, (TeamCount(-1, TEAM_AXIS)) ? fLock : qfalse);
	G_updateSpecLock(TEAM_ALLIES, (TeamCount(-1, TEAM_ALLIES)) ? fLock : qfalse);

	status = va("Referee has ^3SPECTATOR %sLOCKED^7 teams", ((fLock) ? "" : "UN"));

	G_printFull(status, ent);

	// Update viewers as necessary
//	G_pollMultiPlayers();

	if( fLock ) {
		level.server_settings |= CV_SVS_LOCKSPECS;
	} else {
		level.server_settings &= ~CV_SVS_LOCKSPECS;
	}
	trap_SetConfigstring(CS_SERVERTOGGLES, va("%d", level.server_settings));
}

void G_refWarmup_cmd(gentity_t* ent)
{
	char cmd[MAX_TOKEN_CHARS];

	trap_Argv(2, cmd, sizeof(cmd));

	if(!*cmd || atoi(cmd) < 0) {
		trap_Cvar_VariableStringBuffer( "g_warmup", cmd, sizeof(cmd));
		// CHRUKER: b047 - Remove unneeded linebreak
		G_refPrintf(ent, "Warmup Time: %d", atoi(cmd));
		return;
	}

	trap_Cvar_Set("g_warmup", va("%d", atoi(cmd)));
}

void G_refWarning_cmd(gentity_t* ent)
{
	char	cmd[MAX_TOKEN_CHARS];
	char	reason[MAX_TOKEN_CHARS];
	int		kicknum;

	trap_Argv(2, cmd, sizeof(cmd));

	if(!*cmd) {
		G_refPrintf(ent, "usage: ref warn <clientname> [reason]." );
		return;
	}

	trap_Argv( 3, reason, sizeof( reason ) );		

	kicknum = G_refClientnumForName(ent, cmd);

	if (kicknum != MAX_CLIENTS ) {
		if( level.clients[kicknum].sess.referee == RL_NONE || ((!ent || ent->client->sess.referee == RL_RCON) && level.clients[kicknum].sess.referee <= RL_REFEREE) ) {
			trap_SendServerCommand( -1, va( "cpm \"%s^7 was issued a ^1Warning^7 (%s)\n\"\n", level.clients[kicknum].pers.netname, *reason ? reason : "No Reason Supplied" ));
		} else {
			G_refPrintf( ent, "Insufficient rights to issue client a warning." );
		}
	}
}

// (Un)Mutes a player
// yada - just a less intelligent version of G_Mute_v... bye bye
/*void G_refMute_cmd(gentity_t *ent, qboolean mute)
{
	int pid, isMuted = 0;
	char arg[MAX_TOKEN_CHARS];
	gentity_t *player;

	// Find the player to mute.
	trap_Argv(2, arg, sizeof(arg));
	if((pid = ClientNumberFromString(ent, arg)) == -1) return;

	player = g_entities + pid;
	
	// CHRUKER: b060 - Added mute check so that players that are muted
	//          before granted referee status, can be unmuted
	if(player->client->sess.referee != RL_NONE && mute) {
		// CHRUKER: b047 - Remove unneeded linebreak
		G_refPrintf(ent, "Cannot mute a referee." );
		return;
	}

	if(player->client->sess.auto_unmute_time != 0){
		isMuted = 1;
	}

	if(isMuted == mute) {
		// CHRUKER: b047 - Remove unneeded linebreak
		G_refPrintf(ent, "\"%s^*\" %s", player->client->pers.netname, mute ? "is already muted!" : "is not muted!" );
		return;
	}

	if( mute ) {
		CPx(pid, "print \"^5You've been muted\n\"" );
		player->client->sess.auto_unmute_time = -1;
		G_Printf( "\"%s^*\" has been muted\n",  player->client->pers.netname );
		ClientUserinfoChanged( pid );
	} else {
		CPx(pid, "print \"^5You've been unmuted\n\"" );
		player->client->sess.auto_unmute_time = 0;
		G_Printf( "\"%s^*\" has been unmuted\n",  player->client->pers.netname );
		ClientUserinfoChanged( pid );
	}
}*/

/*
================
G_refMakeShoutcaster_cmd
================
*/
void G_refMakeShoutcaster_cmd( gentity_t *ent )
{
	int			pid;
	char		name[MAX_NAME_LENGTH];
	gentity_t	*player;

	if( trap_Argc() != 3 ) {
		G_refPrintf( ent, "Usage: \\ref makeshoutcaster <pid>" );
		return;
	}

	if( !G_IsShoutcastPasswordSet() ) {
		G_refPrintf( ent,
			"Sorry, shoutcaster status disabled on this server." );
		return;
	}

	trap_Argv( 2, name, sizeof( name ) );
	
	if( ( pid = ClientNumberFromString( ent, name ) ) == -1 ) {
		return;
	}

	player = g_entities + pid;

	if( !player || !player->client ) {
		return;
	}

	// ignore bots
	if( player->r.svFlags & SVF_BOT ) {
		G_refPrintf( ent, "Sorry, a bot can not be a shoutcaster." );
		return;
	}

	if( player->client->sess.shoutcaster ) {
		G_refPrintf( ent, "Sorry, %s^7 is already a shoutcaster.",
			player->client->pers.netname );
		return;
	}

	G_MakeShoutcaster( player );
}

/*
================
G_refRemoveShoutcaster_cmd
================
*/
void G_refRemoveShoutcaster_cmd( gentity_t *ent )
{
	int			pid;
	char		name[MAX_NAME_LENGTH];
	gentity_t	*player;

	if( trap_Argc() != 3 ) {
		G_refPrintf( ent, "Usage: \\ref removeshoutcaster <pid>" );
		return;
	}

	if( !G_IsShoutcastPasswordSet() ) {
		G_refPrintf( ent,
			"Sorry, shoutcaster status disabled on this server." );
		return;
	}

	trap_Argv( 2, name, sizeof( name ) );
	
	if( ( pid = ClientNumberFromString( ent, name ) ) == -1 ) {
		return;
	}

	player = g_entities + pid;

	if( !player || !player->client ) {
		return;
	}

	if( !player->client->sess.shoutcaster ) {
		G_refPrintf( ent, "Sorry, %s^7 is not a shoutcaster.",
			player->client->pers.netname );
		return;
	}

	G_RemoveShoutcaster( player );
}

/*
================
G_refLogout_cmd
================
*/
void G_refLogout_cmd( gentity_t *ent )
{ 
	if( ent->client->sess.referee == RL_REFEREE ) {
		ent->client->sess.referee = RL_NONE;
		ClientUserinfoChanged( ent->s.clientNum );
	}
}

//////////////////////////////
//  Client authentication
//
void Cmd_AuthRcon_f(gentity_t *ent)
{
	char buf[MAX_TOKEN_CHARS], cmd[MAX_TOKEN_CHARS];

	trap_Cvar_VariableStringBuffer("rconPassword", buf, sizeof(buf));
	trap_Argv(1, cmd, sizeof( cmd));

	if(*buf && !strcmp(buf, cmd)) {
		ent->client->sess.referee = RL_RCON;
	}
}


//////////////////////////////
//  Console admin commands
//
void G_PlayerBan()
{
	char	cmd[MAX_TOKEN_CHARS];
	int		bannum;

	trap_Argv( 1, cmd, sizeof( cmd ) );

	if(!*cmd) {
		G_Printf( "usage: ban <clientname>." );
		return;
	}

	bannum = G_refClientnumForName(NULL, cmd);

	if(bannum != MAX_CLIENTS ) {
//		if( level.clients[bannum].sess.referee != RL_RCON ) {
			const char* value;
			char userinfo[MAX_INFO_STRING];

			// Dens: use the stored IP to prevent spoofing
			trap_GetUserinfo( bannum, userinfo, sizeof( userinfo ) );
			if( !(g_spoofOptions.integer & SPOOFOPT_USERINFO_IP)){
				value = level.clients[bannum].sess.ip;
			}else{
				value = Info_ValueForKey (userinfo, "ip");
			}

			AddIPBan( value );
//		} else {
//			G_Printf( "^3*** Can't ban a superuser!\n" );
//		}
	}
}

void G_MakeReferee()
{
	char	cmd[MAX_TOKEN_CHARS];
	int		cnum;

	trap_Argv( 1, cmd, sizeof( cmd ) );

	if(!*cmd) {
		G_Printf( "usage: MakeReferee <clientname>." );
		return;
	}

	cnum = G_refClientnumForName(NULL, cmd);

	if (cnum != MAX_CLIENTS ) {
		if(level.clients[cnum].sess.referee == RL_NONE) {
			level.clients[cnum].sess.referee = RL_REFEREE;
			AP(va("cp \"%s\n^3has been made a referee\n\"", cmd));
			G_Printf("%s has been made a referee.\n", cmd);
		} else {
			G_Printf("User is already authed.\n");
		}
	}
}

void G_RemoveReferee()
{ 
	char	cmd[MAX_TOKEN_CHARS];
	int		cnum;

	trap_Argv( 1, cmd, sizeof( cmd ) );

	if(!*cmd) {
		G_Printf( "usage: RemoveReferee <clientname>." );
		return;
	}

	cnum = G_refClientnumForName(NULL, cmd);

	if (cnum != MAX_CLIENTS ) {
		if( level.clients[cnum].sess.referee == RL_REFEREE ) {
			level.clients[cnum].sess.referee = RL_NONE;
			G_Printf( "%s is no longer a referee.\n", cmd );
		} else {
			G_Printf( "User is not a referee.\n" );
		}
	}
}

void G_MuteClient()
{ 
	char	cmd[MAX_TOKEN_CHARS];
	int		cnum;

	trap_Argv( 1, cmd, sizeof( cmd ) );

	if(!*cmd) {
		G_Printf( "usage: Mute <clientname>." );
		return;
	}

	cnum = G_refClientnumForName(NULL, cmd);

	if (cnum != MAX_CLIENTS ) {
		if( level.clients[cnum].sess.referee != RL_RCON ) {
			trap_SendServerCommand( cnum, va( "cpm \"^3You have been muted\"") );
			level.clients[cnum].sess.auto_unmute_time = -1;
			G_Printf( "%s^* has been muted\n", cmd);
			ClientUserinfoChanged( cnum );
		} else {
			G_Printf( "Cannot mute a referee.\n" );
		}
	}
}

void G_UnMuteClient()
{ 
	char	cmd[MAX_TOKEN_CHARS];
	int		cnum;

	trap_Argv( 1, cmd, sizeof( cmd ) );

	if(!*cmd) {
		G_Printf( "usage: Unmute <clientname>.\n" );
		return;
	}

	cnum = G_refClientnumForName(NULL, cmd);

	if (cnum != MAX_CLIENTS ) {
		if( level.clients[cnum].sess.auto_unmute_time != 0 ) {
			trap_SendServerCommand( cnum, va( "cpm \"^2You have been un-muted\"") );
			level.clients[cnum].sess.auto_unmute_time = 0;
			G_Printf( "%s has been un-muted\n", cmd);
			ClientUserinfoChanged( cnum );
		} else {
			G_Printf( "User is not muted.\n" );
		}
	}
}


/////////////////
//   Utility
//
int G_refClientnumForName(gentity_t *ent, const char *name)
{
	char	cleanName[MAX_TOKEN_CHARS];
	int		i;

	if(!*name) return(MAX_CLIENTS);

	for(i=0; i<level.numConnectedClients; i++) {
		Q_strncpyz(cleanName, level.clients[level.sortedClients[i]].pers.netname, sizeof(cleanName));
		Q_CleanStr(cleanName);
		if(!Q_stricmp( cleanName, name)) return(level.sortedClients[i]);
	}

	G_refPrintf(ent, "Client not on server.");

	return(MAX_CLIENTS);
}

void G_refPrintf( gentity_t* ent, const char *fmt, ... )
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, fmt);
	Q_vsnprintf (text, sizeof(text), fmt, argptr);
	va_end (argptr);
	// CHRUKER: b046 - Added the linebreak to the string.
	if(ent == NULL){
		// yada - in server console color codes look stupid
		ConsolizeString(text,text);
		trap_Printf(va("%s\n",text));
	}else{
		// CHRUKER: b046 - Was using the cpm command, but this is really just for the console.
		CP(va("print \"%s\n\"", text));
	}
}
