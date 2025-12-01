#include "cbase.h"
#include "c_te_legacytempents.h"
#include "tempent.h"

class C_BazookaRocket : public C_BaseAnimating
{
	DECLARE_CLASS( C_BazookaRocket, C_BaseAnimating );
	DECLARE_CLIENTCLASS();
public:

	C_BazookaRocket();
	~C_BazookaRocket();

    void Spawn( void );
    void Precache( void );
	int DrawModel( int flags );
	void PostDataUpdate( DataUpdateType_t type );
private:

    float m_flFireTime;
    float m_flBeamTime;

    //int m_iTrail;
    bool m_bBeam;
};

IMPLEMENT_CLIENTCLASS_DT( C_BazookaRocket, DT_BazookaRocket, CBazookaRocket )
//RecvPropFloat( RECVINFO( m_flFireTime ) ),
END_RECV_TABLE()

C_BazookaRocket::C_BazookaRocket()
{
    m_flFireTime = 0.0f;
    m_flBeamTime = 0.0f;

    //m_iTrail = 0;
    m_bBeam = false;
}

C_BazookaRocket::~C_BazookaRocket()
{
    ParticleProp()->StopEmission();
}

void C_BazookaRocket::Spawn( void )
{
    Precache();

    m_flFireTime = gpGlobals->curtime + 0.05f;
    m_flBeamTime = gpGlobals->curtime + 0.1f;

    BaseClass::Spawn();
}

void C_BazookaRocket::Precache()
{
    BaseClass::Precache();

    //m_iTrail = PrecacheModel( "sprites/smoke.vmt" );
}

int C_BazookaRocket::DrawModel( int flags )
{
    if ( m_flFireTime > gpGlobals->curtime )
        return 0;

    if ( !m_bBeam && ( gpGlobals->curtime >= m_flBeamTime ) )
    {
        m_bBeam = true;

     /*   CPASFilter filter( GetAbsOrigin() );
        te->BeamFollow( filter, 0.0f,
            entindex(),
            m_iTrail,
            0,
            4,
            5,
            5,
            0,
            224,
            224,
            255,
            255 );*/

        Vector pos;
        QAngle ang;
        GetAttachment( 1, pos, ang );

        tempents->RocketFlare( pos );
    }

    return BaseClass::DrawModel( flags );
}

void C_BazookaRocket::PostDataUpdate( DataUpdateType_t type )
{
    BaseClass::PostDataUpdate( type );

    if ( type == DATA_UPDATE_CREATED )
    {
        ParticleProp()->Create( "rockettrail1", PATTACH_POINT_FOLLOW, 1 );
    }
}
