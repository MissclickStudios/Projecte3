#include "Channel.h"

Channel::Channel()
{
	name = "[NONE]";
}

Channel::Channel(const char* name)
{
	this->name = (name != nullptr) ? name : "[NONE]";
}

bool Channel::HasKeyframes(KeyframeType type) const
{
	switch (type)
	{
	case KeyframeType::POSITION:	{ return !(positionKeyframes.size() == 1 && positionKeyframes.begin()->first == -1); }	break;
	case KeyframeType::ROTATION:	{ return !(rotationKeyframes.size() == 1 && rotationKeyframes.begin()->first == -1); }	break;
	case KeyframeType::SCALE:		{ return !(scaleKeyframes.size() == 1 && scaleKeyframes.begin()->first == -1); }			break;
	}

	return false;
}

bool Channel::HasPositionKeyframes() const
{
	return HasKeyframes(KeyframeType::POSITION);
}

bool Channel::HasRotationKeyframes() const
{
	return HasKeyframes(KeyframeType::ROTATION);
}

bool Channel::HasScaleKeyframes() const
{
	return HasKeyframes(KeyframeType::SCALE);
}

PositionKeyframe Channel::GetPositionKeyframe(double keyframe) const
{
	PositionKeyframe item	= positionKeyframes.find(keyframe);
	item					= (item == positionKeyframes.end()) ? --item : item;

	return item;
}

RotationKeyframe Channel::GetRotationKeyframe(double keyframe) const
{
	RotationKeyframe item	= rotationKeyframes.find(keyframe);
	item					= (item == rotationKeyframes.end()) ? --item : item;

	return item;
}

ScaleKeyframe Channel::GetScaleKeyframe(double keyframe) const
{
	ScaleKeyframe item	= scaleKeyframes.find(keyframe);
	item				= (item == scaleKeyframes.end()) ? --item : item;

	return item;
}

PositionKeyframe Channel::GetPrevPositionKeyframe(double keyframe) const
{
	PositionKeyframe item	= GetPositionKeyframe(keyframe);
	item					= (item != positionKeyframes.begin()) ? --item : item;

	return item;
}

RotationKeyframe Channel::GetPrevRotationKeyframe(double keyframe) const
{
	RotationKeyframe item	= GetRotationKeyframe(keyframe);
	item					= (item != rotationKeyframes.begin()) ? --item : item;

	return item;
}

ScaleKeyframe Channel::GetPrevScaleKeyframe(double keyframe) const
{
	ScaleKeyframe item	= GetScaleKeyframe(keyframe);
	item				= (item != scaleKeyframes.begin()) ? --item : item;

	return item;
}

PositionKeyframe Channel::GetNextPositionKeyframe(double keyframe) const
{
	PositionKeyframe item	= GetPositionKeyframe(keyframe);
	item					= (item != positionKeyframes.end()) ? ++item : item;

	return item;
}

RotationKeyframe Channel::GetNextRotationKeyframe(double keyframe) const
{
	RotationKeyframe item	= GetRotationKeyframe(keyframe);
	item					= (item != rotationKeyframes.end()) ? ++item : item;

	return item;
}

ScaleKeyframe Channel::GetNextScaleKeyframe(double keyframe) const
{
	ScaleKeyframe item	= GetScaleKeyframe(keyframe);
	item				= (item != scaleKeyframes.end()) ? ++item : item;

	return item;
}

PositionKeyframe Channel::GetClosestPrevPositionKeyframe(double keyframe) const
{
	PositionKeyframe item	= positionKeyframes.lower_bound(keyframe);
	item					= (item != positionKeyframes.begin()) ? --item : item;

	return item;
}

RotationKeyframe Channel::GetClosestPrevRotationKeyframe(double keyframe) const
{
	RotationKeyframe item	= rotationKeyframes.lower_bound(keyframe);
	item					= (item != rotationKeyframes.begin()) ? --item : item;
	
	return item;
}

ScaleKeyframe Channel::GetClosestPrevScaleKeyframe(double keyframe) const
{
	ScaleKeyframe item	= scaleKeyframes.lower_bound(keyframe);
	item				= (item != scaleKeyframes.begin()) ? --item : item;

	return item;
}

PositionKeyframe Channel::GetClosestNextPositionKeyframe(double keyframe) const
{
	PositionKeyframe item	= positionKeyframes.upper_bound(keyframe);
	item					= (item != positionKeyframes.end()) ? item : --item;

	return item;
}

RotationKeyframe Channel::GetClosestNextRotationKeyframe(double keyframe) const
{
	RotationKeyframe item	= rotationKeyframes.upper_bound(keyframe);
	item					= (item != rotationKeyframes.end()) ? item : --item;

	return item;
}

ScaleKeyframe Channel::GetClosestNextScaleKeyframe(double keyframe) const
{
	ScaleKeyframe item	= scaleKeyframes.upper_bound(keyframe);
	item				= (item != scaleKeyframes.end()) ? item : --item;

	return item;
}