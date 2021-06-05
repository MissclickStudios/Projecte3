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
#include "C_RigidBody.h"

#include "ExplosiveBarrel.h"

ExplosiveBarrel::ExplosiveBarrel() : Object()
{
	baseType = ObjectType::EXPLOSIVE_BARREL;
}

ExplosiveBarrel::~ExplosiveBarrel()
{
}

void ExplosiveBarrel::Start()
{
	explosion = new C_AudioSource(gameObject);

	explosionParticles = gameObject->GetComponent<C_ParticleSystem>();

	explosionParticles->StopSpawn();

	explosionGameObject = App->scene->GetGameObjectByName(explosionName.c_str());
}

void ExplosiveBarrel::Update()
{
	if (exploded)
	{
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
			explosionGameObject->SetIsActive(false);
			C_BoxCollider* collider = explosionGameObject->GetComponent<C_BoxCollider>();
			if (collider != nullptr)
				collider->SetIsActive(false);
			C_RigidBody* rigidBody = gameObject->GetComponent<C_RigidBody>();
			if (rigidBody != nullptr)
				rigidBody->ChangeFilter("allow bullets");
		}
	}

	if (toExplode) //Set collider big to explode / play audio / Play particles
	{
		explosion->SetEvent(explosionAudio.c_str());
		explosion->SetVolume(5.0f);
 		explosion->PlayFx(explosion->GetEventId());

		explosionParticles->ResumeSpawn();

		//exploded = true;
		exploded = true;
		toExplode = false;
	}

}

void ExplosiveBarrel::CleanUp()
{
	delete explosion;
}

void ExplosiveBarrel::OnCollisionEnter(GameObject* object)
{
  	if (GetObjectScript(object, ObjectType::BULLET) != nullptr)
	{
		toExplode = true;
		explosionGameObject->SetIsActive(true);
	}
}

ExplosiveBarrel* CreateExplosiveBarrel()
{
	ExplosiveBarrel* script = new ExplosiveBarrel();

	INSPECTOR_INPUT_FLOAT(script->particleEmitttingTime);
	INSPECTOR_STRING(script->explosionName);
	INSPECTOR_STRING(script->explosionAudio);

	return script;
}