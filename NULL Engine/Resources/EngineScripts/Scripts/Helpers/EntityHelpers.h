#include "Effect.h"

#define DEFAULT_BLEND_TIME 0.2f
struct AnimationInfo
{
	std::string name	= "None";
	std::string track	= "Preview";
	float blendTime		= DEFAULT_BLEND_TIME;
	float duration = -1.0f;

	void SetDuration(float duration)
	{
		this->duration = duration;
	}
};