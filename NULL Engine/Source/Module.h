// ----------------------------------------------------
// Module.h
// Interface for all engine modules.
// All module file names will start with "M_"
// ----------------------------------------------------

#ifndef __MODULE_H__
#define __MODULE_H__

#include <vector>
#include <map>
#include <string>

#include "SDL/include/SDL.h"														// TMP. Delete later

#include "Globals.h"																// TMP. Delete later

class ParsonNode;

#define MAX_MODULE_NAME_LENGTH 25

class Module
{
public:
	Module(const char* name, bool is_active = true);
	virtual ~Module();

	virtual bool			Init				(ParsonNode& config);				// Called immediately after the constructor.
	virtual bool			Start				();									// Called before the first frame.
	virtual UPDATE_STATUS	PreUpdate			(float dt);							// Called every frame.
	virtual UPDATE_STATUS	Update				(float dt);							// Called every frame.
	virtual UPDATE_STATUS	PostUpdate			(float dt);							// Called every frame.
	virtual bool			CleanUp				();									// Called after the last frame.

	virtual bool			SaveConfiguration	(ParsonNode& root) const;			// Will save the current configuration of the calling module in a JSON file.
	virtual bool			LoadConfiguration	(ParsonNode& root);					// Will load the configuration of the calling module from a JSON file.

public:
	bool					IsActive			() const;
	bool					SetModuleState		(bool is_active);					// Will modify the state of the module. Will call Start() or CleanUp().

	const char*				GetName				() const;							// Will return the name of the module.

private:
	bool is_active;																	// State in which the module is currently in. If it is active the module will be looped each frame.
	char name[MAX_MODULE_NAME_LENGTH];												// The module name string will be mainly used for debug/LOG purposes.
};

#endif //  !__MODULE_H__