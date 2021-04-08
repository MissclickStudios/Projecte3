#ifndef __C_SCRIPT_H__
#define __C_SCRIPT_H__

#include <string>
#include <vector>
#include "Component.h"

class ParsonNode;
class GameObject;
class R_Script;
class Prefab;

struct NULL_API InspectorScriptData {
	enum DataType {
		INT,
		BOOL,
		FLOAT,
		PREFAB,
		GAMEOBJECT
		/*INT2,
		INT3,
		FLOAT2,
		FLOAT3,
		STRING,
		PREFAB, // DONE
		GAMEOBJECT, // DONE*/
	};

	enum ShowMode {
		NONE,
		INPUT_INT,
		DRAGABLE_INT,
		SLIDER_INT,
		CHECKBOX,
		INPUT_FLOAT,
		DRAGABLE_FLOAT,
		SLIDER_FLOAT
	};

	InspectorScriptData(const std::string& variableName, const DataType& variableType, void* ptr, const ShowMode& mode) {
		this->variableName = variableName;
		this->variableType = variableType;
		this->ptr = ptr;
		this->showAs = mode;
	}

	std::string variableName;
	DataType variableType;
	ShowMode showAs;
	void* ptr = nullptr;

	GameObject** obj = nullptr;
	float minSlider = 0;
	float maxSlider = 0;
};

class NULL_API C_Script : public Component
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
	void OnCollisionEnter();
	void OnCollisionRepeat();
	void OnCollisionExit();
	void OnTriggerEnter();
	void OnTriggerRepeat();
	void OnTriggerExit();

	bool HasData() const;

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

	static void InspectorPrefab(Prefab* variablePtr, const char* ptrName);

	static void InspectorGameObject(GameObject** variablePtr, const char* ptrName);
	
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