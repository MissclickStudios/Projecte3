#include "Application.h"

#include "MC_Time.h"
#include "Log.h"

#include "M_Input.h"
#include "M_Scene.h"

#include "C_Canvas.h"
#include "C_UI_Button.h"
#include "C_UI_Text.h"

#include "GameObject.h"
#include "DebugMenuManager.h"
#include "GameManager.h"

DebugMenuManager::DebugMenuManager()
{
}

DebugMenuManager::~DebugMenuManager()
{
}

void DebugMenuManager::Start()
{
	gameManager = App->scene->GetGameObjectByName(gameManagerName.c_str());
	mando = App->scene->GetGameObjectByName(mandoName.c_str());

	debugMenuCanvas = (C_Canvas*)App->scene->GetGameObjectByName(debugMenuCanvasName.c_str())->GetComponent<C_Canvas>();

	godMode = (C_UI_Button*)App->scene->GetGameObjectByName(godModeName.c_str())->GetComponent<C_UI_Button>();
	roomSkip = (C_UI_Button*)App->scene->GetGameObjectByName(roomSkipName.c_str())->GetComponent<C_UI_Button>();
	addHealth = (C_UI_Button*)App->scene->GetGameObjectByName(addHealthName.c_str())->GetComponent<C_UI_Button>();
	getCredits = (C_UI_Button*)App->scene->GetGameObjectByName(getCreditsName.c_str())->GetComponent<C_UI_Button>();
	getBeskar = (C_UI_Button*)App->scene->GetGameObjectByName(getBeskarName.c_str())->GetComponent<C_UI_Button>();
	spawnBlurgg = (C_UI_Button*)App->scene->GetGameObjectByName(spawnBlurggName.c_str())->GetComponent<C_UI_Button>();
	spawnTrooper = (C_UI_Button*)App->scene->GetGameObjectByName(spawnTrooperName.c_str())->GetComponent<C_UI_Button>();

	fpsText = (C_UI_Text*)App->scene->GetGameObjectByName(fpsTextName.c_str())->GetComponent<C_UI_Text>();
}

void DebugMenuManager::Update()
{
	if(debugMenuCanvas != nullptr)
		if (App->input->GetKey(SDL_SCANCODE_COMMA) == KeyState::KEY_DOWN)
		{
			if (debugMenuCanvas->IsActive())
			{
				debugMenuCanvas->SetIsActive(false);
			}
			else
			{
				debugMenuCanvas->SetIsActive(true);
			}
		}

	if (godMode != nullptr)
		if (godMode->GetState() == UIButtonState::RELEASED)
		{
			//Use Beskar Ingots
			//LOG("OMG YOU HAVE JUST SPENT %d Beskar Ingots!", beskarCost);
		}
	
	if (fpsText != nullptr)
	{
		std::string a = "FPS: "; 
		a += std::to_string(MC_Time::Game::GetFrameData().framesLastSecond).c_str();
		fpsText->SetText(a.c_str());
	}

	if (roomSkip != nullptr)
		if (roomSkip->GetState() == UIButtonState::RELEASED)
		{
			GameManager* gameManagerScript = (GameManager*)gameManager->GetScript("GameManager");
			gameManagerScript->GoNextRoom();
		}

	if (addHealth != nullptr)
		if (addHealth->GetState() == UIButtonState::RELEASED)
		{
			//Use Beskar Ingots
			//LOG("OMG YOU HAVE JUST SPENT %d Beskar Ingots!", beskarCost);
		}

	if (getCredits != nullptr)
		if (getCredits->GetState() == UIButtonState::RELEASED)
		{
			Player* gameManagerScript = (Player*)gameManager->GetScript("Player");
			gameManagerScript->currency += creditsToAdd;
		}

	if (getBeskar != nullptr)
		if (getBeskar->GetState() == UIButtonState::RELEASED)
		{
			Player* gameManagerScript = (Player*)gameManager->GetScript("Player");
			gameManagerScript->hubCurrency += beskarToAdd;
		}

	if (spawnBlurgg != nullptr)
		if (spawnBlurgg->GetState() == UIButtonState::RELEASED)
		{
			//Use Beskar Ingots
			//LOG("OMG YOU HAVE JUST SPENT %d Beskar Ingots!", beskarCost);
		}

	if (spawnTrooper != nullptr)
		if (spawnTrooper->GetState() == UIButtonState::RELEASED)
		{
			//Use Beskar Ingots
			//LOG("OMG YOU HAVE JUST SPENT %d Beskar Ingots!", beskarCost);
		}
}