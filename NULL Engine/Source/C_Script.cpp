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
#include "M_Scene.h"
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
			case InspectorScriptData::FLOAT3:
				variable.SetNumber("float3x", (*(float3*)inspectorVariables[i].ptr).x);
				variable.SetNumber("float3y", (*(float3*)inspectorVariables[i].ptr).y);
				variable.SetNumber("float3z", (*(float3*)inspectorVariables[i].ptr).z);
				break;
			case InspectorScriptData::STRING:
				variable.SetString("string", (*(std::string*)inspectorVariables[i].ptr).c_str()); break;
			case InspectorScriptData::PREFAB:
				variable.SetInteger("prefab", (*(Prefab*)inspectorVariables[i].ptr).uid); break;
			case InspectorScriptData::GAMEOBJECT:
				if (inspectorVariables[i].obj != nullptr && *inspectorVariables[i].obj != nullptr)
					variable.SetInteger("gameobject", (*inspectorVariables[i].obj)->GetUID());
				else
					variable.SetInteger("gameobject", 0);
				break;
			case InspectorScriptData::VECTORSTRING: 
			{
				ParsonArray parsonStringArray = variable.SetArray("vectorstring");
				for (std::vector<std::string>::const_iterator cit = (*(std::vector<std::string>*)inspectorVariables[i].ptr).cbegin(); cit != (*(std::vector<std::string>*)inspectorVariables[i].ptr).cend(); ++cit) 
				{
					parsonStringArray.SetString((*cit).c_str());
				}
				break; 
			}
			case InspectorScriptData::ENUM:
				variable.SetInteger("enum", *(int*)inspectorVariables[i].ptr); break;
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

		if (!inspectorVariables.empty() && root.GetBool("HasInspector"))
		{
			ParsonArray variablesToLoad = root.GetArray("InspectorVariables");
			for (int i = 0; i < inspectorVariables.size(); ++i) 
			{
				for (int j = 0; j < variablesToLoad.size; ++j)
				{
					ParsonNode variable = variablesToLoad.GetNode(j);
					if (inspectorVariables[i].variableType == (InspectorScriptData::DataType)variable.GetInteger("Type") && !strcmp(inspectorVariables[i].variableName.c_str(), variable.GetString("Name")))
					{
						switch (inspectorVariables[i].variableType)
						{
						case InspectorScriptData::DataType::INT:
							*(int*)inspectorVariables[i].ptr = variable.GetInteger("int"); break;
						case InspectorScriptData::BOOL:
							*(bool*)inspectorVariables[i].ptr = variable.GetBool("bool"); break;
						case InspectorScriptData::DataType::FLOAT:
							*(float*)inspectorVariables[i].ptr = variable.GetNumber("float"); break;
						case InspectorScriptData::FLOAT3:
							(*(float3*)inspectorVariables[i].ptr).x = variable.GetNumber("float3x");
							(*(float3*)inspectorVariables[i].ptr).y = variable.GetNumber("float3y");
							(*(float3*)inspectorVariables[i].ptr).z = variable.GetNumber("float3z");
							break;
						case InspectorScriptData::STRING:
							*(std::string*)inspectorVariables[i].ptr = variable.GetString("string"); break;
						case InspectorScriptData::DataType::PREFAB:
							*(Prefab*)inspectorVariables[i].ptr = App->resourceManager->prefabs[(unsigned int)variable.GetInteger("prefab")]; break;
						case InspectorScriptData::DataType::GAMEOBJECT:  //TODO: FINISH THIS !!!!
						{
							uint32 id = variable.GetInteger("gameobject");
							if (id != 0)
								App->scene->ResolveScriptGoPointer(id, inspectorVariables[i].obj);
							break; 
						}
						case InspectorScriptData::VECTORSTRING:
						{
							//std::vector<std::string>& inspectorStringVector = *(std::vector<std::string>*)(*item).ptr;
							ParsonArray parsonStringArray = variable.GetArray("vectorstring");
							//inspectorStringVector.reserve(parsonStringArray.size);
							App->scriptManager->StringVecReserve(inspectorVariables[i].ptr, parsonStringArray.size);
							for (int k = 0; k < parsonStringArray.size; ++k)
							{
								//inspectorStringVector.emplace_back(parsonStringArray.GetString(i));
								App->scriptManager->StringVecEmplaceBackString(inspectorVariables[i].ptr, parsonStringArray.GetString(k));
							}
							break;
						}
						case InspectorScriptData::DataType::ENUM:
							*(int*)inspectorVariables[i].ptr = variable.GetInteger("enum"); break;
						}
					}
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
			script->transform = script->gameObject->transform;
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

void C_Script::OnCollisionEnter(GameObject* object)
{
	if (engineScript && scriptData != nullptr) {
#ifndef GAMEBUILD
		try {
			Script* script = (Script*)scriptData;
			script->OnCollisionEnter(object);
		}
		catch (...)
		{
			LOG("CODE ERROR IN THE ONDISABLE OF THE SCRIPT: %s", dataName.data());
		}
#else
		((Script*)scriptData)->OnCollisionEnter(object);
#endif // !GAMEBUILD
	}
}

void C_Script::OnCollisionRepeat(GameObject* object)
{
	if (engineScript && scriptData != nullptr) {
#ifndef GAMEBUILD
		try {
			Script* script = (Script*)scriptData;
			script->OnCollisionRepeat(object);
		}
		catch (...)
		{
			LOG("CODE ERROR IN THE ONDISABLE OF THE SCRIPT: %s", dataName.data());
		}
#else
		((Script*)scriptData)->OnCollisionRepeat(object);
#endif // !GAMEBUILD
	}
}

void C_Script::OnCollisionExit(GameObject* object)
{
	if (engineScript && scriptData != nullptr) {
#ifndef GAMEBUILD
		try {
			Script* script = (Script*)scriptData;
			script->OnCollisionExit(object);
		}
		catch (...)
		{
			LOG("CODE ERROR IN THE ONDISABLE OF THE SCRIPT: %s", dataName.data());
		}
#else
		((Script*)scriptData)->OnCollisionExit(object);
#endif // !GAMEBUILD
	}
}

void C_Script::OnTriggerEnter(GameObject* object)
{
	if (engineScript && scriptData != nullptr) {
#ifndef GAMEBUILD
		try {
			Script* script = (Script*)scriptData;
			script->OnTriggerEnter(object);
		}
		catch (...)
		{
			LOG("CODE ERROR IN THE ONDISABLE OF THE SCRIPT: %s", dataName.data());
		}
#else
		((Script*)scriptData)->OnTriggerEnter(object);
#endif // !GAMEBUILD
	}
}

void C_Script::OnTriggerRepeat(GameObject* object)
{
	if (engineScript && scriptData != nullptr) {
#ifndef GAMEBUILD
		try {
			Script* script = (Script*)scriptData;
			script->OnTriggerRepeat(object);
		}
		catch (...)
		{
			LOG("CODE ERROR IN THE ONDISABLE OF THE SCRIPT: %s", dataName.data());
		}
#else
		((Script*)scriptData)->OnTriggerRepeat(object);
#endif // !GAMEBUILD
	}
}

void C_Script::OnTriggerExit(GameObject* object)
{
	if (engineScript && scriptData != nullptr) {
#ifndef GAMEBUILD
		try {
			Script* script = (Script*)scriptData;
			script->OnTriggerExit(object);
		}
		catch (...)
		{
			LOG("CODE ERROR IN THE ONDISABLE OF THE SCRIPT: %s", dataName.data());
		}
#else
		((Script*)scriptData)->OnTriggerExit(object);
#endif // !GAMEBUILD
	}
}

bool C_Script::HasData() const
{
	return scriptData != nullptr;
}

//------------------------INSPECTOR VARIABLES------------------------------------------
void C_Script::InspectorInputInt(int* variablePtr, const char* ptrName)
{
	//TODO: Check kind a useless?!
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

void C_Script::InspectorInputFloat3(float3* variablePtr, const char* ptrName)
{
	const char* name = typeid(*variablePtr).name();
	if (strcmp(name, "class math::float3"))
		return;

	std::string variableName = GetVariableName(ptrName);

	C_Script* script = App->scriptManager->actualScriptLoading;
	if (script != nullptr)
		script->inspectorVariables.push_back(InspectorScriptData(variableName, InspectorScriptData::DataType::FLOAT3, variablePtr, InspectorScriptData::ShowMode::INPUT_FLOAT));
}

void C_Script::InspectorDragableFloat3(float3* variablePtr, const char* ptrName)
{
	const char* name = typeid(*variablePtr).name();
	if (strcmp(name, "class math::float3"))
		return;

	std::string variableName = GetVariableName(ptrName);

	C_Script* script = App->scriptManager->actualScriptLoading;
	if (script != nullptr)
		script->inspectorVariables.push_back(InspectorScriptData(variableName, InspectorScriptData::DataType::FLOAT3, variablePtr, InspectorScriptData::ShowMode::DRAGABLE_FLOAT));
}

void C_Script::InspectorSliderFloat3(float3* variablePtr, const char* ptrName, const int& minValue, const int& maxValue)
{
	const char* name = typeid(*variablePtr).name();
	if (strcmp(name, "class math::float3"))
		return;

	std::string variableName = GetVariableName(ptrName);

	C_Script* script = App->scriptManager->actualScriptLoading;
	if (script != nullptr)
	{
		InspectorScriptData inspector(variableName, InspectorScriptData::DataType::FLOAT3, variablePtr, InspectorScriptData::ShowMode::SLIDER_FLOAT);
		inspector.minSlider = minValue;
		inspector.maxSlider = maxValue;
		script->inspectorVariables.push_back(inspector);
	}
}

void C_Script::InspectorString(std::string* variablePtr, const char* ptrName)
{
	std::string variableName = GetVariableName(ptrName);

	C_Script* script = App->scriptManager->actualScriptLoading;
	if (script != nullptr)
		script->inspectorVariables.push_back(InspectorScriptData(variableName, InspectorScriptData::DataType::STRING, variablePtr, InspectorScriptData::ShowMode::NONE));
}

void C_Script::InspectorText(std::string* variablePtr, const char* ptrName)
{
	std::string variableName = GetVariableName(ptrName);

	C_Script* script = App->scriptManager->actualScriptLoading;
	if (script != nullptr)
		script->inspectorVariables.push_back(InspectorScriptData(variableName, InspectorScriptData::DataType::STRING, variablePtr, InspectorScriptData::ShowMode::TEXT));

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
		InspectorScriptData variable(variableName, InspectorScriptData::DataType::GAMEOBJECT, nullptr, InspectorScriptData::NONE);
		variable.obj = variablePtr;
		script->inspectorVariables.push_back(variable);
	}

}

void C_Script::InspectorStringVector(std::vector<std::string>* variablePtr, const char* ptrName)
{
	std::string variableName = GetVariableName(ptrName);

	C_Script* script = App->scriptManager->actualScriptLoading;
	if (script != nullptr)
		script->inspectorVariables.push_back(InspectorScriptData(variableName, InspectorScriptData::DataType::VECTORSTRING, variablePtr, InspectorScriptData::ShowMode::NONE));
}

void C_Script::InspectorEnum(void* variablePtr, const char* ptrName, const char* enumName, const char* definitionFile)
{
	std::string variableName = GetVariableName(ptrName);
	if (App->scriptManager->ParseEnum(enumName,definitionFile))
	{
		C_Script* script = App->scriptManager->actualScriptLoading;
		if (script != nullptr) {
			InspectorScriptData variable(variableName, InspectorScriptData::DataType::ENUM, variablePtr, InspectorScriptData::NONE);
			variable.enumName = enumName;
			script->inspectorVariables.push_back(variable);
		}
	}
}
