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

	barrelCollider = gameObject->GetComponent<C_BoxCollider>();
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
			gameObject->SetIsActive(false);
		}
	}

	if (toExplode) //Set collider big to explode / play audio / Play particles
	{
		explosion->SetEvent("item_barrel_explosion");
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
		barrelCollider->SetIsActive(false);
	}
}

ExplosiveBarrel* CreateExplosiveBarrel() {
	ExplosiveBarrel* script = new ExplosiveBarrel();
	INSPECTOR_INPUT_FLOAT3(script->barrelColliderSize);
	INSPECTOR_INPUT_FLOAT3(script->explosionTriggerSize);
	INSPECTOR_INPUT_FLOAT(script->particleEmitttingTime);
	return script;
}