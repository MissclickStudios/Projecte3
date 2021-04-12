#include "JSONParser.h"

#include "Time.h"

#include "M_ResourceManager.h"

#include "R_ParticleSystem.h"

#include "C_Particles.h"

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
	root.SetNumber("Type", (double)GetType());

	return false;
}

bool C_ParticleSystem::LoadState(ParsonNode& root)
{


	return false;
}

bool C_ParticleSystem::Update()
{

	if (previewEnabled == true && Time::Game::GetDT() == 0)
	{
		for (unsigned int i = 0; i < emitterInstances.size(); ++i)
		{
			emitterInstances[i]->Update(Time::Real::GetDT());
		}
	}
	else
	{
		for (unsigned int i = 0; i < emitterInstances.size(); ++i)
		{
			emitterInstances[i]->Update(Time::Game::GetDT());
		}
	}

	return true;
}

bool C_ParticleSystem::SetAsDefaultComponent()
{
	bool ret = false;

	Reset();
	emitterInstances.clear();

	if (defaultEmitter != nullptr)
	{
		EmitterInstance* emitter = new EmitterInstance();

		emitterInstances.push_back(emitter);
		emitterInstances.back()->Init(defaultEmitter, this);

		ret = true;
	}
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

