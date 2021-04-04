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
};

SCRIPTS_FUNCTION ola* Createola() {
	ola* script = new ola();
	INSPECTOR_INPUT_INT(script->firstVariable);
	return script;
}