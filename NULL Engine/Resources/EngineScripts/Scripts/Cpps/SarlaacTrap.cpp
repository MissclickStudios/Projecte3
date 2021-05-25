#include "Application.h"
#include "MC_Time.h"
#include "M_Scene.h"
#include "Log.h"
#include "GameManager.h"
#include "GameObject.h"

#include "C_Animator.h"
#include "C_ParticleSystem.h"
#include "C_BoxCollider.h"

#include "SarlaacTrap.h"

SarlaacTrap::SarlaacTrap() : Script()
{
}

SarlaacTrap::~SarlaacTrap()
{
}

void SarlaacTrap::Start()
{

	sarlaacAnimator = gameObject->GetComponent<C_Animator>();

	GameObject* idleParticlesGO = gameObject->FindChild("Idle");
	GameObject* attackParticlesGO = gameObject->FindChild("Attack");

	idleParticles = (idleParticlesGO != nullptr) ? idleParticlesGO->GetComponent<C_ParticleSystem>() : nullptr;
	attackParticles = (attackParticlesGO != nullptr) ? attackParticlesGO->GetComponent<C_ParticleSystem>() : nullptr;

	(idleParticles != nullptr) ? idleParticles->StopSpawn() : LOG("[ERROR] SarlaccTrap Script: Could not find { IDLE } Particle System!");
	(attackParticles != nullptr) ? attackParticles->StopSpawn() : LOG("[ERROR] SarlaccTrap Script: Could not find { ATTACK } Particle System!");
}

void SarlaacTrap::Update()
{	
	switch (state)
	{

	case SarlaacState::IDLE:
		if (idleParticles != nullptr)
			idleParticles->ResumeSpawn();
		break;

	case SarlaacState::DAMAGING:

		state = SarlaacState::SLEEPING;

		break;

	case SarlaacState::MOVING:

		if (animationTimer >= activationTime)
		{
			state = SarlaacState::DAMAGING;
			animationTimer = 0.f;
		}
		else
		{
			animationTimer += MC_Time::Game::GetDT();
		}


		break;

	case SarlaacState::SLEEPING:

		if (animationTimer >= sleepingTime)
		{
			state = SarlaacState::IDLE;
			animationTimer = 0.f;

			if (attackParticles != nullptr)
				attackParticles->StopSpawn();
		}
		else
		{
			animationTimer += MC_Time::Game::GetDT();
		}

		break;
	}

}

void SarlaacTrap::CleanUp()
{
}

void SarlaacTrap::OnTriggerRepeat(GameObject* object)
{
	Entity* entity = (Entity*)GetObjectScript(object, ObjectType::ENTITY);

	if (!entity)
		return;


	switch (state)
	{

		case SarlaacState::IDLE:

			StartMoving();

			break;

		case SarlaacState::DAMAGING:

			entity->TakeDamage(damage);

			break;

		case SarlaacState::MOVING:

			break;

		case SarlaacState::SLEEPING:

			break;
	}
	
	

}

void SarlaacTrap::StartMoving()
{
	state = SarlaacState::MOVING;

	if (idleParticles != nullptr)
		idleParticles->StopSpawn();

	if (attackParticles != nullptr)
		attackParticles->ResumeSpawn();

	//Animator play clip
	sarlaacAnimator->PlayClip("Preview",animationName.c_str(), 0u);
}