#ifndef CWPNBOX
#define CWPNBOX

#include "cbase.h"
#include "hl1_items.h"
#include "ammodef.h"

#define WEAPONBOX_MODEL "models/w_weaponbox.mdl"
#define MAX_ENTITIES 128

#define WEAPONS_MAX (ARRAYSIZE(pWeapons) - 1)
#define AMMO_MAX (ARRAYSIZE(pAmmo) - 1)
#define KVENTS_MAX (ARRAYSIZE(pKVEnts) -1)
#define KVAMMO_MAX (ARRAYSIZE(pKVAmmo) -1)

class base_t
{
	DECLARE_SIMPLE_DATADESC();
public:
	base_t(const char *name, int amnt = 0)
	{
		szName = name;
		count = amnt;
	}

	const char *szName;
	int count;
};

class CWpnBox : public CHL1Item
{
	DECLARE_CLASS(CWpnBox, CHL1Item);
	DECLARE_DATADESC();
public:
	CWpnBox();
	~CWpnBox();

	void Spawn(void);
	void Precache(void);
	void Touch(CBaseEntity *pOther);
	bool KeyValue(const char *szName, const char *szValue);

	void AddWeapon(CBaseCombatWeapon *pWep = nullptr);
	void AddAmmo(const char *szName, const int count = 0);

	void GiveWeapon();
	void GiveAmmo();

	void AddKVEntity(base_t pBase);
	void AddKVAmmo(base_t pBase);

	void GiveKVEntity();
	void GiveKVAmmo();
private:

	CBaseCombatWeapon *pWeapons[MAX_WEAPONS];
	base_t *pAmmo[MAX_AMMO_TYPES];
	base_t *pKVEnts[MAX_ENTITIES];
	base_t *pKVAmmo[MAX_AMMO_TYPES];

	uint8 iWeapons;
	uint8 iAmmo;
	uint8 iKVEnts;
	uint8 iKVAmmo;

	uint8 iAmmoTypes;

	CBasePlayer *pGiveTo;
};

#endif //CWPNBOX