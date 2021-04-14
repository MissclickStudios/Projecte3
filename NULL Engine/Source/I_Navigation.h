#ifndef __I_NAVIGATION_H__
#define __I_NAVIGATION_H__

class R_NavMesh;

typedef unsigned int uint;

namespace Importer
{
	namespace Navigation
	{
		bool Import	(const char* buffer, R_NavMesh* rNavMesh);
		uint Save	(const R_NavMesh* rNavMesh, char** buffer);
		bool Load	(const char* buffer, R_NavMesh* rNavMesh);

		namespace Utilities
		{
			// PRIVATE FUNCTIONS OF I_NAVIGATION
		}
	}
}

#endif // !__I_NAVIGATION_H__