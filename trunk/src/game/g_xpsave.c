#include "g_local.h"

g_xpsave_t *g_xpsaves[MAX_XPSAVES];
g_mapstat_t *g_mapstats[MAX_MAPSTATS];

// josh: this is static because is makes it easier to handle
// reconnects. If you have a better idea, like a 2-way linked list,
// I'm all for that. I was just being lazy and simplistic.
g_disconnect_t g_disconnects[MAX_DISCONNECTS];

// josh: Again, saw no reason to not make this static, there's always 1
g_serverstat_t g_serverstat;

/*
 * gabriel: Get g_XPSaveMaxAge with applied multipliers (if any)
 */
int G_getXPSaveMaxAge() {
	int result = g_XPSaveMaxAge.integer;

	if (*g_XPSaveMaxAge.string) {
		switch(g_XPSaveMaxAge.string[strlen(g_XPSaveMaxAge.string) - 1]) {
			case 'O':
			case 'o':
				result *= 4;

			case 'W':
			case 'w':
				result *= 7;

			case 'D':
			case 'd':
				result *= 24;

			case 'H':
			case 'h':
				result *= 60;

			case 'M':
			case 'm':
				result *= 60;

				break;
		}
	}

	return result;
}

/*
 * gabriel: Get g_XPSaveMaxAge_xp with applied multipliers (if any)
 */
int G_getXPSaveMaxAge_xp() {
	int result = g_XPSaveMaxAge_xp.integer;

	if (*g_XPSaveMaxAge_xp.string) {
		switch(g_XPSaveMaxAge_xp.string[strlen(g_XPSaveMaxAge_xp.string) - 1]) {
			case 'O':
			case 'o':
				result *= 4;

			case 'W':
			case 'w':
				result *= 7;

			case 'D':
			case 'd':
				result *= 24;

			case 'H':
			case 'h':
				result *= 60;

			case 'M':
			case 'm':
				result *= 60;

				break;
		}
	}

	return result;
}

void G_xpsave_writeconfig() 
{
	fileHandle_t f;
	int len, i, j;
	time_t t;
	int age = 0;
	int eff_XPSaveMaxAge_xp = G_getXPSaveMaxAge_xp();
	int eff_XPSaveMaxAge = G_getXPSaveMaxAge();

	if(!(g_XPSave.integer & XPSF_ENABLE))
		return;	
	if(!g_XPSaveFile.string[0])
		return;
	time(&t);
	len = trap_FS_FOpenFile(g_XPSaveFile.string, &f, FS_WRITE);
	if(len < 0) {
		G_Printf("G_xpsave_writeconfig: could not open %s\n",
				g_XPSaveFile.string);
	}

	trap_FS_Write("[serverstat]\n", 13, f);
	trap_FS_Write("rating           = ", 19, f);
	G_shrubbot_writeconfig_float(g_serverstat.rating, f);
	trap_FS_Write("rating_variance  = ", 19, f);
	G_shrubbot_writeconfig_float(g_serverstat.rating_variance, f);
	trap_FS_Write("distance_rating  = ", 19, f);
	G_shrubbot_writeconfig_float(g_serverstat.distance_rating, f);
	trap_FS_Write("distance_variance= ", 19, f);
	G_shrubbot_writeconfig_float(g_serverstat.distance_variance, f);
	trap_FS_Write("\n", 1, f);
	G_Printf("xpsave: wrote server rating: %f\n", g_serverstat.rating);

	for(i=0; g_mapstats[i]; i++) {
		trap_FS_Write("[mapstat]\n", 10, f);
		trap_FS_Write("name             = ", 19, f);
		G_shrubbot_writeconfig_string(g_mapstats[i]->name, f);
		trap_FS_Write("rating           = ", 19, f);
		G_shrubbot_writeconfig_float(g_mapstats[i]->rating, f);
		trap_FS_Write("rating_variance  = ", 19, f);
		G_shrubbot_writeconfig_float(g_mapstats[i]->rating_variance, f);
		trap_FS_Write("spree_record     = ", 19, f);
		G_shrubbot_writeconfig_int(g_mapstats[i]->spreeRecord, f);
		trap_FS_Write("spree_name       = ", 19, f);
		G_shrubbot_writeconfig_string(g_mapstats[i]->spreeName, f);
		trap_FS_Write("\n", 1, f);
	}
	G_Printf("xpsave: wrote %d mapstats\n", i);

	for(i=0; g_xpsaves[i]; i++) {
		if(!g_xpsaves[i]->time)
			continue;
		age = t - g_xpsaves[i]->time;
		if(age > eff_XPSaveMaxAge) {
			continue;
		}

		trap_FS_Write("[xpsave]\n", 9, f);
		trap_FS_Write("guid             = ", 19, f);
		G_shrubbot_writeconfig_string(g_xpsaves[i]->guid, f);
		trap_FS_Write("name             = ", 19, f);
		G_shrubbot_writeconfig_string(g_xpsaves[i]->name, f);
		trap_FS_Write("time             = ", 19, f);
		G_shrubbot_writeconfig_int(g_xpsaves[i]->time, f);
		if(age <= eff_XPSaveMaxAge_xp) {
			for(j=0; j<SK_NUM_SKILLS; j++) {
				if(g_xpsaves[i]->skill[j] == 0.0f)
					continue;
				trap_FS_Write(va("skill[%i]         = ", j),
					19, f);
				G_shrubbot_writeconfig_float(
					g_xpsaves[i]->skill[j], f);
			}
		}
		if(g_xpsaves[i]->kill_rating != 0.0f) { 
			trap_FS_Write("kill_rating      = ", 19, f);
			G_shrubbot_writeconfig_float(g_xpsaves[i]->kill_rating,
				f);
		}
		if(g_xpsaves[i]->kill_variance != SIGMA2_DELTA) { 
			trap_FS_Write("kill_variance    = ", 19, f);
			G_shrubbot_writeconfig_float(g_xpsaves[i]->kill_variance,
				f);
		}
		if(g_xpsaves[i]->rating != 0) { 
			trap_FS_Write("rating           = ", 19, f);
			G_shrubbot_writeconfig_float(
				g_xpsaves[i]->rating, f);
		}
		if(g_xpsaves[i]->rating_variance != SIGMA2_THETA) { 
			trap_FS_Write("rating_variance  = ", 19, f);
			G_shrubbot_writeconfig_float(
				g_xpsaves[i]->rating_variance, f);
		}
		if(g_xpsaves[i]->mutetime) {
			trap_FS_Write("mutetime         = ", 19, f);
			G_shrubbot_writeconfig_int(g_xpsaves[i]->mutetime, f);
		}
		if(g_playerRating.integer & PLAYER_RATING_SKILLS) {
			for(j=0; j<SK_NUM_SKILLS; j++) {
				trap_FS_Write(va("pr_skill[%i]      = ", j),
					19, f);
				G_shrubbot_writeconfig_string(
					va("%i %f %i %f %i %f %i %f %i %f",
					g_xpsaves[i]->pr_skill_updates[j][0],
					g_xpsaves[i]->pr_skill[j][0],
					g_xpsaves[i]->pr_skill_updates[j][1],
					g_xpsaves[i]->pr_skill[j][1],
					g_xpsaves[i]->pr_skill_updates[j][2],
					g_xpsaves[i]->pr_skill[j][2],
					g_xpsaves[i]->pr_skill_updates[j][3],
					g_xpsaves[i]->pr_skill[j][3],
					g_xpsaves[i]->pr_skill_updates[j][4],
					g_xpsaves[i]->pr_skill[j][4]),
					f);
			}
		}
		trap_FS_Write("\n", 1, f);
	}
	G_Printf("xpsave: wrote %d xpsaves\n", i);
	trap_FS_FCloseFile(f);
}

void G_xpsave_readconfig() 
{
	g_xpsave_t *x = g_xpsaves[0];
	g_mapstat_t *m = g_mapstats[0];
	int xc = 0;
	int mc = 0;
	fileHandle_t f;
	int i, j;
	int len;
	char *cnf, *cnf2;
	char *t;
	qboolean xpsave_open;
	qboolean mapstat_open;
	qboolean serverstat_open;
	qboolean found_serverstat = qfalse;
	float skill;
	char buffer[MAX_STRING_CHARS];

	if(!(g_XPSave.integer & XPSF_ENABLE))
		return;	
	if(!g_XPSaveFile.string[0])
		return;
	len = trap_FS_FOpenFile(g_XPSaveFile.string, &f, FS_READ) ; 
	if(len < 0) {
		G_Printf("readconfig: could not open xpsave file %s\n",
			g_XPSaveFile.string);
		return;
	}
	cnf = malloc(len+1);
	cnf2 = cnf;
	trap_FS_Read(cnf, len, f);
	*(cnf + len) = '\0';
	trap_FS_FCloseFile(f);

	G_xpsave_cleanup();
	
	t = COM_Parse(&cnf);
	xpsave_open = qfalse;
	mapstat_open = qfalse;
	serverstat_open = qfalse;
	while(*t) {
		if(!Q_stricmp(t, "[xpsave]") ||
			!Q_stricmp(t, "[mapstat]") ||
			!Q_stricmp(t, "[serverstat]")
			) {

			if(xpsave_open)
				g_xpsaves[xc++] = x;
			if(mapstat_open)
				g_mapstats[mc++] = m;
			xpsave_open = qfalse;
			mapstat_open = qfalse;
			serverstat_open = qfalse;
		}

		if(xpsave_open) {
			if(!Q_stricmp(t, "guid")) {
				G_shrubbot_readconfig_string(&cnf, 
					x->guid, sizeof(x->guid)); 
			}
			else if(!Q_stricmp(t, "name")) {
				G_shrubbot_readconfig_string(&cnf, 
					x->name, sizeof(x->name)); 
			}
			else if(!Q_stricmp(t, "time")) {
				G_shrubbot_readconfig_int(&cnf, &x->time);
			}
			else if(!Q_stricmpn(t, "skill[", 6)) {
				for(i=0; i<SK_NUM_SKILLS; i++) {
					if(Q_stricmp(t, va("skill[%i]", i)))
						continue;		
					G_shrubbot_readconfig_float(&cnf,
						&skill);
					x->skill[i] = skill;
					break;
				}
			}
			else if(!Q_stricmp(t, "kill_rating")) {
				G_shrubbot_readconfig_float(&cnf, &x->kill_rating);
			}
			else if(!Q_stricmp(t, "kill_variance")) {
				G_shrubbot_readconfig_float(&cnf, &x->kill_variance);
			}
			else if(!Q_stricmp(t, "rating")) {
				G_shrubbot_readconfig_float(&cnf,
					&x->rating);
			}
			else if(!Q_stricmp(t, "rating_variance")) {
				G_shrubbot_readconfig_float(&cnf,
					&x->rating_variance);
			}
			else if(!Q_stricmp(t, "mutetime")) {
				G_shrubbot_readconfig_int(&cnf, &x->mutetime);
			}
			else if(!Q_stricmpn(t, "pr_skill[", 9)) {
				for(i=0; i<SK_NUM_SKILLS; i++) {
					if(Q_stricmp(t, va("pr_skill[%i]", i)))
						continue;		
					G_shrubbot_readconfig_string(&cnf,
						buffer, sizeof(buffer));
					sscanf(buffer,
						"%i %f %i %f %i %f %i %f %i %f",
						&x->pr_skill_updates[i][0],
						&x->pr_skill[i][0],
						&x->pr_skill_updates[i][1],
						&x->pr_skill[i][1],
						&x->pr_skill_updates[i][2],
						&x->pr_skill[i][2],
						&x->pr_skill_updates[i][3],
						&x->pr_skill[i][3],
						&x->pr_skill_updates[i][4],
						&x->pr_skill[i][4]);
					break;
				}
			}
			else {
				G_Printf("xpsave: [xpsave] parse error near "
					"%s on line %d\n", 
					t, 
					COM_GetCurrentParseLine());
			}
		}
		else if(mapstat_open) {
			if(!Q_stricmp(t, "name")) {
				G_shrubbot_readconfig_string(&cnf, 
					m->name, sizeof(m->name)); 
			}
			else if(!Q_stricmp(t, "rating")) {
				G_shrubbot_readconfig_float(&cnf,
					&m->rating);
			}
			else if(!Q_stricmp(t, "rating_variance")) {
				G_shrubbot_readconfig_float(&cnf,
					&m->rating_variance);
			}
			else if(!Q_stricmp(t, "spree_record")) {
				G_shrubbot_readconfig_int(&cnf,
					&m->spreeRecord);
			}
			else if(!Q_stricmp(t, "spree_name")) {
				G_shrubbot_readconfig_string(&cnf, 
					m->spreeName, sizeof(m->spreeName)); 
			}
			else {
				G_Printf("xpsave: [mapstat] parse error near "
					"%s on line %d\n", 
					t, 
					COM_GetCurrentParseLine());
			}
		}
		else if(serverstat_open) {
			if(!Q_stricmp(t, "rating")) {
				G_shrubbot_readconfig_float(&cnf,
					&g_serverstat.rating);
			}
			else if(!Q_stricmp(t, "rating_variance")) {
				G_shrubbot_readconfig_float(&cnf,
					&g_serverstat.rating_variance);
			}
			else if(!Q_stricmp(t, "distance_rating")) {
				G_shrubbot_readconfig_float(&cnf,
					&g_serverstat.distance_rating);
			}
			else if(!Q_stricmp(t, "distance_variance")) {
				G_shrubbot_readconfig_float(&cnf,
					&g_serverstat.distance_variance);
			}
			else {
				G_Printf("xpsave: [serverstat] parse error near "
					"%s on line %d\n", 
					t, 
					COM_GetCurrentParseLine());
			}
		}

		if(!Q_stricmp(t, "[xpsave]")) {
			if(xc >= MAX_XPSAVES) {
				G_Printf("xpsave: error MAX_XPSAVES exceeded");
				return;
			}
			x = malloc(sizeof(g_xpsave_t));
			x->guid[0] = '\0';
			x->name[0] = '\0';
			x->kill_rating = 0.0f;
			x->kill_variance = SIGMA2_DELTA;
			x->rating = 0.0f;
			x->rating_variance = SIGMA2_THETA;
			for(i=0; i<SK_NUM_SKILLS; i++) {
				x->skill[i] = 0.0f;
				for(j=0; j<NUM_SKILL_LEVELS; j++) {
					x->pr_skill_updates[i][j] = 0;
					x->pr_skill[i][j] = 0.0f;
				}
			}
			x->mutetime = 0;
			x->hits = 0;
			x->team_hits = 0;
			x->time = 0;
			xpsave_open = qtrue;
		}
		if(!Q_stricmp(t, "[mapstat]")) {
			if(mc >= MAX_MAPSTATS) {
				G_Printf("xpsave: error MAX_MAPSTATS exceeded");
				return;
			}
			m = malloc(sizeof(g_mapstat_t));
			m->name[0] = '\0';
			m->rating = 0.0f;
			m->rating_variance = SIGMA2_GAMMA;
			m->spreeRecord = 0;
			m->spreeName[0] = '\0';
			mapstat_open = qtrue;
		}
		if(!Q_stricmp(t, "[serverstat]")) {
			// server prior = 2.6, NOT 0
			g_serverstat.rating = 2.6f;
			g_serverstat.rating_variance = SIGMA2_PSI;
			g_serverstat.distance_rating = 0.0f;
			g_serverstat.distance_variance = SIGMA2_DELTA;
			serverstat_open = qtrue;
			found_serverstat = qtrue;
		}
		t = COM_Parse(&cnf);
	}
	if(xpsave_open)
		g_xpsaves[xc++] = x;
	else if(mapstat_open)
		g_mapstats[mc++] = m;

	free(cnf2);
	if (!found_serverstat) {
		// server prior = 2.6, NOT 0
		g_serverstat.rating = 2.6f;
		g_serverstat.rating_variance = SIGMA2_PSI;
	}
	G_Printf("xpsave: loaded %d mapstats, %d xpsaves\n", mc, xc);
}
void AddDisconnect(gentity_t *ent
	,g_xpsave_t *xpsave
	,int axis_time
	,int allies_time
	,team_t map_ATBd_team
	,team_t last_playing_team
	,float killrating
    ,float killvariance) {

	int i, j, k;

	G_UpdateSkillTime(ent, qtrue);

	for (i = 0; i < MAX_DISCONNECTS; i++) {
		if (g_disconnects[i].xpsave == NULL) {
			g_disconnects[i].xpsave = xpsave;
			g_disconnects[i].axis_time = axis_time;
			g_disconnects[i].allies_time = allies_time;
			g_disconnects[i].map_ATBd_team = map_ATBd_team;
			g_disconnects[i].last_playing_team = last_playing_team;
			g_disconnects[i].killrating = killrating;
			g_disconnects[i].killvariance = killvariance;
			for(j=0; j<SK_NUM_SKILLS; j++) {
				for(k=0; k<NUM_SKILL_LEVELS; k++) {
					g_disconnects[i].skill_time[j][k] =
						ent->client->sess.skill_time[j][k];
				}
			}
			g_disconnects[i].lives = -999;
			// tjw: usually going to be -999 unless the client
			//      reconnected more than once without joining
			//      a team.
			if(!ent->client->maxlivescalced)  {
				g_disconnects[i].lives =
					ent->client->disconnectLives;
			}
			else if(g_maxlives.integer || 
				g_alliedmaxlives.integer || 
				g_axismaxlives.integer) {

				g_disconnects[i].lives =
				(ent->client->ps.persistant[PERS_RESPAWNS_LEFT]
					- 1);
				if(g_disconnects[i].lives < 0)
					g_disconnects[i].lives = 0;
			}

			G_Printf("Added DisconnectRecord guid %s lives %d\n",
				ent->client->sess.guid,
				g_disconnects[i].lives);
			return;
		}
	}
}

void Reconnect(g_xpsave_t *connect, gentity_t *ent) {
	int i, j, k;
	for (i = 0; i < MAX_DISCONNECTS; i++) {
		if (g_disconnects[i].xpsave == connect) {
			g_disconnects[i].xpsave = NULL;
			ent->client->sess.mapAxisTime =
				g_disconnects[i].axis_time;
			ent->client->sess.mapAlliesTime =
			g_disconnects[i].allies_time;
			ent->client->sess.map_ATBd_team =
				g_disconnects[i].map_ATBd_team;
			ent->client->sess.last_playing_team =
				g_disconnects[i].last_playing_team;
			ent->client->sess.overall_killrating =
				g_disconnects[i].killrating;
			ent->client->sess.overall_killvariance =
				g_disconnects[i].killvariance;
			for(j=0; j<SK_NUM_SKILLS; j++) {
				for(k=0; k<NUM_SKILL_LEVELS; k++) {
					ent->client->sess.skill_time[j][k] =
						g_disconnects[i].skill_time[j][k];
				}
			}
			ent->client->disconnectLives =
				g_disconnects[i].lives;
			G_Printf("Found DisconnectRecord guid %s lives %d\n",
				ent->client->sess.guid,
				ent->client->disconnectLives);
			return;
		}
	}
}

void G_reset_disconnects() {
	int i;
	for (i = 0; i < MAX_DISCONNECTS; i++) {
		g_disconnects[i].xpsave = NULL;
	}
}

qboolean G_xpsave_add(gentity_t *ent,qboolean disconnect)
{
	int i = 0;
	int j = 0;
	int k = 0;
	char guid[33];
	char name[MAX_NAME_LENGTH] = {""};
	int clientNum;
	qboolean found = qfalse;
	g_xpsave_t *x = g_xpsaves[0];
	time_t t;

	if(!(g_XPSave.integer & XPSF_ENABLE))
		return qfalse;	
	if(!ent || !ent->client)
		return qfalse;
	if(!time(&t))
		return qfalse;
	if(ent->client->pers.connected != CON_CONNECTED)
		return qfalse;
	if ((g_OmniBotFlags.integer & OBF_DONT_XPSAVE) && 
		(ent->r.svFlags & SVF_BOT))
		return qfalse;

	clientNum = ent - g_entities;
	// tjw: some want raw name
	//SanitizeString(ent->client->pers.netname, name, qtrue);
	Q_strncpyz(name, ent->client->pers.netname, sizeof(name));
	Q_strncpyz(guid, ent->client->sess.guid, sizeof(guid));
	
	if(!guid[0] || strlen(guid) != 32)
		return qfalse;

	for(i=0; g_xpsaves[i]; i++) {
		if(!Q_stricmp(g_xpsaves[i]->guid, guid)) {
			x = g_xpsaves[i];
			found = qtrue;
			break;
		}
	}
	if(!found) {
		if(i == MAX_XPSAVES) {
			G_Printf("xpsave: cannot save. MAX_XPSAVES exceeded");
			return qfalse;
		}
		x = malloc(sizeof(g_xpsave_t));
		x->guid[0] = '\0';
		x->name[0] = '\0';
		x->kill_rating = 0.0f;
		x->kill_variance = SIGMA2_DELTA;
		x->rating = 0.0f;
		x->rating_variance = SIGMA2_THETA;
		for(j=0; j<SK_NUM_SKILLS; j++) {
			x->skill[j] = 0.0f;
			for(k=0; k<NUM_SKILL_LEVELS; k++) {
				x->pr_skill_updates[j][k] = 0;
				x->pr_skill[j][k] = 0.0f;
			}
		}
		x->mutetime = 0;
		x->hits = 0;
		x->team_hits = 0;
		x->time = 0;
		g_xpsaves[i] = x;
	}
	Q_strncpyz(x->guid, guid, sizeof(x->guid));
	Q_strncpyz(x->name, name, sizeof(x->name));
	x->time = t;
	for(i=0; i<SK_NUM_SKILLS; i++) {
		x->skill[i] = ent->client->sess.skillpoints[i];
		for(j=0; j<NUM_SKILL_LEVELS; j++) {
			x->pr_skill_updates[i][j] = 
				ent->client->sess.pr_skill_updates[i][j];
			x->pr_skill[i][j] = 
				ent->client->sess.pr_skill[i][j];
		}
	}
	x->kill_rating = ent->client->sess.overall_killrating;
	x->kill_variance = ent->client->sess.overall_killvariance;
	x->rating = ent->client->sess.rating;
	x->rating_variance = ent->client->sess.rating_variance;

	if(ent->client->sess.auto_unmute_time <= -1){
		x->mutetime = -1;
	}else if(ent->client->sess.auto_unmute_time == 0){
		x->mutetime = 0;
	}else{
		x->mutetime = ((ent->client->sess.auto_unmute_time-level.time)/1000+t);
	}

	x->hits = ent->client->sess.hits;
	x->team_hits = ent->client->sess.team_hits;

	if (disconnect) {
		AddDisconnect(ent
			,x
			,ent->client->sess.mapAxisTime
			,ent->client->sess.mapAlliesTime
			,ent->client->sess.map_ATBd_team
			,ent->client->sess.last_playing_team
			,ent->client->sess.overall_killrating
			,ent->client->sess.overall_killvariance
		);
	}
	return qtrue;
}

qboolean G_xpsave_load(gentity_t *ent)
{
	int i, j;
	qboolean found = qfalse, XPSMuted = qfalse;
	int clientNum;
	g_xpsave_t *x = g_xpsaves[0];
	time_t t;
	char agestr[MAX_STRING_CHARS];
	//char desc[64];
	// josh: Increased this
	// josh: TODO: tjw? What is this desc thing for?
	char desc[115];
	int age;
	int eff_XPSaveMaxAge_xp = G_getXPSaveMaxAge_xp();
	int eff_XPSaveMaxAge = G_getXPSaveMaxAge();

	if(!ent || !ent->client)
		return qfalse;
	if(!(g_XPSave.integer & XPSF_ENABLE))
		return qfalse;	
	if(!time(&t))
		return qfalse;

	desc[0] = '\0';
	clientNum = ent - g_entities;
	
	for(i=0; g_xpsaves[i]; i++) {
		if(!Q_stricmp(g_xpsaves[i]->guid, 
			ent->client->sess.guid)) {
			found = qtrue;
			x = g_xpsaves[i];
			break;
		}
	}
	if(!found)
		return qfalse;

	age = t - x->time;
	if(age > eff_XPSaveMaxAge) {
		return qfalse;
	}

	if(age <= eff_XPSaveMaxAge_xp) {
		for(i=0; i<SK_NUM_SKILLS; i++) {
			ent->client->sess.skillpoints[i] = x->skill[i];
			ent->client->sess.startxptotal += x->skill[i];
		}
		ent->client->ps.stats[STAT_XP] = 
			(int)ent->client->sess.startxptotal;
		Q_strcat(desc, sizeof(desc), "XP/");
		if((g_XPDecay.integer & XPDF_ENABLE) &&
			!(g_XPDecay.integer & XPDF_NO_DISCONNECT_DECAY)) {
			G_XPDecay(ent, age, qtrue);
		}
	}

	ent->client->sess.overall_killrating = x->kill_rating;
	ent->client->sess.overall_killvariance = x->kill_variance;
	//ent->client->sess.playerrating = x->playerrating;
	ent->client->sess.rating = x->rating;
	ent->client->sess.rating_variance = x->rating_variance;

	for(i=0; i<SK_NUM_SKILLS; i++) {
		for(j=0; j<NUM_SKILL_LEVELS; j++) {
			ent->client->sess.pr_skill_updates[i][j] =
				x->pr_skill_updates[i][j];
			ent->client->sess.pr_skill[i][j] =
				x->pr_skill[i][j];
		}
	}

	Q_strcat(desc, sizeof(desc), "ratings/");
	
	if(x->mutetime != 0) {
		if(x->mutetime < 0){
			ent->client->sess.auto_unmute_time = -1;
			XPSMuted = qtrue;
		}else if(x->mutetime > t){
			ent->client->sess.auto_unmute_time = (level.time + 1000*(x->mutetime - t));
			XPSMuted = qtrue;;
		}

		if(XPSMuted == qtrue){
			CP("print \"^5You've been muted by XPSave\n\"" );
			Q_strcat(desc, sizeof(desc), "mute/");
		}
	}

	ent->client->sess.hits = x->hits;
	ent->client->sess.team_hits = x->team_hits;
	G_CalcRank(ent->client);
	BG_PlayerStateToEntityState(&ent->client->ps,
			&ent->s,
			level.time,
			qtrue);
	// tjw: trim last / from desc
	if(strlen(desc))
		desc[strlen(desc)-1] = '\0';
	G_shrubbot_duration(age, agestr, sizeof(agestr)); 
	CP(va(
		"print \"^3server: loaded stored ^7%s^3 state from %s ago\n\"",
		desc,
		agestr));

	// josh: check and update if disconnect found
	Reconnect(x,ent);
	return qtrue;
}


void G_AddSpreeRecord()
{
	g_mapstat_t *mapstat = G_xpsave_mapstat(level.rawmapname);

	if(mapstat->spreeRecord < level.maxspree_count
		&& level.maxspree_count > 0){
		mapstat->spreeRecord = level.maxspree_count;
		Q_strncpyz(mapstat->spreeName, 
			level.maxspree_player,
			sizeof(mapstat->spreeName));
	}
}

void G_ShowSpreeRecord(qboolean command)
{
	g_mapstat_t *mapstat = G_xpsave_mapstat(level.rawmapname);
	int i;
	int record = 0;
	int highest = -1;
	char msg[MAX_STRING_CHARS];

	msg[0] = '\0';

	if(mapstat->spreeRecord > 0 && mapstat->spreeRecord >= level.maxspree_count){
		Q_strcat(msg, sizeof(msg), 
			va("^3Map Spree Record: ^1%d^3 kills by ^7%s^3. ", 
				mapstat->spreeRecord,
				mapstat->spreeName ? mapstat->spreeName : "UNKNOWN"));
	}else if(level.maxspree_count > 0 && command){
		Q_strcat(msg, sizeof(msg), 
			va("^3Map Spree Record: ^1%d^3 kills by ^7%s^3. ", 
				level.maxspree_count,
				level.maxspree_player ? level.maxspree_player : "UNKNOWN"));
	}

	for(i=0; g_mapstats[i]; i++) {
		if(g_mapstats[i]->spreeRecord > record){
			highest = i;
			record = g_mapstats[i]->spreeRecord;
		}
	}

	if(highest > -1 && record > 0 && record >= level.maxspree_count){
		Q_strcat(msg, sizeof(msg), 
			va("^3Overall Spree Record: ^1%d^3 kills by ^7%s", 
				g_mapstats[highest]->spreeRecord,
				g_mapstats[highest]->spreeName ? g_mapstats[highest]->spreeName : "UNKNOWN"));
	}else if(level.maxspree_count > 0 && command){
		Q_strcat(msg, sizeof(msg), 
			va("^3Overall Spree Record: ^1%d^3 kills by ^7%s", 
				level.maxspree_count,
				level.maxspree_player ? level.maxspree_player : "UNKNOWN"));
	}

	if(!msg[0] && command){
		Q_strcat(msg, sizeof(msg),
			"^3No spree records found");
	}

	if (msg[0]) {
		AP(va("chat \"%s\" -1",msg));
	}
}

/*
void G_xpsave_clear()
{
	int i;
	for(i=0; g_xpsaves[i]; i++) {
		free(g_xpsaves[i]);
		g_xpsaves[i] = NULL;
	}
	G_xpsave_writeconfig();
}
*/

void G_xpsave_resetxp()
{
	int i,j;
	for(i=0; g_xpsaves[i]; i++) {
		for(j=0; j<SK_NUM_SKILLS; j++) {
			g_xpsaves[i]->skill[j] = 0.0f;
		}
		g_xpsaves[i]->hits = 0;
		g_xpsaves[i]->team_hits = 0;

	}
}

void G_xpsave_resetpr(qboolean full_reset) {
	int i;
	for(i=0; g_xpsaves[i]; i++) {
		g_xpsaves[i]->rating = 0.0f;
		g_xpsaves[i]->rating_variance = SIGMA2_THETA;
	}
	if (full_reset) {
		// server prior = 2.6, NOT 0
		g_serverstat.rating = 2.6f;
		g_serverstat.rating_variance = SIGMA2_PSI;
		
	}
}

void G_xpsave_resetSpreeRecords()
{
	int i;

	for(i=0; g_mapstats[i]; i++) {
		g_mapstats[i]->spreeRecord = 0;
		g_mapstats[i]->spreeName[0] = '\0';
	}
}

g_mapstat_t *G_xpsave_mapstat(char *mapname)
{
	int i;
	qboolean found = qfalse;
	g_mapstat_t *m = g_mapstats[0];
	
	for(i=0; g_mapstats[i]; i++) {
		if(!Q_stricmp(g_mapstats[i]->name, mapname)) {
			found = qtrue;
			m = g_mapstats[i];
			break;
		}
	}
	if(!found) {
		if(i == MAX_MAPSTATS) {
			G_Printf("xpsave: cannot save. MAX_MAPSTATS exceeded");
			return NULL;
		}
		m = malloc(sizeof(g_mapstat_t));
		Q_strncpyz(m->name, mapname, sizeof(m->name));
		m->rating = 0.0f;
		m->rating_variance = SIGMA2_GAMMA;
		m->spreeRecord = 0;
		m->spreeName[0] = '\0';
		g_mapstats[i] = m;
	}
	return m;
}

g_disconnect_t *G_xpsave_disconnect(int i) {
	return &g_disconnects[i];
}

void G_xpsave_cleanup()
{
	int i = 0;

	for(i=0; g_xpsaves[i]; i++) {
		free(g_xpsaves[i]);
		g_xpsaves[i] = NULL;
	}
	for(i=0; g_mapstats[i]; i++) {
		free(g_mapstats[i]);
		g_mapstats[i] = NULL;
	}
}

g_serverstat_t *G_xpsave_serverstat() {
	return &g_serverstat;
}
