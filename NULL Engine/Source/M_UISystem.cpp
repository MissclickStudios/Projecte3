#include "Application.h"																					

#include "GameObject.h"

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