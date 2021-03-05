#include <map>

#include "Application.h"
#include "M_Physics.h"

#include "GameObject.h"

#include "SimulationCallback.h"

#include "MemoryManager.h"

SimulationCallback::SimulationCallback() {}

void SimulationCallback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
	for (physx::PxU32 i = 0; i < nbPairs; ++i)
	{
		const physx::PxContactPair& cPair = pairs[i];

		GameObject* gameObject1 = nullptr;
		GameObject* gameObject2 = nullptr;
		gameObject1 = App->physics->actors[(physx::PxRigidDynamic*)pairHeader.actors[0]];
		gameObject2 = App->physics->actors[(physx::PxRigidDynamic*)pairHeader.actors[1]];

		if (gameObject1 && gameObject2)
		{
			if (cPair.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				LOG("CONTACT: DOWN");
			}
			else if (cPair.events & physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
			{
			}
			else if (cPair.events & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				LOG("CONTACT: UP");
			}
		}
	}
}

void SimulationCallback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
{
	for (physx::PxU32 i = 0; i < count; ++i)
	{
		GameObject* gameObject1 = nullptr;
		GameObject* gameObject2 = nullptr;
		gameObject1 = App->physics->actors[(physx::PxRigidDynamic*)pairs[i].triggerActor];
		gameObject2 = App->physics->actors[(physx::PxRigidDynamic*)pairs[i].otherActor];


		if (gameObject1 != nullptr && gameObject2 != nullptr)
		{
			if ((pairs[i].status & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND))
			{
				LOG("TRIGGER: DOWN");
			}
			else if ((pairs[i].status & physx::PxPairFlag::eNOTIFY_TOUCH_LOST))
			{
				LOG("TRIGGER: UP");
			}
		}
	}

}