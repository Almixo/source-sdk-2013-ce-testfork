//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Projectile shot from the MP5 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef	WEAPONMP5_H
#define	WEAPONMP5_H

#include "hl1_basegrenade.h"
#include "hl1_basecombatweapon_shared.h"

class CGrenadeMP5;

class CWeaponMP5 : public CBaseHL1CombatWeapon
{
	DECLARE_CLASS( CWeaponMP5, CBaseHL1CombatWeapon );
public:

	CWeaponMP5();

	void	Precache( void );
	void	PrimaryAttack( void );
	void	SecondaryAttack( void );
	void	DryFire( void );
	void	WeaponIdle( void );

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
};


#endif	//WEAPONMP5_H
