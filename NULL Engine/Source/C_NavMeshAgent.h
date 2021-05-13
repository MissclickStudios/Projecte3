#ifndef __C_NAVMESH_H__
#define __C_NAVMESH_H__

#include "Component.h"
#include "MathGeoLib/include/Math/float3.h"

class ParsonNode;
class GameObject;

class R_NavMesh;

class C_NavMeshAgent : public Component
{
public:
	C_NavMeshAgent(GameObject* owner);
	~C_NavMeshAgent();

	bool Start		() override;
	bool Update		() override;
	bool CleanUp	() override;

	bool SaveState	(ParsonNode& root) const override;
	bool LoadState	(ParsonNode& root) override;

public:
	bool CalculatePath(float3 originPos, float3 targetPos);

public:

	int areaMask;
	float3 destination;
	float3 path;
	float radius;

};

#endif	// !__C_NAVMESH_H__