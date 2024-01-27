//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Multiplayer Player for HL1.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hl1mp_player.h"
#include "hl1mp_gamerules.h"
#include "team.h"
#include "ammodef.h"

// -------------------------------------------------------------------------------- //
// Player animation event. Sent to the client when a player fires, jumps, reloads, etc..
// -------------------------------------------------------------------------------- //

class CTEPlayerAnimEvent : public CBaseTempEntity
{
public:
	DECLARE_CLASS( CTEPlayerAnimEvent, CBaseTempEntity );
	DECLARE_SERVERCLASS();

	CTEPlayerAnimEvent( const char *name ) : CBaseTempEntity( name ) {}

	CNetworkHandle( CBasePlayer, m_hPlayer );
	CNetworkVar( int, m_iEvent );
	CNetworkVar( int, m_nData );
};

IMPLEMENT_SERVERCLASS_ST_NOBASE( CTEPlayerAnimEvent, DT_TEPlayerAnimEvent )
	SendPropEHandle( SENDINFO( m_hPlayer ) ),
	SendPropInt( SENDINFO( m_iEvent ), Q_log2( PLAYERANIMEVENT_COUNT ) + 1, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nData ), 32 )
END_SEND_TABLE()

static CTEPlayerAnimEvent g_TEPlayerAnimEvent( "PlayerAnimEvent" );

void TE_PlayerAnimEvent( CBasePlayer *pPlayer, PlayerAnimEvent_t event, int nData )
{
	CPVSFilter filter( (const Vector&)pPlayer->EyePosition() );

	//Tony; pull the player who is doing it out of the recipientlist, this is predicted!!
	/*filter.RemoveRecipient( pPlayer );*/

	g_TEPlayerAnimEvent.m_hPlayer = pPlayer;
	g_TEPlayerAnimEvent.m_iEvent = event;
	g_TEPlayerAnimEvent.m_nData = nData;
	g_TEPlayerAnimEvent.Create( filter, 0 );
}

void* SendProxy_SendNonLocalDataTable( const SendProp* pProp, const void* pStruct, const void* pVarData, CSendProxyRecipients* pRecipients, int objectID )
{
	pRecipients->SetAllRecipients();
	pRecipients->ClearRecipient( objectID - 1 );
	return (void*)pVarData;
}

// -------------------------------------------------------------------------------- //
// Tables.
// -------------------------------------------------------------------------------- //
LINK_ENTITY_TO_CLASS( player_mp, CHL1MP_Player );
PRECACHE_REGISTER( player_mp );

BEGIN_SEND_TABLE_NOBASE( CHL1MP_Player, DT_HL1MP_PLAYER_EXCLUSIVE )
	// send a hi-res origin to the local player for use in prediction
	SendPropVector( SENDINFO( m_vecOrigin ), -1, SPROP_NOSCALE | SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin ),
	SendPropFloat( SENDINFO_VECTORELEM( m_angEyeAngles, 0 ), 8, SPROP_CHANGES_OFTEN, -90.0f, 90.0f ),
END_SEND_TABLE()

BEGIN_SEND_TABLE_NOBASE( CHL1MP_Player, DT_HL1MP_PLAYER_NONEXCLUSIVE )
		// send a lo-res origin to other players
	SendPropVector( SENDINFO( m_vecOrigin ), -1, SPROP_COORD_MP_LOWPRECISION | SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin ),
	SendPropFloat( SENDINFO_VECTORELEM( m_angEyeAngles, 0 ), 8, SPROP_CHANGES_OFTEN, -90.0f, 90.0f ),
	SendPropAngle( SENDINFO_VECTORELEM( m_angEyeAngles, 1 ), 10, SPROP_CHANGES_OFTEN ),
END_SEND_TABLE()

IMPLEMENT_SERVERCLASS_ST( CHL1MP_Player, DT_HL1MP_PLAYER )
	SendPropExclude( "DT_BaseAnimating", "m_flPoseParameter" ),
	SendPropExclude( "DT_BaseAnimating", "m_flPlaybackRate" ),	
	SendPropExclude( "DT_BaseAnimating", "m_nSequence" ),
	SendPropExclude( "DT_BaseAnimating", "m_nNewSequenceParity" ),
	SendPropExclude( "DT_BaseAnimating", "m_nResetEventsParity" ),
	SendPropExclude( "DT_BaseEntity", "m_angRotation" ),
	SendPropExclude( "DT_BaseAnimatingOverlay", "overlay_vars" ),
	SendPropExclude( "DT_BaseEntity", "m_vecOrigin" ),
	
	// playeranimstate and clientside animation takes care of these on the client
	SendPropExclude( "DT_ServerAnimationData", "m_flCycle" ),	
	SendPropExclude( "DT_AnimTimeMustBeFirst", "m_flAnimTime" ),

	// Data that only gets sent to the local player.
	SendPropDataTable( "hl1mp_pLocal", 0, &REFERENCE_SEND_TABLE( DT_HL1MP_PLAYER_EXCLUSIVE ), SendProxy_SendLocalDataTable ),
	// Data that gets sent to all other players
	SendPropDataTable( "hl1mp_pNonLocal", 0, &REFERENCE_SEND_TABLE( DT_HL1MP_PLAYER_NONEXCLUSIVE ), SendProxy_SendNonLocalDataTable ),

	SendPropEHandle( SENDINFO( m_hRagdoll ) ),
	SendPropInt( SENDINFO( m_iSpawnInterpCounter ), 4 ),
	SendPropInt( SENDINFO( m_iRealSequence ), 9 ),
END_SEND_TABLE()

// -------------------------------------------------------------------------------- //
void cc_CreatePredictionError_f()
{
	CBaseEntity *pEnt = CBaseEntity::Instance( 1 );
	pEnt->SetAbsOrigin( pEnt->GetAbsOrigin() + Vector( 63, 0, 0 ) );
}

ConCommand cc_CreatePredictionError( "CreatePredictionError", cc_CreatePredictionError_f, "Create a prediction error", FCVAR_CHEAT );

void cc_GetTeam_f()
{
	CBasePlayer *pPlayer = UTIL_GetListenServerHost();
	Msg( "Player's team is %s.", pPlayer->TeamID() );
}

ConCommand cc_GetTeam( "getteam", cc_GetTeam_f, "Get player's team" );

void cc_GetTeams_f()
{
	for ( int i = 0; i < MAX_TEAMS; i++ )
	{
		CTeam *pTeam = GetGlobalTeam( i );
		if ( !pTeam || pTeam->TeamID()[0] == '\0' )
			continue;

		Warning( "Team: %s | %d\n", pTeam->TeamID(), i);
	}
}

ConCommand cc_GetTeams( "get_teams", cc_GetTeams_f, "Get global teams" );

extern int gEvilImpulse101;
static const char *s_szModelPath = "models/player/mp/";

//void CHL1MP_Player::SetupVisibility( CBaseEntity *pViewEntity, unsigned char *pvs, int pvssize )
//{
//	BaseClass::SetupVisibility( pViewEntity, pvs, pvssize );
//
//	int area = pViewEntity ? pViewEntity->NetworkProp()->AreaNum() : NetworkProp()->AreaNum();
//	PointCameraSetupVisibility( this, area, pvs, pvssize );
//}

CHL1MP_Player::CHL1MP_Player()
{
	m_PlayerAnimState = CreatePlayerAnimState( this );

	UseClientSideAnimation();

	m_angEyeAngles.Init();

	m_flNextModelChangeTime = 0;
	m_flNextTeamChangeTime = 0;
	m_iSpawnInterpCounter = 0;
	m_flDeathTime = 0;

	m_lifeState = LIFE_DEAD; // Start "dead".

	m_hRagdoll = NULL;
}

CHL1MP_Player::~CHL1MP_Player()
{
	m_PlayerAnimState->Release();
}

void CHL1MP_Player::PostThink( void )
{
	BaseClass::PostThink();

	// Keep the model upright; pose params will handle pitch aiming.
	QAngle angles = GetLocalAngles();
	angles[PITCH] = 0;
	SetLocalAngles( angles );

	m_angEyeAngles = EyeAngles();
	m_PlayerAnimState->Update( m_angEyeAngles[YAW], m_angEyeAngles[PITCH] );
}

void CHL1MP_Player::FireBullets( const FireBulletsInfo_t &info )
{
	lagcompensation->StartLagCompensation( this, this->GetCurrentCommand() );
	BaseClass::FireBullets( info );
	lagcompensation->FinishLagCompensation( this );
}

void CHL1MP_Player::Spawn( void )
{
	BaseClass::Spawn();

	RemoveEffects( EF_NODRAW );
	SetMoveType( MOVETYPE_WALK );
	RemoveSolidFlags( FSOLID_NOT_SOLID );

	// if no model, force one
	if ( !GetModelPtr() )
		SetModel( "models/player/mp/gordon/gordon.mdl" );

	if ( !HL1MPRules()->IsTeamplay() )
		SetPlayerModel();

	GiveDefaultItems();

	SetMoveType( MOVETYPE_WALK );

	m_flNextModelChangeTime = 0.0f;
	m_flNextTeamChangeTime = 0.0f;
	m_bHasLongJump = false;
	m_hRagdoll = NULL;
	m_iSpawnInterpCounter = (m_iSpawnInterpCounter + 1) % 8;

	pl.deadflag = false;
	m_lifeState = LIFE_ALIVE;
}

void CHL1MP_Player::DoAnimationEvent( PlayerAnimEvent_t event, int nData )
{
	m_PlayerAnimState->DoAnimationEvent( event, nData );
	TE_PlayerAnimEvent( this, event, nData );	// Send to any clients who can see this guy.
}

void CHL1MP_Player::GiveDefaultItems( void )
{
	GiveNamedItem( "weapon_crowbar" );
	GiveNamedItem( "weapon_glock" );

	CBasePlayer::GiveAmmo( 68, "9mmRound" );
}

void CHL1MP_Player::UpdateOnRemove( void )
{
	if ( m_hRagdoll )
	{
		UTIL_RemoveImmediate( m_hRagdoll );
		m_hRagdoll = NULL;
	}

	BaseClass::UpdateOnRemove();
}


void CHL1MP_Player::DetonateSatchelCharges( void )
{
	CBaseEntity *pSatchel = NULL;

	while ( (pSatchel = gEntList.FindEntityByClassname( pSatchel, "monster_satchel" )) != NULL )
	{
		if ( pSatchel->GetOwnerEntity() == this )
		{
			pSatchel->Use( this, this, USE_ON, 0 );
		}
	}
}

void CHL1MP_Player::Event_Killed( const CTakeDamageInfo &info )
{
	// Note: since we're dead, it won't draw us on the client, but we don't set EF_NODRAW
	// because we still want to transmit to the clients in our PVS.
	CreateRagdollEntity();

	DetonateSatchelCharges();
	PackDeadPlayerItems();

	BaseClass::Event_Killed( info );

	if ( info.GetDamageType() & DMG_DISSOLVE )
	{
		if ( m_hRagdoll )
		{
			m_hRagdoll->GetBaseAnimating()->Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
		}
	}

	FlashlightTurnOff();

	m_lifeState = LIFE_DEAD;

	RemoveEffects( EF_NODRAW );	// still draw player body
}

void CHL1MP_Player::PackDeadPlayerItems( void )
{
	int iAmmoRules = HL1MPRules()->DeadPlayerAmmo( this );
	int iWeaponRules = HL1MPRules()->DeadPlayerWeapons( this );

	if ( iWeaponRules == GR_PLR_DROP_GUN_NO && iAmmoRules == GR_PLR_DROP_AMMO_NO )
	{
		// nothing to pack. Remove the weapons and return. Don't call create on the box!
		RemoveAllItems( true );
		return;
	}

	CBaseCombatWeapon *pWeapon[MAX_WEAPONS];
	Q_memset( pWeapon, 0, sizeof pWeapon );

	int iAmmoIndex[MAX_AMMO_TYPES];
	Q_memset( iAmmoIndex, -1, sizeof iAmmoIndex );

	switch ( iWeaponRules )
	{
		case GR_PLR_DROP_GUN_ACTIVE:
			if ( GetActiveWeapon() != NULL )
				pWeapon[0] = GetActiveWeapon();
			break;

		case GR_PLR_DROP_GUN_ALL:
			for ( int i = 0; i < MAX_WEAPONS; i++ )
			{
				if ( !GetWeapon( i ) )
					continue;

				pWeapon[i] = GetWeapon( i );
			}
		break;
	}
	
	switch ( iAmmoRules )
	{
		case GR_PLR_DROP_AMMO_ALL:
			for ( int i = 0; i < MAX_AMMO_TYPES; i++ )
			{
				Ammo_t *pAmmo = GetAmmoDef()->GetAmmoOfIndex( i );
				if ( !pAmmo )
					continue;

				iAmmoIndex[i] = GetAmmoDef()->Index( pAmmo->pName );
			}
		break;

		case GR_PLR_DROP_AMMO_ACTIVE:
		{
			if ( GetActiveWeapon() != NULL )
			{
				int index1 = GetActiveWeapon()->GetPrimaryAmmoType();
				int index2 = GetActiveWeapon()->GetSecondaryAmmoType();

				if ( index1 >= 0 )
					iAmmoIndex[0] = index1;
				if ( index2 >= 0 )
					iAmmoIndex[1] = index2;
			}
		}
		break;
	}

	CWpnBox *pBox = (CWpnBox *)Create( "w_weaponbox", GetAbsOrigin(), vec3_angle, this );
	
	QAngle qAng = GetAbsAngles();
	qAng.x = 0;
	qAng.y = 0;

	pBox->SetAbsAngles( qAng );

	/*if ( iAmmoRules == GR_PLR_DROP_AMMO_ALL )
	{
		for ( int i = 0; i < ARRAYSIZE( iAmmoIndex ); i++ )
		{
			if ( iAmmoIndex[i] == -1 )
				continue;

			pBox->AddAmmo( base_t( GetAmmoDef()->GetAmmoOfIndex( iAmmoIndex[i] )->pName, GetAmmoCount( iAmmoIndex[i] ) ) );
		}
	}
	else
	{
		if ( iAmmoIndex[0] >= 0 )
		{
			pBox->AddAmmo( base_t( GetAmmoDef()->GetAmmoOfIndex( iAmmoIndex[0] )->pName, GetAmmoCount( iAmmoIndex[0] ) ) );
		
			if ( iAmmoIndex[1] >= 0 )
				pBox->AddAmmo( base_t( GetAmmoDef()->GetAmmoOfIndex( iAmmoIndex[1] )->pName, GetAmmoCount( iAmmoIndex[2] ) ) );
	}*/

	for ( int i = 0; i < ARRAYSIZE( iAmmoIndex ) && iAmmoIndex[i] >= 0; i++ )
	{
		pBox->AddAmmo( base_t( GetAmmoDef()->GetAmmoOfIndex( iAmmoIndex[i] )->pName, GetAmmoCount( iAmmoIndex[i] ) ) );
	}

	for ( int i = 0; i < ARRAYSIZE( pWeapon ) && pWeapon[i] != nullptr; i++ )
	{
		pBox->AddWeapon( pWeapon[i] );
		Weapon_Detach( pWeapon[i] );
	}

	DevMsg( "Packed guns: %s.\n", iWeaponRules > 7 ? "active" : "all" );
	DevMsg( "Packed ammo: %s.\n", iAmmoRules > 10 ? "active" : "all" );
	
	RemoveAllItems( true );
}

void CHL1MP_Player::SetAnimation( PLAYER_ANIM playerAnim )
{
	if ( playerAnim == PLAYER_ATTACK1 )
	{
		DoAnimationEvent( PLAYERANIMEVENT_FIRE_GUN );
	}

	int animDesired = 0;
	char szAnim[64];

	float speed = GetAbsVelocity().Length2D();

	if ( GetFlags() & (FL_FROZEN | FL_ATCONTROLS) )
	{
		speed = 0;
		playerAnim = PLAYER_IDLE;
	}

	if ( playerAnim == PLAYER_ATTACK1 )
	{
		if ( speed > 0 )
		{
			playerAnim = PLAYER_WALK;
		}
		else
		{
			playerAnim = PLAYER_IDLE;
		}
	}

	Activity idealActivity = ACT_WALK;// TEMP!!!!!

	// This could stand to be redone. Why is playerAnim abstracted from activity? (sjb)
	if ( playerAnim == PLAYER_JUMP )
	{
		idealActivity = ACT_HOP;
	}
	else if ( playerAnim == PLAYER_SUPERJUMP )
	{
		idealActivity = ACT_LEAP;
	}
	else if ( playerAnim == PLAYER_DIE )
	{
		if ( m_lifeState == LIFE_ALIVE )
		{
			idealActivity = ACT_DIERAGDOLL;
		}
	}
	else if ( playerAnim == PLAYER_ATTACK1 )
	{
		if ( GetActivity() == ACT_HOVER ||
			GetActivity() == ACT_SWIM ||
			GetActivity() == ACT_HOP ||
			GetActivity() == ACT_LEAP ||
			GetActivity() == ACT_DIESIMPLE )
		{
			idealActivity = GetActivity();
		}
		else
		{
			idealActivity = ACT_RANGE_ATTACK1;
		}
	}
	else if ( playerAnim == PLAYER_IDLE || playerAnim == PLAYER_WALK )
	{
		if ( !(GetFlags() & FL_ONGROUND) && (GetActivity() == ACT_HOP || GetActivity() == ACT_LEAP) )	// Still jumping
		{
			idealActivity = GetActivity();
		}
		else if ( GetWaterLevel() > 1 )
		{
			if ( speed == 0 )
				idealActivity = ACT_HOVER;
			else
				idealActivity = ACT_SWIM;
		}
		else if ( speed > 0 )
		{
			idealActivity = ACT_WALK;
		}
		else
		{
			idealActivity = ACT_IDLE;
		}
	}

	if ( idealActivity == ACT_RANGE_ATTACK1 )
	{
		if ( GetFlags() & FL_DUCKING )	// crouching
		{
			Q_strncpy( szAnim, "crouch_shoot_", sizeof( szAnim ) );
		}
		else
		{
			Q_strncpy( szAnim, "ref_shoot_", sizeof( szAnim ) );
		}
		Q_strncat( szAnim, m_szAnimExtension, sizeof( szAnim ), COPY_ALL_CHARACTERS );
		animDesired = LookupSequence( szAnim );
		if ( animDesired == -1 )
			animDesired = 0;

		if ( GetSequence() != animDesired || !SequenceLoops() )
		{
			SetCycle( 0 );
		}

		// Tracker 24588:  In single player when firing own weapon this causes eye and punchangle to jitter
		/*if ( !SequenceLoops() )
		{
			IncrementInterpolationFrame();
		}*/

		SetActivity( idealActivity );
		ResetSequence( animDesired );
	}
	else if ( idealActivity == ACT_IDLE )
	{
		if ( GetFlags() & FL_DUCKING )
		{
			animDesired = LookupSequence( "crouch_idle" );
		}
		else
		{
			animDesired = LookupSequence( "look_idle" );
		}
		if ( animDesired == -1 )
			animDesired = 0;

		SetActivity( ACT_IDLE );
	}
	else if ( idealActivity == ACT_WALK )
	{
		if ( GetFlags() & FL_DUCKING )
		{
			animDesired = SelectWeightedSequence( ACT_CROUCH );
			SetActivity( ACT_CROUCH );
		}
		else
		{
			animDesired = SelectWeightedSequence( ACT_RUN );
			SetActivity( ACT_RUN );
		}

	}
	else
	{
		if ( GetActivity() == idealActivity )
			return;

		SetActivity( idealActivity );

		animDesired = SelectWeightedSequence( GetActivity() );

		// Already using the desired animation?
		if ( GetSequence() == animDesired )
			return;

		m_iRealSequence = animDesired;
		ResetSequence( animDesired );
		SetCycle( 0 );
		return;
	}

	// Already using the desired animation?
	if ( GetSequence() == animDesired )
		return;

	m_iRealSequence = animDesired;

	//Msg( "Set animation to %d\n", animDesired );
	// Reset to first frame of desired animation
	ResetSequence( animDesired );
	SetCycle( 0 );

	DevMsg("Playing sequence %s\n", GetSequenceName(animDesired));
}

static ConVar sv_debugweaponpickup( "sv_debugweaponpickup", "0", FCVAR_CHEAT, "Prints descriptive reasons as to why pickup did not work." );

// correct respawning of weapons
bool CHL1MP_Player::BumpWeapon( CBaseCombatWeapon *pWeapon )
{
	CBaseCombatCharacter *pOwner = pWeapon->GetOwner();

	// Can I have this weapon type?
	if ( !IsAllowedToPickupWeapons() )
	{
		if ( sv_debugweaponpickup.GetBool() )
			Msg( "sv_debugweaponpickup: IsAllowedToPickupWeapons() returned false\n" );

		return false;
	}

	if ( pOwner || !Weapon_CanUse( pWeapon ) || !g_pGameRules->CanHavePlayerItem( this, pWeapon ) )
	{
		if ( sv_debugweaponpickup.GetBool() && pOwner )
			Msg( "sv_debugweaponpickup: pOwner\n" );

		if ( sv_debugweaponpickup.GetBool() && !Weapon_CanUse( pWeapon ) )
			Msg( "sv_debugweaponpickup: Can't use weapon\n" );

		if ( sv_debugweaponpickup.GetBool() && !g_pGameRules->CanHavePlayerItem( this, pWeapon ) )
			Msg( "sv_debugweaponpickup: Gamerules says player can't have item\n" );

		if ( gEvilImpulse101 )
		{
			UTIL_Remove( pWeapon );
		}
		return false;
	}

	// Don't let the player fetch weapons through walls (use MASK_SOLID so that you can't pickup through windows)
	if ( !pWeapon->FVisible( this, MASK_SOLID ) && !(GetFlags() & FL_NOTARGET) )
	{
		if ( sv_debugweaponpickup.GetBool() && !FVisible( this, MASK_SOLID ) )
			Msg( "sv_debugweaponpickup: Can't fetch weapon through a wall\n" );

		if ( sv_debugweaponpickup.GetBool() && !(GetFlags() & FL_NOTARGET) )
			Msg( "sv_debugweaponpickup: NoTarget\n" );

		return false;
	}

	bool bOwnsWeaponAlready = Weapon_OwnsThisType( pWeapon->GetClassname(), pWeapon->GetSubType() ) != NULL;

	if ( bOwnsWeaponAlready == true )
	{
		//If we have room for the ammo, then "take" the weapon too.
		if ( Weapon_EquipAmmoOnly( pWeapon ) )
		{
			pWeapon->CheckRespawn();

			UTIL_Remove( pWeapon );

			if ( sv_debugweaponpickup.GetBool() )
				Msg( "sv_debugweaponpickup: Picking up weapon\n" );

			return true;
		}
		else
		{
			if ( sv_debugweaponpickup.GetBool() )
				Msg( "sv_debugweaponpickup: Owns weapon already\n" );

			return false;
		}
	}

	pWeapon->CheckRespawn();
	Weapon_Equip( pWeapon );

	if ( sv_debugweaponpickup.GetBool() )
		Msg( "sv_debugweaponpickup: Picking up weapon\n" );

	return true;
}

void CHL1MP_Player::SetPlayerModel( void )
{
	char szBaseName[ 16 ];
	V_snprintf( szBaseName, 16, "%s", engine->GetClientConVarValue( entindex(), "cl_playermodel" ) );

	// Don't let it be 'none'; default to Barney
	if ( V_stricmp( "none", szBaseName ) == 0 )
	{
		V_strcpy( szBaseName, "gordon" );
	}

	char szModelName[ 64 ];
	V_snprintf( szModelName, 64, "%s%s/%s.mdl", s_szModelPath, szBaseName, szBaseName );

	// Check to see if the model was properly precached, do not error out if not.
	int i = modelinfo->GetModelIndex( szModelName );
	if ( i == -1 )
	{
		SetModel( "models/player/mp/gordon/gordon.mdl" );
		engine->ClientCommand( edict(), "cl_playermodel gordon\n" );
		return;
	}

	SetModel( szModelName );

	m_flNextModelChangeTime = gpGlobals->curtime + 5;
}

void CHL1MP_Player::SetPlayerTeamModel( char *sz )
{
}

// -------------------------------------------------------------------------------- //
// Ragdoll entities.
// -------------------------------------------------------------------------------- //

class CHL1MPRagdoll : public CBaseAnimatingOverlay
{
public:
	DECLARE_CLASS( CHL1MPRagdoll, CBaseAnimatingOverlay );
	DECLARE_SERVERCLASS();

	// Transmit ragdolls to everyone.
	virtual int UpdateTransmitState()
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

public:
	// In case the client has the player entity, we transmit the player index.
	// In case the client doesn't have it, we transmit the player's model index, origin, and angles
	// so they can create a ragdoll in the right place.
	CNetworkHandle( CBaseEntity, m_hPlayer );	// networked entity handle 
	CNetworkVector( m_vecRagdollVelocity );
	CNetworkVector( m_vecRagdollOrigin );
};

LINK_ENTITY_TO_CLASS( hl1mp_ragdoll, CHL1MPRagdoll );

IMPLEMENT_SERVERCLASS_ST_NOBASE( CHL1MPRagdoll, DT_HL1MPRagdoll )
SendPropVector( SENDINFO( m_vecRagdollOrigin ), -1, SPROP_COORD ),
SendPropEHandle( SENDINFO( m_hPlayer ) ),
SendPropModelIndex( SENDINFO( m_nModelIndex ) ),
SendPropInt( SENDINFO( m_nForceBone ), 8, 0 ),
SendPropVector( SENDINFO( m_vecForce ), -1, SPROP_NOSCALE ),
SendPropVector( SENDINFO( m_vecRagdollVelocity ) )
END_SEND_TABLE()


void CHL1MP_Player::CreateRagdollEntity( void )
{
	// If we already have a ragdoll, don't make another one.    
	CHL1MPRagdoll *pRagdoll = dynamic_cast<CHL1MPRagdoll*>(m_hRagdoll.Get());

	if ( !pRagdoll )
	{
		// Create a new one
		pRagdoll = dynamic_cast<CHL1MPRagdoll*>(CreateEntityByName( "hl1mp_ragdoll" ));
	}

	if ( pRagdoll )
	{
		pRagdoll->m_hPlayer = this;
		pRagdoll->m_vecRagdollOrigin = GetAbsOrigin();
		pRagdoll->m_vecRagdollVelocity = GetAbsVelocity();
		pRagdoll->m_nModelIndex = m_nModelIndex;
		pRagdoll->m_nForceBone = m_nForceBone;
		pRagdoll->SetAbsOrigin( GetAbsOrigin() );
	}

	m_hRagdoll = pRagdoll;
}

void CHL1MP_Player::CreateCorpse( void )
{
}