// g_message.c

#include "g_local.h"

qboolean firstblood;

/*
================
G_PrintMessage
================
*/
void G_PrintMessage( char *message, int position )
{
	static char output[MAX_STRING_CHARS];

	// reduce to MAX_STRING_CHARS
	Q_strncpyz( output, message, sizeof( output ) );

	switch( position ) {
		case MSGPOS_CHAT:
		default:
			AP( va( "chat \"%s\" -1", output ) );
			break;
		case MSGPOS_CENTER:
			AP( va( "cp \"%s\"", output ) );
			break;
		case MSGPOS_LEFT_BANNER:
			AP( va( "cpm \"%s\"", output ) );
			break;
		case MSGPOS_BANNER:
			AP( va( "bp \"%s\"", output ) );
			break;
		case MSGPOS_CONSOLE:
			G_Printf( "%s\n", output );
			break;
	}
}

/*
================
G_FirstBloodSanitize
================
*/
char *G_FirstBloodSanitize( char *text )
{
	static char n[MAX_SAY_TEXT] = {""};

	if( !text || !*text ) {
		return n;
	}

	Q_strncpyz( n, text, sizeof( n ) );
	Q_strncpyz( n, Q_StrReplace( n, "[a]", "(a)" ), sizeof( n ) );
	Q_strncpyz( n, Q_StrReplace( n, "[v]", "(v)" ), sizeof( n ) );

	return n;
}

/*
================
G_FirstBloodMessage
================
*/
void G_FirstBloodMessage( gentity_t *attacker, gentity_t *victim )
{
	char aname[MAX_NAME_LENGTH] = {"*unknown*"};
	char vname[MAX_NAME_LENGTH] = {"*unknown*"};
	char *message;

	if( attacker ) {
		Q_strncpyz( aname,
			G_FirstBloodSanitize( attacker->client->pers.netname ),
			sizeof( aname ) );
	}

	if( victim ) {
		Q_strncpyz( vname,
			G_FirstBloodSanitize( victim->client->pers.netname ),
			sizeof( vname ) );
	}

	message = Q_StrReplace( g_firstBloodMsg.string, "[a]", aname );
	message = Q_StrReplace( message, "[v]", vname );

	if( g_firstBloodSound.string[0] ) {
		G_globalSound( g_firstBloodSound.string );
	}

	G_PrintMessage( message, g_firstBloodMsgPos.integer );
	
	firstblood = qtrue;
}

/*
================
G_LastBloodMessage
================
*/
void G_LastBloodMessage()
{
	gentity_t *ent = &g_entities[level.lastBloodClient];
	char name[MAX_NAME_LENGTH] = {"*unknown*"};

	if( !level.lastBloodClient || !g_lastBloodMsg.string[0] ) {
		return;
	}

	if( ent ) {
		Q_strncpyz( name, ent->client->pers.netname, sizeof( name ) );
	}

	G_PrintMessage( Q_StrReplace( g_lastBloodMsg.string, "[a]", name ), 0 );
}
