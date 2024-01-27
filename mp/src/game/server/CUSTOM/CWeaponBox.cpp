#include "cbase.h"
#include "CWeaponBox.h"

LINK_ENTITY_TO_CLASS(w_weaponbox, CWpnBox);

BEGIN_SIMPLE_DATADESC(base_t)
DEFINE_FIELD(szName, FIELD_STRING),
DEFINE_FIELD(count, FIELD_INTEGER),
END_DATADESC();

BEGIN_DATADESC(CWpnBox)
	DEFINE_ARRAY(pKVEnts, FIELD_EMBEDDED, MAX_ENTITIES),
	DEFINE_ARRAY(pKVAmmo, FIELD_EMBEDDED, MAX_AMMO_TYPES),
END_DATADESC();

CWpnBox::CWpnBox()
{
	memset(pWeapons, 0, sizeof pWeapons);
	memset(pAmmo, 0, sizeof pAmmo);
	memset(pKVEnts, 0, sizeof pKVEnts);
	memset(pKVAmmo, 0, sizeof pKVAmmo);

	iWeapons = 0;
	iAmmo = 0;
	iKVEnts = 0;
	iKVAmmo = 0;

	iAmmoTypes = 0;

	pGiveTo = nullptr;
}

CWpnBox::~CWpnBox()
{
	memset(pWeapons, 0, sizeof pWeapons);
	memset(pAmmo, 0, sizeof pAmmo);
	memset(pKVEnts, 0, sizeof pKVEnts);
	memset(pKVAmmo, 0, sizeof pKVAmmo);

	iWeapons = 0;
	iAmmo = 0;
	iKVEnts = 0;
	iKVAmmo = 0;

	iAmmoTypes = 0;

	pGiveTo = nullptr;
}

void CWpnBox::Spawn(void)
{
	BaseClass::Spawn();
	Precache();
	SetModel(WEAPONBOX_MODEL);
}

void CWpnBox::Precache(void)
{
	PrecacheModel(WEAPONBOX_MODEL);
	PrecacheScriptSound("Item.Pickup");
}

bool CWpnBox::KeyValue(const char *szKeyName, const char *szKeyValue)
{
	if (GetAmmoDef()->Index(szKeyName) > -1 && iAmmoTypes < MAX_AMMO_TYPES)
	{
		int count = atoi(szKeyValue) == 0 ? 1 : atoi(szKeyValue);
		base_t base(szKeyName, count);
		
		AddKVAmmo(base);

		return true;
	}
	else if (!Q_strnicmp(szKeyName, "weapon_", 7) || !Q_strnicmp(szKeyName, "ammo_", 6))
	{
		int count = atoi(szKeyValue) == 0 ? 1 : atoi(szKeyValue);
		base_t base(szKeyName, count);

		AddKVEntity(base);
		
		return true;
	}
	else
		return BaseClass::KeyValue(szKeyName, szKeyValue);
}

void CWpnBox::Touch(CBaseEntity *pOther)
{
	if (!(GetFlags() & FL_ONGROUND))
		return;

	if (!pOther->IsPlayer())
		return;

	if (!pOther->IsAlive())
		return;

	CBasePlayer *pPlayer = ToBasePlayer(pOther);
	if (!pPlayer)
		return;

	pGiveTo = pPlayer;

	if (pWeapons[0] != nullptr)
		GiveWeapon();
	if (pAmmo[0] != nullptr)
		GiveAmmo();
	if (pKVEnts[0] != nullptr)
		GiveKVEntity();
	if (pKVAmmo[0] != nullptr)
		GiveKVAmmo();

	CPASAttenuationFilter filter(this);
	EmitSound(filter, pOther->entindex(), "Item.Pickup");

	SetTouch(NULL);
	UTIL_Remove(this);
}

void CWpnBox::AddWeapon(CBaseCombatWeapon *pWpn)
{
	if (iWeapons > WEAPONS_MAX)
		return;

	pWeapons[iWeapons] = pWpn;
	iWeapons++;
}

void CWpnBox::AddAmmo(const char *szName, int count)
{
	if (iAmmo > AMMO_MAX)
		return;

	base_t base(szName, count);
	pAmmo[iAmmo] = &base;
	iAmmo++;
}

void CWpnBox::GiveWeapon(void)
{
	for (int i = 0; i < iWeapons && pWeapons[i] != nullptr; i++)
		pWeapons[i]->GiveTo(pGiveTo);
}

void CWpnBox::GiveAmmo(void)
{
	for (int i = 0; i < iAmmo && pAmmo[i] != nullptr; i++)
		pGiveTo->GiveAmmo(pAmmo[i]->count, pAmmo[i]->szName, true);
}

// ========== key value implementation ==========

void CWpnBox::AddKVEntity(base_t base)
{
	if (iKVEnts > KVENTS_MAX)
		return;

	pKVEnts[iKVEnts] = &base;
	iKVEnts++;
}

void CWpnBox::AddKVAmmo(base_t base)
{
	if (iAmmoTypes > KVAMMO_MAX)
		return;

	pKVAmmo[iAmmoTypes] = &base;
	iAmmoTypes++;
}

void CWpnBox::GiveKVEntity(void)
{
	for (int i = 0; i < iKVEnts && pKVEnts[i] != nullptr; i++)
		for (int a = 0; i < pKVEnts[i]->count; a++)
			pGiveTo->GiveNamedItem(pKVEnts[i]->szName);
}

// ========== key value implementation ==========
void CWpnBox::GiveKVAmmo(void)
{
	for (int i = 0; i < iKVAmmo && pKVAmmo[i] != nullptr; i++)
		pGiveTo->GiveAmmo(pKVAmmo[i]->count, pKVAmmo[i]->szName, true);
}
