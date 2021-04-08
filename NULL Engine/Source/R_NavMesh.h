#ifndef __R_NAVMESH_H__
#define __R_NAVMESH_H__

#include "Resource.h"

class R_NavMesh : public Resource
{
public:
	R_NavMesh();
	~R_NavMesh();

	bool CleanUp() override;

	bool SaveMeta(ParsonNode& root) const override;
	bool LoadMeta(const ParsonNode& root) override;

public:


private:

};

#endif // !__R_NAVMESH_H__