#ifndef __R_NAVMESH_H__
#define __R_NAVMESH_H__

#include "Macros.h"
#include "Resource.h"

#include <string>

class dtNavMesh;

class MISSCLICK_API R_NavMesh : public Resource
{
	friend class M_Detour;
public:
	R_NavMesh();
	~R_NavMesh();

	bool CleanUp() override;

	bool SaveMeta(ParsonNode& metaRoot) const override;
	bool LoadMeta(const ParsonNode& metaRoot) override;

	static inline ResourceType GetType() { return ResourceType::NAVMESH; }

public:
	void SetNavMeshName(const char* newName);
	const char* GetNavMeshName();

public:

	std::string navMeshName = "";
 	dtNavMesh* navMesh = nullptr;

};

#endif // !__R_NAVMESH_H__