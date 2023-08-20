//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Barney's armour
//
//=============================================================================//

#include "cbase.h"
#include "hl1_items.h"

//must be last!
#include "tier0/memdbgon.h"

#define ARMOR_VEST_MODEL "models/barney_vest.mdl"
#define ARMOR_HELMET_MODEL "models/barney_helmet.mdl"

/////////////////////
// Barney vest
/////////////////////
class CItemArmorVest : public CHL1Item
{
	DECLARE_CLASS(CItemArmorVest, CHL1Item);
	DECLARE_DATADESC()
public:
	
	short iBackupPose;
	short iPose;

	void Spawn(void)
	{
		CItem::Spawn();

		Precache();
		SetModel(ARMOR_VEST_MODEL);

		if (iBackupPose != NULL)
		{
			SetSequence(iBackupPose);
		}
		else SetSequence(iPose);

		if (GetSequence() == -1 || GetSequence() > 3)
		{
			Warning("%s with a bad pose\n", GetDebugName());
			SetSequence(0);
		}
	}
	void Precache(void)
	{
		PrecacheModel(ARMOR_VEST_MODEL);
		PrecacheScriptSound("Item.Pickup");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if ((pPlayer->ArmorValue() < MAX_NORMAL_BATTERY) && pPlayer->IsSuitEquipped())
		{
			pPlayer->IncrementArmorValue(60, MAX_NORMAL_BATTERY);

			CPASAttenuationFilter filter(pPlayer, "Item.Pickup");
			EmitSound(filter, pPlayer->entindex(), "Item.Pickup");

			CSingleUserRecipientFilter user(pPlayer);
			user.MakeReliable();

			UserMessageBegin(user, "ItemPickup");
			MessageWriteString(GetClassname());
			MessageEnd();

			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_armorvest, CItemArmorVest);
PRECACHE_REGISTER(item_armorvest);
BEGIN_DATADESC(CItemArmorVest)
	DEFINE_KEYFIELD(iBackupPose, FIELD_SHORT, "sequence"),
	DEFINE_KEYFIELD(iPose, FIELD_SHORT, "pose"),
END_DATADESC();

/////////////////////
// Barney helmet
/////////////////////
class CItemHelmet : public CHL1Item
{
	DECLARE_CLASS(CItemHelmet, CHL1Item);
	DECLARE_DATADESC()
public:

	short iBackupPose;
	short iPose;

	void Spawn(void)
	{
		CItem::Spawn();

		Precache();
		SetModel(ARMOR_HELMET_MODEL);	//barney_helmet
		
		if (iBackupPose != NULL)
		{
			SetSequence(iBackupPose);
		}
		else SetSequence(iPose);

		if (GetSequence() == -1 || GetSequence() > 3)
		{
			Warning("%s with a bad pose\n", GetDebugName());
			SetSequence(0);
		}
	}
	void Precache(void)
	{
		PrecacheModel(ARMOR_HELMET_MODEL);
		PrecacheScriptSound("Item.Pickup");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if ((pPlayer->ArmorValue() < MAX_NORMAL_BATTERY) && pPlayer->IsSuitEquipped())
		{
			pPlayer->IncrementArmorValue(40, MAX_NORMAL_BATTERY);

			CPASAttenuationFilter filter(pPlayer, "Item.Pickup");
			EmitSound(filter, pPlayer->entindex(), "Item.Pickup");

			CSingleUserRecipientFilter user(pPlayer);
			user.MakeReliable();

			UserMessageBegin(user, "ItemPickup");
			MessageWriteString(GetClassname());
			MessageEnd();

			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_helmet, CItemHelmet);
PRECACHE_REGISTER(item_helmet);
BEGIN_DATADESC(CItemHelmet)
	DEFINE_KEYFIELD(iBackupPose, FIELD_SHORT, "sequence"),
	DEFINE_KEYFIELD(iPose, FIELD_SHORT, "pose"),
END_DATADESC();