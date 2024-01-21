//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Krystal Barney (so segs)
//
//=============================================================================//
#ifndef NPC_KRYSTAL_H
#define NPC_KRYSTAL_H

#include "hl1_npc_talker.h"
#include "ammodef.h"
#include "IEffects.h"

//=========================================================
//=========================================================
class CNPC_Krystal : public CHL1NPCTalker
{
	DECLARE_CLASS(CNPC_Krystal, CHL1NPCTalker);
	DECLARE_DATADESC();
public:

	virtual void ModifyOrAppendCriteria(AI_CriteriaSet &set);

	void	Precache(void);
	void	Spawn(void);
	void	TalkInit(void);

	void	StartTask(const Task_t *pTask);
	void	RunTask(const Task_t *pTask);

	int		GetSoundInterests(void);
	Class_T  Classify(void);
	void    SetMaxYawSpeed(void);

	int		OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo);
	void	TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator);
	void	Event_Killed(const CTakeDamageInfo &info);

	void    PainSound(const CTakeDamageInfo &info);
	void	DeathSound(const CTakeDamageInfo &info);

	int		TranslateSchedule(int scheduleType);
	int		SelectSchedule(void);

	void	DeclineFollowing(void);

	NPC_STATE CNPC_Krystal::SelectIdealState(void);

	float	m_flPainTime;
	float	m_flCheckAttackTime;
	bool	m_fLastAttackCheck;

	enum
	{
		SCHED_KRYSTAL_FOLLOW = BaseClass::NEXT_SCHEDULE,
		SCHED_KRYSTAL_ENEMY_DRAW,
		SCHED_KRYSTAL_FACE_TARGET,
		SCHED_KRYSTAL_IDLE_STAND,
		SCHED_KRYSTAL_STOP_FOLLOWING,
	};

	DEFINE_CUSTOM_AI;
};

#endif	//NPC_KRYSTAL_H