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

	/*pLaserDot->m_bIsOn = bVisibleDot;
	pLaserDot->SetMoveType(MOVETYPE_NONE);
	pLaserDot->AddSolidFlags(FSOLID_NOT_SOLID);
	pLaserDot->AddEffects(EF_NOSHADOW);
	pLaserDot->m_nRenderMode = kRenderWorldGlow;
	pLaserDot->m_nRenderFX = kRenderFxNoDissipation;

	pLaserDot->SetOwnerEntity(pOwner);

	pLaserDot->AddEFlags(EFL_FORCE_CHECK_TRANSMIT);*/

	pLaserDot->m_bIsOn = bVisibleDot;
	pLaserDot->SetMoveType(MOVETYPE_NONE);
	pLaserDot->AddEffects(EF_NOSHADOW);
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
	CBasePlayer *pPlayer = ToBasePlayer(GetOwnerEntity());
	if (!pPlayer)
		return -1;

	Vector vecPos, vecEnd;
	float flsize = 0.0f;
	color32 color = { 255, 255, 255, 255 };

	if (!pPlayer->IsDormant())
	{
		Vector vecSrc, vecDir, vecLen;
		float fLen = 0.0f;

		if (pPlayer->IsLocalPlayer())
		{
			vecSrc = CurrentViewOrigin();
			vecDir = CurrentViewForward();
		}
		else
		{
			vecSrc = pPlayer->EyePosition();
			AngleVectors(pPlayer->EyeAngles(), &vecDir);
		}

		trace_t tr;
		UTIL_TraceLine(vecSrc, vecSrc + (vecDir * MAX_TRACE_LENGTH), MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);

		vecEnd = tr.endpos + vecDir.Normalized() * -4;

		if (pPlayer->IsLocalPlayer())
		{
			vecLen = vecSrc - vecEnd;
			fLen = vecLen.Length() * 0.025f;

			flsize = fLen + RandomFloat(-.5f, .5f) * fLen;

			flsize < 0.1f ? 0.1f : flsize; //0.1 at minimum
		}
		else
			flsize = 8.0f + RandomFloat(-2.0f, 2.0f);
	}
	else
	{
		vecEnd = GetAbsOrigin();
		flsize = 8.0f + RandomFloat(-2.0f, 2.0f);
	}

	CMatRenderContextPtr pRenderContext(materials);
	pRenderContext->Bind(m_hSpriteMaterial, this);

	DrawSprite(vecEnd, flsize, flsize, color);

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