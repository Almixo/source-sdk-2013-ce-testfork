//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================// 

#ifndef HL1MP_PLAYER_H
#define HL1MP_PLAYER_H
#pragma once

#include "basemultiplayerplayer.h"
#include "hl1/hl1_player_shared.h"
#include "hl1_player.h"
#include "takedamageinfo.h"
#include "ilagcompensationmanager.h"
#include "custom/CWeaponBox.h"

//=============================================================================
// >> HL1MP_Player
//=============================================================================
class CHL1MP_Player : public CHL1_Player
{
	DECLARE_CLASS( CHL1MP_Player, CHL1_Player );
	DECLARE_SERVERCLASS( );
public:
	CHL1MP_Player( );
	~CHL1MP_Player( void );

	void Spawn( void );
	void PostThink( void );
	void Event_Killed( const CTakeDamageInfo &info );
	void DetonateSatchelCharges( void );
	void PackDeadPlayerItems( );
	void FireBullets( const FireBulletsInfo_t &info );
	void GiveDefaultItems( );

	/*void SetupVisibility( CBaseEntity *pViewEntity, unsigned char *pvs, int pvssize );*/
	
	void CreateRagdollEntity( void );
	void UpdateOnRemove( void );
	bool BecomeRagdollOnClient( const Vector &force ) { return true; };
	void CreateCorpse( void );

	void SetAnimation( PLAYER_ANIM playerAnim );
	void DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 );

	void SetPlayerModel( );
	void SetPlayerTeamModel( const char *szTeamName );
	float GetNextModelChangeTime( void ) { return m_flNextModelChangeTime; }
	float GetNextTeamChangeTime( void ) { return m_flNextTeamChangeTime; }

	bool BumpWeapon( CBaseCombatWeapon *pWeapon );

	bool StartObserverMode( int mode ) { return false; }

	CNetworkVar( int, m_iRealSequence );
private:
	CNetworkHandle( CBaseEntity, m_hRagdoll );
	CNetworkVar( int, m_iSpawnInterpCounter );
	CNetworkQAngle( m_angEyeAngles );

	IHL1MPPlayerAnimState*		m_PlayerAnimState;
	float						m_flNextModelChangeTime;
	float						m_flNextTeamChangeTime;
};

inline CHL1MP_Player *ToHL1MPPlayer( CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer( ) )
		return NULL;

	return dynamic_cast< CHL1MP_Player* >( pEntity );
}


#endif //HL1MP_PLAYER_H
