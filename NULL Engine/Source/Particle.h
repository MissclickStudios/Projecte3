#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include "Globals.h"

#include "Dependencies/MathGeoLib/include/Math/float3.h"
#include "Dependencies/MathGeoLib/include/Math/Quat.h"

class Particle
{
public:
	Particle();
	Particle(const Particle& particle);	
	~Particle();

	void Update(float dt);

public:
	float3 position;

};

#endif // !__PARTICLE_H__