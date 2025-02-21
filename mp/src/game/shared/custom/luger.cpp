#include "cbase.h"
#ifdef GAME_DLL
#include "hl1_items.h"
#endif
#include "CBaseDoDWeapon_shared.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#define CLuger C_Luger
#endif

class CLuger : public CBaseDoDCombatWeapon
{
	DECLARE_CLASS(CLuger, CBaseDoDCombatWeapon);
	//DECLARE_SERVERCLASS();

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
public:
	CLuger();

    Activity GetDrawActivity( void ) { return m_iClip1 <= 0 ? ACT_VM_DRAW_EMPTY : ACT_VM_DRAW; }
    Activity GetLastRoundActivity( void ) { return ACT_GLOCK_SHOOTEMPTY; }

	void WeaponIdle( void );
};

IMPLEMENT_NETWORKCLASS_ALIASED( Luger, DT_Luger );

BEGIN_NETWORK_TABLE( CLuger, DT_Luger )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CLuger )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_luger, CLuger);

PRECACHE_WEAPON_REGISTER(weapon_luger);

//IMPLEMENT_SERVERCLASS_ST(CLuger, DT_Luger)
//END_SEND_TABLE();

CLuger::CLuger()
{
	m_bFiresUnderwater = false;
	m_bReloadsSingly = false;
}

void CLuger::WeaponIdle(void)
{
	if (!HasWeaponIdleTimeElapsed())
		return;

	if (m_iClip1 <= 0)
		SendWeaponAnim(ACT_VM_IDLE_EMPTY);
	else
		SendWeaponAnim(ACT_VM_IDLE);

	m_flTimeWeaponIdle = gpGlobals->curtime + 60 * 10;
}

#ifdef GAME_DLL
#define AMMO_MODEL "models/w_chainammo.mdl"
class CMP40Mag : public CHL1Item
{
public:
    DECLARE_CLASS( CMP40Mag, CHL1Item );

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

        if ( pPlayer->GiveAmmo( h.GetInt() > 0 ? h.GetInt() : 128, "9mmRound" ) )
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
LINK_ENTITY_TO_CLASS( ammo_mp40mag, CMP40Mag );
PRECACHE_REGISTER( ammo_mp40mag );
#endif