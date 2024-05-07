#include "cbase.h"
#include "soundent.h"
#include "hl1_basecombatweapon_shared.h"

class CKar98 : public CBaseHL1CombatWeapon
{
	DECLARE_CLASS(CKar98, CBaseHL1CombatWeapon);
	DECLARE_SERVERCLASS();
public:

	CKar98();

	void PrimaryAttack( void );
	bool Reload(void);
	Activity GetDrawActivity(void);
};

LINK_ENTITY_TO_CLASS(weapon_kar98, CKar98);

PRECACHE_WEAPON_REGISTER(weapon_kar98);

IMPLEMENT_SERVERCLASS_ST(CKar98, DT_Kar98)
END_SEND_TABLE();

CKar98::CKar98()
{
	m_bFiresUnderwater = false;
	m_bReloadsSingly = false;
}

void CKar98::PrimaryAttack( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
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
			m_flNextPrimaryAttack = 0.15;
		}

		return;
	}

	WeaponSound( SINGLE );
	pPlayer->DoMuzzleFlash();

	m_flNextPrimaryAttack = gpGlobals->curtime + 1.5f;
	m_flTimeWeaponIdle = gpGlobals->curtime + RandomInt( 2, 5 );

	m_iClip1--;

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector vecAiming = pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );

	pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );

	Vector vecSpread;

	//Disorient the player
	QAngle angles = pPlayer->EyeAngles();
	angles.x -= 2 + RandomFloat( -0.5f, 0.5f );
	vecSpread = Vector( 0.01f, 0.01f, 0.01f );

	pPlayer->SnapEyeAngles( angles );

	pPlayer->FireBullets( 1, vecSrc, vecAiming, vecSpread, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 3 );

	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), 600, 0.2, GetOwner() );
}

bool CKar98::Reload(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if (!pOwner)
		return false;

	// If I don't have any spare ammo, I can't reload
	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return false;

	m_iClip1 = 0;

	return BaseClass::Reload();
}

Activity CKar98::GetDrawActivity(void)
{
	if (m_iClip1 == 0)
		return ACT_VM_DRAW_EMPTY;
	else
		return ACT_VM_DRAW;
}