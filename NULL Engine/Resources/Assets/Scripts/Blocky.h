#pragma once
#include "Script.h"
#include "ScriptMacros.h"

class C_NavMeshAgent;

class SCRIPTS_API Blocky : public Script {
public :
	Blocky();
	~Blocky();
	void Awake() override;
	void Start() override;
	void PreUpdate()override;
	void Update() override;
	void PostUpdate()override;
	void CleanUp()override;

	C_NavMeshAgent* agent;

	GameObject* mando = nullptr;
};

SCRIPTS_FUNCTION Blocky* CreateBlocky() {
	Blocky* script = new Blocky();

	return script;
}