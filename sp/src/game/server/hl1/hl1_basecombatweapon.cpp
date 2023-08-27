//====== Copyright © 1996-2003, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================


#include "cbase.h"
#include "hl1_basecombatweapon_shared.h"
#include "te_effect_dispatch.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseHL1CombatWeapon::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound( "Item.Pickup" );
	PrecacheScriptSound( "BaseCombatWeapon.WeaponDrop" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseHL1CombatWeapon::FallInit( void )
{
	SetModel( GetWorldModel() );
	SetSize( Vector( 0, 0, 0 ), Vector( 0, 0, 0 ) );	// Pointsize until it lands on the ground
	SetMoveType( MOVETYPE_FLYGRAVITY );
	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_TRIGGER );
	AddSolidFlags( FSOLID_NOT_SOLID );

	SetPickupTouch();
	
	SetThink( &CBaseHL1CombatWeapon::FallThink );

	SetNextThink( gpGlobals->curtime + 0.1f );
}

//-----------------------------------------------------------------------------
// Purpose: Items that have just spawned run this think to catch them when 
//			they hit the ground. Once we're sure that the object is grounded, 
//			we change its solid type to trigger and set it in a large box that 
//			helps the player get it.
//-----------------------------------------------------------------------------
void CBaseHL1CombatWeapon::FallThink ( void )
{
	SetNextThink( gpGlobals->curtime + 0.1f );

	if ( CBaseEntity::GetFlags() & FL_ONGROUND )
	{
		// clatter if we have an owner (i.e., dropped by someone)
		// don't clatter if the gun is waiting to respawn (if it's waiting, it is invisible!)
		if ( GetOwnerEntity() )
		{
			CPASAttenuationFilter filter(this);
			EmitSound( filter, entindex(), "BaseCombatWeapon.WeaponDrop" );
		}

		// lie flat
		QAngle ang = GetAbsAngles();
		ang.x = 0;
		ang.z = 0;
		SetAbsAngles( ang );

		Materialize(); 
	}
}

void CBaseHL1CombatWeapon::EjectShell( CBaseEntity *pPlayer, int iType )
{
	QAngle angShellAngles = pPlayer->GetAbsAngles();

	Vector vecForward, vecRight, vecUp;
	AngleVectors(angShellAngles, &vecForward, &vecRight, &vecUp);

	Vector vecShellPosition = pPlayer->GetAbsOrigin() + pPlayer->GetViewOffset();
	switch (iType)
	{
	case 0:
	default:
		vecShellPosition += vecRight * 4;
		vecShellPosition += vecUp * -12;
		vecShellPosition += vecForward * 20;
		break;
	case 1:
		vecShellPosition += vecRight * 6;
		vecShellPosition += vecUp * -12;
		vecShellPosition += vecForward * 32;
		break;
	}

	Vector vecShellVelocity = vec3_origin;
	vecShellVelocity += vecRight * RandomFloat( 50, 70 );
	vecShellVelocity += vecUp * RandomFloat( 100, 150 );
	vecShellVelocity += vecForward * 25;

	angShellAngles.x = 0;
	angShellAngles.z = 0;

	CEffectData	data;
	data.m_vStart = vecShellVelocity;
	data.m_vOrigin = vecShellPosition;
	data.m_vAngles = angShellAngles;
	data.m_fFlags = iType;

	DispatchEffect("HL1ShellEject", data);
}