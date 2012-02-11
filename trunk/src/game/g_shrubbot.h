#ifndef _G_SHRUBBOT_H
#define _G_SHRUBBOT_H

#define MAX_SHRUBBOT_LEVELS 32
// tjw: some nutters !setlevel every player in sight
#define MAX_SHRUBBOT_ADMINS 32768
#define MAX_SHRUBBOT_BANS 1024
#define MAX_SHRUBBOT_FLAGS 64
#define MAX_SHRUBBOT_COMMANDS 64
#define MAX_SHRUBBOT_WARNINGS 1024

#define MAX_SHRUBBOT_CMD_LEN 17

/*
 * flags by redeye
 * spree, tspree E
 *
 * 1 - cannot be vote kicked, vote muted, dropped
 * due to high team damage ratio, or complained against
 * 2 - cannot be censored or flood protected
 * 3 - can run commands "silently" through the console (/!commandname
 * parameters)
 * 4 - can see Axis' and Allies' team chat as a spectator
 * 5 - can switch teams any time, regardless of balance
 * 6 - does not need to specify a reason for a kick/ban
 * 7 - can call a vote at any time (regardless of a vote being disabled or
 * voting limitations)
 * 8 - does not need to specify a duration for a ban
 * 9 - can run commands from team and fireteam chat (as opposed to global chat)
 * 0 - inactivity rules do not apply to them
 * ! - shrubbot commands cannot be used on them
 * @ - does not show up as an admin in !listplayers
 * $ - can do !admintest <other player>
 * ~ - can read and send adminchat
 */
#define SBF_IMMUNITY '1'
#define SBF_NOCENSORFLOOD '2'
#define SBF_STEALTH '3'
#define SBF_SPEC_ALLCHAT '4'
#define SBF_FORCETEAMCHANGE '5'
#define SBF_UNACCOUNTABLE '6'
#define SBF_NO_VOTE_LIMIT '7'
#define SBF_CAN_PERM_BAN '8'
#define SBF_TEAMFTSHRUBCMD '9'
#define SBF_ACTIVITY '0'
#define SBF_IMMUTABLE '!'
#define SBF_INCOGNITO '@'
#define SBF_ADMINTEST_OTHER '$'
#define SBF_ADMINCHAT '~'
// pheno: can rename himself regardless of g_maxNameChanges limit
#define SBF_NO_RENAME_LIMIT '&'

// tjw: used by g_shrubbot_cmd.cmdFlags
#define SCMDF_TYRANNY 1

// expires field seems to be offset by (30 years - 54 hours)...  go figure
#define SHRUBBOT_BAN_EXPIRE_OFFSET 946490400
#define SHRUBBOT_MAX_LOL_NADES 16
#define SHRUBBOT_MAX_SHOWBANS 30
#define SHRUBBOT_KICK_LENGTH 120

struct g_shrubbot_cmd {
	const char *keyword;
	qboolean (* const handler)(gentity_t *ent, int skiparg);
	char flag;
	int cmdFlags; // see SCMDF_ defines
	const char *function;		// used for !help
	const char *syntax;		// used for !help
};

typedef struct g_shrubbot_level {
	int level;
	char name[MAX_NAME_LENGTH];
	char flags[MAX_SHRUBBOT_FLAGS];
	char greeting[MAX_STRING_CHARS];
	char greeting_sound[MAX_STRING_CHARS];
} g_shrubbot_level_t;

typedef struct g_shrubbot_admin {
	char guid[PB_GUID_LENGTH + 1];
	char name[MAX_NAME_LENGTH];
	int level;
	char flags[MAX_SHRUBBOT_FLAGS];
	char greeting[MAX_STRING_CHARS];
	char greeting_sound[MAX_STRING_CHARS];
} g_shrubbot_admin_t;

typedef struct g_shrubbot_ban {
	char name[MAX_NAME_LENGTH];
	char guid[PB_GUID_LENGTH + 1];
	char ip[18];
	char mac[18];
	char reason[MAX_STRING_CHARS];
	char made[50]; // big enough for strftime() %c
	int expires;
	char banner[MAX_NAME_LENGTH];
} g_shrubbot_ban_t;

typedef struct g_shrubbot_command {
	char command[64];
	char exec[MAX_STRING_CHARS];
	char desc[MAX_STRING_CHARS];
	int levels[MAX_SHRUBBOT_LEVELS+1];
} g_shrubbot_command_t;

typedef struct g_shrubbot_warning {
	char name[MAX_NAME_LENGTH];
	char guid[PB_GUID_LENGTH + 1];
	char ip[18];
	char warning[MAX_STRING_CHARS];
	int made;
	char warner[MAX_NAME_LENGTH];
} g_shrubbot_warning_t;

#define WARPROTO(TYPE) \
	qboolean G_shrubbot_ ## TYPE ## war(gentity_t *ent, int skiparg);

qboolean G_shrubbot_tempban(int clientnum, char *reason, int length);
qboolean G_shrubbot_ban_check(char *userinfo, char *reason);
qboolean G_shrubbot_levelconnect_check(char *userinfo, char *reason);
void G_shrubbot_greeting(gentity_t *ent);
qboolean G_shrubbot_cmd_check(gentity_t *ent);
qboolean G_shrubbot_readconfig(gentity_t *ent, int skiparg);
qboolean G_shrubbot_time(gentity_t *ent, int skiparg);
qboolean G_shrubbot_setlevel(gentity_t *ent, int skiparg);
qboolean G_shrubbot_kick(gentity_t *ent, int skiparg);
qboolean G_shrubbot_ban(gentity_t *ent, int skiparg);
qboolean G_shrubbot_unban(gentity_t *ent, int skiparg);
qboolean G_shrubbot_putteam(gentity_t *ent, int skiparg);
qboolean G_shrubbot_pause(gentity_t *ent, int skiparg);
qboolean G_shrubbot_unpause(gentity_t *ent, int skiparg);
qboolean G_shrubbot_listplayers(gentity_t *ent, int skiparg);
qboolean G_shrubbot_mute(gentity_t *ent, int skiparg);
qboolean G_shrubbot_showbans(gentity_t *ent, int skiparg);
qboolean G_shrubbot_help(gentity_t *ent, int skiparg);
qboolean G_shrubbot_admintest(gentity_t *ent, int skiparg);
qboolean G_shrubbot_cancelvote(gentity_t *ent, int skiparg);
qboolean G_shrubbot_passvote(gentity_t *ent, int skiparg);
qboolean G_shrubbot_spec999(gentity_t *ent, int skiparg);
qboolean G_shrubbot_shuffle(gentity_t *ent, int skiparg);
qboolean G_shrubbot_rename(gentity_t *ent, int skiparg);
qboolean G_shrubbot_gib(gentity_t *ent, int skiparg);
qboolean G_shrubbot_giba(gentity_t *ent, int skiparg);
qboolean G_shrubbot_slap(gentity_t *ent, int skiparg);
qboolean G_shrubbot_burn(gentity_t *ent, int skiparg);
qboolean G_shrubbot_warn(gentity_t *ent, int skiparg);
qboolean G_shrubbot_news(gentity_t *ent, int skiparg);
qboolean G_shrubbot_lock(gentity_t *ent, int skiparg);
qboolean G_shrubbot_unlock(gentity_t *ent, int skiparg);
qboolean G_shrubbot_lockteams(gentity_t *ent, int skiparg, qboolean toLock);
qboolean G_shrubbot_lol(gentity_t *ent, int skiparg);
WARPROTO(panzer)
WARPROTO(sniper)
WARPROTO(rifle)
qboolean G_shrubbot_pip(gentity_t *ent, int skiparg);
qboolean G_shrubbot_pop(gentity_t *ent, int skiparg);
qboolean G_shrubbot_reset(gentity_t *ent, int skiparg);
qboolean G_shrubbot_fling(gentity_t *ent, int skiparg);
qboolean G_shrubbot_listteams(gentity_t *ent, int skiparg);
qboolean G_shrubbot_howfair(gentity_t *ent, int skiparg);
qboolean G_shrubbot_disorient(gentity_t *ent, int skiparg);
qboolean G_shrubbot_orient(gentity_t *ent, int skiparg);
qboolean G_shrubbot_resetxp(gentity_t *ent, int skiparg);
qboolean G_shrubbot_swap(gentity_t *ent, int skiparg);
qboolean G_shrubbot_nextmap(gentity_t *ent, int skiparg);
qboolean G_shrubbot_resetmyxp(gentity_t *ent, int skiparg);
qboolean G_shrubbot_balance(gentity_t *ent, int skiparg);
qboolean G_shrubbot_uptime(gentity_t *ent, int skiparg);
qboolean G_shrubbot_userinfo(gentity_t *ent, int skiparg);
qboolean G_shrubbot_stats(gentity_t *ent, int skiparg);
qboolean G_shrubbot_dewarn(gentity_t *ent, int skiparg);
qboolean G_shrubbot_spreerecord(gentity_t *ent, int skiparg);

//qboolean G_shrubbot_sk(gentity_t *ent, int skiparg);
//qboolean G_shrubbot_smoke(gentity_t *ent, int skiparg);
qboolean G_shrubbot_spree(gentity_t *ent, int skiparg);
qboolean G_shrubbot_tspree(gentity_t *ent, int skiparg);
qboolean G_shrubbot_bye(gentity_t *ent, int skiparg);

// pheno
qboolean G_shrubbot_freeze( gentity_t *ent, int skiparg );
qboolean G_shrubbot_unfreeze( gentity_t *ent, int skiparg );

void Cmd_Warning_f( gentity_t *ent );
int _shrubbot_level(gentity_t *ent);

qboolean G_shrubbot_permission(gentity_t *ent, char flag);
void G_shrubbot_print(gentity_t *ent, char *m);
void G_shrubbot_print_chat(gentity_t *ent, char *m);
void G_shrubbot_buffer_print(gentity_t *ent, char *m);
void G_shrubbot_buffer_begin();
void G_shrubbot_buffer_end(gentity_t *ent);

void G_shrubbot_readconfig_string(char **cnf, char *s, int size);
void G_shrubbot_readconfig_int(char **cnf, int *v);
void G_shrubbot_readconfig_float(char **cnf, float *v);
void G_shrubbot_writeconfig_string(char *s, fileHandle_t f);
void G_shrubbot_writeconfig_int(int v, fileHandle_t f);
void G_shrubbot_writeconfig_float(float v, fileHandle_t f);
void G_shrubbot_duration(int secs, char *duration, int dursize);
void G_shrubbot_cleanup();
#endif /* ifndef _G_SHRUBBOT_H */
