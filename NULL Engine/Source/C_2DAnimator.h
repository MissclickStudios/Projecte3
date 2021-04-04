#ifndef __C_2DANIMATOR__
#define __C_2DANIMATOR__

#include "Component.h"
#include <vector>

class GameObject;
class R_Texture;

class NULL_API C_2DAnimator : public Component
{
public:
	C_2DAnimator(GameObject* owner);
	~C_2DAnimator();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::AUDIOLISTENER; }

private:
	std::vector<R_Texture*>		animation;
	int							AnimationCount;

};

#endif // __C_2DANIMATOR__