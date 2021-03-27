// ----------------------------------------------------
// Module.h
// Interface for all engine modules.
// All module file names will start with "M_"
// ----------------------------------------------------

#ifndef __MODULE_H__
#define __MODULE_H__

#include "SDL/include/SDL.h"														// TMP. Delete later

//#include "Globals.h"																// TMP. Delete later
#include "UpdateStatus.h"
#include "Macros.h"

class ParsonNode;

#define MAX_MODULE_NAME_LENGTH 25

class NULL_API Module
{
public:
	Module(const char* name, bool isActive = true);
	virtual ~Module();

	virtual bool			Init				(ParsonNode& config);				// Called immediately after the constructor.
	virtual bool			Start				();									// Called before the first frame.
	virtual UpdateStatus	PreUpdate			(float dt);							// Called every frame.
	virtual UpdateStatus	Update				(float dt);							// Called every frame.
	virtual UpdateStatus	PostUpdate			(float dt);							// Called every frame.
	virtual bool			CleanUp				();									// Called after the last frame.

	virtual bool			SaveConfiguration	(ParsonNode& root) const;			// Will save the current configuration of the calling module in a JSON file.
	virtual bool			LoadConfiguration	(ParsonNode& root);					// Will load the configuration of the calling module from a JSON file.

	//Support for eventual modules (different in engine and game builds) -------------------------------------------------------------------------------------------

	virtual void			ProcessInput(SDL_Event& event);							//Quick solution to let the engine editor get input for ImGui to process it
	virtual void			PostSceneRendering();									//Quick solution to let the engine editor render ImGui after the render of the scene
	virtual void			AddConsoleLog(const char* log);							//Quick solution to have an engine Logger outside the core Dll (just needed in engine executable)

public:
	bool					IsActive			() const;
	bool					SetModuleState		(bool isActive);					// Will modify the state of the module. Will call Start() or CleanUp().

	const char*				GetName				() const;							// Will return the name of the module.

private:
	bool isActive;																	// State in which the module is currently in. If it is active the module will be looped each frame.
	char name[MAX_MODULE_NAME_LENGTH];												// The module name string will be mainly used for debug/LOG purposes.
};

#endif //  !__MODULE_H__