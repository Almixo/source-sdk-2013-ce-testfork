#include "cbase.h"
#include "hl1_ai_basenpc.h"

class CRat : public CHL1BaseNPC
{
	DECLARE_CLASS( CRat, CHL1BaseNPC );
public:
	void Spawn( void );
	void Precache( void );
	float MaxYawSpeed( void );
	Class_T  Classify( void );
};
LINK_ENTITY_TO_CLASS( monster_rat, CRat );

Class_T	CRat::Classify( void )
{
	return	CLASS_INSECT;
}

float CRat::MaxYawSpeed( void )
{
	/*int ys;

	switch ( GetActivity() )
	{
	case ACT_IDLE:
	default:
		ys = 45;
		break;
	}

	return ys;*/

	return 45.0f; //TODO: Does this change anything?
}

void CRat::Spawn()
{
	Precache();

	SetModel( "models/bigrat.mdl" );
	UTIL_SetSize( this, Vector( 0, 0, 0 ), Vector( 0, 0, 0 ) );

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_STEP );
	SetBloodColor( BLOOD_COLOR_RED );
	SetHealth( 8 );
	SetViewOffset( Vector( 0, 0, 6 ) );// position of the eyes relative to monster's origin.
	m_flFieldOfView = 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_NPCState = NPC_STATE_NONE;

	NPCInit();
}

void CRat::Precache()
{
	PrecacheModel( "models/bigrat.mdl" );
}