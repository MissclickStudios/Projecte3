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
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	void LoadData(const char* name, bool engineScript);
	const std::string& GetDataName() const;

	//To get the name of the variable for the inspector when finished with the macro expansion
	static std::string GetVariableName(const char* ptrName);

	//Used mainly on the inspector or called by a script
	//void SetIsActive(bool setTo)override; //-> TODO: Here it might make calls to OnEnable and OnDisable beeing on engine mode???!!! Comented for now
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
	
	R_Script* resource = nullptr;
private:
	//std::vector<InspactorData> inspectorData;
	void* scriptData = nullptr;
	bool engineScript = false;
	std::string dataName; 
};

#endif // !__C_SCRIPT_H__