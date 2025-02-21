#include "cbase.h"
#include "CBaseDoDWeapon_shared.h"

#ifndef CLIENT_DLL
#include "hl1_items.h"
#endif

#ifdef CLIENT_DLL
#define CTommy C_Tommy
#endif

class CTommy : public CBaseDoDCombatWeapon
{
	DECLARE_CLASS( CTommy, CBaseDoDCombatWeapon );

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
public:

	CTommy();

	void GetFire() { return FireAuto(); }

    Activity GetLastRoundActivity( void ) { return ACT_GLOCK_SHOOTEMPTY; }
    Activity GetReloadActivity( void ) { return ACT_VM_RELOAD; }
};

IMPLEMENT_NETWORKCLASS_ALIASED( Tommy, DT_Tommy );

BEGIN_NETWORK_TABLE( CTommy, DT_Tommy )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTommy )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_tommy, CTommy );

PRECACHE_WEAPON_REGISTER( weapon_tommy );

CTommy::CTommy()
{
	m_bReloadsSingly = false;
	m_bFiresUnderwater = false;

	m_iWeaponType = THOMPSON;
}

#ifdef GAME_DLL
#define AMMO_MODEL "models/w_chainammo.mdl"
class CTommyMag : public CHL1Item
{
public:
    DECLARE_CLASS( CTommyMag, CHL1Item );

    void Spawn( void )
    {
        Precache();
        SetModel( AMMO_MODEL );
        BaseClass::Spawn();
    }
    void Precache( void )
    {
        PrecacheModel( AMMO_MODEL );
    }
    bool MyTouch( CBasePlayer* pPlayer )
    {
        ConVarRef h( "sk_max_9mm_bullet" );

        if ( pPlayer->GiveAmmo( h.GetInt() > 0 ? h.GetInt() : 128, ".45ACP" ) )
        {
            if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
            {
                UTIL_Remove( this );
            }
            return true;
        }
        return false;
    }
};
LINK_ENTITY_TO_CLASS( ammo_tommymag, CTommyMag );
PRECACHE_REGISTER( ammo_tommymag );
#endif