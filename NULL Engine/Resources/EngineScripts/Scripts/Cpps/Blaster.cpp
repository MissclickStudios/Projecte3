#include "Blaster.h"

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
    else if (fireRateTimer.ReadSec() >= fireRate)
    {
        fireRateTimer.Stop();
        if (ammo <= 0)
            return ShootState::NO_AMMO;
        else
            return ShootState::RATE_FINISHED;
    }

    return ShootState::WAINTING_FOR_NEXT;
}

SCRIPTS_FUNCTION Blaster* CreateBlaster()
{
    Blaster* script = new Blaster();

    // Weapon
    // Stats
    INSPECTOR_DRAGABLE_FLOAT(script->damage);
    INSPECTOR_DRAGABLE_FLOAT(script->projectileSpeed);
    INSPECTOR_DRAGABLE_FLOAT(script->fireRate);
    INSPECTOR_DRAGABLE_FLOAT(script->bulletLifeTime);
    INSPECTOR_DRAGABLE_INT(script->ammo);
    INSPECTOR_DRAGABLE_INT(script->maxAmmo);
    INSPECTOR_DRAGABLE_INT(script->projectilesPerShot);

    // Reload
    INSPECTOR_DRAGABLE_FLOAT(script->reloadTime);

    // Modifiers
    INSPECTOR_DRAGABLE_FLOAT(script->damageModifier);
    INSPECTOR_DRAGABLE_FLOAT(script->projectileSpeedModifier);
    INSPECTOR_DRAGABLE_FLOAT(script->fireRateModifier);
    INSPECTOR_DRAGABLE_FLOAT(script->bulletLifeTimeModifier);
    INSPECTOR_DRAGABLE_FLOAT(script->reloadTimeModifier);
    INSPECTOR_DRAGABLE_INT(script->maxAmmoModifier);
    INSPECTOR_DRAGABLE_INT(script->PPSModifier);

    // Prefabs
    INSPECTOR_PREFAB(script->weaponModel);
    INSPECTOR_PREFAB(script->projectilePrefab);

    // Projectiles
    INSPECTOR_DRAGABLE_INT(script->projectileNum);
    INSPECTOR_CHECKBOX_BOOL(script->updateProjectiles);

    return script;
}