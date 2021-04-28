#ifndef __C_2DANIMATOR__
#define __C_2DANIMATOR__

#include "Component.h"
#include "Timer.h"
#include <vector>

class GameObject;
class R_Texture;

typedef unsigned __int32 uint;

class MISSCLICK_API C_2DAnimator : public Component
{
public:
	C_2DAnimator(GameObject* owner);
	~C_2DAnimator();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	//Plays the animation
	void PlayAnimation(bool loop, int animationNumber);

	//Stops current animation
	void StopAnimation();

	//Gets the current id texture of the animation frame
	uint GetIdFromAnimation();

	void SetAnimationStepTime(int time);
	int GetAnimationStepTime();

	bool IsAnimationPlaying();

	void SetAnimationPlayFromStart(bool x);
	bool GetAnimationPlayFromStart();
	
	const char* GetName(int animationNum);
	void ChangeName(const char* name, int animationNum);

	void GetAnimationSprites(const char* name, int animationDestination);

	static inline ComponentType GetType() { return ComponentType::ANIMATOR2D; }  

private:
	uint GetTextureIdFromVector(int index, int animationNum);
	void LoopAnimation(int animation);
	int							animationNumberPlaying;
private:

	std::vector<R_Texture*>		animation;
	std::vector<R_Texture*>		animation1;
	std::vector<R_Texture*>		animation2;
	
	int							animationCounter;
	uint						animationStepTime;

	Timer						animationTimer;

	bool						animationLoop;
	bool						animationPlaying;
	bool						playAnimation;

	bool						playFromTheStartOnLoop;

	uint						currentFrameIdTexture;

	std::string					name;
	std::string					name1;
	std::string					name2;
};

#endif // __C_2DANIMATOR__