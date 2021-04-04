#include "R_2DAnimation.h"

R_2DAnimation::R_2DAnimation() : Resource(ResourceType::NONE)
{
}

R_2DAnimation::~R_2DAnimation()
{
}

bool R_2DAnimation::CleanUp()
{
	return true;
}

bool R_2DAnimation::SaveMeta(ParsonNode& metaRoot) const
{
	return true;
}

bool R_2DAnimation::LoadMeta(const ParsonNode& metaRoot)
{
	return true;
}

const char* R_2DAnimation::GetName() const
{
	return name.c_str();
}

double R_2DAnimation::GetDuration() const
{
	return duration;
}

void R_2DAnimation::SetName(const char* name)
{
}

void R_2DAnimation::SetDuration(const double& duration)
{
}
