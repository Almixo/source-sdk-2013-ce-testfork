#include "cbase.h"
#include "CBaseDoDWeapon_shared.h"
#include "in_buttons.h"

#ifdef GAME_DLL
#include "soundent.h"
#include "hl1_items.h"
#endif // GAME_DLL

#ifdef CLIENT_DLL
#define CM1Carbine C_M1Carbine
#endif

class CM1Carbine : public CBaseDoDCombatWeapon
{
	DECLARE_CLASS( CM1Carbine, CBaseDoDCombatWeapon );

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
public:
	CM1Carbine();
};

IMPLEMENT_NETWORKCLASS_ALIASED( M1Carbine, DT_M1Carbine );

BEGIN_NETWORK_TABLE( CM1Carbine, DT_M1Carbine )
END_NETWORK_TABLE();

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CM1Carbine )
END_PREDICTION_DATA();
#endif

LINK_ENTITY_TO_CLASS( weapon_m1carbine, CM1Carbine );

PRECACHE_WEAPON_REGISTER( weapon_m1carbine );

CM1Carbine::CM1Carbine()
{
	m_bReloadsSingly = false;
	m_bFiresUnderwater = false;

	m_iWeaponType = M1CARBINE;
}

#ifdef GAME_DLL
#define AMMO_MODEL "models/w_chainammo.mdl"
class CM1CarbineMag : public CHL1Item
{
public:
    DECLARE_CLASS( CM1CarbineMag, CHL1Item );

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

        if ( pPlayer->GiveAmmo( h.GetInt() > 0 ? h.GetInt() : 128, ".30Carbine" ) )
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
LINK_ENTITY_TO_CLASS( ammo_m1carbinemag, CM1CarbineMag );
PRECACHE_REGISTER( ammo_m1carbinemag );
#endif