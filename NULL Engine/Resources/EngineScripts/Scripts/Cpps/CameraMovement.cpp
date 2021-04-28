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

void CameraMovement::Start()
{
	player = App->scene->GetGameObjectByName(playerName.c_str());
}

void CameraMovement::Update()
{

	if (player == nullptr)
		return;

	float3 position = player->transform->GetWorldPosition();
	position += offset;
	gameObject->transform->SetWorldPosition(position);
}

void CameraMovement::CleanUp()
{
	LOG("CleanUp");
}