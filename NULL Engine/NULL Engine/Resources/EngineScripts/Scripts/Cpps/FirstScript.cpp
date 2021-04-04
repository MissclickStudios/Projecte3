#include "FirstScript.h"
#include "Log.h"
#include "C_Transform.h"
#include "MathGeoLib/include/Math/float3.h"
#include "Application.h"
#include "M_Input.h"

FirstScript::FirstScript() : Script()
{
}

FirstScript::~FirstScript()
{
}

void FirstScript::Awake()
{
	LOG("Awake");
}

void FirstScript::Start()
{
	LOG("Start");
}

void FirstScript::PreUpdate()
{
	LOG("PreUpdate");
}

void FirstScript::Update()
{
	LOG("Update: First variable = %d", firstVariable);
	if (App->input->GetKey(SDL_SCANCODE_A) == KeyState::KEY_REPEAT) 
	{
		float3 pos = transform->GetLocalPosition();
		pos.x += 1;
		transform->SetLocalPosition(pos);
	}
	if (App->input->GetKey(SDL_SCANCODE_D) == KeyState::KEY_REPEAT)
	{
		float3 pos = transform->GetLocalPosition();
		pos.x -= 1;
		transform->SetLocalPosition(pos);
	}
}

void FirstScript::PostUpdate()
{
	LOG("PostUpdate");
}

void FirstScript::CleanUp()
{
	LOG("CleanUp");
}