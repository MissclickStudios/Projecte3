#include "Channel.h"

Channel::Channel()
{
	name = "[NONE]";
}

Channel::Channel(const char* name)
{
	this->name = (name != nullptr) ? name : "[NONE]";
}

bool Channel::HasKeyframes(KEYFRAME_TYPE type) const
{
	switch (type)
	{
	case KEYFRAME_TYPE::POSITION:	{ return !(position_keyframes.size() == 1 && position_keyframes.begin()->first == -1); }	break;
	case KEYFRAME_TYPE::ROTATION:	{ return !(rotation_keyframes.size() == 1 && rotation_keyframes.begin()->first == -1); }	break;
	case KEYFRAME_TYPE::SCALE:		{ return !(scale_keyframes.size() == 1 && scale_keyframes.begin()->first == -1); }			break;
	}

	return false;
}

bool Channel::HasPositionKeyframes() const
{
	return HasKeyframes(KEYFRAME_TYPE::POSITION);
}

bool Channel::HasRotationKeyframes() const
{
	return HasKeyframes(KEYFRAME_TYPE::ROTATION);
}

bool Channel::HasScaleKeyframes() const
{
	return HasKeyframes(KEYFRAME_TYPE::SCALE);
}

PositionKeyframe Channel::GetPositionKeyframe(const double& keyframe) const
{
	PositionKeyframe item	= position_keyframes.find(keyframe);
	item					= (item == position_keyframes.end()) ? --item : item;

	return item;
}

RotationKeyframe Channel::GetRotationKeyframe(const double& keyframe) const
{
	RotationKeyframe item	= rotation_keyframes.find(keyframe);
	item					= (item == rotation_keyframes.end()) ? --item : item;

	return item;
}

ScaleKeyframe Channel::GetScaleKeyframe(const double& keyframe) const
{
	ScaleKeyframe item	= scale_keyframes.find(keyframe);
	item				= (item == scale_keyframes.end()) ? --item : item;

	return item;
}

PositionKeyframe Channel::GetPrevPositionKeyframe(const double& keyframe) const
{
	PositionKeyframe item	= GetPositionKeyframe(keyframe);
	item					= (item != position_keyframes.begin()) ? --item : item;

	return item;
}

RotationKeyframe Channel::GetPrevRotationKeyframe(const double& keyframe) const
{
	RotationKeyframe item	= GetRotationKeyframe(keyframe);
	item					= (item != rotation_keyframes.begin()) ? --item : item;

	return item;
}

ScaleKeyframe Channel::GetPrevScaleKeyframe(const double& keyframe) const
{
	ScaleKeyframe item	= GetScaleKeyframe(keyframe);
	item				= (item != scale_keyframes.begin()) ? --item : item;

	return item;
}

PositionKeyframe Channel::GetNextPositionKeyframe(const double& keyframe) const
{
	PositionKeyframe item	= GetPositionKeyframe(keyframe);
	item					= (item != position_keyframes.end()) ? ++item : item;

	return item;
}

RotationKeyframe Channel::GetNextRotationKeyframe(const double& keyframe) const
{
	RotationKeyframe item	= GetRotationKeyframe(keyframe);
	item					= (item != rotation_keyframes.end()) ? ++item : item;

	return item;
}

ScaleKeyframe Channel::GetNextScaleKeyframe(const double& keyframe) const
{
	ScaleKeyframe item	= GetScaleKeyframe(keyframe);
	item				= (item != scale_keyframes.end()) ? ++item : item;

	return item;
}

PositionKeyframe Channel::GetClosestPrevPositionKeyframe(const double& keyframe) const
{
	PositionKeyframe item	= position_keyframes.lower_bound(keyframe);
	item					= (item != position_keyframes.begin()) ? --item : item;

	return item;
}

RotationKeyframe Channel::GetClosestPrevRotationKeyframe(const double& keyframe) const
{
	RotationKeyframe item	= rotation_keyframes.lower_bound(keyframe);
	item					= (item != rotation_keyframes.begin()) ? --item : item;
	
	return item;
}

ScaleKeyframe Channel::GetClosestPrevScaleKeyframe(const double& keyframe) const
{
	ScaleKeyframe item	= scale_keyframes.lower_bound(keyframe);
	item				= (item != scale_keyframes.begin()) ? --item : item;

	return item;
}

PositionKeyframe Channel::GetClosestNextPositionKeyframe(const double& keyframe) const
{
	PositionKeyframe item	= position_keyframes.upper_bound(keyframe);
	item					= (item != position_keyframes.end()) ? item : --item;

	return item;
}

RotationKeyframe Channel::GetClosestNextRotationKeyframe(const double& keyframe) const
{
	RotationKeyframe item	= rotation_keyframes.upper_bound(keyframe);
	item					= (item != rotation_keyframes.end()) ? item : --item;

	return item;
}

ScaleKeyframe Channel::GetClosestNextScaleKeyframe(const double& keyframe) const
{
	ScaleKeyframe item	= scale_keyframes.upper_bound(keyframe);
	item				= (item != scale_keyframes.end()) ? item : --item;

	return item;
}