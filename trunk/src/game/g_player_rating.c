// g_player_rating.c: Handles player and kill ratings
// -------------------------
//

#include "g_local.h"

void UpdateWinProbabilityModel( team_t winner,win_probability_model_t *model);
float GetRatingPoints(
		team_t team,
		team_t winner,
		qboolean hero,
		win_probability_model_t *model,
		int axis_time,
		int allies_time,
		float rating,
		float *rating_variance
	);

// tjw: pro-rate the rating points based on how long the player
//      was at each skill level during the match
static void UpdateSkillPR(gclient_t *cl, float ratingPoints)
{
	int i, j;
	int timetotal = 0;
	float fraction = 0.0f;

	// tjw: find total time
	for(i=0; i<NUM_SKILL_LEVELS; i++) {
		timetotal += cl->sess.skill_time[SK_BATTLE_SENSE][i];
	}

	if(timetotal < 1)
		return;

	for(i=0; i<SK_NUM_SKILLS; i++) {
		for(j=0; j<NUM_SKILL_LEVELS; j++) {
			if(!cl->sess.skill_time[i][j])
				continue;
			fraction = ((float)cl->sess.skill_time[i][j] /
				 (float)timetotal);
			cl->sess.pr_skill[i][j] +=
				ratingPoints * fraction;
			cl->sess.pr_skill_updates[i][j]++;
		}
	}
}

static void UpdateDisconnectSkillPR(
	g_disconnect_t *disconnect,
	float ratingPoints)
{
	int i, j;
	int timetotal = 0;
	float fraction = 0.0f;

	// tjw: find total time
	for(i=0; i<NUM_SKILL_LEVELS; i++) {
		timetotal += disconnect->skill_time[SK_BATTLE_SENSE][i];
	}

	for(i=0; i<SK_NUM_SKILLS; i++) {
		for(j=0; j<NUM_SKILL_LEVELS; j++) {
			if(!disconnect->skill_time[i][j])
				continue;
			fraction = ((float)disconnect->skill_time[i][j] /
				 (float)timetotal);
			disconnect->xpsave->pr_skill[i][j] +=
				ratingPoints * fraction;
			disconnect->xpsave->pr_skill_updates[i][j]++;
		}
	}
}
// josh: rate the players in this map based on the teams
void G_CalculatePlayerRatings(void) {
	char cs[MAX_STRING_CHARS];
	char *buf;
	int winner;

	// Determine winner
	trap_GetConfigstring( CS_MULTI_MAPWINNER, cs, sizeof(cs) );
	buf = Info_ValueForKey( cs, "winner" );
	winner = atoi( buf );

	// Change from scripting value for winner (0==AXIS, 1==ALLIES) to 
	// spawnflag value
	if( winner == 0 ) {
		winner = TEAM_AXIS;
	} else {
		winner = TEAM_ALLIES;
	}

	// Update model
	G_GetWinProbability(winner,NULL,qtrue); 
}

void RunWinProbabilityModel(win_probability_model_t *model) {
	float net = model->skill_difference
		+ model->allies_rating
		+ (model->num_allies - model->num_axis)
		*G_xpsave_serverstat()->rating;
	model->g_of_x = 1.0 / sqrt(1.0+model->variance*3.0/(M_PI*M_PI));
	model->win_probability = 1.0 / (1.0 + exp(-model->g_of_x*net));
	if (g_playerRating.integer & PLAYER_RATING_DEBUG 
		// Make sure this doesn't spam intermission / warmup ATB calls
		&& g_gamestate.integer == GS_PLAYING) {
		G_LogPrintf("PLAYER_RATING_DEBUG: "
			"Net: %.3f "
			"Allies: %.2f "
			"NumAllies: %.2f "
			"NumAxis: %.2f "
			"WinProb: %.2f"
			"\n",
			model->skill_difference,
			model->allies_rating,
			model->num_allies,
			model->num_axis,
			model->win_probability
		);
	}
}

// If ent is non-NULL, it will include ent as on the tested team
float G_GetWinProbability(team_t team, gentity_t *ent, qboolean updateWeights) {
	win_probability_model_t *model = &level.win_probability_model;
	g_mapstat_t *mapstat = G_xpsave_mapstat(level.rawmapname);
	float skill_difference = 0.0f;
	float numAxis = 0.0f;
	float numAllies = 0.0f;
	float diff;
	int i;
	// sum of the time played per team of all players
	int map_total_player_time_axis = 0;
	int map_total_player_time_allies = 0;
	gclient_t *cl;
	int timepassed = level.timeCurrent - level.startTime;
	int axis_time, allies_time;
	int timelimit = (int)(g_timelimit.value * 60000.0f);
	g_matchinfo_t *matchinfo = NULL;
	g_playermatchinfo_t *playermatchinfo;

	if((float)level.maxTimeLimit > g_timelimit.value)
		timelimit = level.maxTimeLimit * 60000;

	if (updateWeights) {
		if (g_etpub_stats_id.integer != -1) {
			matchinfo = (g_matchinfo_t *)malloc(sizeof(g_matchinfo_t)); 
			matchinfo->length = timepassed;
			matchinfo->maxTimeLimit = timelimit;
			Q_strncpyz(matchinfo->map,level.rawmapname,sizeof(matchinfo->map));
			matchinfo->num_players = 0;
			matchinfo->nextMatchInfo = NULL;
		}
		if (team == TEAM_AXIS) {
			if (g_etpub_stats_id.integer != -1) {
				matchinfo->winner = TEAM_AXIS;
			}
			if (g_playerRating.integer & PLAYER_RATING_DATASET) {
				G_LogPrintf("PLAYER_RATING_DATASET: "
					"Map: %s "
					"Winner: AXIS "
					"Time: %d "
					"Timelimit: %d\n",
					level.rawmapname, 
					timepassed, 
					timelimit);
			}
		} else {
			if (g_etpub_stats_id.integer != -1) {
				matchinfo->winner = TEAM_ALLIES;
			}
			if (g_playerRating.integer & PLAYER_RATING_DATASET) {
				G_LogPrintf("PLAYER_RATING_DATASET: "
					"Map: %s "
					"Winner: ALLIES "
					"Time: %d "
					"Timelimit: %d\n",
					level.rawmapname,
					timepassed, 
					timelimit);
			}
		}
	}

	model->map_total_time = timepassed;
	// first get the total player times
	for( i = 0; i < level.numConnectedClients; i++ ) {
		cl = level.clients + level.sortedClients[ i ];
		map_total_player_time_axis += cl->sess.mapAxisTime;
		map_total_player_time_allies += cl->sess.mapAlliesTime;
		if (!updateWeights) {
			// Simulate staying on this team till map end
			if (ent && ent->client == cl) {
				if (team == TEAM_AXIS) {
					map_total_player_time_axis +=
						timelimit
						- timepassed;
				}
				else if (team == TEAM_ALLIES) {
					map_total_player_time_allies +=
						timelimit
						- timepassed;
				}
			}
			else if (cl->sess.sessionTeam == TEAM_AXIS)
				map_total_player_time_axis +=
					timelimit
					- timepassed;
			else if (cl->sess.sessionTeam == TEAM_ALLIES)
				map_total_player_time_allies +=
					timelimit
					- timepassed;
		}

		if (updateWeights && (g_playerRating.integer & 
				PLAYER_RATING_DATASET || 
				g_etpub_stats_id.integer != -1)) {
			if (g_etpub_stats_id.integer != -1) {
				playermatchinfo = &matchinfo->players[matchinfo->num_players];
				Q_strncpyz(playermatchinfo->name, cl->pers.netname, sizeof(playermatchinfo->name));
				Q_strncpyz(playermatchinfo->guid, cl->sess.guid, sizeof(playermatchinfo->guid));
				playermatchinfo->axis_time = cl->sess.mapAxisTime;
				playermatchinfo->allies_time = cl->sess.mapAlliesTime;
				matchinfo->num_players++; 
			}
			G_LogPrintf("PLAYER_RATING_DATASET: "
				"Name: %s "
				"GUID %s: "
				"Time Axis: %d Time Allies: %d "
				"Killrating: %f "
				"\n"
				,cl->pers.netname
				,cl->sess.guid
				,cl->sess.mapAxisTime
				,cl->sess.mapAlliesTime
				,cl->sess.overall_killrating
			);
		}
	}
	for ( i = 0; i < MAX_DISCONNECTS; i++) {
		g_disconnect_t *disconnect = G_xpsave_disconnect(i);
		if (disconnect->xpsave != NULL) {
			map_total_player_time_axis += disconnect->axis_time;
			map_total_player_time_allies += disconnect->allies_time;
			if (updateWeights && (g_playerRating.integer 
					& PLAYER_RATING_DATASET || 
					g_etpub_stats_id.integer != -1)) {
				if (g_etpub_stats_id.integer != -1) {
					playermatchinfo = &matchinfo->players[matchinfo->num_players];
					Q_strncpyz(playermatchinfo->name, disconnect->xpsave->name, sizeof(playermatchinfo->name));
					Q_strncpyz(playermatchinfo->guid, disconnect->xpsave->guid, sizeof(playermatchinfo->guid));
					playermatchinfo->axis_time = disconnect->axis_time;
					playermatchinfo->allies_time = disconnect->allies_time;
					matchinfo->num_players++; 
				}
				G_LogPrintf("PLAYER_RATING_DATASET: "
					"Name: %s "
					"GUID %s: "
					"Time Axis: %d "
					"Time Allies: %d "
					"Killrating: %f "
					"\n"
					,disconnect->xpsave->name
					,disconnect->xpsave->guid
					,disconnect->axis_time
					,disconnect->allies_time
					,disconnect->killrating
				);
			}
		}
	}

	if (updateWeights && g_etpub_stats_id.integer != -1) {
		G_matchinfo_add(matchinfo);
	}

	if (!updateWeights)
		timepassed = timelimit;

	model->variance = 0.0f;
	for( i = 0; i < level.numConnectedClients; i++ ) {
		cl = level.clients + level.sortedClients[ i ];
		axis_time = cl->sess.mapAxisTime;
		allies_time = cl->sess.mapAlliesTime;

		if (!updateWeights) {
			// Simulate staying on this team till map end
			if (ent && ent->client == cl) {
				// If an ent was passed in, simulate him on
				// the other team for the rest of the match
				if (team == TEAM_AXIS) {
					axis_time += timelimit 
						- model->map_total_time;
				}
				else if (team == TEAM_ALLIES) {
					allies_time += timelimit
						- model->map_total_time;
				}
			}
			else if (cl->sess.sessionTeam == TEAM_AXIS)
				axis_time += timelimit
					- model->map_total_time; 
			else if (cl->sess.sessionTeam == TEAM_ALLIES)
				allies_time += timelimit
					- model->map_total_time; 
		}

		diff =   1.0f*(allies_time-axis_time)/timepassed;
		cl->sess.diff_percent_time = diff;
		cl->sess.total_percent_time = 1.0f*(allies_time+axis_time)/timepassed;
		skill_difference += diff*cl->sess.rating;
		model->variance += diff*diff*cl->sess.rating_variance;
	}

	for ( i = 0; i < MAX_DISCONNECTS; i++) {
		g_disconnect_t *disconnect = G_xpsave_disconnect(i);
		if (disconnect->xpsave != NULL) {
			diff =  1.0f*(disconnect->allies_time
				-disconnect->axis_time)
				/timepassed;
			skill_difference += diff*disconnect->xpsave->rating;
			model->variance += diff*diff*disconnect->xpsave->rating_variance;;
			disconnect->diff_percent_time = diff;
			disconnect->total_percent_time = 1.0f*(disconnect->allies_time+disconnect->axis_time)/timepassed;
		}
	}
	
	numAxis = map_total_player_time_axis * 1.0f
		/ timepassed;
	numAllies = map_total_player_time_allies * 1.0f
		/ timepassed;

	// account for general server variance
	model->variance += (numAllies-numAxis)*(numAllies-numAxis)*(G_xpsave_serverstat()->rating_variance);
	
	// account for map variance
	if (updateWeights || g_gametype.integer != GT_WOLF_STOPWATCH) {
		model->variance += mapstat->rating_variance;
	}

	// No points if not enough players
	// -1 is the "not enough players error code ;)
	if (updateWeights && (numAxis + numAllies 
			< g_playerRating_minplayers.integer)) {
		return -1;
	}

	// No points if one team is empty
	// So we don't divide by 0, etc.
	// yeah, -1 is the uh, empty team error code too . . .
	if (numAxis < 1 || numAllies < 1)
		return -1;

	model->num_allies = numAllies;
	model->num_axis = numAxis;
	model->allies_rating = mapstat->rating;
	model->skill_difference = skill_difference;

	// The first round should be treated normally. The second round should
	// compenstate for the shorter timelimit. Until I do that, I'm just
	// going to ignore the second round.
	// If it's the second round, don't update.
	if (g_gametype.integer == GT_WOLF_STOPWATCH) {

		if (g_currentRound.integer == 1) {
			updateWeights = qfalse;
		}

		// If we're not updating weights, ignore map info for ATB
		if (!updateWeights) {
			model->allies_rating = 0.0;
		}
	}

	RunWinProbabilityModel(model);

	if (updateWeights) {
		UpdateWinProbabilityModel(
				team,
				model);
	}
	return model->win_probability;
}

void UpdateWinProbabilityModel( team_t winner, win_probability_model_t *model) {
	int i;
	gclient_t *cl;
	g_mapstat_t *mapstat = G_xpsave_mapstat(level.rawmapname);
	g_serverstat_t *server = G_xpsave_serverstat();
	float
		allies_first_derivative,
		allies_second_derivative,
		server_first_derivative,
		server_second_derivative,
		num_more_allies
	;
	if (winner == TEAM_ALLIES) {
	    model->error = 1.0 - model->win_probability;
	} else {
	    model->error = -model->win_probability;
	}
	model->deriv_output = model->win_probability*(1.0-model->win_probability);

	num_more_allies = model->num_allies-model->num_axis;
	server_first_derivative = num_more_allies*model->error;
	server_first_derivative *= model->g_of_x;
	server_second_derivative = num_more_allies*num_more_allies*model->deriv_output;
	server_second_derivative *= model->g_of_x*model->g_of_x;
	server->rating_variance = 1.0/(1.0/server->rating_variance
					+ server_second_derivative);
	server->rating += server->rating_variance
				*server_first_derivative;

	allies_first_derivative = model->error;
	allies_first_derivative *= model->g_of_x;
	allies_second_derivative = model->deriv_output;
	allies_second_derivative *= model->g_of_x*model->g_of_x;
	mapstat->rating_variance = 1.0/(1.0/mapstat->rating_variance
				    + allies_second_derivative);
	mapstat->rating += mapstat->rating_variance
				*allies_first_derivative;

	// Update the players
	for( i = 0; i < level.numConnectedClients; i++ ) {
		float ratingPoints = 0;
		cl = level.clients + level.sortedClients[ i ];

		ratingPoints = GetRatingPoints(
				cl->sess.sessionTeam,
				winner,
				cl->sess.hero,
				model,
				cl->sess.mapAxisTime,
				cl->sess.mapAlliesTime,
				cl->sess.rating,
				&cl->sess.rating_variance
			);
		cl->sess.rating += ratingPoints;

		UpdateSkillPR(cl, ratingPoints);

		if (g_playerRating.integer & PLAYER_RATING_VISIBLE) {
			CPx(cl - level.clients, 
				va("chat \"^3Player Rating Points: %s ^3%.3f%s\" -1",
				cl->pers.netname,
				ratingPoints,
				(cl->sess.hero && winner==cl->sess.sessionTeam)
				? " Hero!" : ""));
		}
		if (g_playerRating.integer & PLAYER_RATING_DEBUG) {
			G_LogPrintf( "PLAYER_RATING_DEBUG: "
				"Player Rating Points: %s %.3f%s\n",
				cl->pers.netname,
				ratingPoints,
				(cl->sess.hero && cl->sess.sessionTeam==winner)
				? " Hero!" : "");
		}
	}
	// Update disconnects
	for ( i = 0; i < MAX_DISCONNECTS; i++) {
		float ratingPoints = 0;
		g_disconnect_t *disconnect = G_xpsave_disconnect(i);
		if (disconnect->xpsave) {
			ratingPoints = GetRatingPoints(
					TEAM_FREE,
					winner,
					qfalse,
					model,
					disconnect->axis_time,
					disconnect->allies_time,
					disconnect->xpsave->rating,
					&disconnect->xpsave->rating_variance
				);
			disconnect->xpsave->rating += ratingPoints;

			UpdateDisconnectSkillPR(disconnect, ratingPoints);

			if (g_playerRating.integer & PLAYER_RATING_DEBUG) {
				G_LogPrintf( "PLAYER_RATING_DEBUG: "
					"Disconnected Player Rating Points: "
					"%s %.3f\n",
					disconnect->xpsave->name,
					ratingPoints);
			}
		}
	}
}

float GetRatingPoints(
		team_t team,
		team_t winner,
		qboolean hero,
		win_probability_model_t *model,
		int axis_time,
		int allies_time,
		float rating,
		float *rating_variance) {

	float 
		first_derivative,
		second_derivative,
		diff
	;

	diff = (allies_time-axis_time)*1.0 / model->map_total_time;
	if (diff == 0.0f) {
		// don't update for people who didn't play at all
		// or else their variance will increase
		return 0.0f;
	}

	first_derivative = diff*model->error;
	first_derivative *= model->g_of_x;
	second_derivative = diff*diff*model->deriv_output;
	second_derivative *= model->g_of_x*model->g_of_x;
	(*rating_variance) = 1.0/(1.0/(*rating_variance)
				+second_derivative);
	
	//if (g_playerRating.integer & PLAYER_RATING_DEBUG) {
	//	G_LogPrintf( "PLAYER_RATING_DEBUG: "
	//			"Player Rating Prorate: %s"
	//			"WinRatio: %f LoseRatio: %f\n",
	//			cl->pers.netname, 
	//			diff);
	//}
	
	return (*rating_variance)*first_derivative;
}

void G_UpdateSkillTime(gentity_t *ent, qboolean final)
{
	int i, j;
	int classSkill;
	qboolean applies = qfalse;

	if(!ent->client)
		return;
	
	classSkill = BG_ClassSkillForClass(ent->client->sess.playerType);
	
	for(i=0; i<SK_NUM_SKILLS; i++) {
		applies = (i == SK_BATTLE_SENSE ||
				 i == SK_LIGHT_WEAPONS ||
				 i == classSkill);

		for(j=0; j<NUM_SKILL_LEVELS; j++) {
			// tjw: finalize any started skill levels we're
			//      not a part of any longer
			if(ent->client->sess.skill_start[i][j] &&
				(ent->client->sess.skill[i] != j ||
				!applies ||
				final)) {

				ent->client->sess.skill_time[i][j] +=
					level.time -
					ent->client->sess.skill_start[i][j];
				ent->client->sess.skill_start[i][j] = 0;
			}
			// tjw: start skill levels we are newly a part of
			else if(!ent->client->sess.skill_start[i][j] &&
				ent->client->sess.skill[i] == j &&
				applies) {

				ent->client->sess.skill_start[i][j] = 
					level.time;
			}
		}

	}
}
// these are just for logging, the client prints its own messages
char *modNames2[] =
{
	"MOD_UNKNOWN",
	"MOD_MACHINEGUN",
	"MOD_BROWNING",
	"MOD_MG42",
	"MOD_GRENADE",
	"MOD_ROCKET",

	// (SA) modified wolf weap mods
	"MOD_KNIFE",
	"MOD_LUGER",
	"MOD_COLT",
	"MOD_MP40",
	"MOD_THOMPSON",
	"MOD_STEN",
	"MOD_GARAND",
	"MOD_SNOOPERSCOPE",
	"MOD_SILENCER",	//----(SA)	
	"MOD_FG42",
	"MOD_FG42SCOPE",
	"MOD_PANZERFAUST",
	"MOD_GRENADE_LAUNCHER",
	"MOD_FLAMETHROWER",
	"MOD_GRENADE_PINEAPPLE",
	"MOD_CROSS",
	// end

	"MOD_MAPMORTAR",
	"MOD_MAPMORTAR_SPLASH",

	"MOD_KICKED",
	"MOD_GRABBER",

	"MOD_DYNAMITE",
	"MOD_AIRSTRIKE", // JPW NERVE
	"MOD_SYRINGE",	// JPW NERVE
	"MOD_AMMO",	// JPW NERVE
	"MOD_ARTY",	// JPW NERVE

	"MOD_WATER",
	"MOD_SLIME",
	"MOD_LAVA",
	"MOD_CRUSH",
	"MOD_TELEFRAG",
	"MOD_FALLING",
	"MOD_SUICIDE",
	"MOD_TARGET_LASER",
	"MOD_TRIGGER_HURT",
	"MOD_EXPLOSIVE",

	"MOD_CARBINE",
	"MOD_KAR98",
	"MOD_GPG40",
	"MOD_M7",
	"MOD_LANDMINE",
	"MOD_SATCHEL",
	"MOD_TRIPMINE",
	"MOD_SMOKEBOMB",
	"MOD_MOBILE_MG42",
	"MOD_SILENCED_COLT",
	"MOD_GARAND_SCOPE",

	"MOD_CRUSH_CONSTRUCTION",
	"MOD_CRUSH_CONSTRUCTIONDEATH",
	"MOD_CRUSH_CONSTRUCTIONDEATH_NOATTACKER",

	"MOD_K43",
	"MOD_K43_SCOPE",

	"MOD_MORTAR",

	"MOD_AKIMBO_COLT",
	"MOD_AKIMBO_LUGER",
	"MOD_AKIMBO_SILENCEDCOLT",
	"MOD_AKIMBO_SILENCEDLUGER",

	"MOD_SMOKEGRENADE",

	// RF
	"MOD_SWAP_PLACES",

	// OSP -- keep these 2 entries last
	"MOD_SWITCHTEAM",

	"MOD_GOOMBA",
	"MOD_POISON",
	"MOD_FEAR",
	"MOD_THROWN_KNIFE",
	"MOD_REFLECTED_FF",
};

// josh: these structures are for looking up the weapon's rating for kill
//       rating
typedef struct mod_weapRating_convert_s {
	weapon_t			mod;
	int rating;
} mod_weapRating_convert_t;

static const mod_weapRating_convert_t weaponRating[MOD_NUM_MODS] = {
	{ MOD_MACHINEGUN,			12 },
	{ MOD_GRENADE,				16 },
	{ MOD_ROCKET,				4 },
	{ MOD_KNIFE,				64 },
	{ MOD_LUGER,				24 },
	{ MOD_COLT,				24 },
	{ MOD_MP40,				16 },
	{ MOD_THOMPSON,				16 },
	{ MOD_STEN,				16 },
	{ MOD_GARAND,				24 },
	{ MOD_SILENCER,				24 },
	{ MOD_FG42,				16 },
	{ MOD_FG42SCOPE,			2 },
	{ MOD_PANZERFAUST,			2 },
	{ MOD_GRENADE_LAUNCHER,			24 },
	{ MOD_FLAMETHROWER,			8 },
	{ MOD_GRENADE_PINEAPPLE,		24 },
	{ MOD_AKIMBO_COLT,			16 },
	{ MOD_AKIMBO_LUGER,			16 },
	{ MOD_AKIMBO_SILENCEDCOLT,		16 },
	{ MOD_AKIMBO_SILENCEDLUGER,		16 },
	{ MOD_DYNAMITE,				4 },
	{ MOD_AIRSTRIKE,			8 },
	{ MOD_ARTY,				4 },
	{ MOD_CARBINE,				24 },
	{ MOD_KAR98,				24 },
	{ MOD_GPG40,				8 },
	{ MOD_M7,				8 },
	{ MOD_LANDMINE,				4 },
	{ MOD_SATCHEL,				64 },
	{ MOD_SMOKEGRENADE,			24 }, // rain - airstrike tag
	{ MOD_MOBILE_MG42,			12},
	{ MOD_SILENCED_COLT,			24 },	// where is silencer? // Gordon: up top^
	{ MOD_GARAND_SCOPE,			2 },
	{ MOD_K43,				24 },
	{ MOD_K43_SCOPE,			2 },
	{ MOD_MORTAR,				2 },
	{ MOD_BROWNING,				12 },
	{ MOD_MG42,				12 },
	{ MOD_POISON,				48 },
};

void G_UpdateKillRatings(gentity_t *killer, gentity_t *victim, int mod, int dmg) {
	int i;
	float
		killer_rating
		,victim_rating
		,points
		,distance_apart
		,prob_killer_won
	;
	vec3_t distance_vector;
	int weapon_rating = 0;
	int max_health;
	float damage_ratio;

	//  calculate distance apart
	VectorSubtract(killer->r.currentOrigin,victim->r.currentOrigin,distance_vector);
	distance_apart = VectorLength(distance_vector);

        // Got 140 from AddMedicTeamBonus in g_client.c, there is no
        // constant for it. Also, medics get up to 1.12*max_health
        // Also, I'm using max possible since it's l33ter to pwn someone with
        // more HP.
        max_health = 156;


	// health+dmg since we get this AFTER the dmg has been subtracted
	// from their health
        damage_ratio = min(dmg,victim->health+dmg)*1.0 / max_health;
	
	// If they were already dead, ignore
	if (damage_ratio <= 0)
		return;

	// find weapon rating
	for(i=0; i<MOD_NUM_MODS; i++) {
		if(mod == weaponRating[i].mod) {
			weapon_rating = weaponRating[i].rating;
		}
	}

	if (killer && 
		killer->client && 
		// Turn this off for now
		qfalse) {
		//g_killRating.integer & KILL_RATING_DATASET) {
		char *obit, *holding;
		int held = GetAmmoTableData(victim->s.weapon)->mod;

		if(mod < 0 
			|| mod >= sizeof(modNames2) 
			/ sizeof(modNames2[0])) {
			obit = "<bad obituary>";
		} else {
			obit = modNames2[mod];
		}

		if(held < 0 
			|| held >= sizeof(modNames2) 
			/ sizeof(modNames2[0])) {
			holding = "<bad obituary>";
		} else {
			holding = modNames2[held];
		}


		G_LogPrintf("KILL_RATING_DATASET: %s %s %i %f %i:"
			" \\%s\\ damaged \\%s\\ %f by %s"
			" holding %s\n",
			killer->client->sess.guid, 
			victim->client->sess.guid,
			mod,
			damage_ratio,
			held,
			killer->client->pers.netname,
			victim->client->pers.netname,
			damage_ratio,
			obit,
			holding
		);
	}


	// No reason to continue if unknown weapon
	if (weaponRating == 0)
		return;

	// first do overall
	killer_rating = killer->client->sess.overall_killrating;
	victim_rating = victim->client->sess.overall_killrating;
	//distance_bias = G_xpsave_serverstat->distance_rating*distance_apart;
	//prob_killer_won = 1.0 / (1.0+exp(-(distance_bias+killer_rating) /(integrate!)));
	prob_killer_won = 1.0 / (1.0+pow(10.0, (victim_rating-killer_rating)
				/400.0));

	// award * (observation - prediction)
	points = damage_ratio*weapon_rating*(1.0 - prob_killer_won); 

	// Assign
	killer->client->sess.overall_killrating += points;
	victim->client->sess.overall_killrating -= points;

	// for match, do something a little weird
	// basically, killing people with higher ratings gives more points,
	// but being killed makes you lose the same amount of points
	// I'll try this out till I model it better
	killer_rating = 1600.0;
	victim_rating = victim->client->sess.overall_killrating;
	prob_killer_won = 1.0 / (1.0+pow(10.0, (victim_rating-killer_rating)
				/400.0));

	// award * (observation - prediction)
	points = damage_ratio*weapon_rating*(1.0 - prob_killer_won); 

	// match_killrating just tracks this match's increase
	killer->client->sess.match_killrating += points;
	victim->client->sess.match_killrating -= damage_ratio*weapon_rating*0.5;
}

void G_LogKillGUID(gentity_t *killer, gentity_t *victim, int mod) {

	char *killerClass, *victimClass, *obit;
	vec3_t distance_vector;
	float 
		distance_apart
	;

	if(mod < 0 
		|| mod >= sizeof(modNames2) 
		/ sizeof(modNames2[0])) {
		obit = "<bad obituary>";
	} else {
		obit = modNames2[mod];
	}

	if(mod < 0 
		|| mod >= sizeof(modNames2) 
		/ sizeof(modNames2[0])) {
		obit = "<bad obituary>";
	} else {
		obit = modNames2[mod];
	}

	//  calculate distance apart
	VectorSubtract(killer->r.currentOrigin,victim->r.currentOrigin,distance_vector);
	distance_apart = VectorLength(distance_vector);

	switch (killer->client->sess.playerType) {
		case PC_SOLDIER:
			killerClass = "SOLDIER";
			break;
		case PC_MEDIC:
			killerClass = "MEDIC";
			break;
		case PC_ENGINEER:
			killerClass = "ENGINEER";
			break;
		case PC_FIELDOPS:
			killerClass = "FIELDOPS";
			break;
		case PC_COVERTOPS:
			killerClass = "COVERTOPS";
			break;
	}
	switch (victim->client->sess.playerType) {
		case PC_SOLDIER:
			victimClass = "SOLDIER";
			break;
		case PC_MEDIC:
			victimClass = "MEDIC";
			break;
		case PC_ENGINEER:
			victimClass = "ENGINEER";
			break;
		case PC_FIELDOPS:
			victimClass = "FIELDOPS";
			break;
		case PC_COVERTOPS:
			victimClass = "COVERTOPS";
			break;
	}

	G_LogPrintf("KILL_RATING_DATASET: %s %i %s %i %i %f:"
		" \\%s\\ class: %s killed \\%s\\ class: %s by %s at distance %f\n",
		killer->client->sess.guid, 
		killer->client->sess.playerType,
		victim->client->sess.guid,
		victim->client->sess.playerType,
		mod,
		distance_apart,
		killer->client->pers.netname,
		killerClass,
		victim->client->pers.netname,
		victimClass,
		obit,
		distance_apart
	);
}
