#include "Application.h"
#include "MC_Time.h"

#include "M_Scene.h"
#include "M_Input.h"

#include "Log.h"
#include "GameManager.h"

#include "GameManager.h"
#include "GameObject.h"
#include "C_Transform.h"
#include "C_ParticleSystem.h"
#include "C_BoxCollider.h"
#include "C_AudioSource.h"
#include "C_RigidBody.h"
#include "GroguPush.h"

#include "Entity.h"

#include "Player.h"

GroguPush* CreateGroguPush()
{
	GroguPush* script = new GroguPush();
	/*
	INSPECTOR_INPUT_FLOAT3(script->abilityRadius);
	INSPECTOR_INPUT_FLOAT(script->abilityCooldown);
	INSPECTOR_INPUT_FLOAT(script->abilityPower);
	*/
	return script;
}

GroguPush::GroguPush() : Object()
{
	/*
	baseType = ObjectType::GROGU_ABILITY;
	abilityCooldownTimer.Stop();
	particleTimer.Stop();
	*/
}

GroguPush::~GroguPush()
{
}

void GroguPush::Start()
{
	gameObject->SetIsActive(false);
	/*
	player = App->scene->GetGameObjectByName(playerName.c_str());
	if (player != nullptr)
		playerScript = (Player*)player->GetScript("Player");

	abilityCollider = gameObject->GetComponent<C_BoxCollider>();
	abilityCollider->SetSize(abilityRadius);

	GameObject* tmp = App->scene->GetGameObjectByName("Game Manager");
	if (tmp != nullptr)
		gameManager = (GameManager*)tmp->GetScript("GameManager");
	
	abilityParticles = gameObject->FindChild("Ability")->GetComponent<C_ParticleSystem>();
	abilityParticles->StopSpawn();

	abilityCollider->SetTrigger(true);
	abilityCollider->SetIsActive(false);
	*/
}

void GroguPush::Update()
{
	/*
	doAbility = false;

	// Particles
	if (particleTimer.IsActive())
	{
		if (particleTimer.ReadSec() > particleEmitttingTime)
		{
			particleTimer.Stop();

			abilityParticles->StopSpawn();
		}
	}

	if (abilityCooldownTimer.IsActive())
	{
		if (abilityCooldownTimer.ReadSec() > abilityCooldown)
			abilityCooldownTimer.Stop();
		else
			return;
	}

	if ((App->input->GetKey(SDL_SCANCODE_G) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(SDL_CONTROLLER_BUTTON_LEFTSHOULDER) == ButtonState::BUTTON_DOWN) && gameManager->storyDialogState.defeatedIG11FirstTime)
	{
		doAbility = true;

		abilityCollider->SetIsActive(true);

		abilityCooldownTimer.Start();
		particleTimer.Start();

		abilityParticles->ResumeSpawn();

		playerScript->SetPlayerInteraction(InteractionType::SIGNAL_GROGU);
	}
	*/
}

void GroguPush::CleanUp()
{

}

void GroguPush::OnTriggerRepeat(GameObject* object)
{
	/*
	if (!doAbility)
		return;
	Entity* entity = (Entity*)GetObjectScript(object, ObjectType::ENTITY);
	if (!entity)
		return;

	if (entity->type == EntityType::GROGU || entity->type == EntityType::PLAYER)
		return;

	float2 entityPosition, position;
	entityPosition.x = entity->transform->GetWorldPosition().x;
	entityPosition.y = entity->transform->GetWorldPosition().z;
	position.x = gameObject->transform->GetWorldPosition().x;
	position.y = gameObject->transform->GetWorldPosition().z;
	float2 direction = entityPosition - position;

	float distance = direction.Length();
	if (distance < 1.0f)
		distance = 1.0f;

	//float currentPower = abilityPower / distance;
	float currentPower = abilityPower;
	direction.Normalize();
	direction *= currentPower;
	entity->gameObject->GetComponent<C_RigidBody>()->FreezePositionY(true);
	entity->AddEffect(EffectType::KNOCKBACK, 0.25f, false, currentPower, 0.f, float3(direction.x, 0.f, direction.y), true);
	*/
}

void GroguPush::OnTriggerExit(GameObject* object)
{
	/*
	Entity* entity = (Entity*)GetObjectScript(object, ObjectType::ENTITY);
	if (!entity)
		return;

	if (entity->type == EntityType::GROGU || entity->type == EntityType::PLAYER)
		return;

	entity->gameObject->GetComponent<C_RigidBody>()->FreezePositionY(false);
	*/
}