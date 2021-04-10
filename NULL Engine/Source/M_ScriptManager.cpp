#include "Application.h"
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
	return true;
}

void M_ScriptManager::InitScripts()
{
	std::vector<Script*>::const_iterator it = currentScripts.cbegin();
	for (it; it != currentScripts.cend(); ++it)
	{
		if (*it != nullptr && (*it)->IsScriptEnabled())
			(*it)->Awake();
	}

	it = currentScripts.cbegin();
	for (it; it != currentScripts.cend(); ++it)
	{
		if (*it != nullptr && (*it)->IsScriptEnabled())
			(*it)->Start();
	}
}

UpdateStatus M_ScriptManager::PreUpdate(float dt)
{
	if (App->gameState == GameState::PLAY)
	{
		for (std::vector<Script*>::const_iterator it = currentScripts.cbegin(); it != currentScripts.cend(); ++it)
		{
			if (*it != nullptr && (*it)->IsScriptEnabled())
				(*it)->PreUpdate();
		}
	}
	return UpdateStatus::CONTINUE;
}

UpdateStatus M_ScriptManager::Update(float dt)
{
	if (App->gameState == GameState::PLAY)
	{
		for (std::vector<Script*>::const_iterator it = currentScripts.cbegin(); it != currentScripts.cend(); ++it)
		{
			if (*it != nullptr && (*it)->IsScriptEnabled())
				(*it)->Update();
		}
	}
	return UpdateStatus::CONTINUE;
}

UpdateStatus M_ScriptManager::PostUpdate(float dt)
{
	if (App->gameState == GameState::PLAY)
	{
		for (std::vector<Script*>::const_iterator it = currentScripts.cbegin(); it != currentScripts.cend(); ++it)
		{
			if (*it != nullptr && (*it)->IsScriptEnabled())
				(*it)->PostUpdate();
		}
	}
	return UpdateStatus::CONTINUE;
}

void M_ScriptManager::CleanUpScripts()
{
	std::vector<Script*>::const_iterator it = currentScripts.cbegin();
	for (it; it != currentScripts.cend(); ++it)
	{
		if (*it != nullptr)
			(*it)->CleanUp();
	}
}

bool M_ScriptManager::CleanUp()
{
	//TODO: clean up dels scripts
	CleanUpScripts();
	return true;
}

HINSTANCE M_ScriptManager::GetDllHandle() const
{
	return dllHandle;
}