#include "cbase.h"
#include "soundent.h"
#include "hl1_basecombatweapon_shared.h"

class CM1Rifle : public CBaseHL1CombatWeapon
{
	DECLARE_CLASS( CM1Rifle, CBaseHL1CombatWeapon );
	DECLARE_SERVERCLASS();
public:
	CM1Rifle();

	Activity GetDrawActivity( void );
	void PrimaryAttack( void );
	//void SecondaryAttack( void );
	void WeaponIdle( void );
	bool Reload( void );
};

LINK_ENTITY_TO_CLASS( weapon_garand, CM1Rifle );

PRECACHE_WEAPON_REGISTER( weapon_garand );

IMPLEMENT_SERVERCLASS_ST( CM1Rifle, DT_M1Rifle )
END_SEND_TABLE()

CM1Rifle::CM1Rifle()
{
	m_bReloadsSingly = false;
	m_bFiresUnderwater = false;
}

Activity CM1Rifle::GetDrawActivity( void )
{
	return m_iClip1 <= 0 ? ACT_VM_DRAW_EMPTY : ACT_VM_DRAW;
}

bool CM1Rifle::Reload( void )
{
	if ( m_iClip1 > 0 )
		return false;
	else
		return BaseClass::Reload();
}

void CM1Rifle::PrimaryAttack( void )
{
	/*BaseClass::PrimaryAttack();

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.15f;

	if ( m_iClip1 == 0 )
	{
		WeaponSound( SPECIAL1 );
	}*/

	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( !pPlayer )
	{
		return;
	}

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

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.35f;
	m_flTimeWeaponIdle = gpGlobals->curtime + RandomInt(2, 5);

	m_iClip1--;

	if ( m_iClip1 > 0 )
		SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	else
		SendWeaponAnim( ACT_GLOCK_SHOOTEMPTY );

	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector vecAiming = pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );

	pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );

	Vector vecSpread;

	//Disorient the player
	QAngle angles = pPlayer->EyeAngles();
	
	if ( pPlayer->m_Local.m_bDucked )
	{ 
		angles.x -= 5 + RandomFloat( -1, 1 );
		vecSpread = vec3_origin;
	}
	else
	{
		angles.x -= 10 + RandomFloat( -5.0f, 2.5f );
		vecSpread = Vector( 0.01f, 0.01f, 0.01f );
	}

	pPlayer->SnapEyeAngles( angles );

	pPlayer->FireBullets( 1, vecSrc, vecAiming, vecSpread, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 3 );

	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), 600, 0.2, GetOwner() );

	if ( m_iClip1 <= 0 )
		WeaponSound( SPECIAL1 );
}

void CM1Rifle::WeaponIdle( void )
{
	if ( !HasWeaponIdleTimeElapsed() )
		return;

	if ( m_iClip1 <= 0 )
		SendWeaponAnim( ACT_VM_IDLE_EMPTY );
	else
		SendWeaponAnim( ACT_VM_IDLE );

	m_flTimeWeaponIdle = gpGlobals->curtime + RandomInt( 5, 10 );
}