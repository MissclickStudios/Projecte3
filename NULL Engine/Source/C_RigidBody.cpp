#include "JSONParser.h"

#include "Application.h"
#include "Log.h"
#include "M_Physics.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_RigidBody.h"
#include "C_BoxCollider.h"
#include "C_SphereCollider.h"
#include "C_CapsuleCollider.h"

#include "MathGeoLib/include/Math/Quat.h"

#include "MemoryManager.h"

C_RigidBody::C_RigidBody(GameObject* owner) : Component(owner, ComponentType::RIGIDBODY)
{
	float3 position = GetOwner()->transform->GetWorldPosition();
	Quat rotation = GetOwner()->transform->GetWorldRotation();
	dynamicBody = App->physics->physics->createRigidDynamic(
		physx::PxTransform(physx::PxVec3(position.x, position.y, position.z),
			physx::PxQuat(rotation.x, rotation.y, rotation.z, rotation.w)));

	if (!dynamicBody)
	{
		LOG("[ERROR] RigidBody Component: Couldn't create rigidbody");
		GetOwner()->DeleteComponent(this);

		return;
	}

	ApplyPhysicsChanges();
}

C_RigidBody::~C_RigidBody()
{
}

bool C_RigidBody::Update()
{
	if (!App->physics->simulating)
		return true;

	if (toChangeFilter)
	{
		toChangeFilter = false;
		ChangeFilter(filter);
	}

	if (!isStatic)
	{
		if (dynamicBody)
		{
			RigidBodyMovesTransform();

			if (toUpdate)
				ApplyPhysicsChanges();

			physx::PxVec3 lVel = dynamicBody->getLinearVelocity();
			linearVel = { lVel.x, lVel.y, lVel.z };
			physx::PxVec3 aVel = dynamicBody->getAngularVelocity();
			angularVel = { aVel.x, aVel.y, aVel.z };
		}
	}
	else
		TransformMovesRigidBody(false);

	return true;
}

bool C_RigidBody::CleanUp()
{
	if (dynamicBody)
	{
		App->physics->DeleteActor(dynamicBody);
		dynamicBody->release();
	}
	if (staticBody)
	{
		App->physics->DeleteActor(staticBody);
		staticBody->release();
	}
	
	return true;
}

bool C_RigidBody::SaveState(ParsonNode& root) const
{
	root.SetNumber("Type", (uint)GetType());
	
	root.SetBool("Is Static", isStatic);
	root.SetString("Filter", filter.c_str());

	if (!dynamicBody)
		return false;

	root.SetNumber("Mass", (double)mass);
	root.SetNumber("Density", (double)density);
	root.SetNumber("Linear Damping", (double)linearDamping);
	root.SetNumber("Angular Damping", (double)angularDamping);

	root.SetBool("Use Gravity", useGravity);
	root.SetBool("Is Kinematic", isKinematic);
	root.SetBool("Freeze Position X", freezePositionX);
	root.SetBool("Freeze Position Y", freezePositionY);
	root.SetBool("Freeze Position Z", freezePositionZ);
	root.SetBool("Freeze Rotation X", freezeRotationX);
	root.SetBool("Freeze Rotation Y", freezeRotationY);
	root.SetBool("Freeze Rotation Z", freezeRotationZ);

	return true;
}

bool C_RigidBody::LoadState(ParsonNode& root)
{
	isStatic = root.GetBool("Is Static");
	if (isStatic)
		MakeStatic();
	else
	{
		mass = (float)root.GetNumber("Mass");
		density = (float)root.GetNumber("Density");
		linearDamping = (float)root.GetNumber("Linear Damping");
		angularDamping = (float)root.GetNumber("Angular Damping");

		useGravity = root.GetBool("Use Gravity");
		isKinematic = root.GetBool("Is Kinematic");
		freezePositionX = root.GetBool("Freeze Position X");
		freezePositionY = root.GetBool("Freeze Position Y");
		freezePositionZ = root.GetBool("Freeze Position Z");
		freezeRotationX = root.GetBool("Freeze Rotation X");
		freezeRotationY = root.GetBool("Freeze Rotation Y");
		freezeRotationZ = root.GetBool("Freeze Rotation Z");
	}

	filter = root.GetString("Filter");
	// Used because when loading the rigidbody is created before the colliders so whe have to wait a till the update to update their filters
	toChangeFilter = true;				

	ApplyPhysicsChanges();

	return true;
}

void C_RigidBody::SetIsActive(bool setTo)
{
	isActive = setTo;

	physx::PxRigidActor* body = nullptr;
	if (isStatic)
		body = staticBody;
	else
		body = dynamicBody;
	if (!body)
		return;


	if(body)
		if (isActive)
		{
			toChangeFilter = true;
			TransformMovesRigidBody(false);
			App->physics->AddActor(body, GetOwner());
		}
		else
			App->physics->DeleteActor(body);
}

void C_RigidBody::StopInertia()
{
	if (isStatic)
		return;

	dynamicBody->setLinearVelocity(physx::PxVec3(0, 0, 0));
	dynamicBody->setAngularVelocity(physx::PxVec3(0, 0, 0));

	physx::PxVec3 lVel = dynamicBody->getLinearVelocity();
	linearVel = { lVel.x, lVel.y, lVel.z };
	physx::PxVec3 aVel = dynamicBody->getAngularVelocity();
	angularVel = { aVel.x, aVel.y, aVel.z };
}

void C_RigidBody::ChangeFilter(const std::string& const filter)
{
	this->filter = filter;

	for (uint i = 0; i < GetOwner()->components.size(); ++i)
	{
		switch (GetOwner()->components[i]->GetType())
		{
		case ComponentType::BOX_COLLIDER:
			((C_BoxCollider*)GetOwner()->components[i])->UpdateFilter();
			break;
		case ComponentType::SPHERE_COLLIDER:
			((C_SphereCollider*)GetOwner()->components[i])->UpdateFilter();
			break;
		case ComponentType::CAPSULE_COLLIDER:
			((C_CapsuleCollider*)GetOwner()->components[i])->UpdateFilter();
			break;
		}
	}
}

void C_RigidBody::MakeStatic()
{
	isStatic = true;
	toChangeFilter = true;
	
	if (dynamicBody)
	{

		App->physics->DeleteActor(dynamicBody);

		physx::PxShape* shapes = nullptr;
		dynamicBody->getShapes(&shapes, dynamicBody->getNbShapes());

		float3 position = GetOwner()->transform->GetWorldPosition();
		Quat rotation = GetOwner()->transform->GetWorldRotation();
		staticBody = App->physics->physics->createRigidStatic(physx::PxTransform(
			physx::PxVec3(position.x, position.y, position.z),
			physx::PxQuat(rotation.x, rotation.y, rotation.z, rotation.w)));
		if (!staticBody)
		{
			LOG("[ERROR] RigidBody Component: Couldn't create rigidbody");
			GetOwner()->DeleteComponent(this);

			return;
		}

		if (shapes)
			for (uint i = 0; i < dynamicBody->getNbShapes(); ++i)
				staticBody->attachShape(shapes[i]);

		dynamicBody->release();
		dynamicBody = nullptr;

		App->physics->AddActor(staticBody, GetOwner());
	}
}

void C_RigidBody::MakeDynamic()
{
	isStatic = false;
	toChangeFilter = true;

	if (staticBody)
	{
		App->physics->DeleteActor(staticBody);

		physx::PxShape* shapes = nullptr;
		staticBody->getShapes(&shapes, staticBody->getNbShapes());

		float3 position = GetOwner()->transform->GetWorldPosition();
		Quat rotation = GetOwner()->transform->GetWorldRotation();
		dynamicBody = App->physics->physics->createRigidDynamic(physx::PxTransform(
			physx::PxVec3(position.x, position.y, position.z),
			physx::PxQuat(rotation.x, rotation.y, rotation.z, rotation.w)));
		if (!dynamicBody)
		{
			LOG("[ERROR] RigidBody Component: Couldn't create rigidbody");
			GetOwner()->DeleteComponent(this);

			return;
		}

		if (shapes)
			for (uint i = 0; i < staticBody->getNbShapes(); ++i)
				dynamicBody->attachShape(shapes[i]);

		staticBody->release();
		staticBody = nullptr;

		App->physics->AddActor(dynamicBody, GetOwner());
	}
}

void C_RigidBody::ApplyPhysicsChanges()
{
	if (!isStatic)
	{
		App->physics->DeleteActor(dynamicBody);

		dynamicBody->setMass(mass);
		physx::PxRigidBodyExt::updateMassAndInertia(*dynamicBody, density);

		dynamicBody->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, !useGravity);
		dynamicBody->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, !isKinematic);

		dynamicBody->setLinearVelocity(physx::PxVec3(linearVel.x, linearVel.y, linearVel.z));
		dynamicBody->setAngularVelocity(physx::PxVec3(angularVel.x, angularVel.y, angularVel.z));
		dynamicBody->setLinearDamping(linearDamping);
		dynamicBody->setAngularDamping(angularDamping);

		dynamicBody->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X, freezePositionX);
		dynamicBody->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, freezePositionY);
		dynamicBody->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, freezePositionZ);
		dynamicBody->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, freezeRotationX);
		dynamicBody->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, freezeRotationY);
		dynamicBody->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, freezeRotationZ);

		App->physics->AddActor(dynamicBody, GetOwner());
		dynamicBody->wakeUp();
	}

	toUpdate = false;
}

void C_RigidBody::TransformMovesRigidBody(bool stopInertia)
{
	physx::PxRigidActor* body = nullptr;
	if (isStatic)
		body = staticBody;
	else
		body = dynamicBody;
	if (!body)
		return;

	if (stopInertia)
		StopInertia();

	App->physics->DeleteActor(body);

	float3 position = GetOwner()->transform->GetWorldPosition();
	physx::PxVec3 newPosition = physx::PxVec3(position.x, position.y, position.z);
	Quat rotation = GetOwner()->transform->GetWorldRotation();
	physx::PxQuat newRotation = physx::PxQuat(rotation.x, rotation.y, rotation.z, rotation.w);

	physx::PxTransform transform = physx::PxTransform(newPosition, newRotation);
	body->setGlobalPose(transform, true);

	App->physics->AddActor(body, GetOwner());
}

void C_RigidBody::RigidBodyMovesTransform()
{
	physx::PxRigidActor* body = nullptr;
	if (isStatic)
		body = staticBody;
	else
		body = dynamicBody;
	if (!body)
		return;

	physx::PxTransform transform = body->getGlobalPose();
	float3 position = float3(transform.p.x, transform.p.y, transform.p.z);
	Quat rotation = Quat(transform.q.x, transform.q.y, transform.q.z, transform.q.w);
	float3 scale = GetOwner()->transform->GetWorldScale();

	float4x4 newTransform = float4x4::FromTRS(position, rotation, scale);
	GetOwner()->transform->SetWorldTransform(newTransform);
}
