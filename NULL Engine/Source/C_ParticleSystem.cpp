#include "JSONParser.h"

#include "C_ParticleSystem.h"
#include "R_ParticleSystem.h"
#include "M_ResourceManager.h"
#include "Time.h"

#include "MemoryManager.h"

C_ParticleSystem::C_ParticleSystem(GameObject* owner) : Component(owner, ComponentType::PARTICLE_SYSTEM)
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
	for (unsigned int i = 0; i < emitterInstances.size(); ++i)
	{
		emitterInstances[i]->Update(Time::Game::GetDT());
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

void C_ParticleSystem::Reset()
{
	//loop all emitterInstances and trigger reset() method
	for (unsigned int i = 0; i < emitterInstances.size(); i++)
	{
		emitterInstances[i]->ResetEmitter();
	}
}

