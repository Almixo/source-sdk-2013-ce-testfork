#include "cbase.h"
#include "hl1mp_basecombatweapon_shared.h"
#include "in_buttons.h"

#ifdef GAME_DLL
#include "soundent.h"
#include "hl1_items.h"
#endif // GAME_DLL

#ifdef CLIENT_DLL
#define CM1Rifle C_M1Rifle
#endif

class CM1Rifle : public CBaseHL1MPCombatWeapon
{
	DECLARE_CLASS( CM1Rifle, CBaseHL1CombatWeapon );
	//DECLARE_SERVERCLASS();

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
public:
	CM1Rifle();

	Activity GetDrawActivity( void );
	void PrimaryAttack( void );
	//void SecondaryAttack( void );
	void WeaponIdle( void );
	bool Reload( void );
};

IMPLEMENT_NETWORKCLASS_ALIASED( M1Rifle, DT_M1Rifle );

BEGIN_NETWORK_TABLE( CM1Rifle, DT_M1Rifle )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CM1Rifle )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_garand, CM1Rifle );

PRECACHE_WEAPON_REGISTER( weapon_garand );

/*IMPLEMENT_SERVERCLASS_ST(CM1Rifle, DT_M1Rifle)
END_SEND_TABLE()*/

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

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.37f;
	m_flTimeWeaponIdle = gpGlobals->curtime + RandomInt(2, 5);

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

#ifdef GAME_DLL
#define AMMO_MODEL "models/w_chainammo.mdl"
class CM1RifleClip : public CHL1Item
{
public:
	DECLARE_CLASS(CM1RifleClip, CHL1Item);

	void Spawn(void)
	{
		Precache();
		SetModel(AMMO_MODEL);
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel(AMMO_MODEL);
	}
	bool MyTouch(CBasePlayer* pPlayer)
	{
		if (pPlayer->GiveAmmo(32, "GarandRound"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_garandclip, CM1RifleClip);
PRECACHE_REGISTER(ammo_garandclip);
#endif