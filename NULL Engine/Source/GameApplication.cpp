#include "GameApplication.h"

#include "Module.h"
#include "M_Window.h"
#include "M_Input.h"
#include "M_Scene.h"
#include "M_Renderer3D.h"
#include "M_Camera3D.h"
#include "M_FileSystem.h"
#include "M_ResourceManager.h"
#include "M_Audio.h"
#include "M_Physics.h"
#include "M_UISystem.h"
#include "M_ScriptManager.h"
#include "M_GameManager.h"

#include "EngineMain.h"

//#include "MemoryManager.h" //TODO: coment on build configuration

GameApplication* GameApp = NULL;

Application* CreateApplication()
{
	return new GameApplication();
}

GameApplication::GameApplication()
{
	GameApp = this;

	manager = new M_GameManager();
	scriptManager = new M_ScriptManager();

	//12 = num of modules to pushback
	//if you create or remove 1 module change the 12 accordingly
	modules.reserve(12);

	// Main Modules
	AddModule(window);
	AddModule(camera);
	AddModule(input);
	AddModule(fileSystem);
	AddModule(resourceManager);

	// Scenes
	AddModule(scriptManager);
	AddModule(manager);
	AddModule(audio);
	AddModule(physics);
	AddModule(uiSystem);
	AddModule(scene);

	// Renderer last!
	AddModule(renderer);

	renderer->AddPostSceneRenderModule(manager);
}

GameApplication::~GameApplication()
{
}