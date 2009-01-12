#include "g_local.h"

g_killingSpree_t *g_killingSprees[MAX_KILLSPREES];
g_ks_end_t *g_ks_ends[MAX_KS_ENDS];
g_multiKill_t *g_multiKills[MAX_MULTIKILLS];
g_banner_t *g_banners[MAX_BANNERS];
g_reviveSpree_t *g_reviveSprees[MAX_REVIVESPREES];

void G_settings_readconfig() 
{
	g_killingSpree_t *s = g_killingSprees[0];
	g_ks_end_t *e = g_ks_ends[0];
	g_multiKill_t *k = g_multiKills[0];
	g_banner_t *b = g_banners[0];
	g_reviveSpree_t *r = g_reviveSprees[0];
	int sc = 0;
	int ec = 0;
	int kc = 0;
	int bc = 0;
	int rc = 0;
	fileHandle_t f;
	int len;
	char *cnf, *cnf2;
	char *t;
	qboolean spree_open;
	qboolean end_open;
	qboolean kill_open;
	qboolean banner_open;
	qboolean revive_open;
	int i = 0;

	for(i=0; g_killingSprees[i]; i++) {
		free(g_killingSprees[i]);
		g_killingSprees[i] = NULL;
	}
	for(i=0; g_ks_ends[i]; i++) {
		free(g_ks_ends[i]);
		g_ks_ends[i] = NULL;
	}
	for(i=0; g_multiKills[i]; i++) {
		free(g_multiKills[i]);
		g_multiKills[i] = NULL;
	}
	for(i=0; g_banners[i]; i++) {
		free(g_banners[i]);
		g_banners[i] = NULL;
	}
	for(i=0; g_reviveSprees[i]; i++) {
		free(g_reviveSprees[i]);
		g_reviveSprees[i] = NULL;
	}

	if(!g_settings.string[0]){
		return;
	}

	len = trap_FS_FOpenFile(g_settings.string, &f, FS_READ) ; 
	if(len < 0) {
		G_Printf("G_settings_readconfig: could not open settings file %s\n", g_settings.string);
		return;
	}

	cnf = malloc(len+1);
	cnf2 = cnf;
	trap_FS_Read(cnf, len, f);
	*(cnf + len) = '\0';
	trap_FS_FCloseFile(f);
	
	t = COM_Parse(&cnf);
	spree_open = qfalse;
	end_open = qfalse;
	kill_open = qfalse;
	banner_open = qfalse;
	revive_open = qfalse;

	while(*t) {
		if(!Q_stricmp(t, "[spree]") ||
			!Q_stricmp(t, "[end]") ||
			!Q_stricmp(t, "[kill]") ||
			!Q_stricmp(t, "[banner]") ||
			!Q_stricmp(t, "[revive]")) {
			
			if(spree_open){
				g_killingSprees[sc++] = s;
			}

			if(end_open){
				g_ks_ends[ec++] = e;
			}

			if(kill_open){
				g_multiKills[kc++] = k;
			}

			if(banner_open){
				g_banners[bc++] = b;
			}

			if(revive_open){
				g_reviveSprees[rc++] = r;
			}

			spree_open = qfalse;
			end_open = qfalse;
			kill_open = qfalse;
			banner_open = qfalse;
			revive_open = qfalse;
		}

		if(spree_open) {
			if(!Q_stricmp(t, "number")) {
				G_shrubbot_readconfig_int(&cnf,&s->number); 
			}
			else if(!Q_stricmp(t, "message")) {
				G_shrubbot_readconfig_string(&cnf, 
					s->message, sizeof(s->message)); 
			}
			else if(!Q_stricmp(t, "position")) {
				G_shrubbot_readconfig_string(&cnf, 
					s->position, sizeof(s->position));
			}
			else if(!Q_stricmp(t, "display")) {
				G_shrubbot_readconfig_string(&cnf, 
					s->display, sizeof(s->display));
			}
			else if(!Q_stricmp(t, "sound")) {
				G_shrubbot_readconfig_string(&cnf, 
					s->sound, sizeof(s->sound));
			}
			else if(!Q_stricmp(t, "play")) {
				G_shrubbot_readconfig_string(&cnf, 
					s->play, sizeof(s->play));
			} else {
				G_Printf("settings: [spree] parse error near "
					"%s on line %d\n", 
					t, 
					COM_GetCurrentParseLine());
			}
		}else if(end_open) {
			if(!Q_stricmp(t, "number")) {
				G_shrubbot_readconfig_int(&cnf,&e->number); 
			}
			else if(!Q_stricmp(t, "message")) {
				G_shrubbot_readconfig_string(&cnf, 
					e->message, sizeof(e->message)); 
			}
			else if(!Q_stricmp(t, "position")) {
				G_shrubbot_readconfig_string(&cnf, 
					e->position, sizeof(e->position));
			}
			else if(!Q_stricmp(t, "display")) {
				G_shrubbot_readconfig_string(&cnf, 
					e->display, sizeof(e->display));
			}
            else if(!Q_stricmp(t, "sound")) {
				G_shrubbot_readconfig_string(&cnf, 
					e->sound, sizeof(e->sound));
			}
			else if(!Q_stricmp(t, "play")) {
				G_shrubbot_readconfig_string(&cnf, 
					e->play, sizeof(e->play));
            }
			else if(!Q_stricmp(t, "tkmessage")) {
				G_shrubbot_readconfig_string(&cnf, 
					e->tkmessage, sizeof(e->tkmessage)); 
			}
			else if(!Q_stricmp(t, "tkposition")) {
				G_shrubbot_readconfig_string(&cnf, 
					e->tkposition, sizeof(e->tkposition));
			}
			else if(!Q_stricmp(t, "tkdisplay")) {
				G_shrubbot_readconfig_string(&cnf, 
					e->tkdisplay, sizeof(e->tkdisplay));
			}
			else if(!Q_stricmp(t, "tksound")) {
				G_shrubbot_readconfig_string(&cnf, 
					e->tksound, sizeof(e->tksound));
			}
			else if(!Q_stricmp(t, "tkplay")) {
				G_shrubbot_readconfig_string(&cnf, 
					e->tkplay, sizeof(e->tkplay));
            }
			else if(!Q_stricmp(t, "skmessage")) {
				G_shrubbot_readconfig_string(&cnf, 
					e->skmessage, sizeof(e->skmessage)); 
			}
			else if(!Q_stricmp(t, "skposition")) {
				G_shrubbot_readconfig_string(&cnf, 
					e->skposition, sizeof(e->skposition));
			}
			else if(!Q_stricmp(t, "skdisplay")) {
				G_shrubbot_readconfig_string(&cnf, 
					e->skdisplay, sizeof(e->skdisplay));
			}
			else if(!Q_stricmp(t, "sksound")) {
				G_shrubbot_readconfig_string(&cnf, 
					e->sksound, sizeof(e->sksound));
			}
			else if(!Q_stricmp(t, "skplay")) {
				G_shrubbot_readconfig_string(&cnf, 
					e->skplay, sizeof(e->skplay));
            }
			else if(!Q_stricmp(t, "wkmessage")) {
				G_shrubbot_readconfig_string(&cnf, 
					e->wkmessage, sizeof(e->wkmessage)); 
			}
			else if(!Q_stricmp(t, "wkposition")) {
				G_shrubbot_readconfig_string(&cnf, 
					e->wkposition, sizeof(e->wkposition));
			}
			else if(!Q_stricmp(t, "wkdisplay")) {
				G_shrubbot_readconfig_string(&cnf, 
					e->wkdisplay, sizeof(e->wkdisplay));
			}
            else if(!Q_stricmp(t, "wksound")) {
				G_shrubbot_readconfig_string(&cnf, 
					e->wksound, sizeof(e->wksound));
			}
			else if(!Q_stricmp(t, "wkplay")) {
				G_shrubbot_readconfig_string(&cnf, 
					e->wkplay, sizeof(e->wkplay));
			}else {
				G_Printf("settings: [end] parse error near "
					"%s on line %d\n", 
					t, 
					COM_GetCurrentParseLine());
			}
		}else if(kill_open) {
			if(!Q_stricmp(t, "number")) {
				G_shrubbot_readconfig_int(&cnf,&k->number); 
			}
			else if(!Q_stricmp(t, "message")) {
				G_shrubbot_readconfig_string(&cnf, 
					k->message, sizeof(k->message)); 
			}
			else if(!Q_stricmp(t, "position")) {
				G_shrubbot_readconfig_string(&cnf, 
					k->position, sizeof(k->position));
			}
			else if(!Q_stricmp(t, "display")) {
				G_shrubbot_readconfig_string(&cnf, 
					k->display, sizeof(k->display));
			}
			else if(!Q_stricmp(t, "sound")) {
				G_shrubbot_readconfig_string(&cnf, 
					k->sound, sizeof(k->sound));
			}
			else if(!Q_stricmp(t, "play")) {
				G_shrubbot_readconfig_string(&cnf, 
					k->play, sizeof(k->play));
			} else {
				G_Printf("settings: [kill] parse error near "
					"%s on line %d\n", 
					t, 
					COM_GetCurrentParseLine());
			}
		}else if(banner_open) {
			if(!Q_stricmp(t, "message")) {
				G_shrubbot_readconfig_string(&cnf, 
					b->message, sizeof(b->message)); 
			}
			else if(!Q_stricmp(t, "wait")) {
				G_shrubbot_readconfig_int(&cnf,&b->wait); 
			}
			else if(!Q_stricmp(t, "position")) {
				G_shrubbot_readconfig_string(&cnf, 
					b->position, sizeof(b->position));
			} else {
				G_Printf("settings: [banner] parse error near "
					"%s on line %d\n", 
					t, 
					COM_GetCurrentParseLine());
			}
		}else if(revive_open) {
			if(!Q_stricmp(t, "number")) {
				G_shrubbot_readconfig_int(&cnf,&r->number);
			}
			else if(!Q_stricmp(t, "message")) {
				G_shrubbot_readconfig_string(&cnf,
					r->message, sizeof(r->message));
			}
			else if(!Q_stricmp(t, "position")) {
				G_shrubbot_readconfig_string(&cnf,
					r->position, sizeof(r->position));
			}
			else if(!Q_stricmp(t, "display")) {
				G_shrubbot_readconfig_string(&cnf,
					r->display, sizeof(r->display));
			}
			else if(!Q_stricmp(t, "sound")) {
				G_shrubbot_readconfig_string(&cnf,
					r->sound, sizeof(r->sound));
			}
			else if(!Q_stricmp(t, "play")) {
				G_shrubbot_readconfig_string(&cnf,
					r->play, sizeof(r->play));
			} else {
				G_Printf("settings: [revive] parse error near "
					"%s on line %d\n",
					t,
					COM_GetCurrentParseLine());
			}
		}

		if(!Q_stricmp(t, "[spree]")) {
			if(sc >= MAX_KILLSPREES) {
				G_Printf("settings: error MAX_KILLSPREES exceeded");
				return;
			}
			s = malloc(sizeof(g_killingSpree_t));
			s->number = 0;
			s->message[0] = '\0';
			s->display[0] = '\0';
			s->position[0] = '\0';
			s->sound[0] = '\0';
			s->play[0] = '\0';
			spree_open = qtrue;
		}
		if(!Q_stricmp(t, "[end]")) {
			if(ec >= MAX_KS_ENDS) {
				G_Printf("settings: error MAX_KS_ENDS exceeded");
				return;
			}
			e = malloc(sizeof(g_ks_end_t));
			e->number = 0;
			e->message[0] = '\0';
			e->display[0] = '\0';
			e->position[0] = '\0';
			e->sound[0] = '\0';
			e->play[0] = '\0';
			e->tkmessage[0] = '\0';
			e->tkdisplay[0] = '\0';
			e->tkposition[0] = '\0';
			e->tksound[0] = '\0';
			e->tkplay[0] = '\0';
			e->skmessage[0] = '\0';
			e->skdisplay[0] = '\0';
			e->skposition[0] = '\0';
			e->sksound[0] = '\0';
			e->skplay[0] = '\0';
			e->wkmessage[0] = '\0';
			e->wkdisplay[0] = '\0';
			e->wkposition[0] = '\0';
			e->wksound[0] = '\0';
			e->wkplay[0] = '\0';
			end_open = qtrue;
		}
		if(!Q_stricmp(t, "[kill]")) {
			if(kc >= MAX_MULTIKILLS) {
				G_Printf("settings: error MAX_MULTIKILLS exceeded");
				return;
			}
			k = malloc(sizeof(g_multiKill_t));
			k->number = 0;
			k->message[0] = '\0';
			k->display[0] = '\0';
			k->position[0] = '\0';
			k->sound[0] = '\0';
			k->play[0] = '\0';
			kill_open = qtrue;
		}
		if(!Q_stricmp(t, "[banner]")) {
			if(bc >= MAX_BANNERS) {
				G_Printf("settings: error MAX_BANNERS exceeded");
				return;
			}
			b = malloc(sizeof(g_banner_t));
			b->message[0] = '\0';
			b->wait = 0;
			b->position[0] = '\0';
			banner_open = qtrue;
		}
		if(!Q_stricmp(t, "[revive]")) {
			if(rc >= MAX_REVIVESPREES) {
				G_Printf("settings: error MAX_REVIVESPREES exceeded");
				return;
			}
			r = malloc(sizeof(g_reviveSpree_t));
			r->number = 0;
			r->message[0] = '\0';
			r->display[0] = '\0';
			r->position[0] = '\0';
			r->sound[0] = '\0';
			r->play[0] = '\0';
			revive_open = qtrue;
		}
		t = COM_Parse(&cnf);
	}
	if(spree_open){
		g_killingSprees[sc++] = s;
	}else if(end_open){
		g_ks_ends[ec++] = e;
	}else if(kill_open){
		g_multiKills[kc++] = k;
	}else if(banner_open){
		g_banners[bc++] = b;
	}else if(revive_open){
		g_reviveSprees[rc++] = r;
	}

	free(cnf2);

	G_Printf("settings: loaded %d sprees, %d ends, %d kills, %d banners and %d revive sprees\n"
		, sc, ec, kc, bc, rc);
}

// Dens: moved from g_combat.c
char *G_KillSpreeSanitize(char *text)
{
	static char n[MAX_SAY_TEXT] = {""};

	if(!*text)
		return n;
	Q_strncpyz(n, text, sizeof(n));
	Q_strncpyz(n, Q_StrReplace(n, "[a]", "(a)"), sizeof(n));
	Q_strncpyz(n, Q_StrReplace(n, "[v]", "(v)"), sizeof(n));
	Q_strncpyz(n, Q_StrReplace(n, "[n]", "(n)"), sizeof(n));
	Q_strncpyz(n, Q_StrReplace(n, "[d]", "(d)"), sizeof(n));
	Q_strncpyz(n, Q_StrReplace(n, "[k]", "(k)"), sizeof(n));
	return n;
}

void G_check_killing_spree(gentity_t *ent, int number)
{
	int i;
	gentity_t *tent;
	char name[MAX_NAME_LENGTH] = {"*unknown*"};
	char *output;

	for(i=0; g_killingSprees[i]; i++){
		
		if(!g_killingSprees[i]->number){
			continue;
		}

		if(g_killingSprees[i]->number == number){
			if(g_killingSprees[i]->message[0]){
				
				Q_strncpyz(name,
					G_KillSpreeSanitize(ent->client->pers.netname),
					sizeof(name));
				output = Q_StrReplace(g_killingSprees[i]->message, "[n]", name);

				if(g_killingSprees[i]->display[0] && !Q_stricmp(g_killingSprees[i]->display,"player")){
					if(g_killingSprees[i]->position[0]){
						
						if(!Q_stricmp(g_killingSprees[i]->position,"center") 
							|| !Q_stricmp(g_killingSprees[i]->position,"cp")){
							CP(va("cp \"%s\"",output));
						}else if(!Q_stricmp(g_killingSprees[i]->position,"popup") 
							|| !Q_stricmp(g_killingSprees[i]->position,"cpm")){
							CP(va("cpm \"%s\"",output));
						}else if(!Q_stricmp(g_killingSprees[i]->position,"banner") 
							|| !Q_stricmp(g_killingSprees[i]->position,"bp")){
							CP(va("bp \"%s\"",output));
						}else if(!Q_stricmp(g_killingSprees[i]->position,"console") 
							|| !Q_stricmp(g_killingSprees[i]->position,"print")){
							CP(va("print \"%s\n\"", output));
						}else{
							CP(va("chat \"%s\"",output));
						}
					}else{
						CP(va("chat \"%s\"",output));
					}
				}else{
					if(g_killingSprees[i]->position[0]){
						
						if(!Q_stricmp(g_killingSprees[i]->position,"center") 
							|| !Q_stricmp(g_killingSprees[i]->position,"cp")){
							AP(va("cp \"%s\"",output));
						}else if(!Q_stricmp(g_killingSprees[i]->position,"popup") 
							|| !Q_stricmp(g_killingSprees[i]->position,"cpm")){
							AP(va("cpm \"%s\"",output));
						}else if(!Q_stricmp(g_killingSprees[i]->position,"banner") 
							|| !Q_stricmp(g_killingSprees[i]->position,"bp")){
							AP(va("bp \"%s\"",output));
						}else if(!Q_stricmp(g_killingSprees[i]->position,"console") 
							|| !Q_stricmp(g_killingSprees[i]->position,"print")){
							AP(va("print \"%s\n\"", output));
						}else{
							AP(va("chat \"%s\"",output));
						}
					}else{
						AP(va("chat \"%s\"",output));
					}
				}
			}
			if(g_killingSprees[i]->sound[0]){
				if(g_killingSprees[i]->play[0] && !Q_stricmp(g_killingSprees[i]->play,"envi")){
					G_AddEvent(ent, EV_GENERAL_SOUND,G_SoundIndex(va("%s",g_killingSprees[i]->sound)));
				}else if(g_killingSprees[i]->play[0] && !Q_stricmp(g_killingSprees[i]->play,"player")){
					tent = G_TempEntity(ent->r.currentOrigin,EV_GLOBAL_CLIENT_SOUND);
					tent->s.teamNum = (ent->client - level.clients);
					tent->s.eventParm = G_SoundIndex(va("%s",g_killingSprees[i]->sound));
				}else{
					G_globalSound(g_killingSprees[i]->sound);
				}
			}
		}
	}
	
	return;

}
// Dens: start is added so that it is possible to have multiple outputs for 1 spree end
// Example: it is determined that in structure 5 the highest number can be found. The game
// will than display the number 5 message, and will look further at 6 (start = 6) for the
// SAME amount. Note: start should always be 0 when it is used outside of the function itself
void G_check_killing_spree_end(gentity_t *ent, gentity_t *other, int number, int start)
{
	int i;
	int structure = -1;
	int highest = 0;
	char *output;
	qboolean entOnly = qfalse;
	char pos[6];
	char name[MAX_NAME_LENGTH] = {"*unknown*"};
	char oname[MAX_NAME_LENGTH] = {"*unknown*"};
	gentity_t *tent;

	if(start > 0){
		number = g_ks_ends[start-1]->number;
	}
	if(number > 0){
		for(i=start; g_ks_ends[i]; i++){
			if(!g_ks_ends[i]->number){
				continue;
			}
			if(number >= g_ks_ends[i]->number && g_ks_ends[i]->number > highest ){
				structure = i;
				highest = g_ks_ends[i]->number;
			}
		}
	}else if(number < 0){
		for(i=start; g_ks_ends[i]; i++){
			if(!g_ks_ends[i]->number){
				continue;
			}
			if(number <= g_ks_ends[i]->number && g_ks_ends[i]->number < highest ){
				structure = i;
				highest = g_ks_ends[i]->number;
			}
		}
	}else{
		return;
	}

	if(structure == -1){
		return;
	}

	i = structure;

	if(!other->client){
		if(g_ks_ends[i]->wkmessage[0]){
			
			Q_strncpyz(name, G_KillSpreeSanitize(ent->client->pers.netname),sizeof(name));
			output = Q_StrReplace(g_ks_ends[i]->wkmessage, "[n]", name);
			output = Q_StrReplace(output, "[k]", va("%d", number));
			
			if(g_ks_ends[i]->wkdisplay[0] && !Q_stricmp(g_ks_ends[i]->wkdisplay,"player")){
				entOnly = qtrue;
			}

			if(g_ks_ends[i]->wkposition[0]){	
				if(!Q_stricmp(g_ks_ends[i]->wkposition,"center") 
					|| !Q_stricmp(g_ks_ends[i]->wkposition,"cp")){
					Q_strncpyz(pos, "cp",sizeof(pos));
				}else if(!Q_stricmp(g_ks_ends[i]->wkposition,"popup") 
					|| !Q_stricmp(g_ks_ends[i]->wkposition,"cpm")){
					Q_strncpyz(pos, "cpm",sizeof(pos));
				}else if(!Q_stricmp(g_ks_ends[i]->wkposition,"banner") 
					|| !Q_stricmp(g_ks_ends[i]->wkposition,"bp")){
					Q_strncpyz(pos, "bp",sizeof(pos));
				}else if(!Q_stricmp(g_ks_ends[i]->wkposition,"console") 
					|| !Q_stricmp(g_ks_ends[i]->wkposition,"print")){
					Q_strncpyz(pos, "print",sizeof(pos));
				}else{
					Q_strncpyz(pos, "chat",sizeof(pos));
				}
			}else{
				Q_strncpyz(pos, "chat",sizeof(pos));
			}
	
		}
		
		if(g_ks_ends[i]->wksound[0]){
			if(g_ks_ends[i]->wkplay[0] && !Q_stricmp(g_ks_ends[i]->wkplay,"envi")){
				G_AddEvent(ent, EV_GENERAL_SOUND,G_SoundIndex(va("%s",g_ks_ends[i]->wksound)));
			}else if(g_ks_ends[i]->wkplay[0] && !Q_stricmp(g_ks_ends[i]->wkplay,"player")){
				tent = G_TempEntity(ent->r.currentOrigin,EV_GLOBAL_CLIENT_SOUND);
				tent->s.teamNum = (ent->client - level.clients);
				tent->s.eventParm = G_SoundIndex(va("%s",g_ks_ends[i]->wksound));
			}else{
				G_globalSound(g_ks_ends[i]->wksound);
			}
        }
	}else if(ent == other){
		if(g_ks_ends[i]->skmessage[0]){
			
			Q_strncpyz(name, G_KillSpreeSanitize(ent->client->pers.netname),sizeof(name));
			output = Q_StrReplace(g_ks_ends[i]->skmessage, "[n]", name);
			output = Q_StrReplace(output, "[k]", va("%d", number));
			
			if(g_ks_ends[i]->skdisplay[0] && !Q_stricmp(g_ks_ends[i]->skdisplay,"player")){
				entOnly = qtrue;
			}

			if(g_ks_ends[i]->skposition[0]){	
				if(!Q_stricmp(g_ks_ends[i]->skposition,"center") 
					|| !Q_stricmp(g_ks_ends[i]->skposition,"cp")){
					Q_strncpyz(pos, "cp",sizeof(pos));
				}else if(!Q_stricmp(g_ks_ends[i]->skposition,"popup") 
					|| !Q_stricmp(g_ks_ends[i]->skposition,"cpm")){
					Q_strncpyz(pos, "cpm",sizeof(pos));
				}else if(!Q_stricmp(g_ks_ends[i]->skposition,"banner") 
					|| !Q_stricmp(g_ks_ends[i]->skposition,"bp")){
					Q_strncpyz(pos, "bp",sizeof(pos));
				}else if(!Q_stricmp(g_ks_ends[i]->skposition,"console") 
					|| !Q_stricmp(g_ks_ends[i]->skposition,"print")){
					Q_strncpyz(pos, "print",sizeof(pos));
				}else{
					Q_strncpyz(pos, "chat",sizeof(pos));
				}
			}else{
				Q_strncpyz(pos, "chat",sizeof(pos));
			}
	
		}
		
		if(g_ks_ends[i]->sksound[0]){
			if(g_ks_ends[i]->skplay[0] && !Q_stricmp(g_ks_ends[i]->skplay,"envi")){
				G_AddEvent(ent, EV_GENERAL_SOUND,G_SoundIndex(va("%s",g_ks_ends[i]->sksound)));
			}else if(g_ks_ends[i]->skplay[0] && !Q_stricmp(g_ks_ends[i]->skplay,"player")){
				tent = G_TempEntity(ent->r.currentOrigin,EV_GLOBAL_CLIENT_SOUND);
				tent->s.teamNum = (ent->client - level.clients);
				tent->s.eventParm = G_SoundIndex(va("%s",g_ks_ends[i]->sksound));
			}else{
				G_globalSound(g_ks_ends[i]->sksound);
			}
        }
	}else if(OnSameTeam(ent, other)){
		if(g_ks_ends[i]->tkmessage[0]){
			
			Q_strncpyz(name, G_KillSpreeSanitize(ent->client->pers.netname),sizeof(name));
			Q_strncpyz(oname, G_KillSpreeSanitize(other->client->pers.netname),sizeof(oname));
			output = Q_StrReplace(g_ks_ends[i]->tkmessage, "[n]", name);
			output = Q_StrReplace(output, "[a]", oname);
			output = Q_StrReplace(output, "[k]", va("%d", number));
			
			if(g_ks_ends[i]->tkdisplay[0] && !Q_stricmp(g_ks_ends[i]->tkdisplay,"player")){
				entOnly = qtrue;
			}

			if(g_ks_ends[i]->tkposition[0]){	
				if(!Q_stricmp(g_ks_ends[i]->tkposition,"center") 
					|| !Q_stricmp(g_ks_ends[i]->tkposition,"cp")){
					Q_strncpyz(pos, "cp",sizeof(pos));
				}else if(!Q_stricmp(g_ks_ends[i]->tkposition,"popup") 
					|| !Q_stricmp(g_ks_ends[i]->tkposition,"cpm")){
					Q_strncpyz(pos, "cpm",sizeof(pos));
				}else if(!Q_stricmp(g_ks_ends[i]->tkposition,"banner") 
					|| !Q_stricmp(g_ks_ends[i]->tkposition,"bp")){
					Q_strncpyz(pos, "bp",sizeof(pos));
				}else if(!Q_stricmp(g_ks_ends[i]->tkposition,"console") 
					|| !Q_stricmp(g_ks_ends[i]->tkposition,"print")){
					Q_strncpyz(pos, "print",sizeof(pos));
				}else{
					Q_strncpyz(pos, "chat",sizeof(pos));
				}
			}else{
				Q_strncpyz(pos, "chat",sizeof(pos));
			}
	
		}
		
		if(g_ks_ends[i]->tksound[0]){
			if(g_ks_ends[i]->tkplay[0] && !Q_stricmp(g_ks_ends[i]->tkplay,"envi")){
				G_AddEvent(ent, EV_GENERAL_SOUND,G_SoundIndex(va("%s",g_ks_ends[i]->tksound)));
			}else if(g_ks_ends[i]->tkplay[0] && !Q_stricmp(g_ks_ends[i]->tkplay,"player")){
				tent = G_TempEntity(ent->r.currentOrigin,EV_GLOBAL_CLIENT_SOUND);
				tent->s.teamNum = (ent->client - level.clients);
				tent->s.eventParm = G_SoundIndex(va("%s",g_ks_ends[i]->tksound));
			}else{
				G_globalSound(g_ks_ends[i]->tksound);
			}
        }
	}else{

		if(g_ks_ends[i]->message[0]){
			
			Q_strncpyz(name, G_KillSpreeSanitize(ent->client->pers.netname),sizeof(name));
			Q_strncpyz(oname, G_KillSpreeSanitize(other->client->pers.netname),sizeof(oname));
			output = Q_StrReplace(g_ks_ends[i]->message, "[n]", name);
			if(number < 0){
				output = Q_StrReplace(output, "[v]", oname);
			}else{
				output = Q_StrReplace(output, "[a]", oname);
			}
			output = Q_StrReplace(output, "[k]", va("%d", abs(number)));
			output = Q_StrReplace(output, "[d]", va("%d", abs(number)));
			
			if(g_ks_ends[i]->display[0] && !Q_stricmp(g_ks_ends[i]->display,"player")){
				entOnly = qtrue;
			}

			if(g_ks_ends[i]->position[0]){	
				if(!Q_stricmp(g_ks_ends[i]->position,"center") 
					|| !Q_stricmp(g_ks_ends[i]->position,"cp")){
					Q_strncpyz(pos, "cp",sizeof(pos));
				}else if(!Q_stricmp(g_ks_ends[i]->position,"popup") 
					|| !Q_stricmp(g_ks_ends[i]->position,"cpm")){
					Q_strncpyz(pos, "cpm",sizeof(pos));
				}else if(!Q_stricmp(g_ks_ends[i]->position,"banner") 
					|| !Q_stricmp(g_ks_ends[i]->position,"bp")){
					Q_strncpyz(pos, "bp",sizeof(pos));
				}else if(!Q_stricmp(g_ks_ends[i]->position,"console") 
					|| !Q_stricmp(g_ks_ends[i]->position,"print")){
					Q_strncpyz(pos, "print",sizeof(pos));
				}else{
					Q_strncpyz(pos, "chat",sizeof(pos));
				}
			}else{
				Q_strncpyz(pos, "chat",sizeof(pos));
			}
	
		}
		
		if(g_ks_ends[i]->sound[0]){
			if(g_ks_ends[i]->play[0] && !Q_stricmp(g_ks_ends[i]->play,"envi")){
				G_AddEvent(ent, EV_GENERAL_SOUND,G_SoundIndex(va("%s",g_ks_ends[i]->sound)));
			}else if(g_ks_ends[i]->play[0] && !Q_stricmp(g_ks_ends[i]->play,"player")){
				tent = G_TempEntity(ent->r.currentOrigin,EV_GLOBAL_CLIENT_SOUND);
				tent->s.teamNum = (ent->client - level.clients);
				tent->s.eventParm = G_SoundIndex(va("%s",g_ks_ends[i]->sound));
			}else{
				G_globalSound(g_ks_ends[i]->sound);
			}
        }
	}
	

	if(entOnly == qtrue){
		if(!Q_stricmp(pos,"cp")){
			CP(va("cp \"%s\"",output));
		}else if(!Q_stricmp(pos,"cpm")){
			CP(va("cpm \"%s\"",output));
		}else if(!Q_stricmp(pos,"bp")){
			CP(va("bp \"%s\"",output));
		}else if(!Q_stricmp(pos,"print")){
			CP(va("print \"%s\n\"",output));
		}else{
			CP(va("chat \"%s\"",output));
		}
	}else{
		if(!Q_stricmp(pos,"cp")){
			AP(va("cp \"%s\"",output));
		}else if(!Q_stricmp(pos,"cpm")){
			AP(va("cpm \"%s\"",output));
		}else if(!Q_stricmp(pos,"bp")){
			AP(va("bp \"%s\"",output));
		}else if(!Q_stricmp(pos,"print")){
			AP(va("print \"%s\n\"",output));
		}else{
			AP(va("chat \"%s\"",output));
		}
	}
	G_check_killing_spree_end(ent,other,number,structure + 1);
	return;
}

// Dens: needed for delayed multikill display
int G_highest_multikill(gentity_t *ent)
{
	int i, highest = 0, number;

	number = ent->client->pers.multikill_count;

	for(i=0; g_multiKills[i]; i++){
		if(!g_multiKills[i]->number){
			continue;
		}
		if(number >= g_multiKills[i]->number && g_multiKills[i]->number > highest ){
			highest = g_multiKills[i]->number;
		}
	}
	return highest;
}

void G_check_multikill(gentity_t *ent, int number)
{
	int i;
	gentity_t *tent;
	char name[MAX_NAME_LENGTH] = {"*unknown*"};
	char *output;

	for(i=0; g_multiKills[i]; i++){
		
		if(!g_multiKills[i]->number){
			continue;
		}

		if(g_multiKills[i]->number == number){
			if(g_multiKills[i]->message[0]){
				
				Q_strncpyz(name,
					G_KillSpreeSanitize(ent->client->pers.netname),
					sizeof(name));
				output = Q_StrReplace(g_multiKills[i]->message, "[n]", name);

				if(g_multiKills[i]->display[0] && !Q_stricmp(g_multiKills[i]->display,"player")){
					if(g_multiKills[i]->position[0]){
						
						if(!Q_stricmp(g_multiKills[i]->position,"center") 
							|| !Q_stricmp(g_multiKills[i]->position,"cp")){
							CP(va("cp \"%s\"",output));
						}else if(!Q_stricmp(g_multiKills[i]->position,"popup") 
							|| !Q_stricmp(g_multiKills[i]->position,"cpm")){
							CP(va("cpm \"%s\"",output));
						}else if(!Q_stricmp(g_multiKills[i]->position,"banner") 
							|| !Q_stricmp(g_multiKills[i]->position,"bp")){
							CP(va("bp \"%s\"",output));
						}else if(!Q_stricmp(g_multiKills[i]->position,"console") 
							|| !Q_stricmp(g_multiKills[i]->position,"print")){
							CP(va("print \"%s\n\"", output));
						}else{
							CP(va("chat \"%s\"",output));
						}
					}else{
						CP(va("chat \"%s\"",output));
					}
				}else{
					if(g_multiKills[i]->position[0]){
						
						if(!Q_stricmp(g_multiKills[i]->position,"center") 
							|| !Q_stricmp(g_multiKills[i]->position,"cp")){
							AP(va("cp \"%s\"",output));
						}else if(!Q_stricmp(g_multiKills[i]->position,"popup") 
							|| !Q_stricmp(g_multiKills[i]->position,"cpm")){
							AP(va("cpm \"%s\"",output));
						}else if(!Q_stricmp(g_multiKills[i]->position,"banner") 
							|| !Q_stricmp(g_multiKills[i]->position,"bp")){
							AP(va("bp \"%s\"",output));
						}else if(!Q_stricmp(g_multiKills[i]->position,"console") 
							|| !Q_stricmp(g_multiKills[i]->position,"print")){
							AP(va("print \"%s\n\"", output));
						}else{
							AP(va("chat \"%s\"",output));
						}
					}else{
						AP(va("chat \"%s\"",output));
					}
				}
			}
			if(g_multiKills[i]->sound[0]){
				if(g_multiKills[i]->play[0] && !Q_stricmp(g_multiKills[i]->play,"envi")){
					G_AddEvent(ent, EV_GENERAL_SOUND,G_SoundIndex(va("%s",g_multiKills[i]->sound)));
				}else if(g_multiKills[i]->play[0] && !Q_stricmp(g_multiKills[i]->play,"player")){
					tent = G_TempEntity(ent->r.currentOrigin,EV_GLOBAL_CLIENT_SOUND);
					tent->s.teamNum = (ent->client - level.clients);
					tent->s.eventParm = G_SoundIndex(va("%s",g_multiKills[i]->sound));
				}else{
					G_globalSound(g_multiKills[i]->sound);
				}
			}
		}
	}
	
	return;

}

void G_displayBanner(int loop)
{
	int nextBanner;

	if(level.lastBanner < 0 || level.lastBanner >= (MAX_BANNERS - 1)){
		level.lastBanner = -1;
	}
	// Dens: prevent noob admins causing an overflow
	if(loop >= MAX_BANNERS){
		level.nextBannerTime = (level.time + 60000);
		return;
	}

	if(g_banners[level.lastBanner+1]){
		nextBanner = (level.lastBanner + 1);
	}else if(level.lastBanner != -1 && g_banners[0]){
		nextBanner = 0;
	}else{
		// Dens: no banners found, try again in 10 secs
		level.nextBannerTime = (level.time + 10000);
		return;
	}

	if(g_banners[nextBanner]->message[0]){
		if(g_banners[nextBanner]->position[0]){
			if(!Q_stricmp(g_banners[nextBanner]->position,"center") 
				|| !Q_stricmp(g_banners[nextBanner]->position,"cp")){
				AP(va("cp \"%s\"",g_banners[nextBanner]->message));
			}else if(!Q_stricmp(g_banners[nextBanner]->position,"popup") 
				|| !Q_stricmp(g_banners[nextBanner]->position,"cpm")){
				AP(va("cpm \"%s\"",g_banners[nextBanner]->message));
			}else if(!Q_stricmp(g_banners[nextBanner]->position,"banner") 
				|| !Q_stricmp(g_banners[nextBanner]->position,"bp")){
				AP(va("bp \"%s\"",g_banners[nextBanner]->message));
			}else if(!Q_stricmp(g_banners[nextBanner]->position,"console") 
				|| !Q_stricmp(g_banners[nextBanner]->position,"print")){
				AP(va("print \"%s\n\"",g_banners[nextBanner]->message));
			}else{
				AP(va("chat \"%s\"",g_banners[nextBanner]->message));
			}
		}else{
			AP(va("chat \"%s\"",g_banners[nextBanner]->message));
		}
	}

	level.lastBanner = nextBanner;

	if(g_banners[nextBanner]->wait > 0){
		level.nextBannerTime = (level.time + 1000*g_banners[nextBanner]->wait);
	}else if(g_banners[nextBanner]->wait == 0){
		G_displayBanner(loop+1);
	}else{
		// Dens: bad config, show next banner in 10 secs
		level.nextBannerTime = (level.time + 10000);
	}
	return;
}

void G_check_revive_spree(gentity_t *ent, int number)
{
	int i;
	gentity_t *tent;
	char name[MAX_NAME_LENGTH] = {"*unknown*"};
	char *output;

	for(i=0; g_reviveSprees[i]; i++) {
		if(!g_reviveSprees[i]->number) {
		 	continue;
		}

		if(g_reviveSprees[i]->number == number) {
			if(g_reviveSprees[i]->message[0]) {
				Q_strncpyz(name,
					G_KillSpreeSanitize(ent->client->pers.netname),
					sizeof(name));
				output = Q_StrReplace(g_reviveSprees[i]->message, "[n]", name);

				if(g_reviveSprees[i]->display[0] && !Q_stricmp(g_reviveSprees[i]->display,"player")){
					if(g_reviveSprees[i]->position[0]){

						if(!Q_stricmp(g_reviveSprees[i]->position,"center")
							|| !Q_stricmp(g_reviveSprees[i]->position,"cp")){
							CP(va("cp \"%s\"",output));
						}else if(!Q_stricmp(g_reviveSprees[i]->position,"popup")
							|| !Q_stricmp(g_reviveSprees[i]->position,"cpm")){
							CP(va("cpm \"%s\"",output));
						}else if(!Q_stricmp(g_reviveSprees[i]->position,"banner")
							|| !Q_stricmp(g_reviveSprees[i]->position,"bp")){
							CP(va("bp \"%s\"",output));
						}else if(!Q_stricmp(g_reviveSprees[i]->position,"console")
							|| !Q_stricmp(g_reviveSprees[i]->position,"print")){
							CP(va("print \"%s\n\"", output));
						}else{
							CP(va("chat \"%s\"",output));
						}
					}else{
						CP(va("chat \"%s\"",output));
					}
				}else{
					if(g_reviveSprees[i]->position[0]){

						if(!Q_stricmp(g_reviveSprees[i]->position,"center")
							|| !Q_stricmp(g_reviveSprees[i]->position,"cp")){
							AP(va("cp \"%s\"",output));
						}else if(!Q_stricmp(g_reviveSprees[i]->position,"popup")
							|| !Q_stricmp(g_reviveSprees[i]->position,"cpm")){
							AP(va("cpm \"%s\"",output));
						}else if(!Q_stricmp(g_reviveSprees[i]->position,"banner")
							|| !Q_stricmp(g_reviveSprees[i]->position,"bp")){
							AP(va("bp \"%s\"",output));
						}else if(!Q_stricmp(g_reviveSprees[i]->position,"console")
							|| !Q_stricmp(g_reviveSprees[i]->position,"print")){
							AP(va("print \"%s\n\"", output));
						}else{
							AP(va("chat \"%s\"",output));
						}
					}else{
						AP(va("chat \"%s\"",output));
					}
				}
			}
			if(g_reviveSprees[i]->sound[0]){
				if(g_reviveSprees[i]->play[0] && !Q_stricmp(g_reviveSprees[i]->play,"envi")){
					G_AddEvent(ent, EV_GENERAL_SOUND,G_SoundIndex(va("%s",g_reviveSprees[i]->sound)));
				}else if(g_reviveSprees[i]->play[0] && !Q_stricmp(g_reviveSprees[i]->play,"player")){
					tent = G_TempEntity(ent->r.currentOrigin,EV_GLOBAL_CLIENT_SOUND);
					tent->s.teamNum = (ent->client - level.clients);
					tent->s.eventParm = G_SoundIndex(va("%s",g_reviveSprees[i]->sound));
				}else{
					G_globalSound(g_reviveSprees[i]->sound);
				}
			}
		}
	}
	return;
}
