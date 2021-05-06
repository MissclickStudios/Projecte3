#include "Bullet.h"

#include "GameObject.h"
#include "C_Transform.h"

#include "Weapon.h"

Bullet::Bullet() : Entity()
{
	baseType = ObjectType::BULLET;

	lifeTimeTimer.Stop();
}

Bullet::~Bullet()
{
}

void Bullet::Behavior()
{
	if (lifeTimeTimer.ReadSec() >= lifeTime)
	{
		lifeTimeTimer.Stop();
		hit = true;
	}
	if (hit)
	{
		hit = false;
		if (shooter)
			shooter->ProjectileCollisionReport(index);

		Deactivate();
	}
}

void Bullet::EntityPause()
{
	lifeTimeTimer.Pause();
}

void Bullet::EntityResume()
{
	lifeTimeTimer.Resume();
}

void Bullet::OnCollisionEnter(GameObject* object)
{
	hit = true;

	Entity* entity = (Entity*)GetObjectScript(object, ObjectType::ENTITY);
	if (!entity)
		return;

	entity->TakeDamage(onHitdamage);
	for (uint i = 0; i < onHitEffects.size(); ++i)
		entity->AddEffect(onHitEffects[i].Type(), onHitEffects[i].Duration(), onHitEffects[i].Permanent());
}

void Bullet::SetShooter(Weapon* shooter, int index)
{
	this->shooter = shooter;
	this->index = index;
}

void Bullet::SetOnHitData(float damage, std::vector<Effect> effects, float lifeTime)
{
	onHitdamage = damage;
	onHitEffects = effects;

	this->lifeTime = lifeTime;
	lifeTimeTimer.Start();
}

SCRIPTS_FUNCTION Bullet* CreateBullet()
{
	Bullet* script = new Bullet();
	return script;
}