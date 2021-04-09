#include "JSONParser.h"

#include "Time.h"

#include "M_ResourceManager.h"

#include "R_Particles.h"

#include "C_Particles.h"

#include "MemoryManager.h"

C_Particles::C_Particles(GameObject* owner) : Component(owner, ComponentType::PARTICLES)
{
	AddDefaultEmitter();
	SetAsDefaultComponent();
}

C_Particles::~C_Particles()
{
	delete defaultEmitter;
	emitterInstances.clear();
}

bool C_Particles::SaveState(ParsonNode& root) const
{
	root.SetNumber("Type", (double)GetType());

	return false;
}

bool C_Particles::LoadState(ParsonNode& root)
{


	return false;
}

bool C_Particles::Update()
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

bool C_Particles::SetAsDefaultComponent()
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

void C_Particles::AddDefaultEmitter()
{
	defaultEmitter = new Emitter();
	defaultEmitter->SetAsDefault();
}

void C_Particles::EnginePreview(bool previewEnabled)
{
	if (this->previewEnabled == previewEnabled)	//if the state of the particle system is different than the requested, clear all and switch mode.
	{
		return;
	}

	ClearEmitters();
	this->previewEnabled = previewEnabled;
}

void C_Particles::ClearEmitters()
{
	for(int i = 0; i < emitterInstances.size(); i++)
	{
		emitterInstances[i]->KillAll();
	}
}

void C_Particles::Reset()
{
	//loop all emitterInstances and trigger reset() method
	for (unsigned int i = 0; i < emitterInstances.size(); i++)
	{
		emitterInstances[i]->ResetEmitter();
	}
}

