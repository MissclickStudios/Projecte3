#include "JSONParser.h"

#include "Application.h"
#include "M_Scene.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_PlayerController.h"
#include "C_CameraBehavior.h"

C_CameraBehavior::C_CameraBehavior(GameObject* owner) : Component(owner, ComponentType::CAMERA_BEHAVIOR)
{

}

C_CameraBehavior::~C_CameraBehavior()
{
}

bool C_CameraBehavior::Update()
{
	if (!player)
	{
		std::vector<GameObject*>* objects = App->scene->GetGameObjects();
		for (int i = 0; i < objects->size(); ++i)
			if ((*objects)[i]->GetComponent<C_PlayerController>())
			{
				player = (*objects)[i];
				break;
			}
	}
	else
	{
		float3 position = player->transform->GetWorldPosition();
		position += offset;
		GetOwner()->transform->SetWorldPosition(position);
	}

	return true;
}

bool C_CameraBehavior::CleanUp()
{
	return true;
}

bool C_CameraBehavior::SaveState(ParsonNode& root) const
{
	root.SetNumber("Type", (uint)GetType());

	root.SetNumber("Offset X", (double)offset.x);
	root.SetNumber("Offset Y", (double)offset.y);
	root.SetNumber("Offset Z", (double)offset.z);

	return true;
}

bool C_CameraBehavior::LoadState(ParsonNode& root)
{
	offset.x = (float)root.GetNumber("Offset X");
	offset.y = (float)root.GetNumber("Offset Y");
	offset.z = (float)root.GetNumber("Offset Z");

	return true;
}