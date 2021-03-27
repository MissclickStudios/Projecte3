#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include <vector>
#include <map>
#include "Module.h"

namespace physx
{
	class PxFoundation;
	class PxPhysics;
	class PxScene;
	class PxMaterial;
	class PxRigidActor;
	class PxSimulationEventCallback;
	class PxActor;
	class PxControllerManager;
	class PxCooking;
};

class GameObject;
class SimulationCallback;

class NULL_API M_Physics : public Module
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

	void AddActor(physx::PxActor* actor, GameObject* owner);
	void DeleteActor(physx::PxActor* actor);

	physx::PxPhysics* physics = nullptr;
	physx::PxMaterial* material = nullptr;
	std::map<physx::PxRigidActor*, GameObject*> actors;

	bool simulating = false;

private:

	physx::PxCooking* cooking = nullptr;
	physx::PxFoundation* foundation = nullptr;
	physx::PxControllerManager* controllerManager = nullptr;
	physx::PxScene* scene = nullptr;

	SimulationCallback* simulationCallback = nullptr;

	float gravity = 9.8f;

	std::vector<std::string> filters;
	bool** filterInteractions = nullptr;
};

#endif //__PHYSICS_H__