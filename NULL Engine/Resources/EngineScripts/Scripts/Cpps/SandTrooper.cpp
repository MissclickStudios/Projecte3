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

#include "SandTrooper.h"
#include "Player.h"

#include "Weapon.h"

SandTrooper::SandTrooper() : Script()
{
	reloadTimer.Stop();
}

SandTrooper::~SandTrooper()
{
}

void SandTrooper::Awake()
{
	if (player == nullptr)
	{
		std::map<uint32,GameObject*>* objects = App->scene->GetGameObjects();
		for (auto o = objects->begin(); o != objects->end(); ++o)
			if(o->second->GetScript("Player"))
				player = o->second;
	}

	for (uint i = 0; i < gameObject->childs.size(); ++i)
		if (gameObject->childs[i]->GetComponent<C_Mesh>())
			mesh = gameObject->childs[i];
}

void SandTrooper::Update()
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
		return;

	if (!weapon)
		weapon = new Weapon(gameObject, projectilePrefab, 3u, maxAmmo, projectileSpeed, fireRate, automatic);
	weapon->Update();

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

	direction = LookingAt();

	ammo = weapon->ammo;
	if (!reloadTimer.IsActive())
	{
		if (ammo <= 0)
			reloadTimer.Start();
		if (distance < detectionRange)
			weapon->SandTrooperShoot(direction, speedModifier);
	}
	else if (reloadTimer.ReadSec() >= reloadTime / speedModifier)
	{
		weapon->Reload();
		reloadTimer.Stop();
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
		script->TakeDamage(damage * attackModifier);
	}
}

void SandTrooper::TakeDamage(float damage)
{
	health -= damage * defenseModifier;
	if (health < 0.0f)
		health = 0.0f;
}

void SandTrooper::Freeze(float amount, float duration)
{
	speedModifier = amount;
	freezeDuration = duration;
	freezeTimer.Start();
}

void SandTrooper::Weaken(float amount, float duration)
{
	defenseModifier = amount;
	weakDuration = duration;
	weakTimer.Start();
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

	if (mesh)
	{
		float rad = -lookVector.AimedAngle() + DegToRad(135);
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
	INSPECTOR_PREFAB(script->coin);

	// Weapon
	INSPECTOR_DRAGABLE_FLOAT(script->projectileSpeed);
	INSPECTOR_DRAGABLE_FLOAT(script->fireRate);

	INSPECTOR_DRAGABLE_INT(script->ammo);
	INSPECTOR_DRAGABLE_INT(script->maxAmmo);
	INSPECTOR_DRAGABLE_FLOAT(script->reloadTime);

	INSPECTOR_CHECKBOX_BOOL(script->automatic);

	INSPECTOR_PREFAB(script->projectilePrefab);

	// Health
	INSPECTOR_DRAGABLE_FLOAT(script->health);
	INSPECTOR_DRAGABLE_FLOAT(script->maxHealth);

	// Attack
	INSPECTOR_DRAGABLE_FLOAT(script->damage);

	return script;
}