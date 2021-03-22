#include "Application.h"
#include "M_ScriptManager.h"
#include "GameObject.h"
#include "Script.h"
#include "C_Script.h"
#include "R_Script.h"
#include "C_Transform.h"
#include "Log.h"
#include "JSONParser.h"
#include "M_ResourceManager.h"
#include "FileSystemDefinitions.h"

#include "MemoryManager.h"

C_Script::C_Script(GameObject* owner): Component(owner,ComponentType::SCRIPT)
{
}

C_Script::~C_Script()
{
	if (scriptData != nullptr) 
	{
		if (engineScript)
		{
			Script* script = (Script*)scriptData;
			std::vector<Script*>::iterator it = App->scriptManager->currentScripts.begin();
			for (it; it != App->scriptManager->currentScripts.end(); ++it) 
			{
				if (*it != nullptr && *it == script) {
					App->scriptManager->currentScripts.erase(it);
					break;
				}
			}
			if (script != nullptr)
				delete script;
		}
		else
		{
			void (*Deleter)(void*) = (void (*)(void*))GetProcAddress(App->scriptManager->GetDllHandle(), std::string("Destroy" + dataName).data());
			Deleter(scriptData);
		}
	}
}

bool C_Script::SaveState(ParsonNode& root) const
{
	root.SetNumber("Type", (uint)GetType());
	root.SetNumber("ResourceUID", resourceUID);
	root.SetString("DataName", dataName.c_str());
	/*if (inspectorVariables.empty())
		root.SetBool("HasInspector", false);
	else 
	{
		root.SetBool("HasInspector", true);
		ParsonArray variablesToSave = root.SetArray("InspectorVariables");
		for (int i = 0; i < inspectorVariables.size(); ++i)
		{
			//TODO: Save all the inspector variables on the C_Script
		}
	}*/
	return true;
}

bool C_Script::LoadState(ParsonNode& root)
{
	resourceUID = root.GetNumber("ResourceUID");
	dataName = root.GetString("DataName");

	if (App->resourceManager->AllocateResource(resourceUID, std::string(ASSETS_SCRIPTS_PATH + dataName).c_str())) {

		R_Script* rScript = (R_Script*)App->resourceManager->RequestResource((uint32)root.GetNumber("UID"));
		/*if (root.GetBool("HasInspector"))
		{
			ParsonArray variablesToLoad = root.GetArray("InspectorVariables");
			//TODO: InspectorData
			/*for (int i = 0; i < variablesToLoad.size; ++i)
			{
				//TODO: Load all the inspector variables on the C_Script
				inspectorVariables.push_back();
			}*/
		//}
	}
	else 
	{
		delete this;
	}
	return true;
}

void C_Script::LoadData(const char* name, bool engineScript)
{
	this->engineScript = engineScript;
	void* (*Creator)() = (void* (*)())GetProcAddress(App->scriptManager->dllHandle, std::string("Create" + std::string(name)).data());
	if (Creator != nullptr)
	{
		dataName = name;
		App->scriptManager->actualScriptLoading = this;
#ifndef GAMEBUILD
		try 
		{
			scriptData = Creator();
		}
		catch (...)
		{
			LOG("CODE ERROR IN THE CONSTRUCTOR OF THE SCRIPT: %s", name);
			return;
		}
#else
		scriptData = Creator();
#endif
		//game_object_attached->AddComponent(this);
		if (engineScript) 
		{
			Script* script = (Script*)scriptData;
			App->scriptManager->currentScripts.push_back(script);
			script->gameObject = GetOwner();
			script->transform = script->gameObject->GetComponent<C_Transform>();
			script->enabled = &isActive;
#ifndef GAMEBUILD
			script->dataName = &dataName;
#endif
		}
	}
}

const std::string& C_Script::GetDataName() const
{
	return dataName;
}

std::string C_Script::GetVariableName(const char* ptrName)
{
	std::string ptr_strg(ptrName);
	std::string variable_name;
	for (uint i = ptr_strg.size() - 1; i >= 0; --i) {
		if (ptr_strg[i] == '&' || ptr_strg[i] == '*' || ptr_strg[i] == '>' || ptr_strg[i] == '.')
			break;
		variable_name = ptr_strg[i] + variable_name;
	}
	return variable_name;
}

void C_Script::OnDisable()
{
	if (engineScript && scriptData != nullptr) {
#ifndef GAMEBUILD
		try {
			Script* script = (Script*)scriptData;
			script->OnDisable();
		}
		catch (...)
		{
			LOG("CODE ERROR IN THE ONDISABLE OF THE SCRIPT: %s", dataName.data());
		}
#else
		((Script*)scriptData)->OnDisable();
#endif // !GAMEBUILD
	}
}

void C_Script::OnEnable()
{
	if (engineScript && scriptData != nullptr) {
#ifndef GAMEBUILD
		try {
			Script* script = (Script*)scriptData;
			script->OnEnable();
		}
		catch (...)
		{
			LOG("CODE ERROR IN THE ONDISABLE OF THE SCRIPT: %s", dataName.data());
		}
#else
		((Script*)scriptData)->OnEnable();
#endif // !GAMEBUILD
	}
}
