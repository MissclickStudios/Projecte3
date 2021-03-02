#include "Component.h"
#include "RParticleSystem.h"
#include "CParticleSystem.h"


CParticleSystem::CParticleSystem(GameObject* owner) : Component(ComponentType::PARTICLESYSTEM, owner)
{

}

CParticleSystem::~CParticleSystem()
{

}

void CParticleSystem::Update(float dt)
{

}

void CParticleSystem::Reset()
{

}

void CParticleSystem::Serialize(JsonNode* node)
{

}

void CParticleSystem::Load(JsonNode* node)
{

}