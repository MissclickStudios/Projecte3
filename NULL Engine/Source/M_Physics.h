#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include "Module.h"

namespace physx
{
	class PxFoundation;
	class PxPhysics;
	class PxScene;
	class PxMaterial;
	class PxRigidDynamic;
	class PxSimulationEventCallback;
	class PxActor;
	class PxControllerManager;
	class PxCooking;
};

class GameObject;
class SimulationCallback;

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
	physx::PxMaterial* material = nullptr;
	std::map<physx::PxRigidDynamic*, GameObject*> actors;

	bool simulating = false;

private:

	physx::PxCooking* cooking = nullptr;
	physx::PxFoundation* foundation = nullptr;
	physx::PxControllerManager* controllerManager = nullptr;
	physx::PxScene* scene = nullptr;

	SimulationCallback* simulationCallback = nullptr;

	float gravity = 9.8f;
};

#endif //__PHYSICS_H__