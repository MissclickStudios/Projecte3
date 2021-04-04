#ifndef __C_SCRIPT_H__
#define __C_SCRIPT_H__

#include <string>
#include <vector>
#include "Component.h"

class ParsonNode;
class GameObject;
class R_Script;

struct NULL_API InspectorScriptData {
	friend class ComponentScript;
	enum DataType {
		INT, // DONE
		/*INT2,
		INT3,
		FLOAT, // DONE
		FLOAT2,
		FLOAT3,
		STRING,
		BOOL, // DONE
		PREFAB, // DONE
		GAMEOBJECT, // DONE*/
	};

	enum ShowMode {
		NONE,
		INPUT_INT,
		/*DRAGABLE_INT,
		SLIDER_INT,
		INPUT_FLOAT,
		DRAGABLE_FLOAT,
		SLIDER_FLOAT,
		CHECKBOX,*/
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

	/*GameObject** obj = nullptr;
private:
	//ugly
	float min_slider = 0;
	float max_slider = 0;*/
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

	bool HasData() const;

	static inline ComponentType GetType() { return ComponentType::SCRIPT; }			// This is needed to be able to use templates for functions such as GetComponent<>();

	//------------------Inspector----------------------------------------------------------------
	//To get the name of the variable for the inspector when finished with the macro expansion
	const std::vector<InspectorScriptData>& GetInspectorVariables()const;
	static std::string GetVariableName(const char* ptrName);

	static void InspectorInputInt(int* variablePtr, const char* ptrName);
	
	R_Script* resource = nullptr;
private:
	std::vector<InspectorScriptData> inspectorVariables;
	void* scriptData = nullptr;
	bool engineScript = false;
	std::string dataName;

	friend class M_EngineScriptManager;
};

#endif // !__C_SCRIPT_H__