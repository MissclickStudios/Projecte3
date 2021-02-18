#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include "Module.h"

class M_Physics : public Module
{
public:

	M_Physics(bool isActive = true);
	~M_Physics();

	bool Init(ParsonNode& root) override;
	bool Start() override;
	UpdateStatus Update(float dt) override;
	bool CleanUp() override;

	bool LoadConfiguration(ParsonNode& configuration) override;
	bool SaveConfiguration(ParsonNode& configuration) const override;
};

#endif //__PHYSICS_H__