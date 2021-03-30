#include "Application.h"
#include "M_Scene.h"
#include "Log.h"

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
		//TODO
		//for (int i = 0; i < objects->size(); ++i)
		//	if ((*objects)[i]->GetComponent<C_PlayerController>())
		//	{
		//		player = (*objects)[i];
		//		break;
		//	}
	}
	else
	{
		float3 position = player->transform->GetWorldPosition();
		position += offset;
		gameObject->transform->SetWorldPosition(position);
	}
}

void CameraMovement::CleanUp()
{
	LOG("CleanUp");
}