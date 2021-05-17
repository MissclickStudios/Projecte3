#ifndef __C_NAVMESH_H__
#define __C_NAVMESH_H__

#include "Component.h"
#include "MathGeoLib/include/Math/float3.h"
#include <vector>

class ParsonNode;
class GameObject;

class R_NavMesh;

class MISSCLICK_API C_NavMeshAgent : public Component
{
public:
	C_NavMeshAgent(GameObject* owner);
	~C_NavMeshAgent();

	bool Start() override;
	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::NAVMESH_AGENT; }								// This is needed to use templeates for functions such as GetComponent<>();

public:
	bool SetDestination(float3 destination);
	bool HasDestination();
	void CancelDestination();

	bool hasDestination;
	float3 destinationPoint;

private:
	bool CalculatePath(float3 destination);

	std::vector<float3> path;
	float3 nextPoint;

	float radius;

	int areaMask = 1;
};

#endif	// !__C_NAVMESH_H__