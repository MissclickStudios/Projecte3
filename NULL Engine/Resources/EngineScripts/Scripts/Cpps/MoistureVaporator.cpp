#include "Application.h"
#include "MC_Time.h"

#include "M_Scene.h"
#include "Log.h"
#include "GameManager.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_ParticleSystem.h"
#include "C_BoxCollider.h"
#include "C_AudioSource.h"

#include "MoistureVaporator.h"

MoistureVaporator::MoistureVaporator() : Object()
{
	baseType = ObjectType::EXPLOSIVE_BARREL;
}

MoistureVaporator::~MoistureVaporator()
{
}

void MoistureVaporator::Start()
{
	explosion = new C_AudioSource(gameObject);

	gameManager = App->scene->GetGameObjectByName(gameManagerName.c_str());
	vaporatorObject = gameObject->FindChild(vaporatorObjectName.c_str());

	explosionParticles = gameObject->FindChild("StunParticles")->GetComponent<C_ParticleSystem>();

	explosionParticles->StopSpawn();

	vaporatorCollider = gameObject->GetComponent<C_BoxCollider>();

}

void MoistureVaporator::Update()
{
	if (exploded)
	{
		//deactivate mesh, trigger
		vaporatorCollider->SetIsActive(false);
		vaporatorCollider->SetSize(vaporatorColliderSize);

		//play particles
		if (particleTimer < particleEmitttingTime)
		{
			particleTimer += MC_Time::Game::GetDT();
		}
		else
		{
			//deactivate everything
			explosionParticles->StopSpawn();
			exploded = false;
			gameObject->SetIsActive(false);
		}
	}

	if (toExplode)
	{
		explosion->SetEvent("item_barrel_explosion");
		explosion->SetVolume(5.0f);
		explosion->PlayFx(explosion->GetEventId());
		vaporatorObject->SetIsActive(false);

		explosionParticles->ResumeSpawn();

		exploded = true;
		toExplode = false;
	}

}

void MoistureVaporator::CleanUp()
{
	delete explosion;
}

void MoistureVaporator::OnCollisionEnter(GameObject* object)
{
	if (GetObjectScript(object, ObjectType::BULLET) != nullptr)
	{
		toExplode = true;
		vaporatorCollider->SetTrigger(true);
		vaporatorCollider->SetSize(stunTriggerSize);
	}
}

void MoistureVaporator::OnTriggerRepeat(GameObject* object)
{
	Entity* entity = (Entity*)GetObjectScript(object, ObjectType::ENTITY);
	if (!entity)
		return;

	entity->AddEffect(EffectType::STUN, stunDuration, false);
	entity->TakeDamage(damage);
}