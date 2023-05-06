#include "cbase.h"
#include "hl1mp_gamerules.h"
#include "viewport_panel_names.h"
#include "gameeventdefs.h"
#include <KeyValues.h>
#include "ammodef.h"

#ifdef CLIENT_DLL

#include "hl1/c_hl1mp_player.h"

#else

#include "eventqueue.h"
#include "player.h"
#include "gamerules.h"
#include "game.h"
#include "items.h"
#include "entitylist.h"
#include "in_buttons.h"
#include <ctype.h>
#include "voice_gamemgr.h"
#include "iscorer.h"
#include "hl1mp_player.h"
#include "team.h"
#include "voice_gamemgr.h"

#ifdef DEBUG	
#include "hl1mp_bot_temp.h"
#endif

ConVar sv_hl1mp_weapon_respawn_time( "sv_hl1mp_weapon_respawn_time", "20", FCVAR_GAMEDLL | FCVAR_NOTIFY );
ConVar sv_hl1mp_item_respawn_time( "sv_hl1mp_item_respawn_time", "30", FCVAR_GAMEDLL | FCVAR_NOTIFY );


extern ConVar mp_chattime;
#endif


REGISTER_GAMERULES_CLASS( CHL1MPRules );

BEGIN_NETWORK_TABLE_NOBASE( CHL1MPRules, DT_HL1MPRules )

	#ifdef CLIENT_DLL
		RecvPropBool( RECVINFO( m_bTeamPlayEnabled ) ),
	#else
		SendPropBool( SENDINFO( m_bTeamPlayEnabled ) ),
	#endif

END_NETWORK_TABLE()


LINK_ENTITY_TO_CLASS( hl1mp_gamerules, CHL1MPGameRulesProxy );
IMPLEMENT_NETWORKCLASS_ALIASED( HL1MPGameRulesProxy, DT_HL1MPGameRulesProxy )


#ifdef CLIENT_DLL
	void RecvProxy_HL1MPRules( const RecvProp *pProp, void **pOut, void *pData, int objectID )
	{
		CHL1MPRules *pRules = HL1MPRules();
		Assert( pRules );
		*pOut = pRules;
	}

	BEGIN_RECV_TABLE( CHL1MPGameRulesProxy, DT_HL1MPGameRulesProxy )
		RecvPropDataTable( "hl1mp_gamerules_data", 0, 0, &REFERENCE_RECV_TABLE( DT_HL1MPRules ), RecvProxy_HL1MPRules )
	END_RECV_TABLE()
#else
	void* SendProxy_HL1MPRules( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
	{
		CHL1MPRules *pRules = HL1MPRules();
		Assert( pRules );
		return pRules;
	}

	BEGIN_SEND_TABLE( CHL1MPGameRulesProxy, DT_HL1MPGameRulesProxy )
		SendPropDataTable( "hl1mp_gamerules_data", 0, &REFERENCE_SEND_TABLE( DT_HL1MPRules ), SendProxy_HL1MPRules )
	END_SEND_TABLE()
#endif

#ifndef CLIENT_DLL

#if 0
	class CVoiceGameMgrHelper : public IVoiceGameMgrHelper
	{
	public:
		virtual bool		CanPlayerHearPlayer( CBasePlayer *pListener, CBasePlayer *pTalker )
		{
			return ( pListener->GetTeamNumber() == pTalker->GetTeamNumber() );
		}
	};
	CVoiceGameMgrHelper g_VoiceGameMgrHelper;
	IVoiceGameMgrHelper *g_pVoiceGameMgrHelper = &g_VoiceGameMgrHelper;
#endif

#ifdef DEBUG

	// Handler for the "bot" command.
	void Bot_f()
	{		
		// Look at -count.
		int count = 1;
		count = clamp( count, 1, 16 );

		int iTeam = 0;

		// Look at -frozen.
		bool bFrozen = false;

		// Ok, spawn all the bots.
		while ( --count >= 0 )
		{
			BotPutInServer( bFrozen, iTeam );
		}
	}


	ConCommand cc_Bot( "bot", Bot_f, "Add a bot.", FCVAR_CHEAT );

#endif

#endif


CHL1MPRules::CHL1MPRules()
{
#ifndef CLIENT_DLL
	m_bTeamPlayEnabled = teamplay.GetBool();

	if (IsTeamplay() == true)
	{
// --------------- Create basic server teams --------------- 

		CTeam* pTeam = static_cast<CTeam*>(CreateEntityByName("team_manager"));
		pTeam->Init("Unassigned", 0);
		g_Teams.AddToTail(pTeam);

		pTeam = static_cast<CTeam*>(CreateEntityByName("team_manager"));
		pTeam->Init("Spectator", 1);
		g_Teams.AddToTail(pTeam);

		char szTeamList[32];
		V_FileBase(teamlist.GetString(), szTeamList, 32);		

		int teamnum = 2;
		char* szTeamName = strtok(szTeamList, ";");

		while (szTeamName != NULL)
		{
			CTeam *pTeam = static_cast<CTeam*>(CreateEntityByName("team_manager"));
			pTeam->Init(szTeamName, teamnum);
			g_Teams.AddToTail(pTeam);

			teamnum++;

			szTeamName = strtok(NULL, "");
		}

		if (GetNumberOfTeams() < 4)
		{
			CTeam* pTeam = static_cast<CTeam*>(CreateEntityByName("team_manager"));
			pTeam->Init("robo", 2);
			g_Teams.InsertBefore(3, pTeam);

			pTeam = static_cast<CTeam*>(CreateEntityByName("team_manager"));
			pTeam->Init("hgrunt", 3);
			g_Teams.InsertBefore(4, pTeam);
		}

		for (int i = 0; i < MAX_TEAMS; i++)
		{
			CTeam* pTeam = GetGlobalTeam(i);
			if (pTeam == nullptr)
				continue;

			DevWarning("Team : %s.\n", pTeam->GetName());
		}
	}
#endif
}


CHL1MPRules::~CHL1MPRules( void )
{
#ifndef CLIENT_DLL
	// Note, don't delete each team since they are in the gEntList and will 
	// automatically be deleted from there, instead.
	g_Teams.Purge();
#endif
}

void CHL1MPRules::CreateStandardEntities( void )
{

#ifndef CLIENT_DLL
	// Create the entity that will send our data to the client.

	BaseClass::CreateStandardEntities();

#ifdef _DEBUG
	CBaseEntity *pEnt = 
#endif
	CBaseEntity::Create( "hl1mp_gamerules", vec3_origin, vec3_angle );
	Assert( pEnt );
#endif
}

float CHL1MPRules::GetAmmoDamage( CBaseEntity *pAttacker, CBaseEntity *pVictim, int nAmmoType )
{
    return BaseClass::GetAmmoDamage( pAttacker, pVictim, nAmmoType ) * GetDamageMultiplier();
}

float CHL1MPRules::GetDamageMultiplier( void )
{
    if ( IsMultiplayer() )
        return sk_mp_dmg_multiplier.GetFloat();
    else
        return 1.0f;
}



#ifndef CLIENT_DLL


void CHL1MPRules::Think ( void )
{
	CGameRules::Think();

	if ( g_fGameOver )   // someone else quit the game already
	{
		// check to see if we should change levels now
		if ( m_flIntermissionEndTime < gpGlobals->curtime )
		{
			ChangeLevel(); // intermission is over
		}

		return;
	}

	float flTimeLimit = mp_timelimit.GetFloat() * 60;
	float flFragLimit = fraglimit.GetFloat();
	
	if ( flTimeLimit != 0 && gpGlobals->curtime >= flTimeLimit )
	{
		GoToIntermission();
		return;
	}

	if ( flFragLimit )
	{
		if( IsTeamplay() == true )
		{
			for (int i = TEAM_SPECTATOR+1; i < GetNumberOfTeams(); i++)
			{
				if ( GetGlobalTeam(i)->GetScore() >= flFragLimit )
				{
					GoToIntermission();
					return;
				}
			}
		}
		else
		{
			// check if any player is over the frag limit
			for ( int i = 1; i <= gpGlobals->maxClients; i++ )
			{
				CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );

				if ( pPlayer && pPlayer->FragCount() >= flFragLimit )
				{
					GoToIntermission();
					return;
				}
			}
		}
	}

//	ManageObjectRelocation();
}


void CHL1MPRules::GoToIntermission()
{
	if ( g_fGameOver )
		return;

	g_fGameOver = true;

	m_flIntermissionEndTime = gpGlobals->curtime + mp_chattime.GetInt();

	for ( int i = 0; i < MAX_PLAYERS; i++ )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );

		if ( !pPlayer )
			continue;

		pPlayer->ShowViewPortPanel( PANEL_SCOREBOARD );
		pPlayer->AddFlag( FL_FROZEN );
	}
}


void CHL1MPRules::ClientSettingsChanged(CBasePlayer *pPlayer)
{
	CHL1MP_Player *pHL1Player = ToHL1MPPlayer(pPlayer);
	if (!pHL1Player)
		return;

	SetPlayerTeam(pHL1Player);

	if (IsTeamplay())
	{		
		char szMDL[64];
		V_snprintf(szMDL, 64, "models/player/mp/%s/%s.mdl", pHL1Player->TeamID(), pHL1Player->TeamID());

		UTIL_SetModel(pHL1Player, szMDL);

		char szAccMDL[16];
		V_FileBase(modelinfo->GetModelName(pHL1Player->GetModel()), szAccMDL, 16);

		char szPrtMDL[48];
		V_snprintf(szPrtMDL, 48, "* Your model is '%s'.", szAccMDL);

		ClientPrint(pHL1Player, HUD_PRINTTALK, szPrtMDL);
	}
	else
	{
		char szCurrMDL[32];
		V_FileBase(modelinfo->GetModelName(pHL1Player->GetModel()), szCurrMDL, 32);

		char szAccMDL[32];
		V_FileBase(engine->GetClientConVarValue(pHL1Player->entindex(), "cl_playermodel"), szAccMDL, 32);

		if (!FStrEq(szCurrMDL, szAccMDL))
		{
			if (pHL1Player->GetNextModelChangeTime() >= gpGlobals->curtime)
			{
				engine->ClientCommand(pHL1Player->edict(), "cl_playermodel %s\n", szCurrMDL);

				int iTimeLeft = pHL1Player->GetNextModelChangeTime() - gpGlobals->curtime;
				char szPrint[64];
				V_snprintf(szPrint, 64, "Please wait %d more seconds before trying to switch.\n", iTimeLeft);

				ClientPrint(pHL1Player, HUD_PRINTTALK, szPrint);
			}
			else
			{
				pHL1Player->SetPlayerModel();
			}
		}
	}

	BaseClass::ClientSettingsChanged(pHL1Player);
}

void CHL1MPRules::SetPlayerTeam(CBasePlayer *pPlayer)
{
	if (!IsTeamplay())	//isn't teamplay, don't do anything... the Source will figure the team out somehow
		return;

	if (pPlayer->GetTeamNumber() > 1)	//we already have a team, don't change it again!
		return;

	CHL1MP_Player *pHL1Player = ToHL1MPPlayer(pPlayer);
	if (!pHL1Player)
		return;

	const char *szTeamName = TeamWithFewestPlayers();

	pHL1Player->ChangeTeam( GetTeamIndex(szTeamName) );

	char out[32];
	V_snprintf(out, sizeof(out), "* You are on team '%s.'", szTeamName);

	ClientPrint(pHL1Player, HUD_PRINTTALK, out);
}

const char* CHL1MPRules::TeamWithFewestPlayers()
{
	char* szTeamName = nullptr;
	
	CTeam *pTeam1 = GetGlobalTeam(2);
	CTeam *pTeam2 = GetGlobalTeam(3);

	int iPlr1 = pTeam1->GetNumPlayers();
	int iPlr2 = pTeam2->GetNumPlayers();

	if (iPlr1 == iPlr2)
	{
		int i = RandomInt(2, 3);

		if (i == 2)
			szTeamName = (char*)pTeam1->GetName();
		if (i == 3)
			szTeamName = (char*)pTeam2->GetName();
	}
	else
	{
		if (iPlr1 > iPlr2)
			szTeamName = (char*)pTeam2->GetName();
		if (iPlr1 < iPlr2)
			szTeamName = (char*)pTeam1->GetName();
	}

	return szTeamName;
} 

int CHL1MPRules::GetTeamIndex(const char *pName)
{
	if (pName && pName != nullptr)
	{
		for (int i = 0; i < GetNumberOfTeams(); i++)
		{
			if (FStrEq(g_Teams[i]->GetName(), pName))
				return i;
		}
	}

	return -1; // no match
}

float CHL1MPRules::FlWeaponRespawnTime( CBaseCombatWeapon *pWeapon )
{
	if ( weaponstay.GetInt() > 0 )
	{
		// make sure it's only certain weapons
		if ( !(pWeapon->GetWeaponFlags() & ITEM_FLAG_LIMITINWORLD) )
		{
			return 0;
		}
	}

	return sv_hl1mp_weapon_respawn_time.GetInt();
}

float CHL1MPRules::FlItemRespawnTime( CItem *pItem )
{
	return sv_hl1mp_item_respawn_time.GetInt();
}

// This is a direct rip from CHalfLife1
void CHL1MPRules::InitDefaultAIRelationships( void )
{
	int i, j;

	//  Allocate memory for default relationships
	CBaseCombatCharacter::AllocateDefaultRelationships();

	// --------------------------------------------------------------
	// First initialize table so we can report missing relationships
	// --------------------------------------------------------------
	for (i=0;i<NUM_AI_CLASSES;i++)
	{
		for (j=0;j<NUM_AI_CLASSES;j++)
		{
			// By default all relationships are neutral of priority zero
			CBaseCombatCharacter::SetDefaultRelationship( (Class_T)i, (Class_T)j, D_NU, 0 );
		}
	}

	// ------------------------------------------------------------
	//	> CLASS_NONE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_NONE,				D_NU, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_PLAYER,			D_NU, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_HUMAN_PASSIVE,	D_NU, 0 );		
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_PLAYER_ALLY,		D_NU, 0 );		
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_ALIEN_PREY,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_ALIEN_MILITARY,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_ALIEN_MONSTER,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_ALIEN_PREDATOR,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_HUMAN_MILITARY,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_MACHINE,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_ALIEN_BIOWEAPON,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_PLAYER_BIOWEAPON,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_INSECT,			D_NU, 0 );


	// ------------------------------------------------------------
	//	> CLASS_HUMAN_PASSIVE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HUMAN_PASSIVE,		CLASS_NONE,				D_NU, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HUMAN_PASSIVE,		CLASS_PLAYER,			D_LI, 0 );	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HUMAN_PASSIVE,		CLASS_HUMAN_PASSIVE,	D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HUMAN_PASSIVE,		CLASS_PLAYER_ALLY,		D_LI, 0 );	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HUMAN_PASSIVE,		CLASS_ALIEN_PREY,		D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HUMAN_PASSIVE,		CLASS_ALIEN_MILITARY,	D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HUMAN_PASSIVE,		CLASS_ALIEN_MONSTER,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HUMAN_PASSIVE,		CLASS_ALIEN_PREDATOR,	D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HUMAN_PASSIVE,		CLASS_HUMAN_MILITARY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HUMAN_PASSIVE,		CLASS_MACHINE,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HUMAN_PASSIVE,		CLASS_ALIEN_BIOWEAPON,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HUMAN_PASSIVE,		CLASS_PLAYER_BIOWEAPON,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HUMAN_PASSIVE,		CLASS_INSECT,			D_NU, 0 );



	// ------------------------------------------------------------
	//	> CLASS_PLAYER
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,				CLASS_NONE,				D_NU, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,				CLASS_PLAYER,			D_LI, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,				CLASS_HUMAN_PASSIVE,	D_LI, 0 );	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,				CLASS_PLAYER_ALLY,		D_LI, 0 );	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,		  	    CLASS_ALIEN_PREY,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,		  	    CLASS_ALIEN_MILITARY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,				CLASS_ALIEN_MONSTER,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,				CLASS_ALIEN_PREDATOR,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,				CLASS_HUMAN_MILITARY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,				CLASS_MACHINE,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,				CLASS_ALIEN_BIOWEAPON,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,				CLASS_PLAYER_BIOWEAPON,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,				CLASS_INSECT,			D_NU, 0 );

	// ------------------------------------------------------------
	//	> CLASS_PLAYER_ALLY
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_NONE,				D_NU, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_PLAYER,			D_LI, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_HUMAN_PASSIVE,	D_LI, 0 );	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_PLAYER_ALLY,		D_LI, 0 );	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_ALIEN_PREY,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_ALIEN_MILITARY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_ALIEN_MONSTER,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_ALIEN_PREDATOR,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_HUMAN_MILITARY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_MACHINE,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_ALIEN_BIOWEAPON,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_PLAYER_BIOWEAPON,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_INSECT,			D_NU, 0 );

	// ------------------------------------------------------------
	//	> CLASS_ALIEN_PREY
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_PREY,			CLASS_NONE,				D_NU, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_PREY,			CLASS_PLAYER,			D_HT, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_PREY,			CLASS_PLAYER_ALLY,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_PREY,			CLASS_HUMAN_PASSIVE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_PREY,			CLASS_ALIEN_PREY,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_PREY,			CLASS_ALIEN_MILITARY,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_PREY,			CLASS_ALIEN_MONSTER,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_PREY,			CLASS_ALIEN_PREDATOR,	D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_PREY,			CLASS_HUMAN_MILITARY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_PREY,			CLASS_MACHINE,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_PREY,			CLASS_ALIEN_BIOWEAPON,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_PREY,			CLASS_PLAYER_BIOWEAPON,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_PREY,			CLASS_INSECT,			D_NU, 0 );

	// ------------------------------------------------------------
	//	> CLASS_ALIEN_MILITARY
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_MILITARY,		CLASS_NONE,				D_NU, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_MILITARY,		CLASS_PLAYER,			D_HT, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_MILITARY,		CLASS_PLAYER_ALLY,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_MILITARY,		CLASS_HUMAN_PASSIVE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_MILITARY,		CLASS_ALIEN_PREY,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_MILITARY,		CLASS_ALIEN_MILITARY,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_MILITARY,		CLASS_ALIEN_MONSTER,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_MILITARY,		CLASS_ALIEN_PREDATOR,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_MILITARY,		CLASS_HUMAN_MILITARY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_MILITARY,		CLASS_MACHINE,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_MILITARY,		CLASS_ALIEN_BIOWEAPON,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_MILITARY,		CLASS_PLAYER_BIOWEAPON,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_MILITARY,		CLASS_INSECT,			D_NU, 0 );

	// ------------------------------------------------------------
	//	> CLASS_ALIEN_MONSTER
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_MONSTER,		CLASS_NONE,				D_NU, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_MONSTER,		CLASS_PLAYER,			D_HT, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_MONSTER,		CLASS_PLAYER_ALLY,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_MONSTER,		CLASS_HUMAN_PASSIVE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_MONSTER,		CLASS_ALIEN_PREY,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_MONSTER,		CLASS_ALIEN_MILITARY,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_MONSTER,		CLASS_ALIEN_MONSTER,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_MONSTER,		CLASS_ALIEN_PREDATOR,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_MONSTER,		CLASS_HUMAN_MILITARY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_MONSTER,		CLASS_MACHINE,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_MONSTER,		CLASS_ALIEN_BIOWEAPON,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_MONSTER,		CLASS_PLAYER_BIOWEAPON,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_MONSTER,		CLASS_INSECT,			D_NU, 0 );

	// ------------------------------------------------------------
	//	> CLASS_ALIEN_PREDATOR
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_PREDATOR,		CLASS_NONE,				D_NU, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_PREDATOR,		CLASS_PLAYER,			D_HT, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_PREDATOR,		CLASS_PLAYER_ALLY,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_PREDATOR,		CLASS_HUMAN_PASSIVE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_PREDATOR,		CLASS_ALIEN_PREY,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_PREDATOR,		CLASS_ALIEN_MILITARY,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_PREDATOR,		CLASS_ALIEN_MONSTER,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_PREDATOR,		CLASS_ALIEN_PREDATOR,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_PREDATOR,		CLASS_HUMAN_MILITARY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_PREDATOR,		CLASS_MACHINE,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_PREDATOR,		CLASS_ALIEN_BIOWEAPON,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_PREDATOR,		CLASS_PLAYER_BIOWEAPON,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_PREDATOR,		CLASS_INSECT,			D_NU, 0 );

	// ------------------------------------------------------------
	//	> CLASS_HUMAN_MILITARY
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HUMAN_MILITARY,		CLASS_NONE,				D_NU, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HUMAN_MILITARY,		CLASS_PLAYER,			D_HT, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HUMAN_MILITARY,		CLASS_PLAYER_ALLY,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HUMAN_MILITARY,		CLASS_HUMAN_PASSIVE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HUMAN_MILITARY,		CLASS_ALIEN_PREY,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HUMAN_MILITARY,		CLASS_ALIEN_MILITARY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HUMAN_MILITARY,		CLASS_ALIEN_MONSTER,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HUMAN_MILITARY,		CLASS_ALIEN_PREDATOR,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HUMAN_MILITARY,		CLASS_HUMAN_MILITARY,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HUMAN_MILITARY,		CLASS_MACHINE,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HUMAN_MILITARY,		CLASS_ALIEN_BIOWEAPON,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HUMAN_MILITARY,		CLASS_PLAYER_BIOWEAPON,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HUMAN_MILITARY,		CLASS_INSECT,			D_NU, 0 );

	// ------------------------------------------------------------
	//	> CLASS_MACHINE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MACHINE,			CLASS_NONE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MACHINE,			CLASS_MACHINE,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MACHINE,			CLASS_PLAYER,			D_HT, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MACHINE,			CLASS_HUMAN_PASSIVE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MACHINE,			CLASS_HUMAN_MILITARY,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MACHINE,			CLASS_ALIEN_MILITARY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MACHINE,			CLASS_ALIEN_MONSTER,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MACHINE,			CLASS_ALIEN_PREY,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MACHINE,			CLASS_ALIEN_PREDATOR,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MACHINE,			CLASS_PLAYER_ALLY,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MACHINE,			CLASS_ALIEN_BIOWEAPON,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MACHINE,			CLASS_PLAYER_BIOWEAPON,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MACHINE,			CLASS_INSECT,			D_NU, 0 );

	// ------------------------------------------------------------
	//	> CLASS_ALIEN_BIOWEAPON
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_BIOWEAPON,	CLASS_NONE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_BIOWEAPON,	CLASS_MACHINE,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_BIOWEAPON,	CLASS_PLAYER,			D_HT, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_BIOWEAPON,	CLASS_HUMAN_PASSIVE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_BIOWEAPON,	CLASS_HUMAN_MILITARY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_BIOWEAPON,	CLASS_ALIEN_MILITARY,	D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_BIOWEAPON,	CLASS_ALIEN_MONSTER,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_BIOWEAPON,	CLASS_ALIEN_PREY,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_BIOWEAPON,	CLASS_ALIEN_PREDATOR,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_BIOWEAPON,	CLASS_PLAYER_ALLY,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_BIOWEAPON,	CLASS_ALIEN_BIOWEAPON,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_BIOWEAPON,	CLASS_PLAYER_BIOWEAPON,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ALIEN_BIOWEAPON,	CLASS_INSECT,			D_NU, 0 );

	// ------------------------------------------------------------
	//	> CLASS_PLAYER_BIOWEAPON
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_BIOWEAPON,	CLASS_NONE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_BIOWEAPON,	CLASS_MACHINE,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_BIOWEAPON,	CLASS_PLAYER,			D_HT, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_BIOWEAPON,	CLASS_HUMAN_PASSIVE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_BIOWEAPON,	CLASS_HUMAN_MILITARY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_BIOWEAPON,	CLASS_ALIEN_MILITARY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_BIOWEAPON,	CLASS_ALIEN_MONSTER,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_BIOWEAPON,	CLASS_ALIEN_PREY,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_BIOWEAPON,	CLASS_ALIEN_PREDATOR,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_BIOWEAPON,	CLASS_PLAYER_ALLY,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_BIOWEAPON,	CLASS_ALIEN_BIOWEAPON,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_BIOWEAPON,	CLASS_PLAYER_BIOWEAPON,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_BIOWEAPON,	CLASS_INSECT,			D_NU, 0 );


	// ------------------------------------------------------------
	//	> CLASS_INSECT
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_INSECT,				CLASS_NONE,				D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_INSECT,				CLASS_MACHINE,			D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_INSECT,				CLASS_PLAYER,			D_FR, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_INSECT,				CLASS_HUMAN_PASSIVE,	D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_INSECT,				CLASS_HUMAN_MILITARY,	D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_INSECT,				CLASS_ALIEN_MILITARY,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_INSECT,				CLASS_ALIEN_MONSTER,	D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_INSECT,				CLASS_ALIEN_PREY,		D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_INSECT,				CLASS_ALIEN_PREDATOR,	D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_INSECT,				CLASS_PLAYER_ALLY,		D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_INSECT,				CLASS_ALIEN_BIOWEAPON,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_INSECT,				CLASS_PLAYER_BIOWEAPON,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_INSECT,				CLASS_INSECT,			D_NU, 0 );
}
#endif
