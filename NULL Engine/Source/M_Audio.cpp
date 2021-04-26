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
#include "AK/SoundEngine/Common/AkCallback.h"

#include "AK/SoundEngine/Common/IAkStreamMgr.h"		// Streaming Manager
#include "AK/Tools/Common/AkPlatformFuncs.h"		// Thread defines
#include "AkFilePackageLowLevelIOBlocking.h"		// Sample low-level I/O implementation
#include "AK/SoundEngine/Common/AkSoundEngine.h"	// Sound engine
#include "AK/MusicEngine/Common/AkMusicEngine.h"	// Music Engine
#include "AK/SpatialAudio/Common/AkSpatialAudio.h"	// Spatial Audio

#include "Application.h"
#include "Log.h"
#include "M_FileSystem.h"
#include "FileSystemDefinitions.h"
#include "JSONParser.h"
#include "Profiler.h"

#include "C_AudioListener.h"
#include "C_AudioSource.h"
#include "M_Scene.h"

#include <utility>
#include <iostream>

#include "MemoryManager.h"

M_Audio::M_Audio(bool isActive) : Module("Audio", isActive)
{

}

M_Audio::~M_Audio()
{
	for (int i = 0; i < audioListenerList.size(); ++i)
	{
		delete audioListenerList[i];
		audioListenerList[i] = nullptr;
	}

	for (int i = 0; i < audioSourceList.size(); ++i)
	{
		delete audioSourceList[i];
		audioSourceList[i] = nullptr;
	}
}

bool M_Audio::Init(ParsonNode& root)
{
	InitSoundEngine();

	std::vector<std::string> banks;
	FindSoundBanks(&banks);

	for (std::vector<std::string>::iterator it = banks.begin(); it != banks.end(); ++it)
	{
		LoadSoundBank((*it).c_str());
	}

	return true;
}

bool M_Audio::Start()
{
	LoadEventsFromJson();

	aSourceBackgroundMusic = new C_AudioSource(App->scene->GetMasterRoot());

	aSourceBackgroundMusic->SetEvent("background");

	aSourceBackgroundMusic->PlayFx(aSourceBackgroundMusic->GetEventId());
	
	return true;
}

UpdateStatus M_Audio::Update(float dt)
{
	OPTICK_CATEGORY("M_Audio Update", Optick::Category::Module)
	AK::SoundEngine::RenderAudio();

	//Depending on the engine state pause/play/resume/stop events

	//if (App->gameState == GameState::PLAY || App->gameState == GameState::STEP)
	//{
	//	if (!aSourceBackgroundMusic->isPlaying)
	//		aSourceBackgroundMusic->PlayFx(aSourceBackgroundMusic->GetEventId());
	//}
	//if (App->gameState == GameState::PAUSE)
	//{
	//	if (aSourceBackgroundMusic->isPlaying)
	//		aSourceBackgroundMusic->PauseFx(aSourceBackgroundMusic->GetEventId());
	//}
	//if (App->gameState == GameState::STOP)
	//{
	//	if (aSourceBackgroundMusic->isPlaying)
	//		aSourceBackgroundMusic->StopFx(aSourceBackgroundMusic->GetEventId());
	//}
	//
	return UpdateStatus::CONTINUE;
}

bool M_Audio::CleanUp()
{
	TermSoundEngine();
	return true;
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
		return false;
	}
   // Create and initialize an instance of the default streaming manager. Note
   // that you can override the default streaming manager with your own. 


	AkStreamMgrSettings stmSettings;
	AK::StreamMgr::GetDefaultSettings(stmSettings);

	// Customize the Stream Manager settings here.

	if (!AK::StreamMgr::Create(stmSettings))
	{
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
		return false;
	}

	// Initialize the music engine
	// Using default initialization parameters
	AkMusicSettings musicInit;
	AK::MusicEngine::GetDefaultInitSettings(musicInit);

	if (AK::MusicEngine::Init(&musicInit) != AK_Success)
	{
		return false;
	}


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

	std::string initBankPath = "Assets/SoundBanks/Init.bnk";
	AkBankID bankID;
	AKRESULT result  = AK::SoundEngine::LoadBank(initBankPath.c_str(), bankID);

	if (result == AK_Success)
		LOG("Loaded %s", initBankPath.c_str());
}

void M_Audio::TermSoundEngine()
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

void M_Audio::SetWwiseState(const char* stateGroup, const char* state)
{
	AK::SoundEngine::SetState(stateGroup, state);
}

void M_Audio::PauseAll()
{
	AK::SoundEngine::Suspend();
}

void M_Audio::ResumeAll()
{
	AK::SoundEngine::WakeupFromSuspend();
}

void M_Audio::StopAll()
{
	AK::SoundEngine::StopAll();
}

void M_Audio::LoadSoundBank(const char* soundbankPath)
{
	std::string fullPath = ASSETS_SOUNDBANKS_PATH;
	fullPath += soundbankPath;
	fullPath += ".bnk";

	AkBankID bankId;

	AKRESULT result = AK::SoundEngine::LoadBank(fullPath.c_str(),bankId);

	if (result == AK_Success)
		LOG("Loaded %s", fullPath.c_str());
}

void M_Audio::UnloadSoundBank(const char* soundbankPath)
{
	std::string fullPath = ASSETS_SOUNDBANKS_PATH;
	fullPath += soundbankPath;
	fullPath += ".bnk";

	AKRESULT result = AK::SoundEngine::UnloadBank(fullPath.c_str(), NULL);

	if (result == AK_Success)
		LOG("Unloaded %s", fullPath.c_str());
}

void M_Audio::FindSoundBanks(std::vector<std::string>* banks)
{
	std::vector<std::string> files;
	std::vector<std::string> dirs;
	App->fileSystem->DiscoverFiles(ASSETS_SOUNDBANKS_PATH, files,dirs);

	std::string extension = "bnk";
	std::string file_extension;
	std::string file_name;
	for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); ++it)
	{
		App->fileSystem->SplitFilePath((*it).c_str(), nullptr, &file_name, &file_extension);
		if (!file_extension.compare(extension)) 
		{
			
			if (!file_name.compare("Init")) 
				continue;

			banks->push_back(file_name);
		}
	}
}

void M_Audio::LoadEventsFromJson()
{
	std::vector<std::string> eventList;
	FindSoundBanks(&eventList);

	std::vector<std::string>::iterator it = eventList.begin();

	for (it; it != eventList.end(); ++it)
	{
		std::string path = ASSETS_SOUNDBANKS_PATH + (*it) + ".json";

		unsigned int id = 0;

		char* buffer = nullptr;
		unsigned int load = App->fileSystem->Load(path.c_str(), &buffer);

		ParsonNode eventJson = ParsonNode(buffer);	

		ParsonNode soundBankInfo = eventJson.GetNode("SoundBanksInfo");

		if (!soundBankInfo.NodeIsValid())
			continue;
			
		ParsonArray soundBanks = soundBankInfo.GetArray("SoundBanks");

		if (!soundBanks.ArrayIsValid())
			continue;

		ParsonNode bankNode = soundBanks.GetNode(0);

		if (!bankNode.NodeIsValid())
			continue;

		ParsonArray eventArray = bankNode.GetArray("IncludedEvents");

		for (unsigned int i = 0; i < eventArray.size; ++i)
		{
			ParsonNode eventsJson = eventArray.GetNode(i);

			std::string eventId = eventsJson.GetString("Id");
			std::string eventName = eventsJson.GetString("Name");

			id = std::stoul(eventId);

			eventMap.insert(std::pair<std::string, unsigned int>(eventName, id));
		}
		
	}
}

// Wwise Object
WwiseObject::WwiseObject(unsigned int id, const char* name)
{
	position.Zero();
	orientationFront.Zero();
	orientationUp.Zero();

	this->name = name;
	objectId = id;

	AK::SoundEngine::RegisterGameObj((AkGameObjectID) id, name);
}

WwiseObject::~WwiseObject()
{
}

void WwiseObject::SetPos(float3 pos, float3 front, float3 up)
{
	position = float3ToAkVector(pos.Normalized());
	orientationFront = float3ToAkVector(front.Normalized());
	orientationUp = float3ToAkVector(up.Normalized());

	AkSoundPosition soundPos;
	soundPos.Set(position, orientationFront, orientationUp);
	AK::SoundEngine::SetPosition(objectId, soundPos);
}

void WwiseObject::PlayEvent(unsigned int eventId)
{
	AK::SoundEngine::PostEvent(eventId, objectId);
}

void WwiseObject::PauseEvent(unsigned int eventId)
{
	AK::SoundEngine::ExecuteActionOnEvent(eventId, AK::SoundEngine::AkActionOnEventType::AkActionOnEventType_Pause, objectId);
}

void WwiseObject::ResumeEvent(unsigned int eventId)
{
	AK::SoundEngine::ExecuteActionOnEvent(eventId, AK::SoundEngine::AkActionOnEventType::AkActionOnEventType_Resume, objectId);
}

void WwiseObject::StopEvent(unsigned int eventId)
{
	AK::SoundEngine::ExecuteActionOnEvent(eventId, AK::SoundEngine::AkActionOnEventType::AkActionOnEventType_Stop, objectId);
}

float WwiseObject::GetVolume()
{
	return volume;
}

void WwiseObject::SetVolume(float volume)
{
	AK::SoundEngine::SetGameObjectOutputBusVolume(objectId, AK_INVALID_GAME_OBJECT, volume);
	this->volume = volume;
}

WwiseObject* WwiseObject::CreateAudioSource(unsigned int id, const char* name, float3 position)
{
	WwiseObject* wwiseObject = new WwiseObject(id, name);
	wwiseObject->SetPos(position);
	App->audio->audioSourceList.push_back(wwiseObject);

	return wwiseObject;
}

WwiseObject* WwiseObject::CreateAudioListener(unsigned int id, const char* name, float3 position)
{
	WwiseObject* wwiseObject = new WwiseObject(id, name);

	AkGameObjectID listenerID = id;
	AK::SoundEngine::SetDefaultListeners(&listenerID, 1);
	wwiseObject->SetPos(position);
	App->audio->audioListenerList.push_back(wwiseObject);

	return wwiseObject;
}

void WwiseObject::SetAudioSwitch(const char* switchGroup, const char* switchState, unsigned int objectId)
{
	AKRESULT result = AK::SoundEngine::SetSwitch(switchGroup, switchState, objectId);
	if (result == AK_Success)
		LOG("Set switch");
}

void WwiseObject::SetAudioTrigger(const char* trigger, unsigned int objectId)
{
	AKRESULT result = AK::SoundEngine::PostTrigger(trigger, objectId);
	if (result == AK_Success)
		LOG("Set trigger");
}

void WwiseObject::SetAudioRtcp(const char* rtpc, int value, unsigned int objectId)
{
	AKRESULT result = AK::SoundEngine::SetRTPCValue(rtpc, value, objectId);
	if (result == AK_Success)
		LOG("Set trigger");
}

unsigned int WwiseObject::GetWwiseObjectId() const
{
	return objectId;
}

AkVector WwiseObject::float3ToAkVector(float3 value)
{
	AkVector temp;

	temp.X = value.x;
	temp.Y = value.y;
	temp.Z = value.z;

	return temp;
}

