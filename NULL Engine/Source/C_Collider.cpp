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
	if (GetOwner()->GetComponent<C_RigidBody>() && GetOwner()->GetComponent<C_RigidBody>()->IsStatic())
		GetOwner()->DeleteComponent(GetOwner()->GetComponent<C_RigidBody>());

	if (shape)
		shape->release();
}

bool C_Collider::Update()
{
	return true;
}

bool C_Collider::CleanUp()
{
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
}

void C_Collider::CreateCollider(ColliderType type, bool createAgain)
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

void C_Collider::DisplayComponentMenu()
{
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
