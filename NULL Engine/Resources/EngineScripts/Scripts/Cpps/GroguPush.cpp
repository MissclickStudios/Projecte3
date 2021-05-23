#include "Application.h"
#include "MC_Time.h"

#include "M_Scene.h"
#include "M_Input.h"

#include "Log.h"
#include "GameManager.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_ParticleSystem.h"
#include "C_BoxCollider.h"
#include "C_AudioSource.h"

#include "GroguPush.h"

GroguPush* CreateGroguPush()
{
	GroguPush* script = new GroguPush();

	INSPECTOR_INPUT_FLOAT3(script->abilityRadius);
	INSPECTOR_INPUT_FLOAT(script->abilityCooldown);

	return script;
}

GroguPush::GroguPush() : Object()
{
	baseType = ObjectType::GROGU_ABILITY;
}

GroguPush::~GroguPush()
{
}

void GroguPush::Start()
{
	player = App->scene->GetGameObjectByName(playerName.c_str());

	abilityCollider = gameObject->GetComponent<C_BoxCollider>();
	abilityCollider->SetSize(abilityRadius);
}

void GroguPush::Update()
{
	doAbility = false;

	abilityCollider->SetTrigger(false);
	abilityCollider->SetIsActive(false);

	if (abilityCooldownTimer.IsActive())
	{
		if (abilityCooldownTimer.ReadSec() >= abilityCooldown)
			abilityCooldownTimer.Stop();

		return;
	}

	if (App->input->GetKey(SDL_SCANCODE_G) == KeyState::KEY_DOWN)
	{
		doAbility = true;

		abilityCollider->SetIsActive(true);
		abilityCollider->SetTrigger(true);
	}
}

void GroguPush::CleanUp()
{
	
}

void GroguPush::OnTriggerRepeat(GameObject* object)
{
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

	float currentPower = abilityPower / distance;

	direction.Normalize();
	direction *= currentPower;
	
	entity->AddEffect(EffectType::KNOCKBACK, 0.75f, false, new std::pair<bool, float3>(true, { direction.x, currentPower, direction.y }));
}