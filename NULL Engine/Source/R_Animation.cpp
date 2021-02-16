#include "JSONParser.h"

#include "Channel.h"

#include "R_Animation.h"

R_Animation::R_Animation() : Resource(RESOURCE_TYPE::ANIMATION),
name				("[NONE]"),
duration			(0.0f),
ticks_per_second	(0.0f)
{

}

R_Animation::~R_Animation()
{

}

bool R_Animation::CleanUp()
{
	bool ret = true;

	for (uint i = 0; i < channels.size(); ++i)
	{
		channels[i].position_keyframes.clear();
		channels[i].rotation_keyframes.clear();
		channels[i].scale_keyframes.clear();
	}

	channels.clear();

	return ret;
}

bool R_Animation::SaveMeta(ParsonNode& meta_root) const
{
	bool ret = true;

	ParsonArray contained_array = meta_root.SetArray("ContainedResources");

	ParsonNode settings = meta_root.SetNode("ImportSettings");
	animation_settings.Save(settings);

	return ret;
}

bool R_Animation::LoadMeta(const ParsonNode& meta_root)
{
	bool ret = true;



	return ret;
}

// --- R_ANIMATION METHODS
const char* R_Animation::GetName() const
{
	return name.c_str();
}

double R_Animation::GetDuration() const
{
	return duration;
}

double R_Animation::GetTicksPerSecond() const
{
	return ticks_per_second;
}

void R_Animation::SetName(const char* name)
{
	this->name = name;
}

void R_Animation::SetDuration(const double& duration)
{
	this->duration = duration;
}

void R_Animation::SetTicksPerSecond(const double& ticks_per_second)
{
	this->ticks_per_second = ticks_per_second;
}
