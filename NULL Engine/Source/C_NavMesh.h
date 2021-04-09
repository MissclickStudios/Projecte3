#ifndef __C_NAVMESH_H__
#define __C_NAVMESH_H__

#include "Component.h"

class ParsonNode;
class GameObject;

class R_NavMesh;

class C_NavMesh : public Component
{
public:
	C_NavMesh(GameObject* owner);
	~C_NavMesh();

	bool Start		() override;
	bool Update		() override;
	bool CleanUp	() override;

	bool SaveState	(ParsonNode& root) const override;
	bool LoadState	(ParsonNode& root) override;

public:


private:
	R_NavMesh* rNavMesh;
};

#endif	// !__C_NAVMESH_H__