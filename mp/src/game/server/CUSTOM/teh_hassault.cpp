/***
*
*	Copyright (c) 2020, Magic Nipples.
*
*	Use and modification of this code is allowed as long
*	as credit is provided! Enjoy!
*
****/
//=========================================================
// Human Assault - Yore Dead
//=========================================================

#include "cbase.h"
#include "hl1_ai_basenpc.h"
#include "npcevent.h"
#include "ammodef.h"


//=========================================================
// Monster-specific Schedule Types
//=========================================================
enum
{
	SCHED_ASSAULT_SPINUP = LAST_SHARED_SCHEDULE + 1,
	SCHED_ASSAULT_SPINDOWN,
	SCHED_ASSAULT_FLINCH,
	SCHED_ASSAULT_MELEE,
	SCHED_ASSAULT_CHASE,
};

//=========================================================
// Monster-Specific tasks
//=========================================================
enum
{
	TASK_ASSAULT_SPINUP = LAST_SHARED_TASK + 2,
	TASK_ASSAULT_SPIN,
	TASK_ASSAULT_SPINDOWN,
};

//=========================================================
// Animation Events
//=========================================================
#define		HASSAULT_AE_SHOOT1		( 1 )
#define		HASSAULT_AE_SHOOT2		( 2 )
#define		HASSAULT_AE_SHOOT3		( 3 )
#define		HASSAULT_AE_SHOOT4		( 4 )
#define		HASSAULT_AE_SHOOT5		( 5 )
#define		HASSAULT_AE_SHOOT6		( 6 )
#define		HASSAULT_AE_SHOOT7		( 7 )
#define		HASSAULT_AE_SHOOT8		( 8 )
#define		HASSAULT_AE_SHOOT9		( 9 )
#define		HASSAULT_AE_MELEE		( 10 )

//=========================================================
// Let's define the class!
//=========================================================
class CHAssault : public CHL1BaseNPC
{
	DECLARE_CLASS( CHAssault, CHL1BaseNPC );
	DECLARE_DATADESC();
public:
	void Spawn( void );
	void Precache( void );
	Class_T Classify( void );
	float MaxYawSpeed( void );
	void AlertSound( void );
	void SpinDown( void );
	void HandleAnimEvent( animevent_t* pEvent );
	void Event_Killed( const CTakeDamageInfo &info );
	void FireGun( void );
	void Melee( void );
	void CallForBackup( char* szClassname, float flDist, EHANDLE hEnemy, const Vector vecLocation );

	bool CheckRangeAttack1( float flDot, float flDist );
	bool CheckMeleeAttack1( float flDot, float flDist );
	bool CheckMeleeAttack2( float flDot, float flDist );

	void StartTask( const Task_t* pTask );
	int SelectSchedule( void );
		
	int m_iAmmoType;

	DEFINE_CUSTOM_AI;

private:
	int m_ifirestate;
	bool bAlerted;

};


//=========================================================
// Tasks specific to this monster
//=========================================================
AI_BEGIN_CUSTOM_NPC( monster_human_assault, CHAssault )

	DECLARE_TASK( SCHED_ASSAULT_SPINUP )
	DECLARE_TASK( SCHED_ASSAULT_SPINDOWN )
	DECLARE_TASK( SCHED_ASSAULT_FLINCH )
	DECLARE_TASK( SCHED_ASSAULT_MELEE )
	DECLARE_TASK( SCHED_ASSAULT_CHASE )


	DEFINE_SCHEDULE
	(
		SCHED_ASSAULT_SPINUP,
		"	Tasks"
		"		TASK_STOP_MOVING			0"
		"		TASK_FACE_ENEMY				0"
		"		TASK_ASSAULT_SPINUP			0"
		"		TASK_SET_ACTIVITY			ACTIVITY:ACT_IDLE"
		"		TASK_WAIT					0.5"
		"		TASK_ASSAULT_SPIN			0"
		"		TASK_SET_SCHEDULE			SCHEDULE:SCHED_RANGE_ATTACK1"
		"	"
		"	Interrupts"
	)

	DEFINE_SCHEDULE
	(
		SCHED_ASSAULT_SPINDOWN,
		"	Tasks"
		"		TASK_STOP_MOVING			0"
		"		TASK_ASSAULT_SPINDOWN		0"
		"		TASK_SET_SCHEDULE			SCHEDULE:SCHED_IDLE"
		"	"
		"	Interrupts"
	)

	DEFINE_SCHEDULE
	(
		SCHED_ASSAULT_FLINCH,
		"	Tasks"
		"		TASK_ASSAULT_SPINDOWN		0"
		"		TASK_REMEMBER				MEMORY:FLINCHED"
		"		TASK_STOP_MOVING			0"
		"		TASK_SMALL_FLINCH			0"
		"	Interrupts"
	)

	DEFINE_SCHEDULE
	(
		SCHED_ASSAULT_MELEE,
		"	Tasks"
		"		TASK_ASSAULT_SPINDOWN		0"
		"		TASK_STOP_MOVING			0"
		"		TASK_FACE_ENEMY				0"
		"		TASK_MELEE_ATTACK1			0"
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_ENEMY_OCCLUDED"
	)

	DEFINE_SCHEDULE
	(
		SCHED_ASSAULT_CHASE,
		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE		SCHEDULE:SCHED_CHASE_ENEMY_FAILED"
		"		TASK_GET_PATH_TO_ENEMY		0"
		"		TASK_RUN_PATH				0"
		"		TASK_WAIT_FOR_MOVEMENT		0"
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_CAN_RANGE_ATTACK1"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_TASK_FAILED"
		"		COND_HEAR_SOUND"
		"		COND_HEAR_PLAYER"

	)
AI_END_CUSTOM_NPC()

LINK_ENTITY_TO_CLASS( monster_human_assault, CHAssault );

BEGIN_DATADESC(CHAssault)
	DEFINE_FIELD( bAlerted, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_ifirestate, FIELD_INTEGER ),
END_DATADESC()


//=========================================================
// Spawn
//=========================================================
void CHAssault::Spawn()
{
	Precache();

	SetModel( "models/hassault.mdl" );

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_STEP );
	SetHealth( 100 );
	SetViewOffset( VEC_VIEW );	// position of the eyes relative to monster's origin.

	m_flFieldOfView = 0.3;	// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_bloodColor = BLOOD_COLOR_RED;
	SetState( NPC_STATE_NONE );
	CapabilitiesAdd( bits_CAP_DOORS_GROUP );

	m_ifirestate = -1;
	bAlerted = false;
	
	NPCInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CHAssault::Precache()
{
	PrecacheModel( "models/hassault.mdl" );
	m_iAmmoType = GetAmmoDef()->Index( "12mmRound" );

	/*PrecacheSound( "hassault/hw_gun3.wav" );
	PrecacheSound( "hassault/hw_spin.wav" );
	PrecacheSound( "hassault/hw_spindown.wav" );
	PrecacheSound( "hassault/hw_spinup.wav" );

	PrecacheSound( "weapons/cbar_hitbod1.wav" );
	PrecacheSound( "weapons/cbar_hitbod2.wav" );
	PrecacheSound( "weapons/cbar_hitbod3.wav" );
	PrecacheSound( "hassault/hw_alert.wav" );*/
}

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
Class_T CHAssault::Classify( void )
{
	return	CLASS_HUMAN_MILITARY;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
float CHAssault::MaxYawSpeed( void )
{
	float ys;

	switch ( GetActivity() )
	{
	case ACT_IDLE:
		ys = 150;
		break;

	case ACT_WALK:
		ys = 120;
		break;

	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		ys = 180;
		break;

	case ACT_RANGE_ATTACK1:
		ys = 120;
		break;

	case ACT_MELEE_ATTACK1:
		ys = 50;
		break;

	default:
		ys = 90;
		break;
	}

	return ys;
}

//=========================================================
// Human Assault yells like Mr.T
//=========================================================
void CHAssault::AlertSound()
{
	if ( GetEnemy() != NULL )
	{
		/*if ( !bAlerted )
			EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "hassault/hw_alert.wav", 1, ATTN_NORM, 0, 100 );*/

		CallForBackup( "monster_human_grunt", 2048, GetEnemy(),	GetEnemyLKP());

		bAlerted = true;
	}
}

void CHAssault::SpinDown()
{
	if ( m_ifirestate >= 0 )
	{
		m_ifirestate = -1;
		//STOP_SOUND( ENT( pev ), CHAN_ITEM, "hassault/hw_spin.wav" );
		//STOP_SOUND( ENT( pev ), CHAN_WEAPON, "hassault/hw_gun3.wav" );
		//EMIT_SOUND_DYN( ENT( pev ), CHAN_BODY, "hassault/hw_spindown.wav", 1, ATTN_NORM, 0, 100 );
	}
}

//=========================================================
// Functions for each animation event
//=========================================================
void CHAssault::HandleAnimEvent( animevent_t* pEvent )
{
	switch ( pEvent->event )
	{
	case HASSAULT_AE_SHOOT1:
	case HASSAULT_AE_SHOOT2:
	case HASSAULT_AE_SHOOT3:
	case HASSAULT_AE_SHOOT4:
	case HASSAULT_AE_SHOOT5:
	case HASSAULT_AE_SHOOT6:
	case HASSAULT_AE_SHOOT7:
	case HASSAULT_AE_SHOOT8:
	case HASSAULT_AE_SHOOT9:

		switch ( g_iSkillLevel )
		{
		default:
			m_flPlaybackRate = 1.75;
			break;

		case SKILL_HARD:
			m_flPlaybackRate = 2;
			break;
		}
		FireGun();
		break;


	case HASSAULT_AE_MELEE:
		Melee();
		break;

	default:
		BaseClass::HandleAnimEvent( pEvent );
	}
}

//=========================================================
// Stop all sounds when dead
//=========================================================
void CHAssault::Event_Killed( const CTakeDamageInfo &info )
{
	SpinDown();

	SetUse( NULL );
	BaseClass::Event_Killed( info );
}

void CHAssault::FireGun()
{
	Vector vecShootOrigin;

	vecShootOrigin = GetAbsOrigin() + Vector(0, 0, 35);
	Vector vecShootDir = GetEnemy()->BodyTarget( vec3_origin );
	DoMuzzleFlash();

	/*Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT( 40, 90 ) + gpGlobals->v_up * RANDOM_FLOAT( 75, 200 ) + gpGlobals->v_forward * RANDOM_FLOAT( -40, 40 );
	EjectBrass( vecShootOrigin - vecShootDir, vecShellVelocity, pev->angles.y, m_iShell, TE_BOUNCE_SHELL );
	FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_4DEGREES, 2048, BULLET_MONSTER_MP5, 1 );*/

	FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_4DEGREES, 2048, m_iAmmoType, 1 );

	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), 384, 0.3);

	//EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "hassault/hw_gun3.wav", 1, ATTN_NORM, 0, 100 );
}

void CHAssault::Melee()
{
	//CBaseEntity* pHurt = CheckTraceHullAttack( 50, gSkillData.zombieDmgOneSlash, DMG_CLUB );
	CBaseEntity* pHurt = CheckTraceHullAttack( 50, Vector( -16, -16, -16 ), Vector( 16, 16, 16 ), 50, DMG_CLUB );
	if ( pHurt )
	{
		if ( pHurt->GetFlags() & ( FL_NPC | FL_CLIENT ) )
		{
			QAngle ang;

			switch ( RandomInt( 0, 3 ) )
			{
			case 0:
				/*pHurt->pev->punchangle.z = -15;
				pHurt->pev->punchangle.x = 5;
				pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_forward * -100;*/
				ang.x = 5;
				ang.z = -15;

				pHurt->ViewPunch( ang );
				break;
			case 1:
				/*pHurt->pev->punchangle.z = 15;
				pHurt->pev->punchangle.y = 10;
				pHurt->pev->punchangle.x = -5;
				pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_forward * -100;*/

				ang.x = -5;
				ang.y = 10;
				ang.z = 15;

				pHurt->ViewPunch( ang );
				break;
			case 2:
				/*pHurt->pev->punchangle.z = -15;
				pHurt->pev->punchangle.y = -10;
				pHurt->pev->punchangle.x = 5;
				pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_forward * -100;*/

				ang.x = 5;
				ang.y = -10;
				ang.z = -15;

				pHurt->ViewPunch( ang );
				break;
			case 3:
				/*pHurt->pev->punchangle.z = 15;
				pHurt->pev->punchangle.x = -5;
				pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_forward * -100;*/

				ang.x = -5;
				ang.z = 15;

				pHurt->ViewPunch( ang );
				break;
			}
		}

		// Play a random attack hit sound
		switch ( RandomInt( 0, 2 ) )
		{
		case 0: //EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "weapons/cbar_hitbod1.wav", 1, ATTN_NORM, 0, 100 ); break;
		case 1: //EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "weapons/cbar_hitbod2.wav", 1, ATTN_NORM, 0, 100 ); break;
		case 2: //EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "weapons/cbar_hitbod3.wav", 1, ATTN_NORM, 0, 100 ); break;
			break;
		}
	}
}

void CHAssault::CallForBackup( char* szClassname, float flDist, EHANDLE hEnemy, const Vector vecLocation )
{
	//ALERT( at_console, "help\n" );

	// skip ones not on my netname
	if (  m_SquadName != NULL_STRING )
		return;

	CBaseEntity* pEntity = NULL;

	while ( ( pEntity = gEntList.FindEntityByClassname( this, "monster_human_grunt" ) ) != NULL )//UTIL_FindEntityByString( pEntity, "netname", STRING( pev->netname ) ) ) != NULL )
	{
		variant_t tmp;
		pEntity->ReadKeyField( "netname", &tmp );

		if ( tmp.String()[0] == '\0' )
			continue;

		float d = ( GetAbsOrigin() - pEntity->GetAbsOrigin() ).Length();
		if ( d < flDist )
		{
			CAI_BaseNPC* pMonster = pEntity->MyNPCPointer();
			if ( pMonster )
			{
				pMonster->HasMemory(bits_MEMORY_PROVOKED);
				//pMonster->PushEnemy( hEnemy, vecLocation );
			}
		}
	}
}

//=========================================================
// shoot while are melee range | UNDONE!!
//=========================================================
bool CHAssault::CheckRangeAttack1( float flDot, float flDist )
{
	if ( flDot > 0.5 )
		//if (flDot > 0.5 && flDist > 70)
	{
		return true;
	}
	return false;
}

//=========================================================
// Added 0.52 melee animatons so don't just return false
//=========================================================
bool CHAssault::CheckMeleeAttack1( float flDot, float flDist )
{
	//return FALSE;
	if ( flDot > 0.5 && flDist < 70 )
	{
		return true;
	}
	return false;
}

bool CHAssault::CheckMeleeAttack2( float flDot, float flDist )
{
	return false;
}

void CHAssault::StartTask( const Task_t* pTask )
{
	//ALERT(at_console, "m_ifirestate %i\n", m_ifirestate);
	switch ( pTask->iTask )
	{
	case TASK_ASSAULT_SPINUP:
		if ( m_ifirestate == -1 )
		{
			m_ifirestate = 0;
			//STOP_SOUND( ENT( pev ), CHAN_WEAPON, "hassault/hw_gun3.wav" );
			//EMIT_SOUND_DYN( ENT( pev ), CHAN_BODY, "hassault/hw_spinup.wav", 1, ATTN_NORM, 0, 100 );
		}
		TaskComplete();
		break;

	case TASK_ASSAULT_SPIN:
		m_ifirestate = 1;
		//STOP_SOUND( ENT( pev ), CHAN_BODY, "hassault/hw_spinup.wav" );
		//EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "hassault/hw_spin.wav", 1, ATTN_NORM, 0, 100 );
		TaskComplete();
		break;

	case TASK_ASSAULT_SPINDOWN:
		SpinDown();
		TaskComplete();
		break;


	default:
		BaseClass::StartTask( pTask );
		break;
	}
}

//=========================================================
// Load up the schedules so ai isn't dumb
//=========================================================
int CHAssault::SelectSchedule( void )
{
	// Call another switch class, to check the monster's attitude
	switch ( m_NPCState )
	{
	case NPC_STATE_IDLE:
	case NPC_STATE_ALERT:
		SpinDown();
		break;

	case NPC_STATE_COMBAT:
	{
		if ( HasCondition( COND_NEW_ENEMY ) )
			AlertSound();

		if ( HasCondition( COND_ENEMY_DEAD ) || HasCondition( COND_ENEMY_TOO_FAR ) )
		{
			SpinDown();
			return BaseClass::SelectSchedule();
		}

		if ( HasCondition( COND_LIGHT_DAMAGE ) && !HasMemory( bits_MEMORY_FLINCHED ) )
			return SCHED_ASSAULT_FLINCH;

		if ( ( !HasCondition( COND_SEE_ENEMY ) ) || ( HasCondition( COND_ENEMY_OCCLUDED ) ) )
			return SCHED_ASSAULT_SPINDOWN;

		if ( HasCondition( COND_CAN_MELEE_ATTACK1 ) )
			return SCHED_ASSAULT_MELEE;

		if ( HasCondition( COND_CAN_RANGE_ATTACK1 ) )
		{
			if ( m_ifirestate == -1 )
				return SCHED_ASSAULT_SPINUP;
			if ( m_ifirestate == 0 )
				return SCHED_RANGE_ATTACK1;
		}
	}
	break;
	}
	//if all else fails, the base probably knows what to do
	return BaseClass::SelectSchedule();
}