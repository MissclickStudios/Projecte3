#include "VariableTypedefs.h"
#include "Log.h"

#include "Application.h"

#include "M_Scene.h"

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
		std::vector<GameObject*>* gameObjects = App->scene->GetGameObjects();
		for (auto object = gameObjects->begin(); object != gameObjects->end(); ++object)
			if ((*object)->GetScript("Player"))
				player = (*object);

		if (!player)
			return;
	}
	
	float3 position = player->transform->GetWorldPosition();
	position += offset;
	gameObject->transform->SetWorldPosition(position);
}

void CameraMovement::CleanUp()
{
	LOG("CleanUp");
}