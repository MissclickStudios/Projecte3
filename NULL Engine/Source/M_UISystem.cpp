#include "Application.h"
#include "Log.h"

#include "M_Scene.h"
#include "M_Camera3D.h"

#include "GameObject.h"

#include "C_Canvas.h"
#include "C_Camera.h"
#include "C_Transform.h"
#include "C_UI_Text.h"

#include "M_UISystem.h"

#include "MemoryManager.h"

#pragma comment( lib, "Source/Dependencies/FreeType/libx86/freetype.lib" )

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

	// Initilaizing FreeType
	bool error = FT_Init_FreeType(&library);
	if (error)
	{
		LOG("An error ocurred initializing FreeType Library");
	}

	// Creating a face from the .ttf file
	error = FT_New_Face(library, "Assets/Fonts/arial.ttf",	0, &standardFace);
	if (error == FT_Err_Unknown_File_Format)
	{
		LOG("The file format for font is not supported");
	}
	else if (error)
	{
		LOG("An error ocurred creating the face");
	}

	float w = App->camera->GetCurrentCamera()->GetFrustum().NearPlaneWidth();
	float h = App->camera->GetCurrentCamera()->GetFrustum().NearPlaneHeight();

	//error = FT_Set_Char_Size(
	//	standardFace,	/* handle to face object           */
	//	0,				/* char_width in 1/64th of points  */
	//	16 * 64,		/* char_height in 1/64th of points */
	//	300,			/* horizontal device resolution    */
	//	300);			/* vertical device resolution      */
	// Setting default size for the face
	error = FT_Set_Pixel_Sizes(standardFace, 0, 16);
	if (error)
	{
		LOG("An error ocurred on trying to resize the font face");
	}

	// Getting the glyph index
	glyphIndex = FT_Get_Char_Index(standardFace, charcode);
	
	// Loading the glyph image
	error = FT_Load_Glyph(standardFace, glyphIndex, FT_LOAD_DEFAULT);
	if (error)
	{
		LOG("An error ocurred loading the glyph");
	}
	
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