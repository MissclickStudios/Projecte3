#pragma once
#include "Script.h"
#include "ScriptMacros.h"
#include "Log.h"

class SCRIPTS_API FirstScript : public Script {
public :
	FirstScript();
	~FirstScript();
	void Awake() override;
	void Start() override;
	void PreUpdate()override;
	void Update() override;
	void PostUpdate()override;
	void CleanUp()override;
	int firstVariable = 1;
	int inputInt = 1;
	int dragableInt = 2;
	int sliderInt = 3;
	bool checkBoxBool = false;
	float inputFloat = 1.1f;
	float dragFloat = 2.2f;
	float sliderFloat = 3.3f;
};

SCRIPTS_FUNCTION FirstScript* CreateFirstScript() {
	FirstScript* script = new FirstScript();
	INSPECTOR_INPUT_INT(script->firstVariable);
	INSPECTOR_INPUT_INT(script->inputInt);
	INSPECTOR_DRAGABLE_INT(script->dragableInt);
	INSPECTOR_SLIDER_INT(script->sliderInt, 0, 100);
	INSPECTOR_CHECKBOX_BOOL(script->checkBoxBool);
	INSPECTOR_INPUT_FLOAT(script->inputFloat);
	INSPECTOR_DRAGABLE_FLOAT(script->dragFloat);
	INSPECTOR_SLIDER_FLOAT(script->sliderFloat,0.0,5.5f);
	return script;
}