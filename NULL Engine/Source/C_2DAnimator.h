#ifndef __C_2DANIMATOR__
#define __C_2DANIMATOR__

#include "Component.h"

class GameObject;
class WwiseObject;

class NULL_API C_2DAnimator : public Component
{
public:
	C_2DAnimator(GameObject* owner);
	//C_AudioListener(float3 reference);							//Creates and audio listener for the master camera
	~C_2DAnimator();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::AUDIOLISTENER; }

private:

	WwiseObject* wwiseObject = nullptr;;
};

#endif // __C_2DANIMATOR__