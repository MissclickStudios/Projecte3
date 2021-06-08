#include "Application.h"
#include "Profiler.h"
#include "Script.h"
#include "M_ScriptManager.h"
#include "C_Script.h"
#include "Log.h"
#include "FileSystemDefinitions.h"
#include "M_FileSystem.h"
#include "JSONParser.h"
#include "M_Scene.h"
#include "GameObject.h"
#include "M_ResourceManager.h"
#include "R_Script.h"

#include "MemoryManager.h"

M_ScriptManager::M_ScriptManager(bool isActive) : Module("ScriptManager", isActive), dllHandle(nullptr)
{
}

M_ScriptManager::~M_ScriptManager()
{
}

bool M_ScriptManager::Start()
{
	//TODO: Poder aixo no es necessita en el game
	if (App->fileSystem->Exists(SCRIPTS_DLL_OUTPUT))
	{
		App->fileSystem->Remove(SCRIPTS_DLL_WORKING);
		while (MoveFileA(SCRIPTS_DLL_OUTPUT, SCRIPTS_DLL_WORKING) == FALSE) {}
	}
	dllHandle = LoadLibrary(SCRIPTS_DLL_WORKING);
	ResolveScriptHelperFunctions();
	return true;
}

void M_ScriptManager::InitScripts()
{
	for (int i = 0; i < currentScripts.size(); ++i)
	{
		if (currentScripts[i] != nullptr && currentScripts[i]->IsScriptEnabled())
			currentScripts[i]->Awake();
	}
	for (int i = 0; i < currentScripts.size(); ++i)
	{
		if (currentScripts[i] != nullptr && currentScripts[i]->IsScriptEnabled())
			currentScripts[i]->Start();
	}
}

UpdateStatus M_ScriptManager::PreUpdate(float dt)
{
	OPTICK_CATEGORY("M_ScriptManager PreUpdate", Optick::Category::Module)
	//if (App->gameState == GameState::PLAY)
	//{
	for (int i = 0; i < currentScripts.size(); ++i)
	{
		if (currentScripts[i] != nullptr && currentScripts[i]->IsScriptEnabled())
			currentScripts[i]->PreUpdate();
	}
	//}
	return UpdateStatus::CONTINUE;
}

UpdateStatus M_ScriptManager::Update(float dt)
{
	OPTICK_CATEGORY("M_ScriptManager Update", Optick::Category::Module)
	//if (App->gameState == GameState::PLAY)
	//{
	for (int i = 0; i < currentScripts.size(); ++i)
	{
		if (currentScripts[i] != nullptr && currentScripts[i]->IsScriptEnabled())
			currentScripts[i]->Update();
	}	
	//}
	return UpdateStatus::CONTINUE;
}

UpdateStatus M_ScriptManager::PostUpdate(float dt)
{
	OPTICK_CATEGORY("M_ScriptManager PostUpdate", Optick::Category::Module)
	//if (App->gameState == GameState::PLAY)
	//{
	for (int i = 0; i < currentScripts.size(); ++i)
	{
		if (currentScripts[i] != nullptr && currentScripts[i]->IsScriptEnabled())
			currentScripts[i]->PostUpdate();
	}
	//}
	return UpdateStatus::CONTINUE;
}

void M_ScriptManager::CleanUpScripts()
{
	//TODO: Maybe cleanUp removes scripts !!!
	for (int i = 0; i < currentScripts.size(); ++i)
	{
		if (currentScripts[i] != nullptr && currentScripts[i]->IsScriptEnabled())
			currentScripts[i]->CleanUp();
	}
	/*std::vector<Script*>::const_iterator it = currentScripts.cbegin();
	for (it; it != currentScripts.cend(); ++it)
	{
		if (*it != nullptr)
			(*it)->CleanUp();
	}*/
}

bool M_ScriptManager::CleanUp()
{
	//TODO: clean up dels scripts
	CleanUpScripts();
	currentScripts.clear();
	return true;
}

HINSTANCE M_ScriptManager::GetDllHandle() const
{
	return dllHandle;
}

bool M_ScriptManager::ParseEnum(const char* enumName, const char* definitionFile)
{
	return true;
}

void M_ScriptManager::ResolveScriptHelperFunctions()
{
	if (dllHandle != nullptr)
	{
		StringVecPushBackString = (void(*)(void*, const std::string&))GetProcAddress(dllHandle, "StringVectorPushBackString");
		StringVecPushBackChar = (void(*)(void*, const char*))GetProcAddress(dllHandle, "StringVectorPushBackChar");
		StringVecEmplaceBackString = (void(*)(void*, const std::string&))GetProcAddress(dllHandle, "StringVectorEmplaceBackString");
		StringVecEmplaceBackChar = (void(*)(void*, const char*))GetProcAddress(dllHandle, "StringVectorEmplaceBackChar");
		StringVecReserve = (void(*)(void*, int))GetProcAddress(dllHandle, "StringVectorReserve");
		StringVecErase = (void(*)(void*, int))GetProcAddress(dllHandle, "StringVectorErase");
		SetString = (void(*)(void*, const char*))GetProcAddress(dllHandle, "SetString");
	}
}
