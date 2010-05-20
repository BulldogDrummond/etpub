// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"

// this file is only included when building a dll
// g_syscalls.asm is included instead when building a qvm

static int (QDECL *syscall)( int arg, ... ) = (int (QDECL *)( int, ...))-1;

#if __GNUC__ >= 4
#pragma GCC visibility push(default)
#endif
void dllEntry( int (QDECL *syscallptr)( int arg,... ) ) {
	syscall = syscallptr;
}
#if __GNUC__ >= 4
#pragma GCC visibility pop
#endif

int PASSFLOAT( float x ) {
	float	floatTemp;
	floatTemp = x;
	return *(int *)&floatTemp;
}

void	trap_Printf( const char *fmt ) {
	syscall( G_PRINT, fmt );
}

void	trap_Error( const char *fmt ) {
	syscall( G_ERROR, fmt );
}

int		trap_Milliseconds( void ) {
	return syscall( G_MILLISECONDS ); 
}
int		trap_Argc( void ) {
	return syscall( G_ARGC );
}

void	trap_Argv( int n, char *buffer, int bufferLength ) {
	syscall( G_ARGV, n, buffer, bufferLength );
}

int		trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	return syscall( G_FS_FOPEN_FILE, qpath, f, mode );
}

void	trap_FS_Read( void *buffer, int len, fileHandle_t f ) {
	syscall( G_FS_READ, buffer, len, f );
}

int		trap_FS_Write( const void *buffer, int len, fileHandle_t f ) {
	return syscall( G_FS_WRITE, buffer, len, f );
}

int		trap_FS_Rename( const char *from, const char *to ) {
	return syscall( G_FS_RENAME, from, to );
}

void	trap_FS_FCloseFile( fileHandle_t f ) {
	syscall( G_FS_FCLOSE_FILE, f );
}

int trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize ) {
	return syscall( G_FS_GETFILELIST, path, extension, listbuf, bufsize );
}

void	trap_SendConsoleCommand( int exec_when, const char *text ) {
	syscall( G_SEND_CONSOLE_COMMAND, exec_when, text );
}

qboolean G_CvarUpdateSafe(const char *var_name, const char *value)
{
	const char *serverinfo_cvars[] = {
		"mod_url",
		"mod_version",
		"g_tyranny",
		"P",
		"voteFlags",
		"g_balancedteams",
		"g_bluelimbotime",
		"g_redlimbotime",
		"gamename",
		"g_gametype",
		"g_voteFlags",
		"g_alliedmaxlives",
		"g_axismaxlives",
		"g_minGameClients",
		"g_needpass",
		"sv_allowAnonymous",
		"sv_privateClients",
		"mapname",
		"protocol",
		"version",
		"g_maxlivesRespawnPenalty",
		"g_maxGameClients",
		"sv_maxclients",
		"timelimit",
		"g_heavyWeaponRestriction",
		"g_antilag",
		"g_maxlives",
		"g_friendlyFire",
		"sv_floodProtect",
		"sv_maxPing",
		"sv_minPing",
		"sv_maxRate",
		"sv_minguidage",
		"sv_punkbuster",
		"sv_hostname",
		"Players_Axis",
		"Players_Allies",
		"campaign_maps",
		"C",
		"g_medicChargeTime"
		"g_LTChargeTime"	
		"g_engineerChargeTime"	
		"g_soldierChargeTime"	
		"g_covertopsChargeTime",
		"g_damageXP",
		"g_damageXPLevel",
		NULL
	};
	char cs[MAX_INFO_STRING] = {""};
	int i = 0;
	int size = 0;

	if(!var_name)
		return qtrue;
	if(!value)
		return qtrue;

	for(i=0; serverinfo_cvars[i]; i++) {
		if(!Q_stricmp(serverinfo_cvars[i], var_name)) {
			trap_GetServerinfo(cs, sizeof(cs));
			size = strlen(cs);
			Info_SetValueForKey(cs, var_name, value);
			if((size + 1) >= MAX_INFO_STRING) {
				G_Printf("WARNING: skipping SERVERINFO cvar "
					"set for %s (MAX_INFO_STRING "
					"protection)\n",
					var_name);
				return qfalse;
			}
			// tjw: q3 engine always makes the first char of
			//      GAMESTATE null 
			if((level.csLenTotal + size + 1 - level.csLen[0])
				>= (MAX_GAMESTATE_CHARS - 1)) {

				G_Printf("WARNING: skipping SERVERINFO cvar "
					"set for %s (MAX_GAMESTATE_CHARS "
					"protection)\n",
					var_name);
				return qfalse;
			}
			level.csLenTotal += (size - level.csLen[0] + 1);
			level.csLen[0] = (size + 1);
			return qtrue;
		}
	}
	return qtrue;
}



void	trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags ) {
	if(!G_CvarUpdateSafe(var_name, value)) {
		syscall( G_CVAR_REGISTER, cvar, var_name, "", flags );
	}
	else {
		syscall( G_CVAR_REGISTER, cvar, var_name, value, flags );
	}
}

void	trap_Cvar_Update( vmCvar_t *cvar ) {
	syscall( G_CVAR_UPDATE, cvar );
}

void trap_Cvar_Set( const char *var_name, const char *value ) {
	if(!G_CvarUpdateSafe(var_name, value))
		return;
	syscall( G_CVAR_SET, var_name, value );
}

void trap_Cvar_Setf( const char *var_name, const char *fmt, ...) {
  va_list ap;
  char cvs[MAX_CVAR_VALUE_STRING];

  va_start(ap, fmt);
  Q_vsnprintf(cvs, MAX_CVAR_VALUE_STRING, fmt, ap);
  syscall( G_CVAR_SET, var_name, cvs);
}

int trap_Cvar_VariableIntegerValue( const char *var_name ) {
	return syscall( G_CVAR_VARIABLE_INTEGER_VALUE, var_name );
}

void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	syscall( G_CVAR_VARIABLE_STRING_BUFFER, var_name, buffer, bufsize );
}

void trap_Cvar_LatchedVariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	syscall( G_CVAR_LATCHEDVARIABLESTRINGBUFFER, var_name, buffer, bufsize );
}

void trap_LocateGameData( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t,
						 playerState_t *clients, int sizeofGClient ) {
	syscall( G_LOCATE_GAME_DATA, gEnts, numGEntities, sizeofGEntity_t, clients, sizeofGClient );
}

void trap_DropClient( int clientNum, const char *reason, int length ) {
	syscall( G_DROP_CLIENT, clientNum, reason, length );
}

void trap_SendServerCommand( int clientNum, const char *text ) {
	// rain - #433 - commands over 1022 chars will crash the
	// client engine upon receipt, so ignore them
	if( strlen( text ) > 1022 ) {
		G_LogPrintf( "%s: trap_SendServerCommand( %d, ... ) length exceeds 1022.\n", GAMEVERSION, clientNum );
		G_LogPrintf( "%s: text [%s]\n", GAMEVERSION, text );
		return;
	}
	syscall( G_SEND_SERVER_COMMAND, clientNum, text );
}

void trap_SetConfigstring( int num, const char *string ) {
	int size = 0;

	// tjw: keep track of MAX_GAMESTATE_CHARS
	if(!string) 
		string = "";

	size = strlen(string);

	// tjw: q3 engine always makes the first char of GAMESTATE null 
	if((level.csLenTotal + size + 1 - level.csLen[num])
		>= (MAX_GAMESTATE_CHARS - 1)) {

		G_Printf("WARNING: aborted SetConfigString %i. "
			"(MAX_GAMESTATE_CHARS protection)\n"
			"value: %s\n", num, string);
		return; 
	}
	level.csLenTotal += (size - level.csLen[num] + 1);
	level.csLen[num] = (size + 1);
	syscall( G_SET_CONFIGSTRING, num, string );
}

void trap_GetConfigstring( int num, char *buffer, int bufferSize ) {
	syscall( G_GET_CONFIGSTRING, num, buffer, bufferSize );
}

void trap_GetUserinfo( int num, char *buffer, int bufferSize ) {
	syscall( G_GET_USERINFO, num, buffer, bufferSize );
}

void trap_SetUserinfo( int num, const char *buffer ) {
	syscall( G_SET_USERINFO, num, buffer );
}

void trap_GetServerinfo( char *buffer, int bufferSize ) {
	syscall( G_GET_SERVERINFO, buffer, bufferSize );
}

void trap_SetBrushModel( gentity_t *ent, const char *name ) {
	syscall( G_SET_BRUSH_MODEL, ent, name );
}

void trap_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	gentity_t *hitEnt = NULL;

	syscall(G_TRACE,
		results,
		start,
		mins,
		maxs,
		end,
		passEntityNum,
		contentmask);

	// tjw: the second trace is only necessary if using
	//      one of the g_hitboxes flags
	if(!g_hitboxes.integer) return;

	hitEnt = &g_entities[results->entityNum];
	if(hitEnt && hitEnt->client && hitEnt->takedamage) {
		float oldMaxZ = hitEnt->r.maxs[2];
		float newMaxZ = ClientHitboxMaxZ(hitEnt);

		hitEnt->r.maxs[2] = newMaxZ;
		memset(results, 0, sizeof(results));
		syscall(G_TRACE,
			results,
			start,
			mins,
			maxs,
			end,
			passEntityNum,
			contentmask);
		hitEnt->r.maxs[2] = oldMaxZ;
	}
}

void trap_TraceNoEnts( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	syscall( G_TRACE, results, start, mins, maxs, end, -2, contentmask );
}

void trap_TraceCapsule( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	syscall( G_TRACECAPSULE, results, start, mins, maxs, end, passEntityNum, contentmask );
}

void trap_TraceCapsuleNoEnts( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	syscall( G_TRACECAPSULE, results, start, mins, maxs, end, -2, contentmask );
}

int trap_PointContents( const vec3_t point, int passEntityNum ) {
	return syscall( G_POINT_CONTENTS, point, passEntityNum );
}


qboolean trap_InPVS( const vec3_t p1, const vec3_t p2 ) {
	return syscall( G_IN_PVS, p1, p2 );
}

qboolean trap_InPVSIgnorePortals( const vec3_t p1, const vec3_t p2 ) {
	return syscall( G_IN_PVS_IGNORE_PORTALS, p1, p2 );
}

void trap_AdjustAreaPortalState( gentity_t *ent, qboolean open ) {
	syscall( G_ADJUST_AREA_PORTAL_STATE, ent, open );
}

qboolean trap_AreasConnected( int area1, int area2 ) {
	return syscall( G_AREAS_CONNECTED, area1, area2 );
}

void trap_LinkEntity( gentity_t *ent ) {
	syscall( G_LINKENTITY, ent );
}

void trap_UnlinkEntity( gentity_t *ent ) {
	syscall( G_UNLINKENTITY, ent );
}


int trap_EntitiesInBox( const vec3_t mins, const vec3_t maxs, int *list, int maxcount ) {
	return syscall( G_ENTITIES_IN_BOX, mins, maxs, list, maxcount );
}

qboolean trap_EntityContact( const vec3_t mins, const vec3_t maxs, const gentity_t *ent ) {
	return syscall( G_ENTITY_CONTACT, mins, maxs, ent );
}

qboolean trap_EntityContactCapsule( const vec3_t mins, const vec3_t maxs, const gentity_t *ent ) {
	return syscall( G_ENTITY_CONTACTCAPSULE, mins, maxs, ent );
}

int trap_BotAllocateClient( int clientNum ) {
	return syscall( G_BOT_ALLOCATE_CLIENT, clientNum );
}

int trap_GetSoundLength(sfxHandle_t sfxHandle) {
	return syscall( G_GET_SOUND_LENGTH, sfxHandle );
}

sfxHandle_t	trap_RegisterSound( const char *sample, qboolean compressed ) {
	return syscall( G_REGISTERSOUND, sample, compressed );
}

#ifdef DEBUG
//#define FAKELAG
#ifdef FAKELAG
#define	MAX_USERCMD_BACKUP	256
#define	MAX_USERCMD_MASK	(MAX_USERCMD_BACKUP - 1)

static usercmd_t cmds[MAX_CLIENTS][MAX_USERCMD_BACKUP];
static int cmdNumber[MAX_CLIENTS];
#endif // FAKELAG
#endif // DEBUG

void trap_GetUsercmd( int clientNum, usercmd_t *cmd ) {
	syscall( G_GET_USERCMD, clientNum, cmd );

#ifdef FAKELAG
	{
		char s[MAX_STRING_CHARS];
		int fakeLag;

		trap_Cvar_VariableStringBuffer( "g_fakelag", s, sizeof(s) );
		fakeLag = atoi(s);
		if( fakeLag < 0 )
			fakeLag = 0;

		if( fakeLag ) {
			int i;
			int realcmdtime, thiscmdtime;

			// store our newest usercmd
			cmdNumber[clientNum]++;
			memcpy( &cmds[clientNum][cmdNumber[clientNum] & MAX_USERCMD_MASK], cmd, sizeof(usercmd_t) );

			// find a usercmd that is fakeLag msec behind
			i = cmdNumber[clientNum] & MAX_USERCMD_MASK;
			realcmdtime = cmds[clientNum][i].serverTime;
			i--;
			do {
				thiscmdtime = cmds[clientNum][i & MAX_USERCMD_MASK].serverTime;

				if( realcmdtime - thiscmdtime > fakeLag ) {
					// found the right one
                    cmd = &cmds[clientNum][i & MAX_USERCMD_MASK];
					return;
				}

				i--;
			} while ( (i & MAX_USERCMD_MASK) != (cmdNumber[clientNum] & MAX_USERCMD_MASK) );

			// didn't find a proper one, just use the oldest one we have
			cmd = &cmds[clientNum][(cmdNumber[clientNum] - 1) & MAX_USERCMD_MASK];
			return;
		}
	}
#endif // FAKELAG
}

qboolean trap_GetEntityToken( char *buffer, int bufferSize ) {
	return syscall( G_GET_ENTITY_TOKEN, buffer, bufferSize );
}

int trap_DebugPolygonCreate(int color, int numPoints, vec3_t *points) {
	return syscall( G_DEBUG_POLYGON_CREATE, color, numPoints, points );
}

void trap_DebugPolygonDelete(int id) {
	syscall( G_DEBUG_POLYGON_DELETE, id );
}

int trap_RealTime( qtime_t *qtime ) {
	return syscall( G_REAL_TIME, qtime );
}

void trap_SnapVector( float *v ) {
	syscall( G_SNAPVECTOR, v );
	return;
}

qboolean trap_GetTag( int clientNum, int tagFileNumber, char *tagName, orientation_t *or ) {
	return syscall( G_GETTAG, clientNum, tagFileNumber, tagName, or );
}

qboolean trap_LoadTag( const char* filename ) {
	return syscall( G_REGISTERTAG, filename );
}

int trap_PC_AddGlobalDefine( char *define ) {
	return syscall( BOTLIB_PC_ADD_GLOBAL_DEFINE, define );
}

int trap_PC_LoadSource( const char *filename ) {
	return syscall( BOTLIB_PC_LOAD_SOURCE, filename );
}

int trap_PC_FreeSource( int handle ) {
	return syscall( BOTLIB_PC_FREE_SOURCE, handle );
}

int trap_PC_ReadToken( int handle, pc_token_t *pc_token ) {
	return syscall( BOTLIB_PC_READ_TOKEN, handle, pc_token );
}

int trap_PC_SourceFileAndLine( int handle, char *filename, int *line ) {
	return syscall( BOTLIB_PC_SOURCE_FILE_AND_LINE, handle, filename, line );
}

int trap_PC_UnReadToken( int handle ) {
	return syscall( BOTLIB_PC_UNREAD_TOKEN, handle );
}

int trap_BotGetServerCommand(int clientNum, char *message, int size) {
	return syscall( BOTLIB_GET_CONSOLE_MESSAGE, clientNum, message, size );
}

void trap_BotUserCommand(int clientNum, usercmd_t *ucmd) {
	syscall( BOTLIB_USER_COMMAND, clientNum, ucmd );
}

void trap_EA_Command(int client, char *command) {
	syscall( BOTLIB_EA_COMMAND, client, command );
}

int trap_GeneticParentsAndChildSelection(int numranks, float *ranks, int *parent1, int *parent2, int *child) {
	return syscall( BOTLIB_AI_GENETIC_PARENTS_AND_CHILD_SELECTION, numranks, ranks, parent1, parent2, child );
}

void trap_PbStat ( int clientNum , char *category , char *values ) {
	syscall ( PB_STAT_REPORT , clientNum , category , values ) ;
}

void trap_SendMessage( int clientNum, char *buf, int buflen ) {
	syscall( G_SENDMESSAGE, clientNum, buf, buflen );
}

messageStatus_t trap_MessageStatus( int clientNum ) {
	return syscall( G_MESSAGESTATUS, clientNum );
}
