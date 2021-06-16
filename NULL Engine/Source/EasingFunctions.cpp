#include "Macros.h"
#include "VariableTypedefs.h"

#include "EasingFunctions.h"

#include "MemoryManager.h"

#define NULL 0																								// Required by mmgr new.

using namespace EasingFunctions;																			// Not the cleanest but it will only be employed in this file.

float EasingFunctions::Lineal(float start, float end, float rate)
{
	return start + ((end - start) * rate);
}

float* EasingFunctions::Lineal(float* start, float* end, float rate, const uint size)
{	
	float* tmp = new float[size];

	for (uint i = 0; i < size; ++i)
	{
		tmp[i] = start[i] + ((end[i] - start[i]) * rate);
	}
	
	return tmp;
}

float3 EasingFunctions::Lineal(const float3& start, const float3& end, float rate)
{
	float3 tmp(float3::zero);

	tmp.x = start.x + ((end.x - start.x) * rate);
	tmp.y = start.y + ((end.y - start.y) * rate);
	tmp.z = start.z + ((end.z - start.z) * rate);
	
	return tmp;
}

float4 EasingFunctions::Lineal(const float4& start, const float4& end, float rate)
{
	float4 tmp(float4::zero);

	tmp.x = start.x + ((end.x - start.x) * rate);
	tmp.y = start.y + ((end.y - start.y) * rate);
	tmp.z = start.z + ((end.z - start.z) * rate);
	tmp.w = start.w + ((end.w - start.w) * rate);

	return tmp;
}

Quat EasingFunctions::Lineal(const Quat& start, const Quat& end, float rate)
{
	return Quat();
}

float EasingFunctions::SmoothStart(float start, float end, float rate)
{
	return start + ((end - start) * rate * rate);
}

float EasingFunctions::SmoothStop(float start, float end, float rate)
{
	return start + ((end - start) * (1 - ((1 - rate) * (1 - rate))));
}

float3 EasingFunctions::SmoothStart(const float3& start, const float3& end, float rate)
{
	float3 tmp = float3::zero;

	tmp.x = start.x + ((end.x - start.x) * rate * rate);
	tmp.y = start.y + ((end.y - start.y) * rate * rate);
	tmp.z = start.z + ((end.z - start.z) * rate * rate);
	
	return tmp;
}

float3 EasingFunctions::SmoothStop(const float3& start, const float3& end, float rate)
{
	float3 tmp = float3::zero;

	tmp.x = start.x + ((end.x - start.x) * (1 - ((1 - rate) * (1 - rate))));
	tmp.y = start.y + ((end.y - start.y) * (1 - ((1 - rate) * (1 - rate))));
	tmp.z = start.z + ((end.z - start.z) * (1 - ((1 - rate) * (1 - rate))));

	return tmp;
}
