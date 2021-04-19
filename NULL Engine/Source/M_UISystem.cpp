#include "Application.h"
#include "Log.h"
#include "Profiler.h"

#include "M_ResourceManager.h"
#include "M_Scene.h"
#include "M_Camera3D.h"

#include "GameObject.h"

#include "R_Texture.h"

#include "C_Canvas.h"
#include "C_Camera.h"
#include "C_Transform.h"
#include "C_UI_Text.h"
#include "C_UI_Button.h"
#include "C_UI_Image.h"

#include "M_UISystem.h"

#include "MemoryManager.h"

#include "JSONParser.h"

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

	//// Initilaizing FreeType
	//bool error = FT_Init_FreeType(&library);
	//if (error)
	//{
	//	LOG("An error ocurred initializing FreeType Library");
	//}

	//// Creating a face from the .ttf file
	//error = FT_New_Face(library, "Assets/Fonts/arial.ttf",	0, &standardFace);
	//if (error == FT_Err_Unknown_File_Format)
	//{
	//	LOG("The file format for font is not supported");
	//}
	//else if (error)
	//{
	//	LOG("An error ocurred creating the face");
	//}

	/*float w = App->camera->GetCurrentCamera()->GetFrustum().NearPlaneWidth();
	float h = App->camera->GetCurrentCamera()->GetFrustum().NearPlaneHeight();*/

	//error = FT_Set_Char_Size(
	//	standardFace,	/* handle to face object           */
	//	0,				/* char_width in 1/64th of points  */
	//	16 * 64,		/* char_height in 1/64th of points */
	//	300,			/* horizontal device resolution    */
	//	300);			/* vertical device resolution      */
	// Setting default size for the face
	//error = FT_Set_Pixel_Sizes(standardFace, 0, 16);
	//if (error)
	//{
	//	LOG("An error ocurred on trying to resize the font face");
	//}

	//// Getting the glyph index
	//glyphIndex = FT_Get_Char_Index(standardFace, charcode);
	//
	//// Loading the glyph image
	//error = FT_Load_Glyph(standardFace, glyphIndex, FT_LOAD_DEFAULT);
	//if (error)
	//{
	//	LOG("An error ocurred loading the glyph");
	//}
	
	

	return ret;
}

bool M_UISystem::Start()
{
	buttonHoverDecor = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/UI/MainMenu/ButtonPart1.png");
	return true;
}

// Called every draw update
UpdateStatus M_UISystem::PreUpdate(float dt)
{
	UpdateInputCanvas();

	if (inputCanvas != nullptr)
	{
		if (!inputCanvas->activeButtons.empty())
			InitHoveredDecorations();
	}

	return UpdateStatus::CONTINUE;
}

UpdateStatus M_UISystem::Update(float dt)
{
	OPTICK_CATEGORY("M_UISystem Update", Optick::Category::Module);

	for (std::vector<C_Canvas*>::const_iterator it = canvasList.cbegin(); it != canvasList.cend(); it++)
	{
		(*it)->UpdateActiveButtons();
	}

	if (inputCanvas != nullptr)
		inputCanvas->CheckButtonStates();

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
	for (std::vector<C_Canvas*>::iterator canvasIt = canvasList.begin(); canvasIt != canvasList.end(); canvasIt++)
	{
		(*canvasIt)->activeButtons.clear();
	}
	canvasList.clear();

	inputCanvas = nullptr;

	hoveredDecorationL = nullptr;
	hoveredDecorationR = nullptr;

	App->resourceManager->FreeResource(buttonHoverDecor->GetUID());

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

void M_UISystem::UpdateInputCanvas()
{
	inputCanvas = nullptr;
	if (canvasList.size() == 1)
	{
		inputCanvas = (*canvasList.begin());
		return;
	}

	for (std::vector<C_Canvas*>::const_reverse_iterator it = canvasList.crbegin(); it != canvasList.crend(); it++)
	{
		if ((*it)->IsActive())
		{
			// The last one to enter will be the one receiving input unless it is not active
			inputCanvas = (*it);
			return;
		}
	}
}

void M_UISystem::DeleteCanvas(C_Canvas* canvas)
{
	for (std::vector<C_Canvas*>::const_iterator it = canvasList.cbegin(); it != canvasList.cend(); it++)
	{
		if ((*it) == canvas)
		{
			if ((*it) == inputCanvas)
			{
				isHoverDecorationAdded = false;
				hoveredDecorationL = nullptr;
				hoveredDecorationR = nullptr;
			}

			canvasList.erase(it);
			UpdateInputCanvas();

			return;
		}
	}
}

void M_UISystem::DeleteActiveButton(C_UI_Button* button)
{
	for (std::vector<C_Canvas*>::const_iterator canvasIt = canvasList.cbegin(); canvasIt != canvasList.cend(); canvasIt++)
	{
		for (std::vector<C_UI_Button*>::const_iterator it = (*canvasIt)->activeButtons.begin(); it != (*canvasIt)->activeButtons.end(); it++)
		{
			if ((*it) == button)
			{
				(*canvasIt)->activeButtons.erase(it);
				return;
			}
		}
	}
}

void M_UISystem::InitHoveredDecorations()
{
	if (hoveredDecorationL != nullptr || hoveredDecorationR != nullptr || canvasList.empty())
		return;

	GameObject* canvas = inputCanvas->GetOwner();
	GameObject* newGOL = nullptr;
	GameObject* newGOR = nullptr;

	newGOL = App->scene->CreateGameObject("Hovered Decoration L", canvas);
	hoveredDecorationL = (C_UI_Image*)newGOL->CreateComponent(ComponentType::UI_IMAGE);
	newGOL->CreateComponent(ComponentType::MATERIAL);
	Rect2D rectL = { (inputCanvas->selectedButton->GetRect().x - inputCanvas->selectedButton->GetRect().w / 2 - 0.04), inputCanvas->selectedButton->GetRect().y, 0.05,0.1 };
	hoveredDecorationL->SetRect(rectL);

	newGOR = App->scene->CreateGameObject("Hovered Decoration R", canvas);
	hoveredDecorationR = (C_UI_Image*)newGOR->CreateComponent(ComponentType::UI_IMAGE);
	newGOR->CreateComponent(ComponentType::MATERIAL);
	Rect2D rectR = { inputCanvas->selectedButton->GetRect().x + 0.02 + inputCanvas->selectedButton->GetRect().w / 2, inputCanvas->selectedButton->GetRect().y, 0.05,0.1 };
	hoveredDecorationR->SetRect(rectR);

	isHoverDecorationAdded = true;
}

void M_UISystem::UpdateHoveredDecorations()
{
	if (inputCanvas != nullptr)
	{
		if (!isHoverDecorationAdded)
		{
			InitHoveredDecorations();
		}
		hoveredDecorationL->GetOwner()->parent = inputCanvas->GetOwner();
		hoveredDecorationR->GetOwner()->parent = inputCanvas->GetOwner();

		if (inputCanvas->selectedButton != nullptr)
		{
			hoveredDecorationL->SetX(inputCanvas->selectedButton->GetRect().x - inputCanvas->selectedButton->GetRect().w / 2 - 0.1);
			hoveredDecorationR->SetX(inputCanvas->selectedButton->GetRect().x + inputCanvas->selectedButton->GetRect().w / 2 + 0.15);

			hoveredDecorationL->SetY((inputCanvas->selectedButton->GetRect().y + 0.011) * 3.5);
			hoveredDecorationR->SetY((inputCanvas->selectedButton->GetRect().y + 0.011) * 3.5);
		}
	}
}

void M_UISystem::AddNewCanvas(C_Canvas* canvas)
{
	if (inputCanvas != nullptr)
	{
		for (std::vector<C_UI_Button*>::const_iterator it = inputCanvas->activeButtons.cbegin(); it != inputCanvas->activeButtons.cend(); it++)
		{
			(*it)->SetState(UIButtonState::IDLE);
		}
		inputCanvas->hoveredButton = nullptr;
		inputCanvas->selectedButton = nullptr;
	}

	inputCanvas = canvas; // New one will allways be the input receiving canvas
	canvasList.push_back(canvas);
}