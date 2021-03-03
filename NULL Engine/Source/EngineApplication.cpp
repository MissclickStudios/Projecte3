#include "EngineApplication.h"

#include "Module.h"
#include "M_Window.h"
#include "M_Input.h"
#include "M_Scene.h"
#include "M_Renderer3D.h"
#include "M_Camera3D.h"
#include "M_FileSystem.h"
#include "M_ResourceManager.h"
#include "M_Editor.h"

#include "MemoryManager.h"
#include "EngineMain.h"

EngineApplication* EngineApp = NULL;

Application* CreateApplication() 
{
	return new EngineApplication();
}

EngineApplication::EngineApplication() 
{
	EngineApp = this;

	editor = new M_Editor();

	// Main Modules
	AddModule(window);
	AddModule(camera);
	AddModule(input);
	AddModule(fileSystem);
	AddModule(resourceManager);

	// Scenes
	AddModule(scene);
	AddModule(editor);

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