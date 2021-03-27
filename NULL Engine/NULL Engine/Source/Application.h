#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <vector>
#include <string>

#include "UpdateStatus.h"
#include "Macros.h"
#include "Timer.h"
#include "PerfectTimer.h"
#include "HardwareInfo.h"

class Module;															// Modules are forward declared in call order.
class M_Window;
class M_Input;
class M_Scene;
class M_Renderer3D;
class M_Camera3D;
class M_FileSystem;
class M_ResourceManager;
class M_Audio;
class M_Physics;
class M_UISystem;
class M_ScriptManager;

enum class GameState
{
	PLAY,
	PAUSE,
	STEP,
	STOP
};

class NULL_API Application
{
public:
	Application();
	~Application();

	bool			Init	();
	bool			Start	();
	UpdateStatus	Update	();
	bool			CleanUp	();

	void			SaveConfiguration(const char* file);
	void			LoadConfiguration(const char* file);

private:																			// Exceptionally putting private first due to the special characteristics of Application.h and .cpp.
	void			PrepareUpdate	();
	UpdateStatus	PreUpdate		();
	UpdateStatus	DoUpdate		();
	UpdateStatus	PostUpdate		();
	void			FinishUpdate	();

	void			SaveConfigurationNow	();

public:																				// --- APPLICATION & ENGINE STATE
	void			AddModule				(Module* module);

	const char*		GetEngineName			() const;
	const char*		GetOrganizationName		() const;
	void			SetEngineName			(const char* engineName);
	void			SetOrganizationName		(const char* organizationName);

	void			EngineShortcuts			();

public:																				// --- FRAMERATE
	uint			GetFrameCap				() const;
	void			SetFrameCap				(uint frame);

public:																				// --- EDITOR
	void			AddEditorLog			(const char* log);
	void			RequestBrowser			(const char* link);

	void			LogHardwareInfo			() const;
	HardwareInfo	GetHardwareInfo			() const;

public:
	M_Window*				window;
	M_Input*				input;
	M_Scene*				scene;
	M_Renderer3D*			renderer;
	M_Camera3D*				camera;
	M_FileSystem*			fileSystem;
	M_ResourceManager*		resourceManager;
	M_Audio*				audio;
	M_Physics*				physics;
	M_UISystem*				uiSystem;
	M_ScriptManager*		scriptManager;

	GameState				gameState;

	bool					quit;													// Will keep track of whether or not the application has to close.
	bool					debug;													// Will keep track of whether or not the application is in debug mode.

	// --- SAVE & LOAD
	bool					userHasSaved;											// Will keep track of whether or not the user has saved the app's state at least once since start-up.

	// --- FRAMERATE
	uint					frameCap;												// Stores the cap value that will be aplied to the framerate.
	float					secondsSinceStartup;									// Stores the total amount of seconds that have elapsed since application start.
	bool					framesAreCapped;										// Will keep track of whether or not the frames are currently capped.
	bool					displayFramerateData;									// Will keep track of whether or not to display the framerate data on the window's title.		

private:
	std::vector<Module*>	modules;												// Vector that will contain all the modules. Will be iterated to go through each state of each module.

	std::string				engineName;												// Window title string. In this case the name of the Game Engine.
	std::string				organization;											// String that will store the name of the organization behind the development of this application.

	// --- LOAD & SAVE
	//ParsonNode				config;													// Interface class that will be used to modify .json files with the parson library.
	
	bool					wantToLoad;												// Will keep track of whether or not the user wants to load a configuration.
	bool					wantToSave;												// Will keep track of whether or not the user wants to save a configuration.

	// --- HARDWARE INFO
	HardwareInfo			hardwareInfo;											// All the info/data about the software and the hardware of the system will be extracted from here.
protected:
	Module* logger;
};

extern NULL_API Application* App;													// Allows to access the Application module from anywhere in the project.
Application* CreateApplication();																

#endif // !__APPLICATION_H__