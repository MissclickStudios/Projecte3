#ifndef __C_2DANIMATOR__
#define __C_2DANIMATOR__

#include "Component.h"
#include <vector>

class GameObject;
class R_Texture;

typedef unsigned __int32 uint;

class NULL_API C_2DAnimator : public Component
{
public:
	C_2DAnimator(GameObject* owner);
	~C_2DAnimator();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	uint GetIdFromAnimation();

	static inline ComponentType GetType() { return ComponentType::AUDIOLISTENER; }

private:
	uint GetTextureIdFromVector(int index);

private:

	std::vector<R_Texture*>		animation;
	int							animationCounter;
	int							animationFrames;
	float						animationStepTime;

	bool						animationLoop;
	bool						animationPlating;
};

#endif // __C_2DANIMATOR__