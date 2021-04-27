#ifndef __C_ANIMATION_H__
#define __C_ANIMATION_H__

#include <vector>

#include "AnimationClip.h"

#include "Component.h"

class ParsonNode;
class GameObject;
struct AnimationClip;

class MISSCLICK_API C_Animation : public Component
{
public:
	C_Animation(GameObject* owner);
	~C_Animation();

	bool Update		() override;
	bool CleanUp	() override;

	bool SaveState	(ParsonNode& root) const override;
	bool LoadState	(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::ANIMATION; }

public:
	void AddClip(const AnimationClip& animationClip);

private:
	std::vector<AnimationClip> clips;

};

#endif // !__C_ANIMATION_H__