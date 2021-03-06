#include "Application.h"
#include "Log.h"

#include "M_Scene.h"

#include "GameObject.h"
#include "C_Canvas.h"

#include "M_UISystem.h"

#include "MemoryManager.h"

#pragma comment (lib, "Source/Dependencies/FreeType/libx86/freetype.lib")

M_UISystem::M_UISystem(bool isActive) : Module("UISystem", isActive)
{
	error = FT_Init_FreeType(&libraryFT);
	if (error)
		LOG("Error initiallizing FreeType");
	
	// Create a new font
	error = FT_New_Face(libraryFT, "Assets/Fonts/arial.ttf", 0, &face);

	if (error == FT_Err_Unknown_File_Format) 
		LOG("Failed to open font: unknown font format");
	
	else if (error) 
		LOG("Failed to open font");
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