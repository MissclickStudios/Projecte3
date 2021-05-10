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

void Gate::Start()
{
	gameManager = App->scene->GetGameObjectByName(gameManagerName.c_str());
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
		if (!isLocked)
		{
			GameManager* gameManagerScript = (GameManager*)gameManager->GetScript("GameManager");
			if (gameManagerScript->playerScript)
				gameManagerScript->playerScript->hubCurrency += 20;
			gameManagerScript->GoNextRoom();
		}
	}
}

void Gate::Unlock()
{
	isLocked = false;
}

Gate* CreateGate()
{
	Gate* script = new Gate();
	INSPECTOR_STRING(script->gameManagerName);
	return script;
}
