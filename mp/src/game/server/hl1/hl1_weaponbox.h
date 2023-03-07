#pragma once
#include "cbase.h"
#include "hl1_items.h"
#include "ammodef.h"

#define WEAPONBOX_MODEL "models/w_weaponbox.mdl"


class CWeaponBox : public CHL1Item
{
public:
	DECLARE_CLASS( CWeaponBox, CHL1Item );

	void Spawn( void );
	void Precache( void );
	bool KeyValue( const char *szKeyName, const char *szValue );
	void BoxTouch( CBaseEntity *pPlayer );

	DECLARE_DATADESC();

//private:
	bool		PackAmmo( char *szName, int iCount );
	int			GiveAmmo( int iCount, char *szName, int iMax, int *pIndex = NULL );
	
	int			m_cAmmoTypes;	// how many ammo types packed into this box (if packed by a level designer)
	string_t	m_rgiszAmmo[MAX_AMMO_SLOTS];	// ammo names
	int			m_rgAmmo[MAX_AMMO_SLOTS];		// ammo quantities
};