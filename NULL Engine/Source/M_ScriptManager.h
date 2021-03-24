#ifndef __SCRIPTS_H__
#define __SCRIPTS_H__

#include <vector>
#include <map>
#include <Windows.h>
#include "Module.h"

typedef unsigned __int64 uint64;
class Script;
class C_Script;
//class GameObject;
class ParsonArray;

class NULL_API M_ScriptManager : public Module
{
public:

	M_ScriptManager(bool isActive = true);
	~M_ScriptManager();

	//bool		Init(ParsonNode& root) override;
	bool Start() override;
	void InitScripts();
	UpdateStatus PreUpdate(float dt) override;
	UpdateStatus Update(float dt) override;
	UpdateStatus PostUpdate(float dt) override;
	void CleanUpScripts();
	bool CleanUp() override;

	void AddCurrentEngineScript(Script* script);
	HINSTANCE GetDllHandle() const;

#ifndef GAMEBUILD //TODO: Aixo s'ha de fer en un modul del NULL ENGINE!!!
public:
	const std::map<std::string, std::string>& GetAviableScripts() const { return aviableScripts; }
private:

	void HotReload();
	std::map<std::string, std::string> aviableScripts;
#endif
private://TODO: Treure el que no es necessiti en el game
	void SerializeAllScripts(ParsonArray& scriptsArray);
	void DeSerializeAllScripts(ParsonArray& scriptsArray);

public:
	C_Script* actualScriptLoading;
	//TODO: Maybe not needed on gameplay
	//To know if we have reloaded scripts this frame

private:
	HINSTANCE dllHandle;
	uint64 lastModDll;
	std::vector<Script*>currentScripts;

	friend class C_Script;
};

#endif //__SCRIPTS_H__