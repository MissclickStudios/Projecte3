#include "JSONParser.h"

#include "Application.h"
#include "M_Scene.h"

#include "C_BulletBehavior.h"

#include "GameObject.h"

C_BulletBehavior::C_BulletBehavior(GameObject* owner) : Component(owner, ComponentType::BULLET_BEHAVIOR)
{
	autodestructTimer.Stop(); // Deleting yourself crashes the engine
}

C_BulletBehavior::~C_BulletBehavior()
{
}

bool C_BulletBehavior::Update()
{
	if (autodestructTimer.ReadSec() >= autodestruct)
		App->scene->DeleteGameObject(GetOwner());

	return true;
}

bool C_BulletBehavior::CleanUp()
{
	return true;
}

bool C_BulletBehavior::SaveState(ParsonNode& root) const
{
	root.SetNumber("Type", (uint)GetType());

	root.SetNumber("Autodestruct", (double)autodestruct);

	return true;
}

bool C_BulletBehavior::LoadState(ParsonNode& root)
{
	autodestruct = (float)root.GetNumber("Autodestruct");

	return true;
}

void C_BulletBehavior::OnCollisionEnter()
{
	GetOwner()->to_delete = true;
}