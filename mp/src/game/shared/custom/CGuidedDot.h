#ifndef CGUIDEDDOT_H
#define CGUIDEDDOT_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "baseplayer_shared.h"		//hl1/c_hl1mp_player.h

#ifdef CLIENT_DLL
#include "model_types.h"
#include "beamdraw.h"
#include "fx_line.h"
#include "view.h"
#else
#include "te_effect_dispatch.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	RPG_LASER_SPRITE	"sprites/redglow_mp1"

#ifdef CLIENT_DLL
#define CGuidedDot C_GuidedDot
#endif

class CGuidedDot : public CBaseEntity
{
	DECLARE_CLASS( CGuidedDot, CBaseEntity );

	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();
public:

	CGuidedDot( void );
	~CGuidedDot( void );

	static CGuidedDot *Create( const Vector &origin, CBaseEntity *pOwner = NULL, bool bVisibleDot = true );

	void	SetTargetEntity( CBaseEntity *pTarget ) { m_hTargetEnt = pTarget; }
	CBaseEntity *GetTargetEntity( void ) { return m_hTargetEnt; }

	void	SetDotPosition( const Vector &origin, const Vector &normal );
	Vector	GetChasePosition();
	void	TurnOn( void );
	void	TurnOff( void );
	bool	IsOn() const { return m_bIsOn; }

	int		ObjectCaps() { return (BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DONT_SAVE; }

#ifdef CLIENT_DLL

	virtual bool			IsTransparent( void ) { return true; }
	virtual RenderGroup_t	GetRenderGroup( void ) { return RENDER_GROUP_TRANSLUCENT_ENTITY; }
	virtual int				DrawModel( int flags );
	virtual void			OnDataChanged( DataUpdateType_t updateType );
	virtual bool			ShouldDraw( void ) { return (IsEffectActive(EF_NODRAW)==false); }

	CMaterialReference	m_hSpriteMaterial;
#endif

protected:
	Vector				m_vecSurfaceNormal;
	EHANDLE				m_hTargetEnt;
	bool				m_bVisibleLaserDot;
//	bool				m_bIsOn;
	CNetworkVar( bool, m_bIsOn );

public:
	CGuidedDot			*m_pNext;
};
#endif //CGUIDEDDOT_H