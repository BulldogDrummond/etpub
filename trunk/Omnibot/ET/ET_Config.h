////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: gabriel $
// $LastChangedDate: 2007-06-05 05:59:22 +0300 (Tue, 05 Jun 2007) $
// $LastChangedRevision: 1744 $
//
// Title: ET Config
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __ET_CONFIG_H__
#define __ET_CONFIG_H__

#include "Omni-Bot_Events.h"

typedef enum eET_Version
{
	ET_VERSION_0_52_beta1 = 1,
	ET_VERSION_0_52_beta2,
	ET_VERSION_0_52_beta3,
	ET_VERSION_0_52_beta4,
	ET_VERSION_0_52_beta5,
	ET_VERSION_0_52_beta6,
	ET_VERSION_0_52_FINAL = ET_VERSION_0_52_beta6,
	ET_VERSION_0_53_beta1,
	ET_VERSION_0_53_beta2,
	ET_VERSION_0_6,
	ET_VERSION_0_65,
	ET_VERSION_LAST,
	ET_VERSION_LATEST = ET_VERSION_LAST - 1
} ET_Version;

typedef enum eET_Events
{
	ET_EVENT_BEGIN = EVENT_NUM_EVENTS,
	ET_EVENT_PRETRIGGER_MINE,
	ET_EVENT_POSTTRIGGER_MINE,
	ET_EVENT_MORTAR_IMPACT,
	ET_EVENT_END
} ET_Event;

typedef enum eET_Msgs
{
	ET_MSG_BEGIN = GEN_MSG_END,

	// actions
	//ET_MSG_PLANTDYNAMITE,
	//ET_MSG_PLANTMINE,
	ET_MSG_GOTOLIMBO,

	// misc query sutff
	ET_MSG_ISMEDICNEAR,
	ET_MSG_ISWAITINGFORMEDIC,
	ET_MSG_REINFORCETIME,

	ET_MSG_GETGUNHEALTH,
	ET_MSG_GETGUNHEAT,
	ET_MSG_ISGUNMOUNTED,
	ET_MSG_ISGUNREPAIRABLE,
	ET_MSG_MOUNTEDMG42INFO,
	
	// weapon query stuff
	ET_MSG_WPOVERHEATED,
	//ET_MSG_WPGRENSPEED,
	ET_MSG_PICKWEAPON,
	ET_MSG_PICKWEAPON2,
	ET_MSG_GETHINT,
	ET_MSG_CHECKPOINTTEAM,
	ET_MSG_CHANGESPAWNPOINT,

	// goal query stuff
	ET_MSG_GHASFLAG,
	ET_MSG_GCONSTRUCTABLE,		// check if goal is constructible at the moment
	ET_MSG_GDYNDESTROYABLE,		// check if goal can be destroyed by dynamite
	ET_MSG_GSATDESTROYABLE,		// check if goal can be destroyed by satchel charge
	ET_MSG_GEXPLOSIVESTATE,		// check if state of an explosive
	ET_MSG_GCANBEGRABBED,		// check if an entity can be grabbed
	ET_MSG_GNUMTEAMMINES,		// check the number of team mines
	ET_MSG_CABINETDATA,			// stats from a health/ammo cabinet
	ET_MSG_SKILLLEVEL,			// skill level for all the skills of the bot.

	ET_MSG_END
} ET_Msg;

typedef enum eET_PlayerClass
{
	ET_CLASS_UNKNOWN = 0,
	ET_CLASS_NULL = 0,
	ET_CLASS_SOLDIER,
	ET_CLASS_MEDIC,
	ET_CLASS_ENGINEER,
	ET_CLASS_FIELDOPS,
	ET_CLASS_COVERTOPS,
	ET_CLASS_MAX,
	ET_CLASS_ANY = ET_CLASS_MAX,

	// Other values to identify the "class"
	ET_CLASSEX_MG42MOUNT,
	ET_CLASSEX_DYNAMITE,
	ET_CLASSEX_MINE,
	ET_CLASSEX_SATCHEL,
	ET_CLASSEX_SMOKEBOMB,
	ET_CLASSEX_SMOKEMARKER,
	ET_CLASSEX_VEHICLE,
	ET_CLASSEX_VEHICLE_HVY,
	ET_CLASSEX_VEHICLE_NODAMAGE,
	ET_CLASSEX_BREAKABLE,
	ET_CLASSEX_INJUREDPLAYER,
	ET_CLASSEX_CORPSE,
	ET_CLASSEX_TREASURE,
	ET_CLASSEX_GRENADE,
	ET_CLASSEX_ROCKET,
	ET_CLASSEX_MORTAR,
	ET_CLASSEX_ARTY,
	ET_CLASSEX_AIRSTRIKE,
	ET_CLASSEX_FLAMECHUNK,
	ET_CLASSEX_M7_GRENADE,
	ET_CLASSEX_GPG40_GRENADE,
	ET_CLASSEX_HEALTHCABINET,
	ET_CLASSEX_AMMOCABINET,

	ET_NUM_CLASSES
} ET_PlayerClass;

typedef enum eET_Weapons
{
	ET_WP_UNKNOWN = INVALID_WEAPON,
	ET_WP_NONE = INVALID_WEAPON,

	// general
	ET_WP_KNIFE,
	ET_WP_BINOCULARS,

	// axis
	ET_WP_GREN_AXIS,	// grenades used by axis
	ET_WP_LUGER,		// axis standard secondary weapon
	ET_WP_AKIMBO_LUGER, // double luger
	ET_WP_MP40,			// axis standard weapon

	// allies
	ET_WP_GREN_ALLIES,	// grenades used by allies
	ET_WP_COLT,			// allies standard secondary weapon
	ET_WP_AKIMBO_COLT,	// double colt
	ET_WP_THOMPSON,		// allies standard weapon

	// soldier
	ET_WP_PANZERFAUST,
	ET_WP_FLAMETHROWER,
	ET_WP_MORTAR,
	ET_WP_MORTAR_SET,
	ET_WP_MOBILE_MG42,
	ET_WP_MOBILE_MG42_SET,

	// medic
	ET_WP_SYRINGE,
	ET_WP_MEDKIT,
	ET_WP_ADRENALINE,

	// covertops
	ET_WP_STEN,
	ET_WP_FG42,
	ET_WP_FG42_SCOPE,
	ET_WP_SATCHEL,
	ET_WP_SATCHEL_DET,	// remote control for satchel
	ET_WP_SMOKE_GRENADE,
	// covertops (axis)
	ET_WP_SILENCED_LUGER,
	ET_WP_AKIMBO_SILENCED_LUGER,
	ET_WP_K43,
	ET_WP_K43_SCOPE,
	// covertops (allies)
	ET_WP_SILENCED_COLT,
	ET_WP_AKIMBO_SILENCED_COLT,
	ET_WP_GARAND,
	ET_WP_GARAND_SCOPE,

	// fieldops
	ET_WP_AMMO_PACK,
	ET_WP_SMOKE_MARKER,

	// engineer
	ET_WP_DYNAMITE,
	ET_WP_PLIERS,
	ET_WP_LANDMINE,
	// engineer (axis)
	ET_WP_KAR98,
	ET_WP_GPG40,		// grenade launcher
	// engineer (allies)
	ET_WP_CARBINE,
	ET_WP_M7,			// grenade launcher

	ET_WP_MOUNTABLE_MG42,

	ET_WP_MAX
} ET_Weapon;

// typedef: ET_Team
//		The available teams for this gametype.
typedef enum eET_Team
{
	ET_TEAM_NONE = 0,
	ET_TEAM_AXIS,
	ET_TEAM_ALLIES,
	ET_TEAM_MAX
} ET_Team;

// typedef: ET_GoalType
//		Enumerations for ET specific goal types.
typedef enum eET_GoalType
{
	ET_GOAL_BEGIN = BASE_GOAL_NUM,
	ET_GOAL_CONSTRUCTION,	// construction site
	ET_GOAL_DYNTARGET,		// dyno targets
	ET_GOAL_DYNAMITE,		// a dynomite ;-)
	ET_GOAL_FALLENTEAMMATE,	// a teammate that needs to be revived
	ET_GOAL_HURTTEAMMATE,	// a teammate that needs to be healed
	ET_GOAL_MOVER,			// a mover, could be vehicle or other shit
	ET_GOAL_MG42MOUNT,		// a mountable mg42
	ET_GOAL_MG42REPAIR,		// repair a mountable mg42
	ET_GOAL_MOBILEMG42SPOT,	// a mobile mg42 spot
	ET_GOAL_MORTARPOSITION, // a spot to launch mortars from
	ET_GOAL_SATCHELTARGET,	// satchel charge target
	ET_GOAL_CALLARTY,		// source for artillery strikes
	ET_GOAL_CALLARTYTARGET_S,// static target for artillery strikes
	ET_GOAL_CALLARTYTARGET_D,// dynamic target for artillery strikes
	ET_GOAL_PLANTMINE,		 // area to plant mines in
	ET_GOAL_HEALTH_CAB,		// health cabinet
	ET_GOAL_AMMO_CAB,		// ammo cabinet
	ET_GOAL_CHECKPOINT,		// capturable checkpoint
	// THIS MUST STAY LAST
	ET_GOAL_END
} ET_GoalType;

typedef enum eET_Skills
{
	ET_SKILL_BATTLE_SENSE,
	ET_SKILL_ENGINEERING,
	ET_SKILL_FIRST_AID,
	ET_SKILL_SIGNALS,
	ET_SKILL_LIGHT_WEAPONS,
	ET_SKILL_HEAVY_WEAPONS,
	ET_SKILL_COVERTOPS,

	// THIS MUST STAY LAST
	ET_SKILLS_NUM_SKILLS,
} ET_Skills;

// typedef: ET_Goal
//		Enumerations for ET specific goals.
typedef enum eET_Goal
{
	goal_et_begin = goal_base_num,
	goal_et_buildconstruction,
	goal_et_plantdynamite,
	goal_et_defusedynamite,
	goal_et_reviveteammate,
	goal_et_givehealth,
	goal_et_giveammo,
	goal_et_setmg42,
	goal_et_mountmg42,
	goal_et_repairmg42,
	goal_et_mountvehicle,
	goal_et_setmortar,
	goal_et_callarty,
	goal_et_plantmine,
	goal_et_disarm_mine,
	goal_et_arm_explosive,
	goal_et_plantexplosive,
	goal_et_healthcabinet,
	goal_et_ammocabinet,
	goal_et_takecheckpoint,
	goal_et_end
} ET_Goal;

typedef enum eET_EntityFlags
{
	// bit: ET_ENT_FLAG_DISGUISED
	//		This entity is disguised
	ET_ENT_FLAG_DISGUISED = ENT_FLAG_FIRST_USER,	
	// bit: ET_ENT_MOUNTED
	//		This entity is mounted on an mg42, tank, or aagun
	ET_ENT_FLAG_MOUNTED,
	// bit: ET_ENT_FLAG_MNT_MG42
	//		Currently mounted on an mg42
	ET_ENT_FLAG_MNT_MG42,
	// bit: ET_ENT_FLAG_MNT_TANK
	//		Currently mounted on a tank
	ET_ENT_FLAG_MNT_TANK,
	// bit: ET_ENT_FLAG_MNT_AAGUN
	//		Currently mounted on an aa gun
	ET_ENT_FLAG_MNT_AAGUN,
	// bit: ET_ENT_FLAG_CARRYINGGOAL
	//		This entity is carrying a goal of some sort
	ET_ENT_FLAG_CARRYINGGOAL,
	// bit: ET_ENT_FLAG_INLIMBO
	//		This entity is currently in limbo mode. Players only.
	ET_ENT_FLAG_INLIMBO,
	// bit: ET_ENT_FLAG_ISMOUNTABLE
	//		This entity is mountable.
	ET_ENT_FLAG_ISMOUNTABLE,
	// bit: ET_ENT_FLAG_POISONED
	//		This entity is poisoned.
	ET_ENT_FLAG_POISONED,
} ET_EntityFlags;

//////////////////////////////////////////////////////////////////////////

typedef enum eCursorHintType
{
	CURSOR_HINT_NONE,
	CURSOR_HINT_PLAYER,
	CURSOR_HINT_ACTIVATE,
	CURSOR_HINT_DOOR,
	CURSOR_HINT_DOOR_ROTATING,
	CURSOR_HINT_DOOR_LOCKED,
	CURSOR_HINT_DOOR_ROTATING_LOCKED,
	CURSOR_HINT_MG42,
	CURSOR_HINT_BREAKABLE,
	CURSOR_HINT_BREAKABLE_DYNAMITE,
	CURSOR_HINT_CHAIR,
	CURSOR_HINT_ALARM,
	CURSOR_HINT_HEALTH,
	CURSOR_HINT_TREASURE,
	CURSOR_HINT_KNIFE,
	CURSOR_HINT_LADDER,
	CURSOR_HINT_BUTTON,
	CURSOR_HINT_WATER,
	CURSOR_HINT_CAUTION,
	CURSOR_HINT_DANGER,
	CURSOR_HINT_SECRET,
	CURSOR_HINT_QUESTION,
	CURSOR_HINT_EXCLAMATION,
	CURSOR_HINT_CLIPBOARD,
	CURSOR_HINT_WEAPON,
	CURSOR_HINT_AMMO,
	CURSOR_HINT_ARMOR,
	CURSOR_HINT_POWERUP,
	CURSOR_HINT_HOLDABLE,
	CURSOR_HINT_INVENTORY,
	CURSOR_HINT_SCENARIC,
	CURSOR_HINT_EXIT,
	CURSOR_HINT_NOEXIT,
	CURSOR_HINT_PLYR_FRIEND,
	CURSOR_HINT_PLYR_NEUTRAL,
	CURSOR_HINT_PLYR_ENEMY,
	CURSOR_HINT_PLYR_UNKNOWN,
	CURSOR_HINT_BUILD,
	CURSOR_HINT_DISARM,
	CURSOR_HINT_REVIVE,
	CURSOR_HINT_DYNAMITE,
	CURSOR_HINT_CONSTRUCTIBLE,
	CURSOR_HINT_UNIFORM,
	CURSOR_HINT_LANDMINE,
	CURSOR_HINT_TANK,
	CURSOR_HINT_SATCHELCHARGE,
	CURSOR_HINT_LOCKPICK
} CursorHintType;

typedef enum eExplosiveTargetType
{
	XPLO_TYPE_DYNAMITE	= 1<<0,
	XPLO_TYPE_SATCHEL	= 1<<1
} ExplosiveTargetType;

typedef enum eExplosiveState
{
	XPLO_INVALID			= -1,
	XPLO_ARMED				= 0,
	XPLO_UNARMED			= 1,
} ExplosiveState;

typedef enum eConstructableState
{
	CONST_INVALID			= -1,

	CONST_BUILT				= 0,
	CONST_UNBUILT			= 1,

	CONST_NOTDESTROYABLE	= 0,
	CONST_DESTROYABLE		= 1,

	CONST_BROKEN			= 2,
} ConstructableState;

#endif
