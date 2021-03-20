#ifndef __C_GATEBEHAVIOR__
#define __C_GATEBEHAVIOR__

#include "Component.h"

class C_GateBehavior : public Component
{
public:

	C_GateBehavior(GameObject* owner);
	~C_GateBehavior();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::GATE_BEHAVIOR; }			// This is needed to be able to use templates for functions such as GetComponent<>();

	void OnCollisionEnter();
};

#endif
