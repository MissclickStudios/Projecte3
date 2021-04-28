#ifndef __SCRIPTS_H__
#define __SCRIPTS_H__

#include <vector>
#include <map>
#include <Windows.h>
#include "Module.h"

typedef unsigned __int64 uint64;
class Script;
class C_Script;

class MISSCLICK_API M_ScriptManager : public Module
{
public:

	M_ScriptManager(bool isActive = true);
	virtual ~M_ScriptManager();

	//bool		Init(ParsonNode& root) override;
	virtual bool Start() override;
	virtual void InitScripts();
	virtual UpdateStatus PreUpdate(float dt) override;
	virtual UpdateStatus Update(float dt) override;
	virtual UpdateStatus PostUpdate(float dt) override;
	virtual void CleanUpScripts();
	virtual bool CleanUp() override;

	HINSTANCE GetDllHandle() const;

	virtual bool ParseEnum(const char* enumName, const char* definitionFile);

public:
	C_Script* actualScriptLoading;
	//Scripts Dll vector Helpers
	void (*StringVecPushBackString)(void*,const std::string&) = nullptr;
	void (*StringVecPushBackChar)(void*,const char*) = nullptr;
	void (*StringVecEmplaceBackChar)(void*,const char*) = nullptr;
	void (*StringVecEmplaceBackString)(void*,const std::string&) = nullptr;
	void (*StringVecReserve)(void*, int) = nullptr;
	void (*StringVecErase)(void*, int) = nullptr;

protected:
	HINSTANCE dllHandle;
	std::vector<Script*>currentScripts;
	void ResolveScriptHelperFunctions();

	friend class C_Script;
};

#endif //__SCRIPTS_H__