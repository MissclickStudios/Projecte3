#include "Application.h"
#include "M_Scene.h"
#include "C_Canvas.h"
#include "C_UI_Button.h"
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
	GameObject* a = App->scene->GetGameObjectByName(buttonName.c_str());
	if(a != nullptr)
		backButton = (C_UI_Button*)a->GetComponent<C_UI_Button>();

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
