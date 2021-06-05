#ifndef __C_NAVMESH_H__
#define __C_NAVMESH_H__

#include "Component.h"
#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/float2.h"
#include <vector>

class ParsonNode;
class GameObject;
class C_RigidBody;

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
	void CancelDestination(bool dontStopInertiaInTheUpdateFunction = false);
	void StopAndCancelDestination();

	const float3 GetNextPathPoint() const;

	float3 origin;
	bool hasDestination;
	float3 destinationPoint;

	std::vector<float3> path;

	bool AgentPath(float3 origin, float3 destination);

	float3 currentPos;

	float velocity = 30.0f;

	float3 direction = float3::zero;

private:

	C_RigidBody* rigidBody = nullptr;

	float3 nextPoint;

	float radius;

	int areaMask = 1;

	int indexPath = 1;

	bool dontStopInertiaInTheUpdateFunction = false; // mad? find a better name and i'll change it
};

#endif	// !__C_NAVMESH_H__