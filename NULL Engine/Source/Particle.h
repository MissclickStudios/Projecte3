#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include "Globals.h"
#include "Color.h"

#include "Dependencies/MathGeoLib/include/Math/float4x4.h"
#include "Dependencies/MathGeoLib/include/Math/float3.h"
#include "Dependencies/MathGeoLib/include/Math/Quat.h"

class C_Camera;

enum class Billboarding 
{
	SCREEN_ALIGNED,
	WORLD_ALIGNED,
	AXIS_ALIGNED,
	NONE
};

class Particle
{
public:
	Particle();
	Particle(const Particle& particle);	
	~Particle();

	void Update(float dt);

	/*void ScreenAligned(C_Camera* currentCamera = nullptr);
	void WorldAligned(C_Camera* currentCamera = nullptr);
	void AxisAligned(C_Camera* currentCamera = nullptr);

	void ApplyBillboarding(Billboarding type = Billboarding::NONE);*/

	float4x4* GetTransformAsPtr();

public:
	float4x4 transform;
	float3 position;
	Quat worldRotation;

	float maxLifetime;
	float currentLifetime = 0;  
	float normalizedLifetime;

	float rotation;

	float3 movementDirection;
	float movementIntensity = 0.0f;
	float3 velocity;
	float3 acceleration;

	float size = 1.0f;
	Color color;
	float distanceToCamera;
	bool usesBillboarding;
};

#endif // !__PARTICLE_H__