#ifndef __I_PARTICLES_H__
#define __I_PARTICLES_H__

class R_ParticleSystem;

typedef unsigned int uint;

namespace Importer
{
	namespace Particles
	{
		bool Import(const char* buffer, R_ParticleSystem* rParticles);
		uint Save(R_ParticleSystem* rParticles, char** buffer);
		bool Load(const char* buffer, R_ParticleSystem* rParticles);

		namespace Utilities
		{

		}
	}
}

#endif // !__I_PARTICLES_H__