//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Glock - hand gun
//
//=============================================================================//

#include "cbase.h"
#include "hl1_basecombatweapon_shared.h"
#include "soundent.h"


class CWeaponGlock : public CBaseHL1CombatWeapon
{
	DECLARE_CLASS( CWeaponGlock, CBaseHL1CombatWeapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
public:

	CWeaponGlock( void );

	void	PrimaryAttack( void );
	void	SecondaryAttack( void );
	bool	Reload( void );
	void	WeaponIdle( void );
	void	DryFire( void );

private:
	void	GlockFire( float flSpread, float flCycleTime, bool fUseAutoAim );
};

LINK_ENTITY_TO_CLASS( weapon_glock, CWeaponGlock );

PRECACHE_WEAPON_REGISTER( weapon_glock );

IMPLEMENT_SERVERCLASS_ST( CWeaponGlock, DT_WeaponGlock )
END_SEND_TABLE()

BEGIN_DATADESC( CWeaponGlock )
END_DATADESC()


CWeaponGlock::CWeaponGlock( void )
{
	m_bReloadsSingly = false;
	m_bFiresUnderwater = true;
}

void CWeaponGlock::DryFire( void )
{
	WeaponSound( EMPTY );
	SendWeaponAnim( ACT_VM_DRYFIRE );

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
}

void CWeaponGlock::PrimaryAttack( void )
{
	GlockFire( 0.01, 0.3, true );
}

void CWeaponGlock::SecondaryAttack( void )
{
	GlockFire( 0.1, 0.2, false );
}

void CWeaponGlock::GlockFire( float flSpread, float flCycleTime, bool fUseAutoAim )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( pPlayer == NULL )
		return;

	if ( m_iClip1 <= 0 )
	{
		if ( !m_bFireOnEmpty )
		{
			Reload();
		}
		else
		{
			DryFire();
		}

		return;
	}

	WeaponSound( SINGLE );

	pPlayer->DoMuzzleFlash();

	m_iClip1--;

	if ( m_iClip1 == 0 )
		SendWeaponAnim( ACT_GLOCK_SHOOTEMPTY );
	else
		SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	m_flNextPrimaryAttack = gpGlobals->curtime + flCycleTime;
	m_flNextSecondaryAttack = gpGlobals->curtime + flCycleTime;

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector vecAiming;

	if ( fUseAutoAim )
	{
		vecAiming = pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );
	}
	else
	{
		vecAiming = pPlayer->GetAutoaimVector( 0 );
	}

	pPlayer->FireBullets( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 0 );

	pPlayer->ViewPunch( QAngle( -2, 0, 0 ) );
	pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );

	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), 400, 0.2 );

	if ( !m_iClip1 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
	}

	SetWeaponIdleTime( gpGlobals->curtime + RandomFloat( 10, 15 ) );
}

bool CWeaponGlock::Reload( void )
{
	bool iResult = false;

	if ( m_iClip1 == 0 )
		iResult = DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_GLOCK_SHOOT_RELOAD );
	else
		iResult = DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );

	if ( iResult )
	{
		SetWeaponIdleTime( gpGlobals->curtime + RandomFloat( 10, 15 ) );
	}

	return iResult;
}

void CWeaponGlock::WeaponIdle( void )
{
	// only idle if the slid isn't back
	if ( m_iClip1 != 0 )
	{
		BaseClass::WeaponIdle();
	}
}
