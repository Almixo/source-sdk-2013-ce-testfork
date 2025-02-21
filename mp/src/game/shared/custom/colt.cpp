#include "cbase.h"
#ifdef GAME_DLL
#include "hl1_items.h"
#endif // GAME_DLL
#include "CBaseDoDWeapon_shared.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#define CColt C_Colt
#endif

class CColt : public CBaseDoDCombatWeapon
{
	DECLARE_CLASS(CColt, CBaseDoDCombatWeapon);
	/*DECLARE_SERVERCLASS();*/

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
public:
	CColt();

	Activity GetDrawActivity( void ) { return m_iClip1 <= 0 ? ACT_VM_DRAW_EMPTY : ACT_VM_DRAW; }
    Activity GetLastRoundActivity( void ) { return ACT_GLOCK_SHOOTEMPTY; }

	void WeaponIdle(void);
};

IMPLEMENT_NETWORKCLASS_ALIASED( Colt, DT_Colt );

BEGIN_NETWORK_TABLE( CColt, DT_Colt )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CColt )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_colt, CColt);

PRECACHE_WEAPON_REGISTER(weapon_colt);

//IMPLEMENT_SERVERCLASS_ST(CColt, DT_Colt)
//END_SEND_TABLE();

CColt::CColt()
{
	m_bFiresUnderwater = false;
	m_bReloadsSingly = false;

    m_iWeaponType = COLT;
}

void CColt::WeaponIdle(void)
{
	if (!HasWeaponIdleTimeElapsed())
		return;

	if (m_iClip1 <= 0)
		SendWeaponAnim(ACT_VM_IDLE_EMPTY);
	else
		SendWeaponAnim(ACT_VM_IDLE);

	m_flTimeWeaponIdle = gpGlobals->curtime + 6.0;
}

#ifdef GAME_DLL
#define AMMO_MODEL "models/w_chainammo.mdl"
class CColtMag : public CHL1Item
{
public:
    DECLARE_CLASS( CColtMag, CHL1Item );

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
LINK_ENTITY_TO_CLASS( ammo_coltmag, CColtMag );
PRECACHE_REGISTER( ammo_coltmag );
#endif