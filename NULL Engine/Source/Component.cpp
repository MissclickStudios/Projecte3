#include "Log.h"
#include "Random.h"
#include "VariableTypedefs.h"

#include "GameObject.h"

#include "Component.h"

#include "MemoryManager.h"

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
	case ComponentType::RIGIDBODY:		  { return "RigidBody"; }			break;
	case ComponentType::BOX_COLLIDER:	  { return "Box Collider"; }		break;
	case ComponentType::SPHERE_COLLIDER:  { return "Sphere  Collider"; }	break;
	case ComponentType::CAPSULE_COLLIDER: { return "Capsule Collider"; }	break;
	case ComponentType::CANVAS:			{ return "Canvas"; }		break;
	case ComponentType::PLAYER_CONTROLLER: { return "Player Controller"; }	break;
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

void Component::SetIsActive(bool setTo)
{
	isActive = setTo;
}

GameObject* Component::GetOwner() const
{
	return owner;
}