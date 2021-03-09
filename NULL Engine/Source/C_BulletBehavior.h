#ifndef __C_BULLETBEHAVIOR__
#define __C_BULLETBEHAVIOR__

#include "Component.h"

class C_BulletBehavior : public Component
{
public:

	C_BulletBehavior(GameObject* owner);
	~C_BulletBehavior();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::BULLET_BEHAVIOR; }			// This is needed to be able to use templates for functions such as GetComponent<>();

	void OnCollisionEnter();
};

#endif