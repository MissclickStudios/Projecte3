#pragma once
#include "Script.h"
#include "ScriptMacros.h"

class SCRIPTS_API Gate : public Script {
public:
	Gate();
	~Gate();

	void Update() override;
	void CleanUp()override;

	void OnCollisionEnter();
};

SCRIPTS_FUNCTION Gate* CreateGate() {
	Gate* script = new Gate();
	return script;
}