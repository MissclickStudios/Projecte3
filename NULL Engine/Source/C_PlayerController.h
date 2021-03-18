#ifndef	__C_PLAYERCONTROLLER__
#define	__C_PLAYERCONTROLLER__

#include "Component.h"

#include "Timer.h"

#include "MathGeoLib/include/Math/float3.h"

class C_AudioSource;
class C_RigidBody;
class GameObject;

class C_PlayerController : public Component
{
public:

	C_PlayerController(GameObject* owner);
	~C_PlayerController();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::PLAYER_CONTROLLER; }

	const float Speed() const { return speed; }
	void SetSpeed(float speed) { this->speed = speed; }

	const float Acceleration() const { return acceleration; }
	void SetAcceleration(float force) { this->acceleration = force; }
	const float Deceleration() const { return deceleration; }
	//The deceleration is internaly inverted, so setting a negative deceleration will cause an acceleration
	void SetDeceleration(float force) { this->deceleration = force; }

	const float BulletSpeed() const { return bulletSpeed; }
	void SetBulletSpeed(float speed) { bulletSpeed = speed; }
	const float FireRate() const { return fireRate; }
	void SetFireRate(float rate) { fireRate = rate; }

	const bool IsAutomatic() const { return automatic; }
	void SetAutomatic(bool enable) { automatic = enable; }

	const float DashSpeed() const { return dashSpeed; }
	void SetDashSpeed(float speed) { dashSpeed = speed; }
	const float DashTime() const { return dashingTime; }
	void SetDashTime(float time) { dashingTime = time; }
	const float DashColdown() const { return dashingColdown; }
	void SetDashColdown(float time) { dashingColdown = time; }

private: 

	void Movement();
	void Move(C_RigidBody* rigidBody, int axisX, int axisY);
	void Dash(C_RigidBody* rigidBody, int axisX, int axisY);
	void Rotate();

	void Weapon();
	void SpawnBullet(float3 direction);

	void StepSound(bool a, bool b, bool c, bool d );

	float2 MousePositionToWorldPosition(float mapPositionY = 0);

	void GetMovementVectorAxis(int &axisX, int &axisY);
	void GetAimVectorAxis(int &axisX, int &axisY);

	void HandleHp();

	// Character
	float speed = 20.0f;
	float deceleration = 200.0f;
	float acceleration = 200.0f;

	// Weapon
	float bulletSpeed = 100.0f;
	float fireRate = 0.25f;

	bool automatic = true;

	Timer fireRateTimer;

	// Dash
	float dashSpeed = 100.0f;
	float dashingTime = 0.2f;
	float dashingColdown = 1.0f;

	float3 lastDirection = float3::zero;

	Timer dashTime;// Duration of the dash
	Timer dashColdown;

	GameObject* hearts[3] = { nullptr, nullptr, nullptr };
	float heart = 3;
};

#endif // !__C_PLAYERCONTROLLER__