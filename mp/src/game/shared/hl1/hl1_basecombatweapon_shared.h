//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "basecombatweapon_shared.h"

#ifndef BASEHLCOMBATWEAPON_SHARED_H
#define BASEHLCOMBATWEAPON_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#if defined( CLIENT_DLL )
#define CBaseHL1CombatWeapon C_BaseHL1CombatWeapon
#endif

class CBaseHL1CombatWeapon : public CBaseCombatWeapon
{
	DECLARE_CLASS( CBaseHL1CombatWeapon, CBaseCombatWeapon );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
public:

	void Spawn( void );
	void Equip( CBaseCombatCharacter *pOwner );
	void Drop( const Vector &vecVelocity );
	bool Holster( CBaseCombatWeapon *pSwitchingTo = nullptr, bool noHolsterAnim = true );
	const char *GetPModel( void ) const;

// Server Only Methods
#if !defined( CLIENT_DLL )
	virtual void Precache();

	//int GetPModelIndex( void );

	void FallInit( void );						// prepare to fall to the ground
	void FallThink( void );						// make the weapon fall to the ground after spawning

	void EjectShell( CBaseEntity *pPlayer, int iType );
#else

	virtual void	AddViewmodelBob( CBaseViewModel *viewmodel, Vector &origin, QAngle &angles );
	virtual	float	CalcViewmodelBob( void );

#endif
};

#endif // BASEHLCOMBATWEAPON_SHARED_H
