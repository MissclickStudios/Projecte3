#include "JSONParser.h"

#include "Application.h"

#include "GameObject.h"
#include "C_Camera.h"
#include "C_RigidBody.h"

#include "C_Transform.h"

C_Transform::C_Transform(GameObject* owner) : Component(owner, ComponentType::TRANSFORM),
localTransform(float4x4::identity),
worldTransform(float4x4::identity)
//syncLocalToGlobal(false),
//updateWorldTransform(false)
{	
	localTransform.Decompose(localPosition, localRotation, localScale);

	localEulerRotation = localRotation.ToEulerXYZ();
}

C_Transform::~C_Transform()
{

}

bool C_Transform::Update()
{
	bool ret = true;

	/*if (updateWorldTransform)
	{
		UpdateWorldTransform();
	}*/

	/*if (syncLocalToGlobal)
	{
		SyncLocalToWorld();
	}*/

	return ret;
}

bool C_Transform::CleanUp()
{
	bool ret = true;

	return ret;
}

bool C_Transform::SaveState(ParsonNode& root) const
{
	bool ret = true;

	root.SetNumber("Type", (uint)GetType());

	ParsonArray position = root.SetArray("LocalPosition");

	position.SetNumber(localPosition.x);
	position.SetNumber(localPosition.y);
	position.SetNumber(localPosition.z);

	ParsonArray rotation = root.SetArray("LocalRotation");

	rotation.SetNumber(localRotation.x);
	rotation.SetNumber(localRotation.y);
	rotation.SetNumber(localRotation.z);
	rotation.SetNumber(localRotation.w);

	ParsonArray scale = root.SetArray("LocalScale");

	scale.SetNumber(localScale.x);
	scale.SetNumber(localScale.y);
	scale.SetNumber(localScale.z);

	return ret;
}

bool C_Transform::LoadState(ParsonNode& root)
{
	bool ret = true;

	ParsonArray position = root.GetArray("LocalPosition");

	localPosition.x = (float)position.GetNumber(0);
	localPosition.y = (float)position.GetNumber(1);
	localPosition.z = (float)position.GetNumber(2);

	ParsonArray rotation = root.GetArray("LocalRotation");

	localRotation.x = (float)rotation.GetNumber(0);
	localRotation.y = (float)rotation.GetNumber(1);
	localRotation.z = (float)rotation.GetNumber(2);
	localRotation.w = (float)rotation.GetNumber(3);

	ParsonArray scale = root.GetArray("LocalScale");

	localScale.x = (float)scale.GetNumber(0);
	localScale.y = (float)scale.GetNumber(1);
	localScale.z = (float)scale.GetNumber(2);

	UpdateLocalTransform();

	return ret;
}

// ------ C_TRANSFORM METHODS ------
// --- TRANSFORMS/MATRICES
void C_Transform::UpdateLocalTransform()
{
	localTransform = float4x4::FromTRS(localPosition, localRotation, localScale);

	UpdateWorldTransform();

	//syncLocalToGlobal = false;
	//updateWorldTransform = true;
	//UpdateWorldTransform();
}

void C_Transform::UpdateWorldTransform()
{
	GameObject* owner = GetOwner();

	if (owner->parent != nullptr)
	{
		worldTransform = owner->parent->GetComponent<C_Transform>()->worldTransform * localTransform;
	}
	else
	{
		worldTransform = localTransform;
	}

	for (uint i = 0; i < owner->childs.size(); ++i)
	{
		owner->childs[i]->GetComponent<C_Transform>()->UpdateWorldTransform();
		//owner->childs[i]->GetTransformComponent()->updateWorldTransform = true;
	}

	C_Camera* c_camera = owner->GetComponent<C_Camera>();
	if (c_camera != nullptr)
	{
		c_camera->UpdateFrustumTransform();
	}

	//updateWorldTransform = false;
}

void C_Transform::SyncWorldToLocal()
{
	const GameObject* owner = GetOwner();

	if (owner->parent != nullptr)
	{
		worldTransform = owner->parent->GetComponent<C_Transform>()->worldTransform * localTransform;
	}
	else
	{
		worldTransform = localTransform;
	}

	for (uint i = 0; i < owner->childs.size(); ++i)
	{
		owner->childs[i]->GetComponent<C_Transform>()->UpdateWorldTransform();
		//owner->childs[i]->GetTransformComponent()->updateWorldTransform = true;
	}
}

void C_Transform::SyncLocalToWorld()
{
	GameObject* owner = GetOwner();
	
	if (owner->parent != nullptr)
	{
		localTransform = owner->parent->GetComponent<C_Transform>()->worldTransform.Inverted() * worldTransform;
	}
	else
	{
		localTransform = worldTransform;
	}

	localPosition = localTransform.TranslatePart();
	float3 euler = localTransform.RotatePart().ToEulerXYZ();
	localRotation = Quat::FromEulerXYZ(euler.x, euler.y, euler.z);
	localScale = localTransform.GetScale();

	localEulerRotation = euler;

	for (uint i = 0; i < owner->childs.size(); ++i)
	{
		owner->childs[i]->GetComponent<C_Transform>()->UpdateWorldTransform();
		//owner->childs[i]->GetTransformComponent()->updateWorldTransform = true;
	}

	C_Camera* cCamera = owner->GetComponent<C_Camera>();
	if (cCamera != nullptr)
	{
		cCamera->UpdateFrustumTransform();
	}

	C_RigidBody* c_rigidBody = owner->GetComponent<C_RigidBody>();
	if (c_rigidBody != nullptr)
	{
		c_rigidBody->TransformMovesRigidBody(false);
	}

	//syncLocalToGlobal = false;
}

float4x4 C_Transform::GetLocalTransform() const
{
	return localTransform;
}

float4x4 C_Transform::GetWorldTransform() const
{
	return worldTransform;
}

void C_Transform::SetLocalTransform(const float4x4& localTransform)
{
	this->localTransform = localTransform;

	localPosition = localTransform.TranslatePart();
	float3 euler = localTransform.RotatePart().ToEulerXYZ();
	localRotation = Quat::FromEulerXYZ(euler.x, euler.y, euler.z);
	localScale = localTransform.GetScale();

	localEulerRotation = euler;

	UpdateWorldTransform();

	//updateWorldTransform = true;
}

void C_Transform::SetWorldTransform(const float4x4& worldTransform)
{
	this->worldTransform = worldTransform;

	SyncLocalToWorld();
}

void C_Transform::ImportTransform(const float3& position, const Quat& rotation, const float3& scale)
{	
	localPosition = position;
	localRotation = rotation;
	localScale = scale;
	
	localEulerRotation = localRotation.ToEulerXYZ();

	UpdateLocalTransform();
}

void C_Transform::ImportTransform(const Transform& transform)
{
	localPosition = transform.position;
	localRotation = transform.rotation;
	localScale = transform.scale;

	localEulerRotation = localRotation.ToEulerXYZ();

	UpdateLocalTransform();
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
}

float3 C_Transform::GetLocalScale() const
{
	return localScale;
}

float3 C_Transform::GetWorldPosition() const
{
	return worldTransform.TranslatePart();
}

Quat C_Transform::GetWorldRotation() const
{
	return worldTransform.RotatePart().ToQuat();
}

float3 C_Transform::GetWorldEulerRotation() const
{
	return worldTransform.RotatePart().ToEulerXYZ();
}

float3 C_Transform::GetWorldScale() const
{
	return worldTransform.GetScale();
}

// -- SET METHODS
void C_Transform::SetLocalPosition(const float3& newPosition)
{
	localPosition = newPosition;

	UpdateLocalTransform();
	//updateLocalTransform = true;																// Parameter modifications could be batched to re-calculate the local transform only once.
}																									// However, this would allow access to the dirty local transform before it can be updated.

void C_Transform::SetLocalRotation(const Quat& newRotation)
{
	localRotation = newRotation;

	localEulerRotation = localRotation.ToEulerXYZ();

	UpdateLocalTransform();
}

void C_Transform::SetLocalRotation(const float3& newRotation)
{
	localRotation = Quat::FromEulerXYZ(newRotation.x, newRotation.y, newRotation.z);
	localEulerRotation = newRotation;

	UpdateLocalTransform();
}

void C_Transform::SetLocalEulerRotation(const float3& newEulerRotation)
{
	float3 new_euler = newEulerRotation * DEGTORAD;
	localRotation = Quat::FromEulerXYZ(new_euler.x, new_euler.y, new_euler.z);
	localEulerRotation = new_euler;

	UpdateLocalTransform();
}

void C_Transform::SetLocalScale(const float3& newScale)
{
	if (newScale.x == 0.0f || newScale.y == 0.0f || newScale.z == 0.0f)
	{
		float3 mod_scale = float3::one;
		
		mod_scale.x = (newScale.x == 0.0f) ? 0.01f : newScale.x;
		mod_scale.y = (newScale.y == 0.0f) ? 0.01f : newScale.y;
		mod_scale.z = (newScale.z == 0.0f) ? 0.01f : newScale.z;
		
		localScale = mod_scale;
	}
	else
	{
		localScale = newScale;
	}

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
	localRotation = localRotation * angularVelocity;

	localEulerRotation += angularVelocity.ToEulerXYZ();

	UpdateLocalTransform();
}

void C_Transform::Rotate(const float3& angularVelocity)
{
	localRotation = localRotation * Quat::FromEulerXYZ(angularVelocity.x, angularVelocity.y, angularVelocity.z);		// Only rotate Y? Would only look left/right horizontally.
	localEulerRotation += angularVelocity;
	
	//float3 local = localRotation.ToEulerXYZ() * RADTODEG;																// If the above code is used to rotate objects, when used through
	//float3 euler = localEulerRotation * RADTODEG;																		// the editor with euler angles, they will get out of sync with
	//LOG("[SCENE] Local Rotation: { %.3f, %.3f, %.3f }", local.x, local.y, local.z);										// the localRotation quaternion at some rotations of the Y axis.
	//LOG("[SCENE] Euler Rotation: { %.3f, %.3f, %.3f }", euler.x, euler.y, euler.z);										// Ex: Y = 95.0f --> X = 180.0f, Z = -180.0f ==> Gets out of sync.

	UpdateLocalTransform();
}

void C_Transform::Scale(const float3& expansionRate)
{
	localScale += expansionRate;

	UpdateLocalTransform();
}
