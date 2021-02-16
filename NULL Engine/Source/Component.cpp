#include "Log.h"
#include "Random.h"
#include "VariableTypedefs.h"

#include "GameObject.h"

#include "Component.h"

Component::Component(GameObject* owner, COMPONENT_TYPE type, bool is_active) :
id			(Random::LCG::GetRandomUint()),
type		(type),
owner		(owner),
is_active	(is_active)
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
	case COMPONENT_TYPE::NONE:			{ return "NONE"; }			break;
	case COMPONENT_TYPE::TRANSFORM:		{ return "Transform"; }		break;
	case COMPONENT_TYPE::MESH:			{ return "Mesh"; }			break;
	case COMPONENT_TYPE::MATERIAL:		{ return "Material"; }		break;
	case COMPONENT_TYPE::LIGHT:			{ return "Light"; }			break;
	case COMPONENT_TYPE::CAMERA:		{ return "Camera"; }		break;
	case COMPONENT_TYPE::ANIMATOR:		{ return "Animator"; }		break;
	case COMPONENT_TYPE::ANIMATION:		{ return "Animation"; }		break;
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
	return is_active;
}

void Component::SetIsActive(const bool& set_to)
{
	is_active = set_to;
}

GameObject* Component::GetOwner() const
{
	return owner;
}