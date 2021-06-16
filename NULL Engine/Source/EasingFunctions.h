#ifndef __EASING_FUNCTIONS_H__
#define __EASING_FUNCTIONS_H__

#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/float4.h"
#include "MathGeoLib/include/Math/Quat.h"

#include "Macros.h"

typedef unsigned int uint;

namespace EasingFunctions
{
	float	Lineal		(float start, float end, float rate);							// LERP. Returns the interpolation between two values. Will be const each frame.
	float*	Lineal		(float* start, float* end, float rate, const uint size = 3);	// 
	float3	Lineal		(const float3& start, const float3& end, float rate);			// 
	float4	Lineal		(const float4& start, const float4& end, float rate);			// 
	Quat	Lineal		(const Quat& start, const Quat& end, float rate);				// 

	float	SmoothStart	(float start, float end, float rate);	// AKA EaseIn			// NON-LERP. Will return higher values towards the end of the interpolation.	(Ex: Slow to Fast)
	float	SmoothStop	(float start, float end, float rate);	// AKA EaseOut			// NON-LERP. Will return lower values towards the end of the interpolation.		(Ex: Fast to Slow)
	float3	SmoothStart	(const float3& start, const float3& end, float rate);
	float3	SmoothStop	(const float3& start, const float3& end, float rate);
}

#endif // !__EASING_FUNCTIONS_H__