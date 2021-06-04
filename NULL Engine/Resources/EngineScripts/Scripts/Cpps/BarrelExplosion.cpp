
#include "GameObject.h"
#include "Object.h"
#include "ExplosiveBarrel.h"

#include "MathGeoLib/include/Math/float2.h"

#include "Entity.h"

#include "C_Transform.h"
#include "C_RigidBody.h"
#include "C_BoxCollider.h"

#include "BarrelExplosion.h"


BarrelExplosion::BarrelExplosion() : Object()
{

}

BarrelExplosion::~BarrelExplosion()
{

}

void BarrelExplosion::Start()
{
	barrelScript = (ExplosiveBarrel*)gameObject->parent->GetScript("ExplosiveBarrel");

	explosionCollider = gameObject->GetComponent<C_BoxCollider>();

	explosionCollider->SetTrigger(true);
	explosionCollider->SetIsActive(true);
}

void BarrelExplosion::Update()
{

}

void BarrelExplosion::CleanUp()
{

}

void BarrelExplosion::OnTriggerRepeat(GameObject* object)
{
	Entity* entity = (Entity*)GetObjectScript(object, ObjectType::ENTITY);
	if (!entity)
		return;

	if (barrelScript->toExplode)
	{
		float2 entityPosition, position;
		entityPosition.x = entity->transform->GetWorldPosition().x;
		entityPosition.y = entity->transform->GetWorldPosition().z;
		position.x = gameObject->transform->GetWorldPosition().x;
		position.y = gameObject->transform->GetWorldPosition().z;
		float2 direction = entityPosition - position;

		float distance = direction.Length();
		if (distance < 1.0f)
			distance = 1.0f;

		float currentPower = power / distance;

		direction.Normalize();
		direction *= currentPower;

		entity->gameObject->GetComponent<C_RigidBody>()->FreezePositionY(true);
		entity->AddEffect(EffectType::KNOCKBACK, 0.75f, false, 0.0f, 0.0f, float3(direction.x, 0.0f, direction.y));
		entity->TakeDamage(damage);
	}
}

BarrelExplosion* CreateBarrelExplosion() {
	BarrelExplosion* script = new BarrelExplosion();

	INSPECTOR_INPUT_INT(script->damage);

	return script;
}