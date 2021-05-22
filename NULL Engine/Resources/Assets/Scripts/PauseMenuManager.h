#pragma once
#include <string>
#include "Script.h"
#include "ScriptMacros.h"

class GameObject;
class C_UI_Button;
class C_UI_Checkbox;
class C_Canvas;

class SCRIPTS_API PauseMenuManager : public Script {
public:
	PauseMenuManager();
	~PauseMenuManager();

	void Start() override;

	void Update() override;

	std::string resumeButtonName = "ResumeButton";
	std::string optionsButtonName = "SettingsButton";
	std::string abandonRunButtonName = "AbandonButton";
	std::string mainMenuButtonName = "MenuButton";
	std::string mandoName = "Mandalorian";
	std::string gameManagerName = "Game Manager";
	std::string optionsMenuCanvasStr = "PauseSettingsCanvas";
	std::string optionsFullscreenStr = "FullscreenCheck";
	std::string optionsVsyncStr = "VsyncCheck";
	std::string backButtonStr = "BackButton";

private:
	C_Canvas* pauseMenuCanvas = nullptr;
	C_UI_Button* resumeButton = nullptr;
	C_UI_Button* optionsButton = nullptr;
	C_UI_Button* abandonRunButton = nullptr;
	C_UI_Button* mainMenuButton = nullptr;

	C_Canvas* optionsMenuCanvas = nullptr;
	C_UI_Checkbox* fullScreenCheck = nullptr;
	C_UI_Checkbox* vsyncCheck = nullptr;
	C_UI_Button* backButton = nullptr;

	GameObject* mando = nullptr;
	GameObject* gameManager = nullptr;

	bool canvasActive = false;
	bool onSettings = false;
};

SCRIPTS_FUNCTION PauseMenuManager* CreatePauseMenuManager() {
	PauseMenuManager* script = new PauseMenuManager();

	INSPECTOR_STRING(script->mandoName);
	INSPECTOR_STRING(script->gameManagerName);
	INSPECTOR_STRING(script->resumeButtonName);
	INSPECTOR_STRING(script->optionsButtonName);
	INSPECTOR_STRING(script->abandonRunButtonName);
	INSPECTOR_STRING(script->mainMenuButtonName);
	INSPECTOR_STRING(script->optionsMenuCanvasStr);
	INSPECTOR_STRING(script->optionsFullscreenStr);
	INSPECTOR_STRING(script->optionsVsyncStr);
	INSPECTOR_STRING(script->backButtonStr);
	return script;
}