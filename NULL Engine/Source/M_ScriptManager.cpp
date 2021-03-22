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
#include "R_Script.h"

#include "MemoryManager.h"

M_ScriptManager::M_ScriptManager(bool isActive): Module("ScriptManager", isActive), dllHandle(nullptr), lastModDll(0)
{
}

M_ScriptManager::~M_ScriptManager()
{
}

void M_ScriptManager::InitScripts()
{
	std::vector<Script*>::const_iterator it = currentScripts.cbegin();
	for (it; it != currentScripts.cend(); ++it)
	{
		if (*it != nullptr && (*it)->IsScriptEnabled())
		{
#ifndef GAMEBUILD
			try {
				(*it)->Awake();
			}
			catch (...)
			{
				LOG("CODE ERROR IN THE AWAKE OF THE SCRIPT: %s", (*it)->dataName);
			}
#else
			(*it)->Awake();
#endif // !GAMEBUILD
		}
	}

	it = currentScripts.cbegin();
	for (it; it != currentScripts.cend(); ++it)
	{
		if (*it != nullptr && (*it)->IsScriptEnabled())
		{
#ifndef GAMEBUILD
			try {
				(*it)->Start();
			}
			catch (...)
			{
				LOG("CODE ERROR IN THE START OF THE SCRIPT: %s", (*it)->dataName);
			}
#else
			(*it)->Start();
#endif // !GAMEBUILD
		}
	}
}

UpdateStatus M_ScriptManager::PreUpdate(float dt)
{
#ifndef GAMEBUILD
	if (App->fileSystem->Exists(SCRIPTS_DLL_OUTPUT)) 
	{
		uint64 lastMod = App->fileSystem->GetLastModTime(SCRIPTS_DLL_OUTPUT);
		if ( lastMod != lastModDll)
		{
			/*lastModDll = lastMod;
			HotReload();*/
		}
	}
#endif

	if (App->play && !App->pause)
	{
		for (std::vector<Script*>::const_iterator it = currentScripts.cbegin(); it != currentScripts.cend(); ++it)
		{
			if (*it != nullptr && (*it)->IsScriptEnabled())
			{
#ifndef GAMEBUILD
				try {
					(*it)->PreUpdate();
				}
				catch (...)
				{
					LOG("CODE ERROR IN THE PREUPDATE OF THE SCRIPT: %s", (*it)->dataName);
				}
#else
				(*it)->PreUpdate();
#endif // !GAMEBUILD
			}
		}
	}
	return UpdateStatus::CONTINUE;
}

UpdateStatus M_ScriptManager::Update(float dt)
{
	if (App->play && !App->pause)
	{
		for (std::vector<Script*>::const_iterator it = currentScripts.cbegin(); it != currentScripts.cend(); ++it)
		{
			if (*it != nullptr && (*it)->IsScriptEnabled())
			{
#ifndef GAMEBUILD
				try {
					(*it)->Update();
				}
				catch (...)
				{
					LOG("CODE ERROR IN THE UPDATE OF THE SCRIPT: %s", (*it)->dataName);
				}
#else
				(*it)->Update();
#endif // !GAMEBUILD
			}
		}
	}
	return UpdateStatus::CONTINUE;
}

UpdateStatus M_ScriptManager::PostUpdate(float dt)
{
	if (App->play && !App->pause)
	{
		for (std::vector<Script*>::const_iterator it = currentScripts.cbegin(); it != currentScripts.cend(); ++it) 
		{
			if(*it != nullptr && (*it)->IsScriptEnabled())
			{
#ifndef GAMEBUILD
				try {
					(*it)->PostUpdate();
				}
				catch (...)
				{
					LOG("CODE ERROR IN THE POSTUPDATE OF THE SCRIPT: %s", (*it)->dataName);
				}
#else
				(*it)->PostUpdate();
#endif // !GAMEBUILD
			}
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
		{
#ifndef GAMEBUILD
			try {
				(*it)->CleanUp();
			}
			catch (...)
			{
				LOG("CODE ERROR IN THE CLEANUP OF THE SCRIPT: %s", (*it)->dataName);
			}
#else
			(*it)->CleanUp();
#endif // !GAMEBUILD
		}
	}
}

bool M_ScriptManager::CleanUp()
{
	//TODO: clean up dels scripts
	CleanUpScripts();
	return true;
}

void M_ScriptManager::AddCurrentScript(Script* script)
{
	currentScripts.push_back(script);
}

HINSTANCE M_ScriptManager::GetDllHandle() const
{
	return dllHandle;
}

void M_ScriptManager::HotReload()
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

void M_ScriptManager::SerializeAllScripts(ParsonArray& scriptsArray)
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

void M_ScriptManager::DeSerializeAllScripts(ParsonArray& scriptsArray)
{

}

/*void M_ScriptManager::SerializeChildrenScripts(GameObject* go, ParsonArray& scriptsArray)
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
