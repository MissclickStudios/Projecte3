
#include "Application.h"
#include "GameManager.h"
#include "GameObject.h"
#include "M_Scene.h"
#include "M_Input.h"

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
	{
		gameManager = (GameManager*)tmp->GetScript("GameManager");
	}
}

void CreditsSceneManager::Update()
{



	if (App->input->GetKey(SDL_SCANCODE_ESCAPE) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(0) == ButtonState::BUTTON_DOWN)
	{
		gameManager->ReturnHub();
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