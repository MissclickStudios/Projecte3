#ifndef __ESCRIPTS_H__
#define __ESCRIPTS_H__

#include <vector>
#include <map>
#include <Windows.h>
#include "Module.h"
#include "M_ScriptManager.h"

typedef unsigned __int64 uint64;
class Script;
class C_Script;
class ParsonArray;

class M_EngineScriptManager : public M_ScriptManager
{
public:

	M_EngineScriptManager(bool isActive = true);
	~M_EngineScriptManager();

	//bool		Init(ParsonNode& root) override;
	bool Start() override;
	void InitScripts()override;
	UpdateStatus PreUpdate(float dt) override;
	UpdateStatus Update(float dt) override;
	UpdateStatus PostUpdate(float dt) override;
	void CleanUpScripts()override;
	bool CleanUp() override;;

	const std::map<std::string, std::string>& GetAviableScripts() const { return aviableScripts; }

private:
	void HotReload();
	void SerializeAllScripts(ParsonArray& scriptsArray);
	void DeSerializeAllScripts(const ParsonArray& scriptsArray);


private:
	std::map<std::string, std::string> aviableScripts;
	uint64 lastModDll;
	friend class C_Script;
};

#endif //__ESCRIPTS_H__