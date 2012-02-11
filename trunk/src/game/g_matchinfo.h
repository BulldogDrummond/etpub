#ifndef _G_MATCAHINFO_H
#define _G_MATCHINFO_H

#define MAX_QUEUED_MATCHES 10
#define MAX_PLAYERS_PER_MATCH 64

typedef struct {
	char name[MAX_NETNAME];
	char guid[PB_GUID_LENGTH + 1];
	int axis_time;
	int allies_time;
} g_playermatchinfo_t;

struct _matchinfo_t {
	char map[MAX_QPATH];
	team_t winner;
	int length;
	int maxTimeLimit;
	g_playermatchinfo_t players[MAX_PLAYERS_PER_MATCH];
	int num_players;
	struct _matchinfo_t *nextMatchInfo;
};

typedef struct _matchinfo_t g_matchinfo_t;

void G_matchinfo_add(g_matchinfo_t *matchinfo);
g_matchinfo_t *G_matchinfo_pop();
// This only delete that ONE match info. It does NOT deallocate the list
void G_matchinfo_delete(g_matchinfo_t * matchinfo);

#endif /* ifndef _G_MATCHINFO_H */
