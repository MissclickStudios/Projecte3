#ifndef __SIMULATIONCALLBACK_H__
#define __SIMULATIONCALLBACK_H__

#include "PhysX_4.1/Include/PxPhysicsAPI.h"

class GameObject;

class SimulationCallback : public physx::PxSimulationEventCallback
{
public:
	SimulationCallback();
	~SimulationCallback() {}

	void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs);
	void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count);

	// Needed to not be an abstract class, but will probably never be used
	void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) {}
	void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) {}

	void onWake(physx::PxActor** actors, physx::PxU32 count) {}
	void onSleep(physx::PxActor** actors, physx::PxU32 count) {}
	// ---
};

#endif // !__SIMULATIONCALLBACK_H__