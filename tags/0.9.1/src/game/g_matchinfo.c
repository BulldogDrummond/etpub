
#include "g_local.h"
#include "g_http_client.h"

g_matchinfo_t *top_match_info = NULL;

void print_playermatchinfo(g_playermatchinfo_t *player) {
	G_LogPrintf("Player Match info "
			"Name: %s "
			"guid: %s "
			"Axis: %d "
			"Allies: %d\n",
			player->name,
			player->guid,
			player->axis_time,
			player->allies_time);
}

void print_matchinfo(g_matchinfo_t *matchinfo) {
	int i;
	G_LogPrintf("Match info "
			"Map: %s "
			"Winner: %d "
			"Length: %d\n",
			matchinfo->map,
			matchinfo->winner,
			matchinfo->length);
	for (i = 0; i< matchinfo->num_players; ++i) {
		print_playermatchinfo(&matchinfo->players[i]);
	}
}

// josh: just send it over the pipe
void G_matchinfo_add(g_matchinfo_t *matchinfo) {
	// 1 line per player and one line for who won / map / time, etc.
	int num_lines = matchinfo->num_players + 1;
	int i;
	char *message;

	if (g_etpub_stats_master_url.string[0]) {
		g_http_matchinfo_t *post_matchinfo = (g_http_matchinfo_t *)malloc(sizeof(g_http_matchinfo_t)); 
		post_matchinfo->info_lines = malloc(num_lines * sizeof(char*));
		post_matchinfo->info_lines_lengths = malloc(num_lines * sizeof(int));
		post_matchinfo->num_lines = num_lines;
		Q_strncpyz( post_matchinfo->url, g_etpub_stats_master_url.string, sizeof(post_matchinfo->url) );
		message = va("ID: %s"
			" MAP: %s"
			" Winner: %i"
			" Length: %i"
			" Hostname: %s\n",
			g_etpub_stats_id.string,
			matchinfo->map,
			matchinfo->winner,
			matchinfo->length,
			sv_hostname.string
		);
		post_matchinfo->info_lines_lengths[0] = (strlen(message) + 1) * sizeof(char); // +1 for \0 at the end
		post_matchinfo->info_lines[0] = malloc(post_matchinfo->info_lines_lengths[0]);
		Q_strncpyz( post_matchinfo->info_lines[0], message, post_matchinfo->info_lines_lengths[0]);
		for (i = 0; i < matchinfo->num_players; ++i) {
			message = va("GUID: %s"
				" AXIS: %i"
				" ALLIES: %i"
				" NAME: %s\n",
				matchinfo->players[i].guid,
				matchinfo->players[i].axis_time,
				matchinfo->players[i].allies_time,
				matchinfo->players[i].name
			);
			post_matchinfo->info_lines_lengths[i+1] = (strlen(message) + 1) * sizeof(char); // +1 for \0 at the end
			post_matchinfo->info_lines[i+1] = malloc(post_matchinfo->info_lines_lengths[i+1]);
			Q_strncpyz( post_matchinfo->info_lines[i+1], message, post_matchinfo->info_lines_lengths[i+1]);
		}
		//create_thread(libhttpc_postmatch,(void*)post_matchinfo);
		// try serial
		libhttpc_postmatch((void*)post_matchinfo);
	} else {
		G_LogPrintf("Stats Send: No master url!\n");
	}

	//josh: Legacy code for making a linked list---not needed
	//g_matchinfo_t *current = top_match_info;
	//if (current) {
	//	while (current->nextMatchInfo) {
	//		current = current->nextMatchInfo;
	//	}
	//	current->nextMatchInfo = matchinfo;
	//} else {
	//	top_match_info = matchinfo;
	//}
}

g_matchinfo_t *G_matchinfo_pop() {
	g_matchinfo_t *popped = top_match_info;
	top_match_info = popped->nextMatchInfo;
	popped->nextMatchInfo = NULL;
	return popped;
}

void G_matchinfo_delete(g_matchinfo_t * matchinfo) {
	free(matchinfo);
}

