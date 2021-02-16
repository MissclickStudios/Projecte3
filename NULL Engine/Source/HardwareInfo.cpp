#include <stdio.h>
#include <string>

#include "SDL/include/SDL_version.h"
#include "SDL/include/SDL_cpuinfo.h"
#include "gpudetect/include/DeviceId.h"

#include "OpenGL.h"
#include "DevIL.h"
#include "Log.h"
#include "VariableTypedefs.h"

#include "HardwareInfo.h"

HardwareInfo::HardwareInfo()
{
	// ------ CPU INFO ------
	CPU.cpu_count							= 0;
	CPU.cache_size							= 0;
	CPU.ram_gb								= 0.0f;
	
	CPU.has_RDTSC							= false;
	CPU.has_AltiVec							= false;
	CPU.has_MMX								= false;
	CPU.has_3DNow							= false;
	CPU.has_SSE								= false;
	CPU.has_SSE2							= false;
	CPU.has_SSE3							= false;
	CPU.has_SSE41							= false;
	CPU.has_SSE42							= false;
	CPU.has_AVX								= false;
	CPU.has_AVX2							= false;

	// ------ GPU INFO ------
	GPU.vendor								= 0;
	GPU.device_id							= 0;
	strcpy_s(GPU.brand, "NONE");

	GPU.vram_mb_budget						= 0;
	GPU.vram_mb_usage						= 0;
	GPU.vram_mb_available					= 0;
	GPU.vram_mb_reserved					= 0;

	// ------ SDL INFO ------
	strcpy_s(SDL.sdl_version, "0");

	// ------ OPENGL INFO ------
	OpenGL.model_name						= nullptr;
	OpenGL.renderer_name					= nullptr;
	OpenGL.version							= nullptr;
	OpenGL.shading_language_version			= nullptr;
	//OpenGL.extensions						= nullptr;

	// ------ DevIL INFO ------
	DevIL.vendor							= nullptr;
	DevIL.version							= nullptr;
}

void HardwareInfo::InitializeInfo()
{	
	// ------ CPU DATA ------
	CPU.cpu_count							= SDL_GetCPUCount();								// Getting the CPU's base info.
	CPU.cache_size							= SDL_GetCPUCacheLineSize();						//
	CPU.ram_gb								= (float)SDL_GetSystemRAM() / 1024.0f;				// ----------------------------

	CPU.has_RDTSC							= SDL_HasRDTSC();									// Getting the CPU's driver info/state
	CPU.has_AltiVec							= SDL_HasAltiVec();									//
	CPU.has_MMX								= SDL_HasMMX();										//
	CPU.has_3DNow							= SDL_Has3DNow();									//
	CPU.has_SSE								= SDL_HasSSE();										//
	CPU.has_SSE2							= SDL_HasSSE2();									//
	CPU.has_SSE3							= SDL_HasSSE3();									//
	CPU.has_SSE41							= SDL_HasSSE41();									//
	CPU.has_SSE42							= SDL_HasSSE42();									//
	CPU.has_AVX								= SDL_HasAVX();										//
	CPU.has_AVX2							= SDL_HasAVX2();									// ---------------------

	// ------ GPU DATA ------
	uint			vendor;																		// Tmp/buffer variables.
	uint			device_id;																	// 
	std::wstring	brand;																		// 
																								// 
	uint64			vram_budget;																// 
	uint64			vram_usage;																	// 
	uint64			vram_available;																// 
	uint64			vram_reserved;																// --------------------

	if (getGraphicsDeviceInfo(&vendor, &device_id, &brand, &vram_budget, &vram_usage, &vram_available, &vram_reserved))
	{
		GPU.vendor				= vendor;														// Getting the GPU's base info.
		GPU.device_id			= device_id;													//
		sprintf_s(GPU.brand, 250, "%S", brand.c_str());											// --------------------

		GPU.vram_mb_budget		= float(vram_budget)	/ (1024.0f * 1024.0f * 1024.0f);		// Passing the variables to mb from different types (bits, bytes...)
		GPU.vram_mb_usage		= float(vram_usage)		/ (1024.0f * 1024.0f * 1024.0f);		//
		GPU.vram_mb_available	= float(vram_available)	/ (1024.0f * 1024.0f * 1024.0f);		//
		GPU.vram_mb_reserved	= float(vram_reserved)	/ (1024.0f * 1024.0f * 1024.0f);		// --------------------
	}
	else
	{
		LOG("[ERROR] Could not get GPU Info from getGraphicsDeviceInfo()!");
	}

	// ------ SDL INFO ------
	SDL_version version;
	SDL_GetVersion(&version);

	int str_length = sprintf_s(SDL.sdl_version, 25, "%i.%i.%i", version.major, version.minor, version.patch);

	if (str_length == 0)
	{
		LOG("[ERROR] SDL Version string could not be constructed!");
	}

	// ------ OPENGL INFO ------
	OpenGL.model_name					= (char*)glGetString(GL_VENDOR);
	OpenGL.renderer_name				= (char*)glGetString(GL_RENDERER);
	OpenGL.version						= (char*)glGetString(GL_VERSION);
	OpenGL.shading_language_version		= (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	//OpenGL.extensions					= (char*)glewGetString(GL_EXTENSIONS);

	char* ext = "";
	for (int i = 0; ext != nullptr; ++i)
	{
		ext = (char*)glGetStringi(GL_EXTENSIONS, i);

		OpenGL.extensions.push_back(ext);
	}

	// ------ DevIL INFO ------
	DevIL.vendor						= (char*)ilGetString(IL_VENDOR);
	DevIL.version						= (char*)ilGetString(IL_VERSION_NUM);
}

void HardwareInfo::UpdateInfo()
{
	uint64			vram_budget;
	uint64			vram_usage;
	uint64			vram_available;
	uint64			vram_reserved;

	if (getGraphicsDeviceInfo(nullptr, nullptr, nullptr, &vram_budget, &vram_usage, &vram_available, &vram_reserved))
	{
		float to_mb				= (1024.0f * 1024.0f * 1024.0f);

		GPU.vram_mb_budget		= float(vram_budget)	/ to_mb;
		GPU.vram_mb_usage		= float(vram_usage)		/ to_mb;
		GPU.vram_mb_available	= float(vram_available) / to_mb;
		GPU.vram_mb_reserved	= float(vram_reserved)	/ to_mb;
	}
}

void HardwareInfo::CleanUp()
{
	OpenGL.extensions.clear();
}