#include "cbase.h"
#include "soundent.h"
#include "hl1_basecombatweapon_shared.h"
#include "in_buttons.h"

class CLuger : public CBaseHL1CombatWeapon
{
	DECLARE_CLASS(CLuger, CBaseHL1CombatWeapon);
	DECLARE_SERVERCLASS();
public:
	CLuger();

	Activity GetDrawActivity( void );
	Activity GetPrimaryAttackActivity( void );

	void PrimaryAttack( void );
	bool Reload( void );
	void WeaponIdle( void );
};

LINK_ENTITY_TO_CLASS(weapon_luger, CLuger);

PRECACHE_WEAPON_REGISTER(weapon_luger);

IMPLEMENT_SERVERCLASS_ST(CLuger, DT_Luger)
END_SEND_TABLE();

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

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.175f;
	m_flTimeWeaponIdle = gpGlobals->curtime + 60 * 10;

	m_iClip1--;

	SendWeaponAnim( GetPrimaryAttackActivity() );

	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector vecAiming = pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );

	pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );

	Vector vecSpread;

	//Disorient the player
	QAngle angles = pPlayer->EyeAngles();
	angles.x -= 2 + RandomFloat( -0.5f, 0.5f );
	vecSpread = Vector( 0.055f, 0.055f, 0.055f );

	pPlayer->SnapEyeAngles( angles );

	pPlayer->FireBullets( 1, vecSrc, vecAiming, vecSpread, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 3 );

	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), 600, 0.2, GetOwner() );
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