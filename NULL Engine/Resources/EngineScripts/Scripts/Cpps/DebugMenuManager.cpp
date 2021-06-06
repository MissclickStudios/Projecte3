#include "Application.h"

#include "MC_Time.h"
#include "Log.h"
#include "Prefab.h"

#include "M_Input.h"
#include "M_Scene.h"
#include "M_UISystem.h"

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
	gameManager = App->scene->GetGameObjectByName("Game Manager"); //This is hardcoded. Too bad!
	mando = App->scene->GetGameObjectByName(mandoName.c_str());

	debugMenuCanvas = (C_Canvas*)gameObject->GetComponent<C_Canvas>();

	GameObject* a = App->scene->GetGameObjectByName(godModeName.c_str());
	if (a != nullptr)
		godMode = (C_UI_Button*)a->GetComponent<C_UI_Button>();

	a = App->scene->GetGameObjectByName(roomSkipName.c_str());
	if (a != nullptr)
		roomSkip = (C_UI_Button*)a->GetComponent<C_UI_Button>();

	a = App->scene->GetGameObjectByName(addHealthName.c_str());
	if (a != nullptr)
		addHealth = (C_UI_Button*)a->GetComponent<C_UI_Button>();

	a = App->scene->GetGameObjectByName(getCreditsName.c_str());
	if (a != nullptr)
		getCredits = (C_UI_Button*)a->GetComponent<C_UI_Button>();

	a = App->scene->GetGameObjectByName(getBeskarName.c_str());
	if (a != nullptr)
		getBeskar = (C_UI_Button*)a->GetComponent<C_UI_Button>();

	a = App->scene->GetGameObjectByName(spawnBlurggName.c_str());
	if (a != nullptr)
		spawnBlurgg = (C_UI_Button*)a->GetComponent<C_UI_Button>();

	a = App->scene->GetGameObjectByName(spawnTrooperName.c_str());
	if (a != nullptr)
		spawnTrooper = (C_UI_Button*)a->GetComponent<C_UI_Button>();

	a = App->scene->GetGameObjectByName(fpsTextName.c_str());
	if (a != nullptr)
		fpsText = (C_UI_Text*)a->GetComponent<C_UI_Text>();
}

void DebugMenuManager::Update()
{
	if(debugMenuCanvas != nullptr)
		if (App->input->GetKey(SDL_SCANCODE_COMMA) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(4) == ButtonState::BUTTON_DOWN)
		{
			if (canvasActive)
			{
				App->uiSystem->RemoveActiveCanvas(debugMenuCanvas);
				canvasActive = false;
			}
			else
			{
				App->uiSystem->PushCanvas(debugMenuCanvas);
				canvasActive = true;
			}
		}

	if (godMode != nullptr)
		if (godMode->GetState() == UIButtonState::RELEASED)
		{
			if (mando != nullptr)
			{
				Player* playerScript = (Player*)mando->GetScript("Player");

				if (playerScript->GetGodMode())
				{
					playerScript->SetGodMode(false);
				}
				else
				{
					playerScript->SetGodMode(true);
				}
			}
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
			if (gameManager != nullptr)
			{
				GameManager* gameManagerScript = (GameManager*)gameManager->GetScript("GameManager");
				gameManagerScript->GoNextRoom();
			}
		}

	if (addHealth != nullptr)
		if (addHealth->GetState() == UIButtonState::RELEASED)
		{
			if (mando != nullptr)
			{
				Player* gameManagerScript = (Player*)mando->GetScript("Player");
				gameManagerScript->GiveHeal(healthHealed);
			}
		}

	if (getCredits != nullptr)
		if (getCredits->GetState() == UIButtonState::RELEASED)
		{
			if (mando != nullptr)
			{
				Player* gameManagerScript = (Player*)mando->GetScript("Player");
				gameManagerScript->GiveCredits(creditsToAdd);
			}
		}

	if (getBeskar != nullptr)
		if (getBeskar->GetState() == UIButtonState::RELEASED)
		{
			if (mando != nullptr)
			{
				Player* gameManagerScript = (Player*)mando->GetScript("Player");
				gameManagerScript->GiveBeskar(beskarToAdd);
			}
		}

	if (spawnBlurgg != nullptr)
		if (spawnBlurgg->GetState() == UIButtonState::RELEASED)
		{
			App->scene->InstantiatePrefab(blurgg.uid, nullptr, float3(0, 0, 0), Quat::identity);
		}

	if (spawnTrooper != nullptr)
		if (spawnTrooper->GetState() == UIButtonState::RELEASED)
		{
			App->scene->InstantiatePrefab(trooper.uid, nullptr, float3(0, 0, 0), Quat::identity);
		}
}