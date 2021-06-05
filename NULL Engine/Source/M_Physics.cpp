#include "JSONParser.h"

#include "Application.h"
#include "Log.h"
#include "Profiler.h"
#include "JSONParser.h"

#include "M_Physics.h"

#include "GameObject.h"

#include "SimulationCallback.h" // PhysxAPI already included in here

#include "MemoryManager.h"

#include "MC_Time.h"

#define BOUNCE_THRESHOLD 0.2f
#define THREADS 4

#ifndef _DEBUG
#pragma comment(lib, "Source/Dependencies/PhysX_4.1/libx86/PhysRelease/PhysX_32.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_4.1/libx86/PhysRelease/PhysXCommon_32.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_4.1/libx86/PhysRelease/PhysXExtensions_static_32.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_4.1/libx86/PhysRelease/PhysXFoundation_32.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_4.1/libx86/PhysRelease/PhysXPvdSDK_static_32.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_4.1/libx86/PhysRelease/PhysXCharacterKinematic_static_32.lib")
//#pragma comment(lib, "Source/Dependencies/PhysX_4.1/libx86/PhysRelease/PhysXVehicle_static_32.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_4.1/libx86/PhysRelease/PhysXCooking_32.lib")
//#pragma comment(lib, "Source/Dependencies/PhysX_4.1/libx86/PhysRelease/PhysXTask_static_32.lib")
#else
#pragma comment(lib, "Source/Dependencies/PhysX_4.1/libx86/PhysDebug/PhysX_32.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_4.1/libx86/PhysDebug/PhysXCharacterKinematic_static_32.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_4.1/libx86/PhysDebug/PhysXCommon_32.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_4.1/libx86/PhysDebug/PhysXCooking_32.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_4.1/libx86/PhysDebug/PhysXExtensions_static_32.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_4.1/libx86/PhysDebug/PhysXFoundation_32.lib")
//#pragma comment(lib, "Source/Dependencies/PhysX_4.1/libx86/PhysDebug/PhysXVehicle_static_32.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_4.1/libx86/PhysDebug/PhysXPvdSDK_static_32.lib")
#pragma comment(lib, "Source/Dependencies/PhysX_4.1/libx86/PhysDebug/PhysXTask_static_32.lib")
#endif // _DEBUG

M_Physics::M_Physics(bool isActive) : Module("physics", isActive)
{
}

M_Physics::~M_Physics()
{
	if (filterInteractions)
	{
		for (uint i = 0; i < filters.size(); ++i)
			delete[] filterInteractions[i];
		delete[] filterInteractions;
	}
}

bool M_Physics::Init(ParsonNode& root)
{
	gravity = root.GetNumber("gravity");

	LoadConfiguration(root);


	return true;
}

enum class ContactType
{
	NOTIFY,
	IGNORE,
	DEFAULT
};

physx::PxFilterFlags customFilterShader(
	physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
	physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
	physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
{
	ContactType contact = ContactType::IGNORE;
	int filter0 = App->physics->GetFilterID((std::string*)filterData0.word0);
	int filter1 = App->physics->GetFilterID((std::string*)filterData1.word0);

	// TEMPORARY
	// TODO: ADD A SYSTEM TO ALLOW FILTERS TO DETECT COLLISIONS WITH DEFAULTS
	bool allowDefaultCollisions = false;
	if (filter0 == 1 || filter1 == 1 || filter0 == 4 || filter1 == 4)
		allowDefaultCollisions = true;
	// ---

	if (filter0 == -1 || filter1 == -1)
		contact = ContactType::DEFAULT;
	else if (App->physics->GetInteractions()[filter0][filter1])
		contact = ContactType::NOTIFY;

	if (contact != ContactType::IGNORE)
	{
		pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;
		if (contact == ContactType::NOTIFY || allowDefaultCollisions)
		{
			pairFlags |= physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS;
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_LOST;
		}
	}

	return physx::PxFilterFlag::eDEFAULT;
}

bool M_Physics::Start()
{
	LOG("Initializing PhysX 3.4 --------------");
	static physx::PxDefaultErrorCallback gDefaultErrorCallback;
	static physx::PxDefaultAllocator gDefaultAllocatorCallback;

	foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
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
	sceneDesc.bounceThresholdVelocity = gravity* BOUNCE_THRESHOLD;
	sceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(THREADS);
	//sceneDesc.flags |= physx::PxSceneFlag::eENABLE_KINEMATIC_PAIRS | physx::PxSceneFlag::eENABLE_KINEMATIC_STATIC_PAIRS;
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

	material = physics->createMaterial(0, 0, 0);

	LOG("Physics Controller Manager created succesfully");
	LOG("PhysX 3.4 Initialized correctly --------------");

	return true;
}

UpdateStatus M_Physics::Update(float dt)
{
	OPTICK_CATEGORY("M_Physics Update", Optick::Category::Module)

	if (App->gameState == GameState::PLAY)
		simulating = true;
	else
		simulating = false;

	if (dt < 0.5f && MC_Time::Game::GetDT() < 0.5f)
		if (scene && simulating)
		{
			scene->simulate(MC_Time::Game::GetDT());
			scene->fetchResults(true);
		}

	return UpdateStatus::CONTINUE;
}

bool M_Physics::CleanUp()
{	
	RELEASE(simulationCallback);
	
	if (controllerManager != nullptr)
		controllerManager->release();

	if (material != nullptr)
		material->release();

	if (cooking != nullptr)
		cooking->release();

	if (scene != nullptr)
		scene->release();

	if (physics != nullptr)
		physics->release();

	if (foundation != nullptr)
		foundation->release();

	controllerManager	= nullptr;
	physics				= nullptr;
	foundation			= nullptr;
	scene				= nullptr;

	return true;
}

bool M_Physics::LoadConfiguration(ParsonNode& root)
{
	if (filters.size() != 0)
		filters.erase(filters.begin(), filters.end());

	ParsonArray filtersArray = root.GetArray("Filters");
	for (uint i = 0; i < filtersArray.size; ++i)
		filters.push_back(filtersArray.GetString(i));

	if (filterInteractions)
	{
		for (uint i = 0; i < filters.size(); ++i)
			delete[] filterInteractions[i];
		delete[] filterInteractions;

		filterInteractions = nullptr;
	}
	if (!filters.size())
		return true;

	filterInteractions = new bool*[filters.size()];
	for (uint i = 0; i < filters.size(); ++i)
		filterInteractions[i] = new bool[filters.size()];

	ParsonArray interactionsArray = root.GetArray("Interactions");
	uint x = 0;
	uint y = 0;
	for (uint i = 0; i < interactionsArray.size; ++i)
	{
		filterInteractions[x][y] = interactionsArray.GetBool(i);

		++x;
		if (x >= filters.size())
		{
			x = 0;
			++y;
		}
	}

	return true;
}

bool M_Physics::SaveConfiguration(ParsonNode& root) const
{

	root.SetNumber("gravity", gravity);

	int size = filters.size();

	ParsonArray filtersArray = root.SetArray("Filters");
	for (uint i = 0; i < size; ++i)
		filtersArray.SetString(filters[i].c_str());

	if (!filterInteractions)
		return true;
	ParsonArray interactionsArray = root.SetArray("Interactions");
	for (uint y = 0; y < size; ++y)
		for (uint x = 0; x < size; ++x)
			interactionsArray.SetBool(filterInteractions[x][y]);

	return true;
}

void M_Physics::AddActor(physx::PxActor* actor, GameObject* owner)
{
	if (!actor)
		return;

	scene->addActor(*actor);
	actors.insert(std::make_pair<physx::PxRigidActor*, GameObject*>((physx::PxRigidActor*)actor, (GameObject*)(void*)owner));
}

void M_Physics::DeleteActor(physx::PxActor* actor)
{
	if (!actor)
		return;

	scene->removeActor(*actor);
	actors.erase((physx::PxRigidActor*)actor);
}

void M_Physics::SetGravity(float value)
{
	gravity = value;
	scene->setGravity(physx::PxVec3(0.0f, -gravity, 0.0f));
}

const std::string* const M_Physics::GetFilter(int id) const
{
	--id;

	if (id < 0)
		return &defaultFilter;
	else if (id >= filters.size())
		return nullptr;

	return &filters[id];
}

const int M_Physics::GetFilterID(const std::string* const filter)
{
	if (!filter)
		return -1;
	for (uint i = 0; i < filters.size(); ++i)
		if (filters[i] == *filter)
			return i;
	return -1;
}

void M_Physics::CreateFilter(const std::string& filter)
{
	for (uint i = 0; i < filters.size(); ++i)
		if (filters[i] == filter)
		{
			LOG("[ERROR] The filter [%s] already exists", filter.c_str());
			return;
		}

	filters.push_back(filter);

	// Create new filter interactions
	bool** newFilterInteractions = new bool* [filters.size()];
	for (uint i = 0; i < filters.size(); ++i)
		newFilterInteractions[i] = new bool[filters.size()];

	// Copy all old filter interactions
	for (uint y = 0; y < filters.size(); ++y)
		for (uint x = 0; x < filters.size(); ++x) 
			if (x == filters.size() - 1 || y == filters.size() - 1)			// Exclude all the value related to the new filter (cant be copied, they didnt exist)
				newFilterInteractions[x][y] = true;							// Set new filter values to true
			else
				newFilterInteractions[x][y] = filterInteractions[x][y];
				
	// Free old interactions
	for (uint i = 0; i < filters.size() - 1; ++i)
		delete[] filterInteractions[i];
	delete[] filterInteractions;

	filterInteractions = newFilterInteractions;
}

void M_Physics::DeleteFilter(const std::string& filter)
{
	uint deletedID = 0;
	bool toBreak = false;
	for (std::vector<std::string>::iterator itr = filters.begin(); itr != filters.end(); ++itr)
	{
		if (*itr == filter)
		{
			filters.erase(itr);
			toBreak = true;
		}
		if (toBreak)
			break;
		++deletedID;
	}
	if (!toBreak)
	{
		LOG("[ERROR] Can't delete inexistent filter DUMBASS");
		return;
	}

	// Create new filter interactions
	bool** newFilterInteractions = new bool* [filters.size()];
	for (uint i = 0; i < filters.size(); ++i)
		newFilterInteractions[i] = new bool[filters.size()];

	// Copy all old filter interactions
	uint newX = 0;
	uint newY = 0;
	for (uint y = 0; y < filters.size() + 1; ++y)
	{
		if (y == deletedID)
			continue;
		for (uint x = 0; x < filters.size() + 1; ++x)
		{
			if (x == deletedID)
				continue;

			newFilterInteractions[newX][newY] = filterInteractions[x][y];
			++newX;
		}
		newX = 0;
		++newY;
	}

	// Free old interactions
	for (uint i = 0; i < filters.size() + 1; ++i)
		delete[] filterInteractions[i];
	delete[] filterInteractions;

	filterInteractions = newFilterInteractions;
}
