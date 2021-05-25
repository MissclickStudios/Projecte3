#include "Effect.h"

#define DEFAULT_BLEND_TIME 0.2f
struct AnimationInfo
{
	std::string name	= "None";
	std::string track	= "Preview";
	float blendTime		= DEFAULT_BLEND_TIME;
};