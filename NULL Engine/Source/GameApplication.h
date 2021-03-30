#ifndef __GAMEAPPLICATION_H__
#define __GAMEAPPLICATION_H__

#include "Application.h"

class M_GameManager;
class M_ScriptManager;

class GameApplication : public /*EngineNamespace::*/Application {

public:

	GameApplication();
	~GameApplication();

	//void SaveForBuild(const Broken::json& reference, const char* path) const;
	//void GetDefaultGameConfig(Broken::json& config) const;

public:
	class M_GameManager* manager;
	class M_ScriptManager* scriptManager;

};

extern GameApplication* GameApp;

#endif