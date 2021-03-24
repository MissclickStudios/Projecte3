#include "FirstScript.h"
#include "Log.h"
#include "C_Transform.h"
#include "MathGeoLib/include/Math/float3.h"

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
	float3 pos = transform->GetLocalPosition();
	pos.x += 1;
	transform->SetLocalPosition(pos);
}

void FirstScript::PostUpdate()
{
	LOG("PostUpdate");
}

void FirstScript::CleanUp()
{
	LOG("CleanUp");
}
