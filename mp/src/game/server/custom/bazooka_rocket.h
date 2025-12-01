#pragma once

#include "cbase.h"
#include "te.h"
#include "explode.h"
#include "soundent.h"
#include "smoke_trail.h"

#define ROCKET_MODEL "models/w_bazooka_rocket.mdl"
#define ROCKET_SOUND "BazookaRocket.Fire"

class CBazookaRocket : public CBaseAnimating
{
    DECLARE_CLASS( CBazookaRocket, CBaseAnimating );
    DECLARE_DATADESC();
    
    DECLARE_SERVERCLASS();
public:
    CBazookaRocket();

    void Spawn( void );
    void Precache( void );

    static CBazookaRocket *Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner );

private:

    unsigned int PhysicsSolidMaskForEntity() const;
    bool CreateVPhysics( void );

    void FlyThink( void );
    void RocketTouch( CBaseEntity *pOther );

    void Explode( trace_t *tr );

    short g_sModelIndexWExplosion;

    EHANDLE m_hRocketTrail;
};