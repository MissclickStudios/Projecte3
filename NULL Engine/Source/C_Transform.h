#ifndef __C_TRANSFORM_H__
#define __C_TRANSFORM_H__

#include "MathGeoTransform.h"
#include "Component.h"

class ParsonNode;
class GameObject;

class MISSCLICK_API C_Transform : public Component
{
public:
	C_Transform(GameObject* owner);
	~C_Transform();

	bool Update		() override;
	bool CleanUp	() override;

	bool SaveState	(ParsonNode& root) const override;
	bool LoadState	(ParsonNode& root) override;

	static inline ComponentType GetType()	{ return ComponentType::TRANSFORM; }	// This is needed to be able to use templates for functions such as GetComponent<>();

public:																				// --- C_TRANSFORM METHODS ---
	void		SyncLocalToWorld		();											// Recalcuates the local transform to sync it to worldTransform.	Ex: new world --> SyncLocalToWorld();
	
	float4x4	GetLocalTransform		() const;									// Returns the local transform's 4x4 matrix. 
	float4x4	GetWorldTransform		();											// Returns the world transform's 4x4 matrix.
	float4x4*	GetWorldTransformPtr	();											// Needed for renderers. The transform will be updated even if it is already in a renderer.

	void		SetLocalTransform		(const float4x4& localTransform);			// Sets the local transform's 4x4 matrix with the one passed as argument.
	void		SetWorldTransform		(const float4x4& worldTransform);			// Sets the world transform's 4x4 matrix with the one passed as argument.

	void		ImportTransform			(const float3& position, const Quat& rotation, const float3& scale);
	void		ImportTransform			(const Transform& transform);

	void		SetChildsAsDirty		();											// Will run along the transformations chain and set all children of this component's owner as dirty.
	void		RefreshTransformsChain	();											// Will update the worldTransform of this component as well as those of it's owner's children (if needed).

	float GetDistanceTo(float3 _position);

public:																				// --- GET/SET LOCAL AND WORLD POSITION, ROTATION AND SCALE
	float3		GetLocalPosition		() const;									// Returns the position vector of the local transform.
	Quat		GetLocalRotation		() const;									// Returns the rotation quaternion of the local transform.
	float3		GetLocalEulerRotation	() const;									// Returns the rotation quaternion of the local transform in Euler Angles.
	float3		GetLocalScale			() const;									// Returns the scale vector of the local transform.

	float3		GetWorldPosition		();											// Returns the position vector of the world transform
	Quat		GetWorldRotation		();											// Returns the rotation quaternion of the world transform. In Radiants.
	float3		GetWorldEulerRotation	();											// Returns the rotation quaternion of the world transform in Euler Angles. In Radians.
	float3		GetWorldScale			();											// Returns the scale vector of the world transform

	void		SetLocalPosition		(const float3& newPosition);				// Sets localPosition to the given vector.			Ex: localPosition	= new_position;
	void		SetLocalRotation		(const Quat& newRotation);					// Sets localRotation to the given quaternion.		Ex: localRotation	= new_rotation;
	void		SetLocalRotation		(const float3& newRotation);				// Sets localRotation to the given vector.			Ex: localRotation	= new_rotation;
	void		SetLocalEulerRotation	(const float3& newEulerRotation);			// Sets localRotation to the given vector.			Ex: localRotation	= new_rotation;
	void		SetLocalScale			(const float3& newScale);					// Sets localScale to the given vector.				Ex: localScale		= new_scale;
		 
	void		SetWorldPosition		(const float3& newPosition);				// Sets world position to the given vector.			Ex: worldTransform.SetTranslatePart(new_position);
	void		SetWorldRotation		(const Quat& newRotation);					// Sets world rotation to the given quaternion.		Ex: worldTransform = worldTransform * new_rotation;
	void		SetWorldRotation		(const float3& newRotation);				// Sets world rotation to the given vector.			Ex: worldTransform.SetRotatePart(Quat(new_rotation));
	void		SetWorldScale			(const float3& newScale);					// Sets world scale to the given vector.			Ex: worldTransform.Scale(new_scale);

public:																				// --- TRANSFORM MOVEMENT, ROTATION AND SCALE OVER TIME.
	void		Translate				(const float3& velocity);					// Add/Sub to localPosition the given vector.		Ex: localPosition += velocity;
	void		Rotate					(const Quat& angularVelocity);				// Add/Sub to localRotation the given quaternion.	Ex: localRotation += angular_velocity;
	void		Rotate					(const float3& angularVelocity);			// Add/Sub to localRotation the given vector.		Ex: localRotation += angular_velocity;
	void		Scale					(const float3& expansionRate);				// Add/Sub to localScale the given vector			Ex: localScale	  += expansion_rate;

private:
	void		UpdateLocalTransform	();											// Recalculates localTransform with the values of localPosition, localRotation and localScale.
	void		UpdateWorldTransform	();											// Recalculates worldTransform. worldTransform: parent->worldTransform * localTransform.
	void		SyncWorldToLocal		();											// Recalculates worldTransform to sync it to localTransform.		Ex: new local --> SyncWorldToLocal();
	//void		SyncLocalToWorld		();											// Recalcuates the local transform to sync it to worldTransform.	Ex: new world --> SyncLocalToWorld();

public:
	bool updateWorld;																// Will be set to true if localTransform has been modified and worldTransform has been not synced to it.

private:
	float4x4	localTransform;														// Will represent the position, rotation and scale of the transform in Local Space.
	float4x4	worldTransform;														// Will represent the position, rotation and scale of the transform in World Space.
	
	float3		localPosition;														// Position vector of the local transform.
	Quat		localRotation;														// Rotation quaternion of the local transform.
	float3		localScale;															// Scale vector of the local transform.

	float3		localEulerRotation;													// Rotation vector in euler angles that will be used for display purposes. In Radians
	float3		worldEulerRotation;													// Rotation vector in euler angles that will be used for display purposes. In Radians
};

#endif // !_C_TRANSFORM_H__