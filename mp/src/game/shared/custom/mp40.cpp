#include "cbase.h"
#include "CBaseDoDWeapon_shared.h"

#ifndef CLIENT_DLL
#include "hl1_items.h"
#endif // !CLIENT_DLL

#ifdef CLIENT_DLL
#define CMP40 C_MP40
#endif // CLIENT_DLL

class CMP40 : public CBaseDoDCombatWeapon
{
    DECLARE_CLASS( CMP40, CBaseDoDCombatWeapon );

    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();
public:

    CMP40();

    void GetFire() { return FireAuto(); }

    Activity GetIdleEmptyActivity( void ) { return ACT_VM_IDLE; }
    Activity GetReloadActivity( void ) { return ACT_VM_RELOAD; }
};

IMPLEMENT_NETWORKCLASS_ALIASED( MP40, DT_MP40 );

BEGIN_NETWORK_TABLE( CMP40, DT_MP40 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CMP40 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_mp40, CMP40 );

PRECACHE_WEAPON_REGISTER( weapon_mp40 );

CMP40::CMP40()
{
    m_bReloadsSingly = false;
    m_bFiresUnderwater = false;

    m_iWeaponType = MP_40;
}
//
//#ifdef GAME_DLL
//#define AMMO_MODEL "models/w_chainammo.mdl"
//class CMP40Mag : public CHL1Item
//{
//public:
//    DECLARE_CLASS( CMP40Mag, CHL1Item );
//
//    void Spawn( void )
//    {
//        Precache();
//        SetModel( AMMO_MODEL );
//        BaseClass::Spawn();
//    }
//    void Precache( void )
//    {
//        PrecacheModel( AMMO_MODEL );
//    }
//    bool MyTouch( CBasePlayer* pPlayer )
//    {
//        ConVarRef h( "sk_max_9mm_bullet" );
//
//        if ( pPlayer->GiveAmmo( h.GetInt() > 0 ? h.GetInt() : 128, "9mmRound" ) )
//        {
//            if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
//            {
//                UTIL_Remove( this );
//            }
//            return true;
//        }
//        return false;
//    }
//};
//LINK_ENTITY_TO_CLASS( ammo_mp40mag, CMP40Mag );
//PRECACHE_REGISTER( ammo_mp40mag );
//#endif