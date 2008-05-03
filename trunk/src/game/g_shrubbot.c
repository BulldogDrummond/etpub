/*
 * g_shrubbot.c
 *
 * This code is the original work of Tony J. White
 *
 * The functionality of this code mimics the behaviour of the currently
 * inactive project shrubmod (http://www.etstats.com/shrubet/index.php?ver=2)
 * by Ryan Mannion.   However, shrubmod was a closed-source project and 
 * none of it's code has been copied, only it's functionality.
 *
 * You are free to use this implementation of shrubbot in you're 
 * own mod project if you wish.
 *
 */

#include "g_local.h"
#include "../ui/menudef.h"

char *G_Shortcuts(gentity_t *ent, char *text);

extern char bigTextBuffer[100000];

// note: list ordered alphabetically 
static const struct g_shrubbot_cmd g_shrubbot_cmds[] = {
	{"admintest",	G_shrubbot_admintest,	'a', 0,
		"display your current admin level", "(^3name|slot#^7)"},
	{"balance",	G_shrubbot_balance,	'S', 0,
		"run one cycle of ATB to balance teams", ""},
	{"ban",		G_shrubbot_ban,		'b', 0,
		"ban a player by IP and GUID with an optional expiration time "
		"(seconds) and reason", "[^3name|slot#^7] (^htime^7) (^hreason^7)"},
	{"burn",	G_shrubbot_burn,	'U', SCMDF_TYRANNY,
		"burns a player taking some of his health", 
		"[^3name|slot#^7] (^hreason^7)"},
	{"cancelvote",	G_shrubbot_cancelvote,	'c', 0,
		"cancel a vote taking place", ""},
		{"dewarn",	G_shrubbot_dewarn,	'R', 0,
		"remove a warning of a player", "[^3name|slot#^7] (^hwarning#^7)"},
	{"disorient",	G_shrubbot_disorient,	'd', SCMDF_TYRANNY,
		"disorient a player by flipping player's view and controls",
		"[^3name|slot#^7] (^hreason^7)"},
	{"fling",	G_shrubbot_fling,	'l', SCMDF_TYRANNY,
		"fling a player", "[^3name|slot#^7]"},
	{"flinga",	G_shrubbot_fling,	'L', SCMDF_TYRANNY,
		"fling all players", ""},
	{"gib",		G_shrubbot_gib,		'g', SCMDF_TYRANNY,
		"instantly gib a player", "[^3name|slot#^7]"},
	{"help",	G_shrubbot_help,	'h', 0,
		"display commands available to you or help on a specific command",
		"(^hcommand^7)"},
	{"howfair",	G_shrubbot_howfair,	'I', 0,
		"display a message indicating how fair the teams are", ""},
	{"kick",	G_shrubbot_kick,	'k', 0,
		"kick a player with an optional reason", "(^hreason^7)"},
	{"launch",	G_shrubbot_fling,	'l', SCMDF_TYRANNY,
		"launch a player", "[^3name|slot#^7]"},
	{"launcha",	G_shrubbot_fling,	'L', SCMDF_TYRANNY,
		"launch all players", ""},
	{"listplayers",	G_shrubbot_listplayers,	'i', 0,
		"display a list of players, their client numbers and their levels",
		""},
	{"listteams",	G_shrubbot_listteams,	'I', 0,
		"displays info about the teams", ""},
	{"lock",	G_shrubbot_lock,	'K', 0,
		"lock one or all teams from new players joining", 
		"[^3r|b|s|all^7]"},
	{"lol",		G_shrubbot_lol,		'x', SCMDF_TYRANNY,
		"grenades drop from a specified player or all players", 
		"(^hname|slot#^7)"},
	{"mute",	G_shrubbot_mute,	'm', 0,
		"mute a player", "[^3name|slot#^7] (^htime^7) (^hreason^7)"},
	{"news",	G_shrubbot_news,	'W', 0,
		"play the map's news voiceover or, when specified, another map's",
		"(^hmapname^7)"},
	{"nextmap",	G_shrubbot_nextmap,	'n', 0,
		"go to the next map in the cycle", ""},
	{"orient",	G_shrubbot_orient,	'd', SCMDF_TYRANNY,
		"orient a player after a !disorient", "[^3name|slot#^7]"},
	{"passvote",	G_shrubbot_passvote,	'V', 0,
		"pass a vote currently taking place", ""},
	{"pause",	G_shrubbot_pause,	'Z', 0,
		"pauses the game for all players", ""},
	{"pip",		G_shrubbot_pip,		'z', SCMDF_TYRANNY,
		"show sparks around a specified player or all players", 
		"(^hname|slot#^7)"},
	{"pop",		G_shrubbot_pop,		'z', SCMDF_TYRANNY,
		"pops the helmets off from a specified player or all players",
		"(^hname|slot#^7)"},
	{"putteam",	G_shrubbot_putteam,	'p', 0,
		"move a player to a specified team",
		"[^3name|slot#^7] [^3r|b|s^7]"},
	{"readconfig",	G_shrubbot_readconfig,	'G', 0,
		"reloads the shrubbot config file and refreshes user flags", ""},
	{"rename",	G_shrubbot_rename,	'N', SCMDF_TYRANNY,
		"rename a player", "[^3name|slot#^7] [^3new name^7]"},
	{"reset",	G_shrubbot_reset,	'r', 0,
		"reset the match", ""},
	{"resetmyxp",	G_shrubbot_resetmyxp,	'M', 0,
		"reset your own XP to zero", ""},
	{"resetxp",	G_shrubbot_resetxp,	'X', SCMDF_TYRANNY,
		"reset the XP of a specified player to zero", "[^3name|slot#^7]"},
	{"restart",	G_shrubbot_reset,	'r', 0,
		"restart the current map", ""},
	{"setlevel",	G_shrubbot_setlevel,	's', 0,
		"sets the admin level of a player", "[^3name|slot#^7] [^3level^7]"},
	{"showbans",	G_shrubbot_showbans,	'B', 0,
		"display a (partial) list of active bans", "(^hstart at ban#^7)"},
	{"shuffle",	G_shrubbot_shuffle,	'S', 0,
		"shuffle the teams to even them out", ""},
	{"slap",	G_shrubbot_slap,	'A', SCMDF_TYRANNY,
		"give a player a specified amount of damage for a specified reason", 
		"[^3name|slot#^7] (^hdamage^7) (^hreason^7)"},
	{"spec999",	G_shrubbot_spec999,	'P', 0,
		"move 999 pingers to the spectator team", ""},
	{"spreerecord",G_shrubbot_spreerecord, 't', 0,
		"see the spreerecord of this map and the overall spreerecord", ""},
	{"stats",G_shrubbot_stats, 't', 0,
		"see accuracy and headshotratio of all players", ""},
	{"swap",	G_shrubbot_swap,	'w', 0,
		"swap the teams", ""},
	{"throw",	G_shrubbot_fling,	'l', SCMDF_TYRANNY,
		"throw a player", "[^3name|slot#^7]"},
	{"throwa",	G_shrubbot_fling,	'L', SCMDF_TYRANNY,
		"throw all players", ""},
	{"time",	G_shrubbot_time,	'C', 0,
		"show the current local server time", ""},
	{"unban",	G_shrubbot_unban,	'b', 0,
		"unbans a player specified by the slot as seen in !showbans", 
		"[^3ban slot#^7]"},
	{"unlock",	G_shrubbot_unlock,	'K', 0,
		"unlock one or all locked teams", 
		"[^3r|b|s|all^7]"},
	{"unmute",	G_shrubbot_mute,	'm', 0,
		"unmute a muted player", "[^3name|slot#^7]"}, 
	{"unpause",	G_shrubbot_unpause,	'Z', 0,
		"unpauses the game", ""}, 
	{"uptime",	G_shrubbot_uptime,	'u', 0,
		"displays the uptime of the server", ""},
	{"userinfo",	G_shrubbot_userinfo,	'e', 0,
		"displays basic userinfo of a user", ""},
	{"warn",	G_shrubbot_warn,	'R', 0,
		"warns a player by displaying the reason text", 
		"[^3name|slot#^7] [^3reason^7]"},
	{"", NULL, '\0', 0, "", ""}
};

g_shrubbot_level_t *g_shrubbot_levels[MAX_SHRUBBOT_LEVELS];
g_shrubbot_admin_t *g_shrubbot_admins[MAX_SHRUBBOT_ADMINS];
g_shrubbot_ban_t *g_shrubbot_bans[MAX_SHRUBBOT_BANS];
g_shrubbot_command_t *g_shrubbot_commands[MAX_SHRUBBOT_COMMANDS];
g_shrubbot_warning_t *g_shrubbot_warnings[MAX_SHRUBBOT_WARNINGS];

qboolean G_shrubbot_permission(gentity_t *ent, char flag) 
{
	int i;
	int l = 0;
	char *flags;

	// console always wins
	if(!ent)
		return qtrue;
	
	for(i=0; g_shrubbot_admins[i]; i++) {
		if(!Q_stricmp(ent->client->sess.guid,
			g_shrubbot_admins[i]->guid)) {

			flags = g_shrubbot_admins[i]->flags;
			while(*flags) {
				if(*flags == flag)
					return qtrue;
				else if(*flags == '-') {
					while(*flags++) {
						if(*flags == flag) 
							return qfalse;
						else if(*flags == '+')
							break;
					}
				}
				else if(*flags == '*') {
					while(*flags++) {
						if(*flags == flag)
							return qfalse;
					}
					// tjw: flags for individual admins
					switch(flag) {
					case SBF_IMMUTABLE:
					case SBF_INCOGNITO:
						return qfalse;
					default:
						return qtrue;
					}
				}
				*flags++;
			}	
			l = g_shrubbot_admins[i]->level;
		}
	}
	for(i=0; g_shrubbot_levels[i]; i++) {
		if(g_shrubbot_levels[i]->level == l) {
			flags = g_shrubbot_levels[i]->flags;
			while(*flags) {
				if(*flags == flag)
					return qtrue;
				if(*flags == '*') {
					while(*flags++) {
						if(*flags == flag)
							return qfalse;
					}
					// tjw: flags for individual admins
					switch(flag) {
					case SBF_IMMUTABLE:
					case SBF_INCOGNITO:
						return qfalse;
					default:
						return qtrue;
					}
				}
				*flags++;
			}
		}
	}
	return qfalse;
}

/*
 * Returns qtrue if *admin has a higher shrubbot level than *victim
 */
qboolean _shrubbot_admin_higher(gentity_t *admin, gentity_t *victim) 
{
	int i;
	int alevel = 0;

	// console always wins
	if(!admin) return qtrue;
	// just in case
	if(!victim) return qtrue;

	for(i=0; g_shrubbot_admins[i]; i++) {
		if(!Q_stricmp(admin->client->sess.guid,
			g_shrubbot_admins[i]->guid)) {

			alevel = g_shrubbot_admins[i]->level;

			break;
		}
	}
	for(i=0; g_shrubbot_admins[i]; i++) {
		if(!Q_stricmp(victim->client->sess.guid,
			g_shrubbot_admins[i]->guid)) {

			if(alevel < g_shrubbot_admins[i]->level)
				return qfalse;

			break;
		}
	}
	return qtrue;
}

/*
 * Returns qtrue if *victim is immutable, unless either is NULL or
 * *victim == *admin
 */
qboolean _shrubbot_immutable(gentity_t *admin, gentity_t *victim)
{
    if (!admin ) return qfalse;
    if (!victim ) return qfalse;
	
	if ((g_OmniBotFlags.integer & BOT_FLAGS_SHRUBBOT_IMMUTABLE) &&
		(victim->r.svFlags & SVF_BOT))

		return qtrue;

    if (victim == admin ) return qfalse; // if admin wants to hurt himself, let him

    if (G_shrubbot_permission(victim, SBF_IMMUTABLE))
        return qtrue;
    return qfalse;
}

void G_shrubbot_writeconfig_string(char *s, fileHandle_t f) 
{
	char buf[MAX_STRING_CHARS];

	buf[0] = '\0';
	if(s[0]) {
		//Q_strcat(buf, sizeof(buf), s);
		Q_strncpyz(buf, s, sizeof(buf));
		trap_FS_Write(buf, strlen(buf), f);
	}
	trap_FS_Write("\n", 1, f);
}

void G_shrubbot_writeconfig_int(int v, fileHandle_t f) 
{
	char buf[32];

	Com_sprintf(buf, 32, "%d", v);
	//sprintf(buf, "%d", v);
	if(buf[0]) trap_FS_Write(buf, strlen(buf), f);
	trap_FS_Write("\n", 1, f);
}

void G_shrubbot_writeconfig_float(float v, fileHandle_t f) 
{
	char buf[32];

	Com_sprintf(buf, 32, "%f", v);
	//sprintf(buf, "%f", v);
	if(buf[0]) trap_FS_Write(buf, strlen(buf), f);
	trap_FS_Write("\n", 1, f);
}

void _shrubbot_writeconfig() 
{
	fileHandle_t f;
	int len, i, j;
	time_t t;
	char levels[MAX_STRING_CHARS] = {""};

	if(!g_shrubbot.string[0]) return;
	time(&t);
	t = t - SHRUBBOT_BAN_EXPIRE_OFFSET;
	len = trap_FS_FOpenFile(g_shrubbot.string, &f, FS_WRITE);
	if(len < 0) {
		G_Printf("_shrubbot_writeconfig: could not open %s\n",
				g_shrubbot.string);
	}
	for(i=0; g_shrubbot_levels[i]; i++) {
		trap_FS_Write("[level]\n", 8, f);
		trap_FS_Write("level    = ", 11, f);
		G_shrubbot_writeconfig_int(g_shrubbot_levels[i]->level, f);
		trap_FS_Write("name     = ", 11, f);
		G_shrubbot_writeconfig_string(g_shrubbot_levels[i]->name, f);
		trap_FS_Write("flags    = ", 11, f);
		G_shrubbot_writeconfig_string(g_shrubbot_levels[i]->flags, f);
		trap_FS_Write("greeting = ", 11, f);
		G_shrubbot_writeconfig_string(g_shrubbot_levels[i]->greeting, f);
		trap_FS_Write("\n", 1, f);
	}
	for(i=0; g_shrubbot_admins[i]; i++) {
		// don't write level 0 users
		if(g_shrubbot_admins[i]->level < 1) continue;

		trap_FS_Write("[admin]\n", 8, f);
		trap_FS_Write("name     = ", 11, f);
		G_shrubbot_writeconfig_string(g_shrubbot_admins[i]->name, f);
		trap_FS_Write("guid     = ", 11, f);
		G_shrubbot_writeconfig_string(g_shrubbot_admins[i]->guid, f);
		trap_FS_Write("level    = ", 11, f);
		G_shrubbot_writeconfig_int(g_shrubbot_admins[i]->level, f);
		trap_FS_Write("flags    = ", 11, f);
		G_shrubbot_writeconfig_string(g_shrubbot_admins[i]->flags, f);
		trap_FS_Write("greeting = ", 11, f);
		G_shrubbot_writeconfig_string(g_shrubbot_admins[i]->greeting, f);
		trap_FS_Write("\n", 1, f);
	}
	for(i=0; g_shrubbot_bans[i]; i++) {
		// don't write expired bans
		// if expires is 0, then it's a perm ban
		if(g_shrubbot_bans[i]->expires != 0 &&
			(g_shrubbot_bans[i]->expires - t) < 1) continue;

		trap_FS_Write("[ban]\n", 6, f);
		trap_FS_Write("name     = ", 11, f);
		G_shrubbot_writeconfig_string(g_shrubbot_bans[i]->name, f);
		trap_FS_Write("guid     = ", 11, f);
		G_shrubbot_writeconfig_string(g_shrubbot_bans[i]->guid, f);
		trap_FS_Write("ip       = ", 11, f);
		G_shrubbot_writeconfig_string(g_shrubbot_bans[i]->ip, f);
		trap_FS_Write("reason   = ", 11, f);
		G_shrubbot_writeconfig_string(g_shrubbot_bans[i]->reason, f);
		trap_FS_Write("made     = ", 11, f);
		G_shrubbot_writeconfig_string(g_shrubbot_bans[i]->made, f);
		trap_FS_Write("expires  = ", 11, f);
		G_shrubbot_writeconfig_int(g_shrubbot_bans[i]->expires, f);
		trap_FS_Write("banner   = ", 11, f);
		G_shrubbot_writeconfig_string(g_shrubbot_bans[i]->banner, f);
		trap_FS_Write("\n", 1, f);
	}
	for(i=0; g_shrubbot_commands[i]; i++) {
		levels[0] = '\0';
		trap_FS_Write("[command]\n", 10, f);
		trap_FS_Write("command  = ", 11, f);
		G_shrubbot_writeconfig_string(g_shrubbot_commands[i]->command, f);
		trap_FS_Write("exec     = ", 11, f);
		G_shrubbot_writeconfig_string(g_shrubbot_commands[i]->exec, f);
		trap_FS_Write("desc     = ", 11, f);
		G_shrubbot_writeconfig_string(g_shrubbot_commands[i]->desc, f);
		trap_FS_Write("levels   = ", 11, f);
		for(j=0; g_shrubbot_commands[i]->levels[j] != -1; j++) {
			Q_strcat(levels, sizeof(levels),
				va("%i ", g_shrubbot_commands[i]->levels[j]));
		}
		G_shrubbot_writeconfig_string(levels, f);
		trap_FS_Write("\n", 1, f);
	}
	for(i=0; g_shrubbot_warnings[i]; i++) {
		// don't write warnings older than g_warningDecay hours
		if((t - g_shrubbot_warnings[i]->made) >=
			3600*g_warningDecay.integer ) continue;

		trap_FS_Write("[warning]\n", 10, f);
		trap_FS_Write("name     = ", 11, f);
		G_shrubbot_writeconfig_string(g_shrubbot_warnings[i]->name, f);
		trap_FS_Write("guid     = ", 11, f);
		G_shrubbot_writeconfig_string(g_shrubbot_warnings[i]->guid, f);
		trap_FS_Write("ip       = ", 11, f);
		G_shrubbot_writeconfig_string(g_shrubbot_warnings[i]->ip, f);
		trap_FS_Write("warning  = ", 11, f);
		G_shrubbot_writeconfig_string(g_shrubbot_warnings[i]->warning, f);
		trap_FS_Write("made     = ", 11, f);
		G_shrubbot_writeconfig_int(g_shrubbot_warnings[i]->made, f);
		trap_FS_Write("warner   = ", 11, f);
		G_shrubbot_writeconfig_string(g_shrubbot_warnings[i]->warner, f);
		trap_FS_Write("\n", 1, f);
	}
	trap_FS_FCloseFile(f);
}

void G_shrubbot_readconfig_string(char **cnf, char *s, int size) 
{
	char *t;

	//COM_MatchToken(cnf, "=");
	t = COM_ParseExt(cnf, qfalse);
	if(!strcmp(t, "=")) {
		t = COM_ParseExt(cnf, qfalse);
	}
	else {
		G_Printf("readconfig: warning missing = before "
			"\"%s\" on line %d\n", 
			t, 
			COM_GetCurrentParseLine());
	}
	s[0] = '\0';
	while(t[0]) {
		if((s[0] == '\0' && strlen(t) <= size) ||
			(strlen(t)+strlen(s) < size)) {

			Q_strcat(s, size, t);
			Q_strcat(s, size, " ");
		}
		t = COM_ParseExt(cnf, qfalse);
	}
	// trim the trailing space
	if(strlen(s) > 0 && s[strlen(s)-1] == ' ') 
		s[strlen(s)-1] = '\0';
}

void G_shrubbot_readconfig_int(char **cnf, int *v) 
{
	char *t;

	//COM_MatchToken(cnf, "=");
	t = COM_ParseExt(cnf, qfalse);
	if(!strcmp(t, "=")) {
		t = COM_ParseExt(cnf, qfalse);
	}
	else {
		G_Printf("readconfig: warning missing = before "
			"\"%s\" on line %d\n", 
			t, 
			COM_GetCurrentParseLine());
	}
	*v = atoi(t);
}

void G_shrubbot_readconfig_float(char **cnf, float *v) 
{
	char *t;

	//COM_MatchToken(cnf, "=");
	t = COM_ParseExt(cnf, qfalse);
	if(!strcmp(t, "=")) {
		t = COM_ParseExt(cnf, qfalse);
	}
	else {
		G_Printf("readconfig: warning missing = before "
			"\"%s\" on line %d\n", 
			t, 
			COM_GetCurrentParseLine());
	}
	*v = atof(t);
}


/*
  if we can't parse any levels from readconfig, set up default
  ones to make new installs easier for admins
*/
void _shrubbot_default_levels() {
	g_shrubbot_level_t *l;
	int i;

	for(i=0; g_shrubbot_levels[i]; i++) {
		free(g_shrubbot_levels[i]);
		g_shrubbot_levels[i] = NULL;
	}
	for(i=0; i<=5; i++) {
		l = malloc(sizeof(g_shrubbot_level_t));
		l->level = i;
		*l->name = '\0';
		*l->flags = '\0';
		*l->greeting = '\0';
		g_shrubbot_levels[i] = l;
	}
	Q_strcat(g_shrubbot_levels[0]->flags, 6, "iahCp");
	Q_strcat(g_shrubbot_levels[1]->flags, 6, "iahCp");
	Q_strcat(g_shrubbot_levels[2]->flags, 7, "iahCpP");
	Q_strcat(g_shrubbot_levels[3]->flags, 10, "i1ahCpPkm");
	Q_strcat(g_shrubbot_levels[4]->flags, 12, "i1ahCpPkmBb");
	Q_strcat(g_shrubbot_levels[5]->flags, 13, "i1ahCpPkmBbs");
}

/*
	return a level for a player entity.
*/
int _shrubbot_level(gentity_t *ent) 
{
	int i;
	qboolean found = qfalse;

	if(!ent) {	
		// forty - we are on the console, return something high for now.
		return MAX_SHRUBBOT_LEVELS;
	}

	for(i=0; g_shrubbot_admins[i]; i++) {
		if(!Q_stricmp(g_shrubbot_admins[i]->guid,
			ent->client->sess.guid)) {

			found = qtrue;
			break;	
		}
	}

	if(found) {
		return g_shrubbot_admins[i]->level;
	}

	return 0;
}

static qboolean _shrubbot_command_permission(gentity_t *ent, char *command)
{
	int i, j;
	int level = _shrubbot_level(ent);
	
	if(!ent)
		return qtrue;
	for(i = 0; g_shrubbot_commands[i]; i++) {
		if(!Q_stricmp(command, g_shrubbot_commands[i]->command)) {
			for(j = 0;
				g_shrubbot_commands[i]->levels[j] != -1;
				j++) {

				if(g_shrubbot_commands[i]->levels[j] ==
					level) {

					return qtrue;
				}
			}
		}
	}
	return qfalse;
}

void _shrubbot_log(gentity_t *admin, char *cmd, int skiparg)
{
	fileHandle_t f;
	int len, i, j;
	char string[MAX_STRING_CHARS];
	int	min, tens, sec;
	g_shrubbot_admin_t *a;
	g_shrubbot_level_t *l;
	char flags[MAX_SHRUBBOT_FLAGS*2];
	gentity_t *victim = NULL;
	int pids[MAX_CLIENTS];
	char name[MAX_NAME_LENGTH];

	if(!g_logAdmin.string[0]) return;


	len = trap_FS_FOpenFile(g_logAdmin.string, &f, FS_APPEND);
	if(len < 0) {
		G_Printf("_shrubbot_log: error could not open %s\n",
			g_logAdmin.string);
		return;
	}

	sec = level.time / 1000;
	min = sec / 60;
	sec -= min * 60;
	tens = sec / 10;
	sec -= tens * 10;

	*flags = '\0';
	if(admin) {
		for(i=0; g_shrubbot_admins[i]; i++) {
			if(!Q_stricmp(g_shrubbot_admins[i]->guid , 
				admin->client->sess.guid)) {

				a = g_shrubbot_admins[i];
				Q_strncpyz(flags, a->flags, sizeof(flags));
				for(j=0; g_shrubbot_levels[j]; j++) {
					if(g_shrubbot_levels[j]->level == a->level) {
						l = g_shrubbot_levels[j];
						Q_strcat(flags, sizeof(flags), l->flags);
						break;
					}
				}
				break;
			}
		}
	}

	if(Q_SayArgc() > 1+skiparg) {
		Q_SayArgv(1+skiparg, name, sizeof(name));
		if(ClientNumbersFromString(name, pids) == 1) {
			victim = &g_entities[pids[0]];
		}
	}

	if(victim && Q_stricmp(cmd, "attempted")) {
		Com_sprintf(string, sizeof(string), 
			"%3i:%i%i: %i: %s: %s: %s: %s: %s: %s: \"%s\"\n", 
			min,
			tens,
			sec,
			(admin) ? admin->s.clientNum : -1,
			(admin) ? admin->client->sess.guid 
				: "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
			(admin) ? admin->client->pers.netname : "console",
			flags,
			cmd,
			victim->client->sess.guid,
			victim->client->pers.netname,
			Q_SayConcatArgs(2+skiparg));
	}
	else {
		Com_sprintf(string, sizeof(string), 
			"%3i:%i%i: %i: %s: %s: %s: %s: \"%s\"\n", 
			min,
			tens,
			sec,
			(admin) ? admin->s.clientNum : -1,
			(admin) ? admin->client->sess.guid
				: "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
			(admin) ? admin->client->pers.netname : "console",
			flags,
			cmd,
			Q_SayConcatArgs(1+skiparg));
	}
	trap_FS_Write(string, strlen(string), f);
	trap_FS_FCloseFile(f);
}

void G_shrubbot_duration(int secs, char *duration, int dursize) 
{

	if(secs > (60*60*24*365*50) || secs < 0) {
		Q_strncpyz(duration, "PERMANENT", dursize);
	}
	else if(secs > (60*60*24*365*2)) {
		Com_sprintf(duration, dursize, "%d years",
			(secs / (60*60*24*365)));
	}
	else if(secs > (60*60*24*365)) {
		Q_strncpyz(duration, "1 year", dursize);
	}
	else if(secs > (60*60*24*30*2)) {
		Com_sprintf(duration, dursize, "%i months",
			(secs / (60*60*24*30)));
	}
	else if(secs > (60*60*24*30)) {
		Q_strncpyz(duration, "1 month", dursize);
	}
	else if(secs > (60*60*24*2)) {
		Com_sprintf(duration, dursize, "%i days",
			(secs / (60*60*24)));
	}
	else if(secs > (60*60*24)) {
		Q_strncpyz(duration, "1 day", dursize);
	}
	else if(secs > (60*60*2)) {
		Com_sprintf(duration, dursize, "%i hours",
			(secs / (60*60)));
	}
	else if(secs > (60*60)) {
		Q_strncpyz(duration, "1 hour", dursize);
	}
	else if(secs > (60*2)) {
		Com_sprintf(duration, dursize, "%i mins",
			(secs / 60));
	}
	else if(secs > 60) {
		Q_strncpyz(duration, "1 minute", dursize);
	}
	else {
		Com_sprintf(duration, dursize, "%i secs", secs);
	}
}

qboolean G_shrubbot_ban_check(char *userinfo, char *reason) 
{
	char *guid, *ip;
	int i;
	time_t t;
	int seconds = 0; // Dens: Perm is default

	if(!time(&t)) return qfalse;
	t = t - SHRUBBOT_BAN_EXPIRE_OFFSET;
	if(!*userinfo) return qfalse;
	ip = Info_ValueForKey(userinfo, "ip");
	if(!*ip) return qfalse;
	guid = Info_ValueForKey(userinfo, "cl_guid");
	if(!*guid) return qfalse;
	for(i=0; g_shrubbot_bans[i]; i++) {
		// 0 is for perm ban
		if(g_shrubbot_bans[i]->expires != 0 &&
			(g_shrubbot_bans[i]->expires - t) < 1)
			continue;
		// Dens: lets find out seconds now, we need that later	
		if(g_shrubbot_bans[i]->expires != 0){
			seconds = g_shrubbot_bans[i]->expires - t;
		}
		if(strstr(ip, g_shrubbot_bans[i]->ip)) {
			// Dens: check if there is a reason, than check if the ban expires
			if(*g_shrubbot_bans[i]->reason) {
				if(seconds == 0){
					Com_sprintf(
						reason, 
						MAX_STRING_CHARS, 
						"Reason: %s\nExpires: NEVER.\n", 
						g_shrubbot_bans[i]->reason
					);
				}else{
					Com_sprintf(
					reason, 
					MAX_STRING_CHARS, 
					"Reason: %s\nExpires in: %i seconds.\n", 
					g_shrubbot_bans[i]->reason, 
					seconds
					);
				}
			}else{
				if(seconds == 0){
					Com_sprintf(
						reason, 
						MAX_STRING_CHARS, 
						"Expires: NEVER.\n"
					);
				}else{
					Com_sprintf(
					reason, 
					MAX_STRING_CHARS,
					"Expires in: %i seconds.\n", 
					seconds
					);
				}
			}
			return qtrue;
		}
		if(!Q_stricmp(g_shrubbot_bans[i]->guid, guid)) {
			// Dens: check if there is a reason, than check if the ban expires
			if(*g_shrubbot_bans[i]->reason) {
				if(seconds == 0){
					Com_sprintf(
						reason, 
						MAX_STRING_CHARS, 
						"Reason: %s\nExpires: NEVER.\n", 
						g_shrubbot_bans[i]->reason
					);
				}else{
					Com_sprintf(
					reason, 
					MAX_STRING_CHARS, 
					"Reason: %s\nExpires in: %i seconds.\n", 
					g_shrubbot_bans[i]->reason, 
					seconds
					);
				}
			}else{
				if(seconds == 0){
					Com_sprintf(
						reason, 
						MAX_STRING_CHARS, 
						"Expires: NEVER.\n"
					);
				}else{
					Com_sprintf(
					reason, 
					MAX_STRING_CHARS,
					"Expires in: %i seconds.\n", 
					seconds
					);
				}
			}
			return qtrue;
		}
	}
	return qfalse;
}

qboolean G_shrubbot_levelconnect_check(char *userinfo, char *reason) 
{
	char *guid;
	int i;

	if(*userinfo){
		guid = Info_ValueForKey(userinfo, "cl_guid");
			
		if(*guid){
			for(i=0; g_shrubbot_admins[i]; i++) {
				if(!Q_stricmp(g_shrubbot_admins[i]->guid, guid)) {
					if(g_shrubbot_admins[i]->level >= g_minConnectLevel.integer){
						return qfalse;
					}else{
						break;
					}
				}
			}
		}
	}

	Com_sprintf(
		reason, 
		MAX_STRING_CHARS,
		"This server is closed for users that don't have adminlevel %i or higher.\n", 
		g_minConnectLevel.integer
		);
	return qtrue;
}

void G_shrubbot_greeting(gentity_t *ent)
{
	int i, l = 0;
	char *greeting;

	if(!ent || !ent->client){
		return;
	}

	// Dens: someone who is hidden shouln't be announced
	if(G_shrubbot_permission(ent,SBF_INCOGNITO)){
		return;
	}

	for(i=0; g_shrubbot_admins[i]; i++){
		if(!Q_stricmp(ent->client->sess.guid,
			g_shrubbot_admins[i]->guid)){

			if(*g_shrubbot_admins[i]->greeting){

				greeting = Q_StrReplace(g_shrubbot_admins[i]->greeting,
					"[n]", ent->client->pers.netname);

				switch(g_greetingPos.integer) {
					case MSGPOS_CENTER:
						AP(va("cp \"%s\"",greeting));
						break;
					case MSGPOS_LEFT_BANNER:
						AP(va("cpm \"%s\"",greeting));
						break;
					case MSGPOS_BANNER:
						AP(va("bp \"%s\"",greeting));
						break;
					case MSGPOS_CONSOLE:
						G_Printf( "%s\n",greeting);
						break;
					case MSGPOS_CHAT:
						default:
						AP(va("chat \"%s\" -1",greeting));
				}
			}else{
				l = g_shrubbot_admins[i]->level;
				for(i=0; g_shrubbot_levels[i]; i++) {
					if(g_shrubbot_levels[i]->level == l) {
						if(*g_shrubbot_levels[i]->greeting){

							greeting = Q_StrReplace(g_shrubbot_levels[i]->greeting,
								"[n]", ent->client->pers.netname);

							switch(g_greetingPos.integer) {
								case MSGPOS_CENTER:
									AP(va("cp \"%s\"",greeting));
									break;
								case MSGPOS_LEFT_BANNER:
									AP(va("cpm \"%s\"",greeting));
									break;
								case MSGPOS_BANNER:
									AP(va("bp \"%s\"",greeting));
									break;
								case MSGPOS_CONSOLE:
									G_Printf( "%s\n",greeting);
									break;
								case MSGPOS_CHAT:
									default:
									AP(va("chat \"%s\" -1",greeting));
							}
						}
						break;
					}
				}
			}
			return;
		}
	}
	return;
}

qboolean G_shrubbot_cmd_check(gentity_t *ent)
{
	int i;
	char command[MAX_SHRUBBOT_CMD_LEN];
	char *cmd;
	char *string;
	int skip = 0;
	qboolean millisecs;

	if(g_minCommandWaitTime.integer == 1000){
		string = "second\0";
		millisecs = qfalse;
	}else if(g_minCommandWaitTime.integer == 1) {
		string = "millisecond\0";
		millisecs = qtrue;
	}else if(g_minCommandWaitTime.integer < 2000) {
		string = "milliseconds\0";
		millisecs = qtrue;
	} else {
		string = "seconds\0";
		millisecs = qfalse;
	}

	if(g_shrubbot.string[0] == '\0') 
		return qfalse;

	command[0] = '\0';
	Q_SayArgv(0, command, sizeof(command));
	if(!Q_stricmp(command, "say") || 
		(G_shrubbot_permission(ent, SBF_TEAMFTSHRUBCMD) && 
			(!Q_stricmp(command, "say_team") || 
			!Q_stricmp(command, "say_buddy")))) {
				skip = 1;
				Q_SayArgv(1, command, sizeof(command));
	}
	if(!command[0]) 
		return qfalse;

	if(command[0] == '!') {
		cmd = &command[1];
	}
	else if(ent == NULL) {
		cmd = &command[0];
	}
	else {
		return qfalse;
	}

	for(i=0; g_shrubbot_commands[i]; i++) {
		if(Q_stricmp(cmd, g_shrubbot_commands[i]->command))
			continue;

		if(_shrubbot_command_permission(ent, cmd)) {
			char *cmdLine;
			int argIx;

			// Replace shortcuts
			cmdLine = G_Shortcuts(ent, g_shrubbot_commands[i]->exec);

			// Replace arguments
			for (argIx = 1; argIx <= 9; argIx++) {
				char arg[MAX_NAME_LENGTH];
				Q_SayArgv(skip + argIx, arg, sizeof(arg));
				cmdLine = Q_StrReplace(cmdLine, va("[%i]", argIx), arg);
			} 

			if( ent && level.time - ent->client->pers.lastCommandTime < g_minCommandWaitTime.integer ) {
				SPC(va("^/%s: ^7you have to wait %d %s between using commands",
					g_shrubbot_commands[i]->command,
					millisecs ? g_minCommandWaitTime.integer : g_minCommandWaitTime.integer / 1000,
					string));
				_shrubbot_log(ent, "attempted", skip-1);
				return qfalse;
			} else {
				trap_SendConsoleCommand(EXEC_APPEND, cmdLine);
				if( ent ) 
					ent->client->pers.lastCommandTime = level.time;
				_shrubbot_log(ent, cmd, skip);
			}
			return qtrue;
		}
		else {
			SPC(va("^/%s: ^7permission denied", 
				g_shrubbot_commands[i]->command));
			_shrubbot_log(ent, "attempted", skip-1);
			return qfalse;
		}
	}

	for (i=0; g_shrubbot_cmds[i].keyword[0]; i++) {
		if(Q_stricmp(cmd, g_shrubbot_cmds[i].keyword)) 
			continue;
		if((g_shrubbot_cmds[i].cmdFlags & SCMDF_TYRANNY) &&
			!g_tyranny.integer) {

			SPC(va("^/%s: ^7g_tyranny is not enabled",
				g_shrubbot_cmds[i].keyword));
			_shrubbot_log(ent, "attempted", skip-1);
		}
		else if(G_shrubbot_permission(ent, g_shrubbot_cmds[i].flag)) {
			if( ent && level.time - ent->client->pers.lastCommandTime < g_minCommandWaitTime.integer ) {
				SPC(va("^/%s: ^7you have to wait %d %s between using commands",
					g_shrubbot_cmds[i].keyword,
					millisecs ? g_minCommandWaitTime.integer : g_minCommandWaitTime.integer / 1000,
					string));
				_shrubbot_log(ent, "attempted", skip-1);
				return qfalse;
			} else {
				g_shrubbot_cmds[i].handler(ent, skip);
				_shrubbot_log(ent, cmd, skip);
				if( ent )
					ent->client->pers.lastCommandTime = level.time;
			}
			return qtrue; 
		}
		else {
			SPC(va("^/%s: ^7permission denied",
				g_shrubbot_cmds[i].keyword));
			_shrubbot_log(ent, "attempted", skip-1);
		}
	}
	return qfalse;
}

int G_shrubbot_user_tot_warnings(gentity_t *ent)
{
	int i,count = 0;
	time_t t;

	if(!time(&t)){
		return -1;
	}
	
	if(!ent || !ent->client){
		return -1;
	}

	for(i=0;g_shrubbot_warnings[i];i++){
		if((t- SHRUBBOT_BAN_EXPIRE_OFFSET - g_shrubbot_warnings[i]->made) >
			3600*g_warningDecay.integer ){
				continue;
		}
		if((g_warningOptions.integer & WARNOP_LINK_GUID) && 
			!Q_stricmp(ent->client->sess.guid,g_shrubbot_warnings[i]->guid)){
			count++;
			continue;
		}
		if(!Q_stricmp(ent->client->sess.ip,g_shrubbot_warnings[i]->ip)
			&& (g_warningOptions.integer & WARNOP_LINK_IP)){
			count++;
		}
	}
	return count;
}

int G_shrubbot_user_warning(gentity_t *ent, int number)
{
	int i,count = 0;
	time_t t;

	if(!time(&t)){
		return -1;
	}
	
	if(!ent || !ent->client){
		return -1;
	}

	if(number > G_shrubbot_user_tot_warnings(ent) || number < 1){
		return -1;
	}

	for(i=0;g_shrubbot_warnings[i];i++){
		if((t - SHRUBBOT_BAN_EXPIRE_OFFSET - g_shrubbot_warnings[i]->made) >
			3600*g_warningDecay.integer ){
				continue;
		}
		if((g_warningOptions.integer & WARNOP_LINK_GUID) && 
			!Q_stricmp(ent->client->sess.guid,g_shrubbot_warnings[i]->guid)){
			count++;
			if(count == number){
				break;
			}
			continue;
		}
		if(!Q_stricmp(ent->client->sess.ip,g_shrubbot_warnings[i]->ip)
			&& (g_warningOptions.integer & WARNOP_LINK_IP)){
			count++;
			if(count == number){
				break;
			}
		}
	}

	if(i == MAX_SHRUBBOT_WARNINGS){
		return -1;
	}

	return i;
}

// Dens: show a list of your own warnings
void Cmd_Warning_f( gentity_t *ent )
{
	int i,count,structure = -1;

	count = G_shrubbot_user_tot_warnings(ent);

	if(count < 0){
		CP("print \"^/An error occurred, your warnings cannot be displayed\n\"");
		return;
	}else if(count == 0){
		CP("print \"^/0 warnings found, try to keep it that way\n\"");
		return;
	}else if(count == 1){
		CP("print \"^/1 warning found:\n\"");
	}else{
		CP(va("print \"^/%d warnings found:\n\"",count));
	}

	SBP_begin();
	for(i=1; i<=count;i++){
		structure = G_shrubbot_user_warning(ent,i);
		if(structure != -1){
			SBP(va("^/%i.^7 %s\n",i,g_shrubbot_warnings[structure]->warning));
		}
	}
	SBP_end();
	return;
}

qboolean G_shrubbot_readconfig(gentity_t *ent, int skiparg) 
{
	g_shrubbot_level_t *l = NULL;
	g_shrubbot_admin_t *a = NULL;
	g_shrubbot_ban_t *b = NULL;
	g_shrubbot_command_t *c = NULL;
	g_shrubbot_warning_t *w = NULL;
	int lc = 0, ac = 0, bc = 0, cc = 0, wc = 0;
	fileHandle_t f;
	int len;
	char *cnf, *cnf2;
	char *t;
	qboolean level_open, admin_open, ban_open, command_open, warning_open;
	char levels[MAX_STRING_CHARS] = {""};

	if(!g_shrubbot.string[0]) return qfalse;
	len = trap_FS_FOpenFile(g_shrubbot.string, &f, FS_READ) ; 
	if(len < 0) {
		SPC(va("^/readconfig: ^7could not open shrubbot config file %s",
			g_shrubbot.string));
		_shrubbot_default_levels();
		return qfalse;
	}
	cnf = malloc(len+1);
	cnf2 = cnf;
	trap_FS_Read(cnf, len, f);
	*(cnf + len) = '\0';
	trap_FS_FCloseFile(f);

	G_shrubbot_cleanup();	
	
	t = COM_Parse(&cnf);
	level_open = admin_open = ban_open = command_open = warning_open = qfalse;
	while(*t) {
		if(!Q_stricmp(t, "[level]") || 
			!Q_stricmp(t, "[admin]") ||
			!Q_stricmp(t, "[ban]") ||
			!Q_stricmp(t, "[command]") ||
			!Q_stricmp(t, "[warning]")) {
			
			if(level_open) g_shrubbot_levels[lc++] = l;
			else if(admin_open) g_shrubbot_admins[ac++] = a;
			else if(ban_open) g_shrubbot_bans[bc++] = b;
			else if(command_open) g_shrubbot_commands[cc++] = c;
			else if(warning_open) g_shrubbot_warnings[wc++] = w;
			level_open = admin_open = ban_open = 
				command_open = warning_open = qfalse;
		}

		if(level_open) {
			if(!Q_stricmp(t, "level")) {
				G_shrubbot_readconfig_int(&cnf, &l->level);
			}
			else if(!Q_stricmp(t, "name")) {
				G_shrubbot_readconfig_string(&cnf, 
					l->name, sizeof(l->name)); 
			}
			else if(!Q_stricmp(t, "flags")) {
				G_shrubbot_readconfig_string(&cnf, 
					l->flags, sizeof(l->flags)); 
			}
			else if(!Q_stricmp(t, "greeting")) {
				G_shrubbot_readconfig_string(&cnf, 
					l->greeting, sizeof(l->greeting)); 
			}
			else {
				SPC(va("^/readconfig: ^7[level] parse error near "
					"%s on line %d", 
					t, 
					COM_GetCurrentParseLine()));
			}
		}
		else if(admin_open) {
			if(!Q_stricmp(t, "name")) {
				G_shrubbot_readconfig_string(&cnf, 
					a->name, sizeof(a->name)); 
			}
			else if(!Q_stricmp(t, "guid")) {
				G_shrubbot_readconfig_string(&cnf, 
					a->guid, sizeof(a->guid)); 
			}
			else if(!Q_stricmp(t, "level")) {
				G_shrubbot_readconfig_int(&cnf, &a->level); 
			}
			else if(!Q_stricmp(t, "flags")) {
				G_shrubbot_readconfig_string(&cnf, 
					a->flags, sizeof(a->flags)); 
			}
			else if(!Q_stricmp(t, "greeting")) {
				G_shrubbot_readconfig_string(&cnf, 
					a->greeting, sizeof(a->greeting)); 
			}
			else {
				SPC(va("^/readconfig: ^7[admin] parse error near "
					"%s on line %d", 
					t, 
					COM_GetCurrentParseLine()));
			}

		}
		else if(ban_open) {
			if(!Q_stricmp(t, "name")) {
				G_shrubbot_readconfig_string(&cnf, 
					b->name, sizeof(b->name)); 
			}
			else if(!Q_stricmp(t, "guid")) {
				G_shrubbot_readconfig_string(&cnf, 
					b->guid, sizeof(b->guid)); 
			}
			else if(!Q_stricmp(t, "ip")) {
				G_shrubbot_readconfig_string(&cnf, 
					b->ip, sizeof(b->ip)); 
			}
			else if(!Q_stricmp(t, "reason")) {
				G_shrubbot_readconfig_string(&cnf, 
					b->reason, sizeof(b->reason)); 
			}
			else if(!Q_stricmp(t, "made")) {
				G_shrubbot_readconfig_string(&cnf, 
					b->made, sizeof(b->made)); 
			}
			else if(!Q_stricmp(t, "expires")) {
				G_shrubbot_readconfig_int(&cnf, &b->expires);
			}
			else if(!Q_stricmp(t, "banner")) {
				G_shrubbot_readconfig_string(&cnf, 
					b->banner, sizeof(b->banner)); 
			}
			else {
				SPC(va("^/readconfig: ^7[ban] parse error near "
					"%s on line %d",
					t, 
					COM_GetCurrentParseLine()));
			}
		}
		else if(command_open) {
			if(!Q_stricmp(t, "command")) {
				G_shrubbot_readconfig_string(&cnf, 
					c->command, sizeof(c->command)); 
			}
			else if(!Q_stricmp(t, "exec")) {
				G_shrubbot_readconfig_string(&cnf, 
					c->exec, sizeof(c->exec)); 
			}
			else if(!Q_stricmp(t, "desc")) {
				G_shrubbot_readconfig_string(&cnf, 
					c->desc, sizeof(c->desc)); 
			}
			else if(!Q_stricmp(t, "levels")) {
				char level[6] = {""};
				char *lp = levels;
				int cmdlevel = 0;

				G_shrubbot_readconfig_string(&cnf, 
					levels, sizeof(levels));
				while(*lp) {
					if(*lp == ' ') {
						c->levels[cmdlevel++] = 
							atoi(level);
						level[0] = '\0';
						*lp++;
						continue;
					}
					Q_strcat(level, sizeof(level),
						va("%c", *lp));
					*lp++;
				}
				if(level[0])
					c->levels[cmdlevel++] = atoi(level);
				// tjw: ensure the list is -1 terminated
				c->levels[MAX_SHRUBBOT_LEVELS] = -1;
				// Dens: probably better if this is done earlier
				if(cmdlevel < MAX_SHRUBBOT_LEVELS)
					c->levels[cmdlevel] = -1;
			}
			else {
				SPC(va("^/readconfig: ^7[command] parse error near "
					"%s on line %d", 
					t, 
					COM_GetCurrentParseLine()));
			}
		}
		else if(warning_open) {
			if(!Q_stricmp(t, "name")) {
				G_shrubbot_readconfig_string(&cnf, 
					w->name, sizeof(w->name)); 
			}
			else if(!Q_stricmp(t, "guid")) {
				G_shrubbot_readconfig_string(&cnf, 
					w->guid, sizeof(w->guid)); 
			}
			else if(!Q_stricmp(t, "ip")) {
				G_shrubbot_readconfig_string(&cnf, 
					w->ip, sizeof(w->ip)); 
			}
			else if(!Q_stricmp(t, "warning")) {
				G_shrubbot_readconfig_string(&cnf, 
					w->warning, sizeof(w->warning)); 
			}
			else if(!Q_stricmp(t, "made")) {
				G_shrubbot_readconfig_int(&cnf, &w->made);
			}
			else if(!Q_stricmp(t, "warner")) {
				G_shrubbot_readconfig_string(&cnf, 
					w->warner, sizeof(w->warner)); 
			}
			else {
				SPC(va("^/readconfig: ^7[warning] parse error near "
					"%s on line %d",
					t, 
					COM_GetCurrentParseLine()));
			}
		}

		if(!Q_stricmp(t, "[level]")) {
			if(lc >= MAX_SHRUBBOT_LEVELS) return qfalse;
			l = malloc(sizeof(g_shrubbot_level_t));
			l->level = 0;
			*l->name = '\0';
			*l->flags = '\0';
			*l->greeting = '\0';
			level_open = qtrue;
		}
		else if(!Q_stricmp(t, "[admin]")) {
			if(ac >= MAX_SHRUBBOT_ADMINS) return qfalse;
			a = malloc(sizeof(g_shrubbot_admin_t));
			*a->name = '\0';
			*a->guid = '\0';
			a->level = 0;
			*a->flags = '\0';
			*a->greeting = '\0';
			admin_open = qtrue;
		}
		else if(!Q_stricmp(t, "[ban]")) {
			if(bc >= MAX_SHRUBBOT_BANS) return qfalse;
			b = malloc(sizeof(g_shrubbot_ban_t));
			*b->name = '\0';
			*b->guid = '\0';
			*b->ip = '\0';
			*b->made = '\0';
			b->expires = 0;
			*b->reason = '\0';
			*b->banner = '\0';
			ban_open = qtrue;
		}
		else if(!Q_stricmp(t, "[command]")) {
			if(cc >= MAX_SHRUBBOT_COMMANDS)
				return qfalse;
			c = malloc(sizeof(g_shrubbot_command_t));
			*c->command = '\0';
			*c->exec = '\0';
			*c->desc = '\0';
			memset(c->levels, -1, sizeof(c->levels));
			command_open = qtrue;
		}
		else if(!Q_stricmp(t, "[warning]")) {
			if(wc >= MAX_SHRUBBOT_WARNINGS)
				return qfalse;
			w = malloc(sizeof(g_shrubbot_warning_t));
			*w->name = '\0';
			*w->guid = '\0';
			*w->ip = '\0';
			w->made = 0;
			*w->warning = '\0';
			*w->warner = '\0';
			warning_open = qtrue;
		}
		t = COM_Parse(&cnf);
	}
	if(level_open) g_shrubbot_levels[lc++] = l;
	if(admin_open) g_shrubbot_admins[ac++] = a;
	if(ban_open) g_shrubbot_bans[bc++] = b;
	if(command_open) g_shrubbot_commands[cc++] = c;
	if(warning_open) g_shrubbot_warnings[wc++] = w;
	free(cnf2);
	SPC(va("^/readconfig: ^7loaded %d levels, "
		"%d admins, %d bans, %d commands and %d warnings",
		lc, ac, bc, cc, wc));
	if(lc == 0) _shrubbot_default_levels();
	return qtrue;
}

qboolean G_shrubbot_time(gentity_t *ent, int skiparg) 
{
	time_t t;
	struct tm *lt;
	char s[50];

	if(!time(&t)) return qfalse;
	lt = localtime(&t);
	strftime(s, sizeof(s), "%I:%M%p %Z", lt);
	AP(va("chat \"^/Local Time: ^7%s\" -1", s));
	return qtrue;
}

qboolean G_shrubbot_setlevel(gentity_t *ent, int skiparg) 
{
	int pids[MAX_CLIENTS];
	char name[MAX_NAME_LENGTH], err[MAX_STRING_CHARS];
	char lstr[11]; // 10 is max strlen() for 32-bit int
	char n2[MAX_NAME_LENGTH];
	int l, i;
	gentity_t *vic;
	qboolean updated = qfalse;
	g_shrubbot_admin_t *a;
	qboolean found = qfalse;


	if(Q_SayArgc() < 3+skiparg) {
		SPC("^/setlevel usage:^7 !setlevel [name|slot#] [level]");
		return qfalse;
	}
	Q_SayArgv(1+skiparg, name, sizeof(name));
	Q_SayArgv(2+skiparg, lstr, sizeof(lstr));
	l = atoi(lstr);

	// forty - don't allow privilege escalation
	if(l > _shrubbot_level(ent)) {
		SPC("^/setlevel: ^7you may not setlevel higher than your current level");
		return qfalse;
	}

	// tjw: if somone sets g_shrubbot on a running server then uses
	//      setlevel in the console, we need to make sure we have
	//      at least the default levels loaded.
	if(!ent)
		G_shrubbot_readconfig(NULL, 0);

	for(i=0; g_shrubbot_levels[i]; i++) {
		if(g_shrubbot_levels[i]->level == l) {
			found = qtrue;
			break;
		}
	}
	if(!found) {
		SPC("^/setlevel: ^7level is not defined");
		return qfalse;
	}
	if(ClientNumbersFromString(name, pids) != 1) {
		G_MatchOnePlayer(pids, err, sizeof(err));
		SPC(va("^/setlevel:^7 %s", err));
		return qfalse;
	}
	if(!_shrubbot_admin_higher(ent, &g_entities[pids[0]])) {
		SPC("^/setlevel: ^7sorry, but your intended victim has a higher"
			" admin level than you do");
		return qfalse;
	}
	vic = &g_entities[pids[0]];
	// tjw: use raw name
	//SanitizeString(vic->client->pers.netname, n2, qtrue);
	Q_strncpyz(n2, vic->client->pers.netname, sizeof(n2));

	for(i=0;g_shrubbot_admins[i];i++) {
		if(!Q_stricmp(g_shrubbot_admins[i]->guid,
			vic->client->sess.guid)) {

			g_shrubbot_admins[i]->level = l;
			Q_strncpyz(g_shrubbot_admins[i]->name, n2, 
				sizeof(g_shrubbot_admins[i]->name));
			updated = qtrue;
		}
	}
	if(!updated) {
		if(i == MAX_SHRUBBOT_ADMINS) {
			SPC("^/setlevel: ^7too many admins");
			return qfalse;
		}
		a = malloc(sizeof(g_shrubbot_admin_t));
		a->level = l;
		Q_strncpyz(a->name, n2, sizeof(a->name));
		Q_strncpyz(a->guid, vic->client->sess.guid, sizeof(a->guid));
		*a->flags = '\0';
		*a->greeting = '\0';
		g_shrubbot_admins[i] = a;
	}

	AP(va("chat \"^/setlevel: ^7%s^7 is now a level %d user\" -1",
		vic->client->pers.netname,
		l));
	_shrubbot_writeconfig();
	return qtrue;
}

qboolean G_shrubbot_kick(gentity_t *ent, int skiparg) 
{
	int pids[MAX_CLIENTS];
	char name[MAX_NAME_LENGTH], *reason, err[MAX_STRING_CHARS];
	int minargc, banTime = SHRUBBOT_KICK_LENGTH;

	minargc = 3+skiparg;
	if(G_shrubbot_permission(ent, SBF_UNACCOUNTABLE)) 
		minargc = 2+skiparg;

	if(Q_SayArgc() < minargc) {
		SPC("^/kick usage: ^7!kick [name] [reason]");
		return qfalse;
	}
	Q_SayArgv(1+skiparg, name, sizeof(name));
	reason = Q_SayConcatArgs(2+skiparg);
	if(ClientNumbersFromString(name, pids) != 1) {
		G_MatchOnePlayer(pids, err, sizeof(err));
		SPC(va("^/kick: ^7%s", err));
		return qfalse;
	}
	if(!_shrubbot_admin_higher(ent, &g_entities[pids[0]])) {
		SPC("^/kick: ^7sorry, but your intended victim has a higher admin"
			" level than you do");
		return qfalse;
	}
	if(_shrubbot_immutable(ent, &g_entities[pids[0]])) {
        SPC("^/kick: ^7sorry, but your intended victim is immune to shrubbot commands");
        return qfalse;
    }
	if ((g_OmniBotFlags.integer & BOT_FLAGS_NO_KICKBAN) &&
		(g_entities[pids[0]].r.svFlags & SVF_BOT)) {

		SPC("^/kick: ^7sorry, but bots cannot be !kicked or !banned");
        return qfalse;
	}

	// forty - enforce the temp ban consistently using shrubbot. 
	if ( (g_autoTempBan.integer & TEMPBAN_SHRUB_KICK)
		&& g_autoTempBanTime.integer > 0 ) {
		banTime = g_autoTempBanTime.integer;
		G_shrubbot_tempban(pids[0],	
			va("You have been kicked, Reason: %s",
			(*reason) ? reason : "kicked by admin"), 
			banTime);
	}

	//josh: 2.60 won't kick from the engine. This will call 
	//      ClientDisconnect
	trap_DropClient(pids[0],
		va("You have been kicked, Reason: %s\n%s",
		(*reason) ? reason : "kicked by admin", g_dropMsg.string),
		banTime);
	return qtrue;
}

qboolean G_shrubbot_tempban(int clientnum, char *reason, int length) 
{

	char userinfo[MAX_INFO_STRING];
	char *guid, *ip;
	char tmp[MAX_NAME_LENGTH];
	int i;
	g_shrubbot_ban_t *b = NULL;
	time_t t;
	struct tm *lt;
	gentity_t *vic;

	if(!time(&t)) return qfalse;

	trap_GetUserinfo(clientnum, userinfo, sizeof(userinfo));
	vic = &g_entities[clientnum];
	if(!(g_spoofOptions.integer & SPOOFOPT_USERINFO_GUID)){
		guid = vic->client->sess.guid;
	}else{
		guid = Info_ValueForKey(userinfo, "cl_guid");
	}
	if(!(g_spoofOptions.integer & SPOOFOPT_USERINFO_IP)){
		ip = vic->client->sess.ip;
	}else{
		ip = Info_ValueForKey(userinfo, "ip");
	}

	b = malloc(sizeof(g_shrubbot_ban_t));

	if(!b) 
		return qfalse;

	Q_strncpyz(b->name, vic->client->pers.netname, sizeof(b->name));
	Q_strncpyz(b->guid, guid, sizeof(b->guid));

	// strip port off of ip
	for(i=0; *ip; *ip++) {
		if(i >= sizeof(tmp) || *ip == ':') break;
		tmp[i++] = *ip; 
	}
	tmp[i] = '\0';
	Q_strncpyz(b->ip, tmp, sizeof(b->ip));

	lt = localtime(&t);
	strftime(b->made, sizeof(b->made), "%m/%d/%y %H:%M:%S", lt);
	Q_strncpyz(b->banner, "Temp Ban System", sizeof(b->banner));

	b->expires = t - SHRUBBOT_BAN_EXPIRE_OFFSET + length;
	if(!*reason) {
		Q_strncpyz(b->reason,
			"banned by Temp Ban System",
			sizeof(b->reason));
	} else {
		Q_strncpyz(b->reason, reason, sizeof(b->reason));
	}

	for(i=0; g_shrubbot_bans[i]; i++);
	if(i == MAX_SHRUBBOT_BANS) {
		free(b);
		return qfalse;
	}
	g_shrubbot_bans[i] = b;
	_shrubbot_writeconfig();
	return qtrue;
}

qboolean G_shrubbot_ban(gentity_t *ent, int skiparg) 
{
	int pids[MAX_CLIENTS];
	int seconds;
	char name[MAX_NAME_LENGTH], secs[8];
	char *reason, err[MAX_STRING_CHARS];
	char userinfo[MAX_INFO_STRING];
	char *guid, *ip;
	char tmp[MAX_NAME_LENGTH];
	int i;
	g_shrubbot_ban_t *b = NULL;
	time_t t;
	struct tm *lt;
	gentity_t *vic;
	int minargc;
	char duration[MAX_STRING_CHARS];
	int modifier = 1;

	if(!time(&t)) return qfalse;
	if(G_shrubbot_permission(ent, SBF_CAN_PERM_BAN) &&
		G_shrubbot_permission(ent, SBF_UNACCOUNTABLE)) {
		minargc = 2+skiparg;
	}
	else if(G_shrubbot_permission(ent, SBF_CAN_PERM_BAN) ||
		G_shrubbot_permission(ent, SBF_UNACCOUNTABLE)) {

		minargc = 3+skiparg;
	}
	else {
		minargc = 4+skiparg;
	}
	if(Q_SayArgc() < minargc) {
		SPC("^/ban usage: ^7!ban [name] [time] [reason]");
		return qfalse;
	}
	Q_SayArgv(1+skiparg, name, sizeof(name));
	Q_SayArgv(2+skiparg, secs, sizeof(secs));

	// tjw: support "w" (weeks), "d" (days), "h" (hours),
	//      and "m" (minutes) modifiers
	if(secs[0]) {
		int lastchar = strlen(secs) - 1;

		if(secs[lastchar] == 'w') 
			modifier = 60*60*24*7;
		else if(secs[lastchar] == 'd') 
			modifier = 60*60*24;
		else if(secs[lastchar] == 'h') 
			modifier = 60*60;
		else if(secs[lastchar] == 'm') 
			modifier = 60;

		if (modifier != 1)
			secs[lastchar] = '\0';
	}
	seconds = atoi(secs);
	if(seconds > 0)
		seconds *= modifier;

	if(seconds <= 0) {
		if(G_shrubbot_permission(ent, SBF_CAN_PERM_BAN)) {
			seconds = 0;
		}
		else {
			SPC("^/ban: ^7time must be a positive integer");
			return qfalse;
		}
		reason = Q_SayConcatArgs(2+skiparg);
	}
	else {
		reason = Q_SayConcatArgs(3+skiparg);
	}

	if(ClientNumbersFromString(name, pids) != 1) {
		G_MatchOnePlayer(pids, err, sizeof(err));
		SPC(va("^/ban: ^7%s", err));
		return qfalse;
	}
	vic = &g_entities[pids[0]];
	if(!_shrubbot_admin_higher(ent, vic)) {
		SPC("^/ban: ^7sorry, but your intended victim has a higher admin"
			" level than you do");
		return qfalse;
	}
	if(_shrubbot_immutable(ent, vic)) {
        SPC("^/ban: ^7sorry, but your intended victim is immune to shrubbot commands");
        return qfalse;
    }
	if ((g_OmniBotFlags.integer & BOT_FLAGS_NO_KICKBAN) &&
		(vic->r.svFlags & SVF_BOT)) {

		SPC("^/ban: ^7sorry, but bots cannot be !kicked or !banned");
        return qfalse;
	}

	trap_GetUserinfo(pids[0], userinfo, sizeof(userinfo));
	// Dens: for security reasons we want to use the stored guid and IP
	// (See ClientConnect in g_client.c for more information)
	if(!(g_spoofOptions.integer & SPOOFOPT_USERINFO_GUID)){
		guid = vic->client->sess.guid;
	}else{
		guid = Info_ValueForKey(userinfo, "cl_guid");
	}
	if(!(g_spoofOptions.integer & SPOOFOPT_USERINFO_IP)){
		ip = vic->client->sess.ip;
	}else{
		ip = Info_ValueForKey(userinfo, "ip");
	}
	b = malloc(sizeof(g_shrubbot_ban_t));

	if(!b) 
		return qfalse;

	//SanitizeString(vic->client->pers.netname, tmp, qtrue);
	Q_strncpyz(b->name,
		vic->client->pers.netname,
		sizeof(b->name));
	Q_strncpyz(b->guid, guid, sizeof(b->guid));

	// strip port off of ip
	for(i=0; *ip; *ip++) {
		if(i >= sizeof(tmp) || *ip == ':') break;
		tmp[i++] = *ip; 
	}
	tmp[i] = '\0';
	Q_strncpyz(b->ip, tmp, sizeof(b->ip));

	lt = localtime(&t);
	strftime(b->made, sizeof(b->made), "%m/%d/%y %H:%M:%S", lt);
	if(ent) {
		//SanitizeString(ent->client->pers.netname, tmp, qtrue);
		//Q_strncpyz(b->banner, tmp, sizeof(b->banner));
		Q_strncpyz(b->banner,
			ent->client->pers.netname,
			sizeof(b->banner));
	}
	else Q_strncpyz(b->banner, "console", sizeof(b->banner));
	if(!seconds) 
		b->expires = 0;
	else 
		b->expires = t - SHRUBBOT_BAN_EXPIRE_OFFSET + seconds;
	if(!*reason) {
		Q_strncpyz(b->reason,
			"banned by admin",
			sizeof(b->reason));
	}
	else {
		Q_strncpyz(b->reason, reason, sizeof(b->reason));
	}
	for(i=0; g_shrubbot_bans[i]; i++);
	if(i == MAX_SHRUBBOT_BANS) {
		SPC("^/ban: ^7too many bans");
		free(b);
		return qfalse;
	}
	g_shrubbot_bans[i] = b;
	SPC(va("^/ban: ^7%s^7 is now banned", vic->client->pers.netname));
	_shrubbot_writeconfig();
	// 0 seconds. Let bans handle time
	if(seconds) {
		Com_sprintf(duration,
			sizeof(duration),
			"for %i seconds",
			seconds);
	}
	else {
		Q_strncpyz(duration, "PERMANENTLY", sizeof(duration));
	}
	trap_DropClient(pids[0],
		va("You have been banned %s, Reason: %s\n%s",
		duration,
		(*reason) ? reason : "banned by admin",
		g_dropMsg.string),
		0);
	return qtrue;
}

qboolean G_shrubbot_unban(gentity_t *ent, int skiparg) 
{
	int bnum;
	char bs[5];
	time_t t;
	
	if(!time(&t)) return qfalse;
	if(Q_SayArgc() < 2+skiparg) {
		SPC("^/unban usage: ^7!unban [ban #]");
		return qfalse;
	}
	Q_SayArgv(1+skiparg, bs, sizeof(bs));
	bnum = atoi(bs);
	if(bnum < 1) {
		SPC("^/unban: ^7invalid ban #");
		return qfalse;
	}
	if(!g_shrubbot_bans[bnum-1]) {
		SPC("^/unban: ^7invalid ban #");
		return qfalse;
	}
	g_shrubbot_bans[bnum-1]->expires = t - SHRUBBOT_BAN_EXPIRE_OFFSET;
	SPC(va("^/unban: ^7ban #%d removed", bnum));
	_shrubbot_writeconfig();
	return qtrue;
}

qboolean G_shrubbot_putteam(gentity_t *ent, int skiparg) 
{
	int pids[MAX_CLIENTS];
	char name[MAX_NAME_LENGTH], team[7], err[MAX_STRING_CHARS];
	gentity_t *vic;

	Q_SayArgv(1+skiparg, name, sizeof(name));
	Q_SayArgv(2+skiparg, team, sizeof(team));
	if(Q_SayArgc() < 3+skiparg) {
		SPC("^/putteam usage: ^7!putteam [name] [r|b|s]");
		return qfalse;
	}
	Q_SayArgv(1+skiparg, name, sizeof(name));
	Q_SayArgv(2+skiparg, team, sizeof(team));

	if(ClientNumbersFromString(name, pids) != 1) {
		G_MatchOnePlayer(pids, err, sizeof(err));
		SPC(va("^/putteam: ^7%s", err));
		return qfalse;
	}
	vic = &g_entities[pids[0]];
	if(!_shrubbot_admin_higher(ent, vic)) {
		SPC("^/putteam: ^7sorry, but your intended victim has a higher "
			" admin level than you do");
		return qfalse;
	}
	if(_shrubbot_immutable(ent, vic)) {
		SPC("^/putteam: ^7sorry, but your intended victim is immune to shrubbot commands");
		return qfalse;
	}
	if(!SetTeam(vic, team, qtrue, -1, -1, qfalse)) {
		SPC("^/putteam: ^7SetTeam failed");
		return qfalse;
	}
	
	if ( !Q_stricmp( team, "red" ) 
		|| !Q_stricmp( team, "r" ) 
		|| !Q_stricmp( team, "axis" )
        	|| !Q_stricmp( team, "blue" ) 
		|| !Q_stricmp( team, "b" ) 
		|| !Q_stricmp( team, "allies" ) ) {

		// josh: mute ATB if someone is putteamed. to axis or allies
		level.atb_has_run = qtrue;
	} 
	return qtrue;
}

qboolean G_shrubbot_mute(gentity_t *ent, int skiparg) 
{
	int pids[MAX_CLIENTS], seconds, modifier = 1;
	char *reason, secs[8];
	char name[MAX_NAME_LENGTH], err[MAX_STRING_CHARS];
	char command[MAX_SHRUBBOT_CMD_LEN], *cmd;
	gentity_t *vic;

	Q_SayArgv(skiparg, command, sizeof(command));
	cmd = command;
	if(*cmd == '!'){
		*cmd++;
	}

	if(Q_SayArgc() < 2+skiparg) {
		if(!Q_stricmp(cmd, "mute")){
			SPC("^/mute usage: ^7!mute [name|slot#] [time] [reason]");
		}else{
			SPC("^/unmute usage: ^7!mute [name|slot#]");
		}
		return qfalse;
	}

	Q_SayArgv(1+skiparg, name, sizeof(name)); 
	if(ClientNumbersFromString(name, pids) != 1) {
		G_MatchOnePlayer(pids, err, sizeof(err));
		SPC(va("^/%smute: ^7%s",
			Q_stricmp(cmd, "mute")? "un":"",
			err));
		return qfalse;
	}
	
	Q_SayArgv(2+skiparg, secs, sizeof(secs));
	if(secs[0]) {
		int lastchar = strlen(secs) - 1;

		if(secs[lastchar] == 'w') 
			modifier = 60*60*24*7;
		else if(secs[lastchar] == 'd') 
			modifier = 60*60*24;
		else if(secs[lastchar] == 'h') 
			modifier = 60*60;
		else if(secs[lastchar] == 'm') 
			modifier = 60;

		if (modifier != 1)
			secs[lastchar] = '\0';
	}
	seconds = atoi(secs);
	if(seconds > 0){
		seconds *= modifier;
	}

	if(seconds <= 0){
		reason = Q_SayConcatArgs(2+skiparg);
	}else{
		reason = Q_SayConcatArgs(3+skiparg);
	}

	vic = &g_entities[pids[0]];
	if(!_shrubbot_admin_higher(ent, vic)) {
		SPC(va("^/%smute: ^7sorry, but your intended victim has a higher admin"
			" level than you do", Q_stricmp(cmd, "mute")? "un":""));
		return qfalse;
	}
	if(_shrubbot_immutable(ent, vic)) {
        SPC(va("^/%smute: ^7sorry, but your intended victim is immune to shrubbot commands",
			Q_stricmp(cmd, "mute")? "un":""));
        return qfalse;
    }
	if(vic->client->sess.auto_unmute_time != 0) {
		if(!Q_stricmp(cmd, "mute")) {
			if(vic->client->sess.auto_unmute_time == -1){
				seconds = 0;
			}else{
				seconds = (vic->client->sess.auto_unmute_time - level.time)/1000;
			}
			SPC(va("^/mute: ^7player is already muted%s",
				seconds ? va(" for %i seconds", seconds) : ""));
			return qtrue;
		}
		vic->client->sess.auto_unmute_time = 0;
		CPx(pids[0], "cp \"^5You've been unmuted\n\"" );
		AP(va("chat \"%s^7 has been unmuted\" -1",  
			vic->client->pers.netname ));
	}else{
		if(!Q_stricmp(cmd, "unmute")) {
			SPC("^/unmute: ^7player is not currently muted");
			return qtrue;
		}
		if(seconds<=0){
			vic->client->sess.auto_unmute_time = -1;
		}else{
			vic->client->sess.auto_unmute_time = (level.time + 1000*seconds);
		}
		CPx(pids[0], va("cp \"^5You've been muted%s%s\n\"",
			(seconds > 0) ? va(" for %i seconds",seconds): "",
			*reason ? va(" because: %s", reason): ""));
		AP(va("chat \"%s^7 has been muted%s\" -1",
			vic->client->pers.netname,
			(seconds > 0) ? va(" for %i seconds",seconds): ""));
	}
	ClientUserinfoChanged(pids[0]);
	return qtrue;
}


qboolean G_shrubbot_pause(gentity_t *ent, int skiparg) 
{
	G_refPause_cmd(ent, qtrue);
	return qtrue;
}

qboolean G_shrubbot_unpause(gentity_t *ent, int skiparg) 
{
	G_refPause_cmd(ent, qfalse);
	return qtrue;
}

qboolean G_shrubbot_listplayers(gentity_t *ent, int skiparg) 
{
	int i,j;
	gclient_t *p;
	char c[3], t[2]; // color and team letter
	char n[MAX_NAME_LENGTH] = {""};
	char n2[MAX_NAME_LENGTH] = {""};
	char n3[MAX_NAME_LENGTH] = {""};
	char lname[MAX_NAME_LENGTH];
	char lname2[MAX_NAME_LENGTH];
	char guid_stub[9];
	char fireteam[2];
	char muted[2];
	char warned[2];
	int l;
	fireteamData_t *ft;
	int lname_max = 1;
	char lname_fmt[5];
	char cleanLevelName[MAX_NAME_LENGTH];
	int spaces;

	// detect the longest level name length
	for(i=0; g_shrubbot_levels[i]; i++) {
		//SanitizeString(g_shrubbot_levels[i]->name, n, qtrue);
		DecolorString(g_shrubbot_levels[i]->name, n);
		if(strlen(n) > lname_max)
			lname_max = strlen(n);
		Com_sprintf(n, sizeof(n), "%d", g_shrubbot_levels[i]->level);
		if(strlen(n) > lname_max)
			lname_max = strlen(n);
	}

	SBP_begin();
	SBP(va("^/listplayers: ^7%d players connected:\n", 
		level.numConnectedClients));
	for(i=0; i < level.maxclients; i++) {
		p = &level.clients[i];
		Q_strncpyz(t, "S", sizeof(t));
		Q_strncpyz(c, S_COLOR_YELLOW, sizeof(c));
		if(p->sess.sessionTeam == TEAM_ALLIES) {
			Q_strncpyz(t, "B", sizeof(t));
			Q_strncpyz(c, S_COLOR_BLUE, sizeof(c));
		}
		else if(p->sess.sessionTeam == TEAM_AXIS) {
			Q_strncpyz(t, "R", sizeof(t));
			Q_strncpyz(c, S_COLOR_RED, sizeof(c));
		}

		if(p->pers.connected == CON_CONNECTING) {
			Q_strncpyz(t, "C", sizeof(t));
			Q_strncpyz(c, S_COLOR_ORANGE, sizeof(c));
		}
		else if(p->pers.connected != CON_CONNECTED) {
			continue;
		}

		if (g_entities[p->ps.clientNum].r.svFlags & SVF_BOT) {
			Q_strncpyz(guid_stub, "OMNIBOT*", sizeof(guid_stub));
		} else {
			for(j=0; j<=8; j++) 
				guid_stub[j] = p->sess.guid[j+24];
		}

		fireteam[0] = '\0';
		if(G_IsOnFireteam(i, &ft)) {
			Q_strncpyz(fireteam, 
				bg_fireteamNames[ft->ident - 1],
				sizeof(fireteam));
		}

		muted[0] = '\0';
		if(p->sess.auto_unmute_time != 0) {
			Q_strncpyz(muted,"M",sizeof(muted));
		}

		warned[0] = '\0';
		if(G_shrubbot_user_tot_warnings(&g_entities[i]) > 0) {
			Q_strncpyz(warned,"W",sizeof(warned));
		}
	
		l = 0;
		SanitizeString(p->pers.netname, n2, qtrue);
		n[0] = '\0';
		for(j=0; g_shrubbot_admins[j]; j++) {
			if(!Q_stricmp(g_shrubbot_admins[j]->guid, 
				p->sess.guid)) {

				// tjw: don't gather aka or level info if
				//      the admin is incognito
				if(G_shrubbot_permission(&g_entities[i],
					SBF_INCOGNITO)) {
					break;
				}
				l = g_shrubbot_admins[j]->level;
				SanitizeString(g_shrubbot_admins[j]->name,
					n3, qtrue);
				if(Q_stricmp(n2, n3)) {
					Q_strncpyz(n,
						g_shrubbot_admins[j]->name,
						sizeof(n));
				}
				break;
			}
		}
		lname[0] = '\0';
		for(j=0; g_shrubbot_levels[j]; j++) {
			if(g_shrubbot_levels[j]->level == l)
				Q_strncpyz(lname,
					g_shrubbot_levels[j]->name,
					sizeof(lname));	
		}
		if(!*lname)  {
			Com_sprintf(lname,
				sizeof(lname),
				"%i",
				l);
		}
		// gabriel: level names can have color codes in them. Vary the number of
		// spaces in the print mask so the level names have the correct number
		// of leading spaces
		DecolorString(lname, cleanLevelName);
		spaces = lname_max - strlen(cleanLevelName);
		Com_sprintf(lname_fmt, sizeof(lname_fmt), "%%%is", spaces + strlen(lname));
		Com_sprintf(lname2, sizeof(lname2), lname_fmt, lname);
		SBP(va("%2i %s%s^3%1s^7 %-2i '%s^7' (*%s) ^1%1s^8%1s ^7%s^7 %s%s^7%s\n", 
			i,
			c,
			t,
			fireteam,
			l,
			lname2,
			guid_stub,
			muted,
			warned,
			p->pers.netname,
			(*n) ? "(a.k.a. " : "",
			n,
			(*n) ? ")" : ""
			));
	}
	SBP_end();
	return qtrue;
}

qboolean G_shrubbot_listteams(gentity_t *ent, int skiparg)
{
	int playerCount[3], pings[3], totalXP[3];
	int i, j;
	gclient_t *p;

	playerCount[0] = pings[0] = totalXP[0] = 0;
	playerCount[1] = pings[1] = totalXP[1] = 0;
	playerCount[2] = pings[2] = totalXP[2] = 0;

	for(i=0; i<level.maxclients; i++) {
		p = &level.clients[i];
		if(p->pers.connected != CON_CONNECTED) {
			continue;
		}
		if(p->sess.sessionTeam == TEAM_ALLIES) {
			playerCount[2]++;
			pings[2] += p->ps.ping;
			for(j = 0; j < SK_NUM_SKILLS; j++) {
				totalXP[2] += p->sess.skillpoints[j];
			}
		} else if(p->sess.sessionTeam == TEAM_AXIS) {
			playerCount[1]++;
			pings[1] += p->ps.ping;
			for(j = 0; j < SK_NUM_SKILLS; j++) {
				totalXP[1] += p->sess.skillpoints[j];
			}
		} else if(p->sess.sessionTeam == TEAM_SPECTATOR) {
			playerCount[0]++;
			pings[0] += p->ps.ping;
			for(j = 0; j < SK_NUM_SKILLS; j++) {
				totalXP[0] += p->sess.skillpoints[j];
			}
		}
	}
	SBP_begin();
	SBP(va("Desc       ^4Allies^7(%s^7)      ^1Axis^7(%s^7)       ^3Specs\n",
			teamInfo[TEAM_ALLIES].team_lock ? "^1L" : "^2U",
			teamInfo[TEAM_AXIS].team_lock ? "^1L" : "^2U"));
	SBP("^7---------------------------------------------\n");
	SBP(va("^2Players     ^7%8d     %8d    %8d\n",
			playerCount[2],
			playerCount[1],
			playerCount[0]));
	SBP(va("^2Avg Ping    ^7%8.2f     %8.2f    %8.2f\n",
			playerCount[2]>0?(float)(pings[2])/playerCount[2]:0,
			playerCount[1]>0?(float)(pings[1])/playerCount[1]:0,
			playerCount[0]>0?(float)(pings[0])/playerCount[0]:0));
	SBP(va("^2Map XP      ^7%8d     %8d          --\n",
			level.teamScores[TEAM_ALLIES],
			level.teamScores[TEAM_AXIS]));
	SBP(va("^2Total XP    ^7%8d     %8d    %8d\n",
			totalXP[2],
			totalXP[1],
			totalXP[0]));
	SBP(va("^2Avg Map XP  ^7%8.2f     %8.2f          --\n",
			playerCount[2]>0?
				(float)(level.teamScores[TEAM_ALLIES])/
					playerCount[2]:0,
			playerCount[1]>0?
				(float)(level.teamScores[TEAM_AXIS])/
					playerCount[1]:0));
	SBP(va("^2Avg Tot XP  ^7%8.2f     %8.2f    %8.2f\n",
			playerCount[2]>0?(float)(totalXP[2])/playerCount[2]:0,
			playerCount[1]>0?(float)(totalXP[1])/playerCount[1]:0,
			playerCount[0]>0?(float)(totalXP[0])/playerCount[0]:0));
	SBP("\n");
	SBP("Ratings       ^5Win Prob       ^5Win Points\n");
	SBP("^7---------------------------------------------\n");
	SBP(va("^4Allies      ^7%8.2f\n",
		G_GetWinProbability(TEAM_ALLIES,NULL,qfalse)));
	SBP(va("^1Axis        ^7%8.2f\n",
		1.0 - G_GetWinProbability(TEAM_AXIS,NULL,qfalse)));
	SBP_end();
	return qfalse;
}

qboolean G_shrubbot_balance(gentity_t *ent, int skiparg) {
	AP(va("chat \"^/balance: ^7Single ATB Cycle Ran\""));
	G_ActiveTeamBalance(qtrue, qfalse);
	return qtrue;
}

qboolean G_shrubbot_howfair(gentity_t *ent, int skiparg) {
	float axisProb, alliesProb, difference,fair_interval;
	char * howfair;
	G_GetWinProbability(TEAM_ALLIES,NULL,qfalse);
	alliesProb = level.win_probability_model.win_probability;
	axisProb = 1.0 - level.win_probability_model.win_probability;

	difference = Q_fabs(max(axisProb,alliesProb) - 0.5);
	fair_interval = Q_fabs(g_ATB_diff.integer*1.0 / 100 - 0.5)/4.0;
	if (axisProb == -1 || alliesProb == -1) {
		howfair = "^fOne team is empty";
	}
	else if (difference < 2*fair_interval)
		howfair = "^2FAIR";
	else if (difference < 3*fair_interval)
		howfair = "^3FAIR ENOUGH";
	else if (difference < 4*fair_interval)
		howfair = "^8UNEVEN";
	else
		howfair = "^1UNFAIR: ^3FIX THE TEAMS!!";
	
	AP(va("chat \"^/howfair: %s ^f(Allies: ^3%.2f ^fAxis ^3%.2f^f)\" -1",
		howfair,
		alliesProb,
		axisProb
		));

	G_LogPrintf(va("howfair: %s (Allies: %.2f Axis %.2f)\n",
		howfair,
		alliesProb,
		axisProb
		));

	return qtrue;
}

qboolean G_shrubbot_showbans(gentity_t *ent, int skiparg) 
{
	int i, found = 0;
	time_t t;
	char duration[MAX_STRING_CHARS];
	char fmt[MAX_STRING_CHARS];
	int max_name = 1, max_banner = 1;
	int secs;
	int start = 0;
	char skip[11];
	char date[11];
	char *made;
	int j;
	char n1[MAX_NAME_LENGTH] = {""};
	char n2[MAX_NAME_LENGTH] = {""};
	char tmp[MAX_NAME_LENGTH];
	int spacesName;
	int spacesBanner;

	if(!time(&t))
		return qfalse;
	t = t - SHRUBBOT_BAN_EXPIRE_OFFSET;
	
	for(i=0; g_shrubbot_bans[i]; i++) {
		if(g_shrubbot_bans[i]->expires != 0 &&
			(g_shrubbot_bans[i]->expires - t) < 1) {
			
			continue;
		}
		found++;
	}

	if(Q_SayArgc() < 3+skiparg) {
		Q_SayArgv(1+skiparg, skip, sizeof(skip));
		start = atoi(skip);
		// tjw: !showbans 1 means start with ban 0
		if(start > 0) 
			start -= 1;
		else if(start < 0)
			start = found + start;
	}

	// tjw: sanity check
	if(start >= MAX_SHRUBBOT_BANS || start < 0)
		start = 0;

	for(i=start;
		(g_shrubbot_bans[i] && (i-start) < SHRUBBOT_MAX_SHOWBANS);
		i++) {

		DecolorString(g_shrubbot_bans[i]->name, n1);
		DecolorString(g_shrubbot_bans[i]->banner, n2);
		if(strlen(n1) > max_name) {
			max_name = strlen(n1);
		}
		if(strlen(n2) > max_banner) {
			max_banner = strlen(n2);
		}
	}

	if(start > found) {
		SPC(va("^/showbans: ^7there are only %d active bans", found));
		return qfalse;
	}
	SBP_begin();
	for(i=start;
		(g_shrubbot_bans[i] && (i-start) < SHRUBBOT_MAX_SHOWBANS);
		i++) {

		if(g_shrubbot_bans[i]->expires != 0 &&
			(g_shrubbot_bans[i]->expires - t) < 1) {
			
			continue;
		}

		// tjw: only print out the the date part of made
		date[0] = '\0';
		made = g_shrubbot_bans[i]->made;
		for(j=0; *made; j++) {
			if((j+1) >= sizeof(date)) 
				break;
			if(*made == ' ')
				break;
			date[j] = *made;
			date[j+1] = '\0';
			*made++;
		}

		secs = (g_shrubbot_bans[i]->expires - t);
		G_shrubbot_duration(secs, duration, sizeof(duration));

		DecolorString(g_shrubbot_bans[i]->name, tmp);
		spacesName = max_name - strlen(tmp);
		DecolorString(g_shrubbot_bans[i]->banner, tmp);
		spacesBanner = max_banner - strlen(tmp);

		Com_sprintf(fmt, sizeof(fmt),
			"^F%%4i^7 %%-%is^7 ^F%%-10s^7 %%-%is^7 ^F%%-9s^7 %%s\n",
			spacesName + strlen(g_shrubbot_bans[i]->name),
			spacesBanner + strlen(g_shrubbot_bans[i]->banner));

		SBP(va(fmt, 
			(i+1),
			g_shrubbot_bans[i]->name,
			date,
			g_shrubbot_bans[i]->banner,
			duration,
			g_shrubbot_bans[i]->reason
			));
	}

	SBP(va("^/showbans: ^7showing bans %d - %d of %d\n",
		(start) ? (start + 1) : 0,
		((start + SHRUBBOT_MAX_SHOWBANS) > found) ?
			found : (start + SHRUBBOT_MAX_SHOWBANS),
		found));
	if((start + SHRUBBOT_MAX_SHOWBANS) < found) {
		SBP(va("          type !showbans %d to see more\n",
			(start + SHRUBBOT_MAX_SHOWBANS + 1)));
	}
	SBP_end();
	return qtrue;
}

qboolean G_shrubbot_help(gentity_t *ent, int skiparg) 
{
	int i;

	if(Q_SayArgc() < 2+skiparg) {
		//!help
		int j = 0;
		int count = 0;
		char *str = "";

		
		for (i=0; g_shrubbot_cmds[i].keyword[0]; i++) {

			if(G_shrubbot_permission(ent, g_shrubbot_cmds[i].flag)) {
				if( j == 6){
					str = va( "%s\n^f%-12s", str, g_shrubbot_cmds[i].keyword);
					j = 0;
				}else{
					str = va( "%s^f%-12s", str, g_shrubbot_cmds[i].keyword);
				}
				j++;
				count++;
			}
		}
		SBP_begin();
		SBP(str);
		// tjw: break str into at least two parts to try to avoid
		//      1022 char limitation
		str = "";
		for (i=0; g_shrubbot_commands[i]; i++) {
			if(!_shrubbot_command_permission(ent,
				g_shrubbot_commands[i]->command)) {

				continue;
			}

			if(j == 6){
				str = va( "%s\n^f%-12s", str,
					va("%s", g_shrubbot_commands[i]->command));
				j = 0;
			}else{
				str = va( "%s^f%-12s", str,
					va("%s", g_shrubbot_commands[i]->command));
			}

			j++;
			count++;
		}
		if(count)
			str = va("%s\n",str);
		SPC(va("^/help: ^7%i available commands", count));
		
		SBP( str );
		SBP("^7Type !help [^3command^7] for help with a specific command.\n");
		SBP_end();

		return qtrue;
	} else {
		//!help param
		char param[20];
		
		Q_SayArgv(1+skiparg, param, sizeof(param));
		SBP_begin();
		for( i=0; g_shrubbot_cmds[i].keyword[0]; i++ ){
			if(	!Q_stricmp(param, g_shrubbot_cmds[i].keyword ) ) {
				if(!G_shrubbot_permission(ent, g_shrubbot_cmds[i].flag)) {
					SBP(va("^/help: ^7you have no permission to use '%s'\n", 
						g_shrubbot_cmds[i].keyword ) );
					return qfalse;
				}
				SBP( va("^/help: ^7help for '%s':\n", g_shrubbot_cmds[i].keyword ) );
				SBP( va("^/Funtion: ^7%s\n", g_shrubbot_cmds[i].function ) );
				SBP( va("^/Syntax: ^7!%s %s\n", g_shrubbot_cmds[i].keyword, 
					g_shrubbot_cmds[i].syntax ) );
				SBP( va("^/Flag: ^7'%c'\n", g_shrubbot_cmds[i].flag) );
				SBP_end();
				return qtrue;
			}
		}
		for( i=0; g_shrubbot_commands[i]; i++ ){
			if(	!Q_stricmp(param, g_shrubbot_commands[i]->command ) ) {
				if(!_shrubbot_command_permission(ent,
					g_shrubbot_commands[i]->command)) {

					SBP(va("^/help: ^7you have no permission to use '%s'\n", 
						g_shrubbot_commands[i]->command ) );
					SBP_end();
					return qfalse;
				}
				SBP( va("^/help: ^7help for '%s':\n",
					g_shrubbot_commands[i]->command ) );
				SBP( va("^/Description: ^7%s\n",
					g_shrubbot_commands[i]->desc ) );
				SBP( va("^/Syntax: ^7!%s\n",
					g_shrubbot_commands[i]->command) );
				SBP_end();
				return qtrue;
			}
		}
		SBP( va("^/help: ^7no help found for '%s'\n", param ) );
		SBP_end();
		return qfalse;
	}
}

qboolean G_shrubbot_admintest(gentity_t *ent, int skiparg) 
{
	int i, l = 0;
	char *lname = NULL;
	int pids[MAX_CLIENTS];
	char name[MAX_NAME_LENGTH], err[MAX_STRING_CHARS];
	gentity_t *vic;

	if(Q_SayArgc() < 2+skiparg) {
		if(!ent) {    
			SPC("admintest: you are on the console");
			return qtrue;
		}else{
			vic = ent;
		}
	}else{
		if(!G_shrubbot_permission(ent,SBF_ADMINTEST_OTHER)){
			SPC("^/admintest: ^7you don't have permission to use !admintest on other players");
			return qfalse;
		}
		Q_SayArgv(1+skiparg, name, sizeof(name)); 
		if(ClientNumbersFromString(name, pids) != 1) {
			G_MatchOnePlayer(pids, err, sizeof(err));
			SPC(va("^/admintest: ^7%s", err));
			return qfalse;
		}
		vic = &g_entities[pids[0]];
		// Dens: if someone is incog, feel free to show level 0
		if(!_shrubbot_admin_higher(ent, vic) &&
			!G_shrubbot_permission(vic,	SBF_INCOGNITO)) {
			SPC("^/admintest: ^7sorry, but your intended victim has a higher admin"
				" level than you do");
			return qfalse;
		}

		if(_shrubbot_immutable(ent, vic)) {
			SPC("^/admintest: ^7sorry, but your intended victim is immune to shrubbot commands");
			return qfalse;
		}
	}

	// Get the level number
	for(i=0; g_shrubbot_admins[i]; i++) {
		if(!Q_stricmp(g_shrubbot_admins[i]->guid,
			vic->client->sess.guid)) {

			if(G_shrubbot_permission(vic,
				SBF_INCOGNITO)) {
				break;
			}

			l = g_shrubbot_admins[i]->level;
			break;    
		}
	}

	// Get the label for the level number
	for(i=0; g_shrubbot_levels[i]; i++) {
		if (g_shrubbot_levels[i]->level == l) {
			lname = g_shrubbot_levels[i]->name;
			break;
		}
	}

	AP(va("chat \"^/admintest: ^7%s^7 is a level %d user %s%s^7%s\" -1",
		vic->client->pers.netname,
		l,
		(lname) ? "(" : "",
		(lname) ? lname : "",
		(lname) ? ")" : ""));
	return qtrue;
}

qboolean G_shrubbot_cancelvote(gentity_t *ent, int skiparg) 
{

	if( level.voteInfo.voteTime ){
		level.voteInfo.voteYes = 0;
		level.voteInfo.voteNo = level.numConnectedClients;
		CheckVote( qtrue );
		SPC("^/cancelvote: ^7turns out everyone voted No");
	}else{
		SPC("^/cancelvote: ^7No vote in progress");
	}
	return qtrue;
}

qboolean G_shrubbot_passvote(gentity_t *ent, int skiparg) 
{
	if( level.voteInfo.voteTime ){
		level.voteInfo.voteYes = level.numConnectedClients;
		level.voteInfo.voteNo = 0;
		CheckVote( qtrue );
		SPC("^/passvote: ^7turns out everyone voted Yes");
	}else{
		SPC("^/passvote: ^7No vote in progress");
	}
	return qtrue;
}

qboolean G_shrubbot_uptime(gentity_t *ent, int skiparg)
{
	int secs = level.time / 1000;
	int mins = (secs / 60) % 60;
	int hours = (secs / 3600) % 24;
	int days = (secs / (3600 * 24));

	SPC( va("%s ^/uptime: ^2%d ^7day%s ^2%d ^7hours ^2%d ^7minutes", 
		sv_hostname.string, days, 
		(days != 1 ? "s" : ""),
		hours, mins) );
	return qtrue;
}

qboolean G_shrubbot_spec999(gentity_t *ent, int skiparg) 
{
	int i, movedtospec = 0;
	gentity_t *vic;

	for(i=0; i < level.maxclients; i++) {
		vic = &g_entities[i];
		if(!vic->client) continue;
		if(vic->client->pers.connected != CON_CONNECTED) continue;
		if(vic->client->ps.ping == 999) {
			SetTeam(vic, "s", qtrue, -1, -1, qfalse);
			AP(va("chat \"^/spec999: ^7%s^7 moved to spectators\" -1",
				vic->client->pers.netname));
			movedtospec++;
		}
	}
	// forty - #258 - !spec999 -> notification when 0 users moved to spec
	if(!movedtospec)
		AP("chat \"^/spec999: ^7No clients moved to spectators\" -1");
	return qtrue;
}

qboolean G_shrubbot_shuffle(gentity_t *ent, int skiparg) 
{
	G_shuffleTeams();
	return qtrue;
}

qboolean G_shrubbot_rename(gentity_t *ent, int skiparg) 
{
	int pids[MAX_CLIENTS];
	char name[MAX_NAME_LENGTH], *newname, *oldname,err[MAX_STRING_CHARS];
	char userinfo[MAX_INFO_STRING];

	if(Q_SayArgc() < 3+skiparg) {
		SPC("^/rename usage: ^7!rename [name] [newname]");
		return qfalse;
	}
	Q_SayArgv(1+skiparg, name, sizeof(name));
	newname = Q_SayConcatArgs(2+skiparg);
	if(ClientNumbersFromString(name, pids) != 1) {
		G_MatchOnePlayer(pids, err, sizeof(err));
		SPC(va("^/rename: ^7%s", err));
		return qfalse;
	}
	if(!_shrubbot_admin_higher(ent, &g_entities[pids[0]])) {
		SPC("^/rename: ^7sorry, but your intended victim has a higher admin"
			" level than you do");
		return qfalse;
	}
	if(_shrubbot_immutable(ent, &g_entities[pids[0]])) {
		SPC("^/rename: ^7sorry, but your intended victim is immune to shrubbot commands");
		return qfalse;
	}
	//SPC(va("^/!rename: ^7renaming %s ^7to %s",name,newname));
	trap_GetUserinfo( pids[0], userinfo, sizeof( userinfo ) );
	oldname = Info_ValueForKey( userinfo, "name" );
	// Send to chat for shame
	AP(va("chat \"^/rename: ^7%s^7 has been renamed to %s\" -1",
		oldname,
		newname));
	Info_SetValueForKey( userinfo, "name", newname);
	trap_SetUserinfo( pids[0], userinfo );
	ClientUserinfoChanged(pids[0]);
	return qtrue;
}

qboolean G_shrubbot_gib(gentity_t *ent, int skiparg)
{
	int pids[MAX_CLIENTS];
	char name[MAX_NAME_LENGTH], err[MAX_STRING_CHARS];
	gentity_t *vic;
	qboolean doAll = qfalse;

	if(Q_SayArgc() < 2+skiparg) doAll = qtrue;

	Q_SayArgv(1+skiparg, name, sizeof(name));
	if(!Q_stricmp(name, "-1") || !Q_stricmp(name, "all") || doAll) {
		int it;
		int count = 0;

		for( it = 0; it < level.numConnectedClients; it++ ) {
			vic = g_entities + level.sortedClients[it];
			if( !_shrubbot_admin_higher(ent, vic) ||
				_shrubbot_immutable(ent, vic) ||
				!(vic->client->sess.sessionTeam == TEAM_AXIS ||
				  vic->client->sess.sessionTeam == TEAM_ALLIES))
				continue;
			G_Damage(vic, NULL, NULL, NULL, NULL, 500, 0, MOD_UNKNOWN);
			count++;
		}
		AP(va("chat \"^/gib: ^7%i ^7players gibbed\" -1", count));
		return qtrue;
	}

	if(ClientNumbersFromString(name, pids) != 1) {
		G_MatchOnePlayer(pids, err, sizeof(err));
		SPC(va("^/gib: ^7%s", err));
		return qfalse;
	}
	vic = &g_entities[pids[0]];
	if(!_shrubbot_admin_higher(ent, vic)) {
		SPC("^/gib: ^7sorry, but your intended victim has a higher admin"
			" level than you do");
		return qfalse;
	}
	if(_shrubbot_immutable(ent, vic)) {
        SPC("^/gib: ^7sorry, but your intended victim is immune to shrubbot commands");
        return qfalse;
    }
	if(!(vic->client->sess.sessionTeam == TEAM_AXIS ||
			vic->client->sess.sessionTeam == TEAM_ALLIES)) {
		SPC("^/gib: ^7player must be on a team to be gibbed");
		return qfalse;
	}
	
	G_Damage(vic, NULL, NULL, NULL, NULL, 500, 0, MOD_UNKNOWN);
	AP(va("chat \"^/gib: ^7%s ^7was gibbed\" -1", vic->client->pers.netname));
	return qtrue;
}

qboolean G_shrubbot_slap(gentity_t *ent, int skiparg)
{
	int pids[MAX_CLIENTS], dmg;
	char name[MAX_NAME_LENGTH], err[MAX_STRING_CHARS];
	char damage[4], *reason;
	gentity_t *vic;

	if(Q_SayArgc() < 2+skiparg) {
		SPC("^/slap usage: ^7!slap [name|slot#] [damage] [reason]");
		return qfalse;
	}
	Q_SayArgv(1+skiparg, name, sizeof(name)); 
	Q_SayArgv(2+skiparg, damage, sizeof(damage));

	dmg = atoi(damage);
	if(dmg <= 0) {
		dmg = 20;
		reason = Q_SayConcatArgs(2+skiparg);
	}
	else {
		reason = Q_SayConcatArgs(3+skiparg);
	}

	if(ClientNumbersFromString(name, pids) != 1) {
		G_MatchOnePlayer(pids, err, sizeof(err));
		SPC(va("^/slap: ^7%s", err));
		return qfalse;
	}
	vic = &g_entities[pids[0]];
	if(!_shrubbot_admin_higher(ent, vic)) {
		SPC("^/slap: ^7sorry, but your intended victim has a higher admin"
			" level than you do");
		return qfalse;
	}
	if(_shrubbot_immutable(ent, vic)) {
		SPC("^/slap: ^7sorry, but your intended victim is immune to shrubbot commands");
		return qfalse;
	}
	if(!(vic->client->sess.sessionTeam == TEAM_AXIS ||
			vic->client->sess.sessionTeam == TEAM_ALLIES)) {
		SPC("^/slap: ^7player must be on a team to be slapped");
		return qfalse;
	}
	if(vic->health < 1 || vic->client->ps.pm_flags & PMF_LIMBO) {
		SPC(va("^/slap: ^7%s ^7is not alive",vic->client->pers.netname));
		return qfalse;
	}
	
	if ( vic->health > dmg ) {
		vic->health -= dmg;
	}
	else {
		vic->health = 1;
	}

	G_AddEvent(vic, EV_GENERAL_SOUND, 
		G_SoundIndex("sound/player/hurt_barbwire.wav"));
	
	AP(va("chat \"^/slap: ^7%s ^7was slapped\" -1",
		vic->client->pers.netname));

	CPx(pids[0], va("cp \"%s ^7slapped you%s%s\"", 
		(ent?ent->client->pers.netname:"^3SERVER CONSOLE"),
		(*reason) ? " because:\n" : "",
		(*reason) ? reason : ""));
	return qtrue;
}

qboolean G_shrubbot_burn(gentity_t *ent, int skiparg)
{
	int pids[MAX_CLIENTS];
	char name[MAX_NAME_LENGTH], err[MAX_STRING_CHARS];
	char *reason;
	gentity_t *vic;

	if(Q_SayArgc() < 2+skiparg) {
		SPC("^/burn usage: ^7!burn [name|slot#] [reason]");
		return qfalse;
	}
	Q_SayArgv(1+skiparg, name, sizeof(name));
	reason = Q_SayConcatArgs(2+skiparg);

	if(ClientNumbersFromString(name, pids) != 1) {
		G_MatchOnePlayer(pids, err, sizeof(err));
		SPC(va("^/burn: ^7%s", err));
		return qfalse;
	}
	vic = &g_entities[pids[0]];
	if(!_shrubbot_admin_higher(ent, vic)) {
		SPC("^/burn: ^/sorry, but your intended victim has a higher admin"
			" level than you do");
		return qfalse;
	}
	if(_shrubbot_immutable(ent, vic)) {
        SPC("^/burn: ^7sorry, but your intended victim is immune to shrubbot commands");
        return qfalse;
    }
	if(!(vic->client->sess.sessionTeam == TEAM_AXIS ||
			vic->client->sess.sessionTeam == TEAM_ALLIES)) {
		SPC("^/burn: ^7player must be on a team");
		return qfalse;
	}
	
	G_BurnMeGood(vic, vic, NULL);
	AP(va("chat \"^/burn: ^7%s ^7was set ablaze\" -1",
			vic->client->pers.netname));

	CPx(pids[0], va("cp \"%s ^7burned you%s%s\"", 
			(ent?ent->client->pers.netname:"^3SERVER CONSOLE"),
			(*reason) ? " because:\n" : "",
			(*reason) ? reason : ""));
	return qtrue;
}

qboolean G_shrubbot_warn(gentity_t *ent, int skiparg)
{
	int i,pids[MAX_CLIENTS], oldest = 0, structure = 0;
	int banTime = SHRUBBOT_KICK_LENGTH;
	char name[MAX_NAME_LENGTH], err[MAX_STRING_CHARS];
	char *reason, *ip;
	gentity_t *vic;
	g_shrubbot_warning_t *w = NULL;
	qboolean store = qtrue;
	char tmp[MAX_NAME_LENGTH];
	time_t t;

	if(Q_SayArgc() < 3+skiparg) {
		SPC("^/warn usage: ^7!warn [name|slot#] [reason]");
		return qfalse;
	}
	Q_SayArgv(1+skiparg, name, sizeof(name)); 
	reason = Q_SayConcatArgs(2+skiparg);

	if(ClientNumbersFromString(name, pids) != 1) {
		G_MatchOnePlayer(pids, err, sizeof(err));
		SPC(va("^/warn: ^7%s", err));
		return qfalse;
	}
	vic = &g_entities[pids[0]];
	if(!_shrubbot_admin_higher(ent, vic)) {
		SPC("^/warn: ^7sorry, but your intended victim has a higher admin"
			" level than you do");
		return qfalse;
	}
	if(_shrubbot_immutable(ent, vic)) {
		SPC("^/warn: ^7sorry, but your intended victim is immune to shrubbot commands");
		return qfalse;
	}

	if((g_warningOptions.integer & WARNOP_LINK_GUID) ||
		(g_warningOptions.integer & WARNOP_LINK_IP)){
		if(G_shrubbot_user_tot_warnings(vic) >= g_maxWarnings.integer){
			if(g_warningOptions.integer & WARNOP_KICK){
				if ( (g_autoTempBan.integer & TEMPBAN_SHRUB_WARN)
					&& g_autoTempBanTime.integer > 0 ) {
					banTime = g_autoTempBanTime.integer;
					G_shrubbot_tempban(pids[0],	
						"You have been temporarily banned because you received too many warnings", 
						banTime);
				}
				trap_DropClient(pids[0],
					"You have been kicked because you received too many warnings.",
					banTime);
				SPC(va("^/warn: ^7%s ^7was autokicked, because he received too many warnings",
					vic->client->pers.netname));
				return qtrue;
			}else{
				SPC(va("^/warn: ^7%s ^7has too many warnings. The warning has not been stored",
					vic->client->pers.netname));
				store = qfalse;
			}
		}

		w = malloc(sizeof(g_shrubbot_warning_t));
		if(!w){
			store = qfalse;
			SPC("^/warn: ^7error (warning not stored)");
		}
		
		if(!time(&t)){
			store = qfalse;
		}

		if(store == qtrue){
			Q_strncpyz(w->name, vic->client->pers.netname, sizeof(w->name));
			Q_strncpyz(w->guid, vic->client->sess.guid, sizeof(w->guid));
			ip = vic->client->sess.ip;
			for(i=0; *ip; *ip++) {
				if(i >= sizeof(tmp) || *vic->client->sess.ip == ':') break;
				tmp[i++] = *ip; 
			}
			tmp[i] = '\0';
			Q_strncpyz(w->ip, tmp, sizeof(w->ip));
			Q_strncpyz(w->warning, reason, sizeof(w->warning));
			w->made = (t - SHRUBBOT_BAN_EXPIRE_OFFSET);

			if(ent){
				Q_strncpyz(w->warner, ent->client->pers.netname, sizeof(w->warner));
			}else{
				Q_strncpyz(w->warner, "^/SERVER CONSOLE", sizeof(w->warner));
			}
			
			for(i=0;g_shrubbot_warnings[i];i++){
				if(g_shrubbot_warnings[i]->made < oldest || oldest == 0){
					oldest = g_shrubbot_warnings[i]->made;
					structure = i;
				}
			}
			if(i==MAX_SHRUBBOT_WARNINGS){
				if(g_warningOptions.integer & WARNOP_REMOVE_OLDEST){
					g_shrubbot_warnings[structure] = w;
					_shrubbot_writeconfig();
				}else{
					SPC("^/warn: ^7MAX_SHRUBBOT_WARNINGS reached: no more warnings can be stored");
				}
			}else{
				g_shrubbot_warnings[i] = w;
				_shrubbot_writeconfig();
			}
		}
	}

	G_AddEvent(vic, EV_GENERAL_SOUND,
			G_SoundIndex("sound/misc/referee.wav"));

	AP(va("chat \"^/warn: ^7%s ^7was warned\" -1",
			vic->client->pers.netname));
	// tjw: can't do color code for the reason because the 
	//      client likes to start new lines and lose the
	//      color with long cp strings
	if(g_gamestate.integer != GS_INTERMISSION){
		CPx(pids[0], va("cp \"%s ^3warned ^7you because:\n%s\"", 
			(ent?ent->client->pers.netname:"^3SERVER CONSOLE"),
			reason));
	}else{
		CPx(pids[0], va("chat \"%s ^3warned ^7you because: %s\"", 
			(ent?ent->client->pers.netname:"^3SERVER CONSOLE"),
			reason));
	}
	return qtrue;
}

qboolean G_shrubbot_dewarn(gentity_t *ent, int skiparg) 
{
	int wnum = 0, number, warning,pids[MAX_CLIENTS];
	char numChar[5], name[MAX_NAME_LENGTH], err[MAX_STRING_CHARS];
	time_t t;
	gentity_t *vic;
	qboolean showAll = qfalse;
	
	if(!(g_warningOptions.integer & WARNOP_LINK_GUID ||
		g_warningOptions.integer & WARNOP_LINK_IP)){
		SPC("^/dewarn:^7 warning storage is not enabled on this server");
		return qfalse;
	}

	if(Q_SayArgc() == 2+skiparg){
		showAll = qtrue;
	}else if(Q_SayArgc() < 3+skiparg) {
		SPC("^/dewarn usage: ^7!dewarn [name|slot#] [warning#]");
		return qfalse;
	}

	Q_SayArgv(1+skiparg, name, sizeof(name));
	if(!showAll){
		Q_SayArgv(2+skiparg, numChar, sizeof(numChar));
		wnum = atoi(numChar);
	}

	if(ClientNumbersFromString(name, pids) != 1) {
		G_MatchOnePlayer(pids, err, sizeof(err));
		SPC(va("^/dewarn: ^7%s", err));
		return qfalse;
	}
	vic = &g_entities[pids[0]];
	if(!_shrubbot_admin_higher(ent, vic)) {
		SPC("^/dewarn: ^7sorry, but your intended victim has a higher admin"
			" level than you do");
		return qfalse;
	}
	if(_shrubbot_immutable(ent, vic)) {
		SPC("^/dewarn: ^7sorry, but your intended victim is immune to shrubbot commands");
		return qfalse;
	}
	if(!time(&t)){
		return qfalse;
	}
	if(G_shrubbot_user_tot_warnings(vic) == 0){
		SPC(va("^/dewarn: ^7no warnings found for %s",vic->client->pers.netname));
		if(showAll){
			return qtrue;
		}else{
			return qfalse;
		}
	}

	if(showAll){
		SBP_begin();
		wnum = G_shrubbot_user_tot_warnings(vic);
		SPC(va("^/dewarn:^7 %i warning%s found for %s",
			wnum,wnum == 1 ? "" : "s" ,vic->client->pers.netname));
		for(number=1;number<=wnum;number++){
			if(wnum <= 2){
				warning = G_shrubbot_user_warning(vic,number);
				if(warning != -1){
					SPC(va("^/%i.^7 %s%s", number,
						g_shrubbot_warnings[warning]->warning,
						g_shrubbot_warnings[warning]->warner[0] ? 
						va(" ^/(By: ^7%s^/)",g_shrubbot_warnings[warning]->warner):
						""));
				}
			}else{
				if(number == 1){
					SPC("^/dewarn:^7 warnings are printed in the console");
				}
				warning = G_shrubbot_user_warning(vic,number);
				if(warning != -1){
					SBP(va("^/%i.^7 %s%s\n",number,
						g_shrubbot_warnings[warning]->warning,
						g_shrubbot_warnings[warning]->warner[0] ? 
						va(" ^/(By: ^7%s^/)",g_shrubbot_warnings[warning]->warner):
						""));
				}
			}
		}
		SBP_end();
		return qtrue;
	}
				
	number = G_shrubbot_user_warning(vic, wnum);
	if(number == -1) {
		SPC("^/dewarn: ^7invalid warning #");
		return qfalse;
	}

	g_shrubbot_warnings[number]->made =
		(t - SHRUBBOT_BAN_EXPIRE_OFFSET - 3600*g_warningDecay.integer);
	SPC(va("^/dewarn: ^7warning #%d of %s ^7removed", wnum,vic->client->pers.netname));
	_shrubbot_writeconfig();
	return qtrue;
}

qboolean G_shrubbot_news(gentity_t *ent, int skiparg)
{
	char mapname[MAX_STRING_CHARS];

	if(Q_SayArgc() < 2+skiparg) {
		Q_strncpyz(mapname, level.rawmapname,
				sizeof(mapname));
	} else {
		Q_SayArgv(1+skiparg, mapname, sizeof(mapname));
	}

	G_globalSound(va("sound/vo/%s/news_%s.wav",mapname,mapname));

	return qtrue;
}

qboolean G_shrubbot_lock(gentity_t *ent, int skiparg)
{
	return G_shrubbot_lockteams(ent, skiparg, qtrue);
}

qboolean G_shrubbot_unlock(gentity_t *ent, int skiparg)
{
	return G_shrubbot_lockteams(ent, skiparg, qfalse);
}

qboolean G_shrubbot_lockteams(gentity_t *ent, int skiparg, qboolean toLock)
{
	char team[4];
	char command[MAX_SHRUBBOT_CMD_LEN], *cmd;

	Q_SayArgv(skiparg, command, sizeof(command));
	cmd = command;
	if(*cmd == '!')
		*cmd++;
	
	if(Q_SayArgc() < 2+skiparg) {
		SPC(va("^/%s usage: ^7!%s r|b|s|all",cmd,cmd));
		return qfalse;
	}
	Q_SayArgv(1+skiparg, team, sizeof(team));

	if ( !Q_stricmp(team, "all") ) {
		teamInfo[TEAM_AXIS].team_lock = 
			(TeamCount(-1, TEAM_AXIS)) ? toLock : qfalse;
		teamInfo[TEAM_ALLIES].team_lock = 
			(TeamCount(-1, TEAM_ALLIES)) ? toLock : qfalse;
		G_updateSpecLock(TEAM_AXIS, 
			(TeamCount(-1, TEAM_AXIS)) ? toLock : qfalse);
		G_updateSpecLock(TEAM_ALLIES, 
			(TeamCount(-1, TEAM_ALLIES)) ? toLock : qfalse);
		if( toLock ) {
			level.server_settings |= CV_SVS_LOCKSPECS;
		} else {
			level.server_settings &= ~CV_SVS_LOCKSPECS;
		}
		AP(va("chat \"^/%s: ^7All teams %sed\" -1", cmd, cmd));
	} else if ( !Q_stricmp(team, "r") ) {
		teamInfo[TEAM_AXIS].team_lock = 
			(TeamCount(-1, TEAM_AXIS)) ? toLock : qfalse;
		AP(va("chat \"^/%s: ^7Axis team %sed\" -1", cmd, cmd));
	} else if ( !Q_stricmp(team, "b") ) {
		teamInfo[TEAM_ALLIES].team_lock = 
			(TeamCount(-1, TEAM_ALLIES)) ? toLock : qfalse;
		AP(va("chat \"^/%s: ^7Allied team %sed\" -1", cmd, cmd));
	} else if ( !Q_stricmp(team, "s") ) {
		G_updateSpecLock(TEAM_AXIS, 
			(TeamCount(-1, TEAM_AXIS)) ? toLock : qfalse);
		G_updateSpecLock(TEAM_ALLIES, 
			(TeamCount(-1, TEAM_ALLIES)) ? toLock : qfalse);
		if( toLock ) {
			level.server_settings |= CV_SVS_LOCKSPECS;
		} else {
			level.server_settings &= ~CV_SVS_LOCKSPECS;
		}
		AP(va("chat \"^/%s: ^7Spectators %sed\" -1", cmd, cmd));
	} else {
		SPC(va("^/%s usage: ^7!%s r|b|s|all",cmd,cmd));
		return qfalse;
	}

	if( toLock ) {
		level.server_settings |= CV_SVS_LOCKTEAMS;
	} else {
		level.server_settings &= ~CV_SVS_LOCKTEAMS;
	}
	trap_SetConfigstring(CS_SERVERTOGGLES, 
			va("%d", level.server_settings));

	return qtrue;
}

// created by: dvl
qboolean G_shrubbot_lol(gentity_t *ent, int skiparg)
{
	int pids[MAX_CLIENTS];
	char name[MAX_NAME_LENGTH], err[MAX_STRING_CHARS], numNades[4];
	gentity_t *vic;
	qboolean doAll = qfalse;
	int pcount, nades = 0, count = 0;

	if(Q_SayArgc() < 2+skiparg) doAll = qtrue;
	else if(Q_SayArgc() >= 3+skiparg) {
		Q_SayArgv( 2+skiparg, numNades, sizeof( numNades ) );
		nades = atoi( numNades );
		if( nades < 1 ) 
			nades = 1;
		else if( nades > SHRUBBOT_MAX_LOL_NADES ) 
			nades = SHRUBBOT_MAX_LOL_NADES;
	}
	Q_SayArgv( 1+skiparg, name, sizeof( name ) );
	if( !Q_stricmp( name, "-1" ) || doAll ) {
		int it;
		for( it = 0; it < level.numConnectedClients; it++ ) {
			vic = g_entities + level.sortedClients[it];
			if( !_shrubbot_admin_higher(ent, vic) ||
				_shrubbot_immutable(ent, vic) ||
				!(vic->client->sess.sessionTeam == TEAM_AXIS ||
				  vic->client->sess.sessionTeam == TEAM_ALLIES))
				continue;
			if( nades )
				G_createClusterNade( vic, nades );
			else
				G_createClusterNade( vic, 1 );
			count++;
		}
		AP(va("chat \"^/lol: ^7%d players lol\'d\" -1", count));
		return qtrue;
	} 
	pcount = ClientNumbersFromString(name, pids);
	if(pcount > 1) {
		int it;
		for( it = 0; it < pcount; it++) {
			vic = &g_entities[pids[it]];
			if(!_shrubbot_admin_higher(ent, vic)) {
				SPC(va("^/lol: ^7sorry, but %s^7 has a higher "
					"admin level than you do",
					vic->client->pers.netname));
				continue;
			}
			if(_shrubbot_immutable(ent, vic)) {
				SPC("^/lol: ^7sorry, but your intended victim is immune to shrubbot commands");
				continue;
			}
			if(!(vic->client->sess.sessionTeam == TEAM_AXIS ||
					vic->client->sess.sessionTeam == TEAM_ALLIES)) {
				SPC(va("^/lol: ^7%s^7 must be on a "
						"team to be lol\'d", 
						vic->client->pers.netname));
				continue;
			}
			if( nades )
				G_createClusterNade( vic, nades );
			else
				G_createClusterNade( vic, 8 );
			AP(va("chat \"^/lol: ^7%s\" -1", 
					vic->client->pers.netname));
		}
		return qtrue;
	} else if( pcount < 1 ) {
		G_MatchOnePlayer(pids, err, sizeof(err));
		SPC(va("^/lol: ^7%s", err));
		return qfalse;
	}
	vic = &g_entities[pids[0]];
	if(!_shrubbot_admin_higher(ent, vic)) {
		SPC("^/lol: ^7sorry, but your intended victim has a higher "
			"admin level than you do");
		return qfalse;
	}
	if(_shrubbot_immutable(ent, vic)) {
        SPC("^/lol: ^7sorry, but your intended victim is immune to shrubbot commands");
        return qfalse;
    }
	if(!(vic->client->sess.sessionTeam == TEAM_AXIS ||
			vic->client->sess.sessionTeam == TEAM_ALLIES)) {
		SPC("^/lol: ^7player must be on a team to be lol'd");
		return qfalse;
	}
	
	if( nades )
		G_createClusterNade( vic, nades );
	else
		G_createClusterNade( vic, 8 );
	AP(va("chat \"^/lol: ^7%s\" -1", vic->client->pers.netname));
	return qtrue;

}
													
// Created by: dvl
qboolean G_shrubbot_pop( gentity_t *ent, int skiparg )
{
	vec3_t dir = { 5, 5, 5 };
	int pids[MAX_CLIENTS];
	char name[MAX_NAME_LENGTH], err[MAX_STRING_CHARS];
	gentity_t *vic;
	qboolean doAll = qfalse;
	int pcount, count = 0;

	if(Q_SayArgc() < 2+skiparg)
		doAll = qtrue;
	Q_SayArgv( 1+skiparg, name, sizeof( name ) );
	if( !Q_stricmp( name, "-1" ) || doAll ) {
		int it;
		for( it = 0; it < level.numConnectedClients; it++ ) {
			vic = g_entities + level.sortedClients[it];
			if(!_shrubbot_admin_higher( ent, vic ) ||
				_shrubbot_immutable(ent, vic) ||
				!(vic->client->sess.sessionTeam == TEAM_AXIS ||
				  vic->client->sess.sessionTeam == TEAM_ALLIES) ||
				 (vic->client->ps.eFlags & EF_HEADSHOT))
				continue;
			// tjw: actually take of the players helmet instead of
			//      giving all players an unlimited supply of 
			//      helmets.
			vic->client->ps.eFlags |= EF_HEADSHOT;
			G_AddEvent( vic, EV_LOSE_HAT, DirToByte(dir) );
			count++;
		}
		AP(va("chat \"^/pop: ^7%d players popped\" -1", count));
		return qtrue;
	}
	pcount = ClientNumbersFromString(name, pids);
	if(pcount > 1) {
		int it;
		for( it = 0; it < pcount; it++) {
			vic = &g_entities[pids[it]];
			if(!_shrubbot_admin_higher(ent, vic)) {
				SPC( va("^/pop: ^7sorry, but %s^7 intended "
					"victim has a higher admin"
					" level than you do",
					vic->client->pers.netname));
				continue;
			}
			if(_shrubbot_immutable(ent, vic)) {
				SPC("^/pop: ^7sorry, but your intended victim is immune to shrubbot "
					"commands");
				return qfalse;
			}
			if(!(vic->client->sess.sessionTeam == TEAM_AXIS ||
		 		vic->client->sess.sessionTeam == TEAM_ALLIES)) {
				SPC(va("^/pop: ^7%s^7 must be on a team"
					"to be popped",
					vic->client->pers.netname));
				continue;
			}
			G_AddEvent( vic, EV_LOSE_HAT, DirToByte(dir) );
			AP(va("chat \"^/pop: ^7%s ^7lost his hat\" -1", 
				vic->client->pers.netname));
		}
		return qtrue;
	} else if(pcount < 1) {
		G_MatchOnePlayer(pids, err, sizeof(err));
		SPC(va("^/pop: ^7%s", err));
		return qfalse;
	}
	vic = &g_entities[pids[0]];
	if(!_shrubbot_admin_higher(ent, vic)) {
		SPC( "^/pop: ^7sorry, but your intended victim has a higher "
			"admin level than you do");
		return qfalse;
	}
	if(!(vic->client->sess.sessionTeam == TEAM_AXIS ||
		 vic->client->sess.sessionTeam == TEAM_ALLIES)) {
		SPC("^/pop: ^7player must be on a team to be popped");
		return qfalse;
	}
	
	G_AddEvent( vic, EV_LOSE_HAT, DirToByte(dir) );
	AP(va("chat \"^/pop: ^7%s ^7lost his hat\" -1", 
			vic->client->pers.netname));
	return qtrue;
}

// Created by: dvl
qboolean G_shrubbot_pip( gentity_t *ent, int skiparg )
{
	int pids[MAX_CLIENTS];
	char name[MAX_NAME_LENGTH], err[MAX_STRING_CHARS];
	gentity_t *vic;
	qboolean doAll = qfalse;
	int pcount, count = 0;

	if(Q_SayArgc() < 2+skiparg)
		doAll = qtrue;
	Q_SayArgv( 1+skiparg, name, sizeof( name ) );
	if( !Q_stricmp( name, "-1" ) || doAll ) {
		int it;
		for( it = 0; it < level.numConnectedClients; it++ ) {
			vic = g_entities + level.sortedClients[it];
			if(!_shrubbot_admin_higher( ent, vic ) ||
				_shrubbot_immutable(ent, vic) ||
				!(vic->client->sess.sessionTeam == TEAM_AXIS ||
				  vic->client->sess.sessionTeam == TEAM_ALLIES))
				continue;
			G_MakePip( vic );
			count++;
		}
		AP(va("chat \"^/pip: ^7%d players pipped\" -1", count));
		return qtrue;
	}
	pcount = ClientNumbersFromString(name, pids);
	if(pcount > 1) {
		int it;
		for( it = 0; it < pcount; it++) {
			vic = &g_entities[pids[it]];
			if(!_shrubbot_admin_higher(ent, vic)) {
				SPC( va("^/pip: ^7sorry, but %s^7 intended "
					"victim has a higher admin"
					"level than you do",
					vic->client->pers.netname));
				continue;
			}
			if(_shrubbot_immutable(ent, vic)) {
				SPC("^/pip: ^7sorry, but your intended victim is immune to shrubbot "
					"commands");
				continue;
			}
			if(!(vic->client->sess.sessionTeam == TEAM_AXIS ||
		 		vic->client->sess.sessionTeam == TEAM_ALLIES)) {
				SPC(va("^/pip: ^7%s^7 must be on a team"
					" to be pipped", 
					vic->client->pers.netname));
				continue;
			}
			G_MakePip( vic );
			AP(va("chat \"^/pip: ^7%s ^7was pipped\" -1", 
					vic->client->pers.netname));
		}
		return qtrue;
	} else if(pcount < 1) {
		G_MatchOnePlayer(pids, err, sizeof(err));
		SPC(va("^/pip: ^7%s", err));
		return qfalse;
	}
	vic = &g_entities[pids[0]];
	if(!_shrubbot_admin_higher(ent, &g_entities[pids[0]])) {
		SPC("^/pip: ^7sorry, but your intended victim has a higher"
			" admin level than you do");
		return qfalse;
	}
	if(!(vic->client->sess.sessionTeam == TEAM_AXIS ||
			vic->client->sess.sessionTeam == TEAM_ALLIES)) {
		SPC("^/pip: ^7player must be on a team to be pipped");
		return qfalse;
	}
	
	G_MakePip( vic );
	AP(va("chat \"^/pip: ^7%s ^7was pipped\" -1", vic->client->pers.netname));
	return qtrue;
}

qboolean G_shrubbot_reset(gentity_t *ent, int skiparg)
{
	char command[MAX_SHRUBBOT_CMD_LEN];

	Q_SayArgv(skiparg, command, sizeof(command));
	if(Q_stricmp(command, "reset")) {
		Svcmd_ResetMatch_f(qtrue, qtrue);
	}
	else {
		Svcmd_ResetMatch_f(qfalse, qtrue);
	}
	return qtrue;
}

qboolean G_shrubbot_swap(gentity_t *ent, int skiparg)
{
	Svcmd_SwapTeams_f();
	AP("chat \"^/swap: ^7Teams were swapped\" -1");
	return qtrue;
}

qboolean G_shrubbot_fling(gentity_t *ent, int skiparg)
{
	int pids[MAX_CLIENTS];
	char name[MAX_NAME_LENGTH], err[MAX_STRING_CHARS];
	char fling[9], pastTense[9];
	char *flingCmd;
	gentity_t *vic;
	int flingType=-1; // 0 = fling, 1 = throw, 2 = launch
	int i, count = 0, pcount;
	qboolean doAll = qfalse;

	Q_SayArgv(0+skiparg, fling, sizeof(fling));
	if(fling[0] == '!') {
		flingCmd = &fling[1];
	} else {
		flingCmd = &fling[0];
	}

	if(!Q_stricmp(flingCmd, "throw")) {
		flingType = 1;
		Q_strncpyz(pastTense, "thrown", sizeof(pastTense));
	} else if (!Q_stricmp(flingCmd, "launch")) {
		flingType = 2;
		Q_strncpyz(pastTense, "launched", sizeof(pastTense));
	} else if (!Q_stricmp(flingCmd, "fling")) {
		flingType = 0;
		Q_strncpyz(pastTense, "flung", sizeof(pastTense));
	} else if (!Q_stricmp(flingCmd, "throwa")) {
		doAll = qtrue;
		flingType = 1;
		Q_strncpyz(pastTense, "thrown", sizeof(pastTense));
	} else if (!Q_stricmp(flingCmd, "launcha")) {
		doAll = qtrue;
		flingType = 2;
		Q_strncpyz(pastTense, "launched", sizeof(pastTense));
	} else if (!Q_stricmp(flingCmd, "flinga")) {
		doAll = qtrue;
		flingType = 0;
		Q_strncpyz(pastTense, "flung", sizeof(pastTense));
	}

	if( !doAll && Q_SayArgc() < 2+skiparg) {
		SPC(va("^/%s usage: ^7!%s [name|slot#]", flingCmd, flingCmd));
		return qfalse;
	}
	Q_SayArgv(1+skiparg, name, sizeof(name)); 

	if( doAll ) {
		for( i = 0; i < level.numConnectedClients; i++ ) {
			vic = g_entities + level.sortedClients[i];
			if( !_shrubbot_admin_higher( ent, vic ) ||
				_shrubbot_immutable(ent, vic))
				continue;
			count += G_FlingClient( vic, flingType );
		}
		AP(va("chat \"^/%s: ^7%d players %s\" -1", 
					flingCmd, count, pastTense));
		return qtrue;
	}
	pcount = ClientNumbersFromString(name, pids);
	if(pcount > 1) {
		for( i = 0; i < pcount; i++) {
			vic = &g_entities[pids[i]];
			if( !_shrubbot_admin_higher( ent, vic ) ||
				_shrubbot_immutable(ent, vic))
				continue;
			count += G_FlingClient( vic, flingType );
		}
		AP(va("chat \"^/%s: ^7%d players %s\" -1", 
					flingCmd, count, pastTense));
		return qtrue;
	} else if(pcount < 1) {
		G_MatchOnePlayer(pids, err, sizeof(err));
		SPC(va("^/%s: ^7%s", flingCmd, err));
		return qfalse;
	}
	vic = &g_entities[pids[0]];
	if(!_shrubbot_admin_higher(ent, vic)) {
		SPC(va("^/%s: ^7sorry, but your intended victim has a higher"
			" admin level than you do",flingCmd));
		return qfalse;
	}
	if(_shrubbot_immutable(ent, vic)) {
        SPC(va("^/%s: ^7sorry, but your intended victim is immune to shrubbot"
			" commands",flingCmd));
        return qfalse;
    }
	
	if(G_FlingClient( vic, flingType ))
		AP(va("chat \"^/%s: ^7%s ^7was %s\" -1", 
				flingCmd,
				vic->client->pers.netname, 
				pastTense));
	return qtrue;
}


qboolean G_shrubbot_disorient(gentity_t *ent, int skiparg)
{
	int pids[MAX_CLIENTS];
	char name[MAX_NAME_LENGTH], err[MAX_STRING_CHARS];
	char *reason;
	gentity_t *vic;

	if(Q_SayArgc() < 2+skiparg) {
		SPC("^/disorient usage: ^7!disorient [name|slot#] [reason]");
		return qfalse;
	}
	Q_SayArgv(1+skiparg, name, sizeof(name));
	reason = Q_SayConcatArgs(2+skiparg);

	if(ClientNumbersFromString(name, pids) != 1) {
		G_MatchOnePlayer(pids, err, sizeof(err));
		SPC(va("^/disorient: ^7%s", err));
		return qfalse;
	}
	vic = &g_entities[pids[0]];
	if(!_shrubbot_admin_higher(ent, vic)) {
		SPC("^/disorient: ^7sorry, but your intended victim has a"
			"higher admin level than you do");
		return qfalse;
	}
	if(_shrubbot_immutable(ent, vic)) {
        SPC("^/disorient: ^7sorry, but your intended victim is immune to shrubbot commands");
        return qfalse;
    }
	if(!(vic->client->sess.sessionTeam == TEAM_AXIS ||
			vic->client->sess.sessionTeam == TEAM_ALLIES)) {
		SPC("^/disorient: ^7player must be on a team");
		return qfalse;
	}
	if(vic->client->pmext.disoriented) {
		SPC(va("^/disorient: ^7%s^7 is already disoriented",
			vic->client->pers.netname));
		return qfalse;
	}
	vic->client->pmext.disoriented = qtrue;
	AP(va("chat \"^/disorient: ^7%s ^7is disoriented\" -1",
			vic->client->pers.netname));

	CPx(pids[0], va("cp \"%s ^7disoriented you%s%s\"", 
			(ent?ent->client->pers.netname:"^3SERVER CONSOLE"),
			(*reason) ? " because:\n" : "",
			(*reason) ? reason : ""));
	return qtrue;
}

qboolean G_shrubbot_orient(gentity_t *ent, int skiparg)
{
	int pids[MAX_CLIENTS];
	char name[MAX_NAME_LENGTH], err[MAX_STRING_CHARS];
	gentity_t *vic;

	if(Q_SayArgc() < 2+skiparg) {
		SPC("^/orient usage: ^7!orient [name|slot#]");
		return qfalse;
	}
	Q_SayArgv(1+skiparg, name, sizeof(name));

	if(ClientNumbersFromString(name, pids) != 1) {
		G_MatchOnePlayer(pids, err, sizeof(err));
		SPC(va("^/orient: ^7%s", err));
		return qfalse;
	}
	vic = &g_entities[pids[0]];

	if(!vic->client->pmext.disoriented) {
		SPC(va("^/orient: ^7%s^7 is not currently disoriented",
			vic->client->pers.netname));
		return qfalse;
	}
	if(vic->client->pmext.poisoned) {
		SPC(va("^/orient: ^7%s^7 is poisoned at the moment",
			vic->client->pers.netname));
		return qfalse;
	}
	vic->client->pmext.disoriented = qfalse;
	AP(va("chat \"^/orient: ^7%s ^7is no longer disoriented\" -1",
			vic->client->pers.netname));

	CPx(pids[0], va("cp \"%s ^7oriented you\"", 
			(ent?ent->client->pers.netname:"^3SERVER CONSOLE")));
	return qtrue;
}

qboolean G_shrubbot_resetxp(gentity_t *ent, int skiparg) 
{
	int pids[MAX_CLIENTS];
	char name[MAX_NAME_LENGTH], err[MAX_STRING_CHARS];
	char *reason;
	gentity_t *vic;


	if(Q_SayArgc() < 2+skiparg) {
		SPC("^/resetxp usage: ^7!resetxp [name|slot#] [reason]");
		return qfalse;
	}
	Q_SayArgv(1+skiparg, name, sizeof(name));
	reason = ConcatArgs(2+skiparg);

	if(ClientNumbersFromString(name, pids) != 1) {
		G_MatchOnePlayer(pids, err, sizeof(err));
		SPC(va("^/resetxp: ^7%s", err));
		return qfalse;
	}
	vic = &g_entities[pids[0]];
	if(!_shrubbot_admin_higher(ent, vic)) {
		SPC("^/resetxp: ^7sorry, but your intended victim has a higher "
			" admin level than you do");
		return qfalse;
	}
	if(_shrubbot_immutable(ent, vic)) {
		SPC("^/resetxp: ^7sorry, but your intended victim is immune to shrubbot commands");
		return qfalse;
	}
	G_ResetXP(vic);

	AP(va("chat \"^/resetxp: ^7XP has been reset for player %s\" -1",
			vic->client->pers.netname));

	if(ent && (ent - g_entities) == pids[0])
		return qtrue;

	CPx(pids[0], va("cp \"%s^7 has reset your XP %s%s\"", 
			(ent?ent->client->pers.netname:"^3SERVER CONSOLE"),
			(*reason) ? " because:\n" : "",
			(*reason) ? reason : ""));
	return qtrue;
}

qboolean G_shrubbot_nextmap(gentity_t *ent, int skiparg)
{
	// copied from G_Nextmap_v() in g_vote.c
	int i = 0;
	g_campaignInfo_t *campaign;

	// Chaos: Write xp before going to the next map
	if(!level.intermissiontime){
		if(g_XPSave.integer &  XPSF_STORE_AT_RESTART){
			for( i = 0; i < level.numConnectedClients; i++ ) {
				G_xpsave_add(&g_entities[level.sortedClients[i]],qfalse);
			}
			if(g_spreeOptions.integer & SPREE_SAVE_RECORD_RESTART){
				G_AddSpreeRecord();
			}
			G_xpsave_writeconfig();
		}else if(g_spreeOptions.integer & SPREE_SAVE_RECORD_RESTART){
			// Dens: we don't want to store the xp of the disconnected people
			// so we can use te xpsave structures anymore. Re-read the xp from
			// map start. Takes some recources, but at the moment the only way
			G_xpsave_readconfig();
			G_AddSpreeRecord();
			G_xpsave_writeconfig();
		}	
	}
	G_reset_disconnects();

	if( g_gametype.integer == GT_WOLF_CAMPAIGN ) {
		campaign = &g_campaigns[level.currentCampaign];
		if( campaign->current + 1 < campaign->mapCount ) {
			trap_Cvar_Set("g_currentCampaignMap",
					va( "%i", campaign->current + 1));
			trap_SendConsoleCommand(EXEC_APPEND,
				va( "map %s\n",
				campaign->mapnames[campaign->current + 1]));
			AP("cp \"^3*** Loading next map in campaign! ***\n\"");
		} else {
			// Load in the nextcampaign
			trap_SendConsoleCommand(EXEC_APPEND, 
					"vstr nextcampaign\n");
			AP("cp \"^3*** Loading nextcampaign! ***\n\"");
		}
	} else {
		// Load in the nextmap
		trap_SendConsoleCommand(EXEC_APPEND, "vstr nextmap\n");
		AP("cp \"^3*** Loading nextmap! ***\n\"");
	}
	AP("chat \"^/nextmap: ^7Next map was loaded\" -1");
	return qtrue;
}

qboolean G_shrubbot_resetmyxp(gentity_t *ent, int skiparg) 
{
	int chargebar;

	if(!ent || !ent->client)
		return qfalse;
	// forty - save the chargebar
	chargebar = ent->client->ps.classWeaponTime;
	G_ResetXP(ent);
	SPC("^/resetmyxp: ^7you have reset your XP");
	// forty - restore it
	ent->client->ps.classWeaponTime = chargebar;
	return qtrue;
}

qboolean G_shrubbot_userinfo(gentity_t *ent, int skiparg)
{
	int	pids[MAX_CLIENTS],i;
	char name[MAX_NAME_LENGTH], err[MAX_STRING_CHARS];
	gentity_t *vic;
	char guid[33];
	char *temp;
	char userinfo[MAX_INFO_STRING];
	char guid_short[9];

	if(Q_SayArgc() < 2+skiparg) {
		SPC("^/userinfo usage: ^7!userinfo [name|slot#]");
		return qfalse;
	}
	Q_SayArgv(1+skiparg, name, sizeof(name)); 

	if(ClientNumbersFromString(name, pids) != 1) {
		G_MatchOnePlayer(pids, err, sizeof(err));
		SPC(va("^/userinfo: ^7%s", err));
		return qfalse;
	}
	vic = &g_entities[pids[0]];
	if(!_shrubbot_admin_higher(ent, vic)) {
		SPC("^/userinfo: ^7sorry, but your intended victim has a higher admin"
			" level than you do");
		return qfalse;
	}
	if(_shrubbot_immutable(ent, vic)) {
		SPC("^/userinfo: ^7sorry, but your intended victim is immune to shrubbot commands");
		return qfalse;
	}

	SPC(va("^/Userinfo of user ^7%s", vic->client->pers.netname));
	SPC(va("^/Slot Number: ^d%i    ^/ETPub Client: ^d%s",vic-g_entities, (vic->r.svFlags & SVF_BOT)?
		"Bot" : (vic->client->pers.etpubc > 0)? va("%i",vic->client->pers.etpubc):"No"));
	
	trap_GetUserinfo( vic-g_entities, userinfo, sizeof( userinfo ) );
	temp = Info_ValueForKey(userinfo, "cl_guid");
	Q_strncpyz(guid, temp, sizeof(guid));
	
	if(!Q_stricmp(vic->client->sess.guid, guid)){
		for(i=0; i<=8; i++){
			guid_short[i] = guid[i+24] ? guid[i+24] : '\0';
		}
		SPC(va("^/GUID: ^d%s",guid_short ? guid_short : "Unknown"));
	}else{
		for(i=0; i<=8; i++){
			guid_short[i] = vic->client->sess.guid[i+24] ?
				vic->client->sess.guid[i+24] : '\0';
		}
		SPC(va("^1GUID MISMATCH! ^/Stored GUID: ^d%s", guid_short ?
			guid_short : "Unknown"));

		for(i=0; i<=8; i++){
			guid_short[i] = guid[i+24] ? guid[i+24] : '\0';
		}
		SPC( va("^/Current GUID: ^d%s", guid_short ? guid_short : "Unknown"));
	}

	temp = Info_ValueForKey(userinfo, "ip");
	
	if(!Q_stricmp(vic->client->sess.ip, temp)){
		SPC(va("^/IP: ^d%s",temp ? temp : "Unknown"));
	}else{
		SPC(va("^1IP MISMATCH! ^/Stored IP: ^d%s", vic->client->sess.ip ?
			vic->client->sess.ip : "Unknown"));
		SPC( va("^/Current IP: ^d%s", temp ? temp : "Unknown"));
	}

	return qtrue;
}

qboolean G_shrubbot_stats(gentity_t *ent, int skiparg) 
{
	int i, length = 4, spaces = 0;
	gclient_t *p;
	char fmt[MAX_STRING_CHARS];
	char tmp[MAX_NAME_LENGTH];
	char name[MAX_NAME_LENGTH];
	char name_fmt[5];
	int shots, hits, headshots;
	float accuracy, hsratio, distance;
	char colorAcc = '2';
	char colorHR = '2';

	SBP_begin();
	SBP("^fstats: showing ^5Thompson ^fand ^5MP40 ^fstats of all connected players\n");
	
	// Dens: first find the longest name
	for(i=0; i < level.maxclients; i++) {
		p = &level.clients[i];
		if(p->pers.connected == CON_CONNECTING ||
			p->pers.connected != CON_CONNECTED) {
			continue;
		}
	
		DecolorString(p->pers.netname, tmp);
		if(strlen(tmp) > length){
			length = strlen(tmp);
		}
	}
	
	Com_sprintf(fmt, sizeof(fmt),
		"^5%%%is SHOTS  HITS   ACC  HEAD    HR    DIST\n",
		length);
	
	SBP(va(fmt, "NAME"));
	
	for(i=0; i < level.maxclients; i++) {
		p = &level.clients[i];
		if(p->pers.connected == CON_CONNECTING ||
			p->pers.connected != CON_CONNECTED) {
		continue;
		}

		shots = (p->sess.aWeaponStats[WS_MP40].atts + p->sess.aWeaponStats[WS_THOMPSON].atts);
		hits = (p->sess.aWeaponStats[WS_MP40].hits + p->sess.aWeaponStats[WS_THOMPSON].hits);
		headshots = (p->sess.aWeaponStats[WS_MP40].headshots + p->sess.aWeaponStats[WS_THOMPSON].headshots);

		// Dens: Calculate accuracy only when there are some shots, and no negative hits (you never know)
		accuracy = 0.0;

		if(shots > 0 && hits >= 0){
			accuracy = (float) (hits * 100.0 / shots);
		}

		// Dens: Calculate HR only when there are some hits
		hsratio = 0.0;

		if(hits > 0 && headshots >= 0){
			hsratio = (float) (headshots * 100.0 / hits);
		}
		// Dens: because of g_stats cvar you can get really strange values
		// we don't want them to change the table
		if(accuracy > 999.9){
			accuracy = 999.9;
		}
		if(hsratio > 999.9){
			hsratio = 999.9;
		}

		// Dens: calculate average headshot distance
		distance = 0.0;
		if(headshots > 0){
			distance = (float) (p->pers.headshotDistance / headshots);
		}
		
		// Dens: Now check if we have a good player (or a cheater of course)
		if(accuracy >= 40.0){
			colorAcc = '1';
		}else if(accuracy >= 30.0){
			colorAcc = '3';
		}else{
			colorAcc = '2';
		}

		if(hsratio >= 12.5){
			colorHR = '1';
		}else if(hsratio >= 7.5){
			colorHR = '3';
		}else{
			colorHR = '2';
		}

		DecolorString(p->pers.netname, tmp);
		spaces = length - strlen(tmp);
		Com_sprintf(name_fmt, sizeof(name_fmt), "%%%is", spaces + strlen(p->pers.netname));
		Com_sprintf(name, sizeof(name), name_fmt, p->pers.netname);
		
		SBP(va("^7%s ^2%5i %5i ^%c%5.1f ^2%5i ^%c%5.1f ^2%7.1f\n",
			name,
			shots,
			hits,
			colorAcc,
			accuracy,
			headshots,
			colorHR,
			hsratio,
			distance));
	}
	SBP_end();
	return qtrue;
}

qboolean G_shrubbot_spreerecord(gentity_t *ent, int skiparg)
{
	G_ShowSpreeRecord(qtrue);
	return qtrue;
}

/*
 * This function facilitates the SP define.  SP() is similar to CP except that
 * it prints the message to the server console if ent is not defined.
 */
void G_shrubbot_print(gentity_t *ent, char *m) 
{

	if(ent) CP(va("print \"%s\"", m));
	else {
		char m2[MAX_STRING_CHARS];
		DecolorString(m, m2);
		G_Printf(m2);
	}
}

/* Dens: (SPC)
Exactly the same as G_shrubbot_print, but this time the text is printed in the
chat and console area of the client instead of console only. (If you want to
switch from SP to SPC, you need to remove the \n at the end, otherwise the text
isn't displayed right).
*/
void G_shrubbot_print_chat(gentity_t *ent, char *m) 
{
	char temp[MAX_STRING_CHARS], *p, *s;

	if(ent){
		Q_strncpyz(temp, m ,sizeof(temp));
		s = p = temp;
		while(*p) {
			if(*p == '\n') {
				*p++ = '\0';
				CP(va("chat \"%s\" -1", s));
				s = p;
			} else {
				p++; 
			}
		}
		CP(va("chat \"%s\" -1", s));
	}else {
		char m2[MAX_STRING_CHARS];
		DecolorString(m, m2);
		G_Printf(va("%s\n",m2));
	}
}

void G_shrubbot_buffer_begin() 
{
	bigTextBuffer[0] = '\0';
}

void G_shrubbot_buffer_end(gentity_t *ent) 
{
	SP(bigTextBuffer);	
}

void G_shrubbot_buffer_print(gentity_t *ent, char *m)
{
	// gabriel: Slightly different processing for console prints (engine does
	// not like to send huge rcon replies)
	if (!ent) {
		// Remove color from console reply string (instead of waiting for
		// G_shrubbot_print() to do it) to spread console responses over as few
		// print replies as possible
		char m2[MAX_STRING_CHARS];
		DecolorString(m, m2);

		// We use a "magic number" for console prints.  If someone can get an
		// exact number for the max char limit that can be sent via rcon print
		// reply, we would be grateful :)
		if(strlen(m2) + strlen(bigTextBuffer) > 239) {
			SP(bigTextBuffer);
			bigTextBuffer[0] = '\0';
		}
		Q_strcat(bigTextBuffer, sizeof(bigTextBuffer), m2);
	} else {
		// tjw: 1022 - strlen("print 64 \"\"") - 1 
		if(strlen(m) + strlen(bigTextBuffer) >= 1009) {
			SP(bigTextBuffer);
			bigTextBuffer[0] = '\0';
		}
		Q_strcat(bigTextBuffer, sizeof(bigTextBuffer), m);
	}
}


void G_shrubbot_cleanup()
{
	int i = 0;

	for(i=0; g_shrubbot_levels[i]; i++) {
		free(g_shrubbot_levels[i]);
		g_shrubbot_levels[i] = NULL;
	}
	for(i=0; g_shrubbot_admins[i]; i++) {
		free(g_shrubbot_admins[i]);
		g_shrubbot_admins[i] = NULL;
	}
	for(i=0; g_shrubbot_bans[i]; i++) {
		free(g_shrubbot_bans[i]);
		g_shrubbot_bans[i] = NULL;
	}
	for(i=0; g_shrubbot_commands[i]; i++) {
		free(g_shrubbot_commands[i]);
		g_shrubbot_commands[i] = NULL;
	}
	for(i=0; g_shrubbot_warnings[i]; i++) {
		free(g_shrubbot_warnings[i]);
		g_shrubbot_warnings[i] = NULL;
	}
}
