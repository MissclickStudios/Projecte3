#ifndef __C_TRANSFORM_H__
#define __C_TRANSFORM_H__

#include "MathGeoTransform.h"
#include "Component.h"

class ParsonNode;
class GameObject;

class C_Transform : public Component
{
public:
	C_Transform(GameObject* owner);
	~C_Transform();

	bool Update		() override;
	bool CleanUp	() override;

	bool SaveState	(ParsonNode& root) const override;
	bool LoadState	(ParsonNode& root) override;

	static inline COMPONENT_TYPE GetType()	{ return COMPONENT_TYPE::TRANSFORM; }	// This is needed to be able to use templeates for functions such as GetComponent<>();

public:																				// --- C_TRANSFORM METHODS ---
	void		SyncLocalToWorld();													// Recalcuates the local transform to sync it to world_transform.	Ex: new world --> SyncLocalToWorld();
	
	float4x4	GetLocalTransform		() const;									// Returns the local transform's 4x4 matrix. 
	float4x4	GetWorldTransform		() const;									// Returns the world transform's 4x4 matrix.

	void		SetLocalTransform		(const float4x4& local_transform);			// Sets the local transform's 4x4 matrix with the one passed as argument.
	void		SetWorldTransform		(const float4x4& world_transform);			// Sets the world transform's 4x4 matrix with the one passed as argument.

	void		ImportTransform			(const float3& position, const Quat& rotation, const float3& scale);
	void		ImportTransform			(const Transform& transform);

public:																				// --- GET/SET LOCAL AND WORLD POSITION, ROTATION AND SCALE
	float3		GetLocalPosition		() const;									// Returns the position vector of the local transform.
	Quat		GetLocalRotation		() const;									// Returns the rotation quaternion of the local transform.
	float3		GetLocalEulerRotation	() const;									// Returns the rotation quaternion of the local transform in Euler Angles.
	float3		GetLocalScale			() const;									// Returns the scale vector of the local transform.

	float3		GetWorldPosition		() const;									// Returns the position vector of the world transform
	Quat		GetWorldRotation		() const;									// Returns the rotation quaternion of the world transform. In Radiants.
	float3		GetWorldEulerRotation	() const;									// Returns the rotation quaternion of the world transform in Euler Angles. In Radians.
	float3		GetWorldScale			() const;									// Returns the scale vector of the world transform

	void		SetLocalPosition		(const float3& new_position);				// Sets local_position to the given vector.			Ex: local_position	= new_position;
	void		SetLocalRotation		(const Quat& new_rotation);					// Sets local_rotation to the given quaternion.		Ex: local_rotation	= new_rotation;
	void		SetLocalRotation		(const float3& new_rotation);				// Sets local_rotation to the given vector.			Ex: local_rotation	= new_rotation;
	void		SetLocalEulerRotation	(const float3& new_euler_rotation);			// Sets local_rotation to the given vector.			Ex: local_rotation	= new_rotation;
	void		SetLocalScale			(const float3& new_scale);					// Sets local_scale to the given vector.			Ex: local_scale		= new_scale;

	void		SetWorldPosition		(const float3& new_position);				// Sets world position to the given vector.			Ex: world_transform.SetTranslatePart(new_position);
	void		SetWorldRotation		(const Quat& new_rotation);					// Sets world rotation to the given quaternion.		Ex: world_transform = world_transform * new_rotation;
	void		SetWorldRotation		(const float3& new_rotation);				// Sets world rotation to the given vector.			Ex: world_transform.SetRotatePart(Quat(new_rotation));
	void		SetWorldScale			(const float3& new_scale);					// Sets world scale to the given vector.			Ex: world_transform.Scale(new_scale);

public:																				// --- TRANSFORM MOVEMENT, ROTATION AND SCALE OVER TIME.
	void		Translate				(const float3& velocity);					// Add/Sub to local_position the given vector.		Ex: local_position += velocity;
	void		Rotate					(const Quat& angular_velocity);				// Add/Sub to local_rotation the given quaternion.	Ex: local_rotation += angular_velocity;
	void		Rotate					(const float3& angular_velocity);			// Add/Sub to local_rotation the given vector.		Ex: local_rotation += angular_velocity;
	void		Scale					(const float3& expansion_rate);				// Add/Sub to local_scale the given vector			Ex: local_scale	   += expansion_rate;

private:
	void		UpdateLocalTransform	();											// Recalculates local_transform with the values of local_position, local_rotation and local_scale.
	void		UpdateWorldTransform	();											// Recalculates world_transform. world_transform: parent->world_transform * local_transform.
	void		SyncWorldToLocal		();											// Recalculates world_transform to sync it to local_transform.		Ex: new local --> SyncWorldToLocal();
	//void		SyncLocalToWorld		();											// Recalcuates the local transform to sync it to world_transform.	Ex: new world --> SyncLocalToWorld();

public:
	//bool		sync_local_to_global;												// Will be set to true if a parameter of the local_transform has been modified and it has not been applied.
	//bool		update_world_transform;												// Will be set to true if local_transform has been modified and world_transform has been not synced to it.
	//bool		local_is_dirty;														// Using Dirty flags + local_transform: Would be used to prevent the local_trfm from being used while dirty.

private:
	float4x4	local_transform;													// Will represent the position, rotation and scale of the transform in Local Space.
	float4x4	world_transform;													// Will represent the position, rotation and scale of the transform in World Space.
	
	float3		local_position;														// Position vector of the local transform.
	Quat		local_rotation;														// Rotation quaternion of the local transform.
	float3		local_scale;														// Scale vector of the local transform.

	float3		local_euler_rotation;												// Rotation vector in euler angles that will be used for display purposes. In Radians
};

#endif // !_C_TRANSFORM_H__