#include "VariableTypedefs.h"
#include "Log.h"

#include "Application.h"

#include "M_Scene.h"

#include "M_Scene.h"

#include "GameObject.h"
#include "C_Transform.h"

#include "CameraMovement.h"

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
}

void CameraMovement::Update()
{

	if (player == nullptr)
		return;

	float3 position = player->transform->GetWorldPosition();


	position += offset;
	gameObject->transform->SetWorldPosition(position);
	
	
	if (App->input->GetKey(SDL_SCANCODE_M) == KeyState::KEY_REPEAT) CameraShake(0.15f, 1.0f);
}

void CameraMovement::CleanUp()
{
	LOG("CleanUp");
}

void CameraMovement::CameraShake(float duration, float shakeMagnitude)
{
	float3 originalPos;
	float x;
	float y;
	
	originalPos = gameObject->transform->GetWorldPosition();

	if (timeElapsed < duration)
	{
		x = Random::LCG::GetBoundedRandomFloat(-1.0f, 1.0f) * shakeMagnitude;
		y = Random::LCG::GetBoundedRandomFloat(-1.0f, 1.0f) * shakeMagnitude;

		timeElapsed += MC_Time::Game::GetDT();
		
		gameObject->transform->SetWorldPosition(float3(originalPos.x + x, originalPos.y + y, originalPos.z));

	}
	else
	{
		gameObject->transform->SetWorldPosition(originalPos);
		timeElapsed = 0;
	}

}
