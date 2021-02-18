#include "M_Physics.h"

M_Physics::M_Physics(bool isActive) : Module("physics", isActive)
{
}

M_Physics::~M_Physics()
{
}

bool M_Physics::Init(ParsonNode& root)
{
	return true;
}

bool M_Physics::Start()
{
	return true;
}

UpdateStatus M_Physics::Update(float dt)
{
	return UpdateStatus::CONTINUE;
}

bool M_Physics::CleanUp()
{
	return true;
}

bool M_Physics::LoadConfiguration(ParsonNode& configuration)
{
	return true;
}

bool M_Physics::SaveConfiguration(ParsonNode& configuration) const
{
	return true;
}
