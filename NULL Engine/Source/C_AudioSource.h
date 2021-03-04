#ifndef __C_AUDIOSOURCE__
#define __C_AUDIOSOURCE__

#include "Component.h"

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

	void SetId(unsigned int id);
	const unsigned int GetId();

	static inline ComponentType GetType() { return ComponentType::AUDIOSOURCE; }

	void PlayFx(unsigned int eventId);
	void PauseFx(unsigned int eventId);
	void ResumeFx(unsigned int eventId);
	void StopFx(unsigned int eventId);

	void SetVolume(float volume);
	
private:

	WwiseObject* wwiseObject = nullptr;
	unsigned int id = 0;
};

#endif // __C_AUDIOSOURCE__