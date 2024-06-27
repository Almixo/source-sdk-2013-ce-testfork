#include "cbase.h"
#ifdef GAME_DLL
#include "soundent.h"
#endif // GAME_DLL
#include "hl1mp_basecombatweapon_shared.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#define CKar98 C_Kar98
#endif

class CKar98 : public CBaseHL1MPCombatWeapon
{
	DECLARE_CLASS(CKar98, CBaseHL1CombatWeapon);
	/*DECLARE_SERVERCLASS();*/

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
public:

	CKar98();

	void PrimaryAttack( void );
	bool Reload(void);
	Activity GetDrawActivity( void ) { return ACT_VM_DRAW; };
};

IMPLEMENT_NETWORKCLASS_ALIASED( Kar98, DT_Kar98 );

BEGIN_NETWORK_TABLE( CKar98, DT_Kar98 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CKar98 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_kar98, CKar98);

PRECACHE_WEAPON_REGISTER(weapon_kar98);

//IMPLEMENT_SERVERCLASS_ST(CKar98, DT_Kar98)
//END_SEND_TABLE();

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
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.15f;
		}

		return;
	}

	WeaponSound( SINGLE );
	pPlayer->DoMuzzleFlash();

#ifdef GAME_DLL
	pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );
#endif

	m_flNextPrimaryAttack = gpGlobals->curtime + 1.5f;
	m_flTimeWeaponIdle = gpGlobals->curtime + RandomInt( 2, 5 );

	m_iClip1--;

	if ( m_iClip1 > 0 )
		SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	else
		SendWeaponAnim( ACT_GLOCK_SHOOTEMPTY );

	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector vecAiming = pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );
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
		vecSpread = Vector( 0.014f, 0.014f, 0.014f );
	}

	FireBulletsInfo_t info( 1, vecSrc, vecAiming, vecSpread, MAX_TRACE_LENGTH, m_iPrimaryAmmoType ); // 3 tracer count rip
	pPlayer->FireBullets( info );

#ifdef GAME_DLL
	pPlayer->SnapEyeAngles( angles );
	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), 600, 0.2, GetOwner() );
#endif
}

bool CKar98::Reload(void)
{
	if ( m_bInReload )
		return false;

	CBaseCombatCharacter *pOwner = GetOwner();
	if (!pOwner)
		return false;

	// If I don't have any spare ammo, I can't reload
	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return false;

	if ( m_iClip1 == GetMaxClip1() )
		return false;

	m_iClip1 = 0;

	return BaseClass::DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );
}