#include "Application.h"
#include "C_Collider.h"
#include "GameObject.h"
#include "C_RigidBody.h"
#include "M_Physics.h"
#include "C_Transform.h"
#include "PhysX_3.4/Include/PxPhysicsAPI.h"

ColliderComponent::ColliderComponent(GameObject* owner, ColliderType type) : Component(owner, ComponentType::COLLIDER)
{
	this->type = type;
	CreateCollider(this->type);
}

ColliderComponent::~ColliderComponent()
{
	if (GetOwner()->GetComponent<C_RigidBody>() && GetOwner()->GetComponent<C_RigidBody>()->IsStatic())
		GetOwner()->DeleteComponent(GetOwner()->GetComponent<C_RigidBody>());

	if (shape)
		shape->release();
}

bool ColliderComponent::Update()
{
	return true;
}

bool ColliderComponent::CleanUp()
{
	return true;
}

bool ColliderComponent::SaveState(ParsonNode& root) const
{
	return false;
}

bool ColliderComponent::LoadState(ParsonNode& root)
{
	return false;
}

void ColliderComponent::SetIsActive(bool setTo)
{
	isActive = setTo;
}

void ColliderComponent::CreateCollider(ColliderType type, bool createAgain)
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

void ColliderComponent::DisplayComponentMenu()
{
}
