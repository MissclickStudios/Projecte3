#include "Blaster.h"

#include "MC_Time.h"
#include "Log.h"

Blaster::Blaster() : Weapon()
{
}

Blaster::~Blaster()
{
}

void Blaster::SetUp()
{
}

ShootState Blaster::ShootLogic()
{
    if (!fireRateTimer.IsActive())
    {
        fireRateTimer.Start();
        return ShootState::FIRED_PROJECTILE;
    }
    else 
    {
        if (fireRateTimer.ReadMs() / 1000.0f >= FireRate())
        {
            fireRateTimer.Stop();
            if (ammo <= 0)
                return ShootState::NO_AMMO;
            else
                return ShootState::RATE_FINISHED;
        }
    }

    return ShootState::WAITING_FOR_NEXT;
}

SCRIPTS_FUNCTION Blaster* CreateBlaster()
{
    Blaster* script = new Blaster();

    // Weapon
    // Type
    INSPECTOR_ENUM(script->type, "WeaponType", "EngineScripts/Scripts/Helpers/Weapon.h");

    // Stats
    INSPECTOR_DRAGABLE_FLOAT(script->damage);
    INSPECTOR_DRAGABLE_FLOAT(script->projectileSpeed);
    INSPECTOR_DRAGABLE_FLOAT(script->fireRate);
    INSPECTOR_DRAGABLE_FLOAT(script->fireRateCap);
    INSPECTOR_DRAGABLE_FLOAT(script->bulletLifeTime);
    INSPECTOR_DRAGABLE_INT(script->ammo);
    INSPECTOR_DRAGABLE_INT(script->maxAmmo);
    INSPECTOR_DRAGABLE_INT(script->projectilesPerShot);
    INSPECTOR_DRAGABLE_FLOAT(script->shotSpreadArea);
    INSPECTOR_DRAGABLE_FLOAT3(script->spreadRadius);

    // Reload
    INSPECTOR_DRAGABLE_FLOAT(script->reloadTime);
    INSPECTOR_DRAGABLE_FLOAT(script->reloadTimeCap);

    // Modifiers
    INSPECTOR_DRAGABLE_FLOAT(script->damageModifier);
    INSPECTOR_DRAGABLE_FLOAT(script->projectileSpeedModifier);
    INSPECTOR_DRAGABLE_FLOAT(script->fireRateModifier);
    INSPECTOR_DRAGABLE_FLOAT(script->bulletLifeTimeModifier);
    INSPECTOR_DRAGABLE_FLOAT(script->reloadTimeModifier);
    INSPECTOR_DRAGABLE_FLOAT(script->maxAmmoModifier);
    INSPECTOR_DRAGABLE_INT(script->PPSModifier);
    INSPECTOR_DRAGABLE_FLOAT(script->spreadRadiusModifier);

    // Visuals
    INSPECTOR_PREFAB(script->projectilePrefab);
    INSPECTOR_PREFAB(script->weaponModelPrefab);  
    INSPECTOR_DRAGABLE_FLOAT3(script->position);
    INSPECTOR_DRAGABLE_FLOAT3(script->rotation);
    INSPECTOR_DRAGABLE_FLOAT3(script->scale);

    INSPECTOR_DRAGABLE_FLOAT3(script->modifiedPosition);
    INSPECTOR_DRAGABLE_FLOAT3(script->modifiedRotation);
    // Projectiles
    INSPECTOR_DRAGABLE_INT(script->projectileNum);
    INSPECTOR_CHECKBOX_BOOL(script->updateProjectiles);

    // Audio
    INSPECTOR_STRING(script->shootAudioString);
    INSPECTOR_STRING(script->reloadAudioString);

    return script;
}
