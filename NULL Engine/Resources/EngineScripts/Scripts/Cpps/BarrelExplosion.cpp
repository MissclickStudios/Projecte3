
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
}

void BarrelExplosion::Update()
{
	if (state == 1)
		state = 2;
}

void BarrelExplosion::CleanUp()
{

}

void BarrelExplosion::OnTriggerRepeat(GameObject* object)
{
	if (state == 2)
		return;

	Entity* entity = (Entity*)GetObjectScript(object, ObjectType::ENTITY);
	if (!entity)
		return;

	state = 1;

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

		entity->gameObject->GetComponent<C_RigidBody>()->FreezePositionY(true);
		entity->AddEffect(EffectType::KNOCKBACK, 0.75f, false, 0.0f, 0.0f, float3(direction.x, 0.0f, direction.y));
	}
	else
	{
		entity->AddEffect(EffectType::STUN, power, false, 0.0f, 100.0f);
	}
	entity->TakeDamage(damage);
}

BarrelExplosion* CreateBarrelExplosion() {
	BarrelExplosion* script = new BarrelExplosion();

	INSPECTOR_INPUT_INT(script->damage);
	INSPECTOR_DRAGABLE_FLOAT(script->power);
	INSPECTOR_CHECKBOX_BOOL(script->stun);

	return script;
}