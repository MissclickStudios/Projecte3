#include "FirstScript.h"
#include "Log.h"
#include "C_Transform.h"
#include "MathGeoLib/include/Math/float3.h"
#include "Application.h"
#include "M_Input.h"

ola::ola() : Script()
{
}

ola::~ola()
{
}

void ola::Awake()
{
	LOG("Awake");
}

void ola::Start()
{
	LOG("Start");
}

void ola::PreUpdate()
{
	LOG("PreUpdate");
}

void ola::Update()
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

void ola::PostUpdate()
{
	LOG("PostUpdate");
}

void ola::CleanUp()
{
	LOG("CleanUp");
}
