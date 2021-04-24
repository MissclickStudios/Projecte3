#pragma once
#include <string>
#include <vector>
#include "Script.h"
#include "ScriptMacros.h"

#include "MathGeoLib/include/Math/float3.h"

#include "Prefab.h"

class GameObject;

enum class ENGINE_ENUM TestEnum
{
	FIRST_ELEMENT,
	SECOND_ELEMENT,
	THIRD_ELEMENT = 32,
};

enum class ENGINE_ENUM TestEnum2
{
	a, b = 12, c, d
};

class SCRIPTS_API FirstScript : public Script {
public :
	FirstScript();
	~FirstScript();
	void Awake() override;
	void Start() override;
	void PreUpdate()override;
	void Update() override;
	void PostUpdate()override;
	void CleanUp()override;
	int firstVariable = 1;
	int inputInt = 1;
	int dragableInt = 2;
	int sliderInt = 3;
	bool checkBoxBool = false;
	float inputFloat = 1.1f;
	float dragFloat = 2.2f;
	float sliderFloat = 3.3f;
	float3 tripleFloat = { 1.1f,2.2f,3.3f };
	std::string string;
	Prefab tryPrefab;
	Prefab prefab2;
	GameObject* object = nullptr;
	std::vector<std::string> stringVec;
	TestEnum testingEnums;
	TestEnum2 secondEnum = TestEnum2::c;
};

SCRIPTS_FUNCTION FirstScript* CreateFirstScript() {
	FirstScript* script = new FirstScript();
	INSPECTOR_INPUT_INT(script->firstVariable);
	INSPECTOR_INPUT_INT(script->inputInt);
	INSPECTOR_DRAGABLE_INT(script->dragableInt);
	INSPECTOR_SLIDER_INT(script->sliderInt, 0, 100);
	INSPECTOR_CHECKBOX_BOOL(script->checkBoxBool);
	INSPECTOR_INPUT_FLOAT(script->inputFloat);
	INSPECTOR_DRAGABLE_FLOAT(script->dragFloat);
	INSPECTOR_DRAGABLE_FLOAT3(script->tripleFloat);
	INSPECTOR_STRING(script->string);
	INSPECTOR_SLIDER_FLOAT(script->sliderFloat,0.0,5.5f);
	INSPECTOR_PREFAB(script->tryPrefab);
	INSPECTOR_PREFAB(script->prefab2);
	INSPECTOR_GAMEOBJECT(script->object);
	INSPECTOR_VECTOR_STRING(script->stringVec);
	INSPECTOR_ENUM(script->secondEnum, "TestEnum2", "Assets/Scripts/FirstScript.h");
	INSPECTOR_ENUM(script->testingEnums, "TestEnum", "Assets/Scripts/FirstScript.h");
	return script;
}