#pragma once

enum class EntityType
{
	ENTITY,
	PLAYER,
	BLURG
};

#include "Effect.h"

#define DEFAULT_BLEND_TIME 0.2f
struct AnimationInfo
{
	std::string name = "None";
	float blendTime = DEFAULT_BLEND_TIME;
};