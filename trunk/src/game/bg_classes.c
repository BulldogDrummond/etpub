#include "q_shared.h"
#include "bg_public.h"
#include "bg_classes.h"
#include "bg_local.h"

bg_playerclass_t bg_allies_playerclasses[NUM_PLAYER_CLASSES] = {
	{	
		PC_SOLDIER,
		"characters/temperate/allied/soldier.char",
		"ui/assets/mp_gun_blue.tga",
		"ui/assets/mp_arrow_blue.tga",
		{ 
			WP_THOMPSON,
			WP_MOBILE_MG42,
			WP_FLAMETHROWER,
			WP_PANZERFAUST,
			WP_MORTAR
		},
	},

	{
		PC_MEDIC,
		"characters/temperate/allied/medic.char",
		"ui/assets/mp_health_blue.tga",
		"ui/assets/mp_arrow_blue.tga",
		{ 
			WP_THOMPSON,
		},	
	},

	{
		PC_ENGINEER,
		"characters/temperate/allied/engineer.char",
		"ui/assets/mp_wrench_blue.tga",
		"ui/assets/mp_arrow_blue.tga",
		{ 
			WP_THOMPSON,
			WP_CARBINE,
		},	
	},

	{
		PC_FIELDOPS,
		"characters/temperate/allied/fieldops.char",
		"ui/assets/mp_ammo_blue.tga",
		"ui/assets/mp_arrow_blue.tga",
		{ 
			WP_THOMPSON,
		},	
	},

	{
		PC_COVERTOPS,
		"characters/temperate/allied/cvops.char",
		"ui/assets/mp_spy_blue.tga",
		"ui/assets/mp_arrow_blue.tga",
		{ 
			WP_STEN,
			WP_FG42,
			WP_GARAND,
		},	
	},
};

bg_playerclass_t bg_axis_playerclasses[NUM_PLAYER_CLASSES] = {
	{
		PC_SOLDIER,
		"characters/temperate/axis/soldier.char",
		"ui/assets/mp_gun_red.tga",
		"ui/assets/mp_arrow_red.tga",
		{ 
			WP_MP40,
			WP_MOBILE_MG42,
			WP_FLAMETHROWER,
			WP_PANZERFAUST,
			WP_MORTAR
		},	
	},

	{
		PC_MEDIC,
		"characters/temperate/axis/medic.char",
		"ui/assets/mp_health_red.tga",
		"ui/assets/mp_arrow_red.tga",
		{ 
			WP_MP40,
		},	
	},

	{
		PC_ENGINEER,
		"characters/temperate/axis/engineer.char",
		"ui/assets/mp_wrench_red.tga",
		"ui/assets/mp_arrow_red.tga",
		{ 
			WP_MP40,
			WP_KAR98,
		},	
	},

	{
		PC_FIELDOPS,
		"characters/temperate/axis/fieldops.char",
		"ui/assets/mp_ammo_red.tga",
		"ui/assets/mp_arrow_red.tga",
		{ 
			WP_MP40,
		},	
	},

	{
		PC_COVERTOPS,
		"characters/temperate/axis/cvops.char",
		"ui/assets/mp_spy_red.tga",
		"ui/assets/mp_arrow_red.tga",
		{ 
			WP_STEN,
			WP_FG42,
			WP_K43,
		},	
	},
};

// pheno
#ifdef GAMEDLL
weapon_t bg_unlockedWeaponsME[MAX_WEAPS_PER_CLASS] =
{
	WP_NONE,
	WP_STEN
};

weapon_t bg_unlockedWeapons[MAX_WEAPS_PER_CLASS] =
{
	WP_NONE,
	WP_PANZERFAUST,
	WP_FLAMETHROWER,
	WP_STEN,
	WP_CARBINE,
	WP_GARAND,
	WP_MOBILE_MG42,
	WP_FG42,
	WP_MORTAR
};
#endif

bg_playerclass_t* BG_GetPlayerClassInfo( int team, int cls ) {
	bg_playerclass_t* teamList;

	if( cls < PC_SOLDIER || cls >= NUM_PLAYER_CLASSES ) {
		cls = PC_SOLDIER;
	}

	switch( team ) {
		default:
		case TEAM_AXIS:
			teamList = bg_axis_playerclasses;
			break;
		case TEAM_ALLIES:
			teamList = bg_allies_playerclasses;
			break;
	}

	return &teamList[cls];
}

bg_playerclass_t* BG_PlayerClassForPlayerState(playerState_t* ps) {
	return BG_GetPlayerClassInfo(ps->persistant[PERS_TEAM], ps->stats[STAT_PLAYER_CLASS]);
}

qboolean BG_ClassHasWeapon(bg_playerclass_t* classInfo, weapon_t weap) {
	int i;

	if(!weap) {
		return qfalse;
	}

	for( i = 0; i < MAX_WEAPS_PER_CLASS; i++) {
		if(classInfo->classWeapons[i] == weap) {
			return qtrue;
		}
	}
	return qfalse;
}

/*
================
BG_UnlockWeapons

pheno: unlock weapons for both teams and all classes
================
*/
#ifdef GAMEDLL
void BG_UnlockWeapons(bg_playerclass_t *classInfo, int classnum, team_t team)
{
	if (g_unlockedWeapons.integer & (1 << classnum)) {
		memcpy(classInfo->classWeapons, bg_unlockedWeapons, sizeof(classInfo->classWeapons));

		if (team == TEAM_ALLIES) {
			classInfo->classWeapons[0] = WP_THOMPSON;
		} else if (team == TEAM_AXIS) {
			classInfo->classWeapons[0] = WP_MP40;
		}
	}
}
#endif

qboolean BG_WeaponIsPrimaryForClassAndTeam( int classnum, team_t team, weapon_t weapon )
{
	bg_playerclass_t *classInfo;

	if( team == TEAM_ALLIES ) {
		classInfo = &bg_allies_playerclasses[classnum];

// pheno: unlock weapons for allied team
#ifdef GAMEDLL
		BG_UnlockWeapons(classInfo, classnum, team);
#endif

		return BG_ClassHasWeapon( classInfo, weapon );
	} else if( team == TEAM_AXIS ) {
		classInfo = &bg_axis_playerclasses[classnum];

// pheno: unlock weapons for axis team
#ifdef GAMEDLL
		BG_UnlockWeapons(classInfo, classnum, team);
#endif

		return BG_ClassHasWeapon( classInfo, weapon );
	}

	return qfalse;
}

// Dens: now gives lowercase team
const char *BG_TeamName( int teamNum ) {

	switch( teamNum ) {
	case TEAM_AXIS:
		return "axis";
	case TEAM_ALLIES:
		return "allies";
	case TEAM_SPECTATOR:
		return "spectator";
	}
	return "";
}

const char *BG_ClassName( int classNum ) {

	switch( classNum ) {
	case PC_SOLDIER:
		return "Soldier";
	case PC_MEDIC:
		return "Medic";
	case PC_ENGINEER:
		return "Engineer";
	case PC_FIELDOPS:
		return "Fieldops";
	case PC_COVERTOPS:
		return "Covertops";
	}
	return "";
}

// Dens: lowercaseClassName
const char *BG_lwrcsClassName( int classNum ) {

	switch( classNum ) {
	case PC_SOLDIER:
		return "soldier";
	case PC_MEDIC:
		return "medic";
	case PC_ENGINEER:
		return "engineer";
	case PC_FIELDOPS:
		return "fieldops";
	case PC_COVERTOPS:
		return "covertops";
	}
	return "";
}

const char* BG_ShortClassnameForNumber( int classNum ) {
	switch( classNum ) {
		case PC_SOLDIER:
			return "Soldr";
		case PC_MEDIC:
			return "Medic";
		case PC_ENGINEER:
			return "Engr";
		case PC_FIELDOPS:
			return "FdOps";
		case PC_COVERTOPS:
			return "CvOps";
		default:
			return "^1ERROR";
	}
}

const char* BG_ClassnameForNumber( int classNum ) {
	switch( classNum ) {
		case PC_SOLDIER:
			return "Soldier";
		case PC_MEDIC:
			return "Medic";
		case PC_ENGINEER:
			return "Engineer";
		case PC_FIELDOPS:
			return "Field Ops";
		case PC_COVERTOPS:
			return "Covert Ops";
		default:
			return "^1ERROR";
	}
}

const char* BG_ClassLetterForNumber( int classNum ) {
	switch( classNum ) {
		case PC_SOLDIER:
			return "S";
		case PC_MEDIC:
			return "M";
		case PC_ENGINEER:
			return "E";
		case PC_FIELDOPS:
			return "F";
		case PC_COVERTOPS:
			return "C";
		default:
			return "^1E";
	}
}


#define to_char(n)		((n) + '0')

// kw: this function was unused,
//     It's now used and changed to allow more.
int BG_ClassTextToClass(char *str) {
	
	switch( tolower(str[0]) ) {
		case 's':
		case to_char(PC_SOLDIER):
			return PC_SOLDIER;
		case 'm':
		case to_char(PC_MEDIC):
			return PC_MEDIC;
		case 'e':
		case to_char(PC_ENGINEER):
			return PC_ENGINEER;
		case 'f':
		case 'l':	//lieutenant	// FIXME: remove from missionpack
		case to_char(PC_FIELDOPS):
			return PC_FIELDOPS;
		case 'c':
		case to_char(PC_COVERTOPS):
			return PC_COVERTOPS;
		default:
			return -1;
	}
}

skillType_t BG_ClassSkillForClass( int classnum ) {
	skillType_t classskill[NUM_PLAYER_CLASSES] = { SK_HEAVY_WEAPONS, SK_FIRST_AID, SK_EXPLOSIVES_AND_CONSTRUCTION, SK_SIGNALS, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS };

	if( classnum < 0 || classnum >= NUM_PLAYER_CLASSES ) {
		return SK_BATTLE_SENSE;
	}

	return classskill[ classnum ];
}

