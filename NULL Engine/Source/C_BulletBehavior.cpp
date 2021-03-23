#include "JSONParser.h"

#include "Application.h"
#include "M_Scene.h"

#include "C_BulletBehavior.h"
#include "C_PlayerController.h"
#include "C_Mesh.h"
#include "C_Transform.h"

#include "GameObject.h"

C_BulletBehavior::C_BulletBehavior(GameObject* owner) : Component(owner, ComponentType::BULLET_BEHAVIOR)
{
	autodestructTimer.Stop();
}

C_BulletBehavior::~C_BulletBehavior()
{
}

bool C_BulletBehavior::Update()
{
	if (autodestructTimer.ReadSec() >= autodestruct)
		hit = true;

	if (hit)
	{
		hit = false;

		GetOwner()->transform->SetWorldPosition(float3::zero);
		for (uint i = 0; i < GetOwner()->components.size(); ++i)
			GetOwner()->components[i]->SetIsActive(false);
		GetOwner()->SetIsActive(false);

		shooter->GetComponent<C_PlayerController>()->bullets[index]->inUse = false;
		autodestructTimer.Stop();
	}

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
	hit = true;
}

void C_BulletBehavior::SetShooter(GameObject* shooter, uint index)
{
	this->shooter = shooter;
	this->index = index;
}
