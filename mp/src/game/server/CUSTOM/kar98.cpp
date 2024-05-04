#include "cbase.h"
#include "hl1_basecombatweapon_shared.h"

class CKar98 : public CBaseHL1CombatWeapon
{
	DECLARE_CLASS(CKar98, CBaseHL1CombatWeapon);
	DECLARE_SERVERCLASS();
public:

	CKar98();

	bool Reload(void);
	Activity GetDrawActivity(void);
	Activity GetPrimaryAttackActivity(void);
};

LINK_ENTITY_TO_CLASS(weapon_kar98, CKar98);

PRECACHE_WEAPON_REGISTER(weapon_kar98);

IMPLEMENT_SERVERCLASS_ST(CKar98, DT_Kar98)
END_SEND_TABLE();

CKar98::CKar98()
{
	m_bFiresUnderwater = false;
	m_bReloadsSingly = false;
}

bool CKar98::Reload(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if (!pOwner)
		return false;

	// If I don't have any spare ammo, I can't reload
	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return false;
	
	if (m_iClip1 <= 0)
		return false;

	m_iClip1 = 0;

	return BaseClass::Reload();
}

Activity CKar98::GetDrawActivity(void)
{
	if (m_iClip1 == 0)
		return ACT_VM_DRAW_EMPTY;
	else
		return ACT_VM_DRAW;
}

Activity CKar98::GetPrimaryAttackActivity(void)
{
	if ((m_iClip1 - 1) <= 0)
		return ACT_GLOCK_SHOOTEMPTY;
	else
		return ACT_VM_PRIMARYATTACK;
}