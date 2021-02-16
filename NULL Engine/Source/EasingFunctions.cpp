#include "EasingFunctions.h"

float EasingFunctions::Lineal(float start, float end, float rate)
{
	float increment = (end - start) * rate;

	return start + increment;
}

float EasingFunctions::SmoothStart(float start, float end, float rate)
{
	float increment = (end - start) * rate * rate;

	return start + increment;
}

float EasingFunctions::SmoothStop(float start, float end, float rate)
{
	float r = 1 - ((1 - rate) * (1 - rate));
	float increment = (end - start) * r;

	return start + increment;
}