//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "AI_BaseNPC.h"
#include "player.h"
#include "hl1_weapon_mp5.h"
#include "hl1_grenade_mp5.h"
#include "gamerules.h"
#include "game.h"
#include "in_buttons.h"
#include "soundent.h"

extern ConVar    sk_plr_dmg_mp5_grenade;	
extern ConVar    sk_max_mp5_grenade;
extern ConVar	 sk_mp5_grenade_radius;

//=========================================================
//=========================================================

LINK_ENTITY_TO_CLASS( weapon_mp5, CWeaponMP5 );

PRECACHE_WEAPON_REGISTER(weapon_mp5);

IMPLEMENT_SERVERCLASS_ST( CWeaponMP5, DT_WeaponMP5 )
END_SEND_TABLE()

BEGIN_DATADESC( CWeaponMP5 )
END_DATADESC()

CWeaponMP5::CWeaponMP5( )
{
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= false;
}

void CWeaponMP5::Precache( void )
{
	BaseClass::Precache();

	UTIL_PrecacheOther( "grenade_mp5" );
}


void CWeaponMP5::PrimaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( pPlayer == NULL ) return;

	if ( m_iClip1 <= 0 )
	{
		DryFire();
		return;
	}

	WeaponSound( SINGLE );

	pPlayer->DoMuzzleFlash();

	m_iClip1--;

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	m_flNextPrimaryAttack	= gpGlobals->curtime + 0.1;

	Vector vecSrc		= pPlayer->Weapon_ShootPosition();
	Vector vecAiming	= pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );	

	if ( g_pGameRules->IsMultiplayer() )
	{
		// multi player spread
		FireBulletsInfo_t info( 1, vecSrc, vecAiming, VECTOR_CONE_6DEGREES, MAX_TRACE_LENGTH, m_iPrimaryAmmoType );
		info.m_pAttacker = pPlayer;
		info.m_iTracerFreq = 2;

		pPlayer->FireBullets( info );
	}
	else
	{
		// single player spread
		FireBulletsInfo_t info( 1, vecSrc, vecAiming, VECTOR_CONE_3DEGREES, MAX_TRACE_LENGTH, m_iPrimaryAmmoType );
		info.m_pAttacker = pPlayer;
		info.m_iTracerFreq = 2;

		pPlayer->FireBullets( info );
	}

	EjectShell( pPlayer, 0 );

	pPlayer->ViewPunch( QAngle( random->RandomFloat( -0.5, 0.5 ), 0, 0 ) );

	pPlayer->DoMuzzleFlash();

	pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );

	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), 600, 0.2 );

	if ( !m_iClip1 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	SetWeaponIdleTime( gpGlobals->curtime + random->RandomFloat( 10, 15 ) );
}


void CWeaponMP5::SecondaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( pPlayer == NULL ) return;

	if ( pPlayer->GetAmmoCount( m_iSecondaryAmmoType ) <= 0 )
	{
		DryFire( );
		return;
	}

	WeaponSound(WPN_DOUBLE);

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector vecThrow = pPlayer->GetAutoaimVector( 0 ) * 800;
	QAngle angGrenAngle;

	VectorAngles( vecThrow, angGrenAngle );

	CGrenadeMP5 *m_pMyGrenade = static_cast<CGrenadeMP5*>( Create( "grenade_mp5", vecSrc, angGrenAngle, GetOwner() ) );
	m_pMyGrenade->SetAbsVelocity( vecThrow );
	m_pMyGrenade->SetLocalAngularVelocity( QAngle( random->RandomFloat( -100, 500 ), 0, 0 ) );
	m_pMyGrenade->SetMoveType( MOVETYPE_FLYGRAVITY ); 
	m_pMyGrenade->SetThrower( GetOwner() );
	m_pMyGrenade->SetDamage( sk_plr_dmg_mp5_grenade.GetFloat() * g_pGameRules->GetDamageMultiplier() );

	SendWeaponAnim( ACT_VM_SECONDARYATTACK );
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	pPlayer->ViewPunch( QAngle( -10, 0, 0 ) );

	// Register a muzzleflash for the AI.
	pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );

	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), 600, 0.2 );

	// Decrease ammo
	pPlayer->RemoveAmmo( 1, m_iSecondaryAmmoType );
	if ( pPlayer->GetAmmoCount( m_iSecondaryAmmoType ) <= 0 )
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0); 
	}

	m_flNextPrimaryAttack = gpGlobals->curtime + 1.0;
	m_flNextSecondaryAttack = gpGlobals->curtime + 1.0;
	SetWeaponIdleTime( gpGlobals->curtime + 5.0 );
}


void CWeaponMP5::DryFire( void )
{
	WeaponSound( EMPTY );
	m_flNextPrimaryAttack	= gpGlobals->curtime + 0.15;
	m_flNextSecondaryAttack	= gpGlobals->curtime + 0.15;
}


void CWeaponMP5::WeaponIdle( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( pPlayer == NULL ) return;

	BaseClass::WeaponIdle();

	pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if ( HasWeaponIdleTimeElapsed() )
		SetWeaponIdleTime( gpGlobals->curtime + RandomInt( 3, 5 ) );
}