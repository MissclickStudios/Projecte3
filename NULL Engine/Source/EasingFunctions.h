#ifndef __EASING_FUNCTIONS_H__
#define __EASING_FUNCTIONS_H__

namespace EasingFunctions
{
	float Lineal		(float start, float end, float rate);			// LERP. Returns the interpolation between two values. Will be constant each frame.
	float SmoothStart	(float start, float end, float rate);			// NON-LERP. Will return higher values towards the end of the interpolation. (Ex: Slow to Fast)
	float SmoothStop	(float start, float end, float rate);			// NON-LERP. Will return lower values towards the end of the interpolation. (Ex: Fast to Slow)
}

#endif // !__EASING_FUNCTIONS_H__