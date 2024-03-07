//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Rosenburger
//
//=============================================================================//

#include "cbase.h"
#include "hl1_npc_talker.h"

#define NUM_ROSENBERG_HEADS 1 //only one head duh

class CRosenberg : public CHL1NPCTalker
{
	DECLARE_CLASS(CRosenberg, CHL1NPCTalker);
	DECLARE_DATADESC();
public:
	void	Precache(void);
	void	Spawn(void);
	void	Activate();
	Class_T Classify(void);
	int		GetSoundInterests(void);

	virtual void ModifyOrAppendCriteria(AI_CriteriaSet& set);

	virtual int ObjectCaps(void) { return UsableNPCObjectCaps(BaseClass::ObjectCaps()); }
	float	MaxYawSpeed(void);

	float	TargetDistance(void);
	bool	IsValidEnemy(CBaseEntity *pEnemy);


	int		OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo);
	void	Event_Killed(const CTakeDamageInfo &info);

	void	Heal(void);
	bool	CanHeal(void);

	int		TranslateSchedule(int scheduleType);
	void	HandleAnimEvent(animevent_t* pEvent);
	int		SelectSchedule(void);
	void	StartTask(const Task_t *pTask);
	void	RunTask(const Task_t *pTask);

	NPC_STATE SelectIdealState(void);

	int		FriendNumber(int arrayNumber);

	bool	DisregardEnemy(CBaseEntity *pEnemy) { return !pEnemy->IsAlive() || (gpGlobals->curtime - m_flFearTime) > 15; }

	void	TalkInit(void);

	void	DeclineFollowing(void);

	void	Scream(void);

	Activity GetStoppedActivity(void);
	Activity NPC_TranslateActivity(Activity newActivity);

	void PainSound(const CTakeDamageInfo& info);
	void DeathSound(const CTakeDamageInfo &info);

	enum
	{
		SCHED_ROS_HEAL = BaseClass::NEXT_SCHEDULE,
		SCHED_ROS_FOLLOWTARGET,
		SCHED_ROS_STOPFOLLOWING,
		SCHED_ROS_FACETARGET,
		SCHED_ROS_COVER,
		SCHED_ROS_HIDE,
		SCHED_ROS_IDLESTAND,
		SCHED_ROS_PANIC,
		SCHED_ROS_FOLLOWSCARED,
		SCHED_ROS_FACETARGETSCARED,
		SCHED_ROS_FEAR,
		SCHED_ROS_STARTLE,
	};

	enum
	{
		TASK_SAY_HEAL = BaseClass::NEXT_TASK,
		TASK_HEAL,
		TASK_SAY_FEAR,
		TASK_RUN_PATH_SCARED,
		TASK_SCREAM,
		TASK_RANDOM_SCREAM,
		TASK_MOVE_TO_TARGET_RANGE_SCARED,
	};

	DEFINE_CUSTOM_AI;

	//private:

	float m_flFearTime;
	float m_flHealTime;
	float m_flPainTime;

};

#define RS_PLFEAR	"RS_PLFEAR"
#define RS_FEAR		"RS_FEAR"
#define RS_HEAL		"RS_HEAL"
#define RS_SCREAM	"RS_SCREAM"
#define RS_POK		"RS_POK"

ConVar	sk_rosenberg_health("sk_rosenberg_health", "20");
ConVar	sk_rosenberg_heal("sk_rosenberg_heal", "25");

static int ACT_EXCITED;

//=========================================================
// Makes it fast to check barnacle classnames in 
// IsValidEnemy()
//=========================================================
string_t	s_miszBarnacleClassname;

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		ROSENBERG_AE_HEAL		( 1 )
#define		ROSENBERG_AE_NEEDLEON	( 2 )
#define		ROSENBERG_AE_NEEDLEOFF	( 3 )

//=======================================================
// ROSENBERG
//=======================================================

LINK_ENTITY_TO_CLASS(monster_rosenberg, CRosenberg);

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC(CRosenberg)
	DEFINE_FIELD(m_flFearTime, FIELD_TIME),
	DEFINE_FIELD(m_flHealTime, FIELD_TIME),
	DEFINE_FIELD(m_flPainTime, FIELD_TIME),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CRosenberg::Precache(void)
{
	PrecacheModel("models/rosenberg.mdl");

	PrecacheScriptSound("Rosenberg.Pain");

	TalkInit();

	BaseClass::Precache();
}

void CRosenberg::ModifyOrAppendCriteria(AI_CriteriaSet& criteriaSet)
{
	BaseClass::ModifyOrAppendCriteria(criteriaSet);

	bool predisaster = FBitSet(m_spawnflags, SF_NPC_PREDISASTER) ? true : false;

	criteriaSet.AppendCriteria("disaster", predisaster ? "[disaster::pre]" : "[disaster::post]");
}

// Init talk data
void CRosenberg::TalkInit()
{
	BaseClass::TalkInit();

	GetExpresser()->SetVoicePitch(100);
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CRosenberg::Spawn(void)
{
	SetRenderColor(255, 255, 255, 255);

	Precache();

	SetModel("models/scientist.mdl");

	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);
	m_bloodColor = BLOOD_COLOR_RED;
	ClearEffects();
	m_iHealth = sk_rosenberg_health.GetFloat();
	m_flFieldOfView = VIEW_FIELD_WIDE;
	m_NPCState = NPC_STATE_NONE;

	CapabilitiesClear();
	CapabilitiesAdd(bits_CAP_MOVE_GROUND | bits_CAP_OPEN_DOORS | bits_CAP_AUTO_DOORS | bits_CAP_USE);
	CapabilitiesAdd(bits_CAP_TURN_HEAD | bits_CAP_ANIMATEDFACE);

	// White hands
	m_nSkin = 0;
	m_nBody = 3;

	NPCInit();

	SetUse(&CRosenberg::FollowerUse);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CRosenberg::Activate()
{
	s_miszBarnacleClassname = FindPooledString("monster_barnacle");
	BaseClass::Activate();
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
// Output : 
//-----------------------------------------------------------------------------
Class_T	CRosenberg::Classify(void)
{
	return	CLASS_HUMAN_PASSIVE;
}

int CRosenberg::GetSoundInterests(void)
{
	return	SOUND_WORLD |
		SOUND_COMBAT |
		SOUND_DANGER |
		SOUND_PLAYER;
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CRosenberg::HandleAnimEvent(animevent_t *pEvent)
{
	switch (pEvent->event)
	{
		case ROSENBERG_AE_HEAL:		// Heal my target (if within range)
			Heal();
			break;
		case ROSENBERG_AE_NEEDLEON:
		{
			int oldBody = m_nBody;
			m_nBody = (oldBody % NUM_ROSENBERG_HEADS) + NUM_ROSENBERG_HEADS * 1;
		}
		break;
		case ROSENBERG_AE_NEEDLEOFF:
		{
			int oldBody = m_nBody;
			m_nBody = (oldBody % NUM_ROSENBERG_HEADS) + NUM_ROSENBERG_HEADS * 0;
		}
		break;

		default:
			BaseClass::HandleAnimEvent(pEvent);
			break;
	}
}

void CRosenberg::DeclineFollowing(void)
{
	if (CanSpeakAfterMyself())
	{
		Speak(RS_POK);
	}
}

void CRosenberg::Scream(void)
{
	if (IsOkToSpeak())
	{
		GetExpresser()->BlockSpeechUntil(gpGlobals->curtime + 10);
		SetSpeechTarget(GetEnemy());
		Speak(RS_SCREAM);
	}
}

Activity CRosenberg::GetStoppedActivity(void)
{
	if (GetEnemy() != NULL)
		return (Activity)ACT_EXCITED;

	return BaseClass::GetStoppedActivity();
}

float CRosenberg::MaxYawSpeed(void)
{
	switch (GetActivity())
	{
		case ACT_TURN_LEFT:
		case ACT_TURN_RIGHT:
			return 160;
			break;
		case ACT_RUN:
			return 160;
			break;
		default:
			return 60;
			break;
	}
}

void CRosenberg::StartTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
		case TASK_SAY_HEAL:

			GetExpresser()->BlockSpeechUntil(gpGlobals->curtime + 2);
			SetSpeechTarget(GetTarget());
			Speak(RS_HEAL);

			TaskComplete();
			break;

		case TASK_SCREAM:
			Scream();
			TaskComplete();
			break;

		case TASK_RANDOM_SCREAM:
			if (random->RandomFloat(0, 1) < pTask->flTaskData)
				Scream();
			TaskComplete();
			break;

		case TASK_SAY_FEAR:
			if (IsOkToSpeak())
			{
				GetExpresser()->BlockSpeechUntil(gpGlobals->curtime + 2);
				SetSpeechTarget(GetEnemy());
				if (GetEnemy() && GetEnemy()->IsPlayer())
					Speak(RS_PLFEAR);
				else
					Speak(RS_FEAR);
			}
			TaskComplete();
			break;

		case TASK_HEAL:
			SetIdealActivity(ACT_MELEE_ATTACK1);
			break;

		case TASK_RUN_PATH_SCARED:
			GetNavigator()->SetMovementActivity(ACT_RUN_SCARED);
			break;

		case TASK_MOVE_TO_TARGET_RANGE_SCARED:
		{
			if (GetTarget() == NULL)
			{
				TaskFail(FAIL_NO_TARGET);
			}
			else if ((GetTarget()->GetAbsOrigin() - GetAbsOrigin()).Length() < 1)
			{
				TaskComplete();
			}
		}
		break;

		default:
			BaseClass::StartTask(pTask);
			break;
	}
}

void CRosenberg::RunTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
		case TASK_RUN_PATH_SCARED:
			if (!IsMoving())
				TaskComplete();
			if (random->RandomInt(0, 31) < 8)
				Scream();
			break;

		case TASK_MOVE_TO_TARGET_RANGE_SCARED:
		{
			float distance;

			if (GetTarget() == NULL)
			{
				TaskFail(FAIL_NO_TARGET);
			}
			else
			{
				distance = (GetNavigator()->GetPath()->ActualGoalPosition() - GetAbsOrigin()).Length2D();
				// Re-evaluate when you think your finished, or the target has moved too far
				if ((distance < pTask->flTaskData) || (GetNavigator()->GetPath()->ActualGoalPosition() - GetTarget()->GetAbsOrigin()).Length() > pTask->flTaskData * 0.5)
				{
					GetNavigator()->GetPath()->ResetGoalPosition(GetTarget()->GetAbsOrigin());
					distance = (GetNavigator()->GetPath()->ActualGoalPosition() - GetAbsOrigin()).Length2D();
					//					GetNavigator()->GetPath()->Find();
					GetNavigator()->SetGoal(GOALTYPE_TARGETENT);
				}

				// Set the appropriate activity based on an overlapping range
				// overlap the range to prevent oROSllation
				// BUGBUG: this is checking linear distance (ie. through walls) and not path distance or even visibility
				if (distance < pTask->flTaskData)
				{
					TaskComplete();
					GetNavigator()->GetPath()->Clear();		// Stop moving
				}
				else
				{
					if (distance < 190 && GetNavigator()->GetMovementActivity() != ACT_WALK_SCARED)
						GetNavigator()->SetMovementActivity(ACT_WALK_SCARED);
					else if (distance >= 270 && GetNavigator()->GetMovementActivity() != ACT_RUN_SCARED)
						GetNavigator()->SetMovementActivity(ACT_RUN_SCARED);
				}
			}
		}
		break;

		case TASK_HEAL:
			if (IsSequenceFinished())
			{
				TaskComplete();
			}
			else
			{
				if (TargetDistance() > 90)
					TaskComplete();

				if (GetTarget())
					GetMotor()->SetIdealYaw(UTIL_VecToYaw(GetTarget()->GetAbsOrigin() - GetAbsOrigin()));

				//GetMotor()->SetYawSpeed( m_YawSpeed );
			}
			break;
		default:
			BaseClass::RunTask(pTask);
			break;
	}
}

int CRosenberg::OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo)
{

	if (inputInfo.GetInflictor() && inputInfo.GetInflictor()->GetFlags() & FL_CLIENT)
	{
		Remember(bits_MEMORY_PROVOKED);
		StopFollowing();
	}

	// make sure friends talk about it if player hurts ROSENBERG...
	return BaseClass::OnTakeDamage_Alive(inputInfo);
}

void CRosenberg::Event_Killed(const CTakeDamageInfo &info)
{
	SetUse(NULL);
	BaseClass::Event_Killed(info);
}

bool CRosenberg::CanHeal(void)
{
	CBaseEntity *pTarget = GetFollowTarget();

	if (pTarget == NULL)
		return false;

	if (pTarget->IsPlayer() == false)
		return false;

	if ((m_flHealTime > gpGlobals->curtime) || (pTarget->m_iHealth > (pTarget->m_iMaxHealth * 0.5)))
		return false;

	return true;
}

//=========================================================
// PainSound
//=========================================================
void CRosenberg::PainSound(const CTakeDamageInfo& info)
{
	if (m_flPainTime > gpGlobals->curtime)
		return;

	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Rosenberg.Pain");

	m_flPainTime = gpGlobals->curtime + RandomFloat(0.5f, 0.75f);
}
//=========================================================
// DeathSound 
//=========================================================
void CRosenberg::DeathSound(const CTakeDamageInfo &info)
{
	SentenceStop();

	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Rosenberg.Pain");
}

void CRosenberg::Heal(void)
{
	if (!CanHeal())
		return;

	Vector target = GetFollowTarget()->GetAbsOrigin() - GetAbsOrigin();
	if (target.Length() > 100)
		return;

	GetTarget()->TakeHealth(sk_rosenberg_heal.GetFloat(), DMG_GENERIC);
	// Don't heal again for 1 minute
	m_flHealTime = gpGlobals->curtime + 60;
}

int CRosenberg::TranslateSchedule(int scheduleType)
{
	switch (scheduleType)
	{
		// Hook these to make a looping schedule
		case SCHED_TARGET_FACE:
		{
			int baseType;

			// call base class default so that ROSENBERG will talk
			// when 'used' 
			baseType = BaseClass::TranslateSchedule(scheduleType);

			if (baseType == SCHED_IDLE_STAND)
				return SCHED_TARGET_FACE;	// override this for different target face behavior
			else
				return baseType;
		}
		break;

		case SCHED_TARGET_CHASE:
			return SCHED_ROS_FOLLOWTARGET;
			break;

		case SCHED_IDLE_STAND:
		{
			int baseType;

			baseType = BaseClass::TranslateSchedule(scheduleType);

			if (baseType == SCHED_IDLE_STAND)
				return SCHED_ROS_IDLESTAND;	// override this for different target face behavior
			else
				return baseType;
		}
		break;
	}

	return BaseClass::TranslateSchedule(scheduleType);
}

Activity CRosenberg::NPC_TranslateActivity(Activity newActivity)
{
	if (GetFollowTarget() && GetEnemy())
	{
		CBaseEntity *pEnemy = GetEnemy();

		int relationship = D_NU;

		// Nothing scary, just me and the player
		if (pEnemy != NULL)
			relationship = IRelationType(pEnemy);

		if (relationship == D_HT || relationship == D_FR)
		{
			if (newActivity == ACT_WALK)
				return ACT_WALK_SCARED;
			else if (newActivity == ACT_RUN)
				return ACT_RUN_SCARED;
		}
	}

	return BaseClass::NPC_TranslateActivity(newActivity);
}

int CRosenberg::SelectSchedule(void)
{
	if (m_NPCState == NPC_STATE_PRONE)
	{
		// Immediately call up to the talker code. Barnacle death is priority schedule.
		return BaseClass::SelectSchedule();
	}

	// so we don't keep calling through the EHANDLE stuff
	CBaseEntity *pEnemy = GetEnemy();

	if (GetFollowTarget())
	{
		// so we don't keep calling through the EHANDLE stuff
		CBaseEntity *pEnemy = GetEnemy();

		int relationship = D_NU;

		// Nothing scary, just me and the player
		if (pEnemy != NULL)
			relationship = IRelationType(pEnemy);

		if (relationship != D_HT && relationship != D_FR)
		{
			// If I'm already close enough to my target
			if (TargetDistance() <= 128)
			{
				if (CanHeal())	// Heal opportunistically
				{
					SetTarget(GetFollowTarget());
					return SCHED_ROS_HEAL;
				}
			}
		}
	}
	else if (HasCondition(COND_PLAYER_PUSHING) && !(GetSpawnFlags() & SF_NPC_PREDISASTER))
	{		// Player wants me to move
		return SCHED_HL1TALKER_FOLLOW_MOVE_AWAY;
	}

	if (BehaviorSelectSchedule())
	{
		return BaseClass::SelectSchedule();
	}



	if (HasCondition(COND_HEAR_DANGER) && m_NPCState != NPC_STATE_PRONE)
	{
		CSound *pSound;
		pSound = GetBestSound();

		if (pSound && pSound->IsSoundType(SOUND_DANGER))
			return SCHED_TAKE_COVER_FROM_BEST_SOUND;
	}

	switch (m_NPCState)
	{

		case NPC_STATE_ALERT:
		case NPC_STATE_IDLE:

			if (pEnemy)
			{
				if (HasCondition(COND_SEE_ENEMY))
					m_flFearTime = gpGlobals->curtime;
				else if (DisregardEnemy(pEnemy))		// After 15 seconds of being hidden, return to alert
				{
					SetEnemy(NULL);
					pEnemy = NULL;
				}
			}

			if (HasCondition(COND_LIGHT_DAMAGE) || HasCondition(COND_HEAVY_DAMAGE))
			{
				// flinch if hurt
				return SCHED_SMALL_FLINCH;
			}

			// Cower when you hear something scary
			if (HasCondition(COND_HEAR_DANGER) || HasCondition(COND_HEAR_COMBAT))
			{
				CSound *pSound;
				pSound = GetBestSound();

				if (pSound)
				{
					if (pSound->IsSoundType(SOUND_DANGER | SOUND_COMBAT))
					{
						if (gpGlobals->curtime - m_flFearTime > 3)	// Only cower every 3 seconds or so
						{
							m_flFearTime = gpGlobals->curtime;		// Update last fear
							return SCHED_ROS_STARTLE;	// This will just duck for a second
						}
					}
				}
			}

			if (GetFollowTarget())
			{
				if (!GetFollowTarget()->IsAlive())
				{
					// UNDONE: Comment about the recently dead player here?
					StopFollowing();
					break;
				}

				int relationship = D_NU;

				// Nothing scary, just me and the player
				if (pEnemy != NULL)
					relationship = IRelationType(pEnemy);

				if (relationship != D_HT)
				{
					return SCHED_TARGET_FACE;	// Just face and follow.
				}
				else	// UNDONE: When afraid, ROSENBERG won't move out of your way.  Keep This?  If not, write move away scared
				{
					if (HasCondition(COND_NEW_ENEMY)) // I just saw something new and scary, react
						return SCHED_ROS_FEAR;					// React to something scary
					return SCHED_ROS_FACETARGETSCARED;	// face and follow, but I'm scared!
				}
			}

			// try to say something about smells
			TrySmellTalk();
			break;


		case NPC_STATE_COMBAT:

			if (HasCondition(COND_NEW_ENEMY))
				return SCHED_ROS_FEAR;					// Point and scream!
			if (HasCondition(COND_SEE_ENEMY))
				return SCHED_ROS_COVER;		// Take Cover

			if (HasCondition(COND_HEAR_COMBAT) || HasCondition(COND_HEAR_DANGER))
				return SCHED_TAKE_COVER_FROM_BEST_SOUND;	// Cower and panic from the scary sound!

			return SCHED_ROS_COVER;			// Run & Cower
			break;
	}

	return BaseClass::SelectSchedule();
}

NPC_STATE CRosenberg::SelectIdealState(void)
{
	switch (m_NPCState)
	{
		case NPC_STATE_ALERT:
		case NPC_STATE_IDLE:
			if (HasCondition(COND_NEW_ENEMY))
			{
				if (GetFollowTarget() && GetEnemy())
				{
					int relationship = IRelationType(GetEnemy());
					if (relationship != D_FR || relationship != D_HT && (!HasCondition(COND_LIGHT_DAMAGE) || !HasCondition(COND_HEAVY_DAMAGE)))
					{
						// Don't go to combat if you're following the player
						return NPC_STATE_ALERT;
					}
					StopFollowing();
				}
			}
			else if (HasCondition(COND_LIGHT_DAMAGE) || HasCondition(COND_HEAVY_DAMAGE))
			{
				// Stop following if you take damage
				if (GetFollowTarget())
					StopFollowing();
			}
			break;

		case NPC_STATE_COMBAT:
		{
			CBaseEntity *pEnemy = GetEnemy();
			if (pEnemy != NULL)
			{
				if (DisregardEnemy(pEnemy))		// After 15 seconds of being hidden, return to alert
				{
					// Strip enemy when going to alert
					SetEnemy(NULL);
					return NPC_STATE_ALERT;
				}
				// Follow if only scared a little
				if (GetFollowTarget())
				{
					return NPC_STATE_ALERT;
				}

				if (HasCondition(COND_SEE_ENEMY))
				{
					m_flFearTime = gpGlobals->curtime;
					return NPC_STATE_COMBAT;
				}

			}
		}
		break;
	}

	return BaseClass::SelectIdealState();
}

int CRosenberg::FriendNumber(int arrayNumber)
{
	static int array[3] = { 1, 2, 0 };
	if (arrayNumber < 3)
		return array[arrayNumber];
	return arrayNumber;
}

float CRosenberg::TargetDistance(void)
{
	CBaseEntity *pFollowTarget = GetFollowTarget();

	// If we lose the player, or he dies, return a really large distance
	if (pFollowTarget == NULL || !pFollowTarget->IsAlive())
		return 1e6;

	return (pFollowTarget->WorldSpaceCenter() - WorldSpaceCenter()).Length();
}

bool CRosenberg::IsValidEnemy(CBaseEntity *pEnemy)
{
	if (pEnemy->m_iClassname == s_miszBarnacleClassname)
	{
		// ROSENBERGs ignore barnacles rather than freak out.(sjb)
		return false;
	}

	return BaseClass::IsValidEnemy(pEnemy);
}

//------------------------------------------------------------------------------
//
// Schedules
//
//------------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC(monster_rosenberg, CRosenberg)
	DECLARE_TASK(TASK_SAY_HEAL)
	DECLARE_TASK(TASK_HEAL)
	DECLARE_TASK(TASK_SAY_FEAR)
	DECLARE_TASK(TASK_RUN_PATH_SCARED)
	DECLARE_TASK(TASK_SCREAM)
	DECLARE_TASK(TASK_RANDOM_SCREAM)
	DECLARE_TASK(TASK_MOVE_TO_TARGET_RANGE_SCARED)
DECLARE_ACTIVITY(ACT_EXCITED)

//=========================================================
// > SCHED_ROS_HEAL
//=========================================================
DEFINE_SCHEDULE
(
	SCHED_ROS_HEAL,

	"	Tasks"
	"		TASK_GET_PATH_TO_TARGET				0"
	"		TASK_MOVE_TO_TARGET_RANGE			50"
	"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_ROS_FOLLOWTARGET"
	"		TASK_FACE_IDEAL						0"
	"		TASK_SAY_HEAL						0"
	"		TASK_PLAY_SEQUENCE_FACE_TARGET		ACTIVITY:ACT_ARM"
	"		TASK_HEAL							0"
	"		TASK_PLAY_SEQUENCE_FACE_TARGET		ACTIVITY:ACT_DISARM"
	"	"
	"	Interrupts"
)

//=========================================================
// > SCHED_ROS_FOLLOWTARGET
//=========================================================
DEFINE_SCHEDULE
(
	SCHED_ROS_FOLLOWTARGET,

	"	Tasks"
	//		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_ROS_STOPFOLLOWING"
	"		TASK_GET_PATH_TO_TARGET			0"
	"		TASK_MOVE_TO_TARGET_RANGE		128"
	"		TASK_SET_SCHEDULE				SCHEDULE:SCHED_TARGET_FACE"
	"	"
	"	Interrupts"
	"		COND_NEW_ENEMY"
	"		COND_LIGHT_DAMAGE"
	"		COND_HEAVY_DAMAGE"
	"		COND_HEAR_DANGER"
	"		COND_HEAR_COMBAT"
)

//=========================================================
// > SCHED_ROS_STOPFOLLOWING
//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ROS_STOPFOLLOWING,

		"	Tasks"
		"		TASK_TALKER_CANT_FOLLOW			0"
		"	"
		"	Interrupts"
	)

//=========================================================
// > SCHED_ROS_FACETARGET
//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ROS_FACETARGET,

		"	Tasks"
		"		TASK_STOP_MOVING			0"
		"		TASK_FACE_TARGET			0"
		"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
		"		TASK_SET_SCHEDULE			SCHEDULE:SCHED_ROS_FOLLOWTARGET"
		"	"
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_HEAR_DANGER"
		"		COND_HEAR_COMBAT"
		"		COND_GIVE_WAY"
	)

//=========================================================
// > SCHED_ROS_COVER
//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ROS_COVER,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_ROS_PANIC"
		"		TASK_STOP_MOVING				0"
		"		TASK_FIND_COVER_FROM_ENEMY		0"
		"		TASK_RUN_PATH_SCARED			0"
		"		TASK_TURN_LEFT					179"
		"		TASK_SET_SCHEDULE				SCHEDULE:SCHED_ROS_HIDE"
		"	"
		"	Interrupts"
		"		COND_NEW_ENEMY"
	)

//=========================================================
// > SCHED_ROS_HIDE
//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ROS_HIDE,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE		SCHEDULE:SCHED_ROS_PANIC"
		"		TASK_STOP_MOVING			0"
		"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_CROUCHIDLE"
		"		TASK_SET_ACTIVITY			ACTIVITY:ACT_CROUCHIDLE"
		"		TASK_WAIT_RANDOM			10"
		"	"
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_SEE_ENEMY"
		"		COND_SEE_HATE"
		"		COND_SEE_FEAR"
		"		COND_SEE_DISLIKE"
		"		COND_HEAR_DANGER"
	)

//=========================================================
// > SCHED_ROS_IDLESTAND
//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ROS_IDLESTAND,

		"	Tasks"
		"		TASK_STOP_MOVING			0"
		"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
		"		TASK_WAIT					2"
		"		TASK_TALKER_HEADRESET		0"
		"	"
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_SMELL"
		"		COND_PROVOKED"
		"		COND_HEAR_COMBAT"
		"		COND_GIVE_WAY"
	)

//=========================================================
// > SCHED_ROS_PANIC
//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ROS_PANIC,

		"	Tasks"
		"		TASK_STOP_MOVING					0"
		"		TASK_FACE_ENEMY						0"
		"		TASK_SCREAM							0"
		"		TASK_PLAY_SEQUENCE_FACE_ENEMY		ACTIVITY:ACT_EXCITED"
		"		TASK_SET_ACTIVITY					ACTIVITY:ACT_IDLE"
		"	"
		"	Interrupts"
	)

//=========================================================
// > SCHED_ROS_FOLLOWSCARED
//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ROS_FOLLOWSCARED,

		"	Tasks"
		"		TASK_GET_PATH_TO_TARGET				0"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_ROS_FOLLOWTARGET"
		"		TASK_MOVE_TO_TARGET_RANGE_SCARED	128"
		"	"
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_HEAR_DANGER"
	)

//=========================================================
// > SCHED_ROS_FACETARGETSCARED
//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ROS_FACETARGETSCARED,

		"	Tasks"
		"	TASK_FACE_TARGET				0"
		"		TASK_SET_ACTIVITY			ACTIVITY:ACT_CROUCHIDLE"
		"		TASK_SET_SCHEDULE			SCHEDULE:SCHED_ROS_FOLLOWSCARED"
		"	"
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_HEAR_DANGER"
	)

//=========================================================
// > SCHED_FEAR
//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ROS_FEAR,

		"	Tasks"
		"		TASK_STOP_MOVING			0"
		"		TASK_FACE_ENEMY				0"
		"		TASK_SAY_FEAR				0"
		"	"
		"	Interrupts"
		"		COND_NEW_ENEMY"
	)

//=========================================================
// > SCHED_ROS_STARTLE
//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ROS_STARTLE,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_ROS_PANIC"
		"		TASK_RANDOM_SCREAM					0.3"
		"		TASK_STOP_MOVING					0"
		"		TASK_PLAY_SEQUENCE_FACE_ENEMY		ACTIVITY:ACT_CROUCH"
		"		TASK_RANDOM_SCREAM					0.1"
		"		TASK_PLAY_SEQUENCE_FACE_ENEMY		ACTIVITY:ACT_CROUCHIDLE"
		"		TASK_WAIT_RANDOM					1"
		"	"
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_SEE_ENEMY"
		"		COND_SEE_HATE"
		"		COND_SEE_FEAR"
		"		COND_SEE_DISLIKE"
	)

	AI_END_CUSTOM_NPC()