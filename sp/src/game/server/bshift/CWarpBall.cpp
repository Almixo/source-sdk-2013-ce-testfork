#include "cbase.h"
#include "beam_shared.h"
#include "Sprite.h"
#include "soundent.h"
#include "EnvBeam.h"

#define SF_REMOVE_ON_FIRE 1
#define SF_KILL_CENTER 2

class CWarpBall : public CPointEntity
{
	DECLARE_CLASS(CWarpBall, CPointEntity);
	DECLARE_DATADESC();
public:
	~CWarpBall();

	void Spawn(void);
	void Precache(void);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void Think(void) override;
	void InputActivate(inputdata_t &inputdata);

	void RunBeams(void);
	void RunSprites(void);
	void RunSounds(void);

private:
	COutputEvent OnActivate;

	float fRadius, fDamageDelay, fActiveTime, fBeamTime;

	Vector vecOrigin;

	CSprite *pSpr[2];
	CEnvBeam *pBeam;

	bool bActive = false;
};

LINK_ENTITY_TO_CLASS(env_warpball, CWarpBall);

BEGIN_DATADESC(CWarpBall)
	DEFINE_KEYFIELD(fRadius, FIELD_FLOAT, "radius"),
	DEFINE_KEYFIELD(fDamageDelay, FIELD_TIME, "damage_delay"),

	DEFINE_INPUTFUNC(FIELD_VOID, "Activate", InputActivate),
	DEFINE_OUTPUT(OnActivate, "OnActivate"),

	DEFINE_ARRAY(pSpr, FIELD_CLASSPTR, 2),
	DEFINE_FIELD(pBeam, FIELD_CLASSPTR),

	DEFINE_FIELD(bActive, FIELD_BOOLEAN),
	DEFINE_FIELD(fActiveTime, FIELD_TIME),
	DEFINE_FIELD(fBeamTime, FIELD_TIME),
END_DATADESC();
//============================================================

CWarpBall::~CWarpBall()
{
	if (pBeam != nullptr)
	{
		UTIL_Remove(pBeam);
		pBeam = nullptr;
	}
	for (auto& var : pSpr)
	{
		UTIL_Remove(var);
		var = nullptr;
	}
}

void CWarpBall::Spawn(void)
{
	BaseClass::Spawn();
	Precache();

	vecOrigin = GetAbsOrigin();

	DevMsg("%s spawned at %f %f %f!\n", GetDebugName(), vecOrigin.x, vecOrigin.y, vecOrigin.z);

	SetNextThink(TICK_NEVER_THINK);
}
void CWarpBall::Precache(void)
{
	BaseClass::Precache();

	PrecacheMaterial("sprites/Fexplo1.vmt");
	PrecacheMaterial("sprites/XFlare1.vmt");

	PrecacheScriptSound("Debris.AlienTeleport");
}
void CWarpBall::InputActivate(inputdata_t &inputdata)
{
	Use(inputdata.pActivator, inputdata.pCaller, USE_ON, 1);
}
void CWarpBall::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (bActive)
		return;

	bActive = true;
	fActiveTime = gpGlobals->curtime + 2.0;
	fBeamTime = gpGlobals->curtime + 1.1;

	vecOrigin = GetAbsOrigin();

	DevMsg("%s at %f %f %f recieved use signal!\n", GetDebugName(), vecOrigin.x, vecOrigin.y, vecOrigin.z);

	OnActivate.FireOutput(this, this);

	UTIL_ScreenShake(vecOrigin, 4, 100, 2, 1000, SHAKE_START);

	RunBeams();
	RunSprites();
	RunSounds();

	SetNextThink(gpGlobals->curtime);
}
void CWarpBall::RunBeams(void)
{
	//TODO: remove this sometime...
	//CPVSFilter filter(vecOrigin);
	//for (int i = 0; i < 30; i++)
	//{
	//	trace_t tr;

	//	Vector vDest = Vector(RandomFloat(-1, 1), RandomFloat(-1, 1), RandomFloat(-1, 1)); // Was Normalized() before... works tho?
	//	VectorNormalize(vDest);

	//	UTIL_TraceLine(vecOrigin, vecOrigin + vDest * fRadius, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);

	//	te->BeamPoints(filter, 0, &vecOrigin, &tr.endpos, PrecacheModel("sprites/lgtning.vmt"), 0, 0, 10, 0.5, 1.8, 1.8, 0, 64, 197, 243, 169, 150, 35);
	//}

	pBeam = (CEnvBeam*)CreateNoSpawn("env_beam", vecOrigin, vec3_angle);

	if (!pBeam)
		return;

	pBeam->m_iszSpriteName = MAKE_STRING("sprites/lgtning.vmt");
	pBeam->SetAbsOrigin(vecOrigin);
	pBeam->m_restrike = -0.5;
	pBeam->m_noiseAmplitude = 15.0; //65 :questionmark:
	pBeam->m_boltWidth = 1.8;
	pBeam->m_life = 0.5;
	pBeam->SetColor(0, 255, 0);
	pBeam->AddSpawnFlags(SF_BEAM_SPARKEND | SF_BEAM_TOGGLE | SF_BEAM_DECALS | SF_BEAM_STARTON);
	pBeam->m_radius = fRadius;
	pBeam->m_iszStartEntity = MAKE_STRING(GetDebugName());

	DispatchSpawn(pBeam);

	pBeam->SetThink(&CEnvBeam::StrikeThink);
	pBeam->SetNextThink(gpGlobals->curtime + .1);
}
void CWarpBall::RunSprites(void)
{
	pSpr[0] = CSprite::SpriteCreate("sprites/Fexplo1.vmt", vecOrigin, true);
	pSpr[0]->SetScale(1.0f);
	/*pSpr[0]->SetTransparency(kRenderGlow, 77, 210, 130, 255, kRenderFxNoDissipation);*/

	pSpr[1] = CSprite::SpriteCreate("sprites/XFlare1.vmt", vecOrigin, true);
	pSpr[1]->SetScale(1.2f);
	/*pSpr[1]->SetTransparency(kRenderGlow, 184, 250, 214, 255, kRenderFxNoDissipation);*/

	for (auto& CocaCola : pSpr)
	{
		CocaCola->SetTransparency(kRenderGlow, 77, 210, 130, 255, kRenderFxNoDissipation);
		CocaCola->AddSpawnFlags(SF_SPRITE_ONCE);
		CocaCola->m_flSpriteFramerate = 10.0;
		CocaCola->TurnOn();
	}
}
void CWarpBall::RunSounds(void)
{
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Debris.AlienTeleport");

	CSoundEnt::InsertSound(SOUND_DANGER, vecOrigin, 1024, 0.5);
}
void CWarpBall::Think(void)
{
	if (!bActive)
		return;

	BaseClass::Think();

	if (GetSpawnFlags() & SF_KILL_CENTER)
	{
		if (fDamageDelay < gpGlobals->curtime)
		{
			CBaseEntity *pList[32];
			int iCount = UTIL_EntitiesInSphere(pList, 32, vecOrigin, 48.0f, 0);

			for (int i = 0; i < iCount; i++)
			{
				if (pList[i] == nullptr)
					continue;

				if (pList[i]->IsWorld())
					continue;

				if (pList[i] == this)
					continue;

				//TODO: Simplify this.
				/*if (pList[i]->Classify() == CLASS_ALIEN_MILITARY || 
					pList[i]->Classify() == CLASS_ALIEN_MONSTER ||
					pList[i]->Classify() == CLASS_ALIEN_PREDATOR ||
					pList[i]->Classify() == CLASS_ALIEN_PREY ||
					pList[i]->Classify() == CLASS_ALIEN_BIOWEAPON )
					continue;*/

				//TODO: Why was all that even there? What was I smoking?
				if (pList[i]->Classify() != CLASS_PLAYER)
					continue;

				pList[i]->TakeDamage(CTakeDamageInfo(this, this, 300, DMG_SHOCK | DMG_ALWAYSGIB));
			}
		}
	}
	if (GetSpawnFlags() & SF_REMOVE_ON_FIRE)
		UTIL_Remove(this);

	if (fBeamTime < gpGlobals->curtime)
	{
		pBeam->m_life = 0;
		pBeam->TurnOff();
		pBeam->SetThink(&CEnvBeam::SUB_Remove);
	}

	if (fActiveTime < gpGlobals->curtime)
	{
		bActive = false;
		fActiveTime = 0;

		SetThink(NULL);
		SetNextThink(TICK_NEVER_THINK);
		return;
	}

	SetNextThink(gpGlobals->curtime);
}