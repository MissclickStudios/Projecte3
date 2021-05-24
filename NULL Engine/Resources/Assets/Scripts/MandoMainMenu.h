#pragma once

#include "Script.h"
#include "ScriptMacros.h"

#include <vector>
#include <string>

class GameManager;
class C_Animator;

class SCRIPTS_API MandoMainMenu : public Script
{
public:

	MandoMainMenu();
	~MandoMainMenu();

	void Start() override;
	void Update() override;
	void CleanUp() override;

private:

	C_Animator* mandoAnimator = nullptr;

	bool animating = false;
};

SCRIPTS_FUNCTION MandoMainMenu* CreateMandoMainMenu();