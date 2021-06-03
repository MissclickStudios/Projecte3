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

SarlaacTrap::SarlaacTrap() : Object()
{
	baseType = ObjectType::EXPLOSIVE_BARREL; // cant be bothered creating this useless type, i rather prefer shotting myself in the foot than staving myself in the heart
}

SarlaacTrap::~SarlaacTrap()
{
}

void SarlaacTrap::Start()
{
	sarlaacAnimator = gameObject->GetComponent<C_Animator>();
	StartMoving();
}

void SarlaacTrap::Update()
{	
	if (paused)
		return;

	switch (state)
	{

	case SarlaacState::IDLE:

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
	if (paused)
		return;

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

	//Animator play clip
	sarlaacAnimator->PlayClip("Preview",animationName.c_str(), 0u);
}