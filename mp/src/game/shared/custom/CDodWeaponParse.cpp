#include "cbase.h"
#include "CDoDWeaponParse.h"

CDoDWeaponParse::CDoDWeaponParse()
{
}

// This function probably exists somewhere in your mod already.
FileWeaponInfo_t* CreateWeaponInfo()
{
	return new CDoDWeaponParse;
}

void CDoDWeaponParse::Parse( KeyValues *pKeyValuesData, const char *szWeaponName )
{
	BaseClass::Parse( pKeyValuesData, szWeaponName );

	m_flFireDelay = pKeyValuesData->GetFloat( "FireDelay", 0.15f );
	m_flSecondaryFireDelay = pKeyValuesData->GetFloat( "SeconaryFireDelay", 0.15f );

	m_flRecoil = pKeyValuesData->GetFloat( "Recoil", 1.0f );

	m_flSpread = pKeyValuesData->GetFloat( "Spread", 0.1f );
}