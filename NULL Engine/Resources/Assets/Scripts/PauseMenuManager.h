#pragma once
#include <string>
#include "Script.h"
#include "ScriptMacros.h"

class GameObject;
class C_UI_Button;
class C_UI_Checkbox;
class C_UI_Slider;
class C_Canvas;
class HUDManager;
class GameManager;

class SCRIPTS_API PauseMenuManager : public Script {
public:
	PauseMenuManager();
	~PauseMenuManager();

	void Start() override;

	void Update() override;

	std::string mandoName = "Mandalorian";
	std::string gameManagerName = "Game Manager";
	std::string resumeButtonName = "ResumeButton";
	std::string optionsButtonName = "SettingsButton";
	std::string abandonRunButtonName = "AbandonButton";
	std::string mainMenuButtonName = "MenuButton";
	std::string optionsMenuCanvasStr = "PauseSettingsCanvas";
	std::string optionsFullscreenStr = "FullscreenCheck";
	std::string optionsVsyncStr = "VsyncCheck";
	std::string optionsShakeStr = "ShakeCheck";
	std::string backButtonStr = "BackButton";
	std::string hudCanvasStr = "HUD";
	std::string musicSliderStr = "MusicSlider";
	std::string fxSliderStr = "EffectsSlider";

private:
	C_Canvas* hudCanvas = nullptr;
	HUDManager* hudScript = nullptr;
	C_Canvas* pauseMenuCanvas = nullptr;
	C_UI_Button* resumeButton = nullptr;
	C_UI_Button* optionsButton = nullptr;
	C_UI_Button* abandonRunButton = nullptr;
	C_UI_Button* mainMenuButton = nullptr;

	C_Canvas* optionsMenuCanvas = nullptr;
	C_UI_Slider* musicSlider = nullptr;
	C_UI_Slider* fxSlider = nullptr;
	C_UI_Checkbox* fullScreenCheck = nullptr;
	C_UI_Checkbox* vsyncCheck = nullptr;
	C_UI_Checkbox* shakeCheck = nullptr;
	C_UI_Button* backButton = nullptr;

	GameObject* mando = nullptr;
	GameManager* gameManager = nullptr;

	bool canvasActive = false;
	bool onSettings = false;
};

SCRIPTS_FUNCTION PauseMenuManager* CreatePauseMenuManager() {
	PauseMenuManager* script = new PauseMenuManager();

	INSPECTOR_STRING(script->mandoName);
	INSPECTOR_STRING(script->gameManagerName);
	INSPECTOR_STRING(script->resumeButtonName);
	INSPECTOR_STRING(script->optionsButtonName);
	//INSPECTOR_STRING(script->abandonRunButtonName); (momo leak en debug !!??)
	INSPECTOR_STRING(script->mainMenuButtonName);
	INSPECTOR_STRING(script->optionsMenuCanvasStr);
	INSPECTOR_STRING(script->optionsFullscreenStr);
	INSPECTOR_STRING(script->optionsVsyncStr);
	INSPECTOR_STRING(script->backButtonStr);
	INSPECTOR_STRING(script->hudCanvasStr);
	INSPECTOR_STRING(script->musicSliderStr);
	INSPECTOR_STRING(script->fxSliderStr);
	INSPECTOR_STRING(script->optionsShakeStr);

	return script;
}