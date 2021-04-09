#ifndef __M_PARTICLE_SYSTEM_H__
#define __M_PARTICLE_SYSTEM_H__

#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/float4.h"
#include "MathGeoLib/include/Algorithm/Random/LCG.h"
#include "Macros.h"
#include "Color.h"

class Particle;
class EmitterInstance;

struct NULL_API M_ParticleSystem														// This should be a class that inherits from the Module class. Idea: Separate the PS modules.
{
	enum Type
	{
		None,
		ParticleMovement,
		ParticleColor,
		ParticleLifetime,
		ParticleRotation,
		ParticleSize,
		ParticleBillboarding,
		EmitterBase,		//Origin of the emitter
		EmitterSpawn,		//Spawn Rate and timer
		EmitterArea,
		
		Unknown
	} type;

	M_ParticleSystem(Type type) : type(type) {};

	virtual void Spawn(EmitterInstance* emitter, Particle* particle) = 0;
	virtual void Update(float dt, EmitterInstance* emitter) = 0;

	LCG randomGenerator;
	//virtual void Save / Load
};

struct EmitterBase : M_ParticleSystem
{
	//camera alignment

	EmitterBase() : M_ParticleSystem(Type::EmitterBase) {};

	void Spawn(EmitterInstance* emitter, Particle* particle);	//Loop active particles and update them
	void Update(float dt, EmitterInstance* emitter);

	float3 origin = float3::zero;
};

struct EmitterSpawn : M_ParticleSystem
{
	EmitterSpawn() : M_ParticleSystem(Type::EmitterSpawn) {};

	void Spawn(EmitterInstance* emitter, Particle* particle);
	void Update(float dt, EmitterInstance* emitter);			//spawn ratio and timer management

	float spawnRatio = 0.05f;
	float timer = 0.0f;
};

struct ParticleMovement : M_ParticleSystem
{
	ParticleMovement() : M_ParticleSystem(Type::ParticleMovement) {};

	void Spawn(EmitterInstance* emitter, Particle* particle);	//random pos between initialPosition1 and 2
	void Update(float dt, EmitterInstance* emitter);

	float initialIntensity1 = 1.f;									//stablishes the intensity of movement. It is added to directionOfMovement, resulting in float4 that describes the velocity of the particle
	float initialIntensity2 = 3.0f;

	float3 initialDirection1 = float3(1.0f, 1.0f, 1.0f);			//determines the direction of the movement computing 3 random values (between initialDirection1 and 2)
	float3 initialDirection2 = float3(-1.0f, -1.0f, -1.0f);

	float3 initialPosition1 = float3::zero;
	float3 initialPosition2 = float3::zero;

	bool hideMovement = false;
	bool eraseMovement = false;
};

struct ParticleColor : M_ParticleSystem
{
	ParticleColor() : M_ParticleSystem(Type::ParticleColor) {};

	void Spawn(EmitterInstance* emitter, Particle* particle);
	void Update(float dt, EmitterInstance* emitter);

	Color initialColor = Color(1.0f, 1.0f, 1.0f, 1.0f); //black by default

	bool hideColor = false;
	bool eraseColor = false;
};

struct ParticleLifetime : M_ParticleSystem
{
	ParticleLifetime() : M_ParticleSystem(Type::ParticleLifetime) {};

	void Spawn(EmitterInstance* emitter, Particle* particle);
	void Update(float dt, EmitterInstance* emitter);

	float initialLifetime = 0.2f;

	bool hideLifetime = false;
	bool eraseLifetime = false;
};

struct ParticleBillboarding : M_ParticleSystem
{
	ParticleBillboarding() : M_ParticleSystem(Type::ParticleBillboarding) {};

	void Spawn(EmitterInstance* emitter, Particle* particle);
	void Update(float dt, EmitterInstance* emitter);

	Quat GetAlignmentRotation(const float3& position, const float4x4& cameraTransform);

	bool hideBillboarding = false;
	bool eraseBillboarding = false;
};

#endif