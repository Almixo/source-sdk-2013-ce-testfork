//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Chumtoad
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hl1mp_basecombatweapon_shared.h"

#ifndef CLIENT_DLL
#include "teh_npc_chumtoad.h"
#endif

#include "beam_shared.h"


//-----------------------------------------------------------------------------
// CWeaponChumtoad
//-----------------------------------------------------------------------------

#ifdef CLIENT_DLL
#define CWeaponChumtoad C_WeaponChumtoad
#endif

class CWeaponChumtoad : public CBaseHL1MPCombatWeapon
{
	DECLARE_CLASS( CWeaponChumtoad, CBaseHL1MPCombatWeapon );
public:

	CWeaponChumtoad( void );

	void	Precache( void );
	void	PrimaryAttack( void );
	void	WeaponIdle( void );
	bool	Deploy( void );
	bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );

	DECLARE_NETWORKCLASS();	//serverclass
	DECLARE_PREDICTABLE();
	DECLARE_DATADESC();

private:
	CNetworkVar( bool, m_bJustThrown );
};

LINK_ENTITY_TO_CLASS( weapon_chumtoad, CWeaponChumtoad );

PRECACHE_WEAPON_REGISTER( weapon_chumtoad );

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponChumtoad, DT_WeaponChumtoad )

BEGIN_NETWORK_TABLE( CWeaponChumtoad, DT_WeaponChumtoad )
#ifdef CLIENT_DLL
RecvPropBool( RECVINFO( m_bJustThrown ) ),
#else
SendPropBool( SENDINFO( m_bJustThrown ) ),
#endif // CLIENT_DLL
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponChumtoad )
#ifdef CLIENT_DLL
DEFINE_PRED_FIELD( m_bJustThrown, FIELD_BOOLEAN, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK ),
#endif // CLIENT_DLL
END_PREDICTION_DATA()

BEGIN_DATADESC( CWeaponChumtoad )
	DEFINE_FIELD( m_bJustThrown, FIELD_BOOLEAN ),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponChumtoad::CWeaponChumtoad( void )
{
	m_bReloadsSingly = false;
	m_bFiresUnderwater = true;
	m_bJustThrown = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponChumtoad::Precache( void )
{
	BaseClass::Precache();

	UTIL_PrecacheOther( "monster_chumtoad" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponChumtoad::PrimaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( !pPlayer )
		return;

	if ( pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		return;

	Vector vecForward;
	pPlayer->EyeVectors( &vecForward );

	// find place to toss monster
	// Does this need to consider a crouched player?
	Vector vecStart = pPlayer->EyePosition();
	Vector vecEnd = vecStart + ( vecForward * 44 );
	trace_t tr;
	UTIL_TraceLine( vecStart, vecEnd, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );
	if ( tr.allsolid || tr.startsolid || tr.fraction <= 0.25 )
		return;

	// player "shoot" animation
	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

#ifndef CLIENT_DLL
	CChumToad *pChumtoad = (CChumToad *)Create( "monster_chumtoad", tr.endpos, pPlayer->EyeAngles(), GetOwner() );
	if ( pChumtoad )
	{
		pChumtoad->SetAbsVelocity( vecForward * 200 + pPlayer->GetAbsVelocity() );
	}

	CSoundEnt::InsertSound( SOUND_DANGER, GetAbsOrigin(), 200, 0.2 );
#endif

	// play hunt sound
	WeaponSound( SINGLE );

	pPlayer->RemoveAmmo( 1, m_iPrimaryAmmoType );

	m_bJustThrown = true;

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.3;
	SetWeaponIdleTime( gpGlobals->curtime + 1.0 );
}

void CWeaponChumtoad::WeaponIdle( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( !pPlayer )
		return;

	if ( !HasWeaponIdleTimeElapsed() )
		return;

	if ( m_bJustThrown )
	{
		m_bJustThrown = false;

		if ( pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		{
			if ( !pPlayer->SwitchToNextBestWeapon( pPlayer->GetActiveWeapon() ) )
				Holster();
		}
		else
		{
			SendWeaponAnim( ACT_VM_DRAW );
			SetWeaponIdleTime( gpGlobals->curtime + random->RandomFloat( 10, 15 ) );
		}
	}
	else
	{
		if ( RandomFloat( 0, 1 ) <= 0.75 )
		{
			SendWeaponAnim( ACT_VM_IDLE );
		}
		else
		{
			SendWeaponAnim( ACT_VM_FIDGET );
		}
	}
}

bool CWeaponChumtoad::Deploy( void )
{
	WeaponSound( DEPLOY );

	return BaseClass::Deploy();
}

bool CWeaponChumtoad::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( !pPlayer )
		return true;

	if ( !BaseClass::Holster( pSwitchingTo ) )
		return true;

	if ( pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
	{
#ifndef CLIENT_DLL
		SetThink( &CWeaponChumtoad::DestroyItem );
		SetNextThink( gpGlobals->curtime + 0.1 );
#endif
	}

	pPlayer->SetNextAttack( gpGlobals->curtime + 0.5 );

	return true;
}
