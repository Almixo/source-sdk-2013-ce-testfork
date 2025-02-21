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

void CBaseDoDCombatWeapon::ItemPostFrame( void )
{
    CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
    if ( !pOwner )
        return;

    UpdateAutoFire();

    //Track the duration of the fire
    //FIXME: Check for IN_ATTACK2 as well?
    //FIXME: What if we're calling ItemBusyFrame?
    m_fFireDuration = ( pOwner->m_nButtons & IN_ATTACK ) ? ( m_fFireDuration + gpGlobals->frametime ) : 0.0f;

    if ( UsesClipsForAmmo1() )
    {
        CheckReload();
    }

    bool bFired = false;

    // Secondary attack has priority
    if ( ( pOwner->m_nButtons & IN_ATTACK2 ) && CanPerformSecondaryAttack() )
    {
        if ( UsesSecondaryAmmo() && pOwner->GetAmmoCount( m_iSecondaryAmmoType ) <= 0 )
        {
            if ( m_flNextEmptySoundTime < gpGlobals->curtime )
            {
                m_flNextSecondaryAttack = m_flNextEmptySoundTime = gpGlobals->curtime + 0.5;
            }
        }
        else if ( pOwner->GetWaterLevel() == 3 && m_bAltFiresUnderwater == false )
        {
            // This weapon doesn't fire underwater
            WeaponSound( EMPTY );
            m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
            return;
        }
        else
        {
            SecondaryAttack();

            // Secondary ammo doesn't have a reload animation
            if ( UsesClipsForAmmo2() )
            {
                // reload clip2 if empty
                if ( m_iClip2 < 1 )
                {
                    pOwner->RemoveAmmo( 1, m_iSecondaryAmmoType );
                    m_iClip2 = m_iClip2 + 1;
                }
            }
        }
    }

    if ( !bFired && ( pOwner->m_nButtons & IN_ATTACK ) && ( m_flNextPrimaryAttack <= gpGlobals->curtime ) )
    {
        // Clip empty? Or out of ammo on a no-clip weapon?
        if ( !IsMeleeWeapon() &&
            ( ( UsesClipsForAmmo1() && m_iClip1 <= 0 ) || ( !UsesClipsForAmmo1() && pOwner->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 ) ) )
        {
            HandleFireOnEmpty();
        }
        else if ( pOwner->GetWaterLevel() == 3 && m_bFiresUnderwater == false )
        {
            // This weapon doesn't fire underwater
            WeaponSound( EMPTY );
            m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
            return;
        }
        else
        {
            //NOTENOTE: There is a bug with this code with regards to the way machine guns catch the leading edge trigger
            //			on the player hitting the attack key.  It relies on the gun catching that case in the same frame.
            //			However, because the player can also be doing a secondary attack, the edge trigger may be missed.
            //			We really need to hold onto the edge trigger and only clear the condition when the gun has fired its
            //			first shot.  Right now that's too much of an architecture change -- jdw

            // If the firing button was just pressed, or the alt-fire just released, reset the firing time
            if ( ( pOwner->m_afButtonPressed & IN_ATTACK ) || ( pOwner->m_afButtonReleased & IN_ATTACK2 ) )
            {
                m_flNextPrimaryAttack = gpGlobals->curtime;
            }

            PrimaryAttack();

            if ( AutoFiresFullClip() )
            {
                m_bFiringWholeClip = true;
            }

#ifdef CLIENT_DLL
            pOwner->SetFiredWeapon( true );
#endif
        }
    }

    // -----------------------
    //  Reload pressed / Clip Empty
    //  Can only start the Reload Cycle after the firing cycle
    if ( ( pOwner->m_nButtons & IN_RELOAD ) && m_flNextPrimaryAttack <= gpGlobals->curtime && UsesClipsForAmmo1() && !m_bInReload )
    {
        // reload when reload is pressed, or if no buttons are down and weapon is empty.
        Reload();
        m_fFireDuration = 0.0f;
    }

    // -----------------------
    //  No buttons down
    // -----------------------
    if ( !( ( pOwner->m_nButtons & IN_ATTACK ) || ( pOwner->m_nButtons & IN_ATTACK2 ) || ( CanReload() && pOwner->m_nButtons & IN_RELOAD ) ) )
    {
        // no fire buttons down or reloading
        if ( !ReloadOrSwitchWeapons() && ( m_bInReload == false ) )
        {
            WeaponIdle();
        }
    }
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
	float recoil_x = recoil_y / 2.0f;

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

    if ( !(pPlayer->GetFlags() & FL_ONGROUND) )
    {
        switch ( m_iWeaponType )
        {

        // pistols
        case LUGER:
        case COLT:
            break;
        
        // MGs
        case THOMPSON:
        case MP_40:
            spread *= 1 + 0.5f;
            break;
        
        // Carbines
        case M1CARBINE:
        case K_43:
            spread *= 4.0f;
            break;

        //everything else
        default:
            spread *= 10.0f;
            break;
        }
    }
        

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

        case M1CARBINE:
        case K_43:
            spread *= 2.0f;
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

void CBaseDoDCombatWeapon::FinishReload( void )
{
	if ( m_iWeaponType != KAR98 )
	{
		BaseClass::FinishReload();
		return;
	}
		
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( !pPlayer )
		return;

	// If I use primary clips, reload primary
	if ( UsesClipsForAmmo1() )
	{
		int primary = Min( GetMaxClip1(), pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) );
		m_iClip1 = primary;
		pPlayer->RemoveAmmo( primary, m_iPrimaryAmmoType );
	}

	// If I use secondary clips, reload secondary
	if ( UsesClipsForAmmo2() )
	{
		int secondary = Min( GetMaxClip2(), pPlayer->GetAmmoCount( m_iSecondaryAmmoType ) );
		m_iClip2 = secondary;
		pPlayer->RemoveAmmo( secondary, m_iSecondaryAmmoType );
	}

	if ( m_bReloadsSingly )
	{
		m_bInReload = false;
	}
}

#ifdef CLIENT_DLL
bool CBaseDoDCombatWeapon::ShouldPredict( void )
{
    if ( GetOwner() == C_BasePlayer::GetLocalPlayer() )
        return true;

    return BaseClass::ShouldPredict();
}
#endif // CLIENT_DLL


void CBaseDoDCombatWeapon::FireGun( bool automatic )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( !pPlayer )
		return;

	if ( ( pPlayer->m_afButtonLast & IN_ATTACK ) && !automatic )
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
			m_flNextSecondaryAttack = gpGlobals->curtime + 0.15f;
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

        m_flNextEmptySoundTime = gpGlobals->curtime + 0.1f;
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