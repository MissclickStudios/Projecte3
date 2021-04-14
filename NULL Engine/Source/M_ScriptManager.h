#ifndef __SCRIPTS_H__
#define __SCRIPTS_H__

#include <vector>
#include <map>
#include <Windows.h>
#include "Module.h"

typedef unsigned __int64 uint64;
class Script;
class C_Script;

class NULL_API M_ScriptManager : public Module
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

public:
	C_Script* actualScriptLoading;
	//TODO: Maybe not needed on gameplay
	//To know if we have reloaded scripts this frame

protected:
	HINSTANCE dllHandle;
	std::vector<Script*>currentScripts;

	friend class C_Script;
};

#endif //__SCRIPTS_H__