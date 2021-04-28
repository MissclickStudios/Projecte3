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

class MISSCLICK_API M_Physics : public Module
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

	const float Gravity() const { return gravity; }
	void SetGravity(float value);

	const std::vector<std::string>* const GetFilters() { return &filters; }
	bool** GetInteractions() { return filterInteractions; }

	const std::string* const GetFilter(int id) const;
	const int GetFilterID(const std::string* const filter);

	void CreateFilter(const std::string& filter);
	void DeleteFilter(const std::string& filter);

	bool simulating = false;

private:

	physx::PxCooking* cooking = nullptr;
	physx::PxFoundation* foundation = nullptr;
	physx::PxControllerManager* controllerManager = nullptr;
	physx::PxScene* scene = nullptr;

	SimulationCallback* simulationCallback = nullptr;

	float gravity = 9.8f;

	std::string defaultFilter = "default";
	std::vector<std::string> filters;
	bool** filterInteractions = nullptr;
};

#endif //__PHYSICS_H__