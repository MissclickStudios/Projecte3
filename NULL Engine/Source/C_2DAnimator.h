#ifndef __C_2DANIMATOR__
#define __C_2DANIMATOR__

#include "Component.h"
#include "Timer.h"
#include <vector>

class GameObject;
class R_Texture;
class Spritesheet;
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

	void SetAnimationStepTime(int time);
	int GetAnimationStepTime();

	bool IsAnimationPlaying();

	//void SetSpritesheetTexture(R_Texture* spritesheet, int animationNumber);

	void SetAnimationPlayFromStart(bool x);
	bool GetAnimationPlayFromStart();
	
	const char* GetName(int animationNum);
	void ChangeName(const char* name, int animationNum);

	void GetAnimationSprites(const char* inputName, int animationDestination);
	void GetAnimationSprites(const char* inputName, int animationDestination, R_Texture* texture);

	static inline ComponentType GetType() { return ComponentType::ANIMATOR2D; }  

	Spritesheet*				spritesheet;
	Spritesheet*				spritesheet2;
	Spritesheet*				spritesheet3;

	int GetAnimationNumber();

	
private:
	void LoopAnimation(int animation);
private:
	
	int							animationCounter;
	uint						animationStepTime;

	int							animationNumberPlaying;

	Timer						animationTimer;

	bool						animationLoop;
	bool						animationPlaying;
	bool						playAnimation;

	bool						playFromTheStartOnLoop;

	uint						currentFrameIdTexture;

	std::string					name = "";
	std::string					name1 = "";
	std::string					name2 = "";
};

#endif // __C_2DANIMATOR__