#include "VariableTypedefs.h"
#include "Log.h"

#include "Application.h"

#include "M_Scene.h"

#include "M_Scene.h"

#include "GameObject.h"
#include "C_Transform.h"

#include "CameraMovement.h"
#include "Entity.h"
#include "Player.h"

#include "Random.h"
#include "MC_Time.h"

#include "M_Input.h"


CameraMovement::CameraMovement() : Script()
{
}

CameraMovement::~CameraMovement()
{
}

CameraMovement* CreateCameraMovement() {
	CameraMovement* script = new CameraMovement();

	INSPECTOR_DRAGABLE_FLOAT3(script->offset);
	INSPECTOR_STRING(script->playerName);
	INSPECTOR_INPUT_FLOAT(script->shakeMagnitude);
	INSPECTOR_INPUT_FLOAT(script->shakeDuration);
	INSPECTOR_INPUT_FLOAT(script->cameraSpeed);
	INSPECTOR_INPUT_FLOAT(script->distanceToTransition);
	INSPECTOR_STRING(script->destinationPointsName);
	return script;
}

void CameraMovement::Start()
{
	player = App->scene->GetGameObjectByName(playerName.c_str());
	playerScript = (Player*)player->GetScript("Player");
	initialRot = gameObject->transform->GetWorldRotation();
	destinationPoints = App->scene->GetGameObjectByName(destinationPointsName.c_str());
}

void CameraMovement::Update()
{
	if (player == nullptr)
		return;

	if (App->input->GetKey(SDL_SCANCODE_M) == KeyState::KEY_REPEAT && destinationPoints != nullptr)
	{
		nextPointProgress += (MC_Time::Game::GetDT() * cameraSpeed);
		MoveCameraTo(destinationPoints, nextPointProgress);
		return;
	}

	float3 position = player->transform->GetWorldPosition();
	position += offset;

	gameObject->transform->SetWorldPosition(position);

	if (!gameObject->transform->GetWorldRotation().Equals(initialRot)) 
		gameObject->transform->SetWorldRotation(initialRot);
	
	if (playerScript != nullptr & playerScript->hitTimer.IsActive()) 
		CameraShake(shakeDuration, shakeMagnitude);
}

void CameraMovement::CleanUp()
{
	LOG("CleanUp");
}

void CameraMovement::CameraShake(float duration, float magnitude)
{
	float3 originalPos = gameObject->transform->GetWorldPosition();
	float x = Random::LCG::GetBoundedRandomFloat(-1.0f, 1.0f) * magnitude;
	float y = Random::LCG::GetBoundedRandomFloat(-1.0f, 1.0f) * magnitude;

	if (duration > 0.0f)
	{
		duration -= MC_Time::Game::GetDT();
		
		gameObject->transform->SetWorldPosition(float3(originalPos.x + x, originalPos.y + y, originalPos.z));
	}
	else
	{
		gameObject->transform->SetWorldPosition(originalPos);
		duration = 0;
	}

}

bool CameraMovement::MoveCameraTo(GameObject* destination, float progress)
{
	if (nextPoint >= destination->childs.size())
		return true;

	destinationPos = destination->childs[nextPoint]->transform->GetWorldPosition();

	float3 nextPos = destinationPos.Lerp(gameObject->transform->GetWorldPosition(), 1.0f - progress);

	Quat destinationRot = destination->childs[nextPoint]->transform->GetWorldRotation();

	Quat nextRot = destinationRot.Lerp(gameObject->transform->GetWorldRotation(), 1.0f - progress);

	gameObject->transform->SetWorldPosition(nextPos);

	gameObject->transform->SetWorldRotation(nextRot);

	if (gameObject->transform->GetWorldPosition().Distance(destinationPos) < distanceToTransition)
	{
		nextPoint++;
		nextPointProgress = 0;
		progress = 0;
	}

	return false;
}
