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
	explosionAudio = new C_AudioSource(gameObject);

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

	explosionMarker = gameObject->FindChild(markerName.c_str());
	if (explosionMarker != nullptr)
		explosionMarker->SetIsActive(false);

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

		if (explosionMarker != nullptr)
			explosionMarker->SetIsActive(true);

		if (reload == true)
		{
			if (explosionAudio != nullptr)
			{
				explosionAudio->SetEvent("moisture_active");
				explosionAudio->PlayFx(explosionAudio->GetEventId());
			}
		}
		else
		{
			if (explosionAudio != nullptr)
			{
				explosionAudio->SetEvent("barrel_active");
				explosionAudio->PlayFx(explosionAudio->GetEventId());
			}

		}

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

			if (explosionMarker != nullptr)
				explosionMarker->SetIsActive(false);

			if (!reload)
				Deactivate();
			else
				cooldownTimer.Start();
		}
	}
}

void ExplosiveBarrel::CleanUp()
{
	if (explosionAudio != nullptr)	
		delete explosionAudio;
}

void ExplosiveBarrel::OnCollisionEnter(GameObject* object)
{
	if (toExplode || exploded)
		return;

	if (GetObjectScript(object, ObjectType::BULLET) != nullptr && !cooldownTimer.IsActive())
	{
		toExplode = true;
		GameObject* explosionGameObject = gameObject->FindChild(explosionName.c_str());
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
	INSPECTOR_STRING(script->markerName);
	INSPECTOR_STRING(script->particleName);
	INSPECTOR_CHECKBOX_BOOL(script->reload);
	INSPECTOR_INPUT_FLOAT(script->cooldown);
	
	return script;
}