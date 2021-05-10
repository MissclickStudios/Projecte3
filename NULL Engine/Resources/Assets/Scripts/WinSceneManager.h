#pragma once
#include <string>
#include "Script.h"
#include "ScriptMacros.h"

class GameObject;
class C_UI_Button;

class SCRIPTS_API WinSceneManager : public Script {
public:
	WinSceneManager();
	~WinSceneManager();
	//void Awake() override;
	void Start() override;
	//void PreUpdate()override;
	void Update() override;
	//void PostUpdate()override;
	//void CleanUp()override;
	GameObject* gameManagerObject = nullptr;
	GameObject* canvasObject = nullptr;
};

SCRIPTS_FUNCTION WinSceneManager* CreateWinSceneManager() {
	WinSceneManager* script = new WinSceneManager();
	INSPECTOR_GAMEOBJECT(script->gameManagerObject);
	INSPECTOR_GAMEOBJECT(script->canvasObject);
	return script;
}