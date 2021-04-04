#pragma once
#include "Script.h"
#include "ScriptMacros.h"
#include "Log.h"

class SCRIPTS_API ola : public Script {
public :
	ola();
	~ola();
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
};

SCRIPTS_FUNCTION ola* Createola() {
	ola* script = new ola();
	INSPECTOR_INPUT_INT(script->firstVariable);
	INSPECTOR_INPUT_INT(script->inputInt);
	INSPECTOR_DRAGABLE_INT(script->dragableInt);
	INSPECTOR_SLIDER_INT(script->sliderInt, 0, 100);
	return script;
}