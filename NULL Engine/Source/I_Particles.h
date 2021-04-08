#ifndef __I_PARTICLES_H__
#define __I_PARTICLES_H__

class R_Particles;

typedef unsigned int uint;

namespace Importer
{
	namespace Particles
	{
		bool Import	(const char* buffer, R_Particles* rParticles);
		uint Save	(const R_Particles* rParticles, char** buffer);
		bool Load	(const char* buffer, R_Particles* rParticles);

		namespace Utilities
		{

		}
	}
}

#endif // !__I_PARTICLES_H__