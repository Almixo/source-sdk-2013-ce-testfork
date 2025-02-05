#include "cbase.h"
#include "CBaseDoDWeapon_shared.h"

#ifndef CLIENT_DLL
#include "hl1_items.h"
#endif // !CLIENT_DLL


#ifdef CLIENT_DLL
#define CM1Rifle C_M1Rifle
#endif // CLIENT_DLL

class CM1Rifle : public CBaseDoDCombatWeapon
{
	DECLARE_CLASS( CM1Rifle, CBaseDoDCombatWeapon );

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
public:
	CM1Rifle();

	Activity GetDrawActivity( void );
	Activity GetLastRoundActivity( void ) { return ACT_GLOCK_SHOOTEMPTY; }

	void WeaponIdle( void );
	bool Reload( void );
};

IMPLEMENT_NETWORKCLASS_ALIASED( M1Rifle, DT_M1Rifle );

BEGIN_NETWORK_TABLE( CM1Rifle, DT_M1Rifle )
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CM1Rifle )
//DEFINE_PRED_FIELD_TOL( m_flNextPrimaryAttack, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_garand, CM1Rifle );

PRECACHE_WEAPON_REGISTER( weapon_garand );


CM1Rifle::CM1Rifle()
{
	m_bReloadsSingly = false;
	m_bFiresUnderwater = false;

	m_iWeaponType = M1RIFLE;
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
        ConVarRef h( "sk_max_9mm_bullet" );

		if (pPlayer->GiveAmmo(h.GetInt() > 0 ? h.GetInt() : 128, "GarandRound") )
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