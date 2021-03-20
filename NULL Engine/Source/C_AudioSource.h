#ifndef __C_AUDIOSOURCE__
#define __C_AUDIOSOURCE__

#include "Component.h"

#include <string>

class GameObject;
class WwiseObject;

class NULL_API C_AudioSource : public Component
{
public:
	C_AudioSource(GameObject* owner);
	~C_AudioSource();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	void SetEvent(std::string name, unsigned int id);
	void GetEvent(std::string* name, unsigned int* id) const;
	const std::string& GetEventName() const;
	unsigned int GetEventId() const;

	static inline ComponentType GetType() { return ComponentType::AUDIOSOURCE; }

	void PlayFx(unsigned int eventId);
	void PlayFx(std::string name);

	void PauseFx(unsigned int eventId);
	void ResumeFx(unsigned int eventId);
	void StopFx(unsigned int eventId);

	float GetVolume();
	void SetVolume(float volume);
	
	bool isPlaying = false;
	bool isPause = false;

private:

	WwiseObject* wwiseObject = nullptr;
	std::string eventName;
	unsigned int eventId;

};

#endif // __C_AUDIOSOURCE__