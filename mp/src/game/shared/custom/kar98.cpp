#include "cbase.h"
#include "CBaseDoDWeapon_shared.h"

#ifndef CLIENT_DLL
#include "hl1_items.h"
#endif

#ifdef CLIENT_DLL
#define CKar98 C_Kar98
#endif

class CKar98 : public CBaseDoDCombatWeapon
{
	DECLARE_CLASS(CKar98, CBaseDoDCombatWeapon);
	/*DECLARE_SERVERCLASS();*/

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
public:

	CKar98();

    Activity GetLastRoundActivity( void ) { return ACT_GLOCK_SHOOTEMPTY; }
    Activity GetReloadActivity( void ) { return ACT_VM_RELOAD; }
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

    m_iWeaponType = KAR98;
}

#ifdef GAME_DLL
#define AMMO_MODEL "models/w_chainammo.mdl"
class CKar98Clip : public CHL1Item
{
public:
    DECLARE_CLASS( CKar98Clip, CHL1Item );

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

        if ( pPlayer->GiveAmmo( h.GetInt() > 0 ? h.GetInt() : 128, "Kar98Round" ) )
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
LINK_ENTITY_TO_CLASS( ammo_kar98clip, CKar98Clip );
PRECACHE_REGISTER( ammo_kar98clip );
#endif