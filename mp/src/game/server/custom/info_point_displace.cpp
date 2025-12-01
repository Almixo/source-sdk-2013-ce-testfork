#include "cbase.h"
#include "info_point_displace.h"

#define VECTORMIN Vector( -48, -48, -48 )	// point_displace size
#define VECTORMAX Vector( 48, 48, 48 )		// point_displace size

void CDispPoint::Spawn(void)
{
	BaseClass::Spawn();

	SetMoveType( MOVETYPE_NONE );		//we don't want it to move
	SetSolid( SOLID_NONE );				//not solid
	SetSize( VECTORMIN, VECTORMAX );	//set size

	Vector vOrg = GetAbsOrigin();
	DevWarning("%s spawned at %g %g %g!\n", GetDebugName(), vOrg.x, vOrg.y, vOrg.z);	//print our pos

	SetNextThink( TICK_NEVER_THINK );	//never think
}