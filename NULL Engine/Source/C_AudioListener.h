#ifndef __C_AUDIOLISTENER__
#define __C_AUDIOLISTENER__

#include "Component.h"

class GameObject;
class WwiseObject;

class NULL_API C_AudioListener : public Component
{
public:
	C_AudioListener(GameObject* owner);
	//C_AudioListener(float3 reference);							//Creates and audio listener for the master camera
	~C_AudioListener();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::AUDIOLISTENER; }

private:

	WwiseObject* wwiseObject = nullptr;;
};

#endif // __C_AUDIOLISTENER__