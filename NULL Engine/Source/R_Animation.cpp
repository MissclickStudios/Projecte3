#include "JSONParser.h"

#include "Channel.h"

#include "R_Animation.h"

R_Animation::R_Animation() : Resource(ResourceType::ANIMATION),
name				("[NONE]"),
duration			(0.0f),
ticksPerSecond		(0.0f)
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
		channels[i].positionKeyframes.clear();
		channels[i].rotationKeyframes.clear();
		channels[i].scaleKeyframes.clear();
	}

	channels.clear();

	return ret;
}

bool R_Animation::SaveMeta(ParsonNode& metaRoot) const
{
	bool ret = true;

	ParsonArray containedArray = metaRoot.SetArray("ContainedResources");

	ParsonNode settings = metaRoot.SetNode("ImportSettings");
	animationSettings.Save(settings);

	return ret;
}

bool R_Animation::LoadMeta(const ParsonNode& metaRoot)
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
	return ticksPerSecond;
}

void R_Animation::SetName(const char* name)
{
	this->name = name;
}

void R_Animation::SetDuration(const double& duration)
{
	this->duration = duration;
}

void R_Animation::SetTicksPerSecond(const double& ticksPerSecond)
{
	this->ticksPerSecond = ticksPerSecond;
}
