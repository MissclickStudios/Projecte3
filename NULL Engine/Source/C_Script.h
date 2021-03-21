#ifndef __C_SCRIPT_H__
#define __C_SCRIPT_H__

#include <string>
#include <vector>
#include "Component.h"

class ParsonNode;
class GameObject;
class R_Script;

class NULL_API C_Script : public Component
{
public:
	C_Script(GameObject* owner);
	virtual ~C_Script();

	//bool Update() override;
	//bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	void LoadData(const char* name, bool engineScript);
	const std::string& GetDataName() const;

	//To get the name of the variable for the inspector when finished with the macro expansion
	static std::string GetVariableName(const char* ptrName);

	//Used mainly on the inspector
	void OnDisable(); 
	void OnEnable();

	static inline ComponentType GetType() { return ComponentType::SCRIPT; }			// This is needed to be able to use templates for functions such as GetComponent<>();

private:
	//std::vector<InspactorData> inspectorData;
	uint32 resourceUID = 0;
	void* scriptData = nullptr;
	bool engineScript = false;
	std::string dataName;
};

#endif // !__C_SCRIPT_H__