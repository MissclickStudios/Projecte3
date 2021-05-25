#pragma once
#include <string>
#include "Script.h"
#include "ScriptMacros.h"

class GameObject;
class C_UI_Button;

class SCRIPTS_API WinLoseSceneManager : public Script {
public:
	WinLoseSceneManager();
	~WinLoseSceneManager();
	//void Awake() override;
	void Start() override;
	//void PreUpdate()override;
	void Update() override;
	//void PostUpdate()override;
	//void CleanUp()override;
	GameObject* gameManagerObject = nullptr;
	GameObject* canvasObject = nullptr;
};

SCRIPTS_FUNCTION WinLoseSceneManager* CreateWinLoseSceneManager() {
	WinLoseSceneManager* script = new WinLoseSceneManager();
	INSPECTOR_GAMEOBJECT(script->gameManagerObject);
	INSPECTOR_GAMEOBJECT(script->canvasObject);
	return script;
}