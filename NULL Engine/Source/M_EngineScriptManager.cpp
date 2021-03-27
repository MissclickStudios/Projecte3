#include "Application.h"
#include "Script.h"
#include "M_EngineScriptManager.h"
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

M_EngineScriptManager::M_EngineScriptManager(bool isActive): M_ScriptManager() ,lastModDll(0)
{
}

M_EngineScriptManager::~M_EngineScriptManager()
{
}

bool M_EngineScriptManager::Start()
{
	//TODO: Poder aixo no es necessita en el game
	if (App->fileSystem->Exists(SCRIPTS_DLL_OUTPUT)) 
	{
		App->fileSystem->Remove(SCRIPTS_DLL_WORKING);
		lastModDll = App->fileSystem->GetLastModTime(SCRIPTS_DLL_OUTPUT);
		while (MoveFileA(SCRIPTS_DLL_OUTPUT, SCRIPTS_DLL_WORKING) == FALSE) {}
	}
	dllHandle = LoadLibrary(SCRIPTS_DLL_WORKING);

	App->resourceManager->GetAllScripts(aviableScripts);

	return true;
}

void M_EngineScriptManager::InitScripts()
{
	std::vector<Script*>::const_iterator it = currentScripts.cbegin();
	for (it; it != currentScripts.cend(); ++it)
	{
		if (*it != nullptr && (*it)->IsScriptEnabled())
		{
			try {
				(*it)->Awake();
			}
			catch (...)
			{
				//TODO: Stop the game when error found on script???
				LOG("CODE ERROR IN THE AWAKE OF THE SCRIPT: %s", (*it)->dataName);
			}
		}
	}

	it = currentScripts.cbegin();
	for (it; it != currentScripts.cend(); ++it)
	{
		if (*it != nullptr && (*it)->IsScriptEnabled())
		{
			try {
				(*it)->Start();
			}
			catch (...)
			{
				LOG("CODE ERROR IN THE START OF THE SCRIPT: %s", (*it)->dataName);
			}
		}
	}
}

UpdateStatus M_EngineScriptManager::PreUpdate(float dt)
{
	if (App->fileSystem->Exists(SCRIPTS_DLL_OUTPUT)) 
	{
		uint64 lastMod = App->fileSystem->GetLastModTime(SCRIPTS_DLL_OUTPUT);
		if ( lastMod != lastModDll)
		{
			/*lastModDll = lastMod;
			HotReload();*/
			aviableScripts.clear();
			App->resourceManager->GetAllScripts(aviableScripts);
		}
	}

	if (App->gameState == GameState::PLAY)
	{
		for (std::vector<Script*>::const_iterator it = currentScripts.cbegin(); it != currentScripts.cend(); ++it)
		{
			if (*it != nullptr && (*it)->IsScriptEnabled())
			{
				try {
					(*it)->PreUpdate();
				}
				catch (...)
				{
					LOG("CODE ERROR IN THE PREUPDATE OF THE SCRIPT: %s", (*it)->dataName);
				}
			}
		}
	}
	return UpdateStatus::CONTINUE;
}

UpdateStatus M_EngineScriptManager::Update(float dt)
{
	if (App->gameState == GameState::PLAY)
	{
		for (std::vector<Script*>::const_iterator it = currentScripts.cbegin(); it != currentScripts.cend(); ++it)
		{
			if (*it != nullptr && (*it)->IsScriptEnabled())
			{
				try {
					(*it)->Update();
				}
				catch (...)
				{
					LOG("CODE ERROR IN THE UPDATE OF THE SCRIPT: %s", (*it)->dataName);
				}
			}
		}
	}
	return UpdateStatus::CONTINUE;
}

UpdateStatus M_EngineScriptManager::PostUpdate(float dt)
{
	if (App->gameState == GameState::PLAY)
	{
		for (std::vector<Script*>::const_iterator it = currentScripts.cbegin(); it != currentScripts.cend(); ++it) 
		{
			if(*it != nullptr && (*it)->IsScriptEnabled())
			{
				try {
					(*it)->PostUpdate();
				}
				catch (...)
				{
					LOG("CODE ERROR IN THE POSTUPDATE OF THE SCRIPT: %s", (*it)->dataName);
				}
			}
		}
	}
	return UpdateStatus::CONTINUE;
}

void M_EngineScriptManager::CleanUpScripts()
{
	std::vector<Script*>::const_iterator it = currentScripts.cbegin();
	for (it; it != currentScripts.cend(); ++it)
	{
		if (*it != nullptr)
		{
			try {
				(*it)->CleanUp();
			}
			catch (...)
			{
				LOG("CODE ERROR IN THE CLEANUP OF THE SCRIPT: %s", (*it)->dataName);
			}
		}
	}
}

bool M_EngineScriptManager::CleanUp()
{
	//TODO: clean up dels scripts
	CleanUpScripts();
	aviableScripts.clear();
	return true;
}

void M_EngineScriptManager::HotReload()
{
	ParsonNode root = ParsonNode();
	/*char* buffer = nullptr;
	root.SerializeToFile("Library/ScriptsTemp.temp",&buffer);
	RELEASE_ARRAY(buffer);*/
	if (currentScripts.empty())
	{
		root.SetBool("HaveScripts", false);
	}
	else {
		root.SetBool("HaveScripts", true);
		ParsonArray scriptsArray = root.SetArray("CurrentScripts");
		SerializeAllScripts(scriptsArray);
	}
	char* buffer = nullptr;
	root.SerializeToFile("Library/ScriptsTemp.temp", &buffer);
	RELEASE_ARRAY(buffer);

	currentScripts.clear();
	if (FreeLibrary(dllHandle))
	{
		LOG("Successfully unloaded scriptsDLL");
		if (App->fileSystem->Remove(SCRIPTS_DLL_WORKING))
		{
			LOG("Successfully removed scriptsDLL");
			//ReloadAllScripts();

			while (MoveFileA(SCRIPTS_DLL_OUTPUT, SCRIPTS_DLL_WORKING) == FALSE) {}
		}
	}
}

void M_EngineScriptManager::SerializeAllScripts(ParsonArray& scriptsArray)
{
	/*std::vector<GameObject*>* objects = App->scene->GetGameObjects();
	for (std::vector<GameObject*>::const_iterator it = (*objects).cbegin(); it!= (*objects).cend(); ++it)
	{
		if (*it != nullptr)
		{
			std::vector<C_Script*> scripts;
			(*it)->GetComponents<C_Script>(scripts);
			if (!scripts.empty())
			{
				std::vector<C_Script*>::const_iterator cScript = scripts.cbegin();
				for (cScript; cScript != scripts.cend(); ++cScript)
				{
					if (*cScript != nullptr)
					{
						ParsonNode scriptNode = scriptsArray.SetNode((*cScript)->GetDataName().c_str());
						scriptNode.SetNumber("GameObjectUID", (*it)->GetUID());
						scriptNode.SetNumber("ResourceScriptUID", (*cScript)->resource->GetUID()); //TODO: check if resource not null??? TODO: Guardar nomes el id poder???
						scriptNode.SetString("DataName", (*cScript)->GetDataName().c_str());
						scriptNode.SetNumber("ComponentScriptID", (*cScript)->GetID());
						*//*if (inspectorVariables)
						{
							//TODO:Fer les inspector variables !!!!!!!
						}*//*
					}
				}
			}
		}
	}*/
}

void M_EngineScriptManager::DeSerializeAllScripts(ParsonArray& scriptsArray)
{

}

/*void M_EngineScriptManager::SerializeChildrenScripts(GameObject* go, ParsonArray& scriptsArray)
{
	if (go->HasChilds()) 
	{
		std::vector<GameObject*>::const_iterator it = go->childs.cbegin();
		for (it; it != go->childs.cend(); ++it) 
		{
			if (*it != nullptr) 
			{
				std::vector<C_Script*> scripts;
				(*it)->GetComponents<C_Script>(scripts);
				if (!scripts.empty())
				{
					std::vector<C_Script*>::const_iterator script = scripts.cbegin();
					for (script; script != scripts.cend(); ++script)
					{
						if (*script != nullptr)
						{

						}
					}
				}
			}
		}
	}
}*/
