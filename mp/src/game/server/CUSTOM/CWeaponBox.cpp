#include "cbase.h"
#include "hl1_items.h"
#include "ammodef.h"
#include "hl1mp_gamerules.h"
#include "hl1mp_player.h"

#define WEAPONBOX_MODEL "models/w_weaponbox.mdl"

class CWpnBox : public CHL1Item
{
	DECLARE_CLASS(CWpnBox, CHL1Item);
	DECLARE_DATADESC();
public:
	void Spawn(void);
	void Precache(void);
	bool KeyValue(const char *szKeyName, const char *szValue);
	void Touch(CBaseEntity *pOther);

	void GiveAmmo(CBaseEntity *pOther);
	void GiveWpn(CBaseEntity *pOther);

	~CWpnBox();
private:
	CUtlVectorFixed<char*, MAX_WEAPONS>szWpns;
	CUtlVectorFixed<char*, MAX_AMMO_TYPES>szAmmo;
	CUtlVectorFixed<int, MAX_AMMO_TYPES>iAmmoCount;

	bool bWpn = false;
	bool bAmmo = false;
};
LINK_ENTITY_TO_CLASS(w_weaponbox, CWpnBox);
PRECACHE_REGISTER(w_weaponbox);

BEGIN_DATADESC(CWpnBox)
	DEFINE_ARRAY(szWpns, FIELD_STRING, MAX_WEAPONS),
	DEFINE_ARRAY(szAmmo, FIELD_STRING, MAX_AMMO_TYPES),
	DEFINE_ARRAY(iAmmoCount, FIELD_INTEGER, MAX_AMMO_TYPES),
	DEFINE_FIELD(bWpn, FIELD_BOOLEAN),
	DEFINE_FIELD(bAmmo, FIELD_BOOLEAN),
END_DATADESC();

CWpnBox::~CWpnBox()
{
	szWpns.Purge();
	szAmmo.Purge();
	iAmmoCount.Purge();
}
void CWpnBox::Spawn()
{
	BaseClass::Spawn();
	Precache();

	Vector vecOrg = GetAbsOrigin();

	DevWarning("%s spawned at %f %f %f!\n", GetDebugName(), vecOrg.x, vecOrg.y, vecOrg.z);

	SetModel(WEAPONBOX_MODEL);
}
void CWpnBox::Precache(void)
{
	BaseClass::Precache();
	PrecacheModel(WEAPONBOX_MODEL);
}
bool CWpnBox::KeyValue(const char *szKeyName, const char *szValue)
{
	const char *p = strstr(szKeyName, "weapon_");
	if (p != NULL) //it's a weapon!
	{
		bWpn = true;
		DevWarning("Packed in weapon by the name of : %s.\n", szKeyName);
		szWpns.AddToHead((char*)szKeyName);

		return true;
	}
	if (GetAmmoDef()->Index(szKeyName) != NULL)
	{
		bAmmo = true;
		DevWarning("Packed in ammo by the name of : %s.\n", szKeyName);
		szAmmo.AddToTail((char*)szKeyName);

		if (atoi(szValue) > 0)
			iAmmoCount.AddToTail(atoi(szValue));

		return true;
	}

	BaseClass::KeyValue(szKeyName, szValue);
	return false;
}
void CWpnBox::Touch(CBaseEntity *pOther)
{
	if (!(GetFlags() & FL_ONGROUND))
		return;

	if (!pOther->IsPlayer())
		return;

	if (!pOther->IsAlive())
		return;

	if (bAmmo == true)
		GiveAmmo(pOther);
	if (bWpn == true)
		GiveWpn(pOther);

	UTIL_Remove(this); //delete itself after pickup!
}
void CWpnBox::GiveAmmo(CBaseEntity *pOther)
{
	CBasePlayer *pPlayer = dynamic_cast<CBasePlayer*>(pOther);

	for (int i = 0; i < MAX_AMMO_TYPES; i++)
	{
		if (szAmmo[i] == NULL || iAmmoCount[i] == NULL)
			continue;

		if (FStrEq(szAmmo[i], "TripMine"))
			pPlayer->GiveAmmo(iAmmoCount[i] - 1, szAmmo[i]);
		else if (FStrEq(szAmmo[i], "Satchel"))
			pPlayer->GiveAmmo(iAmmoCount[i] - 1, szAmmo[i]);
		else if (FStrEq(szAmmo[i], "Grenade"))
			pPlayer->GiveAmmo(iAmmoCount[i] - 1, szAmmo[i]);
		else
			pPlayer->GiveAmmo(iAmmoCount[i], szAmmo[i]);

		DevWarning("Giving %i of %s ammo!\n", iAmmoCount[i], szAmmo[i]);
	}
}
void CWpnBox::GiveWpn(CBaseEntity *pOther)
{
	CBasePlayer *pPlayer = dynamic_cast<CBasePlayer*>(pOther);

	for (auto& wpn : szWpns)
	{
		DevWarning("Giving %s weapon!\n", wpn);
		pPlayer->GiveNamedItem(wpn);
	}
}