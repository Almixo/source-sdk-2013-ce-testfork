#include "cbase.h"
#include "hl1_items.h"
#include "ammodef.h"
#include "hl1mp_gamerules.h"
#include "hl1mp_player.h"

#define WEAPONBOX_MODEL "models/w_weaponbox.mdl"

class CWpnBox : public CHL1Item
{
public:
	DECLARE_CLASS( CWpnBox, CHL1Item );

	void Spawn( void );
	void Precache( void );
	bool KeyValue( const char *szKeyName, const char *szValue );
	void Touch( CBaseEntity *pOther );

	DECLARE_DATADESC();

	void GiveAmmo( CBaseEntity *pOther );
	void GiveWpn( CBaseEntity *pOther );

	CUtlVectorFixed<char*, MAX_WEAPONS>szWpns;
	CUtlVectorFixed<char*, MAX_AMMO_TYPES>szAmmo;
	CUtlVectorFixed<int, MAX_AMMO_TYPES>iAmmoCount;

	bool bWpn = false;
	bool bAmmo = false;
};
LINK_ENTITY_TO_CLASS( w_weaponbox, CWpnBox );
PRECACHE_REGISTER( w_weaponbox );

BEGIN_DATADESC( CWpnBox )
DEFINE_ARRAY( szWpns, FIELD_STRING, MAX_WEAPONS ),
DEFINE_ARRAY( szAmmo, FIELD_STRING, MAX_AMMO_TYPES ),
DEFINE_ARRAY( iAmmoCount, FIELD_INTEGER, MAX_AMMO_TYPES ),
DEFINE_FIELD( bWpn, FIELD_BOOLEAN ),
DEFINE_FIELD( bAmmo, FIELD_BOOLEAN ),
END_DATADESC();

void CWpnBox::Spawn()
{
	BaseClass::Spawn();
	Precache();

	DevWarning( "%s spawned at %g %g %g!\n", GetDebugName(), GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z );
	
	SetModel( WEAPONBOX_MODEL );
}
void CWpnBox::Precache( void )
{
	BaseClass::Precache();
	PrecacheModel( WEAPONBOX_MODEL );
}
bool CWpnBox::KeyValue( const char *szKeyName, const char *szValue )
{
	const char *p = strstr( szKeyName, "weapon_" );
	if ( p != NULL ) //it's a weapon!
	{
		bWpn = true;
		Warning( "char *p = %s.\n", p );
		szWpns.AddToHead( (char*)szKeyName );

		return true;
	}
	if ( GetAmmoDef()->Index( szKeyName ) != NULL )
	{
		bAmmo = true;
		Warning("AmmoDef might be %s?\n", szKeyName);
		szAmmo.AddToTail((char*)szKeyName);

		if ( atoi( szValue ) > 0 )
			iAmmoCount.AddToTail(atoi(szValue));

		return true;
	}
	
	BaseClass::KeyValue( szKeyName, szValue );
	return false;
}
void CWpnBox::Touch( CBaseEntity *pOther )
{
	if ( !( GetFlags() & FL_ONGROUND ) )
		return;

	if ( !pOther->IsPlayer() )
		return;

	if ( !pOther->IsAlive() )
		return;

	if ( bAmmo == true )
		GiveAmmo(pOther);
	if ( bWpn == true )
		GiveWpn(pOther);

	UTIL_Remove( this ); // so we don't get more ammo/weapons than we should
}
void CWpnBox::GiveAmmo( CBaseEntity *pOther )
{
	CHL1MP_Player *pPlayer = static_cast<CHL1MP_Player*>(pOther);

	for ( int i = 0; i < MAX_AMMO_TYPES; i++ )
	{
		if ( szAmmo[ i ] == NULL || iAmmoCount[ i ] == NULL )
			continue;

		DevWarning("Would have given %i ammo of type %s.\n", iAmmoCount[i], szAmmo[i]);
		pPlayer->GiveAmmo( iAmmoCount[ i ], szAmmo[ i ] );
	}
}
void CWpnBox::GiveWpn( CBaseEntity *pOther )
{
	CHL1MP_Player *pPlayer = static_cast<CHL1MP_Player*>(pOther);

	for ( int i = 0; i < MAX_WEAPONS; i++ )
	{
		if ( szWpns[ i ] == NULL )
			continue;

		DevWarning("Would have given you weapon %s.\n", szWpns[i]);
		pPlayer->GiveNamedItem( szWpns[ i ] );
	}
}