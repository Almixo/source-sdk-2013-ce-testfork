#include "cbase.h"
#include "CWeaponBox.h"

LINK_ENTITY_TO_CLASS(w_weaponbox, CWpnBox);

BEGIN_SIMPLE_DATADESC(base)
	DEFINE_FIELD(szName, FIELD_STRING),
	DEFINE_FIELD(count, FIELD_INTEGER),
END_DATADESC();

BEGIN_DATADESC(CWpnBox)
	DEFINE_UTLVECTOR(pKVEntity, FIELD_EMBEDDED),
	DEFINE_UTLVECTOR(pKVAmmo, FIELD_EMBEDDED),
END_DATADESC();

CWpnBox::CWpnBox()
{
}
CWpnBox::~CWpnBox()
{
	bGiveAmmo = bGiveWeapon = bGiveKVAmmo = bGiveKVEntity = false;

	for (auto &var : pWeapon)
		var = nullptr;

	pKVEntity.Purge();
	pAmmo.Purge();
	pKVAmmo.Purge();
}
void CWpnBox::Spawn(void)
{
	Precache();
	SetModel(WEAPONBOX_MODEL);

	BaseClass::Spawn();
}
void CWpnBox::Precache(void)
{
	PrecacheModel(WEAPONBOX_MODEL);
	PrecacheScriptSound("Item.Pickup");
}
bool CWpnBox::KeyValue(const char *szKeyName, const char *szKeyValue)
{
	if (GetAmmoDef()->Index(szKeyName) > -1)
	{
		base temp;
		temp.szName = (char *)szKeyName;
		temp.count = atoi(szKeyValue);

		pKVAmmo.AddToTail(temp);

		bGiveKVAmmo = true;

		return true;
	}
	if (!strnicmp(szKeyName, "weapon_", 7) || !strnicmp(szKeyName, "ammo_", 6))
	{
		base temp;
		temp.szName = (char *)szKeyName;
		temp.count = atoi(szKeyValue);

		pKVEntity.AddToTail(temp);

		bGiveKVEntity = true;

		return true;
	}

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

	if (!ToBasePlayer(pOther))
		return;

	//PackDeadPlayerItems()
	if (bGiveAmmo)
		GiveAmmo(ToBasePlayer(pOther));
	if (bGiveWeapon)
		GiveWeapon(ToBasePlayer(pOther));


	if (bGiveKVAmmo)
		GiveKVAmmo(ToBasePlayer(pOther));
	if (bGiveKVEntity)
		GiveKVEntity(ToBasePlayer(pOther));

	CPASAttenuationFilter filter(this);
	EmitSound(filter, pOther->entindex(), "Item.Pickup");

	SetTouch(NULL);
	UTIL_Remove(this);
}
void CWpnBox::AddWeapon(CBaseCombatWeapon *pWpn, int pos)
{
	pWeapon[pos] = pWpn;
	bGiveWeapon = true;
}
void CWpnBox::AddAmmo(const char *szName, int count)
{
	base temp;
	temp.szName = (char *)szName;
	temp.count = count;

	pAmmo.AddToTail(temp);
	bGiveAmmo = true;
}
void CWpnBox::GiveWeapon(CBasePlayer *pPlayer)
{
	for (int i = 0; i < MAX_WEAPONS && pWeapon[i] != NULL; ++i)
		pWeapon[i]->GiveTo(pPlayer);

	bGiveWeapon = false;
}
void CWpnBox::GiveAmmo(CBasePlayer *pPlayer)
{
	for (int i = 0; i < pAmmo.Count(); i++)
		for (int a = 0; a < pAmmo[i].count; a++)
			pPlayer->GiveAmmo(pAmmo[i].count, pAmmo[i].szName, true);

	bGiveAmmo = false;
}
void CWpnBox::GiveKVEntity(CBasePlayer *pPlayer)
{
	for (int i = 0; i < pKVEntity.Count(); i++)
		for (int a = 0; a < pKVEntity[i].count; a++)
			pPlayer->GiveNamedItem(pKVEntity[i].szName);

	bGiveKVEntity = false;
}
void CWpnBox::GiveKVAmmo(CBasePlayer *pPlayer)
{
	for (int i = 0; i < pKVAmmo.Count(); i++)
		pPlayer->GiveAmmo(pKVAmmo[i].count, pKVAmmo[i].szName, true);

	bGiveKVAmmo = false;
}
