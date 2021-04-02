#include "Application.h"
#include "Log.h"

#include "M_Scene.h"
#include "M_Camera3D.h"
#include "M_Input.h"

#include "GameObject.h"

#include "C_Canvas.h"
#include "C_Camera.h"
#include "C_Transform.h"
#include "C_UI_Text.h"
#include "C_UI_Button.h"
#include "C_UI_Image.h"

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
	if (isMainMenu && !isHoverDecorationAdded && !activeButtons.empty())
	{
		InitHoveredDecorations();
	}

	return UpdateStatus::CONTINUE;
}

UpdateStatus M_UISystem::Update(float dt)
{
	UpdateActiveButtons();
	CheckButtonStates();
	
	if (hoveredDecorationL != nullptr && hoveredDecorationR != nullptr)
		UpdateHoveredDecorations();

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

bool M_UISystem::CheckButtonStates()
{
	bool ret = false;

	if (App->input->GetKey(SDL_SCANCODE_G) == KeyState::KEY_DOWN || App->input->GetKey(SDL_SCANCODE_G) == KeyState::KEY_REPEAT)
	{
		hoveredButton->OnPressed();
	}

	else if (App->input->GetKey(SDL_SCANCODE_G) == KeyState::KEY_UP)
	{
		hoveredButton->OnReleased(); /*(?)*/
	}

	if (activeButtons.size() > 1)
	{
		bool prev = false;
		bool next = false;

		if (App->input->GetKey(SDL_SCANCODE_T) == KeyState::KEY_DOWN && !hoveredButton->IsPressed())
		{
			for (std::vector<C_UI_Button*>::reverse_iterator buttonIt = activeButtons.rbegin(); buttonIt != activeButtons.rend(); buttonIt++)
			{
				if ((*buttonIt)->IsActive())
				{
					if ((*buttonIt)->GetState() == UIButtonState::HOVERED)
					{
						(*buttonIt)->SetState(UIButtonState::IDLE);
						prev = true;
					}
					else if (prev)
					{
						(*buttonIt)->SetState(UIButtonState::HOVERED);
						hoveredButton = (*buttonIt);
						prev = false;
					}
				}
			}
			if (prev)
				hoveredButton->SetState(UIButtonState::HOVERED);
		}

		if (App->input->GetKey(SDL_SCANCODE_B) == KeyState::KEY_DOWN && !hoveredButton->IsPressed())
		{
			for (std::vector<C_UI_Button*>::iterator buttonIt = activeButtons.begin(); buttonIt != activeButtons.end(); buttonIt++)
			{
				if ((*buttonIt)->IsActive())
				{
					if ((*buttonIt)->GetState() == UIButtonState::HOVERED)
					{
						(*buttonIt)->SetState(UIButtonState::IDLE);
						next = true;
					}
					else if (next)
					{
						(*buttonIt)->SetState(UIButtonState::HOVERED);
						hoveredButton = (*buttonIt);
						next = false;
					}
				}
			}
			if(next)
				hoveredButton->SetState(UIButtonState::HOVERED);
		}
	}

	return ret;
}

void M_UISystem::UpdateActiveButtons()
{
	if (activeButtons.size() < 2)
		return;

	// Create a new list and empty the other one into this one
	std::vector<C_UI_Button*> newButtonsList;

	while (!activeButtons.empty())
	{
		float y = -999;
		for (std::vector<C_UI_Button*>::iterator buttonIt = activeButtons.begin(); buttonIt != activeButtons.end(); buttonIt++)
		{
			if ((*buttonIt)->GetRect().y > y)
			{
				y = (*buttonIt)->GetRect().y;
				buttonIterator = (*buttonIt);
			}
		}
		for (std::vector<C_UI_Button*>::iterator buttonIt2 = activeButtons.begin(); buttonIt2 != activeButtons.end(); buttonIt2++)
		{
			if ((*buttonIt2) == buttonIterator)
			{
				newButtonsList.push_back(*buttonIt2);
				activeButtons.erase(buttonIt2);
				break;
			}
		}
	}
	activeButtons = newButtonsList;
}

void M_UISystem::InitHoveredDecorations()
{
	GameObject* canvas = hoveredButton->GetOwner()->parent;
	GameObject* newGOL = nullptr;
	GameObject* newGOR = nullptr;

	newGOL = App->scene->CreateGameObject("Hovered Decoration L", canvas);
	hoveredDecorationL = (C_UI_Image*)newGOL->CreateComponent(ComponentType::UI_IMAGE);
	newGOL->CreateComponent(ComponentType::MATERIAL);
	Rect2D rectL = { (hoveredButton->GetRect().x - hoveredButton->GetRect().w / 2 - 0.02), hoveredButton->GetRect().y, 0.01,0.01 };
	hoveredDecorationL->SetRect(rectL);

	newGOR = App->scene->CreateGameObject("Hovered Decoration R", canvas);
	hoveredDecorationR = (C_UI_Image*)newGOR->CreateComponent(ComponentType::UI_IMAGE);
	newGOR->CreateComponent(ComponentType::MATERIAL);
	Rect2D rectR = { hoveredButton->GetRect().x + 0.01 + hoveredButton->GetRect().w / 2, hoveredButton->GetRect().y, 0.01,0.01 };
	hoveredDecorationR->SetRect(rectR);

	isHoverDecorationAdded = true;
}

void M_UISystem::UpdateHoveredDecorations()
{
	hoveredDecorationL->SetX(hoveredButton->GetRect().x - hoveredButton->GetRect().w / 2 - 0.02);
	hoveredDecorationR->SetX(hoveredButton->GetRect().x + 0.01 + hoveredButton->GetRect().w / 2);

	hoveredDecorationL->SetY(hoveredButton->GetRect().y);
	hoveredDecorationR->SetY(hoveredButton->GetRect().y);
}