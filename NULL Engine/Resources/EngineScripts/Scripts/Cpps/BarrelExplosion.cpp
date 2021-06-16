
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
	baseType = ObjectType::COLLECTABLE;
}

BarrelExplosion::~BarrelExplosion()
{

}

void BarrelExplosion::Start()
{
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
	if (!entity || entity->type == EntityType::IG12)
		return;

	Effect* eff = nullptr;
	if (!stun)
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

		eff = entity->AddEffect(EffectType::KNOCKBACK, stunTime, false, 0.0f, 0.0f, float3(direction.x, 0.0f, direction.y));
		if (eff)
			entity->gameObject->GetComponent<C_RigidBody>()->FreezePositionY(true);
	}
	else
	{
		eff = entity->AddEffect(EffectType::STUN, power, false, 0.0f, 100.0f);
	}
	if (eff != nullptr)
		entity->TakeDamage(damage);
}

BarrelExplosion* CreateBarrelExplosion() {
	BarrelExplosion* script = new BarrelExplosion();

	INSPECTOR_INPUT_INT(script->damage);
	INSPECTOR_DRAGABLE_FLOAT(script->power);
	INSPECTOR_CHECKBOX_BOOL(script->stun);
	INSPECTOR_DRAGABLE_FLOAT(script->stunTime);

	return script;
}