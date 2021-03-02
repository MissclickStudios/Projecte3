#include "Log.h"
#include "Random.h"
#include "VariableTypedefs.h"

#include "GameObject.h"

#include "Component.h"

Component::Component(GameObject* owner, ComponentType type, bool isActive) :
id(Random::LCG::GetRandomUint()),
type(type),
owner(owner),
isActive(isActive)
{

}

Component::~Component()
{

}

bool Component::Update()
{
	return true;
}

bool Component::CleanUp()
{
	return true;
}

bool Component::SaveState(ParsonNode& root) const
{
	return true;
}

bool Component::LoadState(ParsonNode& root)
{
	return true;
}

// --- COMPONENT METHODS ---
const char* Component::GetNameFromType() const
{
	switch (type)
	{
	case ComponentType::NONE: { return "NONE"; }			break;
	case ComponentType::TRANSFORM: { return "Transform"; }		break;
	case ComponentType::MESH: { return "Mesh"; }			break;
	case ComponentType::MATERIAL: { return "Material"; }		break;
	case ComponentType::LIGHT: { return "Light"; }			break;
	case ComponentType::CAMERA: { return "Camera"; }		break;
	case ComponentType::ANIMATOR: { return "Animator"; }		break;
	case ComponentType::ANIMATION: { return "Animation"; }		break;
	case ComponentType::AUDIOSOURCE: {return "Audio Source";} break;
	case ComponentType::AUDIOLISTENER: {return "Audio Listener"; } break;
	}
	return "NONE";
}

uint32 Component::GetID() const
{
	return id;
}

void Component::ResetID()
{
	id = Random::LCG::GetRandomUint();
}

bool Component::IsActive() const
{
	return isActive;
}

void Component::SetIsActive(const bool& setTo)
{
	isActive = setTo;
}

GameObject* Component::GetOwner() const
{
	return owner;
}