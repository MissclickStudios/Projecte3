#include "Application.h"
#include "Log.h"

#include "M_Scene.h"

#include "GameObject.h"
#include "C_Transform.h"

#include "CameraMovement.h"

CameraMovement::CameraMovement() : Script()
{
}

CameraMovement::~CameraMovement()
{
}

void CameraMovement::Update()
{
	if (!player)
	{
		std::vector<GameObject*>* objects = App->scene->GetGameObjects();
		for (uint i = 0; i < objects->size(); ++i)
		{
			GameObject* object = (*objects)[i];
			for (uint n = 0; n < object->components.size(); ++n)
			{
				Component* comp = object->components[n];
				if (comp->GetType() == ComponentType::SCRIPT)
				{
					C_Script* script = (C_Script*)comp;
					if (script->GetDataName() == "Player")
						player = object;
				}
			}
		}
	}

	float3 position = player->transform->GetWorldPosition();
	position += offset;
	gameObject->transform->SetWorldPosition(position);
}

void CameraMovement::CleanUp()
{
	LOG("CleanUp");
}