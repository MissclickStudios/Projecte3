#include "Application.h"

#include "M_Scene.h"

#include "GameObject.h"
#include "C_Canvas.h"

#include "M_UISystem.h"

M_UISystem::M_UISystem(bool isActive) : Module("UISystem", isActive)
{
	
}

// Destructor
M_UISystem::~M_UISystem()
{

}

// Called before render is available
bool M_UISystem::Init(ParsonNode& config)
{
	bool ret = true;

	return ret;
}

// Called every draw update
UpdateStatus M_UISystem::PreUpdate(float dt)
{
	return UpdateStatus::CONTINUE;
}

UpdateStatus M_UISystem::Update(float dt)
{
	for (std::vector<GameObject*>::iterator it = App->scene->GetGameObjects()->begin(); it != App->scene->GetGameObjects()->end(); it++)
	{
		if ((*it)->GetComponent<C_Canvas>() != nullptr)
		{
			(*it)->GetComponent<C_Canvas>()->Update();
		}
	}
	return UpdateStatus::CONTINUE;
}

UpdateStatus M_UISystem::PostUpdate(float dt)
{
	return UpdateStatus::CONTINUE;
}

// Called before quitting
bool M_UISystem::CleanUp()
{
	return true;
}

bool M_UISystem::LoadConfiguration(ParsonNode& root)
{
	bool ret = true;

	return ret;
}

bool M_UISystem::SaveConfiguration(ParsonNode& root) const
{
	bool ret = true;

	return ret;
}