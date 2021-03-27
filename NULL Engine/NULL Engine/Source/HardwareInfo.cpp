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
	CPU.cpuCount							= 0;
	CPU.cacheSize							= 0;
	CPU.ramGb								= 0.0f;
	
	CPU.hasRDTSC							= false;
	CPU.hasAltiVec							= false;
	CPU.hasMMX								= false;
	CPU.has3DNow							= false;
	CPU.hasSSE								= false;
	CPU.hasSSE2								= false;
	CPU.hasSSE3								= false;
	CPU.hasSSE41							= false;
	CPU.hasSSE42							= false;
	CPU.hasAVX								= false;
	CPU.hasAVX2								= false;

	// ------ GPU INFO ------
	GPU.vendor								= 0;
	GPU.deviceId							= 0;
	strcpy_s(GPU.brand, "NONE");

	GPU.vramBudget							= 0;
	GPU.vramUsage							= 0;
	GPU.vramAvailable						= 0;
	GPU.vramReserved						= 0;

	// ------ SDL INFO ------
	strcpy_s(SDL.SDLVersion, "0");

	// ------ OPENGL INFO ------
	OpenGL.modelName						= nullptr;
	OpenGL.rendererName						= nullptr;
	OpenGL.version							= nullptr;
	OpenGL.shadingLanguageVersion			= nullptr;
	//OpenGL.extensions						= nullptr;

	// ------ DevIL INFO ------
	DevIL.vendor							= nullptr;
	DevIL.version							= nullptr;
}

void HardwareInfo::InitializeInfo()
{	
	// ------ CPU DATA ------
	CPU.cpuCount							= SDL_GetCPUCount();								// Getting the CPU's base info.
	CPU.cacheSize							= SDL_GetCPUCacheLineSize();						//
	CPU.ramGb								= (float)SDL_GetSystemRAM() / 1024.0f;				// ----------------------------

	CPU.hasRDTSC							= SDL_HasRDTSC();									// Getting the CPU's driver info/state
	CPU.hasAltiVec							= SDL_HasAltiVec();									//
	CPU.hasMMX								= SDL_HasMMX();										//
	CPU.has3DNow							= SDL_Has3DNow();									//
	CPU.hasSSE								= SDL_HasSSE();										//
	CPU.hasSSE2								= SDL_HasSSE2();									//
	CPU.hasSSE3								= SDL_HasSSE3();									//
	CPU.hasSSE41							= SDL_HasSSE41();									//
	CPU.hasSSE42							= SDL_HasSSE42();									//
	CPU.hasAVX								= SDL_HasAVX();										//
	CPU.hasAVX2								= SDL_HasAVX2();									// ---------------------

	// ------ GPU DATA ------
	uint			vendor;																		// Tmp/buffer variables.
	uint			deviceId;																	// 
	std::wstring	brand;																		// 
																								// 
	uint64			vramBudget;																	// 
	uint64			vramUsage;																	// 
	uint64			vramAvailable;																// 
	uint64			vramReserved;																// --------------------

	if (getGraphicsDeviceInfo(&vendor, &deviceId, &brand, &vramBudget, &vramUsage, &vramAvailable, &vramReserved))
	{
		GPU.vendor				= vendor;														// Getting the GPU's base info.
		GPU.deviceId			= deviceId;														//
		sprintf_s(GPU.brand, 250, "%S", brand.c_str());											// --------------------

		GPU.vramBudget		= float(vramBudget)		/ (1024.0f * 1024.0f * 1024.0f);			// Passing the variables to mb from different types (bits, bytes...)
		GPU.vramUsage		= float(vramUsage)		/ (1024.0f * 1024.0f * 1024.0f);			//
		GPU.vramAvailable	= float(vramAvailable)	/ (1024.0f * 1024.0f * 1024.0f);			//
		GPU.vramReserved	= float(vramReserved)	/ (1024.0f * 1024.0f * 1024.0f);			// --------------------
	}
	else
	{
		LOG("[ERROR] Could not get GPU Info from getGraphicsDeviceInfo()!");
	}

	// ------ SDL INFO ------
	SDL_version version;
	SDL_GetVersion(&version);

	int strLength = sprintf_s(SDL.SDLVersion, 25, "%i.%i.%i", version.major, version.minor, version.patch);

	if (strLength == 0)
		LOG("[ERROR] SDL Version string could not be constructed!");

	// ------ OPENGL INFO ------
	OpenGL.modelName					= (char*)glGetString(GL_VENDOR);
	OpenGL.rendererName					= (char*)glGetString(GL_RENDERER);
	OpenGL.version						= (char*)glGetString(GL_VERSION);
	OpenGL.shadingLanguageVersion		= (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
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
	uint64	vramBudget;
	uint64	vramUsage;
	uint64	vramAvailable;
	uint64	vramReserved;

	if (getGraphicsDeviceInfo(nullptr, nullptr, nullptr, &vramBudget, &vramUsage, &vramAvailable, &vramReserved))
	{
		float toMb			= (1024.0f * 1024.0f * 1024.0f);

		GPU.vramBudget		= float(vramBudget)		/ toMb;
		GPU.vramUsage		= float(vramUsage)		/ toMb;
		GPU.vramAvailable	= float(vramAvailable)	/ toMb;
		GPU.vramReserved	= float(vramReserved)	/ toMb;
	}
}

void HardwareInfo::CleanUp()
{
	OpenGL.extensions.clear();
}