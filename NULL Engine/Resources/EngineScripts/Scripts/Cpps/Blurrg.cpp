#include "Application.h"
#include "Log.h"

#include "M_Scene.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_RigidBody.h"

#include "Blurrg.h"
#include "Player.h"

Blurrg::Blurrg()
{
	dashTime.Stop();
	dashColdown.Stop();
	dashCharge.Stop();
}

Blurrg::~Blurrg()
{
}

void Blurrg::Update()
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

	if (dashCharge.IsActive())
	{
		// Dash
		if (dashCharge.ReadSec() >= dashingCharge)
		{
			dashCharge.Stop();
			dashColdown.Start();
			dashTime.Start();

			rigidBody->SetLinearVelocity(direction * dashSpeed);
		}
	}
	else if (!dashTime.IsActive())
	{
		direction = LookingAt();

		if (distance < detectionRange)
		{
			// Move
			direction *= speed;
			rigidBody->SetLinearVelocity(direction);

			if (dashColdown.IsActive())
			{
				if (dashColdown.ReadSec() >= dashingColdown)
					dashColdown.Stop();
			}
			else if (distance < dashRange)
			{
				rigidBody->SetLinearVelocity(float3::zero);
				dashCharge.Start();				// Start Charging Dash
			}
		}
	}
	else if (dashTime.ReadSec() >= dashingTime)
		dashTime.Stop();
}

void Blurrg::CleanUp()
{
}

void Blurrg::OnCollisionEnter(GameObject* object)
{
	if (object == player)
	{
		Player* script = (Player*)object->GetComponent<C_Script>()->GetScriptData();
		script->TakeDamage(0.5f);
	}
}

void Blurrg::TakeDamage(float damage)
{
	health -= damage;
	if (health < 0.0f)
		health = 0.0f;
}

// Return normalized vector 3 of the direction the player is at
float3 Blurrg::LookingAt()
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
	float rad = lookVector.AimedAngle() ;

	GameObject* mesh = gameObject->childs[0];
	if (mesh)
	{
		float rad = -lookVector.AimedAngle() + DegToRad(90);
		mesh->transform->SetLocalRotation(float3(DegToRad(-90), 0, rad));
	}

	return { lookVector.x, 0, lookVector.y };
}

Blurrg* CreateBlurrg()
{
	Blurrg* script = new Blurrg();

	// Movement
	INSPECTOR_DRAGABLE_FLOAT(script->speed);

	INSPECTOR_DRAGABLE_FLOAT(script->detectionRange);

	//INSPECTOR_GAMEOBJECT(script->player);

	// Dash
	INSPECTOR_DRAGABLE_FLOAT(script->dashSpeed);
	INSPECTOR_DRAGABLE_FLOAT(script->dashingTime);
	INSPECTOR_DRAGABLE_FLOAT(script->dashingCharge);
	INSPECTOR_DRAGABLE_FLOAT(script->dashingColdown);

	INSPECTOR_DRAGABLE_FLOAT(script->dashRange);

	// Health
	INSPECTOR_DRAGABLE_FLOAT(script->health);
	INSPECTOR_DRAGABLE_FLOAT(script->maxHealth);

	return script;
}