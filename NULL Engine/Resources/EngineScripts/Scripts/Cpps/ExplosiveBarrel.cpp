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
#include "BarrelExplosion.h"

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
	GameObject* particleGameObject = gameObject->FindChild(particleName.c_str());
	if (particleGameObject != nullptr)
		activeParticles = particleGameObject->GetComponent<C_ParticleSystem>();

	explosionParticles->StopSpawn();
	
	GameObject* explosionGameObject = gameObject->FindChild(explosionName.c_str());
	if (explosionGameObject != nullptr)
	{
		explosionCollider = explosionGameObject->GetComponent<C_BoxCollider>();
		if (explosionCollider != nullptr)
			explosionCollider->SetIsActive(false);
	}

	cooldownTimer.Stop();
}

void ExplosiveBarrel::Update()
{
	if (paused)
		return;

	if (cooldownTimer.IsActive() && cooldownTimer.ReadSec() > cooldown)
	{
		cooldownTimer.Stop();
		if (activeParticles != nullptr)
			activeParticles->ResumeSpawn();
	}


	if (toExplode)
	{
		toExplode = false;

		explosionCollider->SetIsActive(true);
		explosionParticles->ResumeSpawn();
		particleTimer.Start();
		if (activeParticles != nullptr)
			activeParticles->StopSpawn();

		explosion->SetEvent(explosionAudio.c_str());
		explosion->SetVolume(5.0f);
		explosion->PlayFx(explosion->GetEventId());

		exploded = true;
	} 
	else if (exploded)
	{
		if (particleTimer.ReadSec() > particleEmitttingTime)
		{
			exploded = false;

			explosionCollider->SetIsActive(false);
			explosionParticles->StopSpawn();
			particleTimer.Stop();

			if (!reload)
				Deactivate();
			else
				cooldownTimer.Start();
		}
	}
}

void ExplosiveBarrel::CleanUp()
{
	delete explosion;
}

void ExplosiveBarrel::OnCollisionEnter(GameObject* object)
{
	if (GetObjectScript(object, ObjectType::BULLET) != nullptr && !cooldownTimer.IsActive())
	{
		toExplode = true;
		GameObject* explosionGameObject = gameObject->FindChild(explosionName.c_str());
		if (explosionGameObject != nullptr)
		{
			BarrelExplosion* explosionScript = (BarrelExplosion*)GetObjectScript(explosionGameObject, ObjectType::COLLECTABLE);
			if (explosionScript != nullptr)
				explosionScript->state = 0;
		}
	}
}

void ExplosiveBarrel::OnPause()
{
	paused = true;

	cooldownTimer.Pause();
}

void ExplosiveBarrel::OnResume()
{
	paused = false;

	cooldownTimer.Resume();
}

ExplosiveBarrel* CreateExplosiveBarrel()
{
	ExplosiveBarrel* script = new ExplosiveBarrel();

	INSPECTOR_INPUT_FLOAT(script->particleEmitttingTime);
	INSPECTOR_STRING(script->explosionName);
	INSPECTOR_STRING(script->explosionAudio);
	INSPECTOR_STRING(script->particleName);
	INSPECTOR_CHECKBOX_BOOL(script->reload);
	INSPECTOR_INPUT_FLOAT(script->cooldown);
	
	return script;
}