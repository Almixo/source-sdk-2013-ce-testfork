//========= Copyright � 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "usermessages.h"
#include "shake.h"
#include "voice_gamemgr.h"

void RegisterUserMessages( void )
{
	usermessages->Register( "Geiger", 1 );
	usermessages->Register( "Train", 1 );
	usermessages->Register( "HudText", -1 );
	usermessages->Register( "SayText", -1 );
	usermessages->Register( "TextMsg", -1 );
	usermessages->Register( "ResetHUD", 1 );		// called every respawn
	usermessages->Register( "InitHUD", 0 );		// called every time a new player joins the server
	usermessages->Register( "GameTitle", 1 );
	usermessages->Register( "DeathMsg", -1 );
	usermessages->Register( "GameMode", 1 );
	usermessages->Register( "MOTD", -1 );
	usermessages->Register( "ItemPickup", -1 );
	usermessages->Register( "ShowMenu", -1 );
	usermessages->Register( "Fade", sizeof(ScreenFade_t) );
	usermessages->Register( "TeamChange", 1 );
	usermessages->Register( "ClearDecals", 1 );
	usermessages->Register( "Shake", 13 );
	usermessages->Register( "VGUIMenu", -1 );	// Show VGUI menu
	usermessages->Register( "Rumble", 3 );	// Send a rumble to a controller
	usermessages->Register( "Battery", 2 );
	usermessages->Register( "Damage", 18 );
	usermessages->Register( "VoiceMask", VOICE_MAX_PLAYERS_DW*4 * 2 + 1 );
	usermessages->Register( "RequestState", 0 );
	usermessages->Register( "TerrainMod", -1 );
	usermessages->Register( "CloseCaption", -1 ); // Show a caption (by string id number)(duration in 10th of a second)
	usermessages->Register( "HintText", -1 );	// Displays hint text display
	usermessages->Register( "KeyHintText", -1 );	// Displays hint text display
	usermessages->Register( "AmmoDenied", 2 );
	//===========================================
	// Source SDK 2013 Singleplayer messages.
	//===========================================
	usermessages->Register( "SPHapWeapEvent", 4 );
	usermessages->Register( "HapDmg", -1 );
	usermessages->Register( "HapPunch", -1 );
	usermessages->Register( "HapSetDrag", -1 );
	usermessages->Register( "HapSetConst", -1 );
	usermessages->Register( "HapMeleeContact", 0 );

}