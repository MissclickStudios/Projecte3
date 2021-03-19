#include <map>

#include "Application.h"
#include "Log.h"
#include "M_Physics.h"

#include "GameObject.h"
#include "C_BulletBehavior.h"
#include "C_PropBehavior.h"
#include "C_GateBehavior.h"
#include "C_PlayerController.h"

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
			if (gameObject1->GetComponent<C_GateBehavior>() && gameObject2->GetComponent<C_PlayerController>())
			{
				gameObject1->GetComponent<C_GateBehavior>()->OnCollisionEnter();
				return;
			}
			if (gameObject2->GetComponent<C_GateBehavior>() && gameObject1->GetComponent<C_PlayerController>())
			{
				gameObject2->GetComponent<C_GateBehavior>()->OnCollisionEnter();
				return;
			}

			for (int i = 0; i < gameObject1->components.size(); ++i)
			{
				if (gameObject1->components[i]->GetType() == ComponentType::PLAYER_CONTROLLER)
					return;
			}
			for (int i = 0; i < gameObject2->components.size(); ++i)
			{
				if (gameObject2->components[i]->GetType() == ComponentType::PLAYER_CONTROLLER)
					return;
			}
			if (cPair.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				for (int i = 0; i < gameObject1->components.size(); ++i)
				{
					if (gameObject1->components[i]->GetType() == ComponentType::BULLET_BEHAVIOR)
						((C_BulletBehavior*)gameObject1->components[i])->OnCollisionEnter();
					else if (gameObject1->components[i]->GetType() == ComponentType::PROP_BEHAVIOR)
						((C_PropBehavior*)gameObject1->components[i])->OnCollisionEnter();
				}
				for (int i = 0; i < gameObject2->components.size(); ++i)
				{
					if (gameObject2->components[i]->GetType() == ComponentType::BULLET_BEHAVIOR)
						((C_BulletBehavior*)gameObject2->components[i])->OnCollisionEnter();
					else if (gameObject2->components[i]->GetType() == ComponentType::PROP_BEHAVIOR)
						((C_PropBehavior*)gameObject2->components[i])->OnCollisionEnter();
				}
			}
			else if (cPair.events & physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
			{
			}
			else if (cPair.events & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
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
			}
			else if ((pairs[i].status & physx::PxPairFlag::eNOTIFY_TOUCH_LOST))
			{
			}
		}
	}

}