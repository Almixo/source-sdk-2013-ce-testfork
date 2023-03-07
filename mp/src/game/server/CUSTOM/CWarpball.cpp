#include "cbase.h"
#include "beam_shared.h"
#include "Sprite.h"

constexpr auto SF_REMOVE_ON_FIRE	= 1;
constexpr auto SF_KILL_CENTER		= 2;
constexpr auto MAGIC_NUMBER			= 1.0f;
constexpr auto FRAMERATE			= 12.0f;
constexpr auto RADIUS				= 48.0f;

#define curtimer					= gpGlobals->curtime

class CWarpBall : public CBaseEntity
{
	DECLARE_CLASS(CWarpBall, CBaseEntity);
	DECLARE_DATADESC();

public:
//	~CWarpBall();

	void Spawn(void);
	void Precache(void);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void Think(void);
	//legacy purpouses
	void InputActivate(inputdata_t &inputdata);

	void RunSound(void);
	void RunBeams(void);
	void RunSprites(void);
	//disabled for now
	int ObjectCaps(void) { return BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

private:
	COutputEvent FireOutput;

	float fRadius		= 0;
	float fDamageDelay	= 0;

	int iBeamCount		= 0;
	int iMaxBeamCount	= 0;

	const int r = 77, g = 210, b = 130;

	Vector vOrigin;

	CSprite *pSpr[2];
};

LINK_ENTITY_TO_CLASS(env_warpball, CWarpBall);

BEGIN_DATADESC(CWarpBall)

	DEFINE_KEYFIELD(fRadius, FIELD_FLOAT, "radius"),
	DEFINE_KEYFIELD(fDamageDelay, FIELD_FLOAT, "damage_delay"),

	DEFINE_INPUTFUNC(FIELD_VOID, "Activate", InputActivate),
	DEFINE_OUTPUT(FireOutput, "OnActivate"),

	DEFINE_FIELD(iMaxBeamCount, FIELD_INTEGER),
	DEFINE_FIELD(iBeamCount, FIELD_INTEGER),

	DEFINE_ARRAY(pSpr, FIELD_CLASSPTR, 2),

END_DATADESC();

void CWarpBall::Spawn(void)
{
	BaseClass::Spawn();
	
	Precache();

	vOrigin = GetAbsOrigin();

	Msg("%s spawned at %g %g %g!\n", GetDebugName(), vOrigin.x, vOrigin.y, vOrigin.z);

	SetNextThink(gpGlobals->curtime + 1.0f);
}

void CWarpBall::Precache(void)
{
	BaseClass::Precache();

	PrecacheMaterial("sprites/lgtning.vmt");
	PrecacheMaterial("sprites/Fexplo1.vmt");
	PrecacheMaterial("sprites/XFlare1.vmt");

	PrecacheScriptSound("Debris.AlienTeleport");
}

void CWarpBall::InputActivate(inputdata_t& inputdata)
{
	Use(inputdata.pActivator, inputdata.pCaller, USE_ON, 1);
}

void CWarpBall::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	Msg("%s at %g %g %g recieved use signal!\n", GetDebugName(), vOrigin.x, vOrigin.y, vOrigin.z);

	FireOutput.FireOutput(this, this);

	UTIL_ScreenShake(vOrigin, 4, 100, 2, 1000, SHAKE_START);

	//do it again!
	vOrigin = GetAbsOrigin();

	RunSound();
	RunBeams();
	RunSprites();

	SetNextThink(gpGlobals->curtime + 0.1f);
}

inline void CWarpBall::RunSound(void)
{
	CPASFilter filter(GetAbsOrigin());
	EmitSound(filter, entindex(), "Debris.AlienTeleport");
}

void CWarpBall::RunBeams(void)
{
	iMaxBeamCount = RandomInt(20, 40);

	for (iBeamCount = 0; iBeamCount < iMaxBeamCount; iBeamCount++)
	{
		trace_t tr;

		Vector vDest = fRadius * (Vector(RandomFloat(-1, 1), RandomFloat(-1, 1), RandomFloat(-1, 1)).Normalized());

		UTIL_TraceLine(vOrigin, vOrigin + vDest, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);

		if (tr.DidHit())
		{
			CBeam *pBeam = CBeam::BeamCreate("sprites/lgtning.vmt", 1.0f);

			pBeam->AddSpawnFlags(SF_BEAM_DECALS | SF_BEAM_SPARKEND);
			pBeam->PointsInit(vOrigin, tr.endpos);
			pBeam->LiveForTime(1.75f);
			pBeam->SetColor(0, 255, 0);
			pBeam->SetNoise(6.5f);
			pBeam->SetBrightness(220);
			pBeam->SetWidth(2);
			pBeam->SetScrollRate(5);
			pBeam->SetOwnerEntity(this);

			//			---SPECIAL---

			pBeam->DoSparks(vOrigin, tr.endpos);
			pBeam->DecalTrace(&tr, "BigShot");

		}
	}
}

void CWarpBall::RunSprites(void)
{
	pSpr[0] = CSprite::SpriteCreate("sprites/Fexplo1.vmt", vOrigin, true);
	pSpr[1] = CSprite::SpriteCreate("sprites/XFlare1.vmt", vOrigin, true);

	pSpr[0]->SetScale(1.0f);
	pSpr[1]->SetScale(1.2f);

	for (int i = 0; i < 2; i++)
	{
		pSpr[i]->SetTransparency(kRenderGlow, r, g, b, 255, kRenderFxNoDissipation);
		pSpr[i]->SetOwnerEntity(this);
		pSpr[i]->AnimateAndDie(10);
	}
}

void CWarpBall::Think(void)
{
	BaseClass::Think();

	if ( (GetSpawnFlags() & SF_KILL_CENTER) != 0)
	{
		if (fDamageDelay > 0)
		{
			if (gpGlobals->curtime > fDamageDelay)
				RadiusDamage(CTakeDamageInfo(this, this, 300, DMG_SHOCK), vOrigin, RADIUS, CLASS_NONE, this);
		}
		else
			RadiusDamage(CTakeDamageInfo(this, this, 300, DMG_SHOCK), vOrigin, RADIUS, CLASS_NONE, this);
	}
	if ( (GetSpawnFlags() & SF_REMOVE_ON_FIRE) != 0)
		UTIL_Remove(this);
}