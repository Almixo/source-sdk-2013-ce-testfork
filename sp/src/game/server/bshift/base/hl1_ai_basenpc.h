//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Base combat character with no AI
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#ifndef HL1_AI_BASENPC_H
#define HL1_AI_BASENPC_H

#ifdef _WIN32
#pragma once
#endif


#include "ai_basenpc.h"
#include "ai_default.h"
#include "ai_task.h"
#include "ai_schedule.h"
#include "ai_node.h"
#include "ai_hull.h"
#include "ai_hint.h"
#include "ai_memory.h"
#include "ai_route.h"
#include "ai_motor.h"
#include "ai_squad.h"
#include "ai_senses.h"
#include "npcevent.h"
//=============================================================================
// >> CHL1NPCTalker
//=============================================================================

class CHL1BaseNPC : public CAI_BaseNPC
{
	DECLARE_CLASS( CHL1BaseNPC, CAI_BaseNPC );
	
public:
	CHL1BaseNPC( void )
	{

	}

	void	TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator );
	bool	ShouldGib( const CTakeDamageInfo &info );
	bool	CorpseGib( const CTakeDamageInfo &info );

	bool	HasAlienGibs( void );
	bool	HasHumanGibs( void );

	void	Precache( void );

	int		IRelationPriority( CBaseEntity *pTarget );
	bool	NoFriendlyFire( void );

	void	EjectShell( const Vector &vecOrigin, const Vector &vecVelocity, float rotation, int iType );

	virtual int		SelectDeadSchedule();
};

#endif //HL1_AI_BASENPC_H
