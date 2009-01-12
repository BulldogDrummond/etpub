#ifndef _G_SETTINGS_H
#define _G_SETTINGS_H

#define MAX_KILLSPREES 32
#define MAX_KS_ENDS 32
#define MAX_MULTIKILLS 16
#define MAX_BANNERS 32
#define MAX_REVIVESPREES 8

typedef struct {
	int number;
	char message[MAX_STRING_CHARS];
	char position[8];
	char display[7];
	char sound[MAX_STRING_CHARS];
	char play[7];
} g_killingSpree_t;

typedef struct {
	int number;
	char message[MAX_STRING_CHARS];
	char position[8];
	char display[7];
	char sound[MAX_STRING_CHARS];
	char play[7];
	char tkmessage[MAX_STRING_CHARS];
	char tkposition[8];
	char tkdisplay[7];
	char tksound[MAX_STRING_CHARS];
	char tkplay[7];
	char skmessage[MAX_STRING_CHARS];
	char skposition[8];
	char skdisplay[7];
	char sksound[MAX_STRING_CHARS];
	char skplay[7];
	char wkmessage[MAX_STRING_CHARS];
	char wkposition[8];
	char wkdisplay[7];
	char wksound[MAX_STRING_CHARS];
	char wkplay[7];
} g_ks_end_t;

typedef struct {
	int number;
	char message[MAX_STRING_CHARS];
	char position[8];
	char display[7];
	char sound[MAX_STRING_CHARS];
	char play[7];
} g_multiKill_t;

typedef struct {
	char message[MAX_CVAR_VALUE_STRING];
	int wait;
	char position[8];
} g_banner_t;

typedef struct {
	int number;
	char message[MAX_STRING_CHARS];
	char position[8];
	char display[7];
	char sound[MAX_STRING_CHARS];
	char play[7];
} g_reviveSpree_t;

void G_settings_readconfig();
void G_check_killing_spree(gentity_t *ent, int number);
void G_check_killing_spree_end(gentity_t *ent, gentity_t *other, int number, int start);
void G_check_multikill(gentity_t *ent, int number);
void G_displayBanner(int loop);
void G_check_revive_spree(gentity_t *ent, int number);
int G_highest_multikill(gentity_t *ent);

#endif /* ifndef _G_SETTINGS_H */
