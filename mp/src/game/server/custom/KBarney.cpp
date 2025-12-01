//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Krystal Barney (so segs)
//
//=============================================================================//

#include "cbase.h"
#include "KBarney.h"

#define BA_ATTACK	"BA_ATTACK"
#define BA_MAD		"BA_MAD"
#define BA_SHOT		"BA_SHOT"
#define BA_KILL		"BA_KILL"
#define BA_POK		"BA_POK"

ConVar	sk_krystal_health("sk_krystal_health", "35");

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
// first flag is barney dying for scripted sequences?
#define		BARNEY_AE_DRAW		( 2 )
#define		BARNEY_AE_SHOOT		( 3 )
#define		BARNEY_AE_HOLSTER	( 4 )

#define		BARNEY_BODY_GUNHOLSTERED	0
#define		BARNEY_BODY_GUNDRAWN		1
#define		BARNEY_BODY_GUNGONE			2


//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC(CNPC_Krystal)
	DEFINE_FIELD(m_flPainTime, FIELD_TIME),
	DEFINE_FIELD(m_flCheckAttackTime, FIELD_TIME),
	DEFINE_FIELD(m_fLastAttackCheck, FIELD_BOOLEAN),
END_DATADESC()


LINK_ENTITY_TO_CLASS(monster_krystal, CNPC_Krystal);


static bool IsFacing(CBaseEntity *pevTest, const Vector &reference)
{
	Vector vecDir = (reference - pevTest->GetAbsOrigin());
	vecDir.z = 0;
	VectorNormalize(vecDir);
	Vector forward;
	QAngle angle;
	angle = pevTest->GetAbsAngles();
	angle.x = 0;
	AngleVectors(angle, &forward);
	// He's facing me, he meant it
	if (DotProduct(forward, vecDir) > 0.96)	// +/- 15 degrees or so
	{
		return true;
	}
	return false;
}

//=========================================================
// Spawn
//=========================================================
void CNPC_Krystal::Spawn()
{
	Precache();

	SetModel("models/k_barney.mdl");

	SetRenderColor(255, 255, 255, 255);

	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);
	m_bloodColor = BLOOD_COLOR_RED;
	m_iHealth = sk_krystal_health.GetFloat();
	SetViewOffset(Vector(0, 0, 100));// position of the eyes relative to monster's origin.
	m_flFieldOfView = VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_NPCState = NPC_STATE_NONE;

	SetBodygroup(1, 0);

	CapabilitiesClear();
	CapabilitiesAdd(bits_CAP_MOVE_GROUND | bits_CAP_OPEN_DOORS | bits_CAP_AUTO_DOORS | bits_CAP_USE | bits_CAP_DOORS_GROUP);
	CapabilitiesAdd(bits_CAP_INNATE_RANGE_ATTACK1 | bits_CAP_TURN_HEAD | bits_CAP_ANIMATEDFACE);

	NPCInit();

	SetUse(&CNPC_Krystal::FollowerUse);
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CNPC_Krystal::Precache()
{
	PrecacheModel("models/k_barney.mdl");

	PrecacheScriptSound("Krystal.Pain");
	PrecacheScriptSound("Krystal.Die");

	// every new barney must call this, otherwise
	// when a level is loaded, nobody will talk (time is reset to 0)
	TalkInit();
	BaseClass::Precache();
}

void CNPC_Krystal::ModifyOrAppendCriteria(AI_CriteriaSet &criteriaSet)
{
	BaseClass::ModifyOrAppendCriteria(criteriaSet);

	bool predisaster = FBitSet(m_spawnflags, SF_NPC_PREDISASTER) ? true : false;

	criteriaSet.AppendCriteria("disaster", predisaster ? "[disaster::pre]" : "[disaster::post]");
}

// Init talk data
void CNPC_Krystal::TalkInit()
{
	BaseClass::TalkInit();

	// get voice for head - just one barney voice for now
	GetExpresser()->SetVoicePitch(100);
}


//=========================================================
// GetSoundInterests - returns a bit mask indicating which types
// of sounds this monster regards. 
//=========================================================
int CNPC_Krystal::GetSoundInterests(void)
{
	return	SOUND_WORLD |
		SOUND_COMBAT |
		SOUND_CARCASS |
		SOUND_MEAT |
		SOUND_GARBAGE |
		SOUND_DANGER |
		SOUND_PLAYER;
}

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
Class_T	CNPC_Krystal::Classify(void)
{
	return	CLASS_PLAYER_ALLY;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CNPC_Krystal::SetMaxYawSpeed(void)
{
	int ys = 0;

	switch (GetActivity())
	{
	case ACT_IDLE:
		ys = 70;
		break;
	case ACT_WALK:
		ys = 70;
		break;
	case ACT_RUN:
		ys = 90;
		break;
	default:
		ys = 70;
		break;
	}

	GetMotor()->SetYawSpeed(ys);
}

int CNPC_Krystal::OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo)
{
	// make sure friends talk about it if player hurts talkmonsters...
	int ret = BaseClass::OnTakeDamage_Alive(inputInfo);

	if (!IsAlive() || m_lifeState == LIFE_DYING)
		return ret;

	if (m_NPCState != NPC_STATE_PRONE && (inputInfo.GetAttacker()->GetFlags() & FL_CLIENT))
	{
		// This is a heurstic to determine if the player intended to harm me
		// If I have an enemy, we can't establish intent (may just be crossfire)
		if (GetEnemy() == NULL)
		{
			// If the player was facing directly at me, or I'm already suspicious, get mad
			if (HasMemory(bits_MEMORY_SUSPICIOUS) || IsFacing(inputInfo.GetAttacker(), GetAbsOrigin()))
			{
				// Alright, now I'm pissed!
				Speak(BA_MAD);

				/*Remember(bits_MEMORY_PROVOKED);
				StopFollowing();*/
			}
			else
			{
				// Hey, be careful with that
				Speak(BA_SHOT);
				/*Remember(bits_MEMORY_SUSPICIOUS);*/
			}
		}
		else if (!(GetEnemy()->IsPlayer()) && m_lifeState == LIFE_ALIVE)
		{
			Speak(BA_SHOT);
		}
	}

	return ret;
}

//=========================================================
// PainSound
//=========================================================
void CNPC_Krystal::PainSound(const CTakeDamageInfo &info)
{
	if (gpGlobals->curtime < m_flPainTime)
		return;

	m_flPainTime = gpGlobals->curtime + RandomFloat(0.5, 0.75);

	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Barney.Pain");
}
//=========================================================
// DeathSound 
//=========================================================
void CNPC_Krystal::DeathSound(const CTakeDamageInfo &info)
{
	SentenceStop();
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Barney.Die");
}

void CNPC_Krystal::TraceAttack(const CTakeDamageInfo &inputInfo, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator)
{
	if (inputInfo.GetDamage() >= 1.0 && !(inputInfo.GetDamageType() & DMG_SHOCK))
	{
		UTIL_BloodImpact(ptr->endpos, vecDir, BloodColor(), 4);
	}

	AddMultiDamage(inputInfo, this);
}

void CNPC_Krystal::Event_Killed(const CTakeDamageInfo &info)
{
	if (m_nBody < BARNEY_BODY_GUNGONE)
	{
		// drop the gun!
		Vector vecGunPos;
		QAngle angGunAngles;
		CBaseEntity *pGun = NULL;

		SetBodygroup(1, BARNEY_BODY_GUNGONE);

		GetAttachment(0, vecGunPos, angGunAngles);

		angGunAngles.y += 180;
		pGun = DropItem("weapon_glock", vecGunPos, angGunAngles);
	}

	BaseClass::Event_Killed(info);
}

void CNPC_Krystal::StartTask(const Task_t *pTask)
{
	BaseClass::StartTask(pTask);
}

void CNPC_Krystal::RunTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_RANGE_ATTACK1:
		if (GetEnemy() != NULL && (GetEnemy()->IsPlayer()))
		{
			m_flPlaybackRate = 1.5;
		}
		BaseClass::RunTask(pTask);
		break;
	default:
		BaseClass::RunTask(pTask);
		break;
	}
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================

int CNPC_Krystal::TranslateSchedule(int scheduleType)
{
	switch (scheduleType)
	{
		// Hook these to make a looping schedule
	case SCHED_TARGET_FACE:
	{
		// call base class default so that scientist will talk
		// when 'used' 
		int baseType = BaseClass::TranslateSchedule(scheduleType);

		if (baseType == SCHED_IDLE_STAND)
			return SCHED_KRYSTAL_FACE_TARGET;
		else
			return baseType;
	}
	break;

	case SCHED_TARGET_CHASE:
	{
		return SCHED_KRYSTAL_FOLLOW;
		break;
	}

	case SCHED_IDLE_STAND:
	{
		// call base class default so that scientist will talk
		// when 'used' 
		int baseType = BaseClass::TranslateSchedule(scheduleType);

		if (baseType == SCHED_IDLE_STAND)
			return SCHED_KRYSTAL_IDLE_STAND;
		else
			return baseType;
	}
	break;

	}

	return BaseClass::TranslateSchedule(scheduleType);
}

//=========================================================
// SelectSchedule - Decides which type of schedule best suits
// the monster's current state and conditions. Then calls
// monster's member function to get a pointer to a schedule
// of the proper type.
//=========================================================
int CNPC_Krystal::SelectSchedule(void)
{
	if (HasCondition(COND_HEAR_DANGER))
	{
		CSound *pSound = GetBestSound();

		ASSERT(pSound != NULL);

		if (pSound && pSound->IsSoundType(SOUND_DANGER))
			return SCHED_TAKE_COVER_FROM_BEST_SOUND;
	}
	if (HasCondition(COND_ENEMY_DEAD) && IsOkToSpeak())
	{
		Speak(BA_KILL);
	}

	return BaseClass::SelectSchedule();
}

NPC_STATE CNPC_Krystal::SelectIdealState(void)
{
	return BaseClass::SelectIdealState();
}

void CNPC_Krystal::DeclineFollowing(void)
{
	if (CanSpeakAfterMyself())
	{
		Speak(BA_POK);
	}
}

//------------------------------------------------------------------------------
//
// Schedules
//
//------------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC(monster_barney, CNPC_Krystal)

//=========================================================
// > SCHED_KRYSTAL_FOLLOW
//=========================================================
DEFINE_SCHEDULE
(
	SCHED_KRYSTAL_FOLLOW,

	"	Tasks"
	"		TASK_MOVE_TO_TARGET_RANGE		128"
	"	"
	"	Interrupts"
	"			COND_HEAR_DANGER"
	"			COND_PROVOKED"
)

//=========================================================
// > SCHED_KRYSTAL_FACE_TARGET
//=========================================================
DEFINE_SCHEDULE
(
	SCHED_KRYSTAL_FACE_TARGET,

	"	Tasks"
	"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
	"		TASK_FACE_TARGET			0"
	"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
	"		TASK_SET_SCHEDULE			SCHEDULE:SCHED_KRYSTAL_FOLLOW"
	"	"
	"	Interrupts"
	"		COND_GIVE_WAY"
	"		COND_NEW_ENEMY"
	"		COND_LIGHT_DAMAGE"
	"		COND_HEAVY_DAMAGE"
	"		COND_PROVOKED"
	"		COND_HEAR_DANGER"
)

//=========================================================
// > SCHED_KRYSTAL_IDLE_STAND
//=========================================================
DEFINE_SCHEDULE
(
	SCHED_KRYSTAL_IDLE_STAND,

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
	"		COND_PROVOKED"
	"		COND_HEAR_COMBAT"
	"		COND_SMELL"
)

AI_END_CUSTOM_NPC()
