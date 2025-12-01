#ifndef CGUIDEDWEAPON_H
#define CGUIDEDWEAPON_H
#ifdef _WIN32
#pragma once
#endif
#include "cbase.h"
#include "basecombatweapon.h"
#include "gamerules.h"
#include "Sprite.h"

#include "tier0/memdbgon.h"

#define SPRITE_MATERIAL "sprites/redglow_mp1.vmt"
#define SPRITE_TRANS kRenderWorldGlow, 255, 0, 0, 255, kRenderFxNoDissipation

#define VECTORGOOD Vector(0.001f, 0.001f, 0.001f)
#define VECTORNOAIM Vector(0.1f, 0.1f, 0.1f)

const ConVar cstm_dot_size("cstm_dot_size", "0.1f", FCVAR_ARCHIVE);
#define DOTSIZE cstm_dot_size.GetFloat()

class CSprite;

class CGuidedWeapon : public CBaseCombatWeapon
{
	DECLARE_CLASS(CGuidedWeapon, CBaseCombatWeapon);
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

public:
	CGuidedWeapon(void);
	~CGuidedWeapon(void);

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

	CSprite *pSprite;
};
IMPLEMENT_SERVERCLASS_ST(CGuidedWeapon, DT_GuidedWeapon)
END_SEND_TABLE();

LINK_ENTITY_TO_CLASS(weapon_guided, CGuidedWeapon);
PRECACHE_WEAPON_REGISTER(weapon_guided);
BEGIN_DATADESC(CGuidedWeapon)
DEFINE_FIELD(bDot, FIELD_BOOLEAN),
DEFINE_FIELD(bHelperDot, FIELD_BOOLEAN),
DEFINE_FIELD(bReloadDot, FIELD_BOOLEAN),
DEFINE_FIELD(pSprite, FIELD_CLASSPTR),
END_DATADESC();
#endif //CGUIDEDWEAPON_H