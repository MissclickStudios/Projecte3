#pragma once
#include "Script.h"
#include "ScriptMacros.h"

class SCRIPTS_API Prop : public Script {
public:
	Prop();
	~Prop();

	void Update() override;
	void CleanUp()override;

	void OnCollisionEnter();

private:

	bool dead = false;
};

SCRIPTS_FUNCTION Prop* CreateProp() {
	Prop* script = new Prop();
	return script;
}