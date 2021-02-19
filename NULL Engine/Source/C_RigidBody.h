#ifndef __C_RIGIDBODY_H__
#define __C_RIGIDBODY_H__

#include "Component.h"

#include "MathGeoLib/include/Math/float3.h"
#include "PhysX_3.4/Include/PxPhysicsAPI.h"

class C_RigidBody : public Component
{
public:

	C_RigidBody(GameObject* owner);
	~C_RigidBody();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	void SetIsActive(bool setTo) override;

	static inline ComponentType GetType() { return ComponentType::RIGIDBODY; }			// This is needed to be able to use templates for functions such as GetComponent<>();
	
	inline float			GetMass() { return mass; }
	inline void				SetMass(float mass)	{ this->mass = mass; update = true; }
	inline float			GetDensity() { return density; }
	inline void				SetDensity(float density) { this->density = density; update = true; }

	inline bool				UsingGravity() { return useGravity; }
	inline void				UseGravity(bool enable) { useGravity = enable; update = true; }
	inline bool				IsKinematic() { return isKinematic; }
	inline void				SetKinematic(bool enable) { isKinematic = enable; update = true; }

	float					GetSpeed() { return linearVel.Length(); }
	inline float3			GetLinearVelocity() { return linearVel; }
	inline float3			GetAngularVelocity() { return angularVel; }
	inline void				SetLinearVelocity(float3 vel) { linearVel = vel; update = true; }
	inline void				SetAngularVelocity(float3 vel) { angularVel = vel; update = true; }

	inline void				SetAngularDamping(float value) { angularDamping = value; update = true; }
	inline void				SetLinearDamping(float value) { linearDamping = value; update = true; }
	inline physx::PxReal	GetAngularDamping() { return angularDamping; }
	inline physx::PxReal	GetLinearDamping() { return linearDamping; }

	inline void				AddForce(physx::PxVec3 force, physx::PxForceMode::Enum mode) { if (rigidBody) rigidBody->addForce(force, mode); }
	inline void				AddTorque(physx::PxVec3 force, physx::PxForceMode::Enum mode) { if (rigidBody)rigidBody->addTorque(force, mode); }

	inline void				FrozenPositions(bool& x, bool& y, bool& z) { x = freezePositionX; y = freezePositionY; z = freezePositionZ; }
	inline void				FreezePositionX(bool enable) { freezePositionX = enable; update = true; }
	inline void				FreezePositionY(bool enable) { freezePositionY = enable; update = true; }
	inline void				FreezePositionZ(bool enable) { freezePositionZ = enable; update = true; }

	inline void				FrozenRotations(bool& x, bool& y, bool& z) { x = freezeRotationX; y = freezeRotationY; z = freezeRotationZ; }
	inline void				FreezeRotationX(bool enable) { freezeRotationX = enable; update = true; }
	inline void				FreezeRotationY(bool enable) { freezeRotationY = enable; update = true; }
	inline void				FreezeRotationZ(bool enable) { freezeRotationZ = enable; update = true; }

	bool					IsSleeping() { return rigidBody->isSleeping(); }

	inline const physx::PxRigidDynamic* const GetRigidBody() { return rigidBody; }

	bool IsStatic() { return isStatic; }
	void MakeStatic();
	void MakeDynamic();

	void TransformMovesRigidBody(bool stopInertia);

private:

	void ApplyPhysicsChanges();

	void RigidBodyMovesTransform();

	physx::PxRigidDynamic* rigidBody = nullptr;
	bool update = false;

	float mass = 10.f;
	float density = 1.f;
	float linearDamping = 0.f;
	float angularDamping = 0.f;
	float massBuffer = 0.0f;
	float densityBuffer = 0.0f;

	float3 linearVel = float3::zero;
	float3 angularVel = float3::zero;

	bool useGravity = true;
	bool isKinematic = false;
	bool freezePositionX = false;
	bool freezePositionY = false;
	bool freezePositionZ = false;
	bool freezeRotationX = false;
	bool freezeRotationY = false;
	bool freezeRotationZ = false;

	bool isStatic = false;
};

#endif // !__C_RIGIDBODY_H__