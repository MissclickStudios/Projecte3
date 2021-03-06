#include "EmitterInstance.h"

EmitterInstance::EmitterInstance()
{
}

EmitterInstance::~EmitterInstance()
{
}

void EmitterInstance::Init(Emitter* emitter, C_ParticleSystem* component)
{
	this->emitter = emitter;
	this->component = component;

	particles.resize(emitter->maxParticleCount);
}

void EmitterInstance::Update(float dt)
{
	KillDeadParticles();
	UpdateModules();
	DrawParticles();
}

void EmitterInstance::UpdateModules()
{
	/*for (int i = 0; i < emitter->modules.size(); ++i)
	{
		emitterReference->modules[i]->Update(dt, this);
	}*/
}

void EmitterInstance::DrawParticles()
{
	//draw should ask the renderer to print a particle and give the details(transform, material, color,
	//distance to camera, and any other thing i consider necesary).
}

void EmitterInstance::KillDeadParticles()
{
	 
}

void EmitterInstance::KillAll()
{
}
