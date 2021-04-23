#ifndef __R_NAVMESH_H__
#define __R_NAVMESH_H__

#include "Resource.h"

class dtNavMesh;

class R_NavMesh : public Resource
{
public:
	R_NavMesh();
	~R_NavMesh();

	bool CleanUp() override;

	bool SaveMeta(ParsonNode& metaRoot) const override;
	bool LoadMeta(const ParsonNode& metaRoot) override;

public:

	dtNavMesh* navMesh;


};

#endif // !__R_NAVMESH_H__