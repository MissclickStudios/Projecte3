#pragma once
#include "Script.h"
#include "ScriptMacros.h"

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
	int firstVariable = 4;
};

SCRIPTS_FUNCTION FirstScript* CreateFirstScript() {
	FirstScript* script = new FirstScript();
	return script;
}