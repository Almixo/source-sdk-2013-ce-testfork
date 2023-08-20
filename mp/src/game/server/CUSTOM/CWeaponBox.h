#ifndef CWEAPONBOX
#define CWEAPONBOX

#include "cbase.h"
#include "hl1_items.h"
#include "ammodef.h"
#include "saverestore_utlvector.h"

#define WEAPONBOX_MODEL "models/w_weaponbox.mdl"

class base
{
public:
	DECLARE_SIMPLE_DATADESC();
	char *szName;
	int count;
};

class CWpnBox : public CHL1Item
{
	DECLARE_DATADESC();
	DECLARE_CLASS(CWpnBox, CHL1Item);
public:
	CWpnBox();
	~CWpnBox();

	void Spawn(void);
	void Precache(void);
	bool KeyValue(const char *szKeyName, const char *szValue);
	void Touch(CBaseEntity *pOther);

	void AddWeapon(CBaseCombatWeapon *pWpn, int pos);
	void AddAmmo(const char *szName, int count);

	void GiveWeapon(CBasePlayer *pPlayer);
	void GiveAmmo(CBasePlayer *pPlayer);

	//key value port
	void GiveKVEntity(CBasePlayer *pPlayer);
	void GiveKVAmmo(CBasePlayer *pPlayer); //TODO: use GiveAmmo() instead
private:
	bool bGiveAmmo = false, bGiveWeapon = false, bGiveKVAmmo = false, bGiveKVEntity = false;

	CBaseCombatWeapon *pWeapon[MAX_WEAPONS];
	CUtlVector<base>pAmmo;

	CUtlVector<base>pKVEntity;
	CUtlVector<base>pKVAmmo;
};
#endif // !CWEAPONBOX
