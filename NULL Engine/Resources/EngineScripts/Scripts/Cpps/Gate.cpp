#include "Application.h"
#include "M_Scene.h"
#include "Log.h"
#include "GameManager.h"
#include "GameObject.h"

#include "Gate.h"

Gate::Gate() : Script()
{
}

Gate::~Gate()
{
}

void Gate::Update()
{
}

void Gate::CleanUp()
{
}

void Gate::OnCollisionEnter(GameObject* object)
{
	//TODO: GameManager
	if (gameManager != nullptr) 
	{
		GameManager* gameManagerScript = (GameManager*)gameManager->GetScript("GameManager");
		gameManagerScript->level.GoNextRoom();
	}
}
