#include "EngineApplication.h"

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
#include "M_EngineScriptManager.h"
#include "M_Editor.h"
#include "M_Detour.h"

#include "EngineMain.h"

#include "MemoryManager.h"

EngineApplication* EngineApp = NULL;

Application* CreateApplication() 
{
	return new EngineApplication();
}

EngineApplication::EngineApplication() 
{
	EngineApp = this;

	editor = new M_Editor();
	scriptManager = new M_EngineScriptManager();

	//13 = num of modules to pushback
	//if you create or remove 1 module change the 13 accordingly
	modules.reserve(13);

	// Main Modules
	AddModule(window);
	AddModule(camera);
	AddModule(input);
	AddModule(fileSystem);
	AddModule(resourceManager);

	// Scenes
	AddModule(scriptManager);
	AddModule(editor);
	AddModule(audio);
	AddModule(physics);
	AddModule(uiSystem);
	AddModule(detour);
	AddModule(scene);

	// Renderer last!
	AddModule(renderer);
	// -------------------------------------------

	input->AddModuleToProcessInput(editor);
	renderer->AddPostSceneRenderModule(editor);
	logger = editor;
}

EngineApplication::~EngineApplication()
{
}