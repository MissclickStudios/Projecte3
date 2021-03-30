#include "Application.h"
#include "M_Scene.h"
#include "Log.h"

#include "Gate.h"

Gate::Gate() : Script()
{
}

Gate::~Gate()
{
}

void Gate::Update()
{
	LOG("Update");
}

void Gate::CleanUp()
{
	LOG("CleanUp");
}

void Gate::OnCollisionEnter()
{
	if (!App->scene->nextScene)
		App->scene->NextRoom();
}
