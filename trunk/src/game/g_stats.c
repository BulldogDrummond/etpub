#include "g_local.h"

#ifdef LUA_SUPPORT
#include "g_lua.h"
#endif // LUA_SUPPORT

void G_LogDeath( gentity_t* ent, weapon_t weap ) {
	weap = BG_DuplicateWeapon(weap);

	if(!ent->client) {
		return;
	}

	ent->client->pers.playerStats.weaponStats[weap].killedby++;

	trap_PbStat ( ent - g_entities , "death" , 
		va ( "%d %d %d" , ent->client->sess.sessionTeam , ent->client->sess.playerType , weap ) ) ;
}

void G_LogKill( gentity_t* ent, weapon_t weap ) {
	weap = BG_DuplicateWeapon(weap);

	if(!ent->client) {
		return;
	}

	// CHRUKER: b068 - This block didn't do anything, so it has been
	//          commented out.
	/* 
	if(ent->client->sess.playerType == PC_SOLDIER) {
		int i, j;
		qboolean pass = qtrue;

		ent->client->soliderKillTimes[ent->client->soldierKillMarker++] = level.timeCurrent;

		if ( ent->client->soldierKillMarker >= NUM_SOLDIERKILL_TIMES ) {
			ent->client->soldierKillMarker = 0;
		}

		for( i = 0, j = ent->client->soldierKillMarker; i < NUM_SOLDIERKILL_TIMES; i++ ) {

			if( !ent->client->soliderKillTimes[j] || (ent->client->soliderKillTimes[j] < level.timeCurrent - SOLDIERKILL_MAXTIME) ) {
				pass = qfalse;
				break;
			}

			if( ++j == NUM_SOLDIERKILL_TIMES ) {
				j = 0;
			}
		}
	}
	*/

	ent->client->pers.playerStats.weaponStats[weap].kills++;

	trap_PbStat ( ent - g_entities , "kill" , 
		va ( "%d %d %d" , ent->client->sess.sessionTeam , ent->client->sess.playerType , weap ) ) ;
}

void G_LogTeamKill( gentity_t* ent, weapon_t weap ) {
	weap = BG_DuplicateWeapon(weap);

	if(!ent->client) {
		return;
	}

	ent->client->pers.playerStats.weaponStats[weap].teamkills++;

	trap_PbStat ( ent - g_entities , "tk" , 
		va ( "%d %d %d" , ent->client->sess.sessionTeam , ent->client->sess.playerType , weap ) ) ;
}

void G_LogRegionHit( gentity_t* ent, hitRegion_t hr ) {
	if(!ent->client) {
		return;
	}
	ent->client->pers.playerStats.hitRegions[hr]++;

	trap_PbStat ( ent - g_entities , "hr" , 
		va ( "%d %d %d" , ent->client->sess.sessionTeam , ent->client->sess.playerType , hr ) ) ;
}

void G_PrintAccuracyLog( gentity_t *ent ) {
	int i;
	char buffer[2048];

	Q_strncpyz(buffer, "WeaponStats", 2048);

	for( i = 0; i < WP_NUM_WEAPONS; i++ ) {
		if(!BG_ValidStatWeapon(i)) {
			continue;
		}

		Q_strcat(buffer, 2048, va(" %i %i %i", 
			ent->client->pers.playerStats.weaponStats[i].kills, 
			ent->client->pers.playerStats.weaponStats[i].killedby,
			ent->client->pers.playerStats.weaponStats[i].teamkills ));
	}

	Q_strcat( buffer, 2048, va(" %i", ent->client->pers.playerStats.suicides));

	for( i = 0; i < HR_NUM_HITREGIONS; i++ ) {
		Q_strcat( buffer, 2048, va(" %i", ent->client->pers.playerStats.hitRegions[i]));
	}

	Q_strcat( buffer, 2048, va(" %i", 6/*level.numOidTriggers*/ ));

	for( i = 0; i < 6/*level.numOidTriggers*/; i++ ) {
		Q_strcat( buffer, 2048, va(" %i", ent->client->pers.playerStats.objectiveStats[i]));
		Q_strcat( buffer, 2048, va(" %i", ent->client->sess.sessionTeam == TEAM_AXIS ? level.objectiveStatsAxis[i] : level.objectiveStatsAllies[i]));
	}

	trap_SendServerCommand( ent-g_entities, buffer );
}

void G_SetPlayerScore( gclient_t *client ) {
	int i;
	// josh: a possibly buggy hack, but easy to turn off.
	// OK, I think I've covered the situations you wouldn't want
	// this to happen in.
	if ((g_serverInfo.integer & SIF_PLAYER_RATING) &&
			g_shuffle_rating.integer > SHUFR_XPRATE &&
			g_ATB_rating.integer > ATBR_XP &&
			g_gametype.integer != GT_WOLF_LMS ) {
		client->ps.persistant[PERS_SCORE] = 100
			*1.0/(1.0+exp(-client->sess.rating
			/sqrt(1.0+3.0*client->sess.rating_variance*20.0
			/(M_PI*M_PI))));
	} else if ((g_serverInfo.integer & SIF_KILL_RATING) &&
			g_shuffle_rating.integer > SHUFR_XPRATE &&
			g_ATB_rating.integer > ATBR_XP &&
			g_gametype.integer != GT_WOLF_LMS ) {
		client->ps.persistant[PERS_SCORE] = client->sess.overall_killrating;
	} else {
		for( client->ps.persistant[PERS_SCORE] = 0, i = 0; i < SK_NUM_SKILLS; i++ ) {
			client->ps.persistant[PERS_SCORE] += client->sess.skillpoints[i];
		}
	}
}

void G_SetPlayerSkill( gclient_t *client, skillType_t skill ) {
	int i;
	if (g_noSkillUpgrades.integer)
		return;

#ifdef LUA_SUPPORT
	// Lua API callbacks
	if( G_LuaHook_SetPlayerSkill( client - level.clients, skill ) ) {
		return;
	}
#endif // LUA_SUPPORT

	for( i = NUM_SKILL_LEVELS - 1; i >= 0; i-- ) {
		if( client->sess.skillpoints[skill] >= skillLevels[skill][i] ) {
			client->sess.skill[skill] = i;
			break;
		}
	}

	G_SetPlayerScore( client );
}

extern qboolean AddWeaponToPlayer( gclient_t *client, weapon_t weapon, int ammo, int ammoclip, qboolean setcurrent );

// TAT 11/6/2002
//		Local func to actual do skill upgrade, used by both MP skill system, and SP scripted skill system
static void G_UpgradeSkill( gentity_t *ent, skillType_t skill ) {
	int i, cnt = 0;
	clientSession_t *ci;

	ci = &ent->client->sess;
	// See if this is the first time we've reached this skill level
	for( i = 0; i < SK_NUM_SKILLS; i++ ) {
		if( i == skill )
			continue;

		if( ci->skill[skill] <= ci->skill[i] )
			break;
	}

#ifdef LUA_SUPPORT
	// Lua API callbacks
	if( G_LuaHook_UpgradeSkill( g_entities - ent, skill ) ) {
		return;
	}
#endif // LUA_SUPPORT

	G_DebugAddSkillLevel( ent, skill );

	if( i == SK_NUM_SKILLS ) {
		// increase rank
		ci->rank++;
	}

	if( ent->client->sess.rank >=4 ) {
		// Gordon: count the number of maxed out skills
		for( i = 0; i < SK_NUM_SKILLS; i++ ) {
			if( ci->skill[ i ] >= 4 ) {
				cnt++;
			}
		}

		ci->rank = cnt + 3;
		if( ci->rank > 10 ) {
			ci->rank = 10;
		}
	}

	// Give em rightaway
	if( skill == SK_BATTLE_SENSE && ci->skill[skill] == 1 ) {
		if( AddWeaponToPlayer( ent->client, WP_BINOCULARS, 1, 0, qfalse ) ) {
			ent->client->ps.stats[STAT_KEYS] |= ( 1 << INV_BINOCS );
		}
	} else if( skill == SK_FIRST_AID && ci->playerType == PC_MEDIC && 
			ci->skill[skill] == 4 && !(g_medics.integer & MEDIC_NOSELFADREN)) {
		AddWeaponToPlayer( 
			ent->client, 
			WP_MEDIC_ADRENALINE, 
			ent->client->ps.ammo[BG_FindAmmoForWeapon(WP_MEDIC_ADRENALINE)], 
			ent->client->ps.ammoclip[BG_FindClipForWeapon(WP_MEDIC_ADRENALINE)],
			qfalse 
		);
	} 
	// kw: select the newly gained weapons for the next spawn
	else if( skill == SK_HEAVY_WEAPONS && ci->skill[skill] == 4 ) {
		if( ci->sessionTeam == TEAM_AXIS ) {
			ci->latchPlayerWeapon2 = WP_MP40;
		} else {
			ci->latchPlayerWeapon2 = WP_THOMPSON;
		}
	} else if( skill == SK_LIGHT_WEAPONS && ci->skill[skill] == 4 ) {
		// don't select the akimbo if you already have the smg
		if( ci->skill[SK_HEAVY_WEAPONS] < 4 || 
				ci->latchPlayerType != PC_SOLDIER ) {
			if( ci->sessionTeam == TEAM_AXIS ) {
				ci->latchPlayerWeapon2 = WP_AKIMBO_LUGER;
			} else {
				ci->latchPlayerWeapon2 = WP_AKIMBO_COLT;
			}
		}
	}

	ClientUserinfoChanged( ent-g_entities );
}

qboolean G_LoseSkillPointsExt(gentity_t *ent, skillType_t skill, float points)
{
	int oldskill;
	float oldskillpoints;
	
	if(!ent || !ent->client) {
		return qfalse;
	}

	if( g_gametype.integer == GT_WOLF_LMS ) {
		// Gordon: no xp in LMS
		return qfalse;
	}


	oldskillpoints = ent->client->sess.skillpoints[skill];
	ent->client->sess.skillpoints[skill] -= points;

	// see if player increased in skill
	oldskill = ent->client->sess.skill[skill];
	G_SetPlayerSkill( ent->client, skill );
	if(oldskill != ent->client->sess.skill[skill] &&
		ent->client->pers.connected == CON_CONNECTED) {

		G_UpgradeSkill(ent, skill);
	}
	level.teamScores[ent->client->ps.persistant[PERS_TEAM]] -= 
		oldskillpoints - ent->client->sess.skillpoints[skill];
	level.teamXP[skill][ent->client->sess.sessionTeam - TEAM_AXIS] -=
		oldskillpoints - ent->client->sess.skillpoints[skill];
	return qtrue;
}


void G_LoseSkillPoints(gentity_t *ent, skillType_t skill, float points)
{
	float oldskillpoints;

	if(!ent || !ent->client)
		return;
	

	if(ent->client->sess.sessionTeam != TEAM_AXIS &&
		ent->client->sess.sessionTeam != TEAM_ALLIES) {

		return;
	}

	// no skill loss during warmup
	if( g_gamestate.integer != GS_PLAYING ) {
		return;
	}
	
	oldskillpoints = ent->client->sess.skillpoints[skill];

	if(!G_LoseSkillPointsExt(ent, skill, points))
		return; 
	// CHRUKER: b013 - Was printing this with many many decimals
	G_Printf( "%s just lost %.0f skill points for skill %s\n",
		ent->client->pers.netname,
		oldskillpoints - ent->client->sess.skillpoints[skill],
		skillNames[skill]);

	trap_PbStat ((ent - g_entities), "loseskill" , 
		va("%d %d %d %f",
			ent->client->sess.sessionTeam,
			ent->client->sess.playerType, 
			skill,
			oldskillpoints - ent->client->sess.skillpoints[skill]));
}

void G_XPDecay(gentity_t *ent, int seconds, qboolean force)
{
	int i =  0;
	float points;
	float p;
	int divisor;
	float decayRate;
	// translates primary SK_ enum for each PC_ define
	//int skill_for_class[] = {5,2,1,3,6};
	int skill_for_class[] = {
		// PC_SOLDIER
		SK_HEAVY_WEAPONS,
		// PC_MEDIC
		SK_FIRST_AID,
		// PC_ENGINEER
		SK_EXPLOSIVES_AND_CONSTRUCTION,
		// PC_FIELDOPS
		SK_SIGNALS,
		// PC_COVERTOPS
		SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS
	};

	if(!(g_XPDecay.integer & XPDF_ENABLE))
		return;
	if(g_XPDecayRate.value == 0.0f)
		return;

	divisor = 1;
	if (strlen(g_XPDecayRate.string) > 2) {
		char *startComp = g_XPDecayRate.string + strlen(g_XPDecayRate.string) - 2;

		if (startComp[0] == '/') {
			switch(startComp[1]) {
				// Months
				case 'O':
				case 'o':
					divisor *= 4;

				// Weeks
				case 'W':
				case 'w':
					divisor *= 7;

				// Days
				case 'D':
				case 'd':
					divisor *= 24;

				// Hours
				case 'H':
				case 'h':
					divisor *= 60;

				// Minutes
				case 'M':
				case 'm':
					divisor *= 60;

					break;
			}
		}
	}

	decayRate = g_XPDecayRate.value / divisor;

	// tjw: state matters if XPSave is not forcing this decay
	if(!force) {
		if((g_XPDecay.integer & XPDF_NO_SPEC_DECAY) &&
			ent->client->sess.sessionTeam == TEAM_SPECTATOR) {

			return;
		}
		if((g_XPDecay.integer & XPDF_NO_GAMESTATE_DECAY) &&
			g_gamestate.integer != GS_PLAYING) {

			return;
		}
		if((g_XPDecay.integer & XPDF_NO_PLAYING_DECAY) &&
			g_gamestate.integer == GS_PLAYING &&
			(ent->client->sess.sessionTeam == TEAM_AXIS ||
			 ent->client->sess.sessionTeam == TEAM_ALLIES)) {

			return;
		}

	}

 	points = (decayRate * seconds);
	for(i=0; i<SK_NUM_SKILLS; i++) {
		if(!force && (g_XPDecay.integer & XPDF_NO_CLASS_DECAY) &&
			skill_for_class[ent->client->sess.playerType] == i) {
		
			continue;
		}
		else if(!force && (g_XPDecay.integer & XPDF_NO_BS_DECAY) &&
			i == SK_BATTLE_SENSE) {

			continue;
		}
		else if(!force && (g_XPDecay.integer & XPDF_NO_LW_DECAY) &&
			i == SK_LIGHT_WEAPONS) {

			continue;
		}

		p = points;
		// tjw: don't let xp be added
		if(g_XPDecayFloor.value < 0.0f)
			continue;
		// tjw: don't allow xp to be added up to the floor
		if(ent->client->sess.skillpoints[i] < g_XPDecayFloor.value)
			continue;
		// tjw: don't decay past floor
		if((ent->client->sess.skillpoints[i] - p) <
			(g_XPDecayFloor.value * 1.0f)) {

			p = (ent->client->sess.skillpoints[i] - 
				(g_XPDecayFloor.value * 1.0f));
		}
		// tjw: don't decay past 0
		if(ent->client->sess.skillpoints[i] < p)
			p = ent->client->sess.skillpoints[i];

		G_LoseSkillPointsExt(ent, i, p);
	}	
}

void G_ResetXP(gentity_t *ent)
{
	int i = 0;
	int	ammo[MAX_WEAPONS];
	int	ammoclip[MAX_WEAPONS];

	if(!ent || !ent->client)
		return;
	ent->client->sess.rank = 0;
	for(i = 0; i < SK_NUM_SKILLS; i++) {
		ent->client->sess.skillpoints[i] = 0.0f;
		ent->client->sess.skill[i] = 0;
	}
	G_CalcRank(ent->client);
	ent->client->ps.stats[STAT_XP] = 0;
	ent->client->ps.persistant[PERS_SCORE] = 0;

	// tjw: zero out all weapons and grab the default weapons for
	//      a player of this XP level.
	memset(ent->client->ps.weapons, 0,
		sizeof(*ent->client->ps.weapons));
	memcpy(ammo, ent->client->ps.ammo, sizeof(ammo));
	memcpy(ammoclip, ent->client->ps.ammoclip, sizeof(ammoclip));
	SetWolfSpawnWeapons(ent->client);
	memcpy(ent->client->ps.ammo, ammo, sizeof(ammo));
	memcpy(ent->client->ps.ammoclip, ammoclip, sizeof(ammoclip));
	ClientUserinfoChanged(ent-g_entities);
}

void G_AddSkillPoints( gentity_t *ent, skillType_t skill, float points ) {
	int oldskill,i,totalXP = 0;

	if( !ent->client ) {
		return;
	}

	// no skill gaining during warmup
	if( g_gamestate.integer != GS_PLAYING ) {
		return;
	}

	if( ent->client->sess.sessionTeam != TEAM_AXIS && ent->client->sess.sessionTeam != TEAM_ALLIES ) {
		return;
	}

	if( g_gametype.integer == GT_WOLF_LMS ) {
		return; // Gordon: no xp in LMS
	}

	level.teamXP[ skill ][ ent->client->sess.sessionTeam - TEAM_AXIS ] += points;

	ent->client->sess.skillpoints[skill] += points;

	// josh: added this in case they're using PLAYER_RATING_SCOREBOARD
	for( i = 0; i < SK_NUM_SKILLS; i++ ) {
		totalXP += ent->client->sess.skillpoints[i];
	}
	if(g_maxXP.integer >= 0 &&
		totalXP  >= g_maxXP.integer) {

		G_ResetXP(ent);
		
		// tjw: be silent if g_maxXP is 0
		if(g_maxXP.integer)
			CP(va("cp \"^1YOUR XP HAS BEEN RESET \n^7You reached "
			"the maximum XP in this server: ^1%i XP\" 1", g_maxXP.integer));
		return;
	}

	// Reset XP for bots if enabled
	if (ent->r.svFlags & SVF_BOT) {
		if ((g_bot_maxXP.integer >= 0) && (totalXP >= g_bot_maxXP.integer)) {
			G_ResetXP(ent);
		}
	}

	level.teamScores[ ent->client->ps.persistant[PERS_TEAM] ] += points;
	// CHRUKER: b013 - Was printing this with many many decimals
//	G_Printf( "%s just got %.0f skill points for skill %s\n", ent->client->pers.netname, points, skillNames[skill] );

	trap_PbStat ( ent - g_entities , "addskill" , 
		va ( "%d %d %d %f" , ent->client->sess.sessionTeam , ent->client->sess.playerType , 
			skill , points ) ) ;

	// see if player increased in skill
	oldskill = ent->client->sess.skill[skill];
	G_SetPlayerSkill( ent->client, skill );
	if( oldskill != ent->client->sess.skill[skill] ) {
		// TAT - call the new func that encapsulates the skill giving behavior
		G_UpgradeSkill( ent, skill );
		G_UpdateSkillTime(ent, qfalse);
	}
}

void G_LoseKillSkillPoints( gentity_t *tker, meansOfDeath_t mod, hitRegion_t hr, qboolean splash ) {
	// for evil tkers :E

	if( !tker->client ) {
		return;
	}

	switch( mod ) {
		// light weapons
		case MOD_KNIFE:
		case MOD_LUGER:
		case MOD_COLT:
		case MOD_MP40:
		case MOD_THOMPSON:
		case MOD_STEN:
		case MOD_GARAND:
		case MOD_SILENCER:
		case MOD_FG42:
//		case MOD_FG42SCOPE:
		case MOD_CARBINE:
		case MOD_KAR98:
		case MOD_SILENCED_COLT:
		case MOD_K43:
//bani - akimbo weapons lose score now as well
		case MOD_AKIMBO_COLT:
		case MOD_AKIMBO_LUGER:
		case MOD_AKIMBO_SILENCEDCOLT:
		case MOD_AKIMBO_SILENCEDLUGER:
		case MOD_GRENADE_LAUNCHER:
		case MOD_GRENADE_PINEAPPLE:
//bani - airstrike marker kills
		case MOD_SMOKEGRENADE:
			G_LoseSkillPoints( tker, SK_LIGHT_WEAPONS, 3.f ); 
//			G_DebugAddSkillPoints( attacker, SK_LIGHT_WEAPONS, 2.f, "kill" );
			break;

		// scoped weapons
		case MOD_GARAND_SCOPE:
		case MOD_K43_SCOPE:
		case MOD_FG42SCOPE:
		case MOD_SATCHEL:
			G_LoseSkillPoints( tker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, 3.f );
//			G_DebugAddSkillPoints( attacker, SK_LIGHT_WEAPONS, 2.f, "legshot kill" );
			break;

		case MOD_MOBILE_MG42:
		case MOD_MACHINEGUN:
		case MOD_BROWNING:
		case MOD_MG42:
		case MOD_PANZERFAUST:
		case MOD_FLAMETHROWER:
		case MOD_MORTAR:
			G_LoseSkillPoints( tker, SK_HEAVY_WEAPONS, 3.f );
//			G_DebugAddSkillPoints( attacker, SK_HEAVY_WEAPONS, 3.f, "emplaced mg42 kill" );
			break;

		case MOD_DYNAMITE:
		case MOD_LANDMINE:
		case MOD_GPG40:
		case MOD_M7:
			G_LoseSkillPoints( tker, SK_EXPLOSIVES_AND_CONSTRUCTION, 3.f );
//			G_DebugAddSkillPoints( attacker, SK_EXPLOSIVES_AND_CONSTRUCTION, 4.f, "dynamite or landmine kill" );
			break;

		case MOD_ARTY:
		case MOD_AIRSTRIKE:
			G_LoseSkillPoints( tker, SK_SIGNALS, 3.f );
//			G_DebugAddSkillPoints( attacker, SK_SIGNALS, 4.f, "artillery kill" );
			break;

		// no skills for anything else
		default:
			break;
	}
}

void G_AddDamageXP ( gentity_t *attacker, meansOfDeath_t mod)
{
	int ratio;
	int score_to_give;
	float pta; 

	if (!(attacker)) {return; }

	if (g_damageXP.integer == 0){
		return;
	}

	if (attacker->client->sess.XPdmg >= g_damageXPLevel.integer ) 
	{
		// give 1 xp per XPLevel points of damage
		ratio = g_damageXPLevel.integer;  
		if (ratio <= 1) {ratio = 1;}
		score_to_give = attacker->client->sess.XPdmg / ratio ;
		attacker->client->sess.XPdmg -= (score_to_give * ratio);
		// add the xp
		if (score_to_give >= 2) { score_to_give = 2; }
	} else {
		// nothing to do
		return;
	}

	pta = score_to_give;

	// add xp-based skill points to battle sense
	if (g_damageXP.integer == 2) {
		G_AddSkillPoints( attacker, SK_BATTLE_SENSE, pta ); 
		return;
	}

	// add xp-based skill points to the appropriate skill
	switch (mod)
	{
		// light weapons SK_LIGHT_WEAPONS
		case MOD_THROWN_KNIFE:
		case MOD_KNIFE:
		case MOD_LUGER:
		case MOD_COLT:
		case MOD_MP40:
		case MOD_THOMPSON:
		case MOD_STEN:
		case MOD_GARAND:
		case MOD_SILENCER:
		case MOD_FG42:
		case MOD_CARBINE:
		case MOD_KAR98:
		case MOD_SILENCED_COLT:
		case MOD_K43:
		case MOD_AKIMBO_COLT:
		case MOD_AKIMBO_LUGER:
		case MOD_AKIMBO_SILENCEDCOLT:
		case MOD_AKIMBO_SILENCEDLUGER:
		case MOD_GRENADE_LAUNCHER:
		case MOD_GRENADE_PINEAPPLE:
		case MOD_SMOKEGRENADE:
			G_AddSkillPoints( attacker, SK_LIGHT_WEAPONS, pta );
			break;

		// heavy weapons SK_HEAVY_WEAPONS
		case MOD_MOBILE_MG42:
		case MOD_MACHINEGUN:
		case MOD_BROWNING:
		case MOD_MG42:
		case MOD_PANZERFAUST:
		case MOD_FLAMETHROWER:
		case MOD_MORTAR:
			G_AddSkillPoints( attacker, SK_HEAVY_WEAPONS, pta );
			break;

		// SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS
		case MOD_GARAND_SCOPE:
		case MOD_K43_SCOPE:
		case MOD_FG42SCOPE:
		case MOD_SATCHEL:
			G_AddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, pta );
			break;

		// SK_SIGNALS
		case MOD_ARTY:
		case MOD_AIRSTRIKE:
			G_AddSkillPoints( attacker, SK_SIGNALS, pta );
			break;
		
		// SK_EXPLOSIVES_AND_CONSTRUCTION
		case MOD_DYNAMITE:
		case MOD_LANDMINE:
		case MOD_GPG40:
		case MOD_M7:
			G_AddSkillPoints( attacker, SK_EXPLOSIVES_AND_CONSTRUCTION, pta );
			break;

		// SK_FIRST_AID
		case MOD_POISON:
			G_AddSkillPoints( attacker, SK_FIRST_AID, pta );
			break;

		default:
			break;

	}

}


void G_AddKillSkillPoints( gentity_t *attacker, meansOfDeath_t mod, hitRegion_t hr, qboolean splash ) {
	
	float pta;
	
	if( !attacker->client )
		return;
	
	// perro: damage-based XP
	// pre-set the default amount of xp to award.  It will be over-ridden below
	// in special cases (headshots, etc).  If g_damageXP = 1, kills will each earn 1 pt.
	// if g_damageXP is anything else, kills will earn normal ET amount.

	pta = (g_damageXP.integer != 1) ? 3.f : 1.f;

	switch( mod ) 
	{
		// light weapons
		case MOD_THROWN_KNIFE:
		case MOD_KNIFE:
			G_AddSkillPoints( attacker, SK_LIGHT_WEAPONS, pta ); 
			G_DebugAddSkillPoints( attacker, SK_LIGHT_WEAPONS, pta, "knife kill" ); 
			break;

		case MOD_LUGER:
		case MOD_COLT:
		case MOD_MP40:
		case MOD_THOMPSON:
		case MOD_STEN:
		case MOD_GARAND:
		case MOD_SILENCER:
		case MOD_FG42:
//		case MOD_FG42SCOPE:
		case MOD_CARBINE:
		case MOD_KAR98:
		case MOD_SILENCED_COLT:
		case MOD_K43:
		case MOD_AKIMBO_COLT:
		case MOD_AKIMBO_LUGER:
		case MOD_AKIMBO_SILENCEDCOLT:
		case MOD_AKIMBO_SILENCEDLUGER:
			switch( hr ) {
				case HR_HEAD:	
					pta = (g_damageXP.integer != 1) ? 5.f : 1.f;
					G_AddSkillPoints( attacker, SK_LIGHT_WEAPONS, pta ); 
					G_DebugAddSkillPoints( attacker, SK_LIGHT_WEAPONS, pta, "headshot kill" ); 
					break;
				case HR_ARMS:	
					G_AddSkillPoints( attacker, SK_LIGHT_WEAPONS, pta ); 
					G_DebugAddSkillPoints( attacker, SK_LIGHT_WEAPONS, pta, "armshot kill" ); 
					break;
				case HR_BODY:	
					G_AddSkillPoints( attacker, SK_LIGHT_WEAPONS, pta ); 
					G_DebugAddSkillPoints( attacker, SK_LIGHT_WEAPONS, pta, "bodyshot kill" ); 
					break;
				case HR_LEGS:	
					G_AddSkillPoints( attacker, SK_LIGHT_WEAPONS, pta ); 
					G_DebugAddSkillPoints( attacker, SK_LIGHT_WEAPONS, pta, "legshot kill" );	
					break;
				default:		
					G_AddSkillPoints( attacker, SK_LIGHT_WEAPONS, pta ); 
					G_DebugAddSkillPoints( attacker, SK_LIGHT_WEAPONS, pta, "kill" ); 
					break;	// for weapons that don't have localized damage
			}
			break;

		// heavy weapons
		case MOD_MOBILE_MG42:
			G_AddSkillPoints( attacker, SK_HEAVY_WEAPONS, pta );
			G_DebugAddSkillPoints( attacker, SK_HEAVY_WEAPONS, pta, "mobile mg42 kill" ); 
			break;

		// scoped weapons
		case MOD_GARAND_SCOPE:
		case MOD_K43_SCOPE:
		case MOD_FG42SCOPE:
			switch( hr ) {
				case HR_HEAD:	
					pta = (g_damageXP.integer != 1) ? 5.f : 1.f;
					G_AddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, pta ); 
					G_DebugAddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, pta, "headshot kill" ); 
					break;
				case HR_ARMS:	
					G_AddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, pta ); 
					G_DebugAddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, pta, "armshot kill" ); 
					break;
				case HR_BODY:	
					G_AddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, pta ); 
					G_DebugAddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, pta, "bodyshot kill" ); 
					break;
				case HR_LEGS:	
					G_AddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, pta ); 
					G_DebugAddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, pta, "legshot kill" ); 
					break;
				default:		
					G_AddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, pta ); 
					G_DebugAddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, pta, "kill" );
					break;	// for weapons that don't have localized damage
			}
			break;

		// misc weapons (individual handling)
		case MOD_SATCHEL:
			G_AddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, pta );
			G_DebugAddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, pta, "satchel charge kill" );
			break;

		case MOD_MACHINEGUN:
		case MOD_BROWNING:
		case MOD_MG42:
			G_AddSkillPoints( attacker, SK_HEAVY_WEAPONS, pta );
			G_DebugAddSkillPoints( attacker, SK_HEAVY_WEAPONS, pta, "emplaced machinegun kill" );
			break;

		case MOD_PANZERFAUST:
			if( splash ) {
				G_AddSkillPoints( attacker, SK_HEAVY_WEAPONS, pta );
				G_DebugAddSkillPoints( attacker, SK_HEAVY_WEAPONS, pta, "panzerfaust splash damage kill" );
			} else {
				G_AddSkillPoints( attacker, SK_HEAVY_WEAPONS, pta );
				G_DebugAddSkillPoints( attacker, SK_HEAVY_WEAPONS, pta, "panzerfaust direct hit kill" );
			}
			break;
		case MOD_FLAMETHROWER:
			G_AddSkillPoints( attacker, SK_HEAVY_WEAPONS, pta );
			G_DebugAddSkillPoints( attacker, SK_HEAVY_WEAPONS, pta, "flamethrower kill" );
			break;
		case MOD_MORTAR:
			if( splash ) {
				G_AddSkillPoints( attacker, SK_HEAVY_WEAPONS, pta );
				G_DebugAddSkillPoints( attacker, SK_HEAVY_WEAPONS, pta, "mortar splash damage kill" );
			} else {
				G_AddSkillPoints( attacker, SK_HEAVY_WEAPONS, pta );
				G_DebugAddSkillPoints( attacker, SK_HEAVY_WEAPONS, pta, "mortar direct hit kill" );
			}
			break;
		case MOD_GRENADE_LAUNCHER:
		case MOD_GRENADE_PINEAPPLE:
		//bani - airstrike marker kills
		case MOD_SMOKEGRENADE:
			G_AddSkillPoints( attacker, SK_LIGHT_WEAPONS, pta );
			G_DebugAddSkillPoints( attacker, SK_LIGHT_WEAPONS, pta, "hand grenade kill" );
			break;
		case MOD_DYNAMITE:
		case MOD_LANDMINE:
			pta = (g_damageXP.integer != 1) ? 4.f : 1.f;
			G_AddSkillPoints( attacker, SK_EXPLOSIVES_AND_CONSTRUCTION,pta );
			G_DebugAddSkillPoints( attacker, SK_EXPLOSIVES_AND_CONSTRUCTION, pta, "dynamite or landmine kill" );
			break;
		case MOD_ARTY:
			pta = (g_damageXP.integer != 1) ? 4.f : 1.f;
			G_AddSkillPoints( attacker, SK_SIGNALS, pta );
			G_DebugAddSkillPoints( attacker, SK_SIGNALS, pta, "artillery kill" );
			break;
		case MOD_AIRSTRIKE:
			G_AddSkillPoints( attacker, SK_SIGNALS, pta );
			G_DebugAddSkillPoints( attacker, SK_SIGNALS, pta, "airstrike kill" );
			break;
		case MOD_GPG40:
		case MOD_M7:
			G_AddSkillPoints( attacker, SK_EXPLOSIVES_AND_CONSTRUCTION, pta );
			G_DebugAddSkillPoints( attacker, SK_EXPLOSIVES_AND_CONSTRUCTION, pta, "rifle grenade kill" );
			break;
		case MOD_POISON:
			G_AddSkillPoints( attacker, SK_FIRST_AID, pta );
			break;

		// no skills for anything else
		default:
			break;
	}
}


void G_AddKillSkillPointsForDestruction( gentity_t *attacker, meansOfDeath_t mod, g_constructible_stats_t *constructibleStats )
{
	switch( mod ) {
		case MOD_GRENADE_LAUNCHER:
		case MOD_GRENADE_PINEAPPLE:
			G_AddSkillPoints( attacker, SK_LIGHT_WEAPONS, constructibleStats->destructxpbonus );
			G_DebugAddSkillPoints( attacker, SK_LIGHT_WEAPONS, constructibleStats->destructxpbonus, "destroying a constructible/explosive" ); 
			break;
		case MOD_GPG40:
		case MOD_M7:
		case MOD_DYNAMITE:
		case MOD_LANDMINE:
			G_AddSkillPoints( attacker, SK_EXPLOSIVES_AND_CONSTRUCTION, constructibleStats->destructxpbonus );
			G_DebugAddSkillPoints( attacker, SK_EXPLOSIVES_AND_CONSTRUCTION, constructibleStats->destructxpbonus, "destroying a constructible/explosive" ); 
			break;
		case MOD_PANZERFAUST:
		case MOD_MORTAR:
			G_AddSkillPoints( attacker, SK_HEAVY_WEAPONS, constructibleStats->destructxpbonus );
			G_DebugAddSkillPoints( attacker, SK_HEAVY_WEAPONS, constructibleStats->destructxpbonus, "destroying a constructible/explosive" ); 
			break;
		case MOD_ARTY:
		case MOD_AIRSTRIKE:
			G_AddSkillPoints( attacker, SK_SIGNALS, constructibleStats->destructxpbonus );
			G_DebugAddSkillPoints( attacker, SK_SIGNALS, constructibleStats->destructxpbonus, "destroying a constructible/explosive" ); 
			break;
		case MOD_SATCHEL:
			G_AddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, constructibleStats->destructxpbonus );
			G_DebugAddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, constructibleStats->destructxpbonus, "destroying a constructible/explosive" ); 
			break;
		default:
			break;
	}
}

/////// SKILL DEBUGGING ///////
static fileHandle_t skillDebugLog = -1;

void G_DebugOpenSkillLog( void )
{
	vmCvar_t	mapname;
	qtime_t		ct;
	char		*s;

	if( g_debugSkills.integer < 2 )
		return;

	trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );

	trap_RealTime( &ct );

	if( trap_FS_FOpenFile( va( "skills-%d-%02d-%02d-%02d%02d%02d-%s.log",
								1900+ct.tm_year, ct.tm_mon+1,ct.tm_mday,
								ct.tm_hour, ct.tm_min, ct.tm_sec,
								mapname.string ), &skillDebugLog, FS_APPEND_SYNC ) < 0 )
		return;

	s = va( "%02d:%02d:%02d : Logfile opened.\n", ct.tm_hour, ct.tm_min, ct.tm_sec );

	trap_FS_Write( s, strlen( s ), skillDebugLog );
}

void G_DebugCloseSkillLog( void )
{
	qtime_t		ct;
	char		*s;

	if( skillDebugLog == -1 )
		return;

	trap_RealTime( &ct );

	s = va( "%02d:%02d:%02d : Logfile closed.\n", ct.tm_hour, ct.tm_min, ct.tm_sec );

	trap_FS_Write( s, strlen( s ), skillDebugLog );

	trap_FS_FCloseFile( skillDebugLog );
}

void G_DebugAddSkillLevel( gentity_t *ent, skillType_t skill )
{
	qtime_t		ct;

	if( !g_debugSkills.integer )
		return;

	// CHRUKER: b013 - Was printing the float with 6.2 as max. numbers
	trap_SendServerCommand( ent-g_entities, va( "sdbg \"^%c(SK: %2i XP: %.0f) %s: You raised your skill level to %i.\"\n",
								COLOR_RED + skill, ent->client->sess.skill[skill], ent->client->sess.skillpoints[skill], skillNames[skill], ent->client->sess.skill[skill] ) );

	trap_RealTime( &ct );

	if( g_debugSkills.integer >= 2 && skillDebugLog != -1 ) {
		// CHRUKER: b013 - Was printing the float with 6.2 as max. numbers
		char *s = va( "%02d:%02d:%02d : ^%c(SK: %2i XP: %.0f) %s: %s raised in skill level to %i.\n",
			ct.tm_hour, ct.tm_min, ct.tm_sec,
			COLOR_RED + skill, ent->client->sess.skill[skill], ent->client->sess.skillpoints[skill], skillNames[skill], ent->client->pers.netname, ent->client->sess.skill[skill] );
		trap_FS_Write( s, strlen( s ), skillDebugLog );
	}
}

void G_DebugAddSkillPoints( gentity_t *ent, skillType_t skill, float points, const char *reason )
{
	qtime_t		ct;

	if( !g_debugSkills.integer )
		return;

	// CHRUKER: b013 - Was printing the float with 6.2 as max. numbers
	trap_SendServerCommand( ent-g_entities, va( "sdbg \"^%c(SK: %2i XP: %.0f) %s: You gained %.0fXP, reason: %s.\"\n",
								COLOR_RED + skill, ent->client->sess.skill[skill], ent->client->sess.skillpoints[skill], skillNames[skill], points, reason ) );

	trap_RealTime( &ct );

	if( g_debugSkills.integer >= 2 && skillDebugLog != -1 ) {
		// CHRUKER: b013 - Was printing the float with 6.2 as max. numbers
		char *s = va( "%02d:%02d:%02d : ^%c(SK: %2i XP: %.0f) %s: %s gained %.0fXP, reason: %s.\n",
			ct.tm_hour, ct.tm_min, ct.tm_sec,
			COLOR_RED + skill, ent->client->sess.skill[skill], ent->client->sess.skillpoints[skill], skillNames[skill], ent->client->pers.netname, points, reason );
		trap_FS_Write( s, strlen( s ), skillDebugLog );
	}
}
// CHRUKER: b017 - Added a check to make sure that the best result is larger than 0
#define CHECKSTAT1( XX )														\
	best = NULL;																\
	for( i = 0; i < level.numConnectedClients; i++ ) {							\
		gclient_t* cl = &level.clients[ level.sortedClients[ i ] ];				\
		if( cl->sess.sessionTeam == TEAM_SPECTATOR ) {							\
			continue;															\
		}																		\
		if( cl->XX <= 0 ) {														\
			 continue; 															\
		}																		\
		if( !best || cl->XX > best->XX ) {									\
			best = cl;															\
		}																		\
	}																			\
	if( best ) { best->hasaward = qtrue; }										\
	Q_strcat( buffer, 1024, va( ";%s; %i ", best ? best->pers.netname : "", best ? best->sess.sessionTeam : -1 ) )

#define CHECKSTATMIN( XX, YY )													\
	best = NULL;																\
	for( i = 0; i < level.numConnectedClients; i++ ) {							\
		gclient_t* cl = &level.clients[ level.sortedClients[ i ] ];				\
		if( cl->sess.sessionTeam == TEAM_SPECTATOR ) {							\
			continue;															\
		}																		\
		if( !best || cl->XX > best->XX ) {									\
			best = cl;															\
		}																		\
	}																			\
	if( best ) { best->hasaward = qtrue; }										\
	Q_strcat( buffer, 1024, va( ";%s; %i ", best && best->XX >= YY ? best->pers.netname : "", best && best->XX >= YY ? best->sess.sessionTeam : -1 ) )
// CHRUKER: b017 - Moved the minimum skill requirement into a seperate if sentence
// Added a check to make sure only people who have increased their skills are considered
#define CHECKSTATSKILL( XX )															\
	best = NULL;																\
	for( i = 0; i < level.numConnectedClients; i++ ) {							\
		gclient_t* cl = &level.clients[ level.sortedClients[ i ] ];				\
		if( cl->sess.sessionTeam == TEAM_SPECTATOR ) {							\
			continue;															\
		}																		\
		if ((cl->sess.skillpoints[XX] - cl->sess.startskillpoints[XX]) <= 0) {	\
			continue;															\
		}																		\
		if( cl->sess.skill[XX] < 1 ) {											\
			continue;															\
		}																		\
		if( !best || (cl->sess.skillpoints[XX] - cl->sess.startskillpoints[XX]) > (best->sess.skillpoints[XX] - best->sess.startskillpoints[XX]) ) { \
			best = cl;															\
		}																		\
	}																			\
	if( best ) { best->hasaward = qtrue; }										\
	Q_strcat( buffer, 1024, va( ";%s; %i ", best ? best->pers.netname : "", best && best->sess.skillpoints[XX] >= 20 ? best->sess.sessionTeam : -1 ) )

#define CHECKSTAT3( XX, YY, ZZ )												\
	best = NULL;																\
	for( i = 0; i < level.numConnectedClients; i++ ) {							\
		gclient_t* cl = &level.clients[ level.sortedClients[ i ] ];				\
		if( cl->sess.sessionTeam == TEAM_SPECTATOR ) {							\
			continue;															\
		}																		\
		if( !best || cl->XX > best->XX ) {									\
			best = cl;															\
		} else if( cl->XX == best->XX && cl->YY > best->YY) {			\
			best = cl;															\
		} else if( cl->XX == best->XX && cl->YY == best->YY && cl->ZZ > best->ZZ) {			\
			best = cl;															\
		}																		\
	}																			\
	if( best ) { best->hasaward = qtrue; }										\
	Q_strcat( buffer, 1024, va( ";%s; %i ", best ? best->pers.netname : "", best ? best->sess.sessionTeam : -1 ) )

#define CHECKSTATTIME( XX, YY )													\
	best = NULL;																\
	for( i = 0; i < level.numConnectedClients; i++ ) {							\
		gclient_t* cl = &level.clients[ level.sortedClients[ i ] ];				\
		if( cl->sess.sessionTeam == TEAM_SPECTATOR ) {							\
			continue;															\
		}																		\
		if( !best || (cl->XX/(float)(level.time - cl->YY)) > (best->XX/(float)(level.time - best->YY)) ) {\
			best = cl;															\
		}																		\
	}																			\
	if( best ) {																\
		if( (best->sess.startxptotal - best->ps.persistant[PERS_SCORE]) >= 100 || best->medals || best->hasaward) {	\
			best = NULL;														\
		}																		\
	}																			\
	Q_strcat( buffer, 1024, va( ";%s; %i ", best ? best->pers.netname : "", best ? best->sess.sessionTeam : -1 ) )

void G_BuildEndgameStats( void ) {
	char buffer[1024];
	int i;
	gclient_t* best;

	G_CalcClientAccuracies();

	for( i = 0; i < level.numConnectedClients; i++ ) {
		level.clients[ i ].hasaward = qfalse;
	}

	*buffer = '\0';

	CHECKSTAT1( sess.kills );
	CHECKSTAT1( ps.persistant[PERS_SCORE] );
	CHECKSTAT3( sess.rank, medals, ps.persistant[PERS_SCORE] );
	CHECKSTAT1( medals );
	CHECKSTATSKILL( SK_BATTLE_SENSE );
	CHECKSTATSKILL( SK_EXPLOSIVES_AND_CONSTRUCTION );
	CHECKSTATSKILL( SK_FIRST_AID );
	CHECKSTATSKILL( SK_SIGNALS );
	CHECKSTATSKILL( SK_LIGHT_WEAPONS );
	CHECKSTATSKILL( SK_HEAVY_WEAPONS );
	CHECKSTATSKILL( SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS );
	CHECKSTAT1( acc );
	CHECKSTATMIN( sess.team_kills, 5 );
	CHECKSTATTIME( ps.persistant[PERS_SCORE], pers.enterTime );

	trap_SetConfigstring( CS_ENDGAME_STATS, buffer );
}

// Michael
void G_ReassignSkillLevel(skillType_t skill)
{
	int count;
	for( count = 0; count < MAX_CLIENTS; count++ ){
		// doesn't work for those in xpsave mode?
		if(level.gentities[count].client != 0 &&
			level.gentities[count].client->pers.connected == CON_CONNECTED){
				G_SetPlayerSkill(level.gentities[count].client, skill);
				G_UpgradeSkill( &level.gentities[count], skill );
		}
	}
}
