#include "cbase.h"
#include "CBaseDoDWeapon_shared.h"

#ifndef CLIENT_DLL
#include "bazooka_rocket.h"
#endif

#ifdef CLIENT_DLL
#define CWeaponBazooka C_WeaponBazooka
#endif

class CWeaponBazooka : public CBaseDoDCombatWeapon
{
public:
    DECLARE_CLASS( CWeaponBazooka, CBaseDoDCombatWeapon );
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();
    DECLARE_DATADESC();

    CWeaponBazooka();

    void Precache();
    void ItemPostFrame();
    bool Reload();
    void FinishReload();
    void WeaponIdle();
    bool Holster( CBaseCombatWeapon *pOther );

    bool CheckPrimaryAttack( void );
    bool CheckSecondaryAttack( void );

    void ShoulderBazooka();
    void UnshoulderBazooka();
    void FireRocket();

    //bool m_bShouldered;
    //bool m_bShouldering;

    CNetworkVar( bool, m_bShouldered );
    CNetworkVar( bool, m_bShouldering );

    CNetworkVar( float, m_flShoulderTime );

    static const float m_flShoulderDuration;
};

const float CWeaponBazooka::m_flShoulderDuration = 1.0f;


BEGIN_DATADESC( CWeaponBazooka )
//DEFINE_FIELD( m_bShouldered, FIELD_BOOLEAN ),
//DEFINE_FIELD( m_bShouldering, FIELD_BOOLEAN ),
//DEFINE_FIELD( m_flShoulderTime, FIELD_TIME ),
END_DATADESC()

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponBazooka, DT_WeaponBazooka );

BEGIN_NETWORK_TABLE( CWeaponBazooka, DT_WeaponBazooka )
#ifndef CLIENT_DLL
SendPropBool( SENDINFO( m_bShouldered ) ),
SendPropBool( SENDINFO( m_bShouldering ) ),
SendPropFloat( SENDINFO( m_flShoulderTime ) ), 
#else
RecvPropBool( RECVINFO( m_bShouldered ) ),
RecvPropBool( RECVINFO( m_bShouldering ) ),
RecvPropFloat( RECVINFO( m_flShoulderTime ) ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponBazooka )
#ifdef CLIENT_DLL
DEFINE_PRED_FIELD( m_bShouldered, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_bShouldering, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_flShoulderTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
#endif
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_bazooka, CWeaponBazooka );

PRECACHE_WEAPON_REGISTER( weapon_bazooka );

CWeaponBazooka::CWeaponBazooka()
{
    m_bShouldered = false;
    m_bShouldering = false;
    m_flShoulderTime = 0.0f;
}

void CWeaponBazooka::Precache()
{
    BaseClass::Precache();
#ifndef CLIENT_DLL
    PrecacheModel( ROCKET_MODEL );
#endif
}


bool CWeaponBazooka::CheckPrimaryAttack( void )
{
    if ( m_bInReload )
        return false;

    if ( !m_bShouldered || m_bShouldering )
        return false;

    if ( m_flNextPrimaryAttack > gpGlobals->curtime )
        return false;

    if ( m_iClip1 <= 0 )
        return false;

    return true;
}

bool CWeaponBazooka::CheckSecondaryAttack( void )
{
    if ( m_bInReload )
        return false;

    if ( m_bShouldering )
        return false;

    if ( m_flNextSecondaryAttack > gpGlobals->curtime )
        return false;

    return true;
}

// BaseClass::ItemPostFrame() rip below
void CWeaponBazooka::ItemPostFrame()
{
    CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
    if ( !pOwner )
        return;

    if ( m_bShouldering && ( m_flShoulderTime + m_flShoulderDuration ) <= gpGlobals->curtime )
    {
        m_bShouldering = false;
    }

    CheckReload();

    if ( ( pOwner->m_nButtons & IN_ATTACK2 ) && CheckSecondaryAttack() )
    {
        if ( m_bShouldered )
            UnshoulderBazooka();
        else
            ShoulderBazooka();
    }
    else if ( ( pOwner->m_nButtons & IN_ATTACK ) && CheckPrimaryAttack() )
    {
        if ( pOwner->GetWaterLevel() == 3 && m_bFiresUnderwater == false )
        {
            WeaponSound( EMPTY );
            m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
            m_flNextEmptySoundTime = gpGlobals->curtime + 1.0f;
            return;
        }
        else
        {
            FireRocket();

#ifdef CLIENT_DLL
            pOwner->SetFiredWeapon( true );
#endif
        }
    }

    if ( ( pOwner->m_nButtons & IN_RELOAD ) && m_flNextPrimaryAttack <= gpGlobals->curtime && !m_bInReload && !m_bShouldering && m_iClip1 == 0 )
    {
        Reload();
        m_fFireDuration = 0.0f;
    }

    // jesus
    if ( !( ( pOwner->m_nButtons & IN_ATTACK ) || ( pOwner->m_nButtons & IN_ATTACK2 ) || ( CanReload() && pOwner->m_nButtons & IN_RELOAD ) || m_bShouldering || ( m_flNextPrimaryAttack > gpGlobals->curtime ) || ( m_iClip1 == 0 ) ) )
    {
        if ( !ReloadOrSwitchWeapons() && ( m_bInReload == false ) )
        {
            WeaponIdle();
        }
    }
}

// Reload: Called when the player presses the reload button
bool CWeaponBazooka::Reload()
{
    CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
    if ( !pPlayer )
        return false;

    int anim;

    if ( m_bShouldered )
        anim = ACT_VM_RELOAD_DEPLOYED;
    else
        anim = ACT_VM_RELOAD;
        

    bool reloaded = BaseClass::DefaultReload( GetMaxClip1(), GetMaxClip2(), anim );

    if ( reloaded )
    {
        WeaponSound( RELOAD );
        m_flNextPrimaryAttack = gpGlobals->curtime + 3.0f;

        if ( m_bShouldered )
            pPlayer->SetMaxSpeed( 320 );
    }

    return reloaded;
}

void CWeaponBazooka::FinishReload()
{
    CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
    if ( !pPlayer )
        return;

    if ( m_bShouldered )
        pPlayer->SetMaxSpeed( 100 );

    BaseClass::FinishReload();
}

// ShoulderBazooka: Start the shouldering process
void CWeaponBazooka::ShoulderBazooka()
{
    CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
    if ( !pPlayer )
        return;

    pPlayer->SetMaxSpeed( 100.0f );

    SendWeaponAnim( ACT_VM_DEPLOY );

    m_bShouldered = true;
    m_bShouldering = true;

    m_flShoulderTime = gpGlobals->curtime;

    m_flNextPrimaryAttack = gpGlobals->curtime + m_flShoulderDuration;
    m_flNextSecondaryAttack = gpGlobals->curtime + m_flShoulderDuration;
    m_flTimeWeaponIdle = gpGlobals->curtime + m_flShoulderDuration;
}

// UnshoulderBazooka: Start the unshouldering process
void CWeaponBazooka::UnshoulderBazooka()
{
    CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
    if ( !pPlayer )
        return;

    pPlayer->SetMaxSpeed( 320.0f );

    SendWeaponAnim( ACT_VM_UNDEPLOY );

    m_bShouldered = false;
    m_bShouldering = true;

    m_flShoulderTime = gpGlobals->curtime;

    m_flNextPrimaryAttack = gpGlobals->curtime + m_flShoulderDuration;
    m_flNextSecondaryAttack = gpGlobals->curtime + m_flShoulderDuration;
    m_flTimeWeaponIdle = gpGlobals->curtime + m_flShoulderDuration;
}

// FireRocket: Fire the rocket projectile
void CWeaponBazooka::FireRocket()
{
    CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
    if ( !pPlayer )
        return;

#ifndef CLIENT_DLL
   
    Vector vecSrc, vecPos, vecAim;
    QAngle angTemp, angAim;

    vecSrc = pPlayer->Weapon_ShootPosition();
    angAim = pPlayer->EyeAngles();

    AngleVectors( angAim, &vecAim );

    Vector	vForward, vRight, vUp;

    pPlayer->EyeVectors( &vForward, &vRight, &vUp );
    
    vecPos = vecSrc + vForward + vRight * 4.0f + vUp * -4.0f;

    // Create the rocket entity
    CBazookaRocket *pRocket = CBazookaRocket::Create( vecPos, angAim, pPlayer );
    if ( pRocket )
    {
        // Set the rocket's velocity
        Vector vecVelocity = vecAim * 1200.0f; // Adjust speed as needed
        pRocket->SetAbsVelocity( vecVelocity );
    }
#endif // !CLIENT_DLL

     SendWeaponAnim( ACT_VM_PRIMARYATTACK );

    // Decrease ammo
    m_iClip1--;

    // Set the next attack time
    m_flNextPrimaryAttack = gpGlobals->curtime + 1.0f; // 1 second cooldown
    m_flNextSecondaryAttack = gpGlobals->curtime + 1.0f;
    m_flTimeWeaponIdle = gpGlobals->curtime + 1.0f;
}

void CWeaponBazooka::WeaponIdle(void)
{
    if ( m_flTimeWeaponIdle < gpGlobals->curtime )
        return;

    int anim;

    if ( m_bShouldered )
        anim = ACT_VM_IDLE_DEPLOYED;
    else
        anim = ACT_VM_IDLE;

    SendWeaponAnim( anim );

    //ClientPrint( ToBasePlayer( GetOwner() ), HUD_PRINTCENTER, anim == ACT_VM_IDLE ? "ACT_VM_IDLE\n" : "ACT_VM_IDLE_DEPLOYED\n" );

    m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration();
}

bool CWeaponBazooka::Holster( CBaseCombatWeapon *pOther )
{
    m_bShouldered = false;
    m_bShouldering = false;

    CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
    if ( pPlayer != nullptr )
        pPlayer->SetMaxSpeed( 320.0f );

    return BaseClass::Holster( pOther );
}
