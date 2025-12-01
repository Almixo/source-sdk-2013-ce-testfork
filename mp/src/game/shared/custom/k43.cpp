#include "cbase.h"
#include "CBaseDoDWeapon_shared.h"

#ifndef CLIENT_DLL
#include "hl1_items.h"
#endif // !CLIENT_DLL

#ifdef CLIENT_DLL
#define CK43 C_K43
#endif // CLIENT_DLL

class CK43 : public CBaseDoDCombatWeapon
{
    DECLARE_CLASS( CK43, CBaseDoDCombatWeapon );

    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();
public:

    CK43();

    void GetFire( void ) { return FireSingle(); }

    Activity GetIdleEmptyActivity( void ) { return ACT_VM_IDLE; }
    Activity GetReloadActivity( void ) { return ACT_VM_RELOAD; }
};

IMPLEMENT_NETWORKCLASS_ALIASED( K43, DT_K43 );

BEGIN_NETWORK_TABLE( CK43, DT_K43 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CK43 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_k43, CK43 );

PRECACHE_WEAPON_REGISTER( weapon_k43 );

CK43::CK43()
{
    m_bReloadsSingly = false;
    m_bFiresUnderwater = false;

    m_iWeaponType = K_43;
}

#ifdef GAME_DLL
#define AMMO_MODEL "models/w_chainammo.mdl"
class CK43Mag : public CHL1Item
{
public:
    DECLARE_CLASS( CK43Mag, CHL1Item );

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

        if ( pPlayer->GiveAmmo( h.GetInt() > 0 ? h.GetInt() : 128, "K43Round" ) )
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
LINK_ENTITY_TO_CLASS( ammo_k43mag, CK43Mag );
PRECACHE_REGISTER( ammo_k43mag );
#endif