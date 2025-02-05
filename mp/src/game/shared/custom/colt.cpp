#include "cbase.h"
#ifdef GAME_DLL
#include "soundent.h"
#endif // GAME_DLL
#include "hl1mp_basecombatweapon_shared.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#define CColt C_Colt
#endif

class CColt : public CBaseHL1MPCombatWeapon
{
	DECLARE_CLASS(CColt, CBaseHL1CombatWeapon);
	/*DECLARE_SERVERCLASS();*/

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
public:
	CColt();

	Activity GetDrawActivity( void );
	Activity GetPrimaryAttackActivity( void );

	void PrimaryAttack( void );
	bool Reload( void );
	void WeaponIdle(void);
};

IMPLEMENT_NETWORKCLASS_ALIASED( Colt, DT_Colt );

BEGIN_NETWORK_TABLE( CColt, DT_Colt )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CColt )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_colt, CColt);

PRECACHE_WEAPON_REGISTER(weapon_colt);

//IMPLEMENT_SERVERCLASS_ST(CColt, DT_Colt)
//END_SEND_TABLE();

CColt::CColt()
{
	m_bFiresUnderwater = false;
	m_bReloadsSingly = false;
}

Activity CColt::GetDrawActivity( void )
{
	if ( m_iClip1 <= 0 )
		return ACT_VM_DRAW_EMPTY;
	else
		return ACT_VM_DRAW;
}

Activity CColt::GetPrimaryAttackActivity( void )
{
	if ( m_iClip1 <= 0 )
		return ACT_GLOCK_SHOOTEMPTY;
	else
		return ACT_VM_PRIMARYATTACK;
}

void CColt::PrimaryAttack( void )
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
	m_flTimeWeaponIdle = gpGlobals->curtime + 0.75;

	m_iClip1--;

	SendWeaponAnim( GetPrimaryAttackActivity() );

	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector vecAiming = pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );
	static const Vector vecSpread = Vector( 0.055f, 0.055f, 0.055f );

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

bool CColt::Reload( void )
{
	int activity;

	if ( m_iClip1 == 0 )
		activity = ACT_VM_RELOAD_EMPTY;
	else
		activity = ACT_VM_RELOAD;

	return DefaultReload( GetMaxClip1(), GetMaxClip2(), activity );

}

void CColt::WeaponIdle(void)
{
	if (!HasWeaponIdleTimeElapsed())
		return;

	if (m_iClip1 <= 0)
		SendWeaponAnim(ACT_VM_IDLE_EMPTY);
	else
		SendWeaponAnim(ACT_VM_IDLE);

	m_flTimeWeaponIdle = gpGlobals->curtime + 6.0;
}