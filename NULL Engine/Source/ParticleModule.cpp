#include "JSONParser.h"
#include "C_ParticleSystem.h"
#include "ParticleModule.h"

#include "GameObject.h"
#include "Emitter.h"
#include "EmitterInstance.h"

#include "C_Transform.h"
#include "M_Camera3D.h"
#include "Application.h"

#include "MemoryManager.h"

void EmitterBase::Save(ParsonNode& node)
{
	node.SetInteger("Type",(int)type);
	node.SetFloat3("origin", origin);
}

void EmitterBase::Load(ParsonNode& node)
{
	origin = node.GetFloat3("origin");
}

void EmitterBase::Spawn(EmitterInstance* emitter, Particle* particle)
{
	GameObject* go = emitter->component->GetOwner();

	float3 position = go->GetComponent<C_Transform>()->GetWorldPosition();
	position += origin;
	particle->position = position;

	Quat rotation = go->GetComponent<C_Transform>()->GetWorldRotation();
	particle->worldRotation = rotation;
}

void EmitterBase::Update(float dt, EmitterInstance* emitter)
{
	//update particles
	for (unsigned int i = 0; i < emitter->activeParticles; ++i)
	{
		unsigned int particleIndex = emitter->particleIndices[i];
		Particle* particle = &emitter->particles[particleIndex];

		particle->normalizedLifetime = particle->currentLifetime / particle->maxLifetime;
		particle->distanceToCamera = float3(App->camera->GetCurrentCamera()->GetFrustum().WorldMatrix().TranslatePart() - particle->position).LengthSq();
	}
	
}

void EmitterSpawn::Save(ParsonNode& node)
{
	node.SetInteger("Type", (int)type);

	node.SetNumber("spawnRatio", spawnRatio);
	node.SetNumber("timer", timer);
}

void EmitterSpawn::Load(ParsonNode& node)
{
	spawnRatio = node.GetNumber("spawnRatio");
	timer = node.GetNumber("timer");
}

void EmitterSpawn::Spawn(EmitterInstance* emitter, Particle* particle)
{

}

void EmitterSpawn::Update(float dt, EmitterInstance* emitter)
{
	if (hideSpawn == false)
	{
		timer += dt;
		if (timer >= spawnRatio)
		{
			timer = 0;
			emitter->
				SpawnParticle(); //SpawnParticle() will then call the Spawn() method in every particle module
		}
	}
}

void EmitterArea::Load(ParsonNode& node)
{

}

void EmitterArea::Save(ParsonNode& node)
{

}

void EmitterArea::Spawn(EmitterInstance* emitter, Particle* particle)
{
	float positionX = math::Lerp(areaX1, areaX2, randomGenerator.Float());
	float positionY = math::Lerp(areaY1, areaY2, randomGenerator.Float());
	float positionZ = math::Lerp(areaZ1, areaZ2, randomGenerator.Float());

	particle->position = float3(particle->position.x + positionX, particle->position.y + positionY, particle->position.z + positionZ);
}

void EmitterArea::Update(float dt, EmitterInstance* emitter)
{

}

void ParticleMovement::Save(ParsonNode& node)
{
	//TODO PARTICLE SYSTEM
	node.SetInteger("Type", (int)type);

	node.SetNumber("initialIntensity1",initialIntensity1);
	node.SetNumber("initialIntensity2", initialIntensity2);

	node.SetFloat3("initialDirection1",initialDirection1);
	node.SetFloat3("initialDirection2", initialDirection2);

	node.SetFloat3("initialPosition1",initialPosition1);
	node.SetFloat3("initialPosition2", initialPosition2);

	node.SetBool("hideMovement", hideMovement);
	node.SetBool("eraseMovement", eraseMovement);
}

void ParticleMovement::Load(ParsonNode& node)
{
	node.SetInteger("Type", (int)type);

	initialIntensity1 = node.GetNumber("initialIntensity1");
	initialIntensity2 = node.GetNumber("initialIntensity2");

	initialDirection1 = node.GetFloat3("initialDirection1");
	initialDirection2 = node.GetFloat3("initialDirection2");

	initialPosition1 = node.GetFloat3("initialPosition1");
	initialPosition2 = node.GetFloat3("initialPosition2");

	hideMovement = node.GetBool("hideMovement");
	eraseMovement = node.GetBool("eraseMovement");
}

void ParticleMovement::Spawn(EmitterInstance* emitter, Particle* particle)
{
	float directionX = math::Lerp(initialDirection1.x, initialDirection2.x, randomGenerator.Float());
	float directionY = math::Lerp(initialDirection1.y, initialDirection2.y, randomGenerator.Float());
	float directionZ = math::Lerp(initialDirection1.z, initialDirection2.z, randomGenerator.Float());
	particle->movementDirection = float3(directionX, directionY, directionZ);

	particle->velocity = math::Lerp(initialIntensity1, initialIntensity2, randomGenerator.Float());
}

void ParticleMovement::Update(float dt, EmitterInstance* emitter)
{
	if (hideMovement == false)
	{
		for (unsigned int i = 0; i < emitter->activeParticles; i++)
		{
			unsigned int particleIndex = emitter->particleIndices[i];
			Particle* particle = &emitter->particles[particleIndex];

			particle->position += particle->movementDirection * particle->velocity * dt;
		
		}
	}
	if (eraseMovement == true)
	{
		emitter->emitter->DeleteModuleFromType(ParticleModule::Type::PARTICLE_MOVEMENT);
	}
}

void ParticleColor::Save(ParsonNode& node)
{
	node.SetInteger("Type", (int)type);//TODO PARTICLE SYSTEM

	node.SetColor("initialColor1",initialColor1);
	node.SetColor("initialColor2", initialColor2);

	node.SetBool("hideColor",hideColor);
	node.SetBool("eraseColor", eraseColor);
}

void ParticleColor::Load(ParsonNode& node)
{
	initialColor1 = node.GetColor("initialColor1");
	initialColor2 = node.GetColor("initialColor2");

	hideColor = node.GetBool("hideColor");
	eraseColor = node.GetBool("eraseColor");
}

void ParticleColor::Spawn(EmitterInstance* emitter, Particle* particle)
{
	if (!colorOverLifetime)
	{
		particle->color.r = math::Lerp(initialColor1.r, initialColor2.r, randomGenerator.Float());
		particle->color.g = math::Lerp(initialColor1.g, initialColor2.g, randomGenerator.Float());
		particle->color.b = math::Lerp(initialColor1.b, initialColor2.b, randomGenerator.Float());
	}
}

void ParticleColor::Update(float dt, EmitterInstance* emitter)
{
	if (colorOverLifetime)
	{
		for (unsigned int i = 0; i < emitter->activeParticles; ++i)
		{
			unsigned int particleIndex = emitter->particleIndices[i];
			Particle* particle = &emitter->particles[particleIndex];

			float redDifference = initialColor2.r - initialColor1.r;
			float newRed = initialColor1.r + (redDifference * particle->normalizedLifetime);

			float greenDifference = initialColor2.g - initialColor1.g;
			float newGreen = initialColor1.g + (greenDifference * particle->normalizedLifetime);

			float blueDifference = initialColor2.b - initialColor1.b;
			float newBlue = initialColor1.b + (blueDifference * particle->normalizedLifetime);

			float alphaDifference = initialColor2.a - initialColor1.a;
			float newAlpha = initialColor1.a + (alphaDifference * particle->normalizedLifetime);

			particle->color = Color(newRed, newGreen, newBlue, newAlpha);
		}
	}

	if (eraseColor == true)
	{
		emitter->emitter->DeleteModuleFromType(ParticleModule::Type::PARTICLE_COLOR);
	}
}

void ParticleLifetime::Save(ParsonNode& node)
{
	node.SetInteger("Type", (int)type);//TODO PARTICLE SYSTEM

	node.SetNumber("initialLifetime", initialLifetime);

	node.SetBool("hideLifetime", hideLifetime);
	node.SetBool("eraseLifetime", eraseLifetime);
}

void ParticleLifetime::Load(ParsonNode& node)
{
	initialLifetime = node.GetNumber("initialLifetime");

	hideLifetime = node.GetBool("hideLifetime");
	eraseLifetime = node.GetBool("eraseLifetime");
}

void ParticleLifetime::Spawn(EmitterInstance* emitter, Particle* particle)
{
	particle->maxLifetime = initialLifetime;
	particle->currentLifetime = 0.0f;
}

void ParticleLifetime::Update(float dt, EmitterInstance* emitter)
{
	if (hideLifetime == false) 
	{
		for (unsigned int i = 0; i < emitter->activeParticles; i++)
		{
			unsigned int particleIndex = emitter->particleIndices[i];
			Particle* particle = &emitter->particles[particleIndex];

			particle->currentLifetime += dt;
		}
	}
	if (eraseLifetime == true)
	{
		emitter->emitter->DeleteModuleFromType(ParticleModule::Type::PARTICLE_LIFETIME);
	}
}

void ParticleBillboarding::Save(ParsonNode& node)
{
	node.SetInteger("Type", (int)type);

	node.SetInteger("billboardingType",(int)billboardingType);

	node.SetBool("hideBillboarding", hideBillboarding);
	node.SetBool("eraseBillboarding", eraseBillboarding);
}

void ParticleBillboarding::Load(ParsonNode& node)
{
	billboardingType = (BillboardingType)node.GetInteger("billboardingType");

	hideBillboarding = node.GetBool("hideBillboarding");
	eraseBillboarding = node.GetBool("eraseBillboarding");
}

void ParticleBillboarding::Spawn(EmitterInstance* emitter, Particle* particle)
{
	particle->worldRotation = GetAlignmentRotation(particle->position, App->camera->GetCurrentCamera()->GetFrustum().WorldMatrix());
}

void ParticleBillboarding::Update(float dt, EmitterInstance* emitter)
{
	if (hideBillboarding == false)
	{
		for (unsigned int i = 0; i < emitter->activeParticles; ++i)
		{
			unsigned int particleIndex = emitter->particleIndices[i];
			Particle* particle = &emitter->particles[particleIndex];

			particle->worldRotation = GetAlignmentRotation(particle->position, App->camera->GetCurrentCamera()->GetFrustum().WorldMatrix());
		}
	}
	 
	if (eraseBillboarding == true)
	{
		emitter->emitter->DeleteModuleFromType(ParticleModule::Type::PARTICLE_BILLBOARDING);
	}
}

Quat ParticleBillboarding::GetAlignmentRotation(const float3& position, const float4x4& cameraTransform)
{
	float3 N, U, _U, R;
	float3 direction = float3(cameraTransform.TranslatePart() - position).Normalized(); //normalized vector between the camera and gameobject position

	switch (billboardingType)
	{
		case(BillboardingType::ScreenAligned):
		{
			N = cameraTransform.WorldZ().Normalized().Neg();	// N is the inverse of the camera +Z
			U = cameraTransform.WorldY().Normalized();			// U is the up vector from the camera (already perpendicular to N)
			R = U.Cross(N).Normalized();						// R is the cross product between  U and N
		}
		break;
		case(BillboardingType::WorldAligned):
		{
			N = direction;										// N is the direction
			_U = cameraTransform.WorldY().Normalized();			// _U is the up vector form the camera, only used to calculate R
			R = _U.Cross(N).Normalized();						// R is the cross product between U and N
			U = N.Cross(R).Normalized();						// U is the cross product between N and R
		}
		break;
		case(BillboardingType::XAxisAligned):
		{
			R = float3::unitX;									// R = (1,0,0)
			U = direction.Cross(R).Normalized();				// U cross between R and direction
			N = R.Cross(U).Normalized();						// N faces the camera
		}			
		break;
		case(BillboardingType::YAxisAligned):
		{
			U = float3::unitY;
			R = U.Cross(direction).Normalized();
			N = R.Cross(U).Normalized();
		}
		break;
		case(BillboardingType::ZAxisAligned):
		{
			N = float3::unitZ;
			R = direction.Cross(N).Normalized();
			U = N.Cross(R).Normalized();
		}
		break;
	}
	float3x3 result = float3x3(R, U, N);

	/*return result.Inverted().ToQuat();*/
	return result.ToQuat();
}

void ParticleSize::Save(ParsonNode& node)
{

}

void ParticleSize::Load(ParsonNode& node)
{

}

void ParticleSize::Spawn(EmitterInstance* emitter, Particle* particle)
{
	if (!SizeOverTime)
		particle->size = math::Lerp(initialSize1, initialSize2, randomGenerator.Float());
	else
		particle->size = initialSize1;
}

void ParticleSize::Update(float dt, EmitterInstance* emitter)
{
	if (SizeOverTime)
	{
		for (unsigned int i = 0; i < emitter->activeParticles; ++i)
		{
			unsigned int particleIndex = emitter->particleIndices[i];
			Particle* particle = &emitter->particles[particleIndex];

			float sizeDifference = initialSize2 - initialSize1;
			particle->size = initialSize1 + (sizeDifference * particle->normalizedLifetime);
		}
	}
}