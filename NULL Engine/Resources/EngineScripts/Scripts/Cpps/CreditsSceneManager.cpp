
#include "Application.h"
#include "GameManager.h"
#include "GameObject.h"
#include "M_Scene.h"

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
}

void CreditsSceneManager::Update()
{

}

void CreditsSceneManager::CleanUp()
{

}

CreditsSceneManager* CreateCreditsSceneManager()
{
	CreditsSceneManager* script = new CreditsSceneManager();

	return script;
}