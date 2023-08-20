#include "cbase.h"
#include "CGuidedDot.h"

IMPLEMENT_NETWORKCLASS_ALIASED(GuidedDot, DT_GuidedDot)

BEGIN_NETWORK_TABLE(CGuidedDot, DT_GuidedDot)
#ifdef CLIENT_DLL
RecvPropBool(RECVINFO(m_bIsOn)),
#else
SendPropBool(SENDINFO(m_bIsOn)),
#endif
END_NETWORK_TABLE()

#ifndef CLIENT_DLL
// a list of laser dots to search quickly
CEntityClassList<CGuidedDot> g_GuidedDotList;
template <> CGuidedDot *CEntityClassList<CGuidedDot>::m_pClassList = NULL;
CGuidedDot *GetGuidedDotList()
{
	return g_GuidedDotList.m_pClassList;
}

#endif

LINK_ENTITY_TO_CLASS(guided_dot, CGuidedDot);

BEGIN_DATADESC(CGuidedDot)
	DEFINE_FIELD(m_vecSurfaceNormal, FIELD_VECTOR),
	DEFINE_FIELD(m_bIsOn, FIELD_BOOLEAN),
END_DATADESC()


//-----------------------------------------------------------------------------
// Finds missiles in cone
//-----------------------------------------------------------------------------
CBaseEntity *CreateDot(const Vector &origin, CBaseEntity *pOwner, bool bVisibleDot)
{
	return CGuidedDot::Create(origin, pOwner, bVisibleDot);
}

void EnableDot(CBaseEntity *pLaserDot, bool bEnable)
{
	CGuidedDot *pDot = assert_cast<CGuidedDot*>(pLaserDot);
	if (bEnable)
	{
		pDot->TurnOn();
	}
	else
	{
		pDot->TurnOff();
	}
}

CGuidedDot::CGuidedDot(void)
{
	m_bIsOn = true;
#ifndef CLIENT_DLL
	g_GuidedDotList.Insert(this);
#endif
}

CGuidedDot::~CGuidedDot(void)
{
#ifndef CLIENT_DLL
	g_GuidedDotList.Remove(this);
#endif
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
// Output : CGuidedDot
//-----------------------------------------------------------------------------
CGuidedDot *CGuidedDot::Create(const Vector &origin, CBaseEntity *pOwner, bool bVisibleDot)
{
#ifndef CLIENT_DLL
	CGuidedDot *pLaserDot = assert_cast<CGuidedDot*>(CBaseEntity::Create("guided_dot", origin, vec3_angle));

	if (pLaserDot == NULL)
		return NULL;

	pLaserDot->m_bIsOn = bVisibleDot;
	pLaserDot->SetMoveType(MOVETYPE_NONE);
	pLaserDot->AddSolidFlags(FSOLID_NOT_SOLID);
	pLaserDot->AddEffects(EF_NOSHADOW);
	pLaserDot->m_nRenderMode = kRenderWorldGlow;
	pLaserDot->m_nRenderFX = kRenderFxNoDissipation;

	pLaserDot->SetOwnerEntity(pOwner);

	pLaserDot->AddEFlags(EFL_FORCE_CHECK_TRANSMIT);

	return pLaserDot;
#else
	return NULL;
#endif
}

void CGuidedDot::SetDotPosition(const Vector &origin, const Vector &normal)
{
	SetAbsOrigin(origin);
	m_vecSurfaceNormal = normal;
}

Vector CGuidedDot::GetChasePosition()
{
	return GetAbsOrigin() - m_vecSurfaceNormal * 10;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGuidedDot::TurnOn(void)
{
	m_bIsOn = true;
	RemoveEffects(EF_NODRAW);
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGuidedDot::TurnOff(void)
{
	m_bIsOn = false;
	AddEffects(EF_NODRAW);
}


#ifdef CLIENT_DLL

//-----------------------------------------------------------------------------
// Purpose: Draw our sprite
//-----------------------------------------------------------------------------
int CGuidedDot::DrawModel(int flags)
{
	// Get the owning player.
	C_BasePlayer *pPlayer = ToBasePlayer(GetOwnerEntity());
	if (!pPlayer)
		return -1;

	// Get the sprite rendering position.
	Vector vecEndPos;
	float fSize = 0;
	color32 color = { 255, 255, 255, 255 };

	if (!pPlayer->IsDormant())
	{
		Vector vecSrc, vecDir, vecLength;
		float fLength = 1;

		// Always draw the dot in front of our faces when in first-person.
		if (pPlayer->IsLocalPlayer())
		{
			// Take our view position and orientation
			vecSrc = CurrentViewOrigin();
			vecDir = CurrentViewForward();
		}
		else
		{
			// Take the owning player eye position and direction.
			vecSrc = pPlayer->EyePosition();
			QAngle angles = pPlayer->EyeAngles();
			AngleVectors(angles, &vecDir);
		}

		trace_t	trace;
		UTIL_TraceLine(vecSrc, vecSrc + (vecDir * 8192), MASK_SHOT /*MASK_SOLID*/, pPlayer, COLLISION_GROUP_NONE, &trace);

		// Backup off the hit plane, towards the source
		vecEndPos = trace.endpos + vecDir * -4;

		if (pPlayer->IsLocalPlayer())
		{
			vecLength = vecSrc - vecEndPos;
			fLength = (vecLength.Length() / 2) * 0.05f;

			fSize = (fLength + (RandomFloat(-0.5f, 0.5f) * fLength));
			fSize = Clamp(fSize, 0.1f, 32.0f);
		}
		else
			fSize = 8.0f + RandomFloat(-4.0f, 4.0f); //12.0f before
	}
	else
	{
		// Just use our position if we can't predict it otherwise.
		vecEndPos = GetAbsOrigin();

		fSize = 8.0f + RandomFloat(-4.0f, 4.0f); //12.0f before
	}

	// Draw our laser dot in space.
	CMatRenderContextPtr pRenderContext(materials);
	pRenderContext->Bind(m_hSpriteMaterial, this);

	DrawSprite(vecEndPos, fSize, fSize, color);

	DevWarning("Dot's scale is -->%g\n", fSize);

	// Successful.
	return 1;
}
//-----------------------------------------------------------------------------
// Purpose: Setup our sprite reference
//-----------------------------------------------------------------------------
void CGuidedDot::OnDataChanged(DataUpdateType_t updateType)
{
	if (updateType == DATA_UPDATE_CREATED)
	{
		m_hSpriteMaterial.Init(DOT_SPRITE, TEXTURE_GROUP_CLIENT_EFFECTS);
	}
}

#endif	//CLIENT_DLL