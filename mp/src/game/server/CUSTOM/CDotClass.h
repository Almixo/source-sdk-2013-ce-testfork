#ifndef CDOTCLASS_H
#define CDOTCLASS_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "hl1mp_basecombatweapon_shared.h"

#ifdef CLIENT_DLL
#include "hl1/c_hl1mp_player.h"
#include "model_types.h"
#include "beamdraw.h"
#include "fx_line.h"
#include "view.h"
#else
#include "basecombatcharacter.h"
#include "movie_explosion.h"
#include "hl1mp_player.h"
#include "rope.h"
#include "soundent.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "explode.h"
#include "util.h"
#include "te_effect_dispatch.h"
#include "shake.h"
#endif

#define	RPG_LASER_SPRITE	"sprites/redglow_mp1"

class CGDot : public CBaseEntity
{
	DECLARE_CLASS( CGDot, CBaseEntity );
public:

	CGDot( void );
	~CGDot( void );

	static CGDot *Create( const Vector &origin, CBaseEntity *pOwner = nullptr, bool bVisibleDot = true );

	void	SetTargetEntity( CBaseEntity *pTarget ) { m_hTargetEnt = pTarget; }
	CBaseEntity *GetTargetEntity( void ) { return m_hTargetEnt; }

	void	SetLaserPosition( const Vector &origin, const Vector &normal );
	Vector	GetChasePosition();
	void	TurnOn( void );
	void	TurnOff( void );
	bool	IsOn() const { return m_bIsOn; }

	void	Toggle( void );

	int		ObjectCaps() { return (BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DONT_SAVE; }

	void	MakeInvisible( void );

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

	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();
public:
	CGDot			*m_pNext;
};
//IMPLEMENT_NETWORKCLASS_ALIASED(CGDot, DT_CGdot)
IMPLEMENT_NETWORKCLASS(CGDot, DT_CGdot)
BEGIN_NETWORK_TABLE(CGDot, DT_CGdot)
#ifdef CLIENT_DLL
RecvPropBool(RECVINFO(m_bIsOn)),
#else
SendPropBool(SENDINFO(m_bIsOn)),
#endif
END_NETWORK_TABLE()

#ifndef CLIENT_DLL
// a list of laser dots to search quickly
CEntityClassList<CGDot> g_LaserDotList;
template <> CGDot* CEntityClassList<CGDot>::m_pClassList = nullptr;
CGDot* GetLaserDotList()
{
	return g_LaserDotList.m_pClassList;
}

#endif

LINK_ENTITY_TO_CLASS(cguideddot, CGDot);

BEGIN_DATADESC(CGDot)
DEFINE_FIELD(m_vecSurfaceNormal, FIELD_VECTOR),
DEFINE_FIELD(m_hTargetEnt, FIELD_EHANDLE),
DEFINE_FIELD(m_bVisibleLaserDot, FIELD_BOOLEAN),
DEFINE_FIELD(m_bIsOn, FIELD_BOOLEAN),

//DEFINE_FIELD( m_pNext, FIELD_CLASSPTR ),	// don't save - regenerated by constructor
END_DATADESC()


//-----------------------------------------------------------------------------
// Finds missiles in cone
//-----------------------------------------------------------------------------
CBaseEntity* CreateLaserDot(const Vector &origin, CBaseEntity *pOwner, bool bVisibleDot)
{
	return CGDot::Create(origin, pOwner, bVisibleDot);
}

void SetLaserDotTarget(CBaseEntity *pLaserDot, CBaseEntity *pTarget)
{
	CGDot* pDot = assert_cast<CGDot*>(pLaserDot);
	pDot->SetTargetEntity(pTarget);
}

void EnableLaserDot(CBaseEntity *pLaserDot, bool bEnable)
{
	CGDot* pDot = assert_cast<CGDot*>(pLaserDot);
	if (bEnable)
	{
		pDot->TurnOn();
	}
	else
	{
		pDot->TurnOff();
	}
}

CGDot::CGDot(void)
{
	m_hTargetEnt = nullptr;
	m_bIsOn = true;
#ifndef CLIENT_DLL
	g_LaserDotList.Insert(this);
#endif
}

CGDot::~CGDot(void)
{
#ifndef CLIENT_DLL
	g_LaserDotList.Remove(this);
#endif
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
// Output : CGDot
//-----------------------------------------------------------------------------
CGDot* CGDot::Create(const Vector &origin, CBaseEntity *pOwner, bool bVisibleDot)
{
#ifndef CLIENT_DLL
	CGDot* pLaserDot = (CGDot*)CBaseEntity::Create("cguideddot", origin, QAngle(0, 0, 0));

	if (pLaserDot == NULL)
		return NULL;

	pLaserDot->m_bVisibleLaserDot = bVisibleDot;
	pLaserDot->SetMoveType(MOVETYPE_NONE);
	pLaserDot->AddSolidFlags(FSOLID_NOT_SOLID);
	pLaserDot->AddEffects(EF_NOSHADOW);
	UTIL_SetSize(pLaserDot, -Vector(6, 6, 6), Vector(6, 6, 6));

	pLaserDot->SetOwnerEntity(pOwner);

	pLaserDot->AddEFlags(EFL_FORCE_CHECK_TRANSMIT);

	if (!bVisibleDot)
	{
		pLaserDot->MakeInvisible();
	}

	return pLaserDot;
#else
	return NULL;
#endif
}

void CGDot::SetLaserPosition(const Vector &origin, const Vector &normal)
{
	SetAbsOrigin(origin);
	m_vecSurfaceNormal = normal;
}

Vector CGDot::GetChasePosition()
{
	return GetAbsOrigin() - m_vecSurfaceNormal * 10;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGDot::TurnOn(void)
{
	m_bIsOn = true;
	RemoveEffects(EF_NODRAW);

	if (m_bVisibleLaserDot)
	{
		//BaseClass::TurnOn();
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGDot::TurnOff(void)
{
	m_bIsOn = false;
	AddEffects(EF_NODRAW);
	if (m_bVisibleLaserDot)
	{
		//BaseClass::TurnOff();
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGDot::MakeInvisible(void)
{
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: Draw our sprite
//-----------------------------------------------------------------------------
int CGDot::DrawModel(int flags)
{
	color32 color = { 255,255,255,255 };
	Vector	vecAttachment, vecDir;
	QAngle	angles;

	float	scale;
	Vector	endPos;

	C_HL1MP_Player* pOwner = ToHL1MPPlayer(GetOwnerEntity());

	if (pOwner != nullptr && pOwner->IsDormant() == false)
	{
		// Always draw the dot in front of our faces when in first-person
		if (pOwner->IsLocalPlayer())
		{
			// Take our view position and orientation
			vecAttachment = CurrentViewOrigin();
			vecDir = CurrentViewForward();
		}
		else
		{
			// Take the eye position and direction
			vecAttachment = pOwner->EyePosition();

			QAngle angles = pOwner->EyeAngles();
			AngleVectors(angles, &vecDir);
		}

		trace_t tr;
		UTIL_TraceLine(vecAttachment, vecAttachment + (vecDir * MAX_TRACE_LENGTH), MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);

		// Backup off the hit plane
		endPos = tr.endpos + (tr.plane.normal * 4.0f);
	}
	else
	{
		// Just use our position if we can't predict it otherwise
		endPos = GetAbsOrigin();
	}

	// Randomly flutter
	scale = 16.0f + random->RandomFloat(-4.0f, 4.0f);

	// Draw our laser dot in space
	CMatRenderContextPtr pRenderContext(materials);
	pRenderContext->Bind(m_hSpriteMaterial, this);
	DrawSprite(endPos, scale, scale, color);

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: Setup our sprite reference
//-----------------------------------------------------------------------------
void CGDot::OnDataChanged(DataUpdateType_t updateType)
{
	if (updateType == DATA_UPDATE_CREATED)
	{
		m_hSpriteMaterial.Init(RPG_LASER_SPRITE, TEXTURE_GROUP_CLIENT_EFFECTS);
	}
}

#endif	//CLIENT_DLL

#endif //CDOTCLASS_H