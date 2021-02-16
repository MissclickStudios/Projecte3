#include <functional>			//function pointers
#include <algorithm>			//for_each()
#include <memory>				//Smart pointers

#include "JSONParser.h"
#include "Time.h"
#include "FrameData.h"
#include "Hourglass.h"

#include "Module.h"
#include "M_Window.h"
#include "M_Input.h"
#include "M_Scene.h"
#include "M_Renderer3D.h"
#include "M_Editor.h"
#include "M_Camera3D.h"
#include "M_FileSystem.h"
#include "M_ResourceManager.h"

#include "Application.h"

#include "MemoryManager.h"

Application::Application() :
quit			(false),
debug			(false), 
hardwareInfo	(),
window			(nullptr),
input			(nullptr),
scene			(nullptr),
editor			(nullptr),
renderer		(nullptr),
camera			(nullptr),
fileSystem		(nullptr),
resourceManager(nullptr)
{
	//PERF_TIMER_START(perf_timer);
	
	// Modules -----------------------------------
	window				= new M_Window();
	input				= new M_Input();
	camera				= new M_Camera3D();
	renderer			= new M_Renderer3D();
	scene				= new M_Scene();
	editor				= new M_Editor();
	fileSystem			= new M_FileSystem();
	resourceManager	= new M_ResourceManager();

	// The order of calls is very important!
	// Modules will Init() Start() and Update in this order
	// They will CleanUp() in reverse order

	// Main Modules
	AddModule(window);
	AddModule(camera);
	AddModule(input);
	AddModule(fileSystem);
	AddModule(resourceManager);

	// Scenes
	AddModule(scene);
	AddModule(editor);

	// Renderer last!
	AddModule(renderer);
	// -------------------------------------------

	// Save/Load variables
	wantToLoad			= false;
	wantToSave			= false;
	userHasSaved			= false;

	// Framerate variables
	frameCap				= 0;
	secondsSinceStartup	= 0.0f;
	framesAreCapped		= framesAreCapped;
	displayFramerateData	= false;

	// Game Mode variables
	play					= false;
	pause					= false;
	step					= false;
	stop					= false;

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
	//PERF_TIMER_START(perf_timer);
	
	LOG("Application Init --------------");

	bool ret = true;

	char* buffer = nullptr;
	uint size = fileSystem->Load("Engine/Configuration/configuration.JSON", &buffer);
	if (size > 0)																			// Check if the configuration is empty and load the default configuration for the engine.
	{
		engineName			= TITLE;														// Change Later?
		organization		= ORGANIZATION;
		frameCap			= 60;
		framesAreCapped	= true;
	}
	else
	{
		uint defaultSize = fileSystem->Load("Engine/Configuration/default_configuration.JSON", &buffer);
		if (defaultSize <= 0)
		{
			LOG("[ERROR] Failed to load project settings.");
			return false;
		}

		engineName			= TITLE;
		organization		= ORGANIZATION;
		frameCap			= 60;
		framesAreCapped	= true;
	}

	ParsonNode config(buffer);
	ParsonNode node = config.GetNode("EditorState");

	std::vector<Module*>::iterator item = modules.begin();

	while (item != modules.end() && ret)
	{
		ret = (*item)->Init(node.GetNode((*item)->GetName()));		// Constructs and gives every module a handle for their own configuration node. M_Input -> "Input".
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

	Time::Real::InitRealClock();
	Time::Game::SetTimeScale(1.0f);

	//PERF_TIMER_PEEK(perf_timer);

	//ms_timer.Start();
	return ret;
}

bool Application::Start()												// IS IT NEEDED?
{
	// After all Init calls we call Start() in all modules
	LOG("Application Start --------------");

	bool ret = true;

	std::vector<Module*>::iterator item = modules.begin();

	while (item != modules.end() && ret)
	{
		ret = (*item)->Start();
		++item;
	}
	
	return ret;
}

// Call PreUpdate, Update and PostUpdate on all modules
UPDATE_STATUS Application::Update()
{
	UPDATE_STATUS ret = UPDATE_STATUS::CONTINUE;
	
	if (quit)
	{
		return UPDATE_STATUS::STOP;
	}

	PrepareUpdate();

	if (ret == UPDATE_STATUS::CONTINUE)
	{
		ret = PreUpdate();
	}
	
	if (ret == UPDATE_STATUS::CONTINUE)
	{
		ret = DoUpdate();
	}

	if (ret == UPDATE_STATUS::CONTINUE)
	{
		ret = PostUpdate();
	}

	FinishUpdate();

	return ret;
}

bool Application::CleanUp()
{
	bool ret = true;

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
	Time::Real::Update();

	if (play || step)
	{
		Time::Game::Update();
		step = false;
	}
}

UPDATE_STATUS Application::PreUpdate()
{
	UPDATE_STATUS ret = UPDATE_STATUS::CONTINUE;
	
	std::vector<Module*>::iterator item = modules.begin();

	while (item != modules.end() && ret == UPDATE_STATUS::CONTINUE)
	{
		if ((*item)->IsActive())
		{
			ret = (*item)->PreUpdate(Time::Game::GetDT());
		}

		++item;
	}

	if (ret == UPDATE_STATUS::THROW_ERROR)
	{
		LOG("PreUpdate threw an ERROR at Module %s.", (*item)->GetName());
	}

	return ret;
}

UPDATE_STATUS Application::DoUpdate()
{
	UPDATE_STATUS ret = UPDATE_STATUS::CONTINUE;

	std::vector<Module*>::iterator item = modules.begin();
	
	item = modules.begin();

	while (item != modules.end() && ret == UPDATE_STATUS::CONTINUE)
	{
		if ((*item)->IsActive())
		{
			ret = (*item)->Update(Time::Game::GetDT());
		}

		++item;
	}

	if (ret == UPDATE_STATUS::THROW_ERROR)
	{
		LOG("Update threw an ERROR at Module %s.", (*item)->GetName());
	}

	return ret;
}

UPDATE_STATUS Application::PostUpdate()
{
	UPDATE_STATUS ret = UPDATE_STATUS::CONTINUE;

	std::vector<Module*>::iterator item = modules.begin();
	
	item = modules.begin();

	while (item != modules.end() && ret == UPDATE_STATUS::CONTINUE)
	{
		if ((*item)->IsActive())
		{
			//ret = (*item)->PostUpdate(dt);
			ret = (*item)->PostUpdate(Time::Game::GetDT());
		}
		
		++item;
	}

	if (ret == UPDATE_STATUS::THROW_ERROR)
	{
		LOG("PostUpdate threw an ERROR at Module %s.", (*item)->GetName());
	}

	return ret;
}

void Application::FinishUpdate()
{
	if (wantToLoad)
	{
		LoadConfigurationNow(loadConfigFile.c_str());

		wantToLoad = false;
	}

	if (wantToSave)
	{
		SaveConfigurationNow(saveConfigFile.c_str());

		wantToSave = false;
	}

	if (framesAreCapped)
	{
		Time::Real::DelayUntilFrameCap(frameCap);
	}

	if (displayFramerateData)
	{
		App->window->SetTitle("Go to the Time Management header in the Configuration Panel to see all the Framerate Data.");
	}
	else
	{
		App->window->SetTitle(engineName.c_str());
	}

	// Editor: Configuration Frame Data Histograms
	UpdateFrameData(Time::Real::GetFramesLastSecond(), Time::Real::GetMsLastFrame());
}
// ---------------------------------------------

// --- SAVE & LOAD ENGINE CONFIGURATION ---
void Application::SaveConfiguration(const char* file)
{
	wantToSave = true;
	saveConfigFile = ("Engine/Configuration/configuration.json");
}

void Application::LoadConfiguration(const char* file)
{
	wantToLoad = true;

	if (userHasSaved)
	{
		loadConfigFile = ("Configuration.json");
	}
	else
	{
		loadConfigFile = ("DefaultConfiguration.json");
	}
}

void Application::SaveConfigurationNow(const char* file)
{
	if (file == nullptr)
	{
		LOG("[ERROR] Application: Could not Save Engine Configuration! Error: Given File Path string was nullptr.");
		return;
	}
	
	ParsonNode config;
	ParsonNode node = config.SetNode("EditorState");

	for (uint i = 0; i < modules.size(); ++i)
	{
		modules[i]->SaveConfiguration(config.SetNode(modules[i]->GetName()));
	}

	char* buffer = nullptr;
	uint written = config.SerializeToFile(file, &buffer);
	if (written > 0)
	{
		LOG("[STATE] Application: Successfully Saved Engine Configuration! Path: %s", file);
	}
	else
	{
		LOG("[ERROR] Application: Could not Save Engine Configuration! Error: File System could not write the File.");
	}

	RELEASE_ARRAY(buffer);
}

void Application::LoadConfigurationNow(const char* file)
{
	for (uint i = 0; i < modules.size(); ++i)
	{
		//JSON_Object* obj = 

		//modules[i]->LoadConfiguration(Configuration());
	}
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

void Application::SetFrameCap(uint new_cap)
{
	frameCap = new_cap;
}

// --- EDITOR METHODS ---
void Application::AddEditorLog(const char* log)
{
	if (!quit && editor != nullptr)													// Second condition is not really necessary. It's more of a reminder to keep it in mind.
	{
		//std::string full_log = App->fileSystem->NormalizePath(log);				// Switching all "\\" for "/". They need to be changed due to "\" being a Windows-specific thing.

		std::string fullLog = log;													// TMP. Switch to normalize later.

		uint logStart	= fullLog.find_last_of("\\") + 1;							// Gets the position of the last "\" in the log string.
		uint logEnd	= fullLog.size();											// The last position of the log will be equal to the size of it.

		std::string shortLog = fullLog.substr(logStart, logEnd);				// Returns the string that is within the given positions.

		editor->AddConsoleLog(shortLog.c_str());									// Priorized readability over reducing to AddConsoleLog(full_log.substr(log_start, log_end)).
	}
}

void Application::RequestBrowser(const char* link)
{
	ShellExecuteA(NULL, "open", link, NULL, "", 0);
}

void Application::UpdateFrameData(int frames, int ms)
{
	editor->UpdateFrameData(frames, ms);
}


void Application::LogHardwareInfo() const
{
	LOG(" ------------- CPU INFO ------------- ");
	LOG("CPU Cores: %d",						hardwareInfo.CPU.cpuCount);
	LOG("CPU Cache Size: %d ",					hardwareInfo.CPU.cacheSize);
	LOG("CPU RAM Size: %.1f GB",				hardwareInfo.CPU.ramGb);
	LOG(" --- DRIVERS --- ");
	LOG("CPU RDTSC: %s",						hardwareInfo.CPU.hasRDTSC		?	"True" : "False");
	LOG("CPU AltiVec: %s",						hardwareInfo.CPU.hasAltiVec	?	"True" : "False");
	LOG("CPU Now3D: %s",						hardwareInfo.CPU.has3DNow		?	"True" : "False");
	LOG("CPU MMX: %s",							hardwareInfo.CPU.hasMMX		?	"True" : "False");
	LOG("CPU SSE: %s",							hardwareInfo.CPU.hasSSE		?	"True" : "False");
	LOG("CPU SSE2: %s",							hardwareInfo.CPU.hasSSE2		?	"True" : "False");
	LOG("CPU SSE3: %s",							hardwareInfo.CPU.hasSSE3		?	"True" : "False");
	LOG("CPU SSE4.1: %s",						hardwareInfo.CPU.hasSSE41		?	"True" : "False");
	LOG("CPU SSE4.2: %s",						hardwareInfo.CPU.hasSSE42		?	"True" : "False");
	LOG("CPU AVX: %s",							hardwareInfo.CPU.hasAVX		?	"True" : "False");
	LOG("CPU AVX2: %s",							hardwareInfo.CPU.hasAVX2		?	"True" : "False");

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
	//LOG("OpenGL Extensions: %s",				hardware_info.OpenGL.extensions);

	/*for (int i = 0; i < hardware_info.OpenGL.extensions.size(); ++i)
	{
		LOG("OpenGL Extensions: %s",			hardware_info.OpenGL.extensions[i]);
	}*/
}

HardwareInfo Application::GetHardwareInfo() const
{
	return hardwareInfo;
}

/*if (display_framerate_data)
{
	static char framerate_data[256];

	sprintf_s(framerate_data, 256, "Av.FPS: %.2f / Last Frame Ms: %02u / Last sec frames: %i / Last dt: %.3f / Time since startup: %dh %dm %.3fs / Frame Count: %llu",
		frame_data.avg_fps, frame_data.ms_last_frame, frame_data.frames_last_second, frame_data.dt, clock.hours, clock.minutes, clock.seconds, frame_data.frame_count);

	App->window->SetTitle(framerate_data);
}*/