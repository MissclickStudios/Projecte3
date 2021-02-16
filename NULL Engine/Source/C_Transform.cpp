#include "JSONParser.h"

#include "Application.h"

#include "GameObject.h"
#include "C_Camera.h"

#include "C_Transform.h"

C_Transform::C_Transform(GameObject* owner) : Component(owner, COMPONENT_TYPE::TRANSFORM),
local_transform			(float4x4::identity),
world_transform			(float4x4::identity)
//sync_local_to_global	(false),
//update_world_transform	(false)
{	
	local_transform.Decompose(local_position, local_rotation, local_scale);

	local_euler_rotation = local_rotation.ToEulerXYZ();
}

C_Transform::~C_Transform()
{

}

bool C_Transform::Update()
{
	bool ret = true;

	/*if (update_world_transform)
	{
		UpdateWorldTransform();
	}*/

	/*if (sync_local_to_global)
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

	position.SetNumber(local_position.x);
	position.SetNumber(local_position.y);
	position.SetNumber(local_position.z);

	ParsonArray rotation = root.SetArray("LocalRotation");

	rotation.SetNumber(local_rotation.x);
	rotation.SetNumber(local_rotation.y);
	rotation.SetNumber(local_rotation.z);
	rotation.SetNumber(local_rotation.w);

	ParsonArray scale = root.SetArray("LocalScale");

	scale.SetNumber(local_scale.x);
	scale.SetNumber(local_scale.y);
	scale.SetNumber(local_scale.z);

	return ret;
}

bool C_Transform::LoadState(ParsonNode& root)
{
	bool ret = true;

	ParsonArray position = root.GetArray("LocalPosition");

	local_position.x = (float)position.GetNumber(0);
	local_position.y = (float)position.GetNumber(1);
	local_position.z = (float)position.GetNumber(2);

	ParsonArray rotation = root.GetArray("LocalRotation");

	local_rotation.x = (float)rotation.GetNumber(0);
	local_rotation.y = (float)rotation.GetNumber(1);
	local_rotation.z = (float)rotation.GetNumber(2);
	local_rotation.w = (float)rotation.GetNumber(3);

	ParsonArray scale = root.GetArray("LocalScale");

	local_scale.x = (float)scale.GetNumber(0);
	local_scale.y = (float)scale.GetNumber(1);
	local_scale.z = (float)scale.GetNumber(2);

	UpdateLocalTransform();

	return ret;
}

// ------ C_TRANSFORM METHODS ------
// --- TRANSFORMS/MATRICES
void C_Transform::UpdateLocalTransform()
{
	local_transform = float4x4::FromTRS(local_position, local_rotation, local_scale);

	UpdateWorldTransform();

	//sync_local_to_global = false;
	//update_world_transform = true;
	//UpdateWorldTransform();
}

void C_Transform::UpdateWorldTransform()
{
	GameObject* owner = GetOwner();

	if (owner->parent != nullptr)
	{
		world_transform = owner->parent->GetComponent<C_Transform>()->world_transform * local_transform;
	}
	else
	{
		world_transform = local_transform;
	}

	for (uint i = 0; i < owner->childs.size(); ++i)
	{
		owner->childs[i]->GetComponent<C_Transform>()->UpdateWorldTransform();
		//owner->childs[i]->GetTransformComponent()->update_world_transform = true;
	}

	C_Camera* c_camera = owner->GetComponent<C_Camera>();
	if (c_camera != nullptr)
	{
		c_camera->UpdateFrustumTransform();
	}

	//update_world_transform = false;
}

void C_Transform::SyncWorldToLocal()
{
	const GameObject* owner = GetOwner();

	if (owner->parent != nullptr)
	{
		world_transform = owner->parent->GetComponent<C_Transform>()->world_transform * local_transform;
	}
	else
	{
		world_transform = local_transform;
	}

	for (uint i = 0; i < owner->childs.size(); ++i)
	{
		owner->childs[i]->GetComponent<C_Transform>()->UpdateWorldTransform();
		//owner->childs[i]->GetTransformComponent()->update_world_transform = true;
	}
}

void C_Transform::SyncLocalToWorld()
{
	GameObject* owner = GetOwner();
	
	if (owner->parent != nullptr)
	{
		local_transform = owner->parent->GetComponent<C_Transform>()->world_transform.Inverted() * world_transform;
	}
	else
	{
		local_transform = world_transform;
	}

	local_position			= local_transform.TranslatePart();
	float3 euler			= local_transform.RotatePart().ToEulerXYZ();
	local_rotation			= Quat::FromEulerXYZ(euler.x, euler.y, euler.z);
	local_scale				= local_transform.GetScale();

	local_euler_rotation	= euler;

	for (uint i = 0; i < owner->childs.size(); ++i)
	{
		owner->childs[i]->GetComponent<C_Transform>()->UpdateWorldTransform();
		//owner->childs[i]->GetTransformComponent()->update_world_transform = true;
	}

	C_Camera* c_camera = owner->GetComponent<C_Camera>();
	if (c_camera != nullptr)
	{
		c_camera->UpdateFrustumTransform();
	}

	//sync_local_to_global = false;
}

float4x4 C_Transform::GetLocalTransform() const
{
	return local_transform;
}

float4x4 C_Transform::GetWorldTransform() const
{
	return world_transform;
}

void C_Transform::SetLocalTransform(const float4x4& local_transform)
{
	this->local_transform = local_transform;

	local_position	= local_transform.TranslatePart();
	float3 euler	= local_transform.RotatePart().ToEulerXYZ();
	local_rotation	= Quat::FromEulerXYZ(euler.x, euler.y, euler.z);
	local_scale		= local_transform.GetScale();

	local_euler_rotation = euler;

	UpdateWorldTransform();

	//update_world_transform = true;
}

void C_Transform::SetWorldTransform(const float4x4& world_transform)
{
	this->world_transform = world_transform;

	SyncLocalToWorld();
}

void C_Transform::ImportTransform(const float3& position, const Quat& rotation, const float3& scale)
{	
	local_position			= position;
	local_rotation			= rotation;
	local_scale				= scale;
	
	local_euler_rotation	= local_rotation.ToEulerXYZ();

	UpdateLocalTransform();
}

void C_Transform::ImportTransform(const Transform& transform)
{
	local_position	= transform.position;
	local_rotation	= transform.rotation;
	local_scale		= transform.scale;

	local_euler_rotation = local_rotation.ToEulerXYZ();

	UpdateLocalTransform();
}

// --- POSITION, ROTATION AND SCALE METHODS
// -- GET METHODS
float3 C_Transform::GetLocalPosition() const
{
	return local_position;
}

Quat C_Transform::GetLocalRotation() const
{
	return local_rotation;
}

float3 C_Transform::GetLocalEulerRotation() const
{
	return local_euler_rotation;
	//return local_euler_rotation * RADTODEG;
}

float3 C_Transform::GetLocalScale() const
{
	return local_scale;
}

float3 C_Transform::GetWorldPosition() const
{
	return world_transform.TranslatePart();
}

Quat C_Transform::GetWorldRotation() const
{
	return world_transform.RotatePart().ToQuat();
}

float3 C_Transform::GetWorldEulerRotation() const
{
	return world_transform.RotatePart().ToEulerXYZ();
}

float3 C_Transform::GetWorldScale() const
{
	return world_transform.GetScale();
}

// -- SET METHODS
void C_Transform::SetLocalPosition(const float3& new_position)
{
	local_position = new_position;

	UpdateLocalTransform();
	//update_local_transform = true;																// Parameter modifications could be batched to re-calculate the local transform only once.
}																									// However, this would allow access to the dirty local transform before it can be updated.

void C_Transform::SetLocalRotation(const Quat& new_rotation)
{
	local_rotation = new_rotation;

	local_euler_rotation = local_rotation.ToEulerXYZ();

	UpdateLocalTransform();
}

void C_Transform::SetLocalRotation(const float3& new_rotation)
{
	local_rotation			= Quat::FromEulerXYZ(new_rotation.x, new_rotation.y, new_rotation.z);
	local_euler_rotation	= new_rotation;

	UpdateLocalTransform();
}

void C_Transform::SetLocalEulerRotation(const float3& new_euler_rotation)
{
	float3 new_euler = new_euler_rotation * DEGTORAD;
	local_rotation = Quat::FromEulerXYZ(new_euler.x, new_euler.y, new_euler.z);
	local_euler_rotation = new_euler;

	UpdateLocalTransform();
}

void C_Transform::SetLocalScale(const float3& new_scale)
{
	if (new_scale.x == 0.0f || new_scale.y == 0.0f || new_scale.z == 0.0f)
	{
		float3 mod_scale = float3::one;
		
		mod_scale.x = (new_scale.x == 0.0f) ? 0.01f : new_scale.x;
		mod_scale.y = (new_scale.y == 0.0f) ? 0.01f : new_scale.y;
		mod_scale.z = (new_scale.z == 0.0f) ? 0.01f : new_scale.z;
		
		local_scale = mod_scale;
	}
	else
	{
		local_scale = new_scale;
	}

	UpdateLocalTransform();
}

void C_Transform::SetWorldPosition(const float3& new_position)
{
	world_transform.SetTranslatePart(new_position);

	SyncLocalToWorld();
}

void C_Transform::SetWorldRotation(const Quat& new_rotation)
{
	world_transform.SetRotatePart(new_rotation);

	SyncLocalToWorld();
}

void C_Transform::SetWorldRotation(const float3& new_rotation)
{
	world_transform.SetRotatePart(Quat::FromEulerXYZ(new_rotation.x, new_rotation.y, new_rotation.z));

	SyncLocalToWorld();
}

void C_Transform::SetWorldScale(const float3& new_scale)
{
	world_transform.Scale(world_transform.GetScale().Neg());

	if (new_scale.x == 0.0f || new_scale.y == 0.0f || new_scale.z == 0.0f)
	{
		world_transform.Scale(float3(0.1f, 0.1f, 0.1f));
	}
	else
	{
		world_transform.Scale(new_scale);
	}

	SyncLocalToWorld();
}

// --- ADDING/SUBUTRACTING FROM POSITION, ROTATION AND SCALE
void C_Transform::Translate(const float3& velocity)
{
	local_position += velocity;

	UpdateLocalTransform();
}

void C_Transform::Rotate(const Quat& angular_velocity)
{
	local_rotation = local_rotation * angular_velocity;

	local_euler_rotation += angular_velocity.ToEulerXYZ();

	UpdateLocalTransform();
}

void C_Transform::Rotate(const float3& angular_velocity)
{
	local_rotation = local_rotation * Quat::FromEulerXYZ(angular_velocity.x, angular_velocity.y, angular_velocity.z);		// Only rotate Y? Would only look left/right horizontally.
	local_euler_rotation += angular_velocity;
	
	//float3 local = local_rotation.ToEulerXYZ() * RADTODEG;																// If the above code is used to rotate objects, when used through
	//float3 euler = local_euler_rotation * RADTODEG;																		// the editor with euler angles, they will get out of sync with
	//LOG("[SCENE] Local Rotation: { %.3f, %.3f, %.3f }", local.x, local.y, local.z);										// the local_rotation quaternion at some rotations of the Y axis.
	//LOG("[SCENE] Euler Rotation: { %.3f, %.3f, %.3f }", euler.x, euler.y, euler.z);										// Ex: Y = 95.0f --> X = 180.0f, Z = -180.0f ==> Gets out of sync.

	UpdateLocalTransform();
}

void C_Transform::Scale(const float3& expansion_rate)
{
	local_scale += expansion_rate;

	UpdateLocalTransform();
}
