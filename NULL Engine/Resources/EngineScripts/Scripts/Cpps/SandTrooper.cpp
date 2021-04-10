#include "Application.h"
#include "Log.h"

#include "M_Scene.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_RigidBody.h"
#include "C_Mesh.h"

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
	if (!player)
	{
		std::vector<GameObject*>* objects = App->scene->GetGameObjects();
		for (uint i = 0; i < objects->size(); ++i)
			if((*objects)[i]->GetScript("Player"))
				player = (*objects)[i];
	}
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
		return;

	if (!weapon)
		weapon = new Weapon(gameObject, projectilePrefab, 3u, maxAmmo, projectileSpeed, fireRate, automatic);
	weapon->Update();

	C_RigidBody* rigidBody = gameObject->GetComponent<C_RigidBody>();
	if (!rigidBody || rigidBody->IsStatic())
		return;

	direction = LookingAt();

	ammo = weapon->ammo;
	if (!reloadTimer.IsActive())
	{
		if (ammo <= 0)
			reloadTimer.Start();
		if (distance < detectionRange)
			weapon->SandTrooperShoot(direction);
	}
	else if (reloadTimer.ReadSec() >= reloadTime)
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

	for (uint i = 0; i < gameObject->childs.size(); ++i)
		if (gameObject->childs[i]->GetComponent<C_Mesh>())
		{
			float rad = -lookVector.AimedAngle() + DegToRad(135);
			gameObject->childs[i]->transform->SetLocalRotation(float3(DegToRad(-90), 0, rad));
		}

	return { lookVector.x, 0, lookVector.y };
}

SandTrooper* CreateSandTrooper()
{
	SandTrooper* script = new SandTrooper();

	// Movement
	INSPECTOR_DRAGABLE_FLOAT(script->speed);
	INSPECTOR_DRAGABLE_FLOAT(script->detectionRange);
	INSPECTOR_GAMEOBJECT(script->player);

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