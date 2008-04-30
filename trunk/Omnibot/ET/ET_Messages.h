////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2007-02-20 22:25:19 -0800 (Tue, 20 Feb 2007) $
// $LastChangedRevision: 1661 $
//
// Title: TF Message Structure Definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __TF_MESSAGES_H__
#define __TF_MESSAGES_H__

#include "Base_Messages.h"

#pragma pack(push)
#pragma pack(4)

//////////////////////////////////////////////////////////////////////////

typedef struct 
{
	ET_Weapon	m_Weapon;
	obBool		m_IsOverheated;
} ET_WeaponOverheated;

typedef struct 
{
	GameEntity	m_Entity;
	int			m_Current;
	int			m_Max;
} ET_WeaponHeatLevel;

typedef struct 
{
	GameEntity		m_Explosive;
	ExplosiveState	m_State;
} ET_ExplosiveState;

typedef struct 
{
	GameEntity			m_Constructable;
	ConstructableState	m_State;
} ET_ConstructionState;

typedef struct 
{
	GameEntity			m_Entity;
	ConstructableState	m_State;
} ET_Destroyable;

typedef struct 
{
	obBool		m_HasFlag;
} ET_HasFlag;

typedef struct 
{
	GameEntity	m_Entity;
	obBool		m_CanBeGrabbed;
} ET_CanBeGrabbed;

typedef struct 
{
	int			m_Current;
	int			m_Max;
} ET_TeamMines;

typedef struct 
{
	obBool		m_WaitingForMedic;
} ET_WaitingForMedic;

typedef struct 
{
	ET_Weapon	m_Selection;
	obBool		m_Good;
} ET_SelectWeapon;

typedef struct 
{
	int			m_ReinforceTime;
} ET_ReinforceTime;

typedef struct 
{
	obBool		m_MedicNear;
} ET_MedicNear;

typedef struct 
{
	obBool		m_GoLimbo;
} ET_GoLimbo;

typedef struct 
{
	GameEntity	m_MG42Entity;
	GameEntity	m_MountedEntity;
} ET_MG42MountedPlayer;

typedef struct 
{
	GameEntity	m_MG42Entity;
	obBool		m_Repairable;
} ET_MG42MountedRepairable;

typedef struct 
{
	GameEntity	m_MG42Entity;
	int			m_Health;
} ET_MG42Health;

typedef struct 
{
	int			m_Type;
	int			m_Value;
} ET_CursorHint;

typedef struct 
{
	int			m_OwningTeam;
} ET_CheckpointTeam;

typedef struct 
{
	int			m_SpawnPoint;
} ET_SpawnPoint;

typedef struct 
{
	float		m_CenterFacing[3];
	float		m_MinHorizontalArc, m_MaxHorizontalArc;
	float		m_MinVerticalArc, m_MaxVerticalArc;
} ET_MG42Info;

typedef struct  
{
	int			m_CurrentAmount;
	int			m_MaxAmount;
	int			m_Rate;
} ET_CabinetData;

typedef struct  
{
	int		m_Skill[ET_SKILLS_NUM_SKILLS];
} ET_PlayerSkills;

//////////////////////////////////////////////////////////////////////////

typedef struct  
{
	float	m_Position[3];
} Event_MortarImpact_ET;

typedef struct  
{
	GameEntity	m_MineEntity;
} Event_TriggerMine_ET;

#pragma pack(pop)

#endif
