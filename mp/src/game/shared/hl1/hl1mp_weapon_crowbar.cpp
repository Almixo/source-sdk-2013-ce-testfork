//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Crowbar - an old favorite
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hl1mp_basecombatweapon_shared.h"

#ifdef CLIENT_DLL
#include "c_baseplayer.h"
#else
#include "player.h"
#include "soundent.h"
#endif

#include "gamerules.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "in_buttons.h"

#include "vstdlib/random.h"

#include "decals.h"
#include "IEffects.h"

extern ConVar sk_plr_dmg_crowbar;

#define	CROWBAR_RANGE		64.0f
#define	CROWBAR_REFIRE_MISS	0.5f
#define	CROWBAR_REFIRE_HIT	0.25f


#ifdef CLIENT_DLL
#define CWeaponCrowbar C_WeaponCrowbar
#endif

//-----------------------------------------------------------------------------
// CWeaponCrowbar
//-----------------------------------------------------------------------------

class CWeaponCrowbar : public CBaseHL1MPCombatWeapon
{
	DECLARE_CLASS(CWeaponCrowbar, CBaseHL1MPCombatWeapon);
public:
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif

	CWeaponCrowbar();

	void			Precache(void);
	virtual void	ItemPostFrame(void);
	void			PrimaryAttack(void);

public:
	trace_t		m_traceHit;
	Activity	m_nHitActivity;

private:
	virtual void		Swing(void);
	virtual	void		Hit(void);
	virtual	void		ImpactEffect(void);
	virtual Activity	ChooseIntersectionPointAndActivity(trace_t& hitTrace, const Vector& mins, const Vector& maxs, CBasePlayer* pOwner);

public:

};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponCrowbar, DT_WeaponCrowbar);

BEGIN_NETWORK_TABLE(CWeaponCrowbar, DT_WeaponCrowbar)
/// what
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponCrowbar)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_crowbar, CWeaponCrowbar);
PRECACHE_WEAPON_REGISTER(weapon_crowbar);

#ifndef CLIENT_DLL
BEGIN_DATADESC(CWeaponCrowbar)

// DEFINE_FIELD( m_trLineHit, trace_t ),
// DEFINE_FIELD( m_trHullHit, trace_t ),
// DEFINE_FIELD( m_nHitActivity, FIELD_INTEGER ),
// DEFINE_FIELD( m_traceHit, trace_t ),

// Class CWeaponCrowbar:
// DEFINE_FIELD( m_nHitActivity, FIELD_INTEGER ),

// Function Pointers
DEFINE_FUNCTION(Hit),

END_DATADESC()
#endif

#define BLUDGEON_HULL_DIM		16

static const Vector g_bludgeonMins(-BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM);
static const Vector g_bludgeonMaxs(BLUDGEON_HULL_DIM, BLUDGEON_HULL_DIM, BLUDGEON_HULL_DIM);

#ifndef CLIENT_DLL
void HandleSound( trace_t *ptr )
{
	// hit the world, try to play sound based on texture material type

	char chTextureType;
	char *strTextureSound = "";
	float fvol;
	float fvolbar;

	if ( !g_pGameRules->PlayTextureSounds() )
	{
		fvolbar = 0.6;

		CSoundParameters params;
		if ( CBaseEntity::GetParametersForSound( "Weapon_Crowbar.Melee_HitWorld", params, nullptr ) )
		{
			params.volume = fvolbar;

			// play the crowbar sound
			UTIL_EmitAmbientSound( 0, ptr->endpos, params.soundname, fvolbar, params.soundlevel, 0, params.pitch );
		}

		return;
	}

	CBaseEntity *pEntity = ptr->m_pEnt;

	chTextureType = 0;

	if ( pEntity && pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE )
		// hit body
		chTextureType = CHAR_TEX_FLESH;
	else
		chTextureType = TEXTURETYPE_Find( ptr );

	switch ( chTextureType )
	{
	default:
	case CHAR_TEX_CONCRETE:
		fvol = 0.9;
		fvolbar = 0.6;
		strTextureSound = "HL1.Concrete";
		break;
	case CHAR_TEX_METAL:
		fvol = 0.9;
		fvolbar = 0.3;
		strTextureSound = "HL1.Metal";
		break;
	case CHAR_TEX_DIRT:
		fvol = 0.9;
		fvolbar = 0.1;
		strTextureSound = "HL1.Dirt";
		break;
	case CHAR_TEX_VENT:
		fvol = 0.5;
		fvolbar = 0.3;
		strTextureSound = "HL1.Vent";
		break;
	case CHAR_TEX_GRATE:
		fvol = 0.9;
		fvolbar = 0.5;
		strTextureSound = "HL1.Grate";
		break;
	case CHAR_TEX_TILE:
		fvol = 0.8;
		fvolbar = 0.2;
		strTextureSound = "HL1.Tile";
		break;
	case CHAR_TEX_SLOSH:
		fvol = 0.9;
		fvolbar = 0.0;
		strTextureSound = "HL1.Slosh";
		break;
	case CHAR_TEX_WOOD:
		fvol = 0.9;
		fvolbar = 0.2;
		strTextureSound = "HL1.Wood";
		break;
	case CHAR_TEX_GLASS:
	case CHAR_TEX_COMPUTER:
		fvol = 0.8;
		fvolbar = 0.2;
		strTextureSound = "HL1.Glass_Computer";
		break;
	case CHAR_TEX_FLESH:
		fvol = 1.0;
		fvolbar = 0.0;
		strTextureSound = "Weapon_Crowbar.Melee_Hit";
		break;
	}

	// did we hit a breakable?

	if ( pEntity && FClassnameIs( pEntity, "func_breakable" ) )
	{
		// drop volumes, the object will already play a damaged sound
		fvol /= 1.5;
		fvolbar /= 2.0;
	}
	else if ( chTextureType == CHAR_TEX_COMPUTER )
	{
		// play random spark if computer

		if ( ptr->fraction != 1.0 && RandomInt( 0, 1 ) )
		{
			g_pEffects->Sparks( ptr->endpos );
		}
	}

	// play the surface sound
	UTIL_EmitAmbientSound( 0, ptr->endpos, strTextureSound, fvol, SNDLVL_80dB, 0, 96 + RandomInt( 0, 15 ) );

	CSoundParameters params;
	if ( CBaseEntity::GetParametersForSound( "Weapon_Crowbar.Melee_HitWorld", params, nullptr ) )
	{
		params.volume = fvolbar;

		// play the crowbar sound
		UTIL_EmitAmbientSound( 0, ptr->endpos, params.soundname, fvolbar, params.soundlevel, 0, params.pitch );
	}
}
#endif

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponCrowbar::CWeaponCrowbar()
{
	m_bFiresUnderwater = true;
}

//-----------------------------------------------------------------------------
// Purpose: Precache the weapon
//-----------------------------------------------------------------------------
void CWeaponCrowbar::Precache(void)
{
	//Call base class first
	BaseClass::Precache();

	//crowbar
	CBaseEntity::PrecacheScriptSound( "HL1.Concrete" );
	CBaseEntity::PrecacheScriptSound( "HL1.Metal" );
	CBaseEntity::PrecacheScriptSound( "HL1.Dirt" );
	CBaseEntity::PrecacheScriptSound( "HL1.Vent" );
	CBaseEntity::PrecacheScriptSound( "HL1.Grate" );
	CBaseEntity::PrecacheScriptSound( "HL1.Tile" );
	CBaseEntity::PrecacheScriptSound( "HL1.Slosh" );
	CBaseEntity::PrecacheScriptSound( "HL1.Wood" );
	CBaseEntity::PrecacheScriptSound( "HL1.Glass_Computer" );
}

//------------------------------------------------------------------------------
// Purpose : Update weapon
//------------------------------------------------------------------------------
void CWeaponCrowbar::ItemPostFrame(void)
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	if ((pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		PrimaryAttack();
	}
	else
	{
		WeaponIdle();
		return;
	}
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CWeaponCrowbar::PrimaryAttack()
{
	Swing();
}


//------------------------------------------------------------------------------
// Purpose: Implement impact function
//------------------------------------------------------------------------------
void CWeaponCrowbar::Hit(void)
{
	//Make sound for the AI
#ifndef CLIENT_DLL

	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	CSoundEnt::InsertSound(SOUND_BULLET_IMPACT, m_traceHit.endpos, 400, 0.2f, pPlayer);

	CBaseEntity* pHitEntity = m_traceHit.m_pEnt;

	//Apply damage to a hit target
	if (pHitEntity != NULL)
	{
		Vector hitDirection;
		pPlayer->EyeVectors(&hitDirection, NULL, NULL);
		VectorNormalize(hitDirection);

		ClearMultiDamage();
		CTakeDamageInfo info(GetOwner(), GetOwner(), sk_plr_dmg_crowbar.GetFloat(), DMG_CLUB);
		CalculateMeleeDamageForce(&info, hitDirection, m_traceHit.endpos);
		pHitEntity->DispatchTraceAttack(info, hitDirection, &m_traceHit);
		ApplyMultiDamage();

		// Now hit all triggers along the ray that... 
		TraceAttackToTriggers(CTakeDamageInfo(GetOwner(), GetOwner(), sk_plr_dmg_crowbar.GetFloat(), DMG_CLUB), m_traceHit.startpos, m_traceHit.endpos, hitDirection);

		////Play an impact sound	
		//if (pHitEntity->Classify() != CLASS_NONE && pHitEntity->Classify() != CLASS_MACHINE)
		//{
		//	WeaponSound(MELEE_HIT);
		//}
		///*else if (pHitEntity->IsWorld())
		//	WeaponSound(MELEE_HIT_WORLD);*/

		/*if (pHitEntity->Classify() != CLASS_NONE && pHitEntity->Classify() != CLASS_MACHINE)
		{
			WeaponSound(MELEE_HIT);
		}
		else
		{
			CPASAttenuationFilter filter( pPlayer, ATTN_NORM );
			
		}*/
			
		HandleSound( &m_traceHit );

	}
#endif

	//Apply an impact effect
	ImpactEffect();
	//UTIL_DecalTrace( &m_traceHit, "Impact.Concrete");
}

Activity CWeaponCrowbar::ChooseIntersectionPointAndActivity(trace_t& hitTrace, const Vector& mins, const Vector& maxs, CBasePlayer* pOwner)
{
	int			i, j, k;
	float		distance;
	const float* minmaxs[2] = { mins.Base(), maxs.Base() };
	trace_t		tmpTrace;
	Vector		vecHullEnd = hitTrace.endpos;
	Vector		vecEnd;

	distance = 1e6f;
	Vector vecSrc = hitTrace.startpos;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc) * 2);
	UTIL_TraceLine(vecSrc, vecHullEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace);
	if (tmpTrace.fraction == 1.0)
	{
		for (i = 0; i < 2; i++)
		{
			for (j = 0; j < 2; j++)
			{
				for (k = 0; k < 2; k++)
				{
					vecEnd.x = vecHullEnd.x + minmaxs[i][0];
					vecEnd.y = vecHullEnd.y + minmaxs[j][1];
					vecEnd.z = vecHullEnd.z + minmaxs[k][2];

					UTIL_TraceLine(vecSrc, vecEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace);
					if (tmpTrace.fraction < 1.0)
					{
						float thisDistance = (tmpTrace.endpos - vecSrc).Length();
						if (thisDistance < distance)
						{
							hitTrace = tmpTrace;
							distance = thisDistance;
						}
					}
				}
			}
		}
	}
	else
	{
		hitTrace = tmpTrace;
	}


	return ACT_VM_HITCENTER;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCrowbar::ImpactEffect(void)
{
	//FIXME: need new decals
	/*UTIL_ImpactTrace(&m_traceHit, DMG_CLUB);
	DecalTrace( &m_traceHit, DMG_CLUB );*/

	//DecalTrace( &m_traceHit, "bigshot" );
	UTIL_ImpactTrace( &m_traceHit, DMG_CLUB );
}

//------------------------------------------------------------------------------
// Purpose : Starts the swing of the weapon and determines the animation
//------------------------------------------------------------------------------
void CWeaponCrowbar::Swing(void)
{
	// Try a ray
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	Vector swingStart = pOwner->Weapon_ShootPosition();
	Vector forward;

	pOwner->EyeVectors(&forward, NULL, NULL);

	Vector swingEnd = swingStart + forward * CROWBAR_RANGE;

	UTIL_TraceLine(swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &m_traceHit);
	m_nHitActivity = ACT_VM_HITCENTER;

	if (m_traceHit.fraction == 1.0)
	{
		float bludgeonHullRadius = 1.732f * BLUDGEON_HULL_DIM;  // hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point

		// Back off by hull "radius"
		swingEnd -= forward * bludgeonHullRadius;

		UTIL_TraceHull(swingStart, swingEnd, g_bludgeonMins, g_bludgeonMaxs, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &m_traceHit);
		if (m_traceHit.fraction < 1.0)
		{
			m_nHitActivity = ChooseIntersectionPointAndActivity(m_traceHit, g_bludgeonMins, g_bludgeonMaxs, pOwner);
		}
	}


	// -------------------------
	//	Miss
	// -------------------------
	if (m_traceHit.fraction == 1.0f)
	{
		m_nHitActivity = ACT_VM_MISSCENTER;

		//Play swing sound
		WeaponSound(SINGLE);

		//Setup our next attack times
		m_flNextPrimaryAttack = gpGlobals->curtime + CROWBAR_REFIRE_MISS;
	}
	else
	{
		Hit();

		//Setup our next attack times
		m_flNextPrimaryAttack = gpGlobals->curtime + CROWBAR_REFIRE_HIT;
	}

	//Send the anim
	SendWeaponAnim(m_nHitActivity);
	pOwner->SetAnimation(PLAYER_ATTACK1);
}
