#include "cbase.h"
#include "CGuidedSprite.h"

#include "hl1mp_basecombatweapon_shared.h"

#ifdef CLIENT_DLL
#define CGuided C_Guided
#endif

#define SPRITE_MATERIAL "sprites/redglow_mp1.vmt"
#define SPRITE_TRANS kRenderWorldGlow, 255, 0, 0, 255, kRenderFxNoDissipation

enum DotState_t
{
	DOT_WAS = -1,
	DOT_OFF,
	DOT_ON
};

class CGuided : public CBaseHL1MPCombatWeapon
{
	DECLARE_CLASS(CGuided, CBaseHL1MPCombatWeapon);
public:
	DECLARE_DATADESC();
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CGuided();
	~CGuided();

	bool Deploy();
	bool Holster(CBaseCombatWeapon *pSwitchingTo);

	void ItemPostFrame(void);

	void PrimaryAttack(void);
	void SecondaryAttack(void);
	bool Reload(void);
	void FinishReloading(void);

	void ToggleDot();
	void DotOn();
	void DotOff();

	Activity GetDrawActivity( void ) {
		if ( m_iClip1 <= 0 )
			return ACT_VM_DRAW_EMPTY;
		else
			return ACT_VM_DRAW;
	}
		
	Activity GetPrimaryAttackActivity( void ) {
		if ( m_iClip1 <= 0 )
			return ACT_GLOCK_SHOOTEMPTY;
		else
			return ACT_VM_PRIMARYATTACK;
	}

	CGuidedDot *CreateDot() {
		return CGuidedDot::Create( vec3_origin, GetOwnerEntity(), m_iDotState == DOT_ON );
	}

private:
	CHandle<CGuidedDot>m_pDot;

	CNetworkVar(float, m_flDotTurnOn);
	CNetworkVar(int, m_iDotState);
};