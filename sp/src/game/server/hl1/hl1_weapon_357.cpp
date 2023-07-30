//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		357 - hand gun
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hl1_basecombatweapon_shared.h"
#include "gamerules.h"
#include "soundent.h"

//-----------------------------------------------------------------------------
// CWeapon357
//-----------------------------------------------------------------------------

class CWeapon357 : public CBaseHL1CombatWeapon
{
	DECLARE_CLASS( CWeapon357, CBaseHL1CombatWeapon );
public:

	CWeapon357( void );

	bool	Deploy( void );
	void	PrimaryAttack( void );
	void	SecondaryAttack( void );
	bool	Reload( void );
	void	WeaponIdle( void );
	bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

private:
	void	ToggleZoom( void );
	bool	m_bInZoom;
};

LINK_ENTITY_TO_CLASS( weapon_357, CWeapon357 );

PRECACHE_WEAPON_REGISTER( weapon_357 );

IMPLEMENT_SERVERCLASS_ST( CWeapon357, DT_Weapon357 )
END_SEND_TABLE()

BEGIN_DATADESC( CWeapon357 )
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeapon357::CWeapon357( void )
{
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= false;
	m_bInZoom			= false;
}

bool CWeapon357::Deploy( void )
{
	// Bodygroup stuff not currently working correctly
	if ( g_pGameRules->IsMultiplayer() )
	{
		// enable laser sight geometry.
		SetBodygroup( 4, 1 );
	}
	else
	{
		SetBodygroup( 4, 0 );
	}

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeapon357::PrimaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if (pPlayer == NULL) return;

	if ( m_iClip1 <= 0 )
	{
		if ( !m_bFireOnEmpty )
		{
			Reload();
		}
		else
		{
			WeaponSound( EMPTY );
			m_flNextPrimaryAttack = 0.15;
		}

		return;
	}

	WeaponSound( SINGLE );
	pPlayer->DoMuzzleFlash();

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.75;
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.75;

	m_iClip1--;

	Vector vecSrc		= pPlayer->Weapon_ShootPosition();
	Vector vecAiming	= pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );	

	FireBulletsInfo_t info( 1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, MAX_TRACE_LENGTH, m_iPrimaryAmmoType );
	info.m_pAttacker = pPlayer;

	pPlayer->FireBullets( info );

	pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );

	pPlayer->ViewPunch( QAngle( -10, 0, 0 ) );

	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), 600, 0.2 );

	if ( !m_iClip1 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 ); 
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeapon357::SecondaryAttack( void )
{
	if (g_pGameRules->IsMultiplayer() || sv_cheats->GetBool())
	{
		ToggleZoom();
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5;
	}
}


bool CWeapon357::Reload( void )
{
	bool fRet = DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );
	if ( fRet )
	{
		if ( m_bInZoom )
			ToggleZoom();
	}

	return fRet;
}


void CWeapon357::WeaponIdle( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( pPlayer == NULL ) return;

	pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if ( !HasWeaponIdleTimeElapsed() )
		return;

	if ( RandomFloat( 0, 1 ) <= 0.9 )
	{
		SendWeaponAnim( ACT_VM_IDLE );
	}
	else
	{
		SendWeaponAnim( ACT_VM_FIDGET );
	}
}


bool CWeapon357::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	if ( m_bInZoom )
		ToggleZoom();

	return BaseClass::Holster( pSwitchingTo );
}


void CWeapon357::ToggleZoom( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( pPlayer == NULL ) return;

	if ( m_bInZoom )
	{
		if ( pPlayer->SetFOV( this, 0 ) )
		{
			m_bInZoom = false;
			pPlayer->ShowViewModel( true );
		}
	}
	else
	{
		if ( pPlayer->SetFOV( this, 40 ) )
		{
			m_bInZoom = true;
			pPlayer->ShowViewModel( false );
		}
	}
}