#include "JSONParser.h"

#include "C_BulletBehavior.h"

#include "GameObject.h"

C_BulletBehavior::C_BulletBehavior(GameObject* owner) : Component(owner, ComponentType::BULLET_BEHAVIOR)
{
}

C_BulletBehavior::~C_BulletBehavior()
{
}

bool C_BulletBehavior::Update()
{
	return true;
}

bool C_BulletBehavior::CleanUp()
{
	return true;
}

bool C_BulletBehavior::SaveState(ParsonNode& root) const
{
	root.SetNumber("Type", (uint)GetType());

	return true;
}

bool C_BulletBehavior::LoadState(ParsonNode& root)
{
	return true;
}

void C_BulletBehavior::OnCollisionEnter()
{
	GetOwner()->to_delete = true;
}