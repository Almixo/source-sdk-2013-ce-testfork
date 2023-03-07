#ifndef CDOTGUIDEDWEAPON_H
#define CDOTGUIDEDWEAPON_H
#ifdef _WIN32
#pragma once
#endif
#include <cbase.h>
//#include <hl1mp_basecombatweapon_shared.h>
#include "basecombatweapon.h"
#include <gamerules.h>
#include <Sprite.h>

#include "tier0/memdbgon.h"

#define SPRITE_MATERIAL "sprites/redglow_mp1.vmt"
#define SPRITE_TRANS kRenderWorldGlow, 255, 0, 0, 255, kRenderFxNoDissipation

#define VECTORGOOD Vector(0.001f, 0.001f, 0.001f)
#define VECTORNOAIM Vector(0.1f, 0.1f, 0.1f)

const ConVar cstm_dot_size("cstm_dot_size", "0.1f", FCVAR_ARCHIVE);
#define DOTSIZE cstm_dot_size.GetFloat()

class CSprite;
class CGDot : public CBaseEntity
{
	DECLARE_CLASS(CGDot, CBaseEntity);
public:

	CGDot(void);
	~CGDot(void);

	static CGDot* Create(const Vector &origin, CBaseEntity *pOwner = NULL, bool bVisibleDot = true);

	void	SetTargetEntity(CBaseEntity *pTarget) { m_hTargetEnt = pTarget; }
	CBaseEntity* GetTargetEntity(void) { return m_hTargetEnt; }

	void	SetLaserPosition(const Vector& origin, const Vector& normal);
	Vector	GetChasePosition();
	void	TurnOn(void);
	void	TurnOff(void);
	bool	IsOn() const { return m_bIsOn; }

	void	Toggle(void);

	int		ObjectCaps() { return (BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DONT_SAVE; }

	void	MakeInvisible(void);

#ifdef CLIENT_DLL

	virtual bool			IsTransparent(void) { return true; }
	virtual RenderGroup_t	GetRenderGroup(void) { return RENDER_GROUP_TRANSLUCENT_ENTITY; }
	virtual int				DrawModel(int flags);
	virtual void			OnDataChanged(DataUpdateType_t updateType);
	virtual bool			ShouldDraw(void) { return (IsEffectActive(EF_NODRAW) == false); }

	CMaterialReference	m_hSpriteMaterial;
#endif

protected:
	Vector				m_vecSurfaceNormal;
	EHANDLE				m_hTargetEnt;
	bool				m_bVisibleLaserDot;
	//	bool				m_bIsOn;
	CNetworkVar(bool, m_bIsOn);

	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();
public:
	CGDot* m_pNext;
};

CBaseEntity* CreateCGDot(const Vector &origin, CBaseEntity *pOwner, bool bVisibleDot);
void SetCGDotPos(CBaseEntity *pLaserDot, CBaseEntity *pTarget);
void EnableCGDot(CBaseEntity *pLaserDot, bool bEnable);

class CGdWeapon : public CBaseCombatWeapon
{
	DECLARE_CLASS(CGdWeapon, CBaseCombatWeapon);
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

public:
	CGdWeapon(void);
	~CGdWeapon(void);

	//CNetworkVar(bool, bDot);
	//CNetworkVar(bool, bHelperDot);
	//CNetworkVar(bool, bReloadDot);

	bool bDot			= false; 
	bool bHelperDot		= false;
	bool bReloadDot		= false;

	void Spawn(void);
	bool Deploy(void);
	void PrimaryAttack(void);
	void SecondaryAttack(void);
	void Precache(void);
	bool Reload(void);
	void ItemPostFrame(void);
	bool Holster(CBaseCombatWeapon *pSwitchingTo);
	void ToggleDot(void);
	void UpdateDotPos(void);
	void SpawnDot(void);
	void FinishReload(void);
	void WeaponIdle(void);

	bool IsDotOn(void);

	CGDot *pSprite;
};
IMPLEMENT_SERVERCLASS_ST(CGdWeapon, DT_CGdGuidedWeapon)
END_SEND_TABLE();

LINK_ENTITY_TO_CLASS(weapon_cguided, CGdWeapon);
PRECACHE_WEAPON_REGISTER(weapon_cguided);
BEGIN_DATADESC(CGdWeapon)
DEFINE_FIELD(bDot, FIELD_BOOLEAN),
DEFINE_FIELD(bHelperDot, FIELD_BOOLEAN),
DEFINE_FIELD(bReloadDot, FIELD_BOOLEAN),
DEFINE_FIELD(pSprite, FIELD_CLASSPTR),
END_DATADESC();
#endif //CDOTGUIDEDWEAPON_H