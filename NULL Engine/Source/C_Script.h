#ifndef __C_SCRIPT_H__
#define __C_SCRIPT_H__

#include <string>
#include <vector>
#include "Component.h"
#include "MathGeoLib/include/Math/float3.h"

class ParsonNode;
class GameObject;
class R_Script;
class Prefab;

struct MISSCLICK_API InspectorScriptData {
	enum DataType {
		INT,
		BOOL,
		FLOAT,
		FLOAT3,
		PREFAB,
		STRING,
		GAMEOBJECT,
		VECTORSTRING,
		ENUM
	};

	enum ShowMode {
		NONE,
		INPUT_INT,
		DRAGABLE_INT,
		SLIDER_INT,
		CHECKBOX,
		INPUT_FLOAT,
		DRAGABLE_FLOAT,
		SLIDER_FLOAT,
		TEXT
	};

	InspectorScriptData(const std::string& variableName, const DataType& variableType, void* ptr, const ShowMode& mode):
	variableName(variableName), variableType(variableType), showAs(mode), ptr(ptr){}

	std::string variableName;
	DataType variableType;
	ShowMode showAs;
	void* ptr = nullptr;

	//ugly
	std::string enumName;
	GameObject** obj = nullptr;
	float minSlider = 0;
	float maxSlider = 0;
};

class MISSCLICK_API C_Script : public Component
{
public:
	C_Script(GameObject* owner);
	virtual ~C_Script();

	//bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	void LoadData(const char* name, bool engineScript);
	const std::string& GetDataName() const;

	//Used mainly on the inspector or called by a script
	void SetIsActive(bool setTo)override; //-> TODO: Here it might make calls to OnEnable and OnDisable beeing on engine mode???!!! Comented for now
	void OnDisable(); 
	void OnEnable();

	//Physics callbacks
	void OnCollisionEnter(GameObject* object);
	void OnCollisionRepeat(GameObject* object);
	void OnCollisionExit(GameObject* object);
	void OnTriggerEnter(GameObject* object);
	void OnTriggerRepeat(GameObject* object);
	void OnTriggerExit(GameObject* object);

	bool HasData() const;
	void* GetScriptData() { return scriptData; } //TEMPORARY

	static inline ComponentType GetType() { return ComponentType::SCRIPT; }			// This is needed to be able to use templates for functions such as GetComponent<>();

	//------------------Inspector----------------------------------------------------------------
	//To get the name of the variable for the inspector when finished with the macro expansion
	const std::vector<InspectorScriptData>& GetInspectorVariables()const;
	static std::string GetVariableName(const char* ptrName);

	static void InspectorInputInt(int* variablePtr, const char* ptrName);
	static void InspectorDragableInt(int* variablePtr, const char* ptrName);
	static void InspectorSliderInt(int* variablePtr, const char* ptrName, const int& minValue, const int& maxValue);

	static void InspectorBool(bool* variablePtr, const char* ptrName);

	static void InspectorInputFloat(float* variablePtr, const char* ptrName);
	static void InspectorDragableFloat(float* variablePtr, const char* ptrName);
	static void InspectorSliderFloat(float* variablePtr, const char* ptrName, const int& minValue, const int& maxValue);

	static void InspectorInputFloat3(float3* variablePtr, const char* ptrName);
	static void InspectorDragableFloat3(float3* variablePtr, const char* ptrName);
	static void InspectorSliderFloat3(float3* variablePtr, const char* ptrName, const int& minValue, const int& maxValue);

	static void InspectorString(std::string* variablePtr, const char* ptrName);
	static void InspectorText(std::string* variablePtr, const char* ptrName);

	static void InspectorPrefab(Prefab* variablePtr, const char* ptrName);

	static void InspectorGameObject(GameObject** variablePtr, const char* ptrName);

	static void InspectorStringVector(std::vector<std::string>* variablePtr, const char* ptrName);

	static void InspectorEnum(void* variablePtr, const char* ptrName, const char* enumName, const char* definitionFile);
	
	R_Script* resource = nullptr;
private:
	std::vector<InspectorScriptData> inspectorVariables;
	void* scriptData = nullptr;
	bool engineScript = false;
	std::string dataName;

	friend class M_EngineScriptManager;
	friend class Player;
};

#endif // !__C_SCRIPT_H__