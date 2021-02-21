#include "Application.h"
#include "C_Collider.h"
#include "GameObject.h"
#include "C_RigidBody.h"
#include "M_Physics.h"
#include "C_Transform.h"
#include "PhysX_3.4/Include/PxPhysicsAPI.h"

C_Collider::C_Collider(GameObject* owner, ColliderType type) : Component(owner, GetComponentType(type))
{
	colType = type;
	CreateCollider(colType);
}

C_Collider::~C_Collider()
{
}

bool C_Collider::Update()
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

			switch (colType)
			{
			case ColliderType::BOX:
			{
				physx::PxBoxGeometry boxGeometry = physx::PxBoxGeometry(physx::PxVec3(colliderSize.x / 2, colliderSize.y / 2, colliderSize.z / 2));
				shape = App->physics->physics->createShape(boxGeometry, *App->physics->material);
				break;
			}
			case ColliderType::SPHERE:
			{
				physx::PxSphereGeometry SphereGeometry(radius);
				shape = App->physics->physics->createShape(SphereGeometry, *App->physics->material);
				break;
			}
			case ColliderType::CAPSULE:
			{
				physx::PxCapsuleGeometry CapsuleGeometry(radius, height);
				shape = App->physics->physics->createShape(CapsuleGeometry, *App->physics->material);
				break;
			}
			}
			physx::PxVec3 p = physx::PxVec3(centerPosition.x, centerPosition.y, centerPosition.z);
			shape->setLocalPose(physx::PxTransform(p));
		}
		toUpdate = ColliderUpdateType::NONE;

		GetOwner()->GetComponent<C_RigidBody>()->GetRigidBody()->attachShape(*shape);
	}

	return true;
}

bool C_Collider::CleanUp()
{
	if (shape)
		shape->release();

	return true;
}

bool C_Collider::SaveState(ParsonNode& root) const
{
	return false;
}

bool C_Collider::LoadState(ParsonNode& root)
{
	return false;
}

void C_Collider::SetIsActive(bool setTo)
{
	isActive = setTo;

	if(isActive)
		GetOwner()->GetComponent<C_RigidBody>()->GetRigidBody()->attachShape(*shape);
	else
		GetOwner()->GetComponent<C_RigidBody>()->GetRigidBody()->detachShape(*shape);
}

void C_Collider::CreateCollider(ColliderType type)
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

	float3 size;
	if (colliderSize.IsZero())
		size = GetOwner()->transform->GetWorldScale();
	else
		size = colliderSize;

	switch (type)
	{
	case ColliderType::BOX:
	{
		colliderSize = { size.x, size.y, size.z };
		physx::PxBoxGeometry boxGeometry = physx::PxBoxGeometry(physx::PxVec3(size.x / 2, size.y / 2, size.z / 2));
		shape = App->physics->physics->createShape(boxGeometry, *App->physics->material);

		physx::PxVec3 p = physx::PxVec3(centerPosition.x, centerPosition.y + colliderSize.y / 2, centerPosition.z);
		shape->setLocalPose(physx::PxTransform(p));
		break;
	}
	case ColliderType::SPHERE:
	{
		physx::PxSphereGeometry SphereGeometry(radius * 1.5);
		shape = App->physics->physics->createShape(SphereGeometry, *App->physics->material);
		break;
	}
	case ColliderType::CAPSULE:
	{
		physx::PxCapsuleGeometry CapsuleGeometry(radius, height);
		shape = App->physics->physics->createShape(CapsuleGeometry, *App->physics->material);
		break;
	}
	}

	GetOwner()->GetComponent<C_RigidBody>()->GetRigidBody()->attachShape(*shape);
	//GetOwner()->GetComponent<C_RigidBody>()->ApplyPhysicsChanges();

	App->physics->AddActor(GetOwner()->GetComponent<C_RigidBody>()->GetRigidBody());
}

void C_Collider::ToUpdate(ColliderUpdateType update)
{
	if (toUpdate == ColliderUpdateType::NONE) 
		toUpdate = update;
	else  
		toUpdate = ColliderUpdateType::ALL;
}

ComponentType C_Collider::GetComponentType(ColliderType type)
{
	switch (type)
	{
	case ColliderType::BOX:
		return ComponentType::BOX_COLLIDER;
	case ColliderType::SPHERE:
		return ComponentType::SPHERE_COLLIDER;
	case ColliderType::CAPSULE:
		return ComponentType::CAPSULE_COLLIDER;
	}
	return ComponentType::NONE;
}
