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
}

void Gate::CleanUp()
{
}

void Gate::OnCollisionEnter(GameObject* object)
{
	if (!App->scene->nextScene)
		App->scene->NextRoom();
}
