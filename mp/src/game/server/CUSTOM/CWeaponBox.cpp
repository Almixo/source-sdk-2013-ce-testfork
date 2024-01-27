#include "cbase.h"
#include "CWeaponBox.h"

BEGIN_SIMPLE_DATADESC( base_t )
	DEFINE_AUTO_ARRAY( szName, FIELD_CHARACTER ),
	DEFINE_FIELD( count, FIELD_INTEGER ),
END_DATADESC();

BEGIN_DATADESC( CWpnBox )
	DEFINE_AUTO_ARRAY( pAmmo, FIELD_EMBEDDED ),
	DEFINE_AUTO_ARRAY( pKVEnt, FIELD_EMBEDDED ),
END_DATADESC();

LINK_ENTITY_TO_CLASS( w_weaponbox, CWpnBox );

CWpnBox::CWpnBox()
{
	iWeaponIndex = 0;
	iAmmoIndex = 0;
	iKVEntIndex = 0;
}

void CWpnBox::Spawn()
{
	Precache();
	SetModel( WEAPONBOX_MODEL );
	BaseClass::Spawn();

	DevWarning( "Starting weaponbox print.\n" );
	for ( int i = 0; i < iKVEntIndex; i++ )
		DevMsg( "%s | %d.\n", pKVEnt[i].GetStr(), pKVEnt[i].GetVal() );
	DevWarning( "End weaponbox print.\n" );

	SetTouch( &CWpnBox::BoxTouch );
}

void CWpnBox::Precache()
{
	PrecacheModel( WEAPONBOX_MODEL );
	PrecacheScriptSound( "Item.Pickup" );
}

bool CWpnBox::KeyValue( const char *szKeyName, const char *szValue )
{
	if ( GetAmmoDef()->Index( szKeyName ) >= 0 )
	{
		int count = atoi( szValue ) == 0 ? 1 : atoi( szValue );

		AddKVAmmo( base_t( szKeyName, count ), GetAmmoDef()->Index( szKeyName ) );

		return true;
	}
	else if ( !Q_strnicmp( szKeyName, "weapon_", 7 ) || !Q_strnicmp( szKeyName, "ammo_", 5 ) )
	{
		int count = atoi( szValue ) == 0 ? 1 : atoi( szValue );
		AddKVEnt( base_t( szKeyName, count ) );

		return true;
	}
	else
	{
		BaseClass::KeyValue( szKeyName, szValue );
		return false;
	}
}

void CWpnBox::BoxTouch( CBaseEntity *pOther )
{
	if ( !( GetFlags() & FL_ONGROUND ) )
		return;
	if ( !pOther->IsPlayer() )
		return;
	if ( !pOther->IsAlive() )
		return;

	pPlayer = ToBasePlayer( pOther );
	if ( !pPlayer )
		return;

	if ( pWeapon[0] != nullptr )
		GiveWeapon();
	else if ( !pAmmo[0].StrEmpty() )
		GiveAmmo();
	else if ( !pKVEnt[0].StrEmpty() )
		GiveKVEnt();

	Msg( "%d wpn.\n", iWeaponIndex);

	CPASAttenuationFilter filter( pOther );
	EmitSound( filter, pOther->entindex(), "Item.Pickup" );

	SetTouch( NULL );
	SetThink( &BaseClass::SUB_Remove );
	SetNextThink( gpGlobals->curtime );
}

void CWpnBox::AddWeapon( CBaseCombatWeapon *pWpn )
{
	if ( iWeaponIndex < MAX_WEAPONS )
	{
		pWeapon[iWeaponIndex] = pWpn;
		iWeaponIndex++;
	}
}

void CWpnBox::AddAmmo( base_t base )
{
	if ( iAmmoIndex < MAX_AMMO_TYPES )
	{
		pAmmo[iAmmoIndex] = base;
		iAmmoIndex++;
	}
}

void CWpnBox::GiveWeapon()
{
	for ( int i = 0; i < iWeaponIndex; i++ )
		pWeapon[i]->GiveTo( pPlayer );
}

void CWpnBox::GiveAmmo()
{
	for ( int i = 0; i < iAmmoIndex; i++ )
		pPlayer->GiveAmmo( pAmmo[i].GetVal(), pAmmo[i].GetStr(), true);
}

void CWpnBox::AddKVEnt( base_t base )
{
	if ( iKVEntIndex < MAX_ENTS )
	{
		pKVEnt[iKVEntIndex] = base;
		iKVEntIndex++;
	}
}

void CWpnBox::AddKVAmmo( base_t base, int index )
{
	if ( index > MAX_AMMO_TYPES - 1 )
		return;

	pAmmo[index].SetVal( pAmmo[index].GetVal() + base.GetVal() );
}

void CWpnBox::GiveKVEnt()
{
	for ( int i = 0; i < iKVEntIndex; i++ )
		for ( int a = 0; a < pKVEnt[i].GetVal(); i++ )
			pPlayer->GiveNamedItem( pKVEnt[i].GetStr() );
}
