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
	const char *GetPModel( void ) const;

    void OnPickedUp( CBaseCombatCharacter *pNewOwner );
    void Detach( void ) override;

    int GetPlayerModelIndex( void ) const { return modelinfo->GetModelIndex( GetPModel() ); }
    int GetWorldModelIndex( void ) const { return modelinfo->GetModelIndex( GetWorldModel() ); }

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

private:
    CNetworkVar( int, m_iPlayerModelIndex );
};

#endif // BASEHLCOMBATWEAPON_SHARED_H
