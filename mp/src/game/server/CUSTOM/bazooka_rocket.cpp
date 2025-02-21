#include "cbase.h"
#include "bazooka_rocket.h"

#define RICOCHET_SOUND "Metal_Barrel.ImpactHard"

BEGIN_DATADESC( CBazookaRocket )
// Function Pointers
DEFINE_FUNCTION( RocketTouch ),

DEFINE_THINKFUNC( FlyThink ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CBazookaRocket, DT_BazookaRocket )
//SendPropFloat( SENDINFO( m_flFireTime ) ),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( bazooka_rocket, CBazookaRocket );

CBazookaRocket::CBazookaRocket()
{
    g_sModelIndexWExplosion = 0;
}

void CBazookaRocket::Spawn( void )
{
	Precache();

	BaseClass::Spawn();

	SetModel( ROCKET_MODEL );
	UTIL_SetSize( this, -Vector( 1.5f, 1.5f, 1.5f ), Vector( 1.5f, 1.5f, 1.5f ) );

	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );
	SetGravity( 0.1 );

	SetSolid( SOLID_BBOX );
    AddSolidFlags( FSOLID_NOT_STANDABLE );

	UpdateWaterState();

	SetTouch( &CBazookaRocket::RocketTouch );

	CPASAttenuationFilter filter( this );
	EmitSound( filter, entindex(), ROCKET_SOUND );

	SetThink( &CBazookaRocket::FlyThink );
	SetNextThink( gpGlobals->curtime );
}

void CBazookaRocket::Precache( void )
{
	BaseClass::Precache();

	PrecacheModel( ROCKET_MODEL );
	PrecacheScriptSound( ROCKET_SOUND );
    PrecacheScriptSound( RICOCHET_SOUND );

    PrecacheParticleSystem( "rockettrail1" );

	g_sModelIndexWExplosion = PrecacheModel( "sprites/WXplo1.vmt" );
}

unsigned int CBazookaRocket::PhysicsSolidMaskForEntity() const
{
	return BaseClass::PhysicsSolidMaskForEntity() & ~CONTENTS_GRATE;
}

bool CBazookaRocket::CreateVPhysics( void )
{
    VPhysicsInitNormal( SOLID_BBOX, FSOLID_NOT_STANDABLE, false );

	return true;
}

void CBazookaRocket::RocketTouch( CBaseEntity *pOther )
{
    if ( pOther->GetSolidFlags() & FSOLID_NOT_SOLID || pOther->GetSolidFlags() & FSOLID_TRIGGER )
        return;

    trace_t tr;
    tr = GetTouchTrace();

    if ( pOther->m_takedamage == DAMAGE_NO )
    {
        if ( tr.surface.flags & SURF_SKY )
        {
            SetThink( &CBazookaRocket::SUB_Remove );
            SetNextThink( gpGlobals->curtime );

            return;
        }

        Vector vecDir = GetAbsVelocity();
        float speed = VectorNormalize( vecDir );

        float hitDot = DotProduct( tr.plane.normal, -vecDir );

        if ( ( hitDot < 0.17f ) )
        {
            Vector vReflection = 1.5f * tr.plane.normal * hitDot + vecDir;

            QAngle reflectAngles;

            VectorAngles( vReflection, reflectAngles );

            SetLocalAngles( reflectAngles );

            speed *= 0.9f;

            if ( speed < 900.0f ) // clamp somehow didn't work?
                speed = 900.0f;

            SetAbsVelocity( vReflection * speed );

            SetGravity( 0.15f );

            Vector vecPos = GetAbsOrigin();
            CPASFilter filter( vecPos );
            te->Sparks( filter, 0.0f, &vecPos, 1.0f, 4.0f, &tr.plane.normal);

            return;
        }
    }

	Explode( &tr );

	UTIL_Remove( this );
}

void CBazookaRocket::FlyThink( void )
{
    if ( GetWaterLevel() == 3 )
    {
        Vector vecVel = GetAbsVelocity().Normalized();
        SetAbsVelocity( vecVel * 750.0f );
    }

	QAngle angNewAngles;

	VectorAngles( GetAbsVelocity(), angNewAngles );
	SetAbsAngles( angNewAngles );

	SetNextThink( gpGlobals->curtime + 0.1f );
}

void CBazookaRocket::Explode( trace_t *tr )
{
	//ExplosionCreate( GetAbsOrigin(), GetAbsAngles(), GetOwnerEntity(), 100, 100, true, 100.0f );

	StopSound( entindex(), ROCKET_SOUND );

	Vector vecAbsOrigin = GetAbsOrigin();

	Vector vecNormal = tr->plane.normal;
	surfacedata_t *pdata = physprops->GetSurfaceData( tr->surface.surfaceProps );

    int contents = UTIL_PointContents( tr->endpos );

	CPASFilter filter( vecAbsOrigin );
	te->Explosion( filter, 0.0f,
		&vecAbsOrigin,
		!( contents & MASK_WATER ) ? g_sModelIndexFireball : g_sModelIndexWExplosion,
		100 * .03,
		25,
		TE_EXPLFLAG_NONE,
		100,
		100,
		&vecNormal,
		(char)pdata->game.material );

    CBaseEntity *m_pEnts[64];
    int ent_cnt = UTIL_EntitiesInSphere( m_pEnts, 64, GetAbsOrigin(), 96.0f, 0 );

    for ( int i = 0; i < ent_cnt; i++ )
    {
        CBaseEntity *pEnt = m_pEnts[i];
        if ( !pEnt )
            continue;

        if ( pEnt->IsWorld() )
            continue;

        if ( !pEnt->IsAlive() )
            continue;

        if ( pEnt->m_takedamage == DAMAGE_NO )
            continue;

        CTakeDamageInfo info( this, GetOwnerEntity(), 100.0f, DMG_BLAST );
        pEnt->TakeDamage( info );
    }

	UTIL_DecalTrace( tr, "Scorch" );
}

CBazookaRocket *CBazookaRocket::Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner )
{
	CBazookaRocket *pRocket = dynamic_cast<CBazookaRocket*>( CBaseEntity::Create( "bazooka_rocket", vecOrigin, vecAngles ) );
	pRocket->SetOwnerEntity( pOwner );
	pRocket->Spawn();

	return pRocket;
}