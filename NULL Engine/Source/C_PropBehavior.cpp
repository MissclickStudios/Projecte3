#include "JSONParser.h"

#include "C_PropBehavior.h"

#include "GameObject.h"

C_PropBehavior::C_PropBehavior(GameObject* owner) : Component(owner, ComponentType::PROP_BEHAVIOR)
{
}

C_PropBehavior::~C_PropBehavior()
{
}

bool C_PropBehavior::Update()
{
	return true;
}

bool C_PropBehavior::CleanUp()
{
	return true;
}

bool C_PropBehavior::SaveState(ParsonNode& root) const
{
	root.SetNumber("Type", (uint)GetType());

	return true;
}

bool C_PropBehavior::LoadState(ParsonNode& root)
{
	return true;
}

void C_PropBehavior::OnCollisionEnter()
{
	GetOwner()->to_delete = true;
}