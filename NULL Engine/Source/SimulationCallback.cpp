#include <map>

#include "Application.h"
#include "M_Physics.h"

#include "GameObject.h"
#include "C_Script.h"

#include "SimulationCallback.h"

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
			for (uint i = 0; i < gameObject1->components.size(); ++i)
				if (gameObject1->components[i]->GetType() == ComponentType::SCRIPT)
					if (cPair.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
						((C_Script*)gameObject1->components[i])->OnCollisionEnter();
					else if (cPair.events & physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
						((C_Script*)gameObject1->components[i])->OnCollisionRepeat();
					else if (cPair.events & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
						((C_Script*)gameObject1->components[i])->OnCollisionExit();
			for (uint i = 0; i < gameObject2->components.size(); ++i)
				if (gameObject2->components[i]->GetType() == ComponentType::SCRIPT)
					if (cPair.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
						((C_Script*)gameObject2->components[i])->OnCollisionEnter();
					else if (cPair.events & physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
						((C_Script*)gameObject2->components[i])->OnCollisionRepeat();
					else if (cPair.events & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
						((C_Script*)gameObject2->components[i])->OnCollisionExit();
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

		if (gameObject1 && gameObject2)
		{
			for (uint i = 0; i < gameObject1->components.size(); ++i)
				if (gameObject1->components[i]->GetType() == ComponentType::SCRIPT)
					if ((pairs[i].status & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND))
						((C_Script*)gameObject1->components[i])->OnTriggerEnter();
					else if ((pairs[i].status & physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS))
						((C_Script*)gameObject1->components[i])->OnTriggerRepeat();
					else if ((pairs[i].status & physx::PxPairFlag::eNOTIFY_TOUCH_LOST))
						((C_Script*)gameObject1->components[i])->OnTriggerExit();
			for (uint i = 0; i < gameObject2->components.size(); ++i)
				if (gameObject2->components[i]->GetType() == ComponentType::SCRIPT)
					if ((pairs[i].status & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND))
						((C_Script*)gameObject2->components[i])->OnTriggerEnter();
					else if ((pairs[i].status & physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS))
						((C_Script*)gameObject2->components[i])->OnTriggerRepeat();
					else if ((pairs[i].status & physx::PxPairFlag::eNOTIFY_TOUCH_LOST))
						((C_Script*)gameObject2->components[i])->OnTriggerExit();
		}
	}
}