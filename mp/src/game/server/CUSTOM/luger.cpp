#include "cbase.h"
#include "hl1_basecombatweapon_shared.h"

class CLuger : public CBaseHL1CombatWeapon
{
	DECLARE_CLASS(CLuger, CBaseHL1CombatWeapon);
	DECLARE_SERVERCLASS();
public:
	CLuger();

	Activity GetDrawActivity(void);
	Activity GetPrimaryAttackActivity(void);

	void WeaponIdle(void);
};

LINK_ENTITY_TO_CLASS(weapon_luger, CLuger);

PRECACHE_WEAPON_REGISTER(weapon_luger);

IMPLEMENT_SERVERCLASS_ST(CLuger, DT_Luger)
END_SEND_TABLE();

CLuger::CLuger()
{
	m_bFiresUnderwater = false;
	m_bReloadsSingly = false;
}

Activity CLuger::GetDrawActivity(void)
{
	if (m_iClip1 <= 0)
		return ACT_VM_DRAW_EMPTY;
	else
		return ACT_VM_DRAW;
}

Activity CLuger::GetPrimaryAttackActivity(void)
{
	if ((m_iClip1 - 1) <= 0)
		return ACT_GLOCK_SHOOTEMPTY;
	else
		return ACT_VM_PRIMARYATTACK;
}

void CLuger::WeaponIdle(void)
{
	if (!HasWeaponIdleTimeElapsed())
		return;

	if (m_iClip1 <= 0)
		SendWeaponAnim(ACT_VM_IDLE_EMPTY);
	else
		SendWeaponAnim(ACT_VM_IDLE);

	m_flTimeWeaponIdle = gpGlobals->curtime + 60 * 10;
}