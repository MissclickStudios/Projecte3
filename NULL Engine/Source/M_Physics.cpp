#include "Application.h"
#include "Log.h"
#include "M_Physics.h"

#include "GameObject.h"

#include "SimulationCallback.h" // PhysxAPI already included in here

#include "MemoryManager.h"

#define BOUNCE_THRESHOLD 0.2f
#define THREADS 4

#ifndef _DEBUG
#pragma comment(lib, "Source/Dependencies/PhysX_3.4/libx86_/Release_/PhysX3_x86.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_3.4/libx86_/Release_/PhysX3Common_x86.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_3.4/libx86_/Release_/PhysX3Extensions.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_3.4/libx86_/Release_/PxFoundation_x86.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_3.4/libx86_/Release_/PxPvdSDK_x86.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_3.4/libx86_/Release_/PhysX3CharacterKinematic_x86.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_3.4/libx86_/Release_/SceneQuery.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_3.4/libx86_/Release_/PhysX3Cooking_x86.lib")
#else
#pragma comment(lib, "Source/Dependencies/PhysX_3.4/libx86_/Debug_/PhysX3CommonDEBUG_x86.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_3.4/libx86_/Debug_/PhysX3DEBUG_x86.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_3.4/libx86_/Debug_/PhysX3ExtensionsDEBUG.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_3.4/libx86_/Debug_/PxFoundationDEBUG_x86.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_3.4/libx86_/Debug_/PxPvdSDKDEBUG_x86.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_3.4/libx86_/Debug_/PhysX3CharacterKinematicDEBUG_x86.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_3.4/libx86_/Debug_/SceneQueryDEBUG.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_3.4/libx86_/Debug_/PhysX3CookingDEBUG_x86.lib")
#endif // _DEBUG

M_Physics::M_Physics(bool isActive) : Module("physics", isActive)
{
}

M_Physics::~M_Physics()
{
}

bool M_Physics::Init(ParsonNode& root)
{
	return true;
}

physx::PxFilterFlags customFilterShader(
	physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
	physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
	physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
{
	pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;
	pairFlags |= physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;
	pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
	pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS;
	pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_LOST;

	return physx::PxFilterFlag::eDEFAULT;
}

bool M_Physics::Start()
{
	LOG("Initializing PhysX 3.4 --------------");
	static physx::PxDefaultErrorCallback gDefaultErrorCallback;
	static physx::PxDefaultAllocator gDefaultAllocatorCallback;

	foundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
	if (!foundation)
	{
		LOG("[ERROR] Physics Module: PxCreateFoundation failed!");
		return false;
	}
	LOG("Physics Foundation created succesfully");

	bool recordMemoryAllocations = true;

	physics = PxCreateBasePhysics(PX_PHYSICS_VERSION, *foundation, physx::PxTolerancesScale(), recordMemoryAllocations);
	if (!physics)
	{
		LOG("[ERROR] Physics Module: PxCreatePhysics failed!");
		return false;
	}
	LOG("Physics created succesfully");

	cooking = PxCreateCooking(PX_PHYSICS_VERSION, *foundation, physx::PxCookingParams(physx::PxTolerancesScale()));
	if (!cooking)
	{
		LOG("[ERROR] Physics Module: PxCreateCooking failed!");
		return false;
	}
	LOG("Physics Cooking created succesfully");

	simulationCallback = new SimulationCallback();
	physx::PxSceneDesc sceneDesc(physics->getTolerancesScale());

	
	sceneDesc.gravity = physx::PxVec3(0.0f, -gravity, 0.0f);
	sceneDesc.bounceThresholdVelocity = gravity * BOUNCE_THRESHOLD;
	sceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(THREADS);
	sceneDesc.flags |= physx::PxSceneFlag::eENABLE_KINEMATIC_PAIRS | physx::PxSceneFlag::eENABLE_KINEMATIC_STATIC_PAIRS | physx::PxSceneFlag::eENABLE_PCM;
	sceneDesc.filterShader = customFilterShader;
	sceneDesc.simulationEventCallback = simulationCallback;

	scene = physics->createScene(sceneDesc);

	if (!scene)
	{
		LOG("[ERROR] Physics Module: createScene failed!");
		return false;
	}
	LOG("Physics Scene created succesfully");

	controllerManager = PxCreateControllerManager(*scene);
	if (!controllerManager)
	{
		LOG("[ERROR] Physics Module: PxCreateControllerManager failed!");
		return false;
	}

	material = physics->createMaterial(2, 1, 0.25);

	LOG("Physics Controller Manager created succesfully");
	LOG("PhysX 3.4 Initialized correctly --------------");

	return true;
}

UpdateStatus M_Physics::Update(float dt)
{
	if (App->play && !App->pause)
		simulating = true;
	else
		simulating = false;

	if (scene && simulating)
	{
		scene->simulate(dt);
		scene->fetchResults(true);
	}

	return UpdateStatus::CONTINUE;
}

bool M_Physics::CleanUp()
{
	if (controllerManager)
		controllerManager->release();
	if (material)
		material->release();
	if (cooking)
		cooking->release();
	if (scene)
		scene->release();
	if (physics)
		physics->release();
	if (foundation)
		foundation->release();

	controllerManager = nullptr;
	physics = nullptr;
	foundation = nullptr;
	scene = nullptr;

	return true;
}

bool M_Physics::LoadConfiguration(ParsonNode& configuration)
{
	return true;
}

bool M_Physics::SaveConfiguration(ParsonNode& configuration) const
{
	return true;
}

void M_Physics::AddActor(physx::PxActor* actor, GameObject* owner)
{
	if (!actor)
		return;

	scene->addActor(*actor);
	actors.insert(std::make_pair<physx::PxRigidDynamic*, GameObject*>((physx::PxRigidDynamic*)actor, (GameObject*)(void*)owner));
}

void M_Physics::DeleteActor(physx::PxActor* actor)
{
	if (!actor)
		return;

	scene->removeActor(*actor);
	actors.erase((physx::PxRigidDynamic*)actor);
}
