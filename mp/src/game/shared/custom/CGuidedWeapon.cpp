#include "cbase.h"
#include "CGuidedWeapon.h"
#ifdef CLIENT_DLL
#include "view.h"
#endif

#define GW CGuidedWeapon
#define CG GW
#define TRend tr.endpos + (tr.plane.normal * 2e0f)

CGuidedWeapon::CGuidedWeapon()
{
	m_bFiresUnderwater	= false;
	m_bReloadsSingly	= false;

	bDot				= false;
	bReloadDot			= false;
	bHelperDot			= false;
}
CGuidedWeapon::~CGuidedWeapon()
{
	if (pSprite != nullptr)
	{
		pSprite->TurnOff();
		pSprite->Remove();
		pSprite = nullptr;
	}
}
void CGuidedWeapon::Spawn(void)
{
	CBaseCombatWeapon::Spawn(); //bruh
}
void CGuidedWeapon::Precache(void)
{
	BaseClass::Precache();

	PrecacheMaterial(SPRITE_MATERIAL);
}
bool CGuidedWeapon::Deploy(void)
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
void CGuidedWeapon::WeaponIdle(void)
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
void CGuidedWeapon::PrimaryAttack(void)
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
void CGuidedWeapon::SecondaryAttack(void)
{
	ToggleDot();
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.75f;
	m_flNextEmptySoundTime = gpGlobals->curtime + 1.0f;
}
bool CGuidedWeapon::Reload(void)
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
void CGuidedWeapon::FinishReload(void)
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
void CGuidedWeapon::ItemPostFrame(void)
{
	BaseClass::ItemPostFrame();

	UpdateDotPos();
}
bool CGuidedWeapon::Holster(CBaseCombatWeapon* pSwitchingTo)
{
	if (bDot)
	{
		ToggleDot();
		bHelperDot = true;
	}

	return BaseClass::Holster();
}
void CGuidedWeapon::UpdateDotPos(void)
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

	if (!bDot) return;

	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	Vector	VecStart, VecDir, VecEnd;
	trace_t tr;

#ifndef CLIENT_DLL
	VecStart = pPlayer->Weapon_ShootPosition();
	VecDir;
	AngleVectors(pPlayer->EyeAngles() + pPlayer->m_Local.m_vecPunchAngle, &VecDir);
#else
	if (pPlayer->IsLocalPlayer())
	{
		// Take our view position and orientation
		VecStart = CurrentViewOrigin();
		VecDir;
		AngleVectors(CurrentViewAngles(), &VecDir);
	}
	else
	{
		// Take the eye position and direction
		VecStart = pPlayer->EyePosition();
		VecDir;
		AngleVectors(pPlayer->EyeAngles() + pPlayer->m_Local.m_vecPunchAngle, &VecDir);
	}

#endif

	VecEnd = VecStart + (VecDir * MAX_TRACE_LENGTH);

	UTIL_TraceLine(VecStart, VecEnd, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

	UTIL_SetOrigin(pSprite, TRend);

	DotEndPos = TRend;
}
void CGuidedWeapon::SpawnDot(void)
{
#ifndef CLIENT_DLL
	if (pSprite != nullptr) return;

	Vector SpawnHere;

	if (DotEndPos != NULL) SpawnHere = DotEndPos;
	else SpawnHere = Vector(0, 0, 0);
	
	pSprite = CSprite::SpriteCreate(SPRITE_MATERIAL, SpawnHere, false);
	pSprite->AddSpawnFlags(SF_SPRITE_TEMPORARY);
	pSprite->SetOwnerEntity(ToBasePlayer(GetOwnerEntity()));
	pSprite->SetScale(DOTSIZE);
	pSprite->SetTransparency(SPRITE_TRANS);
	pSprite->SetSimulatedEveryTick(true);

	if (!bDot) pSprite->TurnOff();
	else WeaponSound(SPECIAL1);
#endif
}
void CGuidedWeapon::ToggleDot(void)
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
bool CGuidedWeapon::IsDotOn(void)
{
	if (!pSprite)
	{
		Warning("Got error in IsDotOn()!\n");
		return false;
	}

	return pSprite->IsOn();
}