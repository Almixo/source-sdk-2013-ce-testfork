#include "cbase.h"
#include "hl1_items.h"
#include "ammodef.h"

#define WEAPONBOX_MODEL "models/w_weaponbox.mdl"

class CWpnBox : public CHL1Item
{
    DECLARE_CLASS( CWpnBox, CHL1Item );
    DECLARE_DATADESC();
public:
    CWpnBox();
    ~CWpnBox();

    void Spawn();
    void Precache();

    bool KeyValue( const char *szKeyName, const char *szKeyValue );

    void BoxTouch( CBaseEntity *pOther );

    // Add-s
    void AddWeapon( CBaseCombatWeapon *pWeapon );
    void AddAmmo( int index, const char *szName, int count );

    //Give-s
    void GiveWeapon();
    void GiveAmmo();

private:
    CBaseCombatWeapon *pWeapon[MAX_WEAPONS];
    int iWpnIndex = 0;

    string_t szAmmo[MAX_AMMO_TYPES];
    int iAmmoCount[MAX_AMMO_TYPES];

    CBasePlayer *pPlayer;
};