#include "JSONParser.h"

#include "Application.h"
#include "M_Physics.h"

#include "GameObject.h"

#include "C_RigidBody.h"
#include "C_Transform.h"
#include "C_SphereCollider.h"

#include "PhysX_3.4/Include/PxPhysicsAPI.h"

C_SphereCollider::C_SphereCollider(GameObject* owner) : Component(owner, ComponentType::SPHERE_COLLIDER)
{
	CreateCollider();
}

C_SphereCollider::~C_SphereCollider()
{
}

bool C_SphereCollider::Update()
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

			physx::PxSphereGeometry SphereGeometry(radius);
			shape = App->physics->physics->createShape(SphereGeometry, *App->physics->material);
			
			physx::PxVec3 p = physx::PxVec3(centerPosition.x, centerPosition.y, centerPosition.z);
			shape->setLocalPose(physx::PxTransform(p));
		}
		toUpdate = ColliderUpdateType::NONE;

		GetOwner()->GetComponent<C_RigidBody>()->GetRigidBody()->attachShape(*shape);
	}

	return true;
}

bool C_SphereCollider::CleanUp()
{
	if (shape)
		shape->release();

	return true;
}

bool C_SphereCollider::SaveState(ParsonNode& root) const
{
	root.SetNumber("Type", (uint)GetType());
	
	root.SetBool("Is Trigger", isTrigger);
	
	root.SetNumber("Center X", (double)centerPosition.x);
	root.SetNumber("Center Y", (double)centerPosition.y);
	root.SetNumber("Center Z", (double)centerPosition.z);
	root.SetNumber("Radius", (double)radius);

	return true;
}

bool C_SphereCollider::LoadState(ParsonNode& root)
{
	isTrigger = root.GetBool("Is Trigger");
	
	centerPosition.x = (float)root.GetNumber("Center X");
	centerPosition.y = (float)root.GetNumber("Center Y");
	centerPosition.z = (float)root.GetNumber("Center Z");
	radius = (float)root.GetNumber("Radius");
	
	toUpdate = ColliderUpdateType::ALL;

	return true;
}

void C_SphereCollider::SetIsActive(bool setTo)
{
	isActive = setTo;

	if(isActive)
		GetOwner()->GetComponent<C_RigidBody>()->GetRigidBody()->attachShape(*shape);
	else
		GetOwner()->GetComponent<C_RigidBody>()->GetRigidBody()->detachShape(*shape);
}

void C_SphereCollider::CreateCollider()
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

	physx::PxSphereGeometry SphereGeometry(radius * 1.5);
	shape = App->physics->physics->createShape(SphereGeometry, *App->physics->material);

	shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
	shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, isTrigger);

	GetOwner()->GetComponent<C_RigidBody>()->GetRigidBody()->attachShape(*shape);

	App->physics->AddActor(GetOwner()->GetComponent<C_RigidBody>()->GetRigidBody());
}

void C_SphereCollider::ToUpdate(ColliderUpdateType update)
{
	if (toUpdate == ColliderUpdateType::NONE) 
		toUpdate = update;
	else  
		toUpdate = ColliderUpdateType::ALL;
}
