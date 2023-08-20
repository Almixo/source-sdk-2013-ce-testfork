//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef HL1_WEAPON_SATCHEL_H
#define HL1_WEAPON_SATCHEL_H
#ifdef _WIN32
#pragma once
#endif


#include "in_buttons.h"
#include "hl1_basegrenade.h"
#include "hl1_basecombatweapon_shared.h"


//-----------------------------------------------------------------------------
// CWeaponSatchel
//-----------------------------------------------------------------------------

class CWeaponSatchel : public CBaseHL1CombatWeapon
{
	DECLARE_CLASS( CWeaponSatchel, CBaseHL1CombatWeapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
public:

	CWeaponSatchel( void );

	void	Equip( CBaseCombatCharacter *pOwner );
	bool	HasAnyAmmo( void );
	bool	CanDeploy( void );
	void	Precache( void );
	void	PrimaryAttack( void );
	void	SecondaryAttack( void );
	void	WeaponIdle( void );
	bool	Deploy( void );
	bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );

	void		ItemPostFrame( void );
	const char	*GetViewModel( int viewmodelindex = 0 ) const;
	const char	*GetWorldModel( void ) const;

	bool	HasChargeDeployed() { return ( m_iChargeReady != 0 ); }

private:
	void	Throw( void );
	void	ActivateSatchelModel( void );
	void	ActivateRadioModel( void );

	int		m_iRadioViewIndex;
	int		m_iRadioWorldIndex;
	int		m_iSatchelViewIndex;
	int		m_iSatchelWorldIndex;
	int		m_iChargeReady;
};


//-----------------------------------------------------------------------------
// CSatchelCharge
//-----------------------------------------------------------------------------

class CSatchelCharge : public CHL1BaseGrenade
{
	DECLARE_CLASS( CSatchelCharge, CHL1BaseGrenade );
	DECLARE_DATADESC();
public:

	CSatchelCharge();

	void	Spawn( void );
	void	Precache( void );
	void	SatchelTouch( CBaseEntity *pOther );
	void	SatchelThink( void );
	void	SatchelUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

private:
	Vector	m_vLastPosition;
	float	m_flNextBounceSoundTime;
	bool	m_bInAir;

	void	BounceSound( void );
	void	UpdateSlideSound( void );
	void	Deactivate( void );
};


#endif // HL1_WEAPON_SATCHEL_H
