#include "Bullet.h"

#include "GameObject.h"
#include "C_Transform.h"

#include "Weapon.h"
#include "Entity.h"

Bullet::Bullet() : Object()
{
	baseType = ObjectType::BULLET;
}

Bullet::~Bullet()
{
}

void Bullet::Update()
{
	if (hit)
	{
		hit = false;
		//shooter->ProjectileCollisionReport(index);
	}
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

void Bullet::SetOnHitData(float damage, std::vector<Effect> effects)
{
	onHitdamage = damage;
	onHitEffects = effects;
}

SCRIPTS_FUNCTION Bullet* CreateBullet()
{
	Bullet* script = new Bullet();
	return script;
}