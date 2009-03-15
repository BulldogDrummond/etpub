// g_shoutcaster.c

#include "g_local.h"

/*
================
G_ShoutcasterStatusAvailable
================
*/
qboolean G_ShoutcasterStatusAvailable( gentity_t *ent )
{
	// NOTE: shoutcaster support will only be available with
	//       installed etpub client > 20090112
	if( ent->client->pers.etpubc > 20090112 &&
		Q_stricmp( shoutcastPassword.string, "none" ) &&
		shoutcastPassword.string[0] ) {
		return qtrue;
	}

	CP( "print \"Sorry, shoutcaster status disabled on this server.\n\"" );

	return qfalse;
}

/*
================
G_sclogin_cmd

Request for shoutcaster status
================
*/
void G_sclogin_cmd( gentity_t *ent, unsigned int dwCommand, qboolean fValue )
{
	char	cmd[MAX_TOKEN_CHARS], pwd[MAX_TOKEN_CHARS];

	if( !ent || !ent->client ) {
		return;
	}

	trap_Argv( 0, cmd, sizeof( cmd ) );

	if(	ClientIsFlooding( ent, qfalse ) ) {
		CP( va( "print \"^1Spam Protection: ^7dropping %s\n\"", cmd ) );
		return;
	}

	if( !G_ShoutcasterStatusAvailable( ent ) ) {
		return;
	}

	if( ent->client->sess.shoutcaster ) {
		CP( "print \"Sorry, you are already a shoutcaster.\n\"" );
		return;
	}

	if( trap_Argc() < 2 ) {
		CP( "print \"Usage: ... [password]\n\"" );
		return;
	}

	trap_Argv( 1, pwd, sizeof( pwd ) );

	if( Q_stricmp( pwd, shoutcastPassword.string ) ) {
		CP( "print \"Invalid shoutcaster password!\n\"" );
		return;
	}

	// move the player to spectators
	if( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		SetTeam( ent, "spectator", qtrue, -1, -1, qfalse );
	}

	ent->client->sess.shoutcaster = 1;
	ent->client->sess.spec_invite = TEAM_AXIS | TEAM_ALLIES;
	AP( va( "cp \"%s\n^3has become a shoutcaster\n\"",
		ent->client->pers.netname ) );
	ClientUserinfoChanged( ent - g_entities );
}

/*
================
G_sclogout_cmd

Removes shoutcaster status
================
*/
void G_sclogout_cmd( gentity_t *ent, unsigned int dwCommand, qboolean fValue )
{
	char	cmd[MAX_TOKEN_CHARS];

	if( !ent || !ent->client ) {
		return;
	}

	trap_Argv( 0, cmd, sizeof( cmd ) );

	if(	ClientIsFlooding( ent, qfalse ) ) {
		CP( va( "print \"^1Spam Protection: ^7dropping %s\n\"", cmd ) );
		return;
	}

	if( !G_ShoutcasterStatusAvailable( ent ) ) {
		return;
	}

	if( !ent->client->sess.shoutcaster ) {
		CP( "print \"Sorry, you are not logged in as shoutcaster.\n\"" );
		return;
	}

	ent->client->sess.shoutcaster = 0;
	ent->client->sess.spec_invite = 0;
	ClientUserinfoChanged( ent - g_entities );
}
