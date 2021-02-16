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
hardware_info	(),
window			(nullptr),
input			(nullptr),
scene			(nullptr),
editor			(nullptr),
renderer		(nullptr),
camera			(nullptr),
file_system		(nullptr),
resource_manager(nullptr)
{
	//PERF_TIMER_START(perf_timer);
	
	// Modules -----------------------------------
	window				= new M_Window();
	input				= new M_Input();
	camera				= new M_Camera3D();
	renderer			= new M_Renderer3D();
	scene				= new M_Scene();
	editor				= new M_Editor();
	file_system			= new M_FileSystem();
	resource_manager	= new M_ResourceManager();

	// The order of calls is very important!
	// Modules will Init() Start() and Update in this order
	// They will CleanUp() in reverse order

	// Main Modules
	AddModule(window);
	AddModule(camera);
	AddModule(input);
	AddModule(file_system);
	AddModule(resource_manager);

	// Scenes
	AddModule(scene);
	AddModule(editor);

	// Renderer last!
	AddModule(renderer);
	// -------------------------------------------

	// Save/Load variables
	want_to_load			= false;
	want_to_save			= false;
	user_has_saved			= false;

	// Framerate variables
	frame_cap				= 0;
	seconds_since_startup	= 0.0f;
	frames_are_capped		= FRAMES_ARE_CAPPED;
	display_framerate_data	= false;

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
	uint size = file_system->Load("Engine/Configuration/configuration.JSON", &buffer);
	if (size > 0)																			// Check if the configuration is empty and load the default configuration for the engine.
	{
		engine_name			= TITLE;														// Change Later?
		organization		= ORGANIZATION;
		frame_cap			= 60;
		frames_are_capped	= true;
	}
	else
	{
		uint default_size = file_system->Load("Engine/Configuration/default_configuration.JSON", &buffer);
		if (default_size <= 0)
		{
			LOG("[ERROR] Failed to load project settings.");
			return false;
		}

		engine_name			= TITLE;
		organization		= ORGANIZATION;
		frame_cap			= 60;
		frames_are_capped	= true;
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
	hardware_info.InitializeInfo();
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

	hardware_info.CleanUp();

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
	if (want_to_load)
	{
		LoadConfigurationNow(load_config_file.c_str());

		want_to_load = false;
	}

	if (want_to_save)
	{
		SaveConfigurationNow(save_config_file.c_str());

		want_to_save = false;
	}

	if (frames_are_capped)
	{
		Time::Real::DelayUntilFrameCap(frame_cap);
	}

	if (display_framerate_data)
	{
		App->window->SetTitle("Go to the Time Management header in the Configuration Panel to see all the Framerate Data.");
	}
	else
	{
		App->window->SetTitle(engine_name.c_str());
	}

	// Editor: Configuration Frame Data Histograms
	UpdateFrameData(Time::Real::GetFramesLastSecond(), Time::Real::GetMsLastFrame());
}
// ---------------------------------------------

// --- SAVE & LOAD ENGINE CONFIGURATION ---
void Application::SaveConfiguration(const char* file)
{
	want_to_save = true;
	save_config_file = ("Engine/Configuration/configuration.json");
}

void Application::LoadConfiguration(const char* file)
{
	want_to_load = true;

	if (user_has_saved)
	{
		load_config_file = ("Configuration.json");
	}
	else
	{
		load_config_file = ("DefaultConfiguration.json");
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
	return engine_name.c_str();
}

const char* Application::GetOrganizationName() const
{
	return organization.c_str();
}

void Application::SetEngineName(const char* name)
{
	engine_name = name;

	App->window->SetTitle(engine_name.c_str());
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
	return frame_cap;
}

void Application::SetFrameCap(uint new_cap)
{
	frame_cap = new_cap;
}

// --- EDITOR METHODS ---
void Application::AddEditorLog(const char* log)
{
	if (!quit && editor != nullptr)													// Second condition is not really necessary. It's more of a reminder to keep it in mind.
	{
		//std::string full_log = App->file_system->NormalizePath(log);				// Switching all "\\" for "/". They need to be changed due to "\" being a Windows-specific thing.

		std::string full_log = log;													// TMP. Switch to normalize later.

		uint log_start	= full_log.find_last_of("\\") + 1;							// Gets the position of the last "\" in the log string.
		uint log_end	= full_log.size();											// The last position of the log will be equal to the size of it.

		std::string short_log = full_log.substr(log_start, log_end);				// Returns the string that is within the given positions.

		editor->AddConsoleLog(short_log.c_str());									// Priorized readability over reducing to AddConsoleLog(full_log.substr(log_start, log_end)).
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
	LOG("CPU Cores: %d",						hardware_info.CPU.cpu_count);
	LOG("CPU Cache Size: %d ",					hardware_info.CPU.cache_size);
	LOG("CPU RAM Size: %.1f GB",				hardware_info.CPU.ram_gb);
	LOG(" --- DRIVERS --- ");
	LOG("CPU RDTSC: %s",						hardware_info.CPU.has_RDTSC		?	"True" : "False");
	LOG("CPU AltiVec: %s",						hardware_info.CPU.has_AltiVec	?	"True" : "False");
	LOG("CPU Now3D: %s",						hardware_info.CPU.has_3DNow		?	"True" : "False");
	LOG("CPU MMX: %s",							hardware_info.CPU.has_MMX		?	"True" : "False");
	LOG("CPU SSE: %s",							hardware_info.CPU.has_SSE		?	"True" : "False");
	LOG("CPU SSE2: %s",							hardware_info.CPU.has_SSE2		?	"True" : "False");
	LOG("CPU SSE3: %s",							hardware_info.CPU.has_SSE3		?	"True" : "False");
	LOG("CPU SSE4.1: %s",						hardware_info.CPU.has_SSE41		?	"True" : "False");
	LOG("CPU SSE4.2: %s",						hardware_info.CPU.has_SSE42		?	"True" : "False");
	LOG("CPU AVX: %s",							hardware_info.CPU.has_AVX		?	"True" : "False");
	LOG("CPU AVX2: %s",							hardware_info.CPU.has_AVX2		?	"True" : "False");

	LOG(" ------------- GPU INFO ------------- ");
	LOG("GPU Vendor %d Device %d",				hardware_info.GPU.vendor, hardware_info.GPU.device_id);
	LOG("GPU Brand: %s",						hardware_info.GPU.brand);
	LOG(" --- VRAM --- ");
	LOG("GPU VRAM Budget: %.1f MB",				hardware_info.GPU.vram_mb_budget);
	LOG("GPU VRAM Usage: %.1f MB",				hardware_info.GPU.vram_mb_usage);
	LOG("GPU VRAM Available: %.1f MB",			hardware_info.GPU.vram_mb_available);
	LOG("GPU VRAM Reserved: %.1f MB",			hardware_info.GPU.vram_mb_reserved);

	LOG(" ------------- SDL INFO ------------- ");
	LOG("SDL Version: %s",						hardware_info.SDL.sdl_version);

	LOG(" ------------- OPENGL INFO ------------- ");
	LOG("OpenGL Model: %s",						hardware_info.OpenGL.model_name);
	LOG("OpenGL Renderer: %s",					hardware_info.OpenGL.renderer_name);
	LOG("OpenGL Version: %s",					hardware_info.OpenGL.version);
	LOG("OpenGL Shading Language Version: %s",	hardware_info.OpenGL.shading_language_version);
	//LOG("OpenGL Extensions: %s",				hardware_info.OpenGL.extensions);

	/*for (int i = 0; i < hardware_info.OpenGL.extensions.size(); ++i)
	{
		LOG("OpenGL Extensions: %s",			hardware_info.OpenGL.extensions[i]);
	}*/
}

HardwareInfo Application::GetHardwareInfo() const
{
	return hardware_info;
}

/*if (display_framerate_data)
{
	static char framerate_data[256];

	sprintf_s(framerate_data, 256, "Av.FPS: %.2f / Last Frame Ms: %02u / Last sec frames: %i / Last dt: %.3f / Time since startup: %dh %dm %.3fs / Frame Count: %llu",
		frame_data.avg_fps, frame_data.ms_last_frame, frame_data.frames_last_second, frame_data.dt, clock.hours, clock.minutes, clock.seconds, frame_data.frame_count);

	App->window->SetTitle(framerate_data);
}*/