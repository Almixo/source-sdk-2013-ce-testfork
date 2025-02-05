#include "cbase.h"
#include "CGuided.h"

#ifdef CLIENT_DLL
#include "view.h"
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(Guided, DT_Guided);

BEGIN_NETWORK_TABLE(CGuided, DT_Guided)
#ifndef CLIENT_DLL
SendPropFloat(SENDINFO(m_flDotTurnOn)),
SendPropInt(SENDINFO(m_iDotState)),
#else
RecvPropFloat(RECVINFO(m_flDotTurnOn)),
RecvPropInt(RECVINFO(m_iDotState)),
#endif
END_NETWORK_TABLE();

LINK_ENTITY_TO_CLASS(weapon_guided, CGuided);

PRECACHE_WEAPON_REGISTER(weapon_guided);

BEGIN_PREDICTION_DATA( CGuided )
END_PREDICTION_DATA();

BEGIN_DATADESC(CGuided)
DEFINE_FIELD(m_iDotState, FIELD_INTEGER),
END_DATADESC();

CGuided::CGuided()
{
	m_bFiresUnderwater = false;
	m_bReloadsSingly = false;

	m_pDot = nullptr;
	m_flDotTurnOn = -1.0f;
	m_iDotState = DOT_OFF;
}

CGuided::~CGuided()
{
	if ( m_pDot )
	{
		m_pDot->TurnOff();
		m_pDot->Remove();

		m_pDot = nullptr;
	}
}

bool CGuided::Deploy()
{
	if ( !m_pDot )
		m_pDot = CreateDot();

	m_flDotTurnOn = gpGlobals->curtime + SequenceDuration();

	return BaseClass::Deploy();
}

bool CGuided::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	bool holster = BaseClass::Holster(pSwitchingTo);

	if ( holster && m_iDotState == DOT_ON )
	{
		DotOff();
		m_iDotState = DOT_WAS;
	}
	
	return holster;
}

void CGuided::PrimaryAttack(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if ( !pPlayer )
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
			m_flNextEmptySoundTime = 1.0f;
			m_flNextPrimaryAttack = 0.15f;
		}

		return;
	}

	WeaponSound( SINGLE );
	pPlayer->DoMuzzleFlash();

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	m_iClip1--;

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector vecAiming = pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	Vector vecSpread;

	if ( m_iDotState == DOT_ON )
	{
		FireBulletsInfo_t info( 1, vecSrc, vecAiming, Vector(0.001f, 0.001f, 0.001f), MAX_TRACE_LENGTH, m_iPrimaryAmmoType);
		info.m_pAttacker = pPlayer;
		pPlayer->FireBullets( info );

		m_flNextPrimaryAttack = gpGlobals->curtime + 0.75f;
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.75f;
	}
	else
	{
		FireBulletsInfo_t info( 1, vecSrc, vecAiming, Vector( 0.1f, 0.1f, 0.1f ), MAX_TRACE_LENGTH, m_iPrimaryAmmoType );
		info.m_pAttacker = pPlayer;
		pPlayer->FireBullets( info );

		m_flNextPrimaryAttack = gpGlobals->curtime + 0.25f;
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.25f;
	}

#ifndef CLIENT_DLL
	pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 0.5f );
#endif

	pPlayer->ViewPunch( QAngle( -1.5, 0, 0 ) );

	if ( !m_iClip1 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
	}
}

void CGuided::SecondaryAttack(void)
{
	ToggleDot();
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
}

bool CGuided::Reload( void )
{
	bool result = BaseClass::Reload();
	if ( result )
	{
		if ( m_iDotState == DOT_ON )
		{
			DotOff();
			m_iDotState = DOT_WAS;
		}

		m_flDotTurnOn = gpGlobals->curtime + SequenceDuration();

		int activity;
		if ( m_iClip1 == 0 )
			activity = ACT_VM_RELOAD_EMPTY;
		else
			activity = ACT_VM_RELOAD;

		SendWeaponAnim( activity );
	}

	return result;
}

void CGuided::FinishReloading( void )
{
	BaseClass::FinishReload();

	if ( m_iDotState == DOT_WAS )
	{
		m_iDotState = DOT_ON;
		DotOn();
	}
}

void CGuided::ToggleDot(void)
{
	if ( m_iDotState == DOT_OFF )
	{
		DotOn();
		m_iDotState = DOT_ON;
	}
	else
	{
		DotOff();
		m_iDotState = DOT_OFF;
	}

	Msg("Dot is %s\n", m_iDotState == DOT_ON ? "on" : "off");
}

void CGuided::DotOn()
{
	if ( !m_pDot )
		m_pDot = CreateDot();

	if ( !m_pDot )
	{
		Warning( "Couldn't create the dot sprite!\n" );
		return;
	}

	m_pDot->TurnOn();
	m_pDot->SetRenderColorA( 255 );
}

void CGuided::DotOff()
{
	if ( !m_pDot )
		m_pDot = CreateDot();

	if ( !m_pDot )
	{
		Warning( "Couldn't create the dot sprite!\n" );
		return;
	}

	m_pDot->TurnOff();
	m_pDot->SetRenderColorA( 0 );
}

void CGuided::ItemPostFrame( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( !pPlayer )
		return;

	// dot has to be figured out first!
	if ( m_iDotState == DOT_WAS && m_flDotTurnOn <= gpGlobals->curtime )
	{
		m_iDotState = DOT_ON;
		DotOn();
	}

	BaseClass::ItemPostFrame();

	if ( m_iDotState == DOT_ON )
	{
		if ( !m_pDot )
			m_pDot = CreateDot();

		Vector vecOrg, vecDir, vecEnd;
		trace_t tr;
		
#ifndef CLIENT_DLL
		vecOrg = pPlayer->EyePosition();
		AngleVectors(pPlayer->EyeAngles(), &vecDir);
		vecDir.Normalized();
#else
		vecOrg = CurrentViewOrigin();
		vecDir = CurrentViewForward();	
#endif
		UTIL_TraceLine( vecOrg, vecOrg + ( vecDir * MAX_TRACE_LENGTH ), MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr );
		if ( !tr.DidHit() )
			return;

		vecEnd = tr.endpos - vecDir * 4;

		m_pDot->SetAbsOrigin(vecEnd);
	}
}