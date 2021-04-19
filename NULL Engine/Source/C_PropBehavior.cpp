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
	/*if (dead)
	{
		dead = false;

		for (uint i = 0; i < GetOwner()->components.size(); ++i)
			GetOwner()->components[i]->SetIsActive(false);
		GetOwner()->SetIsActive(false);
	}*/

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
	dead = true;
}