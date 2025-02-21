#ifndef CGUIDEDWEAPON_H
#define CGUIDEDWEAPON_H
#ifdef _WIN32
#pragma once
#endif
#include "cbase.h"
#include "basecombatweapon_shared.h"
#include "gamerules.h"
#ifndef CLIENT_DLL
#include "Sprite.h"
#else
#include "c_sprite.h"
#endif

#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
#define CGuidedWeapon C_GuidedWeapon
#endif

#define SPRITE_MATERIAL "sprites/redglow_mp1.vmt"
#define SPRITE_TRANS kRenderWorldGlow, 255, 0, 0, 255, kRenderFxNoDissipation

#define VECTORGOOD Vector(0.001f, 0.001f, 0.001f)
#define VECTORNOAIM Vector(0.1f, 0.1f, 0.1f)

static ConVar cstm_dot_size("cstm_dot_size", "0.1f", FCVAR_ARCHIVE);
#define DOTSIZE cstm_dot_size.GetFloat()

class CSprite;

class CGuidedWeapon : public CBaseCombatWeapon
{
	DECLARE_CLASS(CGuidedWeapon, CBaseCombatWeapon);
public:
	DECLARE_DATADESC();
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CGuidedWeapon(void);
	~CGuidedWeapon(void);

	CNetworkVar(bool, bDot);
	CNetworkVar(bool, bHelperDot);
	CNetworkVar(bool, bReloadDot);
	CNetworkVector(DotEndPos);
	CNetworkHandle(CSprite, pSprite);

	/*bool bDot			= false; 
	bool bHelperDot		= false;
	bool bReloadDot		= false;*/

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

//	Vector DotEndPos;
//	CSprite *pSprite;
};
IMPLEMENT_NETWORKCLASS_ALIASED(GuidedWeapon, DT_GuidedWeapon);

BEGIN_NETWORK_TABLE(CGuidedWeapon, DT_GuidedWeapon)
#ifdef CLIENT_DLL
//RecvPropVector(RECVINFO(DotEndPos)),
RecvPropVector(RECVINFO(DotEndPos)),
RecvPropEHandle(RECVINFO(pSprite)),
RecvPropBool(RECVINFO(bReloadDot)),
RecvPropBool(RECVINFO(bHelperDot)),
RecvPropBool(RECVINFO(bDot)),
#else
SendPropVector(SENDINFO(DotEndPos)),
SendPropEHandle(SENDINFO(pSprite)),
SendPropBool(SENDINFO(bReloadDot)),
SendPropBool(SENDINFO(bHelperDot)),
SendPropBool(SENDINFO(bDot)),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CGuidedWeapon)
DEFINE_PRED_FIELD(bDot, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(bReloadDot, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(bHelperDot, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(DotEndPos, FIELD_VECTOR, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(pSprite, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS(weapon_guided, CGuidedWeapon);

PRECACHE_WEAPON_REGISTER(weapon_guided);

BEGIN_DATADESC(CGuidedWeapon)
DEFINE_FIELD(bDot, FIELD_BOOLEAN),
DEFINE_FIELD(bHelperDot, FIELD_BOOLEAN),
DEFINE_FIELD(bReloadDot, FIELD_BOOLEAN),
//DEFINE_FIELD(pSprite, FIELD_CLASSPTR),
DEFINE_FIELD(pSprite, FIELD_EHANDLE),
DEFINE_FIELD(DotEndPos, FIELD_VECTOR),
END_DATADESC();
#endif //CGUIDEDWEAPON_H