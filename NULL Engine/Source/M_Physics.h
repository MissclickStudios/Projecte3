#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include "Module.h"

namespace physx
{
	class PxPvd;
	class PxPvdSceneClient;
	class PxFoundation;
	class PxPhysics;
	class PxScene;
	class PxMaterial;
	class PxRigidStatic;
	class PxControllerManager;
	class PxRigidActor;
	class PxVolumeCache;
	class PxSimulationEventCallback;
	class PxActor;
	struct PxActorShape;
	class PxQueryFilterCallback;
	class RaycastCCDManager;
	class PxCooking;
	class PxConvexMesh;
	class PxBase;

	typedef uint32_t PxU32;
};

class M_Physics : public Module
{
public:

	M_Physics(bool isActive = true);
	~M_Physics();

	bool Init(ParsonNode& root) override;
	bool Start() override;
	UpdateStatus Update(float dt) override;
	bool CleanUp() override;

	bool LoadConfiguration(ParsonNode& configuration) override;
	bool SaveConfiguration(ParsonNode& configuration) const override;

	void AddActor(physx::PxActor* actor);
	void DeleteActor(physx::PxActor* actor);

	physx::PxPhysics* physics = nullptr;
	bool simulating = false;

private:

	physx::PxPvd* pvd = nullptr;
	physx::PxCooking* cooking = nullptr;
	physx::PxPvdSceneClient* pvdClient = nullptr;
	physx::PxFoundation* foundation = nullptr;
	physx::PxControllerManager* controllerManager = nullptr;
	physx::PxScene* scene = nullptr;
	physx::PxMaterial* material = nullptr;
	physx::PxRigidStatic* plane = nullptr;
	physx::RaycastCCDManager* raycastManager = nullptr;

	float gravity = 9.8f;
};

#endif //__PHYSICS_H__