#ifndef __HARDWARE_INFO_H__
#define __HARDWARE_INFO_H__

#include <vector>

typedef unsigned int uint;

struct CPU
{
	int		cpu_count;							// Number of available CPU cores. SDL_GetCPUCount() returns the number of available CPU cores.
	int		cache_size;							// Size of the CPU's L1 cache line. SDL_GetCPUCacheLineSize() is useful to determine multi-threaded structure padding or SIMD prefetch sizes.
	float	ram_gb;								// Amount of RAM configured in the system in GB. SDL_GetSystemRam(). 

	bool	has_RDTSC;							// True if the CPU has the RDTSC instruction. RDTSC loads the current value of the CPU's time-stamp counter into the EDX:EAX registers.  
	bool	has_AltiVec;						// True if the CPU has AltiVec features. AltiVec describes fundamental data types such as unsinged/signed char, unsigned/signed int...
	bool	has_MMX;							// True if the CPU has MMX features. Intel extension to speed-up multimedia operations. Applicable to graphics and communications.
	bool	has_3DNow;							// True if the CPU has 3DNow! features. AMD extension to x86 Architectures to enable vector processing, enhancing the perf of 3D graphics apps.
	bool	has_SSE;							// True if the CPU has SSE features. Intel ext. to x86 Archs. to enhance the perf. when the same flpt ops. are performed on multiple data objs.
	bool	has_SSE2;							// True if the CPU has SSE2 features. Intel's 2nd iteration of the SSE extension. Aimed to fully replace MMX.
	bool	has_SSE3;							// True if the CPU has SSE3 features. Intel's 3rd iteration of the SSE extension. Adds the capability of working horizontally in a register.
	bool	has_SSE41;							// True if the CPU has SSE4.1 features. Intel's 4th iteration of the SSE ext. Adds instrcts. that execute ops. non-specific to multimedia apps.
	bool	has_SSE42;							// True if the CPU has SSE4.2 features. " ". Adds several new instrcts. to perform char searches and cmps on two 16 bytes operands at a time.
	bool	has_AVX;							// True if the CPU has AVX features. Exts. to x86 Archs. for Intel and AMD microprocs. Expands int commands to 256 bits and introduces FMA ops.
	bool	has_AVX2;							// True if the CPU has AVX2 features. Expansion of AVX. Expands most vector integer SSE and AVX instructions to 256 bits, adds vector shifts...
};

struct GPU
{
	uint	vendor;								// Vendor number of the GPU card.
	uint	device_id;							// Device id number of the GPU card.
	char	brand[250];							// String containing the name of the brand that built the GPU card. 

	float	vram_mb_budget;						// Current VRAM budget in Mb.
	float	vram_mb_usage;						// Current VRAM usage in Mb.
	float	vram_mb_available;					// Current VRAM available in Mb.
	float	vram_mb_reserved;					// Current VRAM reserved in Mb.
};

struct SDLInfo
{
	char sdl_version[25];						// String containing the version or release number of the SDL version that is being used.
};

struct OpenGLInfo
{
	char* model_name;							// String containing the name of the company responsible for this GL implementation. Remains unchanged release to release.
	char* renderer_name;						// String containing the name of the renderer. Typically specific to a particular configuration of a hardware platform. Remains " ".
	char* version;								// String containing the version or release number of the OpenGL version that is being used.
	char* shading_language_version;				// String containing the version or release number of the shading language version that is being used.
	//char* extensions;							// String containing the extension string.

	std::vector<char*> extensions;
};

struct DevILInfo
{
	char* vendor;
	char* version;
};

struct HardwareInfo
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