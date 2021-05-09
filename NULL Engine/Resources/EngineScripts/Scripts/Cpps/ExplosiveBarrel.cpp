#include "Application.h"
#include "M_Scene.h"
#include "Log.h"
#include "GameManager.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_ParticleSystem.h"
#include "C_BoxCollider.h"

#include "ExplosiveBarrel.h"

ExplosiveBarrel::ExplosiveBarrel() : Object()
{
	baseType = ObjectType::EXPLOSIVE_BARREL;
}

ExplosiveBarrel::~ExplosiveBarrel()
{
}

void ExplosiveBarrel::Start()
{
	gameManager = App->scene->GetGameObjectByName(gameManagerName.c_str());
	barrelObject = gameObject->FindChild(barrelObjectName.c_str());

	explosionParticles = gameObject->GetComponent<C_ParticleSystem>();

	explosionParticles->StopSpawn();

	barrelCollider = gameObject->GetComponent<C_BoxCollider>();

}

void ExplosiveBarrel::Update()
{
	if (exploded)
	{
		//deactivate mesh, trigger
		barrelCollider->SetIsActive(false);
		barrelCollider->SetSize(barrelColliderSize);

		exploded = false;
		//play particles
	}

	if (toExplode)
	{
		
		barrelObject->SetIsActive(false);

		explosionParticles->ResumeSpawn();

		exploded = true;
		toExplode = false;
	}

}

void ExplosiveBarrel::CleanUp()
{
}

void ExplosiveBarrel::OnCollisionEnter(GameObject* object)
{
	if (GetObjectScript(object, ObjectType::BULLET) != nullptr)
	{
		toExplode = true;
		barrelCollider->SetTrigger(true);
		barrelCollider->SetSize(explosionTriggerSize);
	}
}

void ExplosiveBarrel::OnTriggerRepeat(GameObject* object)
{
	Entity* entity = (Entity*)GetObjectScript(object, ObjectType::ENTITY);
	if (!entity)
		return;

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
	
	entity->AddEffect(EffectType::KNOCKBACK, 0.75f, false, new std::pair<bool, float3>(true, { direction.x, currentPower, direction.y }));
	entity->TakeDamage(damage);
}