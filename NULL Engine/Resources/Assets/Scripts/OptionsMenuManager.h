#pragma once
#include <string>
#include "Script.h"
#include "ScriptMacros.h"

class GameObject;
class C_UI_Button;

class SCRIPTS_API OptionsMenuManager : public Script {
public:
	OptionsMenuManager();
	~OptionsMenuManager();

	void Start() override;

	void Update() override;

	std::string buttonName = "Back";
private:
	C_UI_Button* backButton;

	void SetVsync(bool active);
	void SetFrameCap(int frameCap);
	void SetFullScreen(bool active);
	void SetVolume(int volume);
};

SCRIPTS_FUNCTION OptionsMenuManager* CreateOptionsMenuManager() {
	OptionsMenuManager* script = new OptionsMenuManager();
	INSPECTOR_STRING(script->buttonName);
	return script;
}