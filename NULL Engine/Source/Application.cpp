/*#include <functional>			//function pointers
#include <algorithm>			//for_each()
#include <memory>				//Smart pointers*/

#include "JSONParser.h"
#include "MC_Time.h"
#include "FrameData.h"
#include "Hourglass.h"
#include "Profiler.h"

#include "Module.h"
#include "M_Window.h"
#include "M_Input.h"
#include "M_Scene.h"
#include "M_Renderer3D.h"
#include "M_Camera3D.h"
#include "M_FileSystem.h"
#include "M_ResourceManager.h"
#include "M_Audio.h"
#include "M_Physics.h"
#include "M_UISystem.h"
#include "M_Detour.h"

#include "Application.h"

#include "MemoryManager.h"

Application* App = nullptr;

Application::Application() :
quit			(false),
debug			(false), 
hardwareInfo	(),
window			(new M_Window()),
input			(new M_Input()),
camera			(new M_Camera3D()),
renderer		(new M_Renderer3D()),
scene			(new M_Scene()),
fileSystem		(new M_FileSystem()),
resourceManager	(new M_ResourceManager()),
audio			(new M_Audio()),
physics			(new M_Physics()),
uiSystem		(new M_UISystem()),
scriptManager	(nullptr), //Very important -> fill the scriptManager on the user side application (Game or Engine) It is left null here to be allocated as a simple script manager or as an extended EngineScriptManager
detour			(new M_Detour()),
logger			(nullptr),
gameState		(GameState::STOP)
{
	App = this;

	// Save/Load variables
	wantToLoad				= false;
	wantToSave				= false;
	userHasSaved			= false;

	// Framerate variables
	frameCap				= 0;
	secondsSinceStartup		= 0.0f;
	framesAreCapped			= framesAreCapped;
	displayFramerateData	= false;

	//PERF_TIMER_PEEK(perf_timer);
}

Application::~Application()
{
	std::vector<Module*>::reverse_iterator item = modules.rbegin();
	
	while (item != modules.rend())
	{
		Module* module_to_delete = (*item);
		++item;

		RELEASE((*item));
	}
}

bool Application::Init()
{	
	LOG("Application Init --------------");

	bool ret = true;

	char* buffer = nullptr;
	uint size = fileSystem->Load(CONFIGURATION_FILE_PATH, &buffer);

	engineName			= TITLE;														// Change Later?
	organization		= ORGANIZATION;
	frameCap			= 60;
	framesAreCapped		= true;
	
	ParsonNode config(buffer);

	std::vector<Module*>::iterator item = modules.begin();

	while (item != modules.end() && ret)
	{
		ret = (*item)->Init(config.GetNode((*item)->GetName()));		// Constructs and gives every module a handle for their own configuration node. M_Input -> "Input".
		++item;
	}
	
	RELEASE_ARRAY(buffer);

	// Initializing hardware info and Logging it.
	hardwareInfo.InitializeInfo();
	LogHardwareInfo();
	// -----------------------------------------

	LOG("Application Start --------------");

	item = modules.begin();

	while (item != modules.end() && ret)								// Move to start()?
	{
		if ((*item)->IsActive())
		{
			ret = (*item)->Start();
		}

		++item;
	}

	MC_Time::Real::InitRealClock();
	MC_Time::Game::SetTimeScale(1.0f);

	//PERF_TIMER_PEEK(perf_timer);

	//ms_timer.Start();
	return ret;
}

bool Application::Start()													// IS IT NEEDED?
{
	// After all Init calls we call Start() in all modules
	LOG("Application Start --------------");

	bool success = true;

	//std::vector<Module*>::iterator item = modules.begin();
	//while (item != modules.end() && success)								// Move to start()?
	//{
	//	if ((*item)->IsActive())
	//	{
	//		success = (*item)->Start();
	//	}

	//	++item;
	//}
	
	return success;
}

// Call PreUpdate, Update and PostUpdate on all modules
UpdateStatus Application::Update()
{
	OPTICK_CATEGORY("Application Update",Optick::Category::Update)

	UpdateStatus ret = UpdateStatus::CONTINUE;
	
	if (quit)
	{
		return UpdateStatus::STOP;
	}

	PrepareUpdate();

	if (ret == UpdateStatus::CONTINUE)
	{
		ret = PreUpdate();
	}
	if (ret == UpdateStatus::CONTINUE)
	{
		ret = DoUpdate();
	}
	if (ret == UpdateStatus::CONTINUE)
	{
		ret = PostUpdate();
	}

	FinishUpdate();

	return ret;
}

bool Application::CleanUp()
{
	bool ret = true;

	SaveConfigurationNow();

	std::vector<Module*>::reverse_iterator item = modules.rbegin();

	while(item != modules.rend() && ret)
	{
		ret = (*item)->CleanUp();
		RELEASE((*item));

		++item;
	}

	modules.clear();

	hardwareInfo.CleanUp();

	return ret;
}

// ---------------------------------------------
void Application::PrepareUpdate()
{
	static bool stepped = false;
	
	MC_Time::Real::Update();

	if (stepped)																										// TMP. Dirty fix so Animations can be stepped. Clean Later.
	{
		gameState	= GameState::PAUSE;
		stepped		= false;
	}

	if (gameState == GameState::PLAY || gameState == GameState::STEP)
	{
		MC_Time::Game::Update();

		if (gameState == GameState::STEP)
		{
			stepped = true;
		}
	}
}

UpdateStatus Application::PreUpdate()
{
	UpdateStatus ret = UpdateStatus::CONTINUE;
	
	std::vector<Module*>::iterator item = modules.begin();

	while (item != modules.end() && ret == UpdateStatus::CONTINUE)
	{
		if ((*item)->IsActive())
		{
			ret = (*item)->PreUpdate(MC_Time::Game::GetDT());
		}

		++item;
	}

	if (ret == UpdateStatus::THROW_ERROR)
	{
		LOG("PreUpdate threw an ERROR at Module %s.", (*item)->GetName());
	}

	return ret;
}

UpdateStatus Application::DoUpdate()
{

	UpdateStatus ret = UpdateStatus::CONTINUE;

	std::vector<Module*>::iterator item = modules.begin();

	OPTICK_CATEGORY("Modules update", Optick::Category::Update)

	item = modules.begin();
	while (item != modules.end() && ret == UpdateStatus::CONTINUE)
	{
		if ((*item)->IsActive())
		{
			ret = (*item)->Update(MC_Time::Game::GetDT());
		}

		++item;
	}

	if (ret == UpdateStatus::THROW_ERROR)
	{
		LOG("Update threw an ERROR at Module %s.", (*item)->GetName());
	}

	return ret;
}

UpdateStatus Application::PostUpdate()
{
	OPTICK_CATEGORY("Application Post Update", Optick::Category::Update)
	
	UpdateStatus ret = UpdateStatus::CONTINUE;

	std::vector<Module*>::iterator item = modules.begin();
	
	item = modules.begin();
	while (item != modules.end() && ret == UpdateStatus::CONTINUE)
	{
		if ((*item)->IsActive())
		{
			//ret = (*item)->PostUpdate(dt);
			ret = (*item)->PostUpdate(MC_Time::Game::GetDT());
		}
		
		++item;
	}

	if (ret == UpdateStatus::THROW_ERROR)
	{
		LOG("PostUpdate threw an ERROR at Module %s.", (*item)->GetName());
	}

	return ret;
}

void Application::FinishUpdate()
{
	if (wantToLoad)
	{

		wantToLoad = false;
	}

	if (wantToSave)
	{
		SaveConfigurationNow();

		wantToSave = false;
	}

	if (framesAreCapped)
	{
		MC_Time::Real::DelayUntilFrameCap(frameCap);
	}

	if (displayFramerateData)
	{
		App->window->SetTitle("Go to the Time Management header in the Configuration Panel to see all the Framerate Data.");
	}
	else
	{
		App->window->SetTitle(engineName.c_str());
	}
}
// ---------------------------------------------

// --- SAVE & LOAD ENGINE CONFIGURATION ---
void Application::SaveConfiguration(const char* file)
{
	wantToSave = true;
}

void Application::LoadConfiguration(const char* file)
{
	wantToLoad = true;
}

void Application::SaveConfigurationNow()
{
	ParsonNode config;

	for (uint i = 0; i < modules.size(); ++i)
	{
		modules[i]->SaveConfiguration(config.SetNode(modules[i]->GetName()));
	}

	char* buffer = nullptr;
	uint written = config.SerializeToFile(CONFIGURATION_FILE_PATH, &buffer);
	if (written > 0)
	{
		LOG("[STATE] Application: Successfully Saved Engine Configuration! Path: %s");
	}
	else
	{
		LOG("[ERROR] Application: Could not Save Engine Configuration! Error: File System could not write the File.");
	}

	RELEASE_ARRAY(buffer);
}

// --- APPLICATION & ENGINE STATE ---
void Application::AddModule(Module* module)
{
	modules.push_back(module);
}

const char* Application::GetEngineName() const
{
	return engineName.c_str();
}

const char* Application::GetOrganizationName() const
{
	return organization.c_str();
}

void Application::SetEngineName(const char* name)
{
	engineName = name;

	App->window->SetTitle(engineName.c_str());
}

void Application::SetOrganizationName(const char* name)
{
	organization = name;
}

void Application::EngineShortcuts()
{

}

// --- FRAMERATE METHODS ---
uint Application::GetFrameCap() const
{
	return frameCap;
}

void Application::SetFrameCap(uint newCap)
{
	frameCap = newCap;
}

// --- EDITOR METHODS ---
void Application::AddEditorLog(const char* log)
{
	if (!quit && logger != nullptr)													// Second condition is not really necessary. It's more of a reminder to keep it in mind.
	{
		//std::string full_log = App->fileSystem->NormalizePath(log);				// Switching all "\\" for "/". They need to be changed due to "\" being a Windows-specific thing.

		std::string fullLog = log;													// TMP. Switch to normalize later.

		uint logStart	= fullLog.find_last_of("\\") + 1;							// Gets the position of the last "\" in the log string.
		uint logEnd	= fullLog.size();											// The last position of the log will be equal to the size of it.

		std::string shortLog = fullLog.substr(logStart, logEnd);				// Returns the string that is within the given positions.

		logger->AddConsoleLog(shortLog.c_str());									// Priorized readability over reducing to AddConsoleLog(full_log.substr(log_start, log_end)).
	}
}

void Application::RequestBrowser(const char* link)
{
	ShellExecuteA(NULL, "open", link, NULL, "", 0);
}


void Application::LogHardwareInfo() const
{
	LOG(" ------------- CPU INFO ------------- ");
	LOG("CPU Cores: %d",						hardwareInfo.CPU.cpuCount);
	LOG("CPU Cache Size: %d ",					hardwareInfo.CPU.cacheSize);
	LOG("CPU RAM Size: %.1f GB",				hardwareInfo.CPU.ramGb);
	LOG(" --- DRIVERS --- ");
	LOG("CPU RDTSC: %s",						hardwareInfo.CPU.hasRDTSC	?	"True" : "False");
	LOG("CPU AltiVec: %s",						hardwareInfo.CPU.hasAltiVec	?	"True" : "False");
	LOG("CPU Now3D: %s",						hardwareInfo.CPU.has3DNow	?	"True" : "False");
	LOG("CPU MMX: %s",							hardwareInfo.CPU.hasMMX		?	"True" : "False");
	LOG("CPU SSE: %s",							hardwareInfo.CPU.hasSSE		?	"True" : "False");
	LOG("CPU SSE2: %s",							hardwareInfo.CPU.hasSSE2	?	"True" : "False");
	LOG("CPU SSE3: %s",							hardwareInfo.CPU.hasSSE3	?	"True" : "False");
	LOG("CPU SSE4.1: %s",						hardwareInfo.CPU.hasSSE41	?	"True" : "False");
	LOG("CPU SSE4.2: %s",						hardwareInfo.CPU.hasSSE42	?	"True" : "False");
	LOG("CPU AVX: %s",							hardwareInfo.CPU.hasAVX		?	"True" : "False");
	LOG("CPU AVX2: %s",							hardwareInfo.CPU.hasAVX2	?	"True" : "False");

	LOG(" ------------- GPU INFO ------------- ");
	LOG("GPU Vendor %d Device %d",				hardwareInfo.GPU.vendor, hardwareInfo.GPU.deviceId);
	LOG("GPU Brand: %s",						hardwareInfo.GPU.brand);
	LOG(" --- VRAM --- ");
	LOG("GPU VRAM Budget: %.1f MB",				hardwareInfo.GPU.vramBudget);
	LOG("GPU VRAM Usage: %.1f MB",				hardwareInfo.GPU.vramUsage);
	LOG("GPU VRAM Available: %.1f MB",			hardwareInfo.GPU.vramAvailable);
	LOG("GPU VRAM Reserved: %.1f MB",			hardwareInfo.GPU.vramReserved);

	LOG(" ------------- SDL INFO ------------- ");
	LOG("SDL Version: %s",						hardwareInfo.SDL.SDLVersion);

	LOG(" ------------- OPENGL INFO ------------- ");
	LOG("OpenGL Model: %s",						hardwareInfo.OpenGL.modelName);
	LOG("OpenGL Renderer: %s",					hardwareInfo.OpenGL.rendererName);
	LOG("OpenGL Version: %s",					hardwareInfo.OpenGL.version);
	LOG("OpenGL Shading Language Version: %s",	hardwareInfo.OpenGL.shadingLanguageVersion);
}

HardwareInfo Application::GetHardwareInfo() const
{
	return hardwareInfo;
}