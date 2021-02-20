#ifndef _DEBUG
#define AK_OPTIMIZED
#else
#include "AK/Comm/AkCommunication.h"
#pragma comment(lib,"CommunicationCentral.lib")
#pragma comment(lib,"ws2_32.lib")
#endif

#include "M_Audio.h"
#include "AK/SoundEngine/Common/AkMemoryMgr.h"		// Memory Manager interface		
#include "AK/SoundEngine/Common/AkModule.h"			// Default memory manager		

#include "AK/SoundEngine/Common/IAkStreamMgr.h"		// Streaming Manager
#include "AK/Tools/Common/AkPlatformFuncs.h"		// Thread defines
#include "AkFilePackageLowLevelIOBlocking.h"		// Sample low-level I/O implementation
#include "AK/SoundEngine/Common/AkSoundEngine.h"	// Sound engine
#include "AK/MusicEngine/Common/AkMusicEngine.h"	// Music Engine
#include "AK/SpatialAudio/Common/AkSpatialAudio.h"	// Spatial Audio

/*#include "AK/SoundEngine/Common/AkStreamMgrModule.h"
#include "AK/SoundEngine/Common/AkSoundEngine.h"
#include "AK/MusicEngine/Common/AkMusicEngine.h"
#include "AK/SoundEngine/Common/AkTypes.h"
#include "AK/Tools/Common/AkPlatformFuncs.h"
#include "AK/AkPlatforms.h"*/

M_Audio::M_Audio(bool isActive) : Module("Audio", isActive)
{
}

M_Audio::~M_Audio()
{
}

// We're using the default Low-Level I/O implementation that's part
// of the SDK's sample code, with the file package extension
CAkFilePackageLowLevelIOBlocking g_lowLevelIO;

bool M_Audio::InitSoundEngine()
{
	AkMemSettings memSettings;
	AK::MemoryMgr::GetDefaultSettings(memSettings);
	if (AK::MemoryMgr::Init(&memSettings) != AK_Success)
	{

		AKASSERT(!"Could not create the memory manager.");

		return false;
	}
   // Create and initialize an instance of the default streaming manager. Note
   // that you can override the default streaming manager with your own. 


	AkStreamMgrSettings stmSettings;
	AK::StreamMgr::GetDefaultSettings(stmSettings);

	// Customize the Stream Manager settings here.

	if (!AK::StreamMgr::Create(stmSettings))
	{
		//assert(!"Could not create the Streaming Manager");
		return false;
	}

	// Create a streaming device with blocking low-level I/O handshaking.
	// Note that you can override the default low-level I/O module with your own. 

	AkDeviceSettings deviceSettings;
	AK::StreamMgr::GetDefaultDeviceSettings(deviceSettings);

	// Customize the streaming device settings here.

	// CAkFilePackageLowLevelIOBlocking::Init() creates a streaming device
	// in the Stream Manager, and registers itself as the File Location Resolver.

	if (g_lowLevelIO.Init(deviceSettings) != AK_Success)
	{
		//assert(!"Could not create the streaming device and Low-Level I/O system");

		return false;
	}

   // Create the Sound Engine

   // Using default initialization parameters
	AkInitSettings initSettings;
	AkPlatformInitSettings platformInitSettings;
	AK::SoundEngine::GetDefaultInitSettings(initSettings);
	AK::SoundEngine::GetDefaultPlatformInitSettings(platformInitSettings);

	if (AK::SoundEngine::Init(&initSettings, &platformInitSettings) != AK_Success)
	{
		//assert(!"Could not initialize the Sound Engine.");

		return false;
	}

	// Initialize the music engine
	// Using default initialization parameters
	AkMusicSettings musicInit;
	AK::MusicEngine::GetDefaultInitSettings(musicInit);

	if (AK::MusicEngine::Init(&musicInit) != AK_Success)
	{
		//assert(!"Could not initialize the Music Engine.");

		return false;
	}

	/*// Initialize Spatial Audio
	// Using default initialization parameters
	AkSpatialAudioInitSettings settings; // The constructor fills AkSpatialAudioInitSettings with the recommended default settings. 

	if (AK::SpatialAudio::Init(settings) != AK_Success)
	{
		//assert(!"Could not initialize the Spatial Audio.");

		return false;
	}*/

#ifndef AK_OPTIMIZED

	// Initialize communications (not in release build!)

	AkCommSettings commSettings;
	AK::Comm::GetDefaultInitSettings(commSettings);

	if (AK::Comm::Init(commSettings) != AK_Success)
	{
		//assert(!"Could not initialize communication.");

		return false;
	}

#endif // AK_OPTIMIZED

}


void TermSoundEngine()
{
#ifndef AK_OPTIMIZED
	// Terminate Communication Services
	AK::Comm::Term();
#endif // AK_OPTIMIZED
	/*// Terminate Spatial Audio
	AK::SpatialAudio::Term();*/

	// Terminate the music engine
	AK::MusicEngine::Term();

	// Terminate the sound engine
	AK::SoundEngine::Term();

	// Terminate the streaming device and streaming manager
	// CAkFilePackageLowLevelIOBlocking::Term() destroys its associated streaming device 
	// that lives in the Stream Manager, and unregisters itself as the File Location Resolver.
	g_lowLevelIO.Term();
	if (AK::IAkStreamMgr::Get())
		AK::IAkStreamMgr::Get()->Destroy();

	// Terminate the Memory Manager
	AK::MemoryMgr::Term();
}

/*void M_Audio::SndInit()
{
	// Initialize audio engine
	// Memory.
	AkMemSettings memSettings;
	memSettings.uMaxNumPools = 20;
	// Streaming.
	AkStreamMgrSettings stmSettings;
	AK::StreamMgr::GetDefaultSettings(stmSettings);
	AkDeviceSettings deviceSettings;
	AK::StreamMgr::GetDefaultDeviceSettings(deviceSettings);
	AkInitSettings l_InitSettings;
	AkPlatformInitSettings l_platInitSetings;
	AK::SoundEngine::GetDefaultInitSettings(l_InitSettings);
	AK::SoundEngine::GetDefaultPlatformInitSettings(l_platInitSetings);
	// Setting pool sizes for this game. Here, allow for user content; every game should determine its own optimal values.
	l_InitSettings.uDefaultPoolSize = 2 * 1024 * 1024;
	l_platInitSetings.uLEngineDefaultPoolSize = 4 * 1024 * 1024;
	AkMusicSettings musicInit;
	AK::MusicEngine::GetDefaultInitSettings(musicInit);
	// Create and initialise an instance of our memory manager.
	if (AK::MemoryMgr::Init(&memSettings) != AK_Success)
	{
		AKASSERT(!"Could not create the memory manager.");
		return;
	}
	// Create and initialise an instance of the default stream manager.
	if (!AK::StreamMgr::Create(stmSettings))
	{
		AKASSERT(!"Could not create the Stream Manager");
		return;
	}
	// Create an IO device.
	if (g_lowLevelIO.Init(deviceSettings) != AK_Success)
	{
		AKASSERT(!"Cannot create streaming I/O device");
		return;
	}
	// Initialize sound engine.
	if (AK::SoundEngine::Init(&l_InitSettings, &l_platInitSetings) != AK_Success)
	{
		AKASSERT(!"Cannot initialize sound engine");
		return;
	}
	// Initialize music engine.
	if (AK::MusicEngine::Init(&musicInit) != AK_Success)
	{
		AKASSERT(!"Cannot initialize music engine");
		return;
	}
	// load initialization and main soundbanks
}*/