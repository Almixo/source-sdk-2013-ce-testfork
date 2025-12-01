#include "cbase.h"
#include "hl1mp_basecombatweapon_shared.h"
#ifndef CLIENT_DLL
#include "player.h"
#include "CUSTOM/info_point_displace.h"
#endif // !CLIENT_DLL


#ifdef CLIENT_DLL
#define CDisplacer C_Displacer
#endif //CLIENT_DLL

class CDisplacer : public CBaseCombatWeapon
{
	DECLARE_CLASS(CDisplacer, CBaseCombatWeapon);
	DECLARE_DATADESC();
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

public:

	CDisplacer(void);

	void PrimaryAttack(void);
	bool CanHolster(void);

	void ItemPostFrame(void);

private:

	CBaseEntity *pFound;
//	CBaseEntity *pEnt;

	CNetworkVar(float, fNextThink);
	CNetworkVar(bool, bCharged);
	CNetworkVar(bool, bInThink);
};
IMPLEMENT_NETWORKCLASS_ALIASED(Displacer, DT_Displacer);

BEGIN_NETWORK_TABLE(CDisplacer, DT_Displacer)
#ifdef CLIENT_DLL
RecvPropFloat(RECVINFO(fNextThink)),
RecvPropBool(RECVINFO(bCharged)),
#else
SendPropFloat(SENDINFO(fNextThink)),
SendPropBool(SENDINFO(bCharged)),
#endif
END_NETWORK_TABLE();

BEGIN_PREDICTION_DATA(CDisplacer)
#ifdef CLIENT_DLL
DEFINE_PRED_FIELD(fNextThink,	FIELD_FLOAT,	FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(bCharged,		FIELD_BOOLEAN,	FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(bInThink,		FIELD_BOOLEAN,	FTYPEDESC_INSENDTABLE),
#endif
END_PREDICTION_DATA();

LINK_ENTITY_TO_CLASS(weapon_displacer, CDisplacer);

PRECACHE_WEAPON_REGISTER(weapon_displacer);

BEGIN_DATADESC(CDisplacer)
DEFINE_FIELD(pFound, FIELD_CLASSPTR),
END_DATADESC();

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
CDisplacer::CDisplacer(void)
{
	m_bReloadsSingly	=	false;
	m_bFiresUnderwater	=	false;

	bInThink			=	false;
	fNextThink			=	0;
	pFound				=	NULL;
}
void CDisplacer::PrimaryAttack(void)
{
#ifndef CLIENT_DLL
//	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return;
	}

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 20)
	{
		WeaponSound(EMPTY);
		pPlayer->SetNextAttack(gpGlobals->curtime + 1.0f);
		return;
	}

	//auto pEnt = gEntList.FindEntityByClassnameWithin(NULL, "point_displace", Vector(-64, -64, -64), Vector(64, 64, 64));
	auto pEnt = gEntList.FindEntityByClassnameNearest("point_displace", pPlayer->GetAbsOrigin(), 64.0f);
	if (pEnt && !bInThink)
	{

		pFound = pEnt;

		bInThink = true; //we don't want to switch current weapon now

		Warning("waiting for itempostframe()!\n");

		pPlayer->RemoveAmmo(20, m_iPrimaryAmmoType);

		pPlayer->m_flNextAttack = gpGlobals->curtime + 1.5f;

		fNextThink = gpGlobals->curtime + 0.5f;

	}
	else
	{
		WeaponSound(SPECIAL3);
		pPlayer->m_flNextAttack = gpGlobals->curtime + 1.5f;
	}
#endif
}
bool CDisplacer::CanHolster(void)
{
	if (bInThink)
		return false;

	return BaseClass::CanHolster();
}
void CDisplacer::ItemPostFrame(void)
{
	BaseClass::ItemPostFrame();

	if (bInThink)
	{
		SendWeaponAnim(ACT_VM_PRIMARYATTACK);
		WeaponSound(SPECIAL1);

		if (fNextThink < gpGlobals->curtime)
		{
#ifndef CLIENT_DLL
			CDispPoint *pPoint = dynamic_cast<CDispPoint*>(pFound);

			Warning("entity name is %s", pFound->GetDebugName());

			CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

			auto ptemp = gEntList.FindEntityByName(NULL, pPoint->string);
			if (!ptemp) return;

			pPlayer->SetAbsOrigin(ptemp->GetAbsOrigin());
			pPlayer->SetAbsAngles(ptemp->GetAbsAngles());
			pPlayer->SetAbsVelocity(Vector(0, 0, 0));

			WeaponSound(SPECIAL2);
			UTIL_ScreenFade(pPlayer, { 0, 200, 0, 255 }, 0.5f, 0.1f, FFADE_IN);

			bInThink = false;

			DevWarning("itempostframe() portion done!\n");
#endif
		}
	}
}
