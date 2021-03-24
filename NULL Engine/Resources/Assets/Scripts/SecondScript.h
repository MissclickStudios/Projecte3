#pragma once
//#include "..\..\..\Source\Script.h"
#include "ScriptMacros.h"

class SCRIPTS_API SecondScript {

public:
	SecondScript();
	~SecondScript();
	int i = 3;
};

SCRIPTS_FUNCTION SecondScript * CreateSecondScript() {
	SecondScript* script = new SecondScript();
	return script;
}

SCRIPTS_FUNCTION void DeleteSecondScript(void* ptr) {
	delete (SecondScript*)ptr;
	ptr = nullptr;
}