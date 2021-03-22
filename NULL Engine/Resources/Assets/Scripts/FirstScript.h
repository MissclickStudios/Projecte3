#pragma once
#include "..\..\..\Source\Script.h"
#include "..\..\EngineScripts\Helpers\ScriptMacros.h"

class SCRIPTS_API FirstScript : public Script {
public :
	FirstScript();
	~FirstScript();
	void Update() override;
};

SCRIPTS_FUNCTION FirstScript* CreateFirstScript() {
	FirstScript* script = new FirstScript();
	return script;
}