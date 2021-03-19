#include "Application.h"
#include "M_Scene.h"

#include "C_GateBehavior.h"

C_GateBehavior::C_GateBehavior(GameObject* owner) : Component(owner, ComponentType::GATE_BEHAVIOR)
{
}

C_GateBehavior::~C_GateBehavior()
{
}

bool C_GateBehavior::Update()
{
	return true;
}

bool C_GateBehavior::CleanUp()
{
	return true;
}

bool C_GateBehavior::SaveState(ParsonNode& root) const
{
	return true;
}

bool C_GateBehavior::LoadState(ParsonNode& root)
{
	return true;
}

void C_GateBehavior::OnCollisionEnter()
{
	// Change Sceme
	int i = 0;
	i++;

	int a = 2;
	int b = a + i;

	return;
}
