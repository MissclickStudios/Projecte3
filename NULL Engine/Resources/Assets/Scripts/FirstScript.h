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
	int firstVariable = 6;
};

SCRIPTS_FUNCTION ola* Createola() {
	ola* script = new ola();
	return script;
}

class SCRIPTS_API f : public Script {
public:
	f() {}
	~f() {}
	void Update() override { LOG("f"); LOG("el meta !!!!!!!!!! f"); }
	int firstVariable = 6;
};

SCRIPTS_FUNCTION f* Createf() {
	f* script = new f();
	return script;
}