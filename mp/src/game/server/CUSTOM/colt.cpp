#include "cbase.h"
#include "hl1_basecombatweapon_shared.h"

class CColt : public CBaseHL1CombatWeapon
{
	DECLARE_CLASS(CColt, CBaseHL1CombatWeapon);
	DECLARE_SERVERCLASS();
public:
	CColt();

	Activity GetDrawActivity(void);
	Activity GetPrimaryAttackActivity(void);

	void WeaponIdle(void);
};

LINK_ENTITY_TO_CLASS(weapon_colt, CColt);

PRECACHE_WEAPON_REGISTER(weapon_colt);

IMPLEMENT_SERVERCLASS_ST(CColt, DT_Colt)
END_SEND_TABLE();

CColt::CColt()
{
	m_bFiresUnderwater = false;
	m_bReloadsSingly = false;
}

Activity CColt::GetDrawActivity(void)
{
	if (m_iClip1 <= 0)
		return ACT_VM_DRAW_EMPTY;
	else
		return ACT_VM_DRAW;
}

Activity CColt::GetPrimaryAttackActivity(void)
{
	if ((m_iClip1 - 1) <= 0)
		return ACT_GLOCK_SHOOTEMPTY;
	else
		return ACT_VM_PRIMARYATTACK;
}

void CColt::WeaponIdle(void)
{
	if (!HasWeaponIdleTimeElapsed())
		return;

	if (m_iClip1 <= 0)
		SendWeaponAnim(ACT_VM_IDLE_EMPTY);
	else
		SendWeaponAnim(ACT_VM_IDLE);

	m_flTimeWeaponIdle = gpGlobals->curtime + 10.0f;
}