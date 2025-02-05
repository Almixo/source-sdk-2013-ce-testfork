#include "cbase.h"
#include "CBaseDodWeapon_shared.h"

LINK_ENTITY_TO_CLASS( basedodcombatweapon, CBaseDoDCombatWeapon );

IMPLEMENT_NETWORKCLASS_ALIASED( BaseDoDCombatWeapon, DT_BaseDoDCombatWeapon )

BEGIN_NETWORK_TABLE( CBaseDoDCombatWeapon, DT_BaseDoDCombatWeapon )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CBaseDoDCombatWeapon )
END_PREDICTION_DATA()

CBaseDoDCombatWeapon::CBaseDoDCombatWeapon()
{
	m_iWeaponType = -1;

	m_pWpnInfo = nullptr;
}

void CBaseDoDCombatWeapon::Spawn( void )
{
	DoDSpawn();
}

void CBaseDoDCombatWeapon::DoDSpawn( void )
{
	WEAPON_FILE_INFO_HANDLE	hWpnInfo = LookupWeaponInfoSlot( GetClassname() );

	Assert( hWpnInfo != GetInvalidWeaponInfoHandle() );

	CDoDWeaponParse *pWeaponInfo = dynamic_cast<CDoDWeaponParse*>( GetFileWeaponInfoFromHandle( hWpnInfo ) );

	Assert( pWeaponInfo && "Failed to get CDoDWeaponParse in weapon spawn" );

	m_pWpnInfo = pWeaponInfo;

	BaseClass::Spawn();
}

float CBaseDoDCombatWeapon::GetFireDelay( void ) const
{
	return m_pWpnInfo->m_flFireDelay;
}

float CBaseDoDCombatWeapon::GetSecondaryFireDelay( void ) const
{
	return m_pWpnInfo->m_flSecondaryFireDelay;
}

void CBaseDoDCombatWeapon::ApplyRecoil( CBasePlayer *pPlayer )
{
#ifndef CLIENT_DLL
    float recoil_y = m_pWpnInfo->m_flRecoil;
    float recoil_x = recoil_y / 4.0f;

    if ( pPlayer->m_Local.m_bDucked )
    {
        recoil_x /= 2.0f;
        recoil_y /= 2.0f;
    }

    recoil_y *= RandomFloat( 0.8f, 1.15f );
    recoil_x *= RandomFloat( -0.8f, 0.8f );

    QAngle ang = pPlayer->EyeAngles();
    ang[PITCH] += recoil_y * -1;
    ang[YAW] += recoil_x;

	pPlayer->SnapEyeAngles( ang );
#endif
}

void CBaseDoDCombatWeapon::ApplySpread( CBasePlayer *pPlayer, Vector *vecSpread )
{
	float spread = m_pWpnInfo->m_flSpread;
	
    if ( pPlayer->GetAbsVelocity().Length2D() > 175.0f )
    {
        switch ( m_iWeaponType )
        {
        case COLT:
        case LUGER:
            spread *= 1 + 0.1f;
            break;

        case THOMPSON:
        case MP_40:
            spread *= 1 + 0.1f;
            break;

        case KAR98:
        case M1RIFLE:
        default:
            spread *= 4.0f;
            break;
        }
    }

	if ( pPlayer->m_Local.m_bDucked )
		spread /= 2.0f;

	vecSpread->x = vecSpread->y = vecSpread->z = spread;
}

void CBaseDoDCombatWeapon::FireGun( bool automatic )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( !pPlayer )
		return;

	if ( !automatic && ( pPlayer->m_afButtonLast & IN_ATTACK ) )
		return;

	if ( m_iClip1 <= 0 )
	{
		if ( !m_bFireOnEmpty )
		{
			Reload();
		}
		else
		{
			WeaponSound( EMPTY );
			m_flNextEmptySoundTime = gpGlobals->curtime + 1.0;
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.15f;
            m_flNextSecondaryAttack = gpGlobals->curtime + 0.15;
		}

		return;
	}

	WeaponSound( SINGLE );
	pPlayer->DoMuzzleFlash();

#ifdef GAME_DLL
	pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );
#endif

	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireDelay();
    m_flNextSecondaryAttack = gpGlobals->curtime + GetSecondaryFireDelay();
	m_flTimeWeaponIdle = gpGlobals->curtime + RandomInt( 2, 5 );

	m_iClip1--;

	if ( m_iClip1 == 0 )
	{
		SendWeaponAnim( GetLastRoundActivity() );

		if ( m_iWeaponType == M1RIFLE )
			WeaponSound( SPECIAL1 );
	}
	else
		SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	/*if ( m_iClip1 == 0 && m_bHasLastRoundActivity )
	{
		SendWeaponAnim( GetLastRoundActivity() );

		if ( m_iWeaponType == M1RIFLE )
			WeaponSound( SPECIAL1 );
	}
	else
		SendWeaponAnim( ACT_VM_PRIMARYATTACK );*/

	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector vecAiming = pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );
	Vector vecSpread;

	ApplySpread( pPlayer, &vecSpread );

	FireBulletsInfo_t info( 1, vecSrc, vecAiming, vecSpread, MAX_TRACE_LENGTH, m_iPrimaryAmmoType ); // 3 tracer count rip
	pPlayer->FireBullets( info );

	ApplyRecoil( pPlayer );

#ifndef CLIENT_DLL
	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), 600, 0.2, GetOwner() );
#endif
}

void CBaseDoDCombatWeapon::WeaponIdle( void )
{
	if ( !HasWeaponIdleTimeElapsed() )
		return;

	if ( m_iClip1 == 0 )
		SendWeaponAnim( GetIdleEmptyActivity() );
	else
		SendWeaponAnim( ACT_VM_IDLE );

	m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration();
}