#include "Application.h"
#include "M_Physics.h"

#include "C_RigidBody.h"

#include "GameObject.h"
#include "C_Transform.h"

#include "MathGeoLib/include/Math/Quat.h"

C_RigidBody::C_RigidBody(GameObject* owner) : Component(owner, ComponentType::RIGIDBODY)
{
	float3 position = GetOwner()->transform->GetWorldPosition();
	Quat rotation = GetOwner()->transform->GetWorldRotation();
	rigidBody = App->physics->physics->createRigidDynamic(
		physx::PxTransform(physx::PxVec3(position.x, position.y, position.z),
			physx::PxQuat(rotation.x, rotation.y, rotation.z, rotation.w)));

	if (!rigidBody)
	{
		delete this;

		LOG("[ERROR] RigidBody Component: Couldn't create rigidbody");
		return;
	}

	ApplyPhysicsChanges();

	App->physics->AddActor(rigidBody);
}

C_RigidBody::~C_RigidBody()
{
	if (rigidBody)
	{
		App->physics->DeleteActor(rigidBody);
		rigidBody->release();
		//GetOwner()->rigidbody = nullptr;
	}
}

bool C_RigidBody::Update()
{
	if (App->pause)
		return true;

	if (rigidBody)
	{
		RigidBodyMovesTransform();

		if (update)
			ApplyPhysicsChanges();
	}

	return true;
}

bool C_RigidBody::CleanUp()
{
	return true;
}

bool C_RigidBody::SaveState(ParsonNode& root) const
{
	return true;
}

bool C_RigidBody::LoadState(ParsonNode& root)
{
	return true;
}

void C_RigidBody::MakeStatic()
{
	isStatic = true;

	freezePositionX = true;
	freezePositionY = true;
	freezePositionZ = true;
	freezeRotationX = true;
	freezeRotationY = true;
	freezeRotationZ = true;
	useGravity = false;
	mass = 1000000.0f;
	massBuffer = mass;
	density = 1000.0f;
	densityBuffer = density;

	update = true;
	ApplyPhysicsChanges();
}

void C_RigidBody::MakeDynamic()
{
	isStatic = false;

	freezePositionX = false;
	freezePositionY = false;
	freezePositionZ = false;
	freezeRotationX = false;
	freezeRotationY = false;
	freezeRotationZ = false;
	useGravity = true;
	mass = 10.0f;
	massBuffer = mass;
	density = 1.0f;
	densityBuffer = density;

	ApplyPhysicsChanges();
}

void C_RigidBody::ApplyPhysicsChanges()
{
	App->physics->DeleteActor(rigidBody);

	rigidBody->setMass(mass);
	physx::PxRigidBodyExt::updateMassAndInertia(*rigidBody, density);

	rigidBody->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, !useGravity);
	rigidBody->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, isKinematic);

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

	App->physics->AddActor(rigidBody);

	rigidBody->wakeUp();
	update = false;
}

void C_RigidBody::TransformMovesRigidBody(bool stopInertia)
{
	if (!rigidBody)
		return;

	if (stopInertia)
		SetLinearVelocity(float3::zero);

	App->physics->DeleteActor(rigidBody);

	float3 position = GetOwner()->transform->GetWorldPosition();
	physx::PxVec3 newPosition = physx::PxVec3(position.x, position.y, position.z);
	Quat rotation = GetOwner()->transform->GetWorldRotation();
	physx::PxQuat newRotation = physx::PxQuat(rotation.x, rotation.y, rotation.z, rotation.w);

	physx::PxTransform transform = physx::PxTransform(newPosition, newRotation);
	rigidBody->setGlobalPose(transform, true);

	App->physics->AddActor(rigidBody);
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