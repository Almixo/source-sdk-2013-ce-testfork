#pragma once

#ifndef BASEDODCOMBATWEAPON_SHARED_H
#define BASEDODCOMBATWEAPON_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "hl1mp_basecombatweapon_shared.h"
#include "CDoDWeaponParse.h"
#include "in_buttons.h"

#ifndef CLIENT_DLL
#include "soundent.h"
#endif

#define DEFINE_WEAPON(Class, Inherit) DECLARE_CLASS(Class, Inherit) \
DECLARE_NETWORKCLASS() \
DECLARE_PREDICTABLE()

#define LINK_WEAPON(hammername, Class) LINK_ENTITY_TO_CLASS(hammername, Class) PRECACHE_WEAPON_REGISTER( hammername );

enum weapon_type
{
	COLT,
	LUGER,
	KAR98,
	M1RIFLE,
	M1CARBINE,
	THOMPSON,
    MP_40,
    K_43,
    M1A1BAZOOKA
};

#ifdef CLIENT_DLL
#define CBaseDoDCombatWeapon C_BaseDoDCombatWeapon
#endif

class CBaseDoDCombatWeapon : public CBaseHL1MPCombatWeapon
{
	DECLARE_CLASS( CBaseDoDCombatWeapon, CBaseHL1MPCombatWeapon );

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
public:
	CBaseDoDCombatWeapon();

	void Spawn( void );
	void DoDSpawn( void );

    void ItemPostFrame( void );

	void PrimaryAttack( void ) { return GetFire(); }

	virtual void WeaponIdle( void );

	void FireGun( bool automatic = false );

	void FireSingle( void ) { FireGun( false ); };
	void FireAuto( void ) { FireGun( true ); }

	float GetFireDelay( void ) const;
	float GetSecondaryFireDelay( void ) const;

    bool Reload( void ) { return BaseClass::DefaultReload( GetMaxClip1(), GetMaxClip2(), GetReloadActivity() ); }

	void ApplyRecoil( CBasePlayer *pPlayer );
	void ApplySpread( CBasePlayer *pPlayer, Vector *vec );

    virtual Activity GetReloadActivity( void ) { return m_iClip1 <= 0 ? ACT_VM_RELOAD_EMPTY : ACT_VM_RELOAD; }

	virtual Activity GetLastRoundActivity( void ) { return ACT_VM_PRIMARYATTACK; }
	virtual Activity GetIdleEmptyActivity( void ) { return ACT_VM_IDLE_EMPTY; }

	virtual void GetFire( void ) { return FireSingle(); }

    void FinishReload( void );

#ifdef CLIENT_DLL
    bool ShouldPredict( void );
#endif

public:
	int m_iWeaponType;

	/*bool m_bHasLastRoundActivity;
	bool m_bHasEmptyActivity;*/

	CDoDWeaponParse *m_pWpnInfo;
};

#endif // BASEDODCOMBATWEAPON_SHARED_H