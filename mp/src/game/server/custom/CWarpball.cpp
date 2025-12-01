#include "cbase.h"
#include "EnvBeam.h"
#include "Sprite.h"

#define SF_REMOVE_ON_FIRE (1<<0)
#define SF_KILL_CENTER (1<<1)

class CWarpBall : public CBaseEntity
{
	DECLARE_CLASS(CWarpBall, CBaseEntity);
	DECLARE_DATADESC();
public:

	CWarpBall();
	~CWarpBall();

	void Spawn(void);
	void Precache(void);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void Think(void);
	void InputActivate(inputdata_t &inputdata);

	void RunBeams(void);
	void RunSprites(void);
	void RunSound(void);
private:

	COutputEvent OnActivate;

	float fRadius;
	float fDamageDelay;
	float fActiveTime;
	
	CSprite *pSprite[2];
	CEnvBeam *pBeam;

	bool bActive;
};
LINK_ENTITY_TO_CLASS(env_warpball, CWarpBall);

BEGIN_DATADESC(CWarpBall)
	DEFINE_KEYFIELD(fRadius, FIELD_FLOAT, "radius"),
	DEFINE_KEYFIELD(fDamageDelay, FIELD_TIME, "damage_delay"),

	DEFINE_INPUTFUNC(FIELD_VOID, "Activate", InputActivate),
	DEFINE_OUTPUT(OnActivate, "OnActivate"),

	DEFINE_ARRAY(pSprite, FIELD_CLASSPTR, 2),
	DEFINE_FIELD(pBeam, FIELD_CLASSPTR),

	DEFINE_FIELD(bActive, FIELD_BOOLEAN),
	DEFINE_FIELD(fActiveTime, FIELD_TIME),
END_DATADESC();

CWarpBall::CWarpBall()
{
	memset(pSprite, 0, sizeof pSprite);
	pBeam = nullptr;

	fActiveTime = 0;
	bActive = false;
}
CWarpBall::~CWarpBall()
{
	memset(pSprite, 0, sizeof pSprite);
	pBeam = nullptr;

	fActiveTime = 0;
	bActive = false;
}

void CWarpBall::Spawn(void)
{
	BaseClass::Spawn();

	Precache();
	SetNextThink(TICK_NEVER_THINK);
}

void CWarpBall::Precache(void)
{
	BaseClass::Precache();

	PrecacheMaterial("sprites/Fexplo1.vmt");
	PrecacheMaterial("sprites/XFlare1.vmt");

	PrecacheScriptSound("Debris.AlienTeleport");

	UTIL_PrecacheOther("env_beam");
	UTIL_PrecacheDecal("env_sprite");
}

void CWarpBall::InputActivate(inputdata_t &inputdata)
{
	Use(inputdata.pActivator, inputdata.pCaller, USE_ON, 0);
}

void CWarpBall::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE usetype, float value)
{
	if (bActive)
		return;

	bActive = true;

	fActiveTime = gpGlobals->curtime + 2;

	RunBeams();
	RunSprites();
	RunSound();

	SetNextThink(gpGlobals->curtime);
}

void CWarpBall::RunBeams(void)
{
	pBeam = (CEnvBeam *)CreateNoSpawn("env_beam", GetAbsOrigin(), vec3_angle, this);
	if (!pBeam)
		return;

	char szName[65];
	Q_snprintf(szName, 65, "beams_%s", GetDebugName());

	string_t tmp = AllocPooledString( szName );

	pBeam->m_boltWidth = 1.8;
	pBeam->m_iszSpriteName = MAKE_STRING("sprites/lgtning.vmt");
	pBeam->SetName(tmp);
	pBeam->m_life = .5;
	pBeam->m_restrike = -.5;
	pBeam->m_iszStartEntity = tmp;
	pBeam->AddSpawnFlags(SF_BEAM_TOGGLE /*| SF_BEAM_RANDOM*/ | SF_BEAM_DECALS | SF_BEAM_SPARKEND);
	pBeam->m_noiseAmplitude = 10.4f;
	pBeam->m_radius = fRadius;
	pBeam->SetRenderColor(0, 255, 0);
	pBeam->SetBrightness(150);

	DispatchSpawn(pBeam);

	inputdata_t input;
	input.pActivator = input.pCaller = this;

	pBeam->InputToggle(input);
}

void CWarpBall::RunSprites(void)
{
	pSprite[0] = CSprite::SpriteCreate("sprites/Fexplo1.vmt", GetAbsOrigin(), true);
	pSprite[0]->SetScale(1.0f);
	/*pSpr[0]->SetTransparency(kRenderGlow, 77, 210, 130, 255, kRenderFxNoDissipation);*/

	pSprite[1] = CSprite::SpriteCreate("sprites/XFlare1.vmt", GetAbsOrigin(), true);
	pSprite[1]->SetScale(1.2f);
	/*pSpr[1]->SetTransparency(kRenderGlow, 184, 250, 214, 255, kRenderFxNoDissipation);*/

	for (auto &sprites : pSprite)
	{
		sprites->SetTransparency(kRenderGlow, 77, 210, 130, 255, kRenderFxNoDissipation);
		sprites->AddSpawnFlags(SF_SPRITE_ONCE);
		sprites->m_flSpriteFramerate = 10.0;
		sprites->TurnOn();
	}
}

void CWarpBall::RunSound(void)
{
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Debris.AlienTeleport");
}

void CWarpBall::Think(void)
{
	if (!bActive)
		return;

	BaseClass::Think();

	if (fActiveTime <= gpGlobals->curtime)
	{
		bActive = false;
		fActiveTime = 0;

		SetThink(NULL);
		SetNextThink(TICK_NEVER_THINK);
		return;
	}

	if ((fActiveTime - 0.7f) <= gpGlobals->curtime)
	{
		pBeam->m_life = 0;
		pBeam->TurnOff();
		pBeam->SetThink(&BaseClass::SUB_Remove);
	}

	if ((fDamageDelay <= gpGlobals->curtime) && HasSpawnFlags(SF_KILL_CENTER))
	{
		CBaseEntity *pEnts[64];
		memset(pEnts, 0, sizeof pEnts);

		int iEntCnt = UTIL_EntitiesInSphere(pEnts, 64, GetAbsOrigin(), 48.0f, 0);

		for (int i = 0; i < iEntCnt; i++)
		{
			if (!pEnts[i])
				continue;

			if (pEnts[i]->IsWorld())
				continue;

			if (pEnts[i] == this)
				continue;

			if ((pEnts[i]->Classify() > CLASS_HUMAN_MILITARY) && pEnts[i]->Classify() < CLASS_PLAYER_ALLY)
				continue;

			if (pEnts[i]->Classify() == CLASS_ALIEN_BIOWEAPON)
				continue;

			pEnts[i]->TakeDamage(CTakeDamageInfo(this, this, 300, DMG_SHOCK));
		}
	}

	if (HasSpawnFlags(SF_REMOVE_ON_FIRE))
		UTIL_Remove(this);

	SetNextThink(gpGlobals->curtime + 0.1);
}
