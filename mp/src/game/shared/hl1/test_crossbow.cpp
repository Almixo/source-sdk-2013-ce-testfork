//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"

#ifdef CLIENT_DLL
	#define CWeaponSnipertest C_WeaponSnipertest
	#include "c_sdk_player.h"
#else
	#include "sdk_player.h"
#endif


class CWeaponSnipertest : public CWeaponSDKBase
{
public:
	DECLARE_CLASS(CWeaponSnipertest, CWeaponSDKBase);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponSnipertest();
	CWeaponSnipertest(const CWeaponSnipertest &);

	void SecondaryAttack(void);
	void AddViewKick();
	bool Holster(CBaseCombatWeapon *pSwitchingTo);	// Required so that you know to un-zoom when switching weapons
	void ToggleZoom(void);	// If the weapon is zoomed, un-zoom and vice versa
	bool Reload(void);

	virtual int GetFireMode() const { return FM_SEMIAUTOMATIC; }
	virtual SDKWeaponID GetWeaponID(void) const { return SDK_WEAPON_TESTSNIPER; } //what are these for?

private:
	CNetworkVar(bool, m_bInZoom);	// Set to true when you are zooming, false when not
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponSnipertest, DT_WeaponSnipertest)

BEGIN_NETWORK_TABLE(CWeaponSnipertest, DT_WeaponSnipertest)
#ifndef CLIENT_DLL
	SendPropBool(SENDINFO(m_bInZoom)),
#else
	RecvPropBool(RECVINFO(m_bInZoom)),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponSnipertest)
#ifdef CLIENT_DLL
	DEFINE_PRED_FIELD(m_bInZoom, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
#endif
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_snipertest, CWeaponSnipertest);
PRECACHE_WEAPON_REGISTER(weapon_snipertest);

//Tony; todo; add ACT_MP_PRONE* activities, so we have them.
acttable_t CWeaponSnipertest::m_acttable[] =
{
	{ ACT_MP_STAND_IDLE,					ACT_DOD_STAND_IDLE_TOMMY,				false },
	{ ACT_MP_CROUCH_IDLE,					ACT_DOD_CROUCH_IDLE_TOMMY,				false },
	{ ACT_MP_PRONE_IDLE,					ACT_DOD_PRONE_AIM_TOMMY,				false },

	{ ACT_MP_RUN,							ACT_DOD_RUN_AIM_TOMMY,					false },
	{ ACT_MP_WALK,							ACT_DOD_WALK_AIM_TOMMY,					false },
	{ ACT_MP_CROUCHWALK,					ACT_DOD_CROUCHWALK_AIM_TOMMY,			false },
	{ ACT_MP_PRONE_CRAWL,					ACT_DOD_PRONEWALK_IDLE_TOMMY,			false },
	{ ACT_SPRINT,							ACT_DOD_SPRINT_IDLE_TOMMY,				false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,		ACT_DOD_PRIMARYATTACK_TOMMY,			false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,		ACT_DOD_PRIMARYATTACK_TOMMY,			false },
	{ ACT_MP_ATTACK_PRONE_PRIMARYFIRE,		ACT_DOD_PRIMARYATTACK_PRONE_TOMMY,		false },

	{ ACT_MP_RELOAD_STAND,					ACT_DOD_RELOAD_TOMMY,					false },
	{ ACT_MP_RELOAD_CROUCH,					ACT_DOD_RELOAD_CROUCH_TOMMY,			false },
	{ ACT_MP_RELOAD_PRONE,					ACT_DOD_RELOAD_PRONE_TOMMY,				false },

};
IMPLEMENT_ACTTABLE(CWeaponSnipertest);

CWeaponSnipertest::CWeaponSnipertest()
{
	m_bFiresUnderwater = false;
	m_bInZoom = false;
}

void CWeaponSnipertest::SecondaryAttack(void)
{
	ToggleZoom();

	m_flNextSecondaryAttack = gpGlobals->curtime + 0.35f; //make sure we can't spam it!
}

void CWeaponSnipertest::AddViewKick()
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	QAngle viewPunch;

	viewPunch.x = random->RandomFloat(-5.3f, -5.6f); //Originally was "(0.25f, 0.5f);" //Originally was "(0.25f, 0.5f);" reminder: this is vertical, set to minus to go upwards
	viewPunch.y = random->RandomFloat(-0.0f, -0.0f); //Originally was (-.6f, .6f); //Originally was (-.6f, .6f); reminder: this is horizontal, I guess minus will make it go right direction
	viewPunch.z = 0.0f;

	// Add it to the view punch
	pPlayer->ViewPunch(viewPunch);
}

bool CWeaponSnipertest::Reload(void)
{
	if (m_bInZoom)
		ToggleZoom(); //zoom out when reloading if you are zoomed in!

	return BaseClass::Reload();
}

void CWeaponSnipertest::ToggleZoom(void)
{
	m_bInZoom = !m_bInZoom; //toggle our zoom boolean

#ifndef CLIENT_DLL //why don't we move all this to server anyway?
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	if (!m_bInZoom)
	{
		if (pPlayer->SetFOV(this, 0, 0.2f))
		{
			// Send a message to hide the scope
			CSingleUserRecipientFilter filter(pPlayer);
			UserMessageBegin(filter, "ShowScope");
			WRITE_BYTE(0); //MessageWriteByte()
			MessageEnd();
		}
	}
	else
	{
		if (pPlayer->SetFOV(this, 20, 0.1f))
		{
			// Send a message to Show the scope
			CSingleUserRecipientFilter filter(pPlayer);
			UserMessageBegin(filter, "ShowScope");
			WRITE_BYTE(1); //MessageWriteByte()
			MessageEnd();
		}
	}
#endif
}

bool CWeaponSnipertest::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	if (m_bInZoom)
		ToggleZoom();

	return BaseClass::Holster(pSwitchingTo);
}