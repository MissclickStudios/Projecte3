#ifndef __C_2DANIMATOR__
#define __C_2DANIMATOR__

#include "Component.h"
#include "Timer.h"
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

	//Plays the animation
	void PlayAnimation(bool loop);

	//Adds a texture to animation vector
	void AddTexture(R_Texture* tex);

	//Gets the current id texture of the animation frame
	uint GetIdFromAnimation();

	void SetAnimationStepTime(int time);
	int GetAnimationStepTime();

	void GetAnimationSprites(const char* name);

	static inline ComponentType GetType() { return ComponentType::ANIMATOR2D; }  

private:
	uint GetTextureIdFromVector(int index);

private:

	std::vector<R_Texture*>		animation;
	
	int							animationCounter;
	int							animationFrames;
	uint						animationStepTime;

	Timer						animationTimer;

	bool						animationLoop;
	bool						animationPlaying;
	bool						playAnimation;

	uint						currentFrameIdTexture;
};

#endif // __C_2DANIMATOR__