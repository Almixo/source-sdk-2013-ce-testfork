#include "cbase.h"
#include "CWeaponBox.h"

BEGIN_DATADESC( CWpnBox )
	DEFINE_ENTITYFUNC(BoxTouch),
	DEFINE_AUTO_ARRAY(szAmmo, FIELD_STRING),
	DEFINE_AUTO_ARRAY(iAmmoCount, FIELD_INTEGER),
END_DATADESC()

LINK_ENTITY_TO_CLASS( w_weaponbox, CWpnBox )

CWpnBox::CWpnBox()
{
	memset( szAmmo, 0, sizeof szAmmo );
	memset( iAmmoCount, 0, sizeof iAmmoCount );
	memset( pWeapon, 0, sizeof pWeapon );

	pPlayer = nullptr;
}
CWpnBox::~CWpnBox()
{
	memset( szAmmo, 0, sizeof szAmmo );
	memset( iAmmoCount, 0, sizeof iAmmoCount );
	memset( pWeapon, 0, sizeof pWeapon );

	pPlayer = nullptr;
}
void CWpnBox::Spawn()
{
	Precache();
	SetModel( WEAPONBOX_MODEL );
	BaseClass::Spawn();

	SetTouch( &CWpnBox::BoxTouch );
}
void CWpnBox::Precache()
{
	BaseClass::Precache();

	PrecacheModel( WEAPONBOX_MODEL );
	PrecacheScriptSound( "Item.Pickup" );
}
bool CWpnBox::KeyValue( const char *szKeyName, const char *szKeyValue )
{
	int index = GetAmmoDef()->Index( szKeyName );
	if ( index >= 0 )
	{
		AddAmmo( index, szKeyName, atoi( szKeyValue ) );
	}

	if ( !strncmp( szKeyName, "weapon_", 7 ) )
	{
		for ( int i = 0; i < atoi( szKeyValue ); i++ )
		{
			CBaseCombatWeapon *pEnt = (CBaseCombatWeapon*)CreateEntityByName( szKeyName );
			AddWeapon( pEnt );
		}
	}

	return BaseClass::KeyValue( szKeyName, szKeyValue );
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

	GiveWeapon();
	GiveAmmo();

	CPASAttenuationFilter filter( pOther );
	EmitSound( filter, pOther->entindex(), "Item.Pickup" );

	SetTouch( NULL );
	SetThink( &BaseClass::SUB_Remove );
	SetNextThink( gpGlobals->curtime + 0.1f );
}
void CWpnBox::AddWeapon( CBaseCombatWeapon *pWpn )
{
	pWeapon[iWpnIndex++] = pWpn;
}
void CWpnBox::AddAmmo( int index, const char *szName, int count )
{	
	szAmmo[index] = AllocPooledString( szName );
	iAmmoCount[index] += count;
}
void CWpnBox::GiveWeapon()
{
	for ( int i = 0; i < MAX_WEAPONS; i++ )
	{
		if ( pWeapon[i] == nullptr )
			continue;

		if ( pWeapon[i]->GetAbsOrigin() == vec3_origin ) // it wasn't spawned yet, packed through KVs
			pWeapon[i]->Spawn();

		pWeapon[i]->GiveTo( pPlayer );
	}
}
void CWpnBox::GiveAmmo()
{
	for ( int i = 0; i < MAX_AMMO_TYPES; i++ )
	{
		if ( szAmmo[i] == NULL_STRING )
			continue;

		pPlayer->GiveAmmo( iAmmoCount[i], STRING(szAmmo[i]), true);
	}
}

