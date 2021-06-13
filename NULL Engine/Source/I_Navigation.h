#ifndef __I_NAVIGATION_H__
#define __I_NAVIGATION_H__

class R_NavMesh;

typedef unsigned int uint;

struct null_dtNavMeshParams {
	float orig[3];					///< The world space origin of the navigation mesh's tile space. [(x, y, z)]
	float tileWidth;				///< The width of each tile. (Along the x-axis.)
	float tileHeight;				///< The height of each tile. (Along the z-axis.)
	int maxTiles;					///< The maximum number of tiles the navigation mesh can contain.
	int maxPolys;					///< The maximum number of polygons each tile can contain.
};

struct NavMeshSetHeader {
	int magic;
	int version;
	int numTiles;
	null_dtNavMeshParams params;
};

struct NavMeshTileHeader {
	unsigned int tileRef;
	int dataSize;
};

static const int NAVMESHSET_MAGIC = 'M' << 24 | 'S' << 16 | 'E' << 8 | 'T'; //'MSET';
static const int NAVMESHSET_VERSION = 1;

namespace Importer
{
	namespace Navigation
	{
		bool Import(const char* buffer, R_NavMesh* rNavMesh);
		uint Save(R_NavMesh* rNavMesh, char** buffer);
		bool Load(const char* buffer, R_NavMesh* rNavMesh);

		namespace Utilities
		{
			// PRIVATE FUNCTIONS OF I_NAVIGATION
		}
	}
}

#endif // !__I_NAVIGATION_H__