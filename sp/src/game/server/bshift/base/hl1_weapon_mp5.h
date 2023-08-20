//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Projectile shot from the MP5
//
//=============================================================================//

#ifndef	WEAPONMP5_H
#define	WEAPONMP5_H

#include "hl1_basegrenade.h"
#include "hl1_basecombatweapon_shared.h"

class CGrenadeMP5;

class CWeaponMP5 : public CBaseHL1CombatWeapon
{
	DECLARE_CLASS( CWeaponMP5, CBaseHL1CombatWeapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
public:

	CWeaponMP5();

	void	Precache( void );
	void	PrimaryAttack( void );
	void	SecondaryAttack( void );
	void	DryFire( void );
};


#endif	//WEAPONMP5_H
