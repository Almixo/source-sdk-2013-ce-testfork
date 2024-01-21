#include "cbase.h"
#include "hl1_items.h"
#include "ammodef.h"

#define WEAPONBOX_MODEL "models/w_weaponbox.mdl"
#define MAX_ENTS 32

class base_t
{
	DECLARE_SIMPLE_DATADESC();

public:
	base_t()
	{
		memset( szName, 0, sizeof szName );
		count = 0;
	}
	base_t( const char *name, int cnt )
	{
		memcpy( szName, name, sizeof szName );
		szName[63] = '\0';
		count = cnt;
	}

	const char *GetStr() { return szName; }
	int GetVal() { return count; }

	void SetStr( const char *str ) { strcpy( szName, str ); }
	void SetVal( int val ) { count = val; }

	bool StrEmpty() { return this->szName[0] == '\0'; }
private:
	char szName[64];
	int count;
};


class CWpnBox : public CHL1Item
{
	DECLARE_CLASS( CWpnBox, CHL1Item );
	DECLARE_DATADESC();
public:
	CWpnBox();

	void Spawn();
	void Precache();

	bool KeyValue( const char *szKeyName, const char *szKeyValue );

	void BoxTouch( CBaseEntity *pOther );

	// Add-s
	void AddWeapon( CBaseCombatWeapon *pWeapon );
	void AddAmmo( base_t base );
	void AddKVAmmo( base_t base, int index );
	void AddKVEnt( base_t base );

	//Give-s
	void GiveWeapon();
	void GiveAmmo();
	void GiveKVEnt();

private:
	CBaseCombatWeapon *pWeapon[MAX_WEAPONS];
	
	base_t pAmmo[MAX_AMMO_TYPES];	
	base_t pKVEnt[MAX_ENTS];

	int iWeaponIndex;
	int iAmmoIndex;
	int iKVEntIndex;

	CBasePlayer *pPlayer;
};
