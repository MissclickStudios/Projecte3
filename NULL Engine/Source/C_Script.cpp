#include "C_Script.h"
#include <Windows.h>
#include <typeinfo>
#include "Application.h"
#include "GameObject.h"
#include "Script.h"
#include "C_Script.h"
#include "R_Script.h"
#include "C_Transform.h"
#include "Log.h"
#include "JSONParser.h"
#include "Prefab.h"
#include "M_ResourceManager.h"
#include "M_ScriptManager.h"
#include "FileSystemDefinitions.h"
#include "Prefab.h"

#include "MemoryManager.h"

C_Script::C_Script(GameObject* owner): Component(owner,ComponentType::SCRIPT), resource(nullptr)
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
			{
				delete script;
				scriptData = nullptr;
			}
		}
		else
		{
			void (*Deleter)(void*) = (void (*)(void*))GetProcAddress(App->scriptManager->GetDllHandle(), std::string("Delete" + dataName).data());
			Deleter(scriptData);
			scriptData = nullptr;
		}
	}
}

bool C_Script::CleanUp()
{
	if (resource != nullptr)
	{
		App->resourceManager->FreeResource(resource->GetUID());
		resource = nullptr;
	}
	return true;
}

bool C_Script::SaveState(ParsonNode& root) const
{
	root.SetNumber("Type", (uint)GetType());
	root.SetNumber("ResourceUID", resource->GetUID());
	root.SetString("AssetsPath", resource->GetAssetsPath());
	root.SetString("DataName", dataName.c_str());
	if (inspectorVariables.empty())
		root.SetBool("HasInspector", false);
	else 
	{
		root.SetBool("HasInspector", true);
		ParsonArray variablesToSave = root.SetArray("InspectorVariables");
		for (int i = 0; i < inspectorVariables.size(); ++i)
		{
			ParsonNode variable = variablesToSave.SetNode(inspectorVariables[i].variableName.c_str());
			variable.SetInteger("Type", (int)inspectorVariables[i].variableType);
			variable.SetString("Name", inspectorVariables[i].variableName.c_str());
			switch (inspectorVariables[i].variableType) 
			{
			case InspectorScriptData::DataType::INT:
				variable.SetInteger("int", *(int*)inspectorVariables[i].ptr); break;
			case InspectorScriptData::BOOL:
				variable.SetBool("bool", *(bool*)inspectorVariables[i].ptr); break;
			case InspectorScriptData::FLOAT:
				variable.SetNumber("float", *(float*)inspectorVariables[i].ptr); break;
			case InspectorScriptData::PREFAB:
				variable.SetInteger("prefab", (*(Prefab*)inspectorVariables[i].ptr).uid); break;
			case InspectorScriptData::GAMEOBJECT:
				/*if (inspectorVariables[i].obj != nullptr)
					variable.SetInteger("gameobject", (*inspectorVariables[i].obj)->GetUID());
				else
					variable.SetInteger("gameobject", 0);*/
				break;
			}
		}
	}
	return true;
}

bool C_Script::LoadState(ParsonNode& root)
{
	dataName = root.GetString("DataName");

	if (App->resourceManager->AllocateResource(root.GetNumber("ResourceUID"), root.GetString("AssetsPath"))) 
	{
		bool found = false;
		resource = (R_Script*)App->resourceManager->RequestResource(root.GetNumber("ResourceUID"));
		for (int i = 0; i < resource->dataStructures.size(); ++i) 
		{
			if (dataName.data() == resource->dataStructures[i].first) 
			{
				LoadData(dataName.data(), resource->dataStructures[i].second);
				found = true;
				break;
			}
		}

		if (!found)
			return false;

		if (root.GetBool("HasInspector"))
		{
			ParsonArray variablesToLoad = root.GetArray("InspectorVariables");
			for (int i = 0; i < variablesToLoad.size; ++i)
			{
				ParsonNode variable = variablesToLoad.GetNode(i);
				inspectorVariables[i].variableType = (InspectorScriptData::DataType)variable.GetInteger("Type");
				inspectorVariables[i].variableName = variable.GetString("Name");
				switch (inspectorVariables[i].variableType)
				{
				case InspectorScriptData::DataType::INT:
					*(int*)inspectorVariables[i].ptr = variable.GetInteger("int"); break;
				case InspectorScriptData::BOOL:
					*(bool*)inspectorVariables[i].ptr = variable.GetBool("bool"); break;
				case InspectorScriptData::DataType::FLOAT:
					*(float*)inspectorVariables[i].ptr = variable.GetNumber("float"); break;
				case InspectorScriptData::DataType::PREFAB:
					*(Prefab*)inspectorVariables[i].ptr = App->resourceManager->prefabs[(unsigned int)variable.GetInteger("prefab")]; break;
				/*case InspectorScriptData::DataType::GAMEOBJECT: //TODO: FINISH THIS !!!!
					uint32 id = variable.GetInteger("gameobject");
					if (id != 0)
						;
					break;*/
				}
			}
		}
	}
	else 
	{
		return false;
	}
	return true;
}

//TODO: Maybe put this function on the constructor of the component ?
void C_Script::LoadData(const char* name, bool engineScript)
{
	this->engineScript = engineScript;
	void* (*Creator)() = (void* (*)())GetProcAddress(App->scriptManager->dllHandle, std::string("Create" + std::string(name)).data());
	if (Creator != nullptr)
	{
		dataName = name;
		App->scriptManager->actualScriptLoading = this;
		//TODO: Remove tots els ifndef del C_Script !!!!! (fer calls al script manager)
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

const std::vector<InspectorScriptData>& C_Script::GetInspectorVariables() const
{
	return inspectorVariables;
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

void C_Script::SetIsActive(bool setTo)
{
	if (setTo != isActive) 
	{
		if (App->gameState == GameState::PLAY) 
		{
			isActive = setTo;
			if (isActive)
				OnEnable();
			else
				OnDisable();
		}
	}
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

void C_Script::OnCollisionEnter()
{
	if (engineScript && scriptData != nullptr) {
#ifndef GAMEBUILD
		try {
			Script* script = (Script*)scriptData;
			script->OnCollisionEnter();
		}
		catch (...)
		{
			LOG("CODE ERROR IN THE ONDISABLE OF THE SCRIPT: %s", dataName.data());
		}
#else
		((Script*)scriptData)->OnCollisionEnter();
#endif // !GAMEBUILD
	}
}

void C_Script::OnCollisionRepeat()
{
	if (engineScript && scriptData != nullptr) {
#ifndef GAMEBUILD
		try {
			Script* script = (Script*)scriptData;
			script->OnCollisionRepeat();
		}
		catch (...)
		{
			LOG("CODE ERROR IN THE ONDISABLE OF THE SCRIPT: %s", dataName.data());
		}
#else
		((Script*)scriptData)->OnCollisionRepeat();
#endif // !GAMEBUILD
	}
}

void C_Script::OnCollisionExit()
{
	if (engineScript && scriptData != nullptr) {
#ifndef GAMEBUILD
		try {
			Script* script = (Script*)scriptData;
			script->OnCollisionExit();
		}
		catch (...)
		{
			LOG("CODE ERROR IN THE ONDISABLE OF THE SCRIPT: %s", dataName.data());
		}
#else
		((Script*)scriptData)->OnCollisionExit();
#endif // !GAMEBUILD
	}
}

void C_Script::OnTriggerEnter()
{
	if (engineScript && scriptData != nullptr) {
#ifndef GAMEBUILD
		try {
			Script* script = (Script*)scriptData;
			script->OnTriggerEnter();
		}
		catch (...)
		{
			LOG("CODE ERROR IN THE ONDISABLE OF THE SCRIPT: %s", dataName.data());
		}
#else
		((Script*)scriptData)->OnTriggerEnter();
#endif // !GAMEBUILD
	}
}

void C_Script::OnTriggerRepeat()
{
	if (engineScript && scriptData != nullptr) {
#ifndef GAMEBUILD
		try {
			Script* script = (Script*)scriptData;
			script->OnTriggerRepeat();
		}
		catch (...)
		{
			LOG("CODE ERROR IN THE ONDISABLE OF THE SCRIPT: %s", dataName.data());
		}
#else
		((Script*)scriptData)->OnTriggerRepeat();
#endif // !GAMEBUILD
	}
}

void C_Script::OnTriggerExit()
{
	if (engineScript && scriptData != nullptr) {
#ifndef GAMEBUILD
		try {
			Script* script = (Script*)scriptData;
			script->OnTriggerExit();
		}
		catch (...)
		{
			LOG("CODE ERROR IN THE ONDISABLE OF THE SCRIPT: %s", dataName.data());
		}
#else
		((Script*)scriptData)->OnTriggerExit();
#endif // !GAMEBUILD
	}
}

bool C_Script::HasData() const
{
	return scriptData != nullptr;
}

void C_Script::InspectorInputInt(int* variablePtr, const char* ptrName)
{
	const char* name = typeid(*variablePtr).name();
	if (strcmp(name, "int"))
		return;

	std::string variableName = GetVariableName(ptrName);

	C_Script* script = App->scriptManager->actualScriptLoading;
	if (script != nullptr)
		script->inspectorVariables.push_back(InspectorScriptData(variableName, InspectorScriptData::DataType::INT, variablePtr, InspectorScriptData::ShowMode::INPUT_INT));
}

void C_Script::InspectorDragableInt(int* variablePtr, const char* ptrName)
{
	const char* name = typeid(*variablePtr).name();
	if (strcmp(name, "int"))
		return;

	std::string variableName = GetVariableName(ptrName);

	C_Script* script = App->scriptManager->actualScriptLoading;
	if (script != nullptr)
		script->inspectorVariables.push_back(InspectorScriptData(variableName, InspectorScriptData::DataType::INT, variablePtr, InspectorScriptData::ShowMode::DRAGABLE_INT));
}

void C_Script::InspectorSliderInt(int* variablePtr, const char* ptrName, const int& minValue, const int& maxValue)
{
	const char* name = typeid(*variablePtr).name();
	if (strcmp(name, "int"))
		return;

	std::string variableName = GetVariableName(ptrName);

	C_Script* script = App->scriptManager->actualScriptLoading;
	if (script != nullptr) 
	{
		InspectorScriptData inspector(variableName, InspectorScriptData::DataType::INT, variablePtr, InspectorScriptData::ShowMode::SLIDER_INT);
		inspector.minSlider = minValue;
		inspector.maxSlider = maxValue;
		script->inspectorVariables.push_back(inspector);
	}
}

void C_Script::InspectorBool(bool* variablePtr, const char* ptrName)
{
	const char* name = typeid(*variablePtr).name();
	if (strcmp(name, "bool"))
		return;

	std::string variableName = GetVariableName(ptrName);

	C_Script* script = App->scriptManager->actualScriptLoading;
	if (script != nullptr)
	{
		InspectorScriptData inspector(variableName, InspectorScriptData::DataType::BOOL, variablePtr, InspectorScriptData::ShowMode::CHECKBOX);
		script->inspectorVariables.push_back(inspector);
	}
}

void C_Script::InspectorInputFloat(float* variablePtr, const char* ptrName)
{
	const char* name = typeid(*variablePtr).name();
	if (strcmp(name, "float"))
		return;

	std::string variableName = GetVariableName(ptrName);

	C_Script* script = App->scriptManager->actualScriptLoading;
	if (script != nullptr)
		script->inspectorVariables.push_back(InspectorScriptData(variableName, InspectorScriptData::DataType::FLOAT, variablePtr, InspectorScriptData::ShowMode::INPUT_FLOAT));
}

void C_Script::InspectorDragableFloat(float* variablePtr, const char* ptrName)
{
	const char* name = typeid(*variablePtr).name();
	if (strcmp(name, "float"))
		return;

	std::string variableName = GetVariableName(ptrName);

	C_Script* script = App->scriptManager->actualScriptLoading;
	if (script != nullptr)
		script->inspectorVariables.push_back(InspectorScriptData(variableName, InspectorScriptData::DataType::FLOAT, variablePtr, InspectorScriptData::ShowMode::DRAGABLE_FLOAT));
}

void C_Script::InspectorSliderFloat(float* variablePtr, const char* ptrName, const int& minValue, const int& maxValue)
{
	const char* name = typeid(*variablePtr).name();
	if (strcmp(name, "float"))
		return;

	std::string variableName = GetVariableName(ptrName);

	C_Script* script = App->scriptManager->actualScriptLoading;
	if (script != nullptr)
	{
		InspectorScriptData inspector(variableName, InspectorScriptData::DataType::FLOAT, variablePtr, InspectorScriptData::ShowMode::SLIDER_FLOAT);
		inspector.minSlider = minValue;
		inspector.maxSlider = maxValue;
		script->inspectorVariables.push_back(inspector);
	}
}

void C_Script::InspectorPrefab(Prefab* variablePtr, const char* ptrName)
{
	if (variablePtr != nullptr) {
		const char* name = typeid(*variablePtr).name();
		if (strcmp(name, "struct Prefab"))
			return;

		std::string variableName = GetVariableName(ptrName);

		C_Script* script = App->scriptManager->actualScriptLoading;
		if (script != nullptr) 
			script->inspectorVariables.push_back(InspectorScriptData(variableName, InspectorScriptData::DataType::PREFAB, (void*)variablePtr, InspectorScriptData::NONE));

	}
	else {
		LOG("Inspector type Prefab variable must not be a non-NULL Prefab pointer!!");
	}
}

void C_Script::InspectorGameObject(GameObject** variablePtr, const char* ptrName)
{
	const char* name = typeid(variablePtr).name();
	if (strcmp(name, "class GameObject * *"))
		return;

	std::string variableName = GetVariableName(ptrName);

	C_Script* script = App->scriptManager->actualScriptLoading;
	if (script != nullptr)
	{
		script->inspectorVariables.push_back(InspectorScriptData(variableName, InspectorScriptData::DataType::GAMEOBJECT, variablePtr, InspectorScriptData::NONE));
		//variable.obj = variablePtr;
		//script->inspectorVariables.push_back(variable);
	}

}
