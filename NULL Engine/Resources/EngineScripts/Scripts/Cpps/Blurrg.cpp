#include "Application.h"
#include "Log.h"

#include "M_Scene.h"
#include "M_ResourceManager.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_RigidBody.h"
#include "C_BoxCollider.h"
#include "C_Mesh.h"
#include "C_Material.h"

#include "Blurrg.h"
#include "Player.h"

Blurrg::Blurrg()
{
	dashTime.Stop();
	dashColdown.Stop();
	dashCharge.Stop();
	restTimer.Stop();

	freezeTimer.Stop();
}

Blurrg::~Blurrg()
{
}

void Blurrg::Awake()
{
	for (uint i = 0; i < gameObject->childs.size(); ++i)
		if (gameObject->childs[i]->GetComponent<C_Mesh>())
			mesh = gameObject->childs[i];
}

void Blurrg::Update()
{
	if (health <= 0.0f)
	{
		for (uint i = 0; i < gameObject->components.size(); ++i)
			gameObject->components[i]->SetIsActive(false);
		gameObject->SetIsActive(false);

		GameObject* coinGo = App->resourceManager->LoadPrefab(coin.uid, App->scene->GetSceneRoot());
		coinGo->GetComponent<C_BoxCollider>()->Update();
		float3 position = gameObject->transform->GetWorldPosition();
		position.y += 2;
		coinGo->transform->SetWorldPosition(position);

		return;
	}

	if (player == nullptr)
	{
		std::vector<GameObject*>* gameObjects = App->scene->GetGameObjects();
		for (auto object = gameObjects->begin(); object != gameObjects->end(); ++object)
			if ((*object)->GetScript("Player"))
				player = (*object);

		if (player == nullptr)
			return;
	}

	C_RigidBody* rigidBody = gameObject->GetComponent<C_RigidBody>();
	if (!rigidBody || rigidBody->IsStatic())
		return;

	attackModifier = 1;
	defenseModifier = 1;
	Color color = Color(150.0f / 255.0f, 150.0f / 255.0f, 150.0f / 255.0f);
	if (freezeTimer.IsActive())
	{
		if (freezeTimer.ReadSec() >= freezeDuration)
		{
			freezeTimer.Stop();
		}
		else
		{
			speedModifier = 0.3f;
			color = Color(20.0f / 255.0f, 220.0f / 255.0f, 255.0f / 255.0f);
		}
	}
	else
		speedModifier = 1;

	if (weakTimer.IsActive())
	{
		if (weakTimer.ReadSec() >= weakDuration)
		{
			weakTimer.Stop();
		}
		else
		{
			color = Color(250.0f / 255.0f, 20.0f / 255.0f, 10.0f / 255.0f);
		}
	}
	else
		defenseModifier = 1;

	if (mesh)
	{
		C_Material* material = mesh->GetComponent<C_Material>();
		material->SetMaterialColour(color);
	}

	if (!restTimer.IsActive())
	{
		if (dashCharge.IsActive())
		{
			// Dash
			if (dashCharge.ReadSec() >= dashingCharge)
			{
				dashCharge.Stop();
				dashColdown.Start();
				dashTime.Start();

				rigidBody->SetLinearVelocity(direction * dashSpeed * speedModifier);
			}
		}
		else if (!dashTime.IsActive())
		{
			direction = LookingAt();

			if (distance < detectionRange)
			{
				// Move
				direction *= speed * speedModifier;
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
		else if (dashTime.ReadSec() >= dashingTime / speedModifier)
		{
			dashTime.Stop();
			restTimer.Start();
			rigidBody->SetLinearVelocity(direction * restSpeed * speedModifier);
		}
	}
	else
	{
		if (restTimer.ReadSec() >= dashRest / speedModifier)
			restTimer.Stop();

		float2 currentDir(direction.x, direction.z);
		float currentRad = currentDir.AimedAngle();
		currentRad += 0.05 * speedModifier;
		direction.x = cos(currentRad);
		direction.z = sin(currentRad);

		if (mesh)
			mesh->transform->SetLocalRotation(float3(DegToRad(-90), 0, currentRad));
	}
}

void Blurrg::CleanUp()
{
}

void Blurrg::OnCollisionEnter(GameObject* object)
{
	if (object == player)
	{
		float hitDamage = damage;
		if (dashTime.IsActive())
			hitDamage = dashDamage;

		Player* script = (Player*)object->GetComponent<C_Script>()->GetScriptData();
		script->TakeDamage(hitDamage * attackModifier);
	}
}

void Blurrg::TakeDamage(float damage)
{
	health -= damage * defenseModifier;
	if (health < 0.0f)
		health = 0.0f;
}

void Blurrg::Freeze(float amount, float duration)
{
	speedModifier = amount;
	freezeDuration = duration;
	freezeTimer.Start();
}

void Blurrg::Weaken(float amount, float duration)
{
	defenseModifier = amount;
	weakDuration = duration;
	weakTimer.Start();
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

	for (uint i = 0; i < gameObject->childs.size(); ++i)
		if (gameObject->childs[i]->GetComponent<C_Mesh>())
		{
			float rad = -lookVector.AimedAngle() + DegToRad(90);
			gameObject->childs[i]->transform->SetLocalRotation(float3(DegToRad(-90), 0, rad));
		}

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
	INSPECTOR_PREFAB(script->coin);

	// Dash
	INSPECTOR_DRAGABLE_FLOAT(script->dashSpeed);
	INSPECTOR_DRAGABLE_FLOAT(script->dashingTime);
	INSPECTOR_DRAGABLE_FLOAT(script->dashingCharge);
	INSPECTOR_DRAGABLE_FLOAT(script->dashingColdown);

	INSPECTOR_DRAGABLE_FLOAT(script->dashRange);

	INSPECTOR_DRAGABLE_FLOAT(script->dashRest);
	INSPECTOR_DRAGABLE_FLOAT(script->restSpeed);

	// Health
	INSPECTOR_DRAGABLE_FLOAT(script->health);
	INSPECTOR_DRAGABLE_FLOAT(script->maxHealth);

	// Attack
	INSPECTOR_DRAGABLE_FLOAT(script->damage);
	INSPECTOR_DRAGABLE_FLOAT(script->dashDamage);

	return script;
}