#ifndef _G_XPSAVE_H
#define _G_XPSAVE_H

#define MAX_XPSAVES 32768
#define MAX_MAPSTATS 1024
#define MAX_DISCONNECTS 1024

typedef struct {
	char guid[33];
	char name[MAX_NAME_LENGTH];
	int time;
	float skill[SK_NUM_SKILLS];
	float hits;
	float team_hits;
	int pr_skill_updates[SK_NUM_SKILLS][NUM_SKILL_LEVELS];
	float pr_skill[SK_NUM_SKILLS][NUM_SKILL_LEVELS];
	// tjw: lives moved into g_disconnect_t
	//int lives;
    
    // josh: killrating removed, now kill_rating AND kill_variance
    float kill_rating;
    float kill_variance;

	// The real player rating for 0.8.x
	float rating;
	//rating variance for 0.8.x. NOT a sum. This IS the variance.
	float rating_variance;

	// Dens: store the real time when someone needs to be unmuted
	// 0 means not muted, -1 means permanent (never)
	int mutetime;

} g_xpsave_t;

typedef struct {
	char name[MAX_QPATH];

	// rating and variance always allies for 0.8.x
	float rating;
	float rating_variance;

	int spreeRecord;
	char spreeName[MAX_NETNAME];
} g_mapstat_t;

typedef struct {
	// for 0.8.x
	float rating;
	float rating_variance;
    float distance_rating;
    float distance_variance;
} g_serverstat_t;

typedef struct {
	g_xpsave_t *xpsave;
	int axis_time;
	int allies_time;
	team_t map_ATBd_team;
	team_t last_playing_team;
	int skill_time[SK_NUM_SKILLS][NUM_SKILL_LEVELS];
	int lives;
	float killrating;
	float killvariance;
	// These are temp variables for PR system usage, not really tracked
	float total_percent_time;
	float diff_percent_time;
} g_disconnect_t;

void G_xpsave_writeconfig();
void G_xpsave_readconfig();
qboolean G_xpsave_add(gentity_t *ent,qboolean disconnect);
qboolean G_xpsave_load(gentity_t *ent);
g_mapstat_t *G_xpsave_mapstat(char *mapname);
g_disconnect_t *G_xpsave_disconnect(int i);
void G_reset_disconnects();
void G_xpsave_resetxp();
void G_xpsave_cleanup();
g_serverstat_t *G_xpsave_serverstat();
void G_xpsave_resetpr(qboolean full_reset);
void G_xpsave_resetSpreeRecords();
void G_AddSpreeRecord();
void G_ShowSpreeRecord(qboolean command);
// pheno
void G_xpsave_writexp();
#endif /* ifndef _G_XPSAVE_H */
