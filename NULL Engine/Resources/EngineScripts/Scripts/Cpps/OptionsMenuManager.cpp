#include "Application.h"
#include "M_Scene.h"
#include "C_Canvas.h"
#include "C_UI_Button.h"
#include "LevelGenerator.h"
#include "GameObject.h"
#include "OptionsMenuManager.h"
#include "M_Window.h"
#include "M_Audio.h"
#include "M_Renderer3D.h"

OptionsMenuManager::OptionsMenuManager() : Script()
{
}

OptionsMenuManager::~OptionsMenuManager()
{
}

void OptionsMenuManager::Start()
{
	backButton = (C_UI_Button*)App->scene->GetGameObjectByName(buttonName.c_str())->GetComponent<C_UI_Button>();

}

void OptionsMenuManager::Update()
{
	if (backButton != nullptr)
	{
		if (backButton->IsPressed())
		{

		}
	}

}

void OptionsMenuManager::SetVsync(bool active)
{
	App->renderer->SetVsync(active);
}

void OptionsMenuManager::SetFrameCap(int frameCap)
{
	App->SetFrameCap(frameCap);
}

void OptionsMenuManager::SetFullScreen(bool active)
{
	App->window->SetFullscreen(active);
}

void OptionsMenuManager::SetVolume(int volume)
{

}
