#include "cbase.h"
#include "CGuidedDotWeapon.h"

#define GW CGdWeapon
#define CG GW
#define TRend tr.endpos + (tr.plane.normal * 2.0f)

CGdWeapon::CGdWeapon()
{
	m_bFiresUnderwater	= false;
	m_bReloadsSingly	= false;

	bDot				= false;
	bReloadDot			= false;
	bHelperDot			= false;
}
CGdWeapon::~CGdWeapon()
{
	if (pSprite != nullptr)
	{
		pSprite->TurnOff();
		pSprite->Remove();
		pSprite = nullptr;
	}
}
void CGdWeapon::Spawn(void)
{
	CBaseCombatWeapon::Spawn(); //bruh
}
void CGdWeapon::Precache(void)
{
	BaseClass::Precache();

	UTIL_PrecacheOther("cgdot");
}
bool GW::Deploy(void)
{
	if (pSprite != nullptr)
	{
		pSprite->TurnOff();
		pSprite->Remove();
		pSprite = nullptr;
	}

	int i = FindBodygroupByName("scope");
	SetBodygroup(i, 1);

	return BaseClass::Deploy();
}
void GW::WeaponIdle(void)
{
	BaseClass::WeaponIdle();

	if (!pSprite) SpawnDot();

	if (!bDot)
	{
		if (bHelperDot || bReloadDot)
		{
			UpdateDotPos();
			ToggleDot();

			bHelperDot = false;
			bReloadDot = false;
		}
	}
}
void CGdWeapon::PrimaryAttack(void)
{
	// Only the player fires this way so we can cast
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
	{
		return;
	}

	if (m_iClip1 <= 0)
	{
		if (!m_bFireOnEmpty)
		{
			Reload();
		}
		else
		{
			WeaponSound(EMPTY);
			m_flNextEmptySoundTime = 1.0f;
			m_flNextPrimaryAttack = 0.15f;
		}

		return;
	}

	WeaponSound(SINGLE);
	pPlayer->DoMuzzleFlash();

	SendWeaponAnim(ACT_VM_PRIMARYATTACK);
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_iClip1--;

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector vecAiming = pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (bDot)
	{
		FireBulletsInfo_t info(1, vecSrc, vecAiming, VECTORGOOD, MAX_TRACE_LENGTH, m_iPrimaryAmmoType);
		info.m_pAttacker = pPlayer;
		pPlayer->FireBullets(info);

		m_flNextPrimaryAttack = gpGlobals->curtime + 0.75f;
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.75f;
	}
	else
	{
		FireBulletsInfo_t info(1, vecSrc, vecAiming, VECTORNOAIM, MAX_TRACE_LENGTH, m_iPrimaryAmmoType);
		info.m_pAttacker = pPlayer;
		pPlayer->FireBullets(info);

		m_flNextPrimaryAttack = gpGlobals->curtime + 0.25f;
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.25f;
	}

//	EjectShell(pPlayer, 0); //would eject boolet casings if um... hls codebase :/

#ifndef CLIENT_DLL
	pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 0.5f);
#endif

	pPlayer->ViewPunch(QAngle(-1.5, 0, 0));

	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}
}
void CGdWeapon::SecondaryAttack(void)
{
	ToggleDot();
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.75f;
	m_flNextEmptySoundTime = gpGlobals->curtime + 1.0f;
}
bool CGdWeapon::Reload(void)
{
	bool fRet;

	fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD);
	if (fRet)
	{
		if (bDot && IsDotOn())
		{
			ToggleDot();
			bReloadDot = true;
		}
	}
	WeaponSound(RELOAD);
	return fRet;
}
void CGdWeapon::FinishReload(void)
{
	BaseClass::FinishReload();

	if (!bDot && bReloadDot)
	{
		SpawnDot();
		UpdateDotPos();
		ToggleDot();
		bReloadDot = false;
	}
}
void CGdWeapon::ItemPostFrame(void)
{
	BaseClass::ItemPostFrame();

	UpdateDotPos();
}
bool CGdWeapon::Holster(CBaseCombatWeapon* pSwitchingTo)
{
	if (bDot)
	{
		ToggleDot();
		bHelperDot = true;
	}

	return BaseClass::Holster();
}
void CGdWeapon::UpdateDotPos(void)
{
	if (!pSprite)
	{
		Warning("Dot doesn't exist yet we still want to update it in UpdateDotPos()!\n");
		return;
	}

	if (bDot && !IsDotOn())
	{
		Warning("Dot should be on but isn't in IsDotOn()! --> UpdateDotPos()!\n");
		return;
	}

	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	Vector VecStart = pPlayer->Weapon_ShootPosition();
	Vector VecDir;
	AngleVectors(pPlayer->EyeAngles() + pPlayer->m_Local.m_vecPunchAngle, &VecDir);
	Vector VecEnd = VecStart + (VecDir * MAX_TRACE_LENGTH);

	trace_t tr;

	UTIL_TraceLine(VecStart, VecEnd, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

	UTIL_SetOrigin(pSprite, TRend);
}
void CGdWeapon::SpawnDot(void)
{
	if (pSprite != nullptr) return;

	pSprite = CSprite::SpriteCreate(SPRITE_MATERIAL, Vector(0, 0, 0), false);
	pSprite->AddSpawnFlags(SF_SPRITE_TEMPORARY);
	pSprite->SetOwnerEntity(ToBasePlayer(GetOwner()));
	pSprite->SetScale(DOTSIZE);
	pSprite->SetTransparency(SPRITE_TRANS);
	pSprite->SetSimulatedEveryTick(true);

	if (!bDot) pSprite->TurnOff();
	else WeaponSound(SPECIAL1);
}
void CGdWeapon::ToggleDot(void)
{

	if (!pSprite)
	{
		Warning("Dot's gone but still calling ToggleDot()!\n");
		return;
	}

	if (bDot && !IsDotOn())
	{
		Warning("Dot's broken in ToggleDot()! --> bDot && !IsDotOn()!\n");
		pSprite->TurnOn();
		return;
	}

	bDot = !bDot;

	if (bDot)
	{
		pSprite->TurnOn();
		WeaponSound(SPECIAL1);
	}
	if (!bDot)
	{
		pSprite->TurnOff();
		WeaponSound(SPECIAL2);
	}
}
bool CGdWeapon::IsDotOn(void)
{
	if (!pSprite)
	{
		Warning("Got error in IsDotOn()!\n");
		return false;
	}

	return pSprite->IsOn();
}