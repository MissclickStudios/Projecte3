#include "JSONParser.h"

#include "Application.h"
#include "M_Physics.h"

#include "GameObject.h"

#include "C_RigidBody.h"
#include "C_Transform.h"
#include "C_BoxCollider.h"

#include "PhysX_3.4/Include/PxPhysicsAPI.h"

#include "MemoryManager.h"

C_BoxCollider::C_BoxCollider(GameObject* owner) : Component(owner, ComponentType::BOX_COLLIDER)
{
	CreateCollider();
}

C_BoxCollider::~C_BoxCollider()
{
}

bool C_BoxCollider::Update()
{
	if (toUpdate != ColliderUpdateType::NONE)
	{
		GetOwner()->GetComponent<C_RigidBody>()->GetRigidBody()->detachShape(*shape);

		if (toUpdate == ColliderUpdateType::STATE || toUpdate == ColliderUpdateType::ALL)
		{
			shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
			shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, isTrigger);
		}
		if (toUpdate == ColliderUpdateType::SHAPE || toUpdate == ColliderUpdateType::ALL)
		{
			shape->release();

			physx::PxBoxGeometry boxGeometry = physx::PxBoxGeometry(physx::PxVec3(colliderSize.x / 2, colliderSize.y / 2, colliderSize.z / 2));
			shape = App->physics->physics->createShape(boxGeometry, *App->physics->material);

			physx::PxVec3 p = physx::PxVec3(centerPosition.x, centerPosition.y, centerPosition.z);
			shape->setLocalPose(physx::PxTransform(p));
		}
		toUpdate = ColliderUpdateType::NONE;

		GetOwner()->GetComponent<C_RigidBody>()->GetRigidBody()->attachShape(*shape);
	}

	return true;
}

bool C_BoxCollider::CleanUp()
{
	if (!shape)
		return true;

	if (GetOwner()->GetComponent<C_RigidBody>())
		GetOwner()->GetComponent<C_RigidBody>()->GetRigidBody()->detachShape(*shape);

	shape->release();
	shape = nullptr;

	return true;
}

bool C_BoxCollider::SaveState(ParsonNode& root) const
{
	root.SetNumber("Type", (uint)GetType());
	
	root.SetBool("Is Trigger", isTrigger);
	
	root.SetNumber("Size X", (double)colliderSize.x);
	root.SetNumber("Size Y", (double)colliderSize.y);
	root.SetNumber("Size Z", (double)colliderSize.z);
	root.SetNumber("Center X", (double)centerPosition.x);
	root.SetNumber("Center Y", (double)centerPosition.y);
	root.SetNumber("Center Z", (double)centerPosition.z);

	return true;
}

bool C_BoxCollider::LoadState(ParsonNode& root)
{
	isTrigger = root.GetBool("Is Trigger");
	
	colliderSize.x = (float)root.GetNumber("Size X");
	colliderSize.y = (float)root.GetNumber("Size Y");
	colliderSize.z = (float)root.GetNumber("Size Z");
	centerPosition.x = (float)root.GetNumber("Center X");
	centerPosition.y = (float)root.GetNumber("Center Y");
	centerPosition.z = (float)root.GetNumber("Center Z");
	
	toUpdate = ColliderUpdateType::ALL;

	return true;
}

void C_BoxCollider::SetIsActive(bool setTo)
{
	isActive = setTo;

	if(isActive)
		GetOwner()->GetComponent<C_RigidBody>()->GetRigidBody()->attachShape(*shape);
	else
		GetOwner()->GetComponent<C_RigidBody>()->GetRigidBody()->detachShape(*shape);
}

void C_BoxCollider::CreateCollider()
{
	if (shape)
	{
		shape->release();
		shape = nullptr;
	}

	if (GetOwner()->GetComponent<C_RigidBody>())
		App->physics->DeleteActor(GetOwner()->GetComponent<C_RigidBody>()->GetRigidBody());
	else
	{
		GetOwner()->CreateComponent(ComponentType::RIGIDBODY);
		GetOwner()->GetComponent<C_RigidBody>()->MakeStatic();
	}

	float3 size = colliderSize;

	physx::PxBoxGeometry boxGeometry = physx::PxBoxGeometry(physx::PxVec3(size.x / 2, size.y / 2, size.z / 2));
	shape = App->physics->physics->createShape(boxGeometry, *App->physics->material);

	physx::PxVec3 p = physx::PxVec3(centerPosition.x, centerPosition.y + colliderSize.y / 2, centerPosition.z);
	shape->setLocalPose(physx::PxTransform(p));
	
	shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
	shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, isTrigger);

	GetOwner()->GetComponent<C_RigidBody>()->GetRigidBody()->attachShape(*shape);

	App->physics->AddActor(GetOwner()->GetComponent<C_RigidBody>()->GetRigidBody());
}

void C_BoxCollider::ToUpdate(ColliderUpdateType update)
{
	if (toUpdate == ColliderUpdateType::NONE) 
		toUpdate = update;
	else  
		toUpdate = ColliderUpdateType::ALL;
}
