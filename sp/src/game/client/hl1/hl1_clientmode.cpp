//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// $Header: $
// $NoKeywords: $
//
//=============================================================================
#include "cbase.h"
#include "ivmodemanager.h"
#include "clientmode_hlnormal.h"

// default FOV for HL1
ConVar default_fov( "default_fov", "90", 0 );

// The current client mode. Always ClientModeNormal in HL.
IClientMode *g_pClientMode = NULL;

class CHLModeManager : public IVModeManager
{
public:
				CHLModeManager( void );
	virtual		~CHLModeManager( void );

	virtual void	Init( void );
	virtual void	SwitchMode( bool commander, bool force );
	virtual void	OverrideView( CViewSetup *pSetup );
	virtual void	CreateMove( float flFrameTime, float flInputSampleTime, CUserCmd *cmd );
	virtual void	LevelInit( const char *newmap );
	virtual void	LevelShutdown( void );
};

CHLModeManager::CHLModeManager( void )
{
}

CHLModeManager::~CHLModeManager( void )
{
}

void CHLModeManager::Init( void )
{
	g_pClientMode = GetClientModeNormal();
}

void CHLModeManager::SwitchMode( bool commander, bool force )
{
}

void CHLModeManager::OverrideView( CViewSetup *pSetup )
{
}

void CHLModeManager::CreateMove( float flFrameTime, float flInputSampleTime, CUserCmd *cmd )
{
}

void CHLModeManager::LevelInit( const char *newmap )
{
	g_pClientMode->LevelInit( newmap );
}

void CHLModeManager::LevelShutdown( void )
{
	g_pClientMode->LevelShutdown();
}


static CHLModeManager g_HLModeManager;
IVModeManager *modemanager = &g_HLModeManager;

//-----------------------------------------------------------------------------
// Purpose: this is the viewport that contains all the hud elements
//-----------------------------------------------------------------------------
class CHudViewport : public CBaseViewport
{
private:
	DECLARE_CLASS_SIMPLE( CHudViewport, CBaseViewport );

protected:
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme )
	{
		BaseClass::ApplySchemeSettings( pScheme );

		gHUD.InitColors( pScheme );

		SetPaintBackgroundEnabled( false );
	}

	virtual void CreateDefaultPanels( void )
	{
		CBaseViewport::CreateDefaultPanels();
	}

	virtual IViewPortPanel *CreatePanelByName( const char *szPanelName );
};
#include "clientmode_shared.h"

class ClientModeHL1Normal : public ClientModeShared 
{
	DECLARE_CLASS( ClientModeHL1Normal, ClientModeShared );
public:
					ClientModeHL1Normal();
	virtual			~ClientModeHL1Normal();

	virtual	void	InitViewport();

	virtual float	GetViewModelFOV( void );

	virtual int		GetDeathMessageStartHeight( void );
};


extern IClientMode *GetClientModeNormal();
extern ClientModeHL1Normal* GetClientModeHL1Normal();


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ClientModeHL1Normal::ClientModeHL1Normal()
{
}

//-----------------------------------------------------------------------------
// Purpose: If you don't know what a destructor is by now, you are probably going to get fired
//-----------------------------------------------------------------------------
ClientModeHL1Normal::~ClientModeHL1Normal()
{
}

void ClientModeHL1Normal::InitViewport()
{
	m_pViewport = new CHudViewport();
	m_pViewport->Start( gameuifuncs, gameeventmanager );
}

float ClientModeHL1Normal::GetViewModelFOV( void )
{
	return 90.0f;
}


int	ClientModeHL1Normal::GetDeathMessageStartHeight( void )
{
	return m_pViewport->GetDeathMessageStartHeight();
}


ClientModeHL1Normal g_ClientModeNormal;

IClientMode *GetClientModeNormal()
{
	return &g_ClientModeNormal;
}

ClientModeHL1Normal* GetClientModeHL1Normal()
{
	Assert( dynamic_cast< ClientModeHL1Normal* >( GetClientModeNormal() ) );

	return static_cast< ClientModeHL1Normal* >( GetClientModeNormal() );
}

IViewPortPanel* CHudViewport::CreatePanelByName( const char *szPanelName )
{
/*	else if ( Q_strcmp(PANEL_INFO, szPanelName) == 0 )
	{
		newpanel = new CHL2MPTextWindow( this );
		return newpanel;
	}*/

	return BaseClass::CreatePanelByName( szPanelName ); 
}