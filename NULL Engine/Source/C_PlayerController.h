#ifndef	__C_PLAYERCONTROLLER__
#define	__C_PLAYERCONTROLLER__

#include "Component.h"
#include "MathGeoLib/include/Math/float3.h"

class C_AudioSource;
class C_RigidBody;

class C_PlayerController : public Component
{
public:

	C_PlayerController(GameObject* owner);
	~C_PlayerController();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::PLAYER_CONTROLLER; }			// This is needed to be able to use templates for functions such as GetComponent<>();

	const float Speed() const { return speed; }
	void SetSpeed(float speed) { this->speed = speed; }

	const bool UsingAcceleration() const { return useAcceleration; }
	void UseAcceleration(bool enable) { useAcceleration = enable; }

	const float Acceleration() const { return acceleration; }
	void SetAcceleration(float force) { this->acceleration = force; }
	const float Deceleration() const { return deceleration; }
	//The deceleration is internaly inverted, so setting a negative deceleration will cause an acceleration
	void SetDeceleration(float force) { this->deceleration = force; }

	const float BulletSpeed() const { return bulletSpeed; }
	void SetBulletSpeed(float speed) { bulletSpeed = speed; }

	const bool IsCamera() const { return cameraMode; }
	void SetCameraMode(bool enable) { cameraMode = enable; }

private: 

	void MoveVelocity(C_RigidBody* rigidBody);
	void MoveAcceleration(C_RigidBody* rigidBody);
	void Rotate();

	float2 MousePositionToWorldPosition(float mapPositionY = 0);

	float speed = 30.0f;

	bool useAcceleration = false;

	float acceleration = 2.0f;
	float deceleration = 2.0f;

	float bulletSpeed = 100.0f;

	bool cameraMode = false;

	C_AudioSource* aSource;
};

#endif // !__C_PLAYERCONTROLLER__