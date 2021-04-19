#include "Profiler.h"

#include "JSONParser.h"

#include "Application.h"
#include "VariableDefinitions.h"

#include "GameObject.h"
#include "C_Camera.h"
#include "C_RigidBody.h"

#include "C_Transform.h"

#include "MemoryManager.h"

C_Transform::C_Transform(GameObject* owner) : Component(owner, ComponentType::TRANSFORM),
localTransform	(float4x4::identity),
worldTransform	(float4x4::identity),
updateWorld		(false),
syncLocal		(false)
{	
	localTransform.Decompose(localPosition, localRotation, localScale);

	localEulerRotation = localRotation.ToEulerXYZ();
}

C_Transform::~C_Transform()
{

}

bool C_Transform::Update()
{
	/*if (updateWorld)
		UpdateWorldTransform();

	if (syncLocal)
		SyncLocalToWorld();*/

	return true;
}

bool C_Transform::CleanUp()
{
	return true;
}

bool C_Transform::SaveState(ParsonNode& root) const
{
	root.SetNumber("Type", (uint)GetType());

	root.SetFloat3("LocalPosition", localPosition);
	root.SetFloat4("LocalRotation", localRotation.CastToFloat4());
	root.SetFloat3("LocalScale", localScale);

	return true;
}

bool C_Transform::LoadState(ParsonNode& root)
{
	localPosition	= root.GetFloat3("LocalPosition");
	localRotation	= root.GetQuat("LocalRotation");
	localScale		= root.GetFloat3("LocalScale");

	UpdateLocalTransform();

	return true;
}

// ------ C_TRANSFORM METHODS ------
// --- TRANSFORMS/MATRICES
void C_Transform::UpdateLocalTransform()
{
	localTransform = float4x4::FromTRS(localPosition, localRotation, localScale);

	updateWorld = true;
}

void C_Transform::UpdateWorldTransform()
{
	OPTICK_CATEGORY("Update World Transform", Optick::Category::GameLogic);
	
	GameObject* owner = GetOwner();

	worldTransform = (owner->parent != nullptr) ? owner->parent->GetComponent<C_Transform>()->worldTransform * localTransform : localTransform;

	SetChildsAsDirty();
	
	updateWorld = false;

	C_Camera* c_camera = owner->GetComponent<C_Camera>();
	if (c_camera != nullptr)
	{
		c_camera->UpdateFrustumTransform();
	}
}

void C_Transform::SyncWorldToLocal()
{
	UpdateWorldTransform();
}

void C_Transform::SyncLocalToWorld()
{
	GameObject* owner = GetOwner();
	
	localTransform = (owner->parent != nullptr) ? owner->parent->GetComponent<C_Transform>()->worldTransform.Inverted() * worldTransform : worldTransform;

	SetLocalTransform(localTransform);

	SetChildsAsDirty();

	C_Camera* cCamera = owner->GetComponent<C_Camera>();
	if (cCamera != nullptr)
	{
		cCamera->UpdateFrustumTransform();
	}

	C_RigidBody* c_rigidBody = owner->GetComponent<C_RigidBody>();
	if (c_rigidBody != nullptr)
	{
		if(App->gameState != GameState::PLAY)
			c_rigidBody->TransformMovesRigidBody(false);
	}

	//syncLocalToGlobal = false;
}

float4x4 C_Transform::GetLocalTransform() const
{
	return localTransform;
}

float4x4 C_Transform::GetWorldTransform()
{
	if (updateWorld)
		UpdateWorldTransform();
	
	return worldTransform;
}

float4x4* C_Transform::GetWorldTransformPtr()
{
	if (updateWorld)
		UpdateWorldTransform();
	
	return &worldTransform;
}

void C_Transform::SetLocalTransform(const float4x4& localTransform)
{
	this->localTransform = localTransform;

	localTransform.Decompose(localPosition, localRotation, localScale);
	localEulerRotation = localTransform.RotatePart().ToEulerXYZ();																// Use localRotation?

	updateWorld = true;
	
	//UpdateWorldTransform();
	//updateWorldTransform = true;
}

void C_Transform::SetWorldTransform(const float4x4& worldTransform)
{
	this->worldTransform = worldTransform;

	SyncLocalToWorld();
}

void C_Transform::ImportTransform(const float3& position, const Quat& rotation, const float3& scale)
{	
	localPosition	= position;
	localRotation	= rotation;
	localScale		= scale;
	
	localEulerRotation = localRotation.ToEulerXYZ();

	UpdateLocalTransform();
}

void C_Transform::ImportTransform(const Transform& transform)
{
	OPTICK_CATEGORY("Import Transform", Optick::Category::GameLogic);
	
	localPosition	= transform.position;
	localRotation	= transform.rotation;
	localScale		= transform.scale;

	localEulerRotation = localRotation.ToEulerXYZ();

	UpdateLocalTransform();
}

void C_Transform::SetChildsAsDirty()
{
	GameObject* owner = GetOwner();

	if (owner->childs.empty())
		return;

	for (uint i = 0; i < owner->childs.size(); ++i)
	{
		C_Transform* childTransform = owner->childs[i]->GetComponent<C_Transform>();

		if (childTransform != nullptr)
		{
			childTransform->updateWorld = true;
			childTransform->SetChildsAsDirty();
		}
	}
}

void C_Transform::RefreshTransformsChain()
{
	if (updateWorld)
		UpdateWorldTransform();

	GameObject* owner = GetOwner();

	if (owner->childs.empty())
		return;

	for (uint i = 0; i < owner->childs.size(); ++i)
	{
		owner->childs[i]->GetComponent<C_Transform>()->RefreshTransformsChain();
	}
}

// --- POSITION, ROTATION AND SCALE METHODS
// -- GET METHODS
float3 C_Transform::GetLocalPosition() const
{
	return localPosition;
}

Quat C_Transform::GetLocalRotation() const
{
	return localRotation;
}

float3 C_Transform::GetLocalEulerRotation() const
{
	return localEulerRotation;
	//return localEulerRotation * RADTODEG;
	//return localRotation.ToEulerXYZ() * RADTODEG;
}

float3 C_Transform::GetLocalScale() const
{
	return localScale;
}

float3 C_Transform::GetWorldPosition()
{
	if (updateWorld)
		UpdateWorldTransform();
	
	float3 p, s;
	Quat rotation;
	worldTransform.Decompose(p, rotation, s);

	//GetWorldTransform().Decompose(p, rotation, s);

	return p;
}

Quat C_Transform::GetWorldRotation()
{	
	if (updateWorld)
		UpdateWorldTransform();
	
	float3 p, s;
	Quat rotation;
	worldTransform.Decompose(p,rotation,s);

	return rotation;
}

float3 C_Transform::GetWorldEulerRotation()
{
	if (updateWorld)
		UpdateWorldTransform();
	
	float3 p, s;
	Quat rotation;

	worldTransform.Decompose(p, rotation, s);

	return rotation.ToEulerXYZ() * RADTODEG;
}

float3 C_Transform::GetWorldScale()
{
	if (updateWorld)
		UpdateWorldTransform();
	
	float3 p, s;
	Quat rotation;

	worldTransform.Decompose(p, rotation, s);

	return s;
}

// -- SET METHODS
void C_Transform::SetLocalPosition(const float3& newPosition)
{
	localPosition = newPosition;

	UpdateLocalTransform();
	//updateLocalTransform = true;																// Parameter modifications could be batched to re-calculate the local transform only once.
}																								// However, this would allow access to the dirty local transform before it can be updated.

void C_Transform::SetLocalRotation(const Quat& newRotation)
{
	localRotation		= newRotation;
	localEulerRotation	= localRotation.ToEulerXYZ();

	UpdateLocalTransform();
}

void C_Transform::SetLocalRotation(const float3& newRotation)
{
	localRotation		= Quat::FromEulerXYZ(newRotation.x, newRotation.y, newRotation.z);
	localEulerRotation	= newRotation;

	UpdateLocalTransform();
}

void C_Transform::SetLocalEulerRotation(const float3& newEulerRotation)
{
	localEulerRotation	= newEulerRotation * DEGTORAD;
	localRotation		= Quat::FromEulerXYZ(localEulerRotation.x, localEulerRotation.y, localEulerRotation.z);

	UpdateLocalTransform();
}

void C_Transform::SetLocalScale(const float3& newScale)
{
	localScale.x = (newScale.x != 0.0f) ? newScale.x : 0.001f;																	// Allow negative scale values?
	localScale.y = (newScale.y != 0.0f) ? newScale.y : 0.001f;
	localScale.z = (newScale.z != 0.0f) ? newScale.z : 0.001f;

	UpdateLocalTransform();
}

void C_Transform::SetWorldPosition(const float3& newPosition)
{
	worldTransform.SetTranslatePart(newPosition);

	SyncLocalToWorld();
}

void C_Transform::SetWorldRotation(const Quat& newRotation)
{
	worldTransform.SetRotatePart(newRotation);

	SyncLocalToWorld();
}

void C_Transform::SetWorldRotation(const float3& newRotation)
{
	worldTransform.SetRotatePart(Quat::FromEulerXYZ(newRotation.x, newRotation.y, newRotation.z));

	SyncLocalToWorld();
}

void C_Transform::SetWorldScale(const float3& newScale)
{
	worldTransform.Scale(worldTransform.GetScale().Neg());

	if (newScale.x == 0.0f || newScale.y == 0.0f || newScale.z == 0.0f)
	{
		worldTransform.Scale(float3(0.1f, 0.1f, 0.1f));
	}
	else
	{
		worldTransform.Scale(newScale);
	}

	SyncLocalToWorld();
}

// --- ADDING/SUBUTRACTING FROM POSITION, ROTATION AND SCALE
void C_Transform::Translate(const float3& velocity)
{
	localPosition += velocity;

	UpdateLocalTransform();
}

void C_Transform::Rotate(const Quat& angularVelocity)
{
	localRotation		= localRotation * angularVelocity;
	localEulerRotation	= localRotation.ToEulerXYZ();

	UpdateLocalTransform();
}

void C_Transform::Rotate(const float3& angularVelocity)
{
	localRotation		= localRotation * Quat::FromEulerXYZ(angularVelocity.x, angularVelocity.y, angularVelocity.z);
	localEulerRotation	+= angularVelocity;

	UpdateLocalTransform();
}

void C_Transform::Scale(const float3& expansionRate)
{
	localScale += expansionRate;

	UpdateLocalTransform();
}
