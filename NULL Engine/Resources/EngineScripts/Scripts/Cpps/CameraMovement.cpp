#include "VariableTypedefs.h"
#include "Log.h"

#include "Application.h"

#include "M_Scene.h"

#include "M_Scene.h"

#include "GameObject.h"
#include "C_Transform.h"

#include "GameManager.h"
#include "CameraMovement.h"
#include "DialogManager.h"
#include "Entity.h"
#include "Player.h"
#include "IG12.h"

#include "Random.h"
#include "MC_Time.h"

#include "M_Input.h"
#include "EasingFunctions.h"


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
	INSPECTOR_STRING(script->playerPointsName);
	return script;
}

void CameraMovement::Start()
{
	player = App->scene->GetGameObjectByName(playerName.c_str());
	if(player)
		playerScript = (Player*)player->GetScript("Player");

	 GameObject* ig12 = App->scene->GetGameObjectByName("IG12");
	 if (ig12)
		 ig12Script = (IG12*)ig12->GetScript("IG12");

	gameManagerObject = App->scene->GetGameObjectByName(gameManagerName.c_str());	
	if (gameManagerObject != nullptr)
		gameManagerScript = (GameManager*)gameManagerObject->GetScript("GameManager");

	initialRot = Quat(-0.4461978,0,0, 0.8949344);

	if(gameObject->parent != nullptr)
		gameObject->parent->transform->SetLocalRotation(float3(0,0,0));

	destinationPoints = App->scene->GetGameObjectByName(destinationPointsName.c_str());
	playerDestinationPoints = App->scene->GetGameObjectByName(playerPointsName.c_str());
}

void CameraMovement::Update()
{
	if (player == nullptr)
		return;

	if (gameManagerScript->doCameraCutscene)
	{
		nextPointProgress += (MC_Time::Game::GetDT() * cameraSpeed);
		MoveCameraTo(destinationPoints, nextPointProgress);
		return;
	}
	else if (playerScript->doDieCutscene && playerDestinationPoints != nullptr)
	{
		nextPointProgress += (MC_Time::Game::GetDT() * cameraSpeed) / 7;
		MoveCameraTo(playerDestinationPoints, nextPointProgress);
		return;
	}

	float3 position = player->transform->GetWorldPosition();
	position += offset;

	gameObject->transform->SetWorldPosition(position);

	if (!gameObject->transform->GetLocalRotation().Equals(initialRot))
		gameObject->transform->SetLocalRotation(initialRot);
	
	if (gameManagerScript && gameManagerScript->cameraShake && playerScript != nullptr && playerScript->hitTimer.IsActive())
		CameraShake(shakeDuration, shakeMagnitude);

	if (gameManagerScript && gameManagerScript->cameraShake && ig12Script != nullptr && ig12Script->bombExploding == true)
		CameraShake(shakeDuration, shakeMagnitude);

	if (gameManagerScript && gameManagerScript->dialogManager->GetDialogState() == DialogState::NO_DIALOG)
	{
		nextPointProgress = 0;
		nextPoint = 0;
	}
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

bool CameraMovement::MoveCameraTo(GameObject* destination, float& progress)
{
	if (nextPoint >= destination->childs.size())
		return true;

	float3 destinationPos = destination->childs[nextPoint]->transform->GetWorldPosition();
	
	float3 nextPos = destinationPos.Lerp(gameObject->transform->GetWorldPosition(), 1.0f - progress);

	Quat destinationRot = destination->childs[nextPoint]->transform->GetWorldRotation();

	Quat nextRot = destinationRot.Lerp(gameObject->transform->GetWorldRotation(), 1.0f - progress);

	gameObject->transform->SetWorldPosition(nextPos);

	gameObject->transform->SetWorldRotation(nextRot);

	if (gameObject->transform->GetWorldPosition().Distance(destinationPos) < distanceToTransition)
	{
		nextPoint++;
		//nextPointProgress = 0;
		progress = 0;
	}

	return false;
}
