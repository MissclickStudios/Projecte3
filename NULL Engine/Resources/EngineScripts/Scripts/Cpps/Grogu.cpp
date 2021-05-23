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
	INSPECTOR_DRAGABLE_FLOAT(script->speedModifier);
	// Behaviour
	INSPECTOR_DRAGABLE_FLOAT(script->maxDistanceToMando);

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
	player = App->scene->GetGameObjectByName(playerName.c_str());

	if (rigidBody != nullptr)
	{
		rigidBody->FreezeRotationX(true);
		rigidBody->FreezeRotationY(true);
		rigidBody->FreezeRotationZ(true);
	}
}

void Grogu::Behavior()
{
	// Check if player is referenced properly
	if (player == nullptr)
		return;

	ManageMovement();
	ManageRotation();
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

}	

void Grogu::ManageMovement()
{
	float3 position = gameObject->transform->GetWorldPosition();
	float3 playerPos = player->transform->GetWorldPosition();

	// Update Grogu's Y
	gameObject->transform->SetLocalPosition({ position.x, playerPos.y, position.z });

	// Check if player and grogu are apart
	float3 deltaPos = playerPos - position;

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

void Grogu::Movement()
{
	float2 moveDirection = { direction.x, direction.z };
	moveDirection *= Speed();

	if (rigidBody != nullptr)
		rigidBody->Set2DVelocity(moveDirection);
}