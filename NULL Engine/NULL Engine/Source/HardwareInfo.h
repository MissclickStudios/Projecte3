#ifndef __HARDWARE_INFO_H__
#define __HARDWARE_INFO_H__

#include <vector>
#include "Macros.h"

typedef unsigned int uint;

struct NULL_API CPU
{
	int		cpuCount;							// Number of available CPU cores. SDL_GetCPUCount() returns the number of available CPU cores.
	int		cacheSize;							// Size of the CPU's L1 cache line. SDL_GetCPUCacheLineSize() is useful to determine multi-threaded structure padding or SIMD prefetch sizes.
	float	ramGb;								// Amount of RAM configured in the system in GB. SDL_GetSystemRam(). 

	bool	hasRDTSC;							// True if the CPU has the RDTSC instruction. RDTSC loads the current value of the CPU's time-stamp counter into the EDX:EAX registers.  
	bool	hasAltiVec;							// True if the CPU has AltiVec features. AltiVec describes fundamental data types such as unsinged/signed char, unsigned/signed int...
	bool	hasMMX;								// True if the CPU has MMX features. Intel extension to speed-up multimedia operations. Applicable to graphics and communications.
	bool	has3DNow;							// True if the CPU has 3DNow! features. AMD extension to x86 Architectures to enable vector processing, enhancing the perf of 3D graphics apps.
	bool	hasSSE;								// True if the CPU has SSE features. Intel ext. to x86 Archs. to enhance the perf. when the same flpt ops. are performed on multiple data objs.
	bool	hasSSE2;							// True if the CPU has SSE2 features. Intel's 2nd iteration of the SSE extension. Aimed to fully replace MMX.
	bool	hasSSE3;							// True if the CPU has SSE3 features. Intel's 3rd iteration of the SSE extension. Adds the capability of working horizontally in a register.
	bool	hasSSE41;							// True if the CPU has SSE4.1 features. Intel's 4th iteration of the SSE ext. Adds instrcts. that execute ops. non-specific to multimedia apps.
	bool	hasSSE42;							// True if the CPU has SSE4.2 features. " ". Adds several new instrcts. to perform char searches and cmps on two 16 bytes operands at a time.
	bool	hasAVX;								// True if the CPU has AVX features. Exts. to x86 Archs. for Intel and AMD microprocs. Expands int commands to 256 bits and introduces FMA ops.
	bool	hasAVX2;							// True if the CPU has AVX2 features. Expansion of AVX. Expands most vector integer SSE and AVX instructions to 256 bits, adds vector shifts...
};

struct NULL_API GPU
{
	uint	vendor;								// Vendor number of the GPU card.
	uint	deviceId;							// Device id number of the GPU card.
	char	brand[250];							// String containing the name of the brand that built the GPU card. 

	float	vramBudget;							// Current VRAM budget in Mb.
	float	vramUsage;							// Current VRAM usage in Mb.
	float	vramAvailable;						// Current VRAM available in Mb.
	float	vramReserved;						// Current VRAM reserved in Mb.
};

struct NULL_API SDLInfo
{
	char SDLVersion[25];						// String containing the version or release number of the SDL version that is being used.
};

struct NULL_API OpenGLInfo
{
	char* modelName;							// String containing the name of the company responsible for this GL implementation. Remains unchanged release to release.
	char* rendererName;							// String containing the name of the renderer. Typically specific to a particular configuration of a hardware platform. Remains " ".
	char* version;								// String containing the version or release number of the OpenGL version that is being used.
	char* shadingLanguageVersion;				// String containing the version or release number of the shading language version that is being used.
	//char* extensions;							// String containing the extension string.

	std::vector<char*> extensions;
};

struct NULL_API DevILInfo
{
	char* vendor;
	char* version;
};

struct NULL_API HardwareInfo
{
	HardwareInfo();								// Will initialize all Hardware variables. Preventive measure to avoid variables being used elsewhere while uninitialized.
	
	void InitializeInfo();						// Will initialize all Hardware Info. Currently it inititializes CPU, GPU, SDL and OpenGL info.
	void UpdateInfo();							// Will update the Hardware Info. Currently it only updates GPU VRAM's information.
	void CleanUp();

	CPU			CPU;							// Container for CPU Information.
	GPU			GPU;							// Container for GPU Information.
	SDLInfo		SDL;							// Container for SDL Information.
	OpenGLInfo	OpenGL;							// Container for OpenGL Information.
	DevILInfo	DevIL;
};

#endif // !__HARDWARE_INFO_H__