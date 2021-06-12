
#include "MC_Time.h"
#include "Application.h"
#include "GameManager.h"
#include "GameObject.h"
#include "M_Scene.h"
#include "M_Input.h"

#include "C_Transform.h"

#include "CreditsSceneManager.h"


CreditsSceneManager::CreditsSceneManager()
{

}

CreditsSceneManager::~CreditsSceneManager()
{

}

void CreditsSceneManager::Start()
{
	GameObject* tmp = App->scene->GetGameObjectByName("Game Manager");
	if (tmp != nullptr)
		gameManager = (GameManager*)tmp->GetScript("GameManager");

	creditsPlane = App->scene->GetGameObjectByName("CreditsPlane");
}

void CreditsSceneManager::Update()
{
	if(creditsPlane != nullptr)
		creditsPlane->transform->SetLocalPosition(creditsPlane->transform->GetLocalPosition() + float3(0 * moveSpeed * MC_Time::Game::GetDT(), 0.3 * moveSpeed * MC_Time::Game::GetDT(), -1 * moveSpeed * MC_Time::Game::GetDT()));


	if (App->input->GetKey(SDL_SCANCODE_RETURN) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(0) == ButtonState::BUTTON_DOWN)
	{
		if (!App->scene->creditsMainMenu)
		{
			gameManager->ReturnHub();
		}
		else
		{
			gameManager->ReturnToMainMenu();
			App->scene->creditsMainMenu = false;
		}
	}
}

void CreditsSceneManager::CleanUp()
{

}

CreditsSceneManager* CreateCreditsSceneManager()
{
	CreditsSceneManager* script = new CreditsSceneManager();

	return script;
}