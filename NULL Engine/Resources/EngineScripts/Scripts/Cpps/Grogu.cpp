#include "JSONParser.h"

#include "Grogu.h"

#include "Application.h"
#include "M_Scene.h"
#include "M_Input.h"
#include "M_ResourceManager.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_RigidBody.h"
#include "C_BoxCollider.h"
#include "C_ParticleSystem.h"

#include "GameManager.h"
#include "Bullet.h"

#include "MC_Time.h"
#include "Log.h"
#include "Random.h"

#define MAX_INPUT 32767

Grogu* CreateGrogu()
{
	Grogu* script = new Grogu();

	INSPECTOR_DRAGABLE_FLOAT(script->speed);
	INSPECTOR_DRAGABLE_FLOAT(script->minDistanceToMando);
	INSPECTOR_DRAGABLE_FLOAT(script->maxDistanceToMando);
	INSPECTOR_DRAGABLE_FLOAT(script->sprintDistance);
	INSPECTOR_DRAGABLE_FLOAT(script->slowDistance);

	INSPECTOR_DRAGABLE_FLOAT(script->power);
	INSPECTOR_DRAGABLE_FLOAT(script->cooldown);
	INSPECTOR_CHECKBOX_BOOL(script->deflectBullets);

	INSPECTOR_PREFAB(script->ParticlePrefab);
	INSPECTOR_DRAGABLE_FLOAT(script->particleTime);

	INSPECTOR_DRAGABLE_FLOAT(script->circleTime);
	INSPECTOR_DRAGABLE_FLOAT(script->radius);

	return script;
}

Grogu::Grogu() : Entity()
{
	type = EntityType::GROGU;

	cooldownTimer.Stop();
	particleTimer.Stop();
}

Grogu::~Grogu()
{
}

void Grogu::SetUp()
{
	player = App->scene->GetGameObjectByName(playerName.c_str());
	mesh = App->scene->GetGameObjectByName(meshName.c_str());

	attackCollider = gameObject->GetComponent<C_BoxCollider>();
	if (attackCollider != nullptr)
		attackCollider->SetIsActive(false);

	if (rigidBody != nullptr)
		rigidBody->DisableY(false);

	if (player != nullptr)
	{
		GameObject* particleGameObject = App->scene->InstantiatePrefab(ParticlePrefab.uid, player, float3::zero, Quat());
		if (particleGameObject != nullptr)
		{
			particles = particleGameObject->GetComponent<C_ParticleSystem>();
			if (particles != nullptr)
				particles->StopSpawn();
		}
	}

}

void Grogu::Behavior()
{
	if (player == nullptr)
		return;

	GetDistance();
	if (!cooldownTimer.IsActive())
	{
		if (App->input->GetKey(SDL_SCANCODE_F) == KeyState::KEY_DOWN
			|| App->input->GetGameControllerButton(SDL_CONTROLLER_BUTTON_LEFTSHOULDER) == ButtonState::BUTTON_DOWN)
		{
			state = GroguState::ATTACK_IN;
			cooldownTimer.Start();
		}
	}
	else if (cooldownTimer.ReadSec() > cooldown)
		cooldownTimer.Stop();

	if (particleTimer.IsActive() && particleTimer.ReadSec() > particleTime)
	{
		particleTimer.Stop();
		if (particles != nullptr)
			particles->StopSpawn();

		if (attackCollider != nullptr)
			attackCollider->SetIsActive(false);
		cooldownTimer.Start();
	}

	switch (state)
	{
	case GroguState::IDLE:
		if (distance > maxDistanceToMando)
			state = GroguState::MOVE;
		if (rigidBody != nullptr)
			rigidBody->StopInertia();
		break;
	case GroguState::MOVE:
		if (distance < minDistanceToMando)
			state = GroguState::IDLE;
		Move();
		break;
	case GroguState::ATTACK_IN:
		//state = GroguState::ATTACK;
		state = GroguState::IDLE;
		if (attackCollider != nullptr)
			attackCollider->SetIsActive(true);
		particleTimer.Start();
		if (particles != nullptr)
			particles->ResumeSpawn();
		break;
	}

	Rotate();
	Levitate();
}

void Grogu::CleanUp()
{
	
}

void Grogu::OnTriggerRepeat(GameObject* object)
{
	Entity* entity = (Entity*)GetObjectScript(object, ObjectType::ENTITY);
	if (entity != nullptr)
	{
		float2 entityPosition, position;
		entityPosition.x = entity->transform->GetWorldPosition().x;
		entityPosition.y = entity->transform->GetWorldPosition().z;
		position.x = player->transform->GetWorldPosition().x;
		position.y = player->transform->GetWorldPosition().z;
		float2 direction = entityPosition - position;

		float distance = direction.Length();
		if (distance < 1.0f)
			distance = 1.0f;

		float currentPower = power / distance;

		direction.Normalize();
		direction *= currentPower;

		entity->AddEffect(EffectType::KNOCKBACK, 0.5f, false, 0.0f, 0.0f, float3(direction.x, 0.0f, direction.y));

		return;
	}
	Bullet* bullet = (Bullet*)GetObjectScript(object, ObjectType::BULLET);
	if (bullet != nullptr)
	{
		if (!deflectBullets)
			bullet->Hit();
		else
		{
			C_RigidBody* bulletRigidBody = bullet->gameObject->GetComponent<C_RigidBody>();
			if (bulletRigidBody != nullptr)
			{
				float3 velocity = bulletRigidBody->GetLinearVelocity();
				velocity /= 1.10f;
				velocity.y = 0.0f;

				bulletRigidBody->SetLinearVelocity(velocity);
				bulletRigidBody->UseGravity(true);
				bulletRigidBody->DisableY(false);
			}
		}
	}
}

void Grogu::SaveState(ParsonNode& groguNode)
{
	
}

void Grogu::LoadState(ParsonNode& groguNode)
{
	
}

void Grogu::Move()
{
	if (rigidBody == nullptr)
		return;

	direction.Normalize();

	if (distance > sprintDistance)
		direction *= Speed() * 2.0f;
	else if (distance < slowDistance)
		direction *= Speed() / 4.0f;
	else
		direction *= Speed();

	rigidBody->SetLinearVelocity(direction);
}

void Grogu::Rotate()
{
	if (mesh == nullptr)
		return;

	direction.Normalize();
	float2 aimDirection = { direction.x, direction.z };
	float rad = aimDirection.AimedAngle();

	mesh->transform->SetLocalRotation(float3(0, -rad + DegToRad(180), 0));
}

void Grogu::Levitate()
{
	if (mesh == nullptr)
		return;
}

void Grogu::Attack()
{
}

void Grogu::GetDistance()
{
	if (player == nullptr)
		return;

	float3 position = gameObject->transform->GetWorldPosition();
	float3 playerPosition = player->transform->GetWorldPosition();

	if (!circleTimer.IsActive())
	{
		circleTimer.Start();
	}
	float seconds = circleTimer.ReadSec();
	if (seconds > circleTime)
	{
		circleTimer.Start();
		circleMultiplier *= -1;
	}

	float w = (2 * PI) / circleTime;
	float rad = w * seconds;
	if (0 < rad && rad < PI)
		circleMultiplier = 1.0f;
	else
		circleMultiplier = -1.0f;

	float x = radius * Cos(rad);
	float y = 2.0f * Sin((circleTime / 2.0f) * seconds);
	float z = sqrt(radius * radius - x * x) * circleMultiplier;

	float3 movePosition = float3(x + playerPosition.x, y + 5.0f + playerPosition.y, z + playerPosition.z);

	direction = movePosition - position;
	distance = direction.Length();
	if (attackCollider != nullptr)
	{
		float3 colliderPosition = playerPosition - position;
		attackCollider->SetCenter(colliderPosition.x, colliderPosition.y, colliderPosition.z);
	}
}
