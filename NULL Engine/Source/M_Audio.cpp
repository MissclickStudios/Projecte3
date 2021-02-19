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

#include "AK/SoundEngine/Common/AkStreamMgrModule.h"
#include "AK/SoundEngine/Common/AkSoundEngine.h"
#include "AK/MusicEngine/Common/AkMusicEngine.h"
#include "AK/SoundEngine/Common/AkTypes.h"
#include "AK/Tools/Common/AkPlatformFuncs.h"
#include "AK/AkPlatforms.h"

M_Audio::M_Audio(bool isActive) : Module("Audio", isActive)
{
}

M_Audio::~M_Audio()
{
}

bool M_Audio::InitSoundEngine()
{
	AkMemSettings memSettings;

	AK::MemoryMgr::GetDefaultSettings(memSettings);



	if (AK::MemoryMgr::Init(&memSettings) != AK_Success)

	{

		AKASSERT(!"Could not create the memory manager.");

		return false;

	}

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