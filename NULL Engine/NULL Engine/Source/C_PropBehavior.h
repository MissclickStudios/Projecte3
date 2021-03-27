#ifndef __C_PROPBEHAVIOR__
#define __C_PROPBEHAVIOR__

#include "Component.h"

class C_PropBehavior : public Component
{
public:

	C_PropBehavior(GameObject* owner);
	~C_PropBehavior();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::PROP_BEHAVIOR; }			// This is needed to be able to use templates for functions such as GetComponent<>();

	void OnCollisionEnter();

private:

	bool dead = false;
};

#endif
