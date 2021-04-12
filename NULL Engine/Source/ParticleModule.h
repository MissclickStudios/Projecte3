#ifndef __M_PARTICLE_SYSTEM_H__
#define __M_PARTICLE_SYSTEM_H__

#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/float4.h"
#include "MathGeoLib/include/Algorithm/Random/LCG.h"
#include "Macros.h"
#include "Color.h"

class Particle;
class EmitterInstance;

struct NULL_API ParticleModule
{
	enum class Type
	{
		NONE,
		PARTICLE_MOVEMENT,
		PARTICLE_COLOR,
		PARTICLE_LIFETIME,
		PARTICLE_ROTATION,
		PARTICLE_SIZE,
		PARTICLE_BILLBOARDING,
		EMITTER_BASE,			//Origin of the emitter
		EMITTER_AREA,
		EMITTER_SPAWN,			//Spawn Rate and timer
		
		UNKNOWN
	} type;

	ParticleModule(Type type) : type(type) {};

	virtual void Save(ParsonNode& node) = 0;
	virtual void Load(ParsonNode& node) = 0;

	virtual void Spawn(EmitterInstance* emitter, Particle* particle) = 0;
	virtual void Update(float dt, EmitterInstance* emitter) = 0;

	LCG randomGenerator;
	//virtual void Save / Load
};

struct EmitterBase : ParticleModule
{
	//camera alignment

	EmitterBase() : ParticleModule(Type::EMITTER_BASE) {};

	void Save(ParsonNode& node) override;
	void Load(ParsonNode& node) override;

	void Spawn(EmitterInstance* emitter, Particle* particle);	//Loop active particles and update them
	void Update(float dt, EmitterInstance* emitter);

	float3 origin = float3::zero;
};

struct EmitterSpawn : ParticleModule
{
	EmitterSpawn() : ParticleModule(Type::EMITTER_SPAWN) {};
	
	void Save(ParsonNode& node) override;
	void Load(ParsonNode& node) override;

	void Spawn(EmitterInstance* emitter, Particle* particle);
	void Update(float dt, EmitterInstance* emitter);			//spawn ratio and timer management

	float spawnRatio = 0.05f;
	float timer = 0.0f;
};

struct ParticleMovement : ParticleModule
{
	ParticleMovement() : ParticleModule(Type::PARTICLE_MOVEMENT) {};

	void Save(ParsonNode& node) override;
	void Load(ParsonNode& node) override;

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

struct ParticleColor : ParticleModule
{
	ParticleColor() : ParticleModule(Type::PARTICLE_COLOR) {};

	void Save(ParsonNode& node) override;
	void Load(ParsonNode& node) override;

	void Spawn(EmitterInstance* emitter, Particle* particle);
	void Update(float dt, EmitterInstance* emitter);

	Color initialColor = Color(1.0f, 1.0f, 1.0f, 1.0f); //black by default

	bool hideColor = false;
	bool eraseColor = false;
};

struct ParticleLifetime : ParticleModule
{
	ParticleLifetime() : ParticleModule(Type::PARTICLE_LIFETIME) {};

	void Save(ParsonNode& node) override;
	void Load(ParsonNode& node) override;

	void Spawn(EmitterInstance* emitter, Particle* particle);
	void Update(float dt, EmitterInstance* emitter);

	float initialLifetime = 0.2f;

	bool hideLifetime = false;
	bool eraseLifetime = false;
};

struct ParticleBillboarding : ParticleModule
{
	enum class BillboardingType
	{
		ScreenAligned,
		WorldAligned,
		XAxisAligned,
		YAxisAligned,
		ZAxisAligned,

		None,
	};

	ParticleBillboarding() : ParticleModule(Type::PARTICLE_BILLBOARDING) {};

	void Save(ParsonNode& node) override;
	void Load(ParsonNode& node) override;

	void Spawn(EmitterInstance* emitter, Particle* particle);
	void Update(float dt, EmitterInstance* emitter);

	Quat GetAlignmentRotation(const float3& position, const float4x4& cameraTransform);

	BillboardingType billboardingType = BillboardingType::ScreenAligned;
	bool hideBillboarding = false;
	bool eraseBillboarding = false;
};

#endif