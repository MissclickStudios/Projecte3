#include "Bullet.h"

#include "GameObject.h"
#include "C_Transform.h"

#include "Weapon.h"
#include "Entity.h"

Bullet::Bullet() 
{
}

Bullet::~Bullet()
{
}

void Bullet::Update()
{
	if (hit)
	{
		hit = false;
		shooter->ProjectileCollisionReport(index);
	}
}

void Bullet::OnCollisionEnter(GameObject* object)
{
	hit = true;

//	Entity* entity = (Entity*)object->GetScript("Entity");
//	entity->TakeDamage(onhitDamage);
//	for (uint i = 0; i < onHitEffects.size(); ++i)
//		entity->AddEffect(onHitEffects[i].type, onHitEffects[i].duration, onHitEffects[i].permanent);
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