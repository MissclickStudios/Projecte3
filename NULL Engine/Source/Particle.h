#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include "Globals.h"

#include "Dependencies/MathGeoLib/include/Math/float3.h"
#include "Dependencies/MathGeoLib/include/Math/Quat.h"

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

	void ApplyBillboarding(Billboarding type = Billboarding::NONE);

public:
	float3 position;
	Quat worldRotation;

	float relativeLifetime;  //Lifetime in range [0, 1]

	float rotation;
	float4 velocity;
	float size = 1.0f;
	float color;
	float distanceToCamera;
	bool usesBillboarding;
};

#endif // !__PARTICLE_H__