#pragma once

#include "cbase.h"
#include "weapon_parse.h"

class CDoDWeaponParse : public FileWeaponInfo_t
{
public:
	DECLARE_CLASS_GAMEROOT( CDoDWeaponParse, FileWeaponInfo_t );

	CDoDWeaponParse();

	void Parse( KeyValues* pKeyValuesData, const char* szWeaponName );

public:
	float m_flFireDelay;
	float m_flSecondaryFireDelay;

	float m_flRecoil;
	float m_flSpread;
};