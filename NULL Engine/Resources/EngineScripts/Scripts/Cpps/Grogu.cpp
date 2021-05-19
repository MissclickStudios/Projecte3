#include "JSONParser.h"

#include "Grogu.h"

#include "Application.h"
#include "M_Scene.h"
#include "M_Input.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_RigidBody.h"
#include "C_BoxCollider.h"

#include "GameManager.h"

#include "Log.h"

#define MAX_INPUT 32767

Grogu* CreateGrogu()
{
	Grogu* script = new Grogu();
	
	// Basic Stats
	INSPECTOR_DRAGABLE_FLOAT(script->speed);
	// Modifiers
	INSPECTOR_DRAGABLE_FLOAT(script->cooldownModifier);
	INSPECTOR_DRAGABLE_FLOAT(script->speedModifier);
	// Behaviour
	INSPECTOR_DRAGABLE_FLOAT(script->maxDistanceToMando);
	// Ability
	INSPECTOR_DRAGABLE_FLOAT(script->abilityCooldown);
	INSPECTOR_DRAGABLE_FLOAT(script->abilityRadius);

	return script;
}

Grogu::Grogu() : Entity()
{
	type = EntityType::GROGU;
}

Grogu::~Grogu()
{
}

void Grogu::SetUp()
{
	abilityCooldownTimer.Stop();
	player = App->scene->GetGameObjectByName(playerName.c_str());

	if (rigidBody != nullptr)
		rigidBody->FreezeRotationY(true);

	abilityCollider = gameObject->GetComponent<C_BoxCollider>();
	if (abilityCollider != nullptr)
		abilityCollider->SetIsActive(true);
}

void Grogu::Behavior()
{
	// Check if player is referenced properly
	if (player == nullptr)
		return;

	ManageMovement();
	ManageRotation();
	ManageAbility();
}

void Grogu::CleanUp()
{
	
}

void Grogu::SaveState(ParsonNode& groguNode)
{
	
}

void Grogu::LoadState(ParsonNode& groguNode)
{
	
}

void Grogu::Reset()
{
	// Reset ability's cooldown
}	

void Grogu::ManageMovement()
{
	// Check if player and grogu are apart
	float3 deltaPos = player->transform->GetWorldPosition() - gameObject->transform->GetWorldPosition();

	if (sqrt(deltaPos.x * deltaPos.x + deltaPos.z * deltaPos.z) > maxDistanceToMando)
	{
		direction = deltaPos;
		direction.Normalize();

		Movement();
	}
	else
	{
		if (rigidBody != nullptr)
			rigidBody->Set2DVelocity(float2::zero);
	}
}

void Grogu::ManageRotation()
{
	float3 direction = player->transform->GetWorldPosition() - gameObject->transform->GetWorldPosition();
	direction.Normalize();
	float2 aimDirection = { direction.x, direction.z };

	float rad = aimDirection.AimedAngle();
	gameObject->transform->SetLocalRotation(float3(0, -rad + DegToRad(180), 0));
}

void Grogu::ManageAbility()
{
	if (abilityCooldownTimer.IsActive())
	{
		if (abilityCooldownTimer.ReadSec() >= AbilityCooldown())
			abilityCooldownTimer.Stop();

		/*if (abilityCollider != nullptr)
			abilityCollider->SetIsActive(false);*/

		return;
	}
	if (App->input->GetKey(SDL_SCANCODE_N) == KeyState::KEY_DOWN)
	{
		Ability();
	}
}

void Grogu::Movement()
{
	float2 moveDirection = { direction.x, direction.z };
	moveDirection *= Speed();

	if (rigidBody != nullptr)
		rigidBody->Set2DVelocity(moveDirection);
}

void Grogu::Ability()
{
	abilityCooldownTimer.Start();

	if (abilityCollider != nullptr)
	{
		//abilityCollider->SetIsActive(true);
		abilityCollider->SetTrigger(true);
	}
}
//
//void Grogu::OnTriggerRepeat(GameObject* object)
//{
//	if (object == gameObject)
//		return;
//
//	Entity* entity = (Entity*)GetObjectScript(object, ObjectType::ENTITY);
//	if (!entity)
//		return;
//
//	float2 entityPosition, position;
//	entityPosition.x = entity->transform->GetWorldPosition().x;
//	entityPosition.y = entity->transform->GetWorldPosition().z;
//	position.x = gameObject->transform->GetWorldPosition().x;
//	position.y = gameObject->transform->GetWorldPosition().z;
//	float2 direction = entityPosition - position;
//
//	float distance = direction.Length();
//	if (distance < 1.0f)
//		distance = 1.0f;
//
//	float currentPower = abilityPower / distance;
//
//	direction.Normalize();
//	direction *= currentPower;
//
//	entity->AddEffect(EffectType::KNOCKBACK, 0.75f, false, new std::pair<bool, float3>(true, { direction.x, currentPower, direction.y }));
//	entity->TakeDamage(damage);
//}