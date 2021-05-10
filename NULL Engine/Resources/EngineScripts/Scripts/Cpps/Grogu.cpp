#include "JSONParser.h"

#include "Grogu.h"

#include "Application.h"
#include "M_Scene.h"
#include "M_Input.h"

#include "GameObject.h"
#include "C_Transform.h"

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
	position = gameObject->transform->GetWorldPosition();
	float3 deltaPos = player->transform->GetWorldPosition() - gameObject->transform->GetWorldPosition();

	if (sqrt(deltaPos.x * deltaPos.x + deltaPos.z * deltaPos.z) > maxDistanceToMando)
	{
		float3 direction = deltaPos;
		direction.Normalize();

		direction *= Speed();
		position += direction;

		Movement();
	}
}

void Grogu::ManageRotation()
{
	float3 deltaPos = player->transform->GetWorldPosition() - gameObject->transform->GetWorldPosition();
	float3 direction = deltaPos;
	direction.Normalize();
	aimDirection.x = direction.x;
	aimDirection.y = direction.z;

	float rad = aimDirection.AimedAngle();
	gameObject->transform->SetLocalRotation(float3(0, -rad + DegToRad(180), 0));
}

void Grogu::ManageAbility()
{
	if (abilityCooldownTimer.IsActive())
	{
		if (abilityCooldownTimer.ReadSec() >= AbilityCooldown())
			abilityCooldownTimer.Stop();
		return;
	}
	if (App->input->GetKey(SDL_SCANCODE_N) == KeyState::KEY_DOWN)
	{
		Ability();
	}
}

void Grogu::Movement()
{
	gameObject->transform->SetWorldPosition(position);
}

void Grogu::Ability()
{
	abilityCooldownTimer.Start();
	gameObject->transform->SetWorldPosition(position);
}