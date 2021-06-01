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

#include "MC_Time.h"
#include "Log.h"

#define MAX_INPUT 32767

Grogu* CreateGrogu()
{
	Grogu* script = new Grogu();
	
	// Basic Stats
	INSPECTOR_DRAGABLE_FLOAT(script->speed);
	INSPECTOR_DRAGABLE_FLOAT(script->verticalSpeed);
	// Modifiers
	INSPECTOR_DRAGABLE_FLOAT(script->speedModifier);
	// Behaviour
	INSPECTOR_DRAGABLE_FLOAT(script->minDistanceToMando);
	INSPECTOR_DRAGABLE_FLOAT(script->maxDistanceToMando);
	INSPECTOR_CHECKBOX_BOOL(script->isLevitationEnabled);

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

	isGoingUp = false;
}

void Grogu::Behavior()
{
	// Check if player is referenced properly
	if (player == nullptr)
		return;

	ManageMovement();
	ManageRotation();
	ManageLevitation();
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
	else if(sqrt(deltaPos.x * deltaPos.x + deltaPos.z * deltaPos.z) < minDistanceToMando)
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

	GameObject* go = App->scene->GetGameObjectByName("Grogu Mesh");
	if (go != nullptr)
	{
		go->transform->SetLocalRotation(float3(0, -rad - DegToRad(60), 0));
	}
}


void Grogu::ManageLevitation()
{
	if (!isLevitationEnabled)
		return;

	GameObject* go = App->scene->GetGameObjectByName("Grogu Mesh");
	if (go != nullptr)
	{
		if (go->transform->GetWorldPosition().y >= -3)
			isGoingUp = false;
		else if (go->transform->GetWorldPosition().y <= -5)
			isGoingUp = true;

		float3 pos = go->transform->GetWorldPosition();

		if (isGoingUp)
			pos.y += verticalSpeed * MC_Time::Game::GetDT();
		else
			pos.y -= verticalSpeed * MC_Time::Game::GetDT();

		go->transform->SetWorldPosition(pos);
	}
}
void Grogu::Movement()
{
	float2 moveDirection = { direction.x, direction.z };
	moveDirection *= Speed();

	if (rigidBody != nullptr)
		rigidBody->Set2DVelocity(moveDirection);
}