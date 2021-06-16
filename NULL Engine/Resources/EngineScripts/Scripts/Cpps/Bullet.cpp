#include "Bullet.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_RigidBody.h"

#include "Weapon.h"

Bullet::Bullet() : Object()
{
	baseType = ObjectType::BULLET;

	lifeTimeTimer.Stop();
}

Bullet::~Bullet()
{
}

void Bullet::Awake()
{
}

void Bullet::Update()
{
	if (rigidBody == nullptr)
		rigidBody = gameObject->GetComponent<C_RigidBody>();
	if (paused)
		return;

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

void Bullet::CleanUp()
{
}

void Bullet::OnPause()
{
	paused = true;
	lifeTimeTimer.Pause();

	if (rigidBody != nullptr)
		rigidBody->SetIsActive(false);
}

void Bullet::OnResume()
{
	paused = false;
	lifeTimeTimer.Resume();

	if (rigidBody != nullptr)
		rigidBody->SetIsActive(true);
}

void Bullet::OnCollisionEnter(GameObject* object)
{
	Hit();

	Entity* entity = (Entity*)GetObjectScript(object, ObjectType::ENTITY);
	if (!entity)
		return;

	entity->TakeDamage(onHitdamage);
	for (uint i = 0; i < onHitEffects.size(); ++i)
		entity->AddEffect(onHitEffects[i].Type(), onHitEffects[i].Duration(), onHitEffects[i].Permanent(), onHitEffects[i].Power(), onHitEffects[i].Chance(), onHitEffects[i].Direction(), onHitEffects[i].start);
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