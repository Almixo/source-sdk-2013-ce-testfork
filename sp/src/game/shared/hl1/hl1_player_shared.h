//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef HL1_PLAYER_SHARED_H
#define HL1_PLAYER_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "Base_PlayerAnimState.h"

// Shared header file for players
#if defined( CLIENT_DLL )
#define CHL1_Player C_HL1_Player	//FIXME: Lovely naming job between server and client here...
#include "hl1_c_player.h"
#else
#include "hl1_player.h"
#endif

#include "studio.h"

enum PlayerAnimEvent_t
{
	PLAYERANIMEVENT_FIRE_GUN=0,
	PLAYERANIMEVENT_THROW_GRENADE,
	PLAYERANIMEVENT_JUMP,
	PLAYERANIMEVENT_RELOAD,
	PLAYERANIMEVENT_DIE,
	
	PLAYERANIMEVENT_COUNT
};

class IHL1MPPlayerAnimState : virtual public IPlayerAnimState
{
public:
	// This is called by both the client and the server in the same way to trigger events for
	// players firing, jumping, throwing grenades, etc.
	virtual void DoAnimationEvent( PlayerAnimEvent_t event, int nData ) = 0;
};

IHL1MPPlayerAnimState* CreatePlayerAnimState( CHL1_Player *pPlayer );

// If this is set, then the game code needs to make sure to send player animation events
// to the local player if he's the one being watched.
extern ConVar cl_showanimstate;

#endif // HL1_PLAYER_SHARED_H
