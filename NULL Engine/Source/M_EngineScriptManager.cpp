#include "EngineApplication.h"
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
#include "Prefab.h"

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
	ResolveScriptHelperFunctions();

	App->resourceManager->GetAllScripts(aviableScripts);

	return true;
}

void M_EngineScriptManager::InitScripts()
{
	for (int i = 0; i < currentScripts.size(); ++i)
	{
		if (currentScripts[i] != nullptr && currentScripts[i]->IsScriptEnabled())
		{
			try {
				currentScripts[i]->Awake();
			}
			catch (...)
			{
				//TODO: Stop the game when error found on script???
				LOG("CODE ERROR IN THE AWAKE OF THE SCRIPT: %s", currentScripts[i]->dataName);
			}
		}
	}

	for (int i = 0; i < currentScripts.size(); ++i)
	{
		if (currentScripts[i] != nullptr && currentScripts[i]->IsScriptEnabled())
		{
			try {
				currentScripts[i]->Start();
			}
			catch (...)
			{
				LOG("CODE ERROR IN THE START OF THE SCRIPT: %s", currentScripts[i]->dataName);
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
			lastModDll = lastMod;
			HotReload();
		}
	}

	if (App->gameState == GameState::PLAY)
	{
		for (int i = 0; i < currentScripts.size(); ++i)
		{
			if (currentScripts[i] != nullptr && currentScripts[i]->IsScriptEnabled())
			{
				try {
					currentScripts[i]->PreUpdate();
				}
				catch (...)
				{
					LOG("CODE ERROR IN THE PREUPDATE OF THE SCRIPT: %s", currentScripts[i]->dataName);
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
		for (int i = 0; i < currentScripts.size(); ++i)
		{
			if (currentScripts[i] != nullptr && currentScripts[i]->IsScriptEnabled())
			{
				try {
					currentScripts[i]->Update();
				}
				catch (...)
				{
					LOG("CODE ERROR IN THE UPDATE OF THE SCRIPT: %s", currentScripts[i]->dataName);
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
		for (int i = 0; i < currentScripts.size(); ++i) 
		{
			if(currentScripts[i] != nullptr && currentScripts[i]->IsScriptEnabled())
			{
				try {
					currentScripts[i]->PostUpdate();
				}
				catch (...)
				{
					LOG("CODE ERROR IN THE POSTUPDATE OF THE SCRIPT: %s", currentScripts[i]->dataName);
				}
			}
		}
	}
	return UpdateStatus::CONTINUE;
}

void M_EngineScriptManager::CleanUpScripts()
{
	for (int i = 0; i < currentScripts.size(); ++i)
	{
		if (currentScripts[i] != nullptr)
		{
			try {
				currentScripts[i]->CleanUp();
			}
			catch (...)
			{
				LOG("CODE ERROR IN THE CLEANUP OF THE SCRIPT: %s", currentScripts[i]->dataName);
			}
		}
	}
}

bool M_EngineScriptManager::CleanUp()
{
	//TODO: clean up dels scripts
	CleanUpScripts();
	aviableScripts.clear();
	currentScripts.clear();
	return true;
}

void M_EngineScriptManager::HotReload()
{
	ParsonNode root = ParsonNode();
	if (currentScripts.empty())
	{
		root.SetBool("HaveScripts", false);
	}
	else {
		root.SetBool("HaveScripts", true);
		ParsonArray scriptsArray = root.SetArray("CurrentScripts");
		SerializeAllScripts(scriptsArray);
	}
	/*char* buffer = nullptr;
	root.SerializeToFile("Library/ScriptsTemp.temp", &buffer);
	RELEASE_ARRAY(buffer);*/

	currentScripts.clear();
	if (FreeLibrary(dllHandle))
	{
		LOG("Successfully unloaded scriptsDLL");
		if (App->fileSystem->Remove(SCRIPTS_DLL_WORKING))
		{
			LOG("Successfully removed scriptsDLL");
			EngineApp->resourceManager->ReloadAllScripts();

			while (MoveFileA(SCRIPTS_DLL_OUTPUT, SCRIPTS_DLL_WORKING) == FALSE) {}
			LOG("New dll correctly moved!");
			dllHandle = LoadLibrary(SCRIPTS_DLL_WORKING);
			if (dllHandle != nullptr) 
			{
				LOG("Successfully loaded new scripts dll");
				ResolveScriptHelperFunctions();
				if (root.GetBool("HaveScripts")) 
				{
					DeSerializeAllScripts(root.GetArray("CurrentScripts"));
					//root.Release();
					aviableScripts.clear();
					EngineApp->resourceManager->GetAllScripts(aviableScripts);
					if (EngineApp->gameState == GameState::PLAY)
						InitScripts();
				}
			}
		}
	}
}

void M_EngineScriptManager::SerializeAllScripts(ParsonArray& scriptsArray)
{
	std::vector<GameObject*>* gameObjects = App->scene->GetGameObjects();
	for (auto it = (*gameObjects).cbegin(); it!= (*gameObjects).cend(); ++it)
	{
		if ((*it) != nullptr)
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
						scriptNode.SetString("ResourceAssetsPath", (*cScript)->resource->GetAssetsPath());
						scriptNode.SetString("DataName", (*cScript)->GetDataName().c_str());
						//TODO: Do i need to reset the component id???? IF yes how do I do it???
						scriptNode.SetNumber("ComponentScriptID", (*cScript)->GetID());
						const std::vector<InspectorScriptData> scriptVariables = (*cScript)->GetInspectorVariables();
						if (scriptVariables.empty())
							scriptNode.SetBool("HasInspector", false);
						else
						{
							scriptNode.SetBool("HasInspector", true);
							ParsonArray variablesToSave = scriptNode.SetArray("InspectorVariables");
							for (int i = 0; i < scriptVariables.size(); ++i)
							{
								ParsonNode variable = variablesToSave.SetNode(scriptVariables[i].variableName.c_str());
								variable.SetInteger("Type", (int)scriptVariables[i].variableType);
								variable.SetString("Name", scriptVariables[i].variableName.c_str());
								switch (scriptVariables[i].variableType)
								{
								case InspectorScriptData::DataType::INT:
									variable.SetInteger("int", *(int*)scriptVariables[i].ptr); break;
								case InspectorScriptData::DataType::BOOL:
									variable.SetBool("bool", *(bool*)scriptVariables[i].ptr); break;
								case InspectorScriptData::DataType::FLOAT:
									variable.SetNumber("float", *(float*)scriptVariables[i].ptr); break;
								case InspectorScriptData::FLOAT3:
									variable.SetNumber("float3x", (*(float3*)scriptVariables[i].ptr).x);
									variable.SetNumber("float3y", (*(float3*)scriptVariables[i].ptr).y);
									variable.SetNumber("float3z", (*(float3*)scriptVariables[i].ptr).z);
									break;
								case InspectorScriptData::STRING:
									variable.SetString("string", (*(std::string*)scriptVariables[i].ptr).c_str()); break;
								case InspectorScriptData::DataType::PREFAB:
									variable.SetInteger("prefab", (*(Prefab*)scriptVariables[i].ptr).uid); break;
								case InspectorScriptData::DataType::GAMEOBJECT:
									if (scriptVariables[i].obj != nullptr && *scriptVariables[i].obj != nullptr)
										variable.SetInteger("gameobject", (*scriptVariables[i].obj)->GetUID());
									else
										variable.SetInteger("gameobject", 0);
									break;
								case InspectorScriptData::VECTORSTRING:
								{
									ParsonArray parsonStringArray = variable.SetArray("vectorstring");
									for (std::vector<std::string>::const_iterator cit = (*(std::vector<std::string>*)scriptVariables[i].ptr).cbegin(); cit != (*(std::vector<std::string>*)scriptVariables[i].ptr).cend(); ++cit)
									{
										parsonStringArray.SetString((*cit).c_str());
									}
									break;
								}
								}
							}
						}
						(*it)->DeleteComponent((*cScript)); //Is it necessari to remove it and add it again later?
					}
				}
			}
		}
	}
}

void M_EngineScriptManager::DeSerializeAllScripts(const ParsonArray& scriptsArray)
{
	for (int i = 0; i < scriptsArray.size; ++i)
	{
		ParsonNode scriptNode = scriptsArray.GetNode(i);
		GameObject* scriptGO = EngineApp->scene->GetGameObjectByUID((uint32)scriptNode.GetNumber("GameObjectUID"));
		if (scriptGO == nullptr)
			continue;
		C_Script* cScript = (C_Script*)scriptGO->CreateComponent(ComponentType::SCRIPT);
		//cScript->id = ...;
		uint32 resourceUID = (uint32)scriptNode.GetNumber("ResourceScriptUID");
		if (!EngineApp->resourceManager->AllocateResource(resourceUID, scriptNode.GetString("ResourceAssetsPath")))
		{
			LOG("[WARNING] The resource for script %s from header file %s doesn't exist so the script has not been loaded", scriptNode.GetString("DataName"), scriptNode.GetString("ResourceAssetsPath"));
			scriptGO->DeleteComponent(cScript);
			continue;
		}
		cScript->resource = (R_Script*)EngineApp->resourceManager->RequestResource(resourceUID);
		const char* name = scriptNode.GetString("DataName");
		bool found = false;
		for (int i = 0; i < cScript->resource->dataStructures.size(); ++i)
		{
			if (!strcmp(name,cScript->resource->dataStructures[i].first.c_str()))
			{
				cScript->LoadData(name, cScript->resource->dataStructures[i].second);
				if (scriptNode.GetBool("HasInspector")) 
				{
					ParsonArray variablesToLoad = scriptNode.GetArray("InspectorVariables");
					for (int i = 0; i < variablesToLoad.size; ++i)
					{
						ParsonNode variable = variablesToLoad.GetNode(i);
						std::string variableName = variable.GetString("Name");
						InspectorScriptData::DataType type = (InspectorScriptData::DataType)variable.GetInteger("Type");
						std::vector<InspectorScriptData>::iterator item =cScript->inspectorVariables.begin();
						for(item;item != cScript->inspectorVariables.end();++item)
						{
							if (type == (*item).variableType && (*item).variableName == variableName)
							{
								switch (type)
								{
								case InspectorScriptData::DataType::INT:
									*(int*)(*item).ptr = variable.GetInteger("int"); break;
								case InspectorScriptData::DataType::BOOL:
									*(bool*)(*item).ptr = variable.GetBool("bool"); break;
								case InspectorScriptData::DataType::FLOAT:
									*(float*)(*item).ptr = (float)variable.GetNumber("float"); break;
								case InspectorScriptData::FLOAT3:
									(*(float3*)(*item).ptr).x = variable.GetNumber("float3x");
									(*(float3*)(*item).ptr).y = variable.GetNumber("float3y");
									(*(float3*)(*item).ptr).z = variable.GetNumber("float3z");
									break;
								case InspectorScriptData::STRING:
									*(std::string*)(*item).ptr = variable.GetString("string"); break;
								case InspectorScriptData::DataType::PREFAB:
									*(Prefab*)(*item).ptr = EngineApp->resourceManager->prefabs[(unsigned int)variable.GetNumber("prefab")]; break;
								case InspectorScriptData::DataType::GAMEOBJECT: 
								{
									uint32 id = variable.GetInteger("gameobject");
									if (id != 0)
										*(*item).obj = EngineApp->scene->GetGameObjectByUID(id);
									break; 
								}
								case InspectorScriptData::VECTORSTRING:
								{
									//std::vector<std::string>& inspectorStringVector = *(std::vector<std::string>*)(*item).ptr;
									ParsonArray parsonStringArray = variable.GetArray("vectorstring");
									//inspectorStringVector.reserve(parsonStringArray.size);
									EngineApp->scriptManager->StringVecReserve((*item).ptr, parsonStringArray.size);
									for (int j = 0; j < parsonStringArray.size; ++j)
									{
										//inspectorStringVector.emplace_back(parsonStringArray.GetString(i));
										EngineApp->scriptManager->StringVecEmplaceBackString((*item).ptr, parsonStringArray.GetString(j));
									}
									break;
								}
								}
							}
						}
					}
				}
				found = true;
				break;
			}
		}
		if (!found) 
		{
			LOG("[WARNING] The resource for script %s from header file %s exists but doesn't contain this script so it has not been loaded", scriptNode.GetString("DataName"), scriptNode.GetString("ResourceAssetsPath"));
			scriptGO->DeleteComponent(cScript);
			continue;
		}
	}
}