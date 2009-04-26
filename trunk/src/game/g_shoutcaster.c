// g_shoutcaster.c

#include "g_local.h"

/*
================
G_IsShoutcastPasswordSet
================
*/
qboolean G_IsShoutcastPasswordSet( void )
{
	if( Q_stricmp( shoutcastPassword.string, "none" ) &&
		shoutcastPassword.string[0] ) {
		return qtrue;
	}

	return qfalse;
}

/*
================
G_IsShoutcastStatusAvailable
================
*/
qboolean G_IsShoutcastStatusAvailable( gentity_t *ent )
{
	if( !( ent->r.svFlags & SVF_BOT ) &&
		// NOTE: shoutcaster support will only be available with
		//       installed etpub client > 20090112
		ent->client->pers.etpubc <= 20090112 ) {
		return qfalse;
	}

	// check for available password
	if( G_IsShoutcastPasswordSet() ) {
		return qtrue;
	}

	return qfalse;
}

/*
================
G_MakeShoutcaster
================
*/
void G_MakeShoutcaster( gentity_t *ent )
{
	if( !ent || !ent->client ) {
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
G_RemoveShoutcaster
================
*/
void G_RemoveShoutcaster( gentity_t *ent )
{
	if( !ent || !ent->client ) {
		return;
	}

	ent->client->sess.shoutcaster = 0;
	
	if( !ent->client->sess.referee ) { // don't remove referee's invitation
		ent->client->sess.spec_invite = 0;
	}
	
	ClientUserinfoChanged( ent - g_entities );
}

/*
================
G_RemoveAllShoutcasters
================
*/
void G_RemoveAllShoutcasters( void )
{
	int i;

	for( i = 0; i < level.numConnectedClients; i++ ) {
		gclient_t *cl = &level.clients[level.sortedClients[i]];

		if( cl->sess.shoutcaster ) {
			cl->sess.shoutcaster = 0;

			// don't remove referee's invitation
			if( !cl->sess.referee ) {
				cl->sess.spec_invite = 0;
			}

			ClientUserinfoChanged( cl - level.clients );
		}
	}
}

/*
================
G_sclogin_cmd

Request for shoutcaster status
================
*/
void G_sclogin_cmd( gentity_t *ent, unsigned int dwCommand, qboolean fValue )
{
	char cmd[MAX_TOKEN_CHARS], pwd[MAX_TOKEN_CHARS];

	if( !ent || !ent->client ) {
		return;
	}

	trap_Argv( 0, cmd, sizeof( cmd ) );

	if( ClientIsFlooding( ent, qfalse ) ) {
		CP( va( "print \"^1Spam Protection: ^7dropping %s\n\"", cmd ) );
		return;
	}

	if( !G_IsShoutcastStatusAvailable( ent ) ) {
		CP( "print \"Sorry, shoutcaster status disabled on this server.\n\"" );
		return;
	}

	if( ent->client->sess.shoutcaster ) {
		CP( "print \"Sorry, you are already logged in as shoutcaster.\n\"" );
		return;
	}

	if( trap_Argc() < 2 ) {
		CP( va( "print \"Usage: %s [password]\n\"", cmd ) );
		return;
	}

	trap_Argv( 1, pwd, sizeof( pwd ) );

	if( Q_stricmp( pwd, shoutcastPassword.string ) ) {
		CP( "print \"Invalid shoutcaster password!\n\"" );
		return;
	}

	G_MakeShoutcaster( ent );
}

/*
================
G_sclogout_cmd

Removes shoutcaster status
================
*/
void G_sclogout_cmd( gentity_t *ent, unsigned int dwCommand, qboolean fValue )
{
	char cmd[MAX_TOKEN_CHARS];

	if( !ent || !ent->client ) {
		return;
	}

	trap_Argv( 0, cmd, sizeof( cmd ) );

	if( ClientIsFlooding( ent, qfalse ) ) {
		CP( va( "print \"^1Spam Protection: ^7dropping %s\n\"", cmd ) );
		return;
	}

	if( !G_IsShoutcastStatusAvailable( ent ) ) {
		CP( "print \"Sorry, shoutcaster status disabled on this server.\n\"" );
		return;
	}

	if( !ent->client->sess.shoutcaster ) {
		CP( "print \"Sorry, you are not logged in as shoutcaster.\n\"" );
		return;
	}

	G_RemoveShoutcaster( ent );
}

/*
================
G_makesc_cmd
================
*/
void G_makesc_cmd( void )
{
	char		cmd[MAX_TOKEN_CHARS], name[MAX_NAME_LENGTH];
	int			pcount, pids[MAX_CLIENTS];
	gentity_t	*ent;

	trap_Argv( 0, cmd, sizeof( cmd ) );

	if( trap_Argc() != 2 ) {
		G_Printf( "Usage: %s <slot#|name>\n", cmd );
		return;
	}

	if( !G_IsShoutcastPasswordSet() ) {
		G_Printf( "%s: Sorry, shoutcaster status disabled on this server.\n",
			cmd );
		return;
	}

	trap_Argv( 1, name, sizeof( name ) );
	pcount = ClientNumbersFromString( name, pids );

	if( pcount > 1 ) {
		G_Printf( "%s: More than one player matches. "
			"Be more specific or use the slot number.\n", cmd );
		return;
	} else if( pcount < 1 ) {
		G_Printf( "%s: No connected player found with that "
			"name or slot number.\n", cmd );
		return;
	}

	ent = pids[0] + g_entities;

	if( !ent || !ent->client ) {
		return;
	}

	if( ent->client->sess.shoutcaster ) {
		G_Printf( "%s: Sorry, %s^7 is already a shoutcaster.\n",
			cmd, ent->client->pers.netname );
		return;
	}

	G_MakeShoutcaster( ent );
}

/*
================
G_removesc_cmd
================
*/
void G_removesc_cmd( void )
{
	char		cmd[MAX_TOKEN_CHARS], name[MAX_NAME_LENGTH];
	int			pcount, pids[MAX_CLIENTS];
	gentity_t	*ent;

	trap_Argv( 0, cmd, sizeof( cmd ) );

	if( trap_Argc() != 2 ) {
		G_Printf( "Usage: %s <slot#|name>\n", cmd );
		return;
	}

	if( !G_IsShoutcastPasswordSet() ) {
		G_Printf( "%s: Sorry, shoutcaster status disabled on this server.\n",
			cmd );
		return;
	}

	trap_Argv( 1, name, sizeof( name ) );
	pcount = ClientNumbersFromString( name, pids );

	if( pcount > 1 ) {
		G_Printf( "%s: More than one player matches. "
			"Be more specific or use the slot number.\n", cmd );
		return;
	} else if( pcount < 1 ) {
		G_Printf( "%s: No connected player found with that "
			"name or slot number.\n", cmd );
		return;
	}

	ent = pids[0] + g_entities;

	if( !ent || !ent->client ) {
		return;
	}

	if( !ent->client->sess.shoutcaster ) {
		G_Printf( "%s: Sorry, %s^7 is not a shoutcaster.\n",
			cmd, ent->client->pers.netname );
		return;
	}

	G_RemoveShoutcaster( ent );
}
