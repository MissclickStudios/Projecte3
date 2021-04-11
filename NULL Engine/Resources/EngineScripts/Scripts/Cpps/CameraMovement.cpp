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
		std::map<uint32, GameObject*>* objects = App->scene->GetGameObjects();
		for (auto i = objects->begin(); i != objects->end(); ++i)
			if (i->second->GetScript("Player"))
				player = i->second;

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