#include "Application.h"
#include "Log.h"

#include "M_Scene.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_RigidBody.h"

#include "SandTrooper.h"
#include "Player.h"

SandTrooper::SandTrooper() : Script()
{
}

SandTrooper::~SandTrooper()
{
}

void SandTrooper::Update()
{
	if (health <= 0.0f)
	{
		for (uint i = 0; i < gameObject->components.size(); ++i)
			gameObject->components[i]->SetIsActive(false);
		gameObject->SetIsActive(false);

		return;
	}

	if (!player)
	{
		std::vector<GameObject*>* objects = App->scene->GetGameObjects();
		for (uint i = 0; i < objects->size(); ++i)
		{
			GameObject* object = (*objects)[i];
			for (uint n = 0; n < object->components.size(); ++n)
			{
				Component* comp = object->components[n];
				if (comp->GetType() == ComponentType::SCRIPT)
				{
					C_Script* script = (C_Script*)comp;
					if (script->GetDataName() == "Player")
						player = object;
				}
			}
		}
		if (!player)
			return;
	}

	C_RigidBody* rigidBody = gameObject->GetComponent<C_RigidBody>();
	if (!rigidBody || rigidBody->IsStatic())
		return;

	direction = LookingAt();

	if (distance < detectionRange)
	{
		
	}
}

void SandTrooper::CleanUp()
{
}

void SandTrooper::OnCollisionEnter(GameObject* object)
{
	if (object == player)
	{
		Player* script = (Player*)object->GetComponent<C_Script>()->GetScriptData();
		script->TakeDamage(damage);
	}
}

void SandTrooper::TakeDamage(float damage)
{
	health -= damage;
	if (health < 0.0f)
		health = 0.0f;
}

// Return normalized vector 3 of the direction the player is at
float3 SandTrooper::LookingAt()
{
	float2 playerPosition, position, lookVector;
	playerPosition.x = player->transform->GetWorldPosition().x;
	playerPosition.y = player->transform->GetWorldPosition().z;
	position.x = gameObject->transform->GetWorldPosition().x;
	position.y = gameObject->transform->GetWorldPosition().z;

	lookVector = playerPosition - position;

	distance = lookVector.Length();
	if (lookVector.x == 0 && lookVector.y == 0) {}
	else
		lookVector.Normalize();
	float rad = lookVector.AimedAngle();

	GameObject* mesh = gameObject->childs[0];
	if (mesh)
	{
		float rad = -lookVector.AimedAngle() + DegToRad(90);
		mesh->transform->SetLocalRotation(float3(DegToRad(-90), 0, rad));
	}

	return { lookVector.x, 0, lookVector.y };
}

SandTrooper* CreateSandTrooper()
{
	SandTrooper* script = new SandTrooper();

	// Movement
	INSPECTOR_DRAGABLE_FLOAT(script->speed);

	INSPECTOR_DRAGABLE_FLOAT(script->detectionRange);

	//INSPECTOR_GAMEOBJECT(script->player);

	// Health
	INSPECTOR_DRAGABLE_FLOAT(script->health);
	INSPECTOR_DRAGABLE_FLOAT(script->maxHealth);

	// Attack
	INSPECTOR_DRAGABLE_FLOAT(script->damage);

	return script;
}