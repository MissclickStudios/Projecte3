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

void CameraMovement::Start()
{
	player = App->scene->GetGameObjectByName(playerName.c_str());
	playerScript = (Player*)player->GetScript("Player");
}

void CameraMovement::Update()
{

	if (player == nullptr)
		return;

	float3 position = player->transform->GetWorldPosition();
	position += offset;
	gameObject->transform->SetWorldPosition(position);
	
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
