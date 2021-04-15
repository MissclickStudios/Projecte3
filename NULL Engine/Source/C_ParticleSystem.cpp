#include "JSONParser.h"

#include "MC_Time.h"
#include "GameObject.h"

#include "Application.h"
#include "M_ResourceManager.h"

#include "R_ParticleSystem.h"

#include "I_Particles.h"

#include "C_ParticleSystem.h"

#include "MemoryManager.h"

C_ParticleSystem::C_ParticleSystem(GameObject* owner) : Component(owner, ComponentType::PARTICLES)
{
	AddDefaultEmitter();
	SetAsDefaultComponent();
}

C_ParticleSystem::~C_ParticleSystem()
{
	delete defaultEmitter;
	emitterInstances.clear();
}

bool C_ParticleSystem::SaveState(ParsonNode& root) const
{
	SaveParticleSystem();

	root.SetNumber("Type", (double)GetType());

	root.SetInteger("particleSystemUID",resource->GetUID());
	root.SetString("particleSystemAssetsPath", resource->GetAssetsPath());

	root.SetBool("stopSpawn", stopSpawn);
	root.SetBool("tempDelete", tempDelete);
	root.SetBool("previewEnabled", previewEnabled);
	root.SetBool("stopAndDeleteCheck", stopAndDeleteCheck);

	return true;
}

bool C_ParticleSystem::LoadState(ParsonNode& root)
{
	uint resourceUID = root.GetInteger("particleSystemUID");

	std::string path = root.GetString("particleSystemAssetsPath");
	resource = (R_ParticleSystem*)App->resourceManager->GetResourceFromLibrary(path.c_str());

	stopSpawn = root.GetBool("stopSpawn");
	tempDelete = root.GetBool("tempDelete");
	previewEnabled = root.GetBool("previewEnabled");
	stopAndDeleteCheck = root.GetBool("stopAndDeleteCheck");

	return true;
}

bool C_ParticleSystem::Update()
{
	if (previewEnabled == true && MC_Time::Game::GetDT() == 0)
	{
		for (unsigned int i = 0; i < emitterInstances.size(); ++i)
		{
			emitterInstances[i]->Update(MC_Time::Real::GetDT());
		}
	}
	else
	{
		for (unsigned int i = 0; i < emitterInstances.size(); ++i)
		{
			emitterInstances[i]->Update(MC_Time::Game::GetDT());
		}
	}

	if (stopAndDeleteCheck == true)
	{
		InternalStopAndDelete();
	}

	return true;
}

void C_ParticleSystem::SetParticleSystem(R_ParticleSystem* newParticleSystem)
{
	resource = newParticleSystem;
	RefreshEmitters();
}

void C_ParticleSystem::RefreshEmitters()
{
	Reset();
	emitterInstances.clear();

	for (auto emit = resource->emitters.begin(); emit != resource->emitters.end(); ++emit)
	{
		EmitterInstance* emitter = new EmitterInstance(&(*emit),this);
		emitterInstances.push_back(emitter);
	}
}

void C_ParticleSystem::AddParticleSystem(const char* name)
{
	//resource = new R_ParticleSystem();
	std::string assetsPath = ASSETS_PARTICLESYSTEMS_PATH + std::string(name) + PARTICLESYSTEMS_AST_EXTENSION;
	resource = (R_ParticleSystem*)App->resourceManager->CreateResource(ResourceType::PARTICLE_SYSTEM, assetsPath.c_str());

	resource->AddDefaultEmitter();
	RefreshEmitters();

}

void C_ParticleSystem::SaveParticleSystem() const
{
	App->resourceManager->SaveResourceToLibrary(resource);
	
	//char* buffer = nullptr;
	//Importer::Particles::Save(resource,&buffer);
}

bool C_ParticleSystem::SetAsDefaultComponent()
{
	bool ret = false;

	AddParticleSystem("Default Particle System");

	RefreshEmitters();

	return ret;
}

void C_ParticleSystem::AddDefaultEmitter()
{
	defaultEmitter = new Emitter();
	defaultEmitter->SetAsDefault();
}

void C_ParticleSystem::EnginePreview(bool previewEnabled)
{
	if (this->previewEnabled == previewEnabled)	//if the state of the particle system is different than the requested, clear all and switch mode.
	{
		return;
	}

	ClearEmitters();
	this->previewEnabled = previewEnabled;
}

void C_ParticleSystem::StopSpawn()
{
	for (int i = 0; i < emitterInstances.size(); i++)
	{
		emitterInstances[i]->stopSpawn = true;
	}
}

void C_ParticleSystem::ResumeSpawn()
{
	for (int i = 0; i < emitterInstances.size(); i++)
	{
		emitterInstances[i]->stopSpawn = false;
	}
}

void C_ParticleSystem::StopAndDelete()
{
	stopAndDeleteCheck = true;
	StopSpawn();
}

void C_ParticleSystem::InternalStopAndDelete()
{
	bool stopFinished = true;
	for (int i = 0; i < emitterInstances.size(); i++)
	{
		if (emitterInstances[i]->activeParticles > 0)
		{
			stopFinished = false;	//if all particles have died, just check next component
			break;
		}
	}

	if (stopFinished == true)
	{
		GameObject* owner = this->GetOwner();
		owner->DeleteComponent(this);
	}
}

void C_ParticleSystem::ClearEmitters()
{
	for(int i = 0; i < emitterInstances.size(); i++)
	{
		emitterInstances[i]->KillAll();
	}
}

void C_ParticleSystem::Reset()
{
	//loop all emitterInstances and trigger reset() method
	for (unsigned int i = 0; i < emitterInstances.size(); i++)
	{
		emitterInstances[i]->ResetEmitter();
	}
}



