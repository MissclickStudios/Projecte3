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
		return;

	float3 position = player->transform->GetWorldPosition();
	position += offset;
	gameObject->transform->SetWorldPosition(position);
}

void CameraMovement::CleanUp()
{
	LOG("CleanUp");
}