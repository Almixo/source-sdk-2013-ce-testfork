#ifndef TEH_CHUMTOAD_H
#define TEH_CHUMTOAD_H

#include "cbase.h"
#include "hl1_ai_basenpc.h"


class CChumToad : public CHL1BaseNPC
{
	DECLARE_CLASS( CChumToad, CHL1BaseNPC );
public:

	DECLARE_DATADESC();

	void	Precache( void );
	void	Spawn( void );
	Class_T Classify( void );
	void	Event_Killed( const CTakeDamageInfo &info );
	bool	Event_Gibbed( const CTakeDamageInfo &info );
	void	HuntThink( void );
	void	SuperBounceTouch( CBaseEntity *pOther );

	virtual void ResolveFlyCollisionCustom( trace_t &trace, Vector &vecVelocity );

	virtual unsigned int PhysicsSolidMaskForEntity( void ) const;

	virtual bool ShouldGib( const CTakeDamageInfo &info ) { return false; }
	static float	m_flNextBounceSoundTime;

private:
	Class_T	m_iMyClass;
	float	m_flDie;
	Vector	m_vecTarget;
	float	m_flNextHunt;
	float	m_flNextHit;
	Vector	m_posPrev;
	EHANDLE m_hOwner;
};

#endif //TEH_CHUMTOAD_H