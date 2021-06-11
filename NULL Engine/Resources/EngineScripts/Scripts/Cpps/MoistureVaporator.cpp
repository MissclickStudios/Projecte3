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
	moistureAudio = new C_AudioSource(gameObject);

	if (moistureAudio != nullptr)
		moistureAudio->SetEvent("moisture_active");

	gameManager = App->scene->GetGameObjectByName(gameManagerName.c_str());
	vaporatorObject = gameObject->FindChild(vaporatorObjectName.c_str());

	explosionParticles = gameObject->FindChild("StunParticles")->GetComponent<C_ParticleSystem>();
	explosionParticles->StopSpawn();

	idleParticles = gameObject->FindChild("Idle")->GetComponent<C_ParticleSystem>();
	idleParticles->ResumeSpawn(); // Not sure what default state is

	vaporatorCollider = gameObject->GetComponent<C_BoxCollider>();
	vaporatorColliderSize = vaporatorCollider->Size();

	cooldownTimer.Stop();
	explosionParticlesTimer.Stop();

	state = VaporatorState::IDLE;
}

void MoistureVaporator::Update()
{
	switch (state)
	{
		case VaporatorState::IDLE:
		{
			break;
		}
		case VaporatorState::STUNNING:
		{
			// Cooldown 
			cooldownTimer.Start();

			// Audio
			if(moistureAudio != nullptr)
				moistureAudio->PlayFx(moistureAudio->GetEventId());

			// Particles and particle timer 
			idleParticles->StopSpawn();
			explosionParticles->ResumeSpawn();
			explosionParticlesTimer.Start();

			// Collider
			vaporatorCollider->SetTrigger(false);
			vaporatorCollider->SetSize(vaporatorColliderSize);

			// State
			state = VaporatorState::REFRESHING;

			break;
		}
		case VaporatorState::REFRESHING:
		{
			// Particles
			if (explosionParticlesTimer.IsActive())
			{
				if (explosionParticlesTimer.ReadSec() > particleEmitttingTime)
				{
					explosionParticlesTimer.Stop();

					explosionParticles->StopSpawn();
				}
			}

			// Cooldown
			if (cooldownTimer.ReadSec() > cooldown)
			{
				cooldownTimer.Stop();

				state = VaporatorState::IDLE;

				idleParticles->ResumeSpawn();

				// Just in case
				explosionParticlesTimer.Stop();
				explosionParticles->StopSpawn();
			}
			break;
		}
	}
	//if (exploded)
	//{
	//	//deactivate mesh, trigger
	//	vaporatorCollider->SetIsActive(false);
	//	vaporatorCollider->SetSize(vaporatorColliderSize);

	//	//play particles
	//	if (particleTimer < particleEmitttingTime)
	//	{
	//		particleTimer += MC_Time::Game::GetDT();
	//	}
	//	else
	//	{
	//		//deactivate everything
	//		explosionParticles->StopSpawn();
	//		exploded = false;
	//		gameObject->SetIsActive(false);
	//	}
	//}

	//if (toExplode)
	//{
	//	explosion->SetEvent("item_barrel_explosion");
	//	explosion->SetVolume(5.0f);
	//	explosion->PlayFx(explosion->GetEventId());
	//	vaporatorObject->SetIsActive(false);

	//	explosionParticles->ResumeSpawn();

	//	exploded = true;
	//	toExplode = false;
	//}

}

void MoistureVaporator::CleanUp()
{
	if (moistureAudio != nullptr)
		delete moistureAudio;
}

void MoistureVaporator::OnCollisionEnter(GameObject* object)
{
	if (GetObjectScript(object, ObjectType::BULLET) != nullptr && state == VaporatorState::IDLE)
	{
		state = VaporatorState::STUNNING;
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