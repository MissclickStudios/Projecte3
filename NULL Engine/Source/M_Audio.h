#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "Module.h"

#include <string>
#include <vector>

#include "MathGeoLib/include/Math/float3.h"
#include "AK/SoundEngine/Common/AkTypes.h"

class WwiseObject
{
public:
	WwiseObject(unsigned int id, const char* name);
	~WwiseObject();

	void		SetPos(float3 pos, float3 front = { 0,0,1 }, float3 up = { 0,1,0 });

	void		PlayEvent(unsigned int eventId);
	void		PauseEvent(unsigned int eventId);
	void		ResumeEvent(unsigned int eventId);
	void		StopEvent(unsigned int eventId);

	void		SetVolume(float volume);

	WwiseObject* CreateAudioSource(unsigned int id, const char* name, float3 position);
	WwiseObject* CreateAudioListener(unsigned int id, const char* name, float3 position);

	void		SetAudioSwitch(const char* switchGroup, const char* switchState, unsigned int objectId);	//Group of the current switch (Same name as in Wwise), State in the current group
	void		SetAudioTrigger(const char* trigger, unsigned int objectId);
	void		SetAudioRtcp(const char* rtpc, int value, unsigned int objectId);

	unsigned int GetId() const;

	AkVector	float3ToAkVector(float3 value);

private:

	std::string name;
	unsigned int objectId;
	float volume = 0;
	AkVector position;
	AkVector orientationFront;
	AkVector orientationUp;

};

class M_Audio : public Module
{
public:

	M_Audio(bool isActive = true);
	~M_Audio();

	bool		Init(ParsonNode& root) override;
	bool		Start() override;
	UpdateStatus Update(float dt) override;
	bool		CleanUp() override;

	bool		InitSoundEngine();
	void		TermSoundEngine();

	void		SetWwiseState(const char* stateGroup, const char* state);

	void		PauseAll();
	void		ResumeAll();
	void		StopAll();

	void		LoadSoundBank(const char* soundbankPath);
	void		UnloadSoundBank(const char* soundbankPath);

	void		FindSoundBanks(std::vector<std::string>* banks);
	void		LoadEventsFromJson();

	std::map <std::string, unsigned int> eventMap;
	std::vector<WwiseObject*> audioListenerList;
	std::vector<WwiseObject*> audioSourceList;

private:

};

#endif //__AUDIO_H__