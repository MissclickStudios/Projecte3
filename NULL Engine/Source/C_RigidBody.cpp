#include "JSONParser.h"

#include "Application.h"
#include "Log.h"
#include "M_Physics.h"

#include "C_RigidBody.h"

#include "GameObject.h"
#include "C_Transform.h"

#include "MathGeoLib/include/Math/Quat.h"

#include "MemoryManager.h"

C_RigidBody::C_RigidBody(GameObject* owner) : Component(owner, ComponentType::RIGIDBODY)
{
	float3 position = GetOwner()->transform->GetWorldPosition();
	Quat rotation = GetOwner()->transform->GetWorldRotation();
	rigidBody = App->physics->physics->createRigidDynamic(
		physx::PxTransform(physx::PxVec3(position.x, position.y, position.z),
			physx::PxQuat(rotation.x, rotation.y, rotation.z, rotation.w)));

	if (!rigidBody)
	{
		LOG("[ERROR] RigidBody Component: Couldn't create rigidbody");
		delete this;

		return;
	}

	ApplyPhysicsChanges();

	App->physics->AddActor(rigidBody, GetOwner());
}

C_RigidBody::~C_RigidBody()
{
	if (rigidBody)
	{
		App->physics->DeleteActor(rigidBody);
		rigidBody->release();
	}
}

bool C_RigidBody::Update()
{
	if (!App->physics->simulating)
		return true;

	if (rigidBody)
	{
		RigidBodyMovesTransform();

		if (toUpdate)
			ApplyPhysicsChanges();

		physx::PxVec3 lVel = rigidBody->getLinearVelocity();
		linearVel = { lVel.x, lVel.y, lVel.z };
		physx::PxVec3 aVel = rigidBody->getAngularVelocity();
		angularVel = { aVel.x, aVel.y, aVel.z };
	}

	return true;
}

bool C_RigidBody::CleanUp()
{
	return true;
}

bool C_RigidBody::SaveState(ParsonNode& root) const
{
	if (!rigidBody)
		return false;

	root.SetNumber("Type", (uint)GetType());

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

	root.SetBool("Is Static", isStatic);

	return true;
}

bool C_RigidBody::LoadState(ParsonNode& root)
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

	isStatic = root.GetBool("Is Static");

	ApplyPhysicsChanges();

	return true;
}

void C_RigidBody::SetIsActive(bool setTo)
{
	isActive = setTo;

	if(rigidBody)
		if (isActive)
		{
			TransformMovesRigidBody(false);
			App->physics->AddActor(rigidBody, GetOwner());
		}
		else
			App->physics->DeleteActor(rigidBody);
}

void C_RigidBody::StopInertia()
{
	rigidBody->setLinearVelocity(physx::PxVec3(0, 0, 0));
	rigidBody->setAngularVelocity(physx::PxVec3(0, 0, 0));

	physx::PxVec3 lVel = rigidBody->getLinearVelocity();
	linearVel = { lVel.x, lVel.y, lVel.z };
	physx::PxVec3 aVel = rigidBody->getAngularVelocity();
	angularVel = { aVel.x, aVel.y, aVel.z };
}

void C_RigidBody::MakeStatic()
{
	isStatic = true;

	rigidBody->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
}

void C_RigidBody::MakeDynamic()
{
	isStatic = false;

	rigidBody->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, false);
}

void C_RigidBody::ApplyPhysicsChanges()
{
	App->physics->DeleteActor(rigidBody);

	rigidBody->setMass(mass);
	physx::PxRigidBodyExt::updateMassAndInertia(*rigidBody, density);

	rigidBody->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, !useGravity);
	rigidBody->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, !isKinematic);

	rigidBody->setLinearVelocity(physx::PxVec3(linearVel.x, linearVel.y, linearVel.z));
	rigidBody->setAngularVelocity(physx::PxVec3(angularVel.x, angularVel.y, angularVel.z));
	rigidBody->setLinearDamping(linearDamping);
	rigidBody->setAngularDamping(angularDamping);

	rigidBody->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X, freezePositionX);
	rigidBody->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, freezePositionY);
	rigidBody->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, freezePositionZ);
	rigidBody->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, freezeRotationX);
	rigidBody->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, freezeRotationY);
	rigidBody->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, freezeRotationZ);

	App->physics->AddActor(rigidBody, GetOwner());

	rigidBody->wakeUp();
	toUpdate = false;
}

void C_RigidBody::TransformMovesRigidBody(bool stopInertia)
{
	if (!rigidBody)
		return;

	if (stopInertia)
		StopInertia();

	App->physics->DeleteActor(rigidBody);

	float3 position = GetOwner()->transform->GetWorldPosition();
	physx::PxVec3 newPosition = physx::PxVec3(position.x, position.y, position.z);
	Quat rotation = GetOwner()->transform->GetWorldRotation();
	physx::PxQuat newRotation = physx::PxQuat(rotation.x, rotation.y, rotation.z, rotation.w);

	physx::PxTransform transform = physx::PxTransform(newPosition, newRotation);
	rigidBody->setGlobalPose(transform, true);

	App->physics->AddActor(rigidBody, GetOwner());
}

void C_RigidBody::RigidBodyMovesTransform()
{
	if (!rigidBody)
		return;

	physx::PxTransform transform = rigidBody->getGlobalPose();
	float3 position = float3(transform.p.x, transform.p.y, transform.p.z);
	Quat rotation = Quat(transform.q.x, transform.q.y, transform.q.z, transform.q.w);
	float3 scale = GetOwner()->transform->GetWorldScale();

	float4x4 newTransform = float4x4::FromTRS(position, rotation, scale);
	GetOwner()->transform->SetWorldTransform(newTransform);
}