#ifndef __PARTICLE_MODULE_H__
#define __PARTICLE_MODULE_H__

#include "MathGeoLib/include/Math/float3.h"
#include "Macros.h"

class Particle;
class EmitterInstance;

struct NULL_API ParticleModule
{
	enum Type
	{
		None,
		EmitterBase,		//Origin of the emitter
		EmitterSpawn,		//Spawn Rate and timer
		EmitterArea,
		ParticlePosition,
		ParticleRotation,
		ParticleSize,
		ParticleColor,
		ParticleLifetime,
		ParticleVelocity,
		Unknown
	} type;

	ParticleModule(Type type) : type(type) {};

	virtual void Spawn(EmitterInstance* emitter, Particle* particle) = 0;
	virtual void Update(float dt, EmitterInstance* emitter) = 0;

	//virtual void Save / Load

};

struct EmitterBase : ParticleModule
{
	//camera alignment

	EmitterBase() : ParticleModule(Type::EmitterBase) {};

	void Spawn(EmitterInstance* emitter, Particle* particle);	//Loop active particles and update them
	void Update(float dt, EmitterInstance* emitter);

	float3 origin = float3::zero;
};

struct EmitterSpawn : ParticleModule
{
	EmitterSpawn() : ParticleModule(Type::EmitterSpawn) {};

	void Spawn(EmitterInstance* emitter, Particle* particle);
	void Update(float dt, EmitterInstance* emitter);			//spawn ratio and timer management

	float spawnRatio = 0.1f;
	float timer = 0.0f;
};

struct ParticlePosition : ParticleModule
{
	ParticlePosition() : ParticleModule(Type::ParticlePosition) {};

	void Spawn(EmitterInstance* emitter, Particle* particle);	//random pos between initialPosition1 and 2
	void Update(float dt, EmitterInstance* emitter);

	float3 initialPosition1 = float3::zero;
	float3 initialPosition2 = float3::zero;
};

struct ParticleColor : ParticleModule
{
	ParticleColor() : ParticleModule(Type::ParticleColor) {};

	void Spawn(EmitterInstance* emitter, Particle* particle);
	void Update(float dt, EmitterInstance* emitter);

	float4 initialColor1 = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 initialColor2 = float4(1.0f, 1.0f, 1.0f, 1.0f);
};

#endif