//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Crossbow
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hl1_basecombatweapon_shared.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "entitylist.h"
#include "game.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "IEffects.h"
#include "te_effect_dispatch.h"

#define BOLT_MODEL			"models/crossbow_bolt.mdl"

#define BOLT_AIR_VELOCITY	2000
#define BOLT_WATER_VELOCITY	1000

extern ConVar sk_plr_dmg_xbow_bolt_plr;
extern ConVar sk_plr_dmg_xbow_bolt_npc;

extern short	g_sModelIndexFireball;		// (in combatweapon.cpp) holds the index for the fireball

//-----------------------------------------------------------------------------
// Crossbow Bolt
//-----------------------------------------------------------------------------
class CCrossbowBolt : public CBaseCombatCharacter
{
	DECLARE_CLASS( CCrossbowBolt, CBaseCombatCharacter );

public:
	CCrossbowBolt()
    {
        m_bExplode = true;
    }

	Class_T Classify( void ) { return CLASS_NONE; }

public:
	void Spawn( void );
	void Precache( void );
	void BubbleThink( void );
	void BoltTouch( CBaseEntity *pOther );
    void ExplodeThink( void );
    void SetExplode( bool bVal ) { m_bExplode = bVal; }

	static CCrossbowBolt *BoltCreate( const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner = NULL );

	DECLARE_DATADESC();

private:
    bool m_bExplode;
};
LINK_ENTITY_TO_CLASS( crossbow_bolt, CCrossbowBolt );

BEGIN_DATADESC( CCrossbowBolt )
	// Function Pointers
	DEFINE_FUNCTION( BubbleThink ),
	DEFINE_FUNCTION( BoltTouch ),
    DEFINE_FUNCTION( ExplodeThink ),
END_DATADESC()

CCrossbowBolt *CCrossbowBolt::BoltCreate( const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner )
{
	// Create a new entity with CCrossbowBolt private data
	CCrossbowBolt *pBolt = (CCrossbowBolt *)CreateEntityByName( "crossbow_bolt" );
	UTIL_SetOrigin( pBolt, vecOrigin );
	pBolt->SetAbsAngles( angAngles );
	pBolt->Spawn();
	pBolt->SetOwnerEntity( pentOwner );

	return pBolt;
}

void CCrossbowBolt::Spawn( )
{
	Precache( );

	SetModel( BOLT_MODEL );
	UTIL_SetSize( this, -Vector(1, 1, 1), Vector(1, 1, 1) );

	SetSolid( SOLID_BBOX );
	SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
	SetGravity(UTIL_ScaleForGravity(40));

	SetTouch( &CCrossbowBolt::BoltTouch );

	SetThink( &CCrossbowBolt::BubbleThink );
	SetNextThink( gpGlobals->curtime + 0.2 );

    m_bExplode = true;
}


void CCrossbowBolt::Precache( )
{
	PrecacheModel( BOLT_MODEL );
	PrecacheScriptSound( "BaseGrenade.Explode" );    
}

void CCrossbowBolt::BoltTouch( CBaseEntity *pOther )
{
	if ( !pOther->IsSolid() || pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS) )
		return;

	SetTouch( NULL );
	SetThink( NULL );

	if ( pOther->m_takedamage != DAMAGE_NO )
	{
		trace_t tr, tr2;
		tr = BaseClass::GetTouchTrace( );
		Vector	vecNormalizedVel = GetAbsVelocity();

		ClearMultiDamage( );
		VectorNormalize( vecNormalizedVel );

		if ( pOther->IsPlayer() )
		{
            float m_fDamage = sk_plr_dmg_xbow_bolt_plr.GetFloat() * g_pGameRules->GetDamageMultiplier();

            // If multiplayer sniper bolt, multiply damage by 4
            if ( g_pGameRules->IsMultiplayer() && !m_bExplode )
                m_fDamage *= 4;
            
			CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), m_fDamage,DMG_NEVERGIB );
			CalculateMeleeDamageForce( &dmgInfo, vecNormalizedVel, tr.endpos );
			pOther->DispatchTraceAttack( dmgInfo, vecNormalizedVel, &tr );
		}
		else
		{
			CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), sk_plr_dmg_xbow_bolt_npc.GetFloat() * g_pGameRules->GetDamageMultiplier(), DMG_BULLET | DMG_NEVERGIB );
			CalculateMeleeDamageForce( &dmgInfo, vecNormalizedVel, tr.endpos );
			pOther->DispatchTraceAttack( dmgInfo, vecNormalizedVel, &tr );
		}

		ApplyMultiDamage();

		SetAbsVelocity( Vector( 0, 0, 0 ) );

		// play body "thwack" sound
		EmitSound( "Weapon_Crossbow.BoltHitBody" );

		SetThink( &CCrossbowBolt::SUB_Remove );
		SetNextThink( gpGlobals->curtime );// this will get changed below if the bolt is allowed to stick in what it hit.

		Vector vForward;

		AngleVectors( GetAbsAngles(), &vForward );
		VectorNormalize ( vForward );
		
		UTIL_TraceLine( GetAbsOrigin(),	GetAbsOrigin() + vForward * 128, MASK_OPAQUE, pOther, COLLISION_GROUP_NONE, &tr2 );

		if ( tr2.fraction != 1.0f )
		{
//			NDebugOverlay::Box( tr2.endpos, Vector( -16, -16, -16 ), Vector( 16, 16, 16 ), 0, 255, 0, 0, 10 );
//			NDebugOverlay::Box( GetAbsOrigin(), Vector( -16, -16, -16 ), Vector( 16, 16, 16 ), 0, 0, 255, 0, 10 );

			CEffectData	data;

			data.m_vOrigin = tr2.endpos;
			data.m_vNormal = vForward;
			data.m_nEntIndex = tr2.fraction != 1.0f;
		
			DispatchEffect( "BoltImpact", data );
		}

        //		if ( !g_pGameRules->IsMultiplayer() )
        if ( g_pGameRules->IsMultiplayer() && !m_bExplode )
		{
			UTIL_Remove( this );
		}
	}
	else
	{
		EmitSound( "Weapon_Crossbow.BoltHitWorld" );

		SetThink( &CCrossbowBolt::SUB_Remove );
		SetNextThink( gpGlobals->curtime );// this will get changed below if the bolt is allowed to stick in what it hit.

		if ( m_bExplode == false )
		{
			Vector vForward;
			AngleVectors( GetAbsAngles(), &vForward );
			VectorNormalize ( vForward );

			CEffectData	data;

			data.m_vOrigin = GetAbsOrigin();
			data.m_vNormal = vForward;
			data.m_nEntIndex = 0;

			DispatchEffect( "BoltImpact", data );
		}

		if (  UTIL_PointContents( GetAbsOrigin() ) != CONTENTS_WATER)
		{
			g_pEffects->Sparks( GetAbsOrigin() );
		}
	}

    // Set up an explosion in one tenth of a second
	if ( g_pGameRules->IsMultiplayer() && m_bExplode )
	{
        SetThink( &CCrossbowBolt::ExplodeThink );
        SetNextThink( gpGlobals->curtime + 0.1f );
	}
}

void CCrossbowBolt::ExplodeThink( void )
{
    //    int iContents = UTIL_PointContents( pev->origin );
    CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), sk_plr_dmg_xbow_bolt_npc.GetFloat() * g_pGameRules->GetDamageMultiplier(), DMG_BLAST );

    ::RadiusDamage( dmgInfo, GetAbsOrigin(), 128, CLASS_NONE, NULL );

    CPASFilter filter( GetAbsOrigin() );

    te->Explosion( filter,                /* filter */
                   0.0,                   /* delay  */
                   &GetAbsOrigin(),       /* pos    */
                   g_sModelIndexFireball, /* modelindex */
                   0.2,                   /* scale  */
                   25,                    /* framerate */
                   TE_EXPLFLAG_NONE,      /* flags */
                   128,                   /* radius */
                   64,                    /* magnitude */
                   NULL,                  /* normal */
                   'C' );                 /* materialType */

	//CSoundEnt::InsertSound ( SOUND_COMBAT, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0 );
	EmitSound( "BaseGrenade.Explode" );    
    
    SetThink( &CCrossbowBolt::SUB_Remove );
    SetNextThink( gpGlobals->curtime );
    
    AddEffects( EF_NODRAW );
}

void CCrossbowBolt::BubbleThink( void )
{
	QAngle angNewAngles;

	VectorAngles( GetAbsVelocity(), angNewAngles );
	SetAbsAngles( angNewAngles );

	SetNextThink( gpGlobals->curtime + 0.1f );

	if ( GetWaterLevel()  == 0 )
		return;

	UTIL_BubbleTrail( GetAbsOrigin() - GetAbsVelocity() * 0.1, GetAbsOrigin(), 1 );
}

//-----------------------------------------------------------------------------
// CWeaponCrossbow
//-----------------------------------------------------------------------------

class CWeaponCrossbow : public CBaseHL1CombatWeapon
{
	DECLARE_CLASS( CWeaponCrossbow, CBaseHL1CombatWeapon );
public:

	CWeaponCrossbow( void );

	void	Precache( void );
	void	PrimaryAttack( void );
	void	SecondaryAttack( void );
	bool	Reload( void );
	void	WeaponIdle( void );
	bool	Deploy( void );
	bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

private:
	void	FireBolt( void );
	void	ToggleZoom( void );

	bool	m_bInZoom;
};
LINK_ENTITY_TO_CLASS( weapon_crossbow, CWeaponCrossbow );

PRECACHE_WEAPON_REGISTER( weapon_crossbow );

IMPLEMENT_SERVERCLASS_ST( CWeaponCrossbow, DT_WeaponCrossbow )
END_SEND_TABLE()

BEGIN_DATADESC( CWeaponCrossbow )
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponCrossbow::CWeaponCrossbow( void )
{
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= true;
	m_bInZoom			= false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCrossbow::Precache( void )
{
	UTIL_PrecacheOther( "crossbow_bolt" );

	PrecacheScriptSound( "Weapon_Crossbow.BoltHitBody" );
	PrecacheScriptSound( "Weapon_Crossbow.BoltHitWorld" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponCrossbow::PrimaryAttack( void )
{
	if ( m_bInZoom && g_pGameRules->IsMultiplayer() )
	{
		FireBolt();
	}
	else
	{
		FireBolt();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponCrossbow::SecondaryAttack( void )
{
	ToggleZoom();
	m_flNextSecondaryAttack = gpGlobals->curtime + 1.0;
}


void CWeaponCrossbow::FireBolt( void )
{
	if ( m_iClip1 <= 0 )
	{
		if ( !m_bFireOnEmpty )
		{
			Reload();
		}
		else
		{
			WeaponSound( EMPTY );
			m_flNextPrimaryAttack = 0.15;
		}

		return;
	}

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner == NULL ) return;

	Vector vecAiming	= pOwner->GetAutoaimVector( AUTOAIM_2DEGREES );	
	Vector vecSrc		= pOwner->Weapon_ShootPosition();

	QAngle angAiming;
	VectorAngles( vecAiming, angAiming );

	CCrossbowBolt *pBolt = CCrossbowBolt::BoltCreate( vecSrc, angAiming, pOwner );

    // In multiplayer, secondary fire is instantaneous.
    if ( g_pGameRules->IsMultiplayer() && m_bInZoom )
    {
        Vector vecEnd = vecSrc + ( vecAiming * MAX_TRACE_LENGTH );
        
        trace_t trace;
        UTIL_TraceLine( vecSrc, vecEnd, MASK_SHOT, GetOwner(), COLLISION_GROUP_NONE, &trace );
        pBolt->SetAbsOrigin( trace.endpos );

        // We hit someone
        if ( trace.m_pEnt && trace.m_pEnt->m_takedamage )
        {
            pBolt->SetExplode( false );            
            pBolt->BoltTouch( trace.m_pEnt );
            return;
        }
    }
    else
    {
        if ( pOwner->GetWaterLevel() == 3 )
        {
            pBolt->SetAbsVelocity( vecAiming * BOLT_WATER_VELOCITY );
        }
        else
        {
            pBolt->SetAbsVelocity( vecAiming * BOLT_AIR_VELOCITY );
        }
    }

	pBolt->SetLocalAngularVelocity( QAngle( 0, 0, 10 ) );

    if ( m_bInZoom )
        pBolt->SetExplode( false );

	m_iClip1--;

	pOwner->ViewPunch( QAngle( -2, 0, 0 ) );

	WeaponSound( SINGLE );
	WeaponSound( RELOAD );

	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), 200, 0.2 );

	if ( m_iClip1 > 0 )
	{
		SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	}
	else if ( pOwner->GetAmmoCount(m_iPrimaryAmmoType) > 0 )
	{
		SendWeaponAnim( ACT_VM_SECONDARYATTACK );
	}

	if ( !m_iClip1 && pOwner->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
	{
		// HEV suit - indicate out of ammo condition
		pOwner->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	m_flNextPrimaryAttack	= gpGlobals->curtime + 0.75;
	m_flNextSecondaryAttack	= gpGlobals->curtime + 0.75;

	if ( m_iClip1 > 0 )
	{
		SetWeaponIdleTime( gpGlobals->curtime + 5.0 );
	}
	else
	{
		SetWeaponIdleTime( gpGlobals->curtime + 0.75 );
	}
}


bool CWeaponCrossbow::Reload( void )
{
	bool fRet;

	fRet = DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );
	if ( fRet )
	{
		if ( m_bInZoom )
		{
			ToggleZoom();
		}

		WeaponSound( RELOAD );
	}

	return fRet;
}


void CWeaponCrossbow::WeaponIdle( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( !pPlayer ) return;
	

	pPlayer->GetAutoaimVector( AUTOAIM_2DEGREES );

	if ( !HasWeaponIdleTimeElapsed() )
		return;

	int iAnim;
	float flRand = random->RandomFloat( 0, 1 );
	if ( flRand <= 0.75 )
	{
		if ( m_iClip1 <= 0 )
			iAnim = ACT_CROSSBOW_IDLE_UNLOADED;
		else
			iAnim = ACT_VM_IDLE;
	}
	else
	{
		if ( m_iClip1 <= 0 )
			iAnim = ACT_CROSSBOW_FIDGET_UNLOADED;
		else
			iAnim = ACT_VM_FIDGET;
	}

	SendWeaponAnim( iAnim );
}


bool CWeaponCrossbow::Deploy( void )
{
	if ( m_iClip1 <= 0 )
	{
		return DefaultDeploy( (char*)GetViewModel(), (char*)GetWorldModel(), ACT_CROSSBOW_DRAW_UNLOADED, (char*)GetAnimPrefix() );
	}

	return DefaultDeploy( (char*)GetViewModel(), (char*)GetWorldModel(), ACT_VM_DRAW, (char*)GetAnimPrefix() );
}


bool CWeaponCrossbow::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	if ( m_bInZoom )
		ToggleZoom();

	return BaseClass::Holster( pSwitchingTo );
}


void CWeaponCrossbow::ToggleZoom( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( pPlayer == NULL ) return;

	if ( m_bInZoom )
	{
		if ( pPlayer->SetFOV( this, 0 ) )
		{
			m_bInZoom = false;
		}
	}
	else
	{
		if ( pPlayer->SetFOV( this, 20 ) )
		{
			m_bInZoom = true;
		}
	}
}