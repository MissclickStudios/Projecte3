#include "JSONParser.h"

#include "Profiler.h"
#include "Application.h"
#include "M_Physics.h"

#include "GameObject.h"

#include "C_RigidBody.h"
#include "C_Transform.h"
#include "C_BoxCollider.h"

#include "PhysX_3.4/Include/PxPhysicsAPI.h"

#include "MathGeoLib/include/Math/float3x3.h"
#include "MathGeoBoundingBox.h"

#include "MemoryManager.h"

C_BoxCollider::C_BoxCollider(GameObject* owner) : Component(owner, ComponentType::BOX_COLLIDER)
{
	boxColliderVertices = new float3[8];

	CreateCollider();
}

C_BoxCollider::~C_BoxCollider()
{
	RELEASE_ARRAY(boxColliderVertices);
}

bool C_BoxCollider::Update()
{
	OPTICK_CATEGORY("C_BoxCollider Update", Optick::Category::Update);

	fil = (std::string*)shape->getSimulationFilterData().word0;

	if (toUpdate != ColliderUpdateType::NONE)
	{
		GetOwner()->GetComponent<C_RigidBody>()->GetRigidBody()->detachShape(*shape);

		if (toUpdate == ColliderUpdateType::STATE || toUpdate == ColliderUpdateType::ALL)
		{
			shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
			shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, isTrigger);
			
			physx::PxFilterData filterData;
			const std::string* filter = GetOwner()->GetComponent<C_RigidBody>()->GetFilter();
			filterData.word0 = (int)filter;

			shape->setSimulationFilterData(filterData);
			shape->setQueryFilterData(filterData);
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
	root.SetBool("Show Collider", showBoxCollider);
	
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
	showBoxCollider = root.GetBool("Show Collider");
	
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

float3* C_BoxCollider::GetCornerPoints() const
{
	physx::PxTransform transform = GetOwner()->GetComponent<C_RigidBody>()->GetRigidBody()->getGlobalPose();
	float3x3 rotation = Quat(transform.q.x, transform.q.y, transform.q.z, transform.q.w).ToFloat3x3().Inverted();
	float3 position = float3(transform.p.x, transform.p.y, transform.p.z);

	float3 axis[3] = { rotation.Row(0), rotation.Row(1), rotation.Row(2) };
	OBB obb(position + centerPosition, colliderSize / 2, axis[0], axis[1], axis[2]);
	obb.GetCornerPoints(boxColliderVertices);

	return boxColliderVertices;
}

void C_BoxCollider::UpdateFilter()
{
	ToUpdate(ColliderUpdateType::STATE);
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

	physx::PxFilterData filterData;
	filterData.word0 = (int)GetOwner()->GetComponent<C_RigidBody>()->GetFilter();

	shape->setSimulationFilterData(filterData);
	shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
	shape->setQueryFilterData(filterData);

	GetOwner()->GetComponent<C_RigidBody>()->GetRigidBody()->attachShape(*shape);

	App->physics->AddActor(GetOwner()->GetComponent<C_RigidBody>()->GetRigidBody(), GetOwner());
}

void C_BoxCollider::ToUpdate(ColliderUpdateType update)
{
	if (toUpdate == ColliderUpdateType::NONE) 
		toUpdate = update;
	else  
		toUpdate = ColliderUpdateType::ALL;
}
