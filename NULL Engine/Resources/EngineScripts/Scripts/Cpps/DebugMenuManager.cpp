#include "Application.h"

#include "Log.h"
#include "M_Scene.h"

#include "C_Canvas.h"
#include "C_UI_Button.h"
#include "C_UI_Text.h"

#include "GameObject.h"
#include "DebugMenuManager.h"

DebugMenuManager::DebugMenuManager()
{
}

DebugMenuManager::~DebugMenuManager()
{
}

void DebugMenuManager::Start()
{
	godMode = (C_UI_Button*)App->scene->GetGameObjectByName(godModeName.c_str())->GetComponent<C_UI_Button>();
	roomSkip = (C_UI_Button*)App->scene->GetGameObjectByName(roomSkipName.c_str())->GetComponent<C_UI_Button>();
	addHealth = (C_UI_Button*)App->scene->GetGameObjectByName(addHealthName.c_str())->GetComponent<C_UI_Button>();
	getCredits = (C_UI_Button*)App->scene->GetGameObjectByName(getCreditsName.c_str())->GetComponent<C_UI_Button>();
	getBeskar = (C_UI_Button*)App->scene->GetGameObjectByName(getBeskarName.c_str())->GetComponent<C_UI_Button>();
	spawnBlurgg = (C_UI_Button*)App->scene->GetGameObjectByName(spawnBlurggName.c_str())->GetComponent<C_UI_Button>();
	spawnTrooper = (C_UI_Button*)App->scene->GetGameObjectByName(spawnTrooperName.c_str())->GetComponent<C_UI_Button>();

	fpsText = (C_UI_Text*)App->scene->GetGameObjectByName(fpsTextName.c_str())->GetComponent<C_UI_Button>();
}

void DebugMenuManager::Update()
{
	if (godMode != nullptr)
		if (godMode->IsPressed())
		{
			//Use Beskar Ingots
			//LOG("OMG YOU HAVE JUST SPENT %d Beskar Ingots!", beskarCost);
		}
	
	if (fpsText != nullptr)
	{
		std::string FPSString = 
		fpsText->SetText();
	}

	if (roomSkip != nullptr)
		if (roomSkip->IsPressed())
		{
			//Use Beskar Ingots
			//LOG("OMG YOU HAVE JUST SPENT %d Beskar Ingots!", beskarCost);
		}

	if (addHealth != nullptr)
		if (addHealth->IsPressed())
		{
			//Use Beskar Ingots
			//LOG("OMG YOU HAVE JUST SPENT %d Beskar Ingots!", beskarCost);
		}

	if (getCredits != nullptr)
		if (getCredits->IsPressed())
		{
			//Use Beskar Ingots
			//LOG("OMG YOU HAVE JUST SPENT %d Beskar Ingots!", beskarCost);
		}

	if (getBeskar != nullptr)
		if (getBeskar->IsPressed())
		{
			//Use Beskar Ingots
			//LOG("OMG YOU HAVE JUST SPENT %d Beskar Ingots!", beskarCost);
		}

	if (spawnBlurgg != nullptr)
		if (spawnBlurgg->IsPressed())
		{
			//Use Beskar Ingots
			//LOG("OMG YOU HAVE JUST SPENT %d Beskar Ingots!", beskarCost);
		}

	if (spawnTrooper != nullptr)
		if (spawnTrooper->IsPressed())
		{
			//Use Beskar Ingots
			//LOG("OMG YOU HAVE JUST SPENT %d Beskar Ingots!", beskarCost);
		}
}