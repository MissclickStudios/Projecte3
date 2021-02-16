#ifndef __C_ANIMATION_H__
#define __C_ANIMATION_H__

#include <vector>

#include "AnimationClip.h"

#include "Component.h"

class ParsonNode;
class GameObject;
struct AnimationClip;

class C_Animation : public Component
{
public:
	C_Animation(GameObject* owner);
	~C_Animation();

	bool Update		() override;
	bool CleanUp	() override;

	bool SaveState	(ParsonNode& root) const override;
	bool LoadState	(ParsonNode& root) override;

	static inline COMPONENT_TYPE GetType() { return COMPONENT_TYPE::ANIMATION; }

public:
	void AddClip(const AnimationClip& animation_clip);

private:
	std::vector<AnimationClip> clips;

};

#endif // !__C_ANIMATION_H__