#ifndef _G_PLAYER_RATING_H
#define _G_PLAYER_RATING_H

#define SIGMA2_THETA 1.0f // prior on player rating variance
#define SIGMA2_PSI 1.0f // prior on server rating variance
#define SIGMA2_GAMMA 1.0f // prior on allies map rating variance
#define SIGMA2_DELTA 1.0f // prior on kill rating variance

typedef struct win_probability_model_s {
	// w = winner, l = loser
	int map_total_time;
	float
		skill_difference,
		allies_rating,
		win_probability,
		num_allies, 
		num_axis,
		error,
		deriv_output,
		variance,
		g_of_x
	;
} win_probability_model_t;

void G_CalculatePlayerRatings();
float G_GetWinProbability(team_t team, gentity_t *ent, qboolean updateWeights);
void G_UpdateKillRatings(gentity_t *killer, gentity_t *victim, int mod);
void G_LogKillGUID(gentity_t *killer, gentity_t *victim, int mod);
float G_GetAdjKillsPerDeath(float rating, float variance);

#endif /* ifndef _G_PLAYER_RATING_H */
