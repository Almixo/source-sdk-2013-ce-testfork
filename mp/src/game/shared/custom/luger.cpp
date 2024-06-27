#include "cbase.h"
#ifdef GAME_DLL
#include "soundent.h"
#endif
#include "hl1mp_basecombatweapon_shared.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#define CLuger C_Luger
#endif

class CLuger : public CBaseHL1MPCombatWeapon
{
	DECLARE_CLASS(CLuger, CBaseHL1CombatWeapon);
	//DECLARE_SERVERCLASS();

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
public:
	CLuger();

	Activity GetDrawActivity( void );
	Activity GetPrimaryAttackActivity( void );

	void PrimaryAttack( void );
	bool Reload( void );
	void WeaponIdle( void );
};

IMPLEMENT_NETWORKCLASS_ALIASED( Luger, DT_Luger );

BEGIN_NETWORK_TABLE( CLuger, DT_Luger )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CLuger )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_luger, CLuger);

PRECACHE_WEAPON_REGISTER(weapon_luger);

//IMPLEMENT_SERVERCLASS_ST(CLuger, DT_Luger)
//END_SEND_TABLE();

CLuger::CLuger()
{
	m_bFiresUnderwater = false;
	m_bReloadsSingly = false;
}

Activity CLuger::GetDrawActivity( void )
{
	if ( m_iClip1 <= 0 )
		return ACT_VM_DRAW_EMPTY;
	else
		return ACT_VM_DRAW;
}

Activity CLuger::GetPrimaryAttackActivity( void )
{
	if ( m_iClip1 <= 0 )
		return ACT_GLOCK_SHOOTEMPTY;
	else
		return ACT_VM_PRIMARYATTACK;
}

void CLuger::PrimaryAttack( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( !pPlayer )
		return;

	if ( pPlayer->m_afButtonLast & IN_ATTACK )
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
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.1f;
		}

		return;
	}

	WeaponSound( SINGLE );
	pPlayer->DoMuzzleFlash();

#ifdef GAME_DLL
	pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );
#endif

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.175f;
	m_flTimeWeaponIdle = gpGlobals->curtime + 60 * 10;

	m_iClip1--;

	SendWeaponAnim( GetPrimaryAttackActivity() );

	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector vecAiming = pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );
	Vector vecSpread = Vector( 0.055f, 0.055f, 0.055f );

	FireBulletsInfo_t info( 1, vecSrc, vecAiming, vecSpread, MAX_TRACE_LENGTH, m_iPrimaryAmmoType ); // 3 tracer count rip
	pPlayer->FireBullets( info );

#ifdef GAME_DLL
	//Disorient the player
	QAngle angles = pPlayer->EyeAngles();
	angles.x -= 2 + RandomFloat( -0.5f, 0.5f );

	pPlayer->SnapEyeAngles( angles );
	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), 600, 0.2, GetOwner() );
#endif
}

bool CLuger::Reload( void )
{
	int activity;

	if ( m_iClip1 == 0 )
		activity = ACT_VM_RELOAD_EMPTY;
	else
		activity = ACT_VM_RELOAD;

	return DefaultReload( GetMaxClip1(), GetMaxClip2(), activity );
}

void CLuger::WeaponIdle(void)
{
	if (!HasWeaponIdleTimeElapsed())
		return;

	if (m_iClip1 <= 0)
		SendWeaponAnim(ACT_VM_IDLE_EMPTY);
	else
		SendWeaponAnim(ACT_VM_IDLE);

	m_flTimeWeaponIdle = gpGlobals->curtime + 60 * 10;
}