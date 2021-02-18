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

	static inline ComponentType GetType() { return ComponentType::RIGIDBODY; }			// This is needed to be able to use templates for functions such as GetComponent<>();
	
	inline float			GetMass() { return mass; }
	inline void				SetMass(float mass)	{ this->mass = mass; update = true; }
	inline void				SetDensity(float density) { this->density = density; update = true; }

	inline void				UseGravity(bool enable) { useGravity = enable; update = true; }
	inline void				SetKinematic(bool enable) { isKinematic = enable; update = true; }

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

	inline void				FreezePositionX(bool enable) { freezePositionX = true; update = true; }
	inline void				FreezePositionY(bool enable) { freezePositionY = true; update = true; }
	inline void				FreezePositionZ(bool enable) { freezePositionZ = true; update = true; }
	inline void				FreezeRotationX(bool enable) { freezeRotationX = true; update = true; }
	inline void				FreezeRotationY(bool enable) { freezeRotationY = true; update = true; }
	inline void				FreezeRotationZ(bool enable) { freezeRotationZ = true; update = true; }

	inline const physx::PxRigidDynamic* const GetRigidBody() { return rigidBody; }

	bool IsStatic() { return isStatic; }
	void MakeStatic();
	void MakeDynamic();

private:

	void ApplyPhysicsChanges();

	void TransformMovesRigidBody(bool stopInertia);
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