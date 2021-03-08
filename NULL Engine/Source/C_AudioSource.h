#ifndef __C_AUDIOSOURCE__
#define __C_AUDIOSOURCE__

#include "Component.h"

#include <utility>
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
	const std::pair<std::string, unsigned int> GetEvent();

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
	std::pair<std::string, unsigned int> eventInfo;

};

#endif // __C_AUDIOSOURCE__