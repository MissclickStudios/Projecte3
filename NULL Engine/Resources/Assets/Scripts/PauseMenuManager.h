#pragma once
#include <string>
#include "Script.h"
#include "ScriptMacros.h"

class GameObject;
class C_UI_Button;
class C_Canvas;

class SCRIPTS_API PauseMenuManager : public Script {
public:
	PauseMenuManager();
	~PauseMenuManager();

	void Start() override;

	void Update() override;

	std::string continueButtonName = "ContinueButton";
	std::string optionsButtonName = "OptionsButton";
	std::string abandonRunButtonName = "AbandonRunButton";
	std::string mainMenuButtonName = "MainMenuButton";
	std::string exitButtonName = "ExitButton";
	std::string pauseMenuCanvasName = "PauseMenuCanvas";
	std::string mandoName = "Mandalorian";
	std::string gameManagerName = "Game Manager";

private:
	C_UI_Button* continueButton = nullptr;
	C_UI_Button* optionsButton = nullptr;
	C_UI_Button* abandonRunButton = nullptr;
	C_UI_Button* mainMenuButton = nullptr;
	C_UI_Button* exitButton = nullptr;

	C_Canvas* pauseMenuCanvas = nullptr;

	GameObject* mando = nullptr;
	GameObject* gameManager = nullptr;
};

SCRIPTS_FUNCTION PauseMenuManager* CreatePauseMenuManager() {
	PauseMenuManager* script = new PauseMenuManager();

	INSPECTOR_STRING(script->mandoName);
	INSPECTOR_STRING(script->gameManagerName);
	INSPECTOR_STRING(script->pauseMenuCanvasName);
	INSPECTOR_STRING(script->continueButtonName);
	INSPECTOR_STRING(script->optionsButtonName);
	INSPECTOR_STRING(script->abandonRunButtonName);
	INSPECTOR_STRING(script->mainMenuButtonName);
	INSPECTOR_STRING(script->exitButtonName);
	return script;
}