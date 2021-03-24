#ifndef	__C_PLAYERCONTROLLER__
#define	__C_PLAYERCONTROLLER__

#include "Component.h"

#include "Timer.h"

#include "MathGeoLib/include/Math/float3.h"

#define BULLET_AMOUNT 10

class R_Texture;

class C_AudioSource;
class C_Animator;
class C_RigidBody;
class GameObject;


struct Bullet
{
	Bullet() : inUse(false), object(nullptr) {}
	Bullet(GameObject* object) : inUse(false), object(object) {}
	Bullet(bool inUse, GameObject* object) : inUse(inUse), object(object) {}

	bool inUse;
	GameObject* object;
};

enum class PlayerState
{
	IDLE,
	RUNNING,
	DASHING,
	SHOOTING
};

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
	const int CurrentAmmo() const { return ammo; }
	void SetCurrentAmmo(int amount) { ammo = amount; }
	const int MaxAmmo() const { return maxAmmo; }
	void SetMaxAmmo(int amount) { maxAmmo = amount; }

	const bool IsAutomatic() const { return automatic; }
	void SetAutomatic(bool enable) { automatic = enable; }

	const float DashSpeed() const { return dashSpeed; }
	void SetDashSpeed(float speed) { dashSpeed = speed; }
	const float DashTime() const { return dashingTime; }
	void SetDashTime(float time) { dashingTime = time; }
	const float DashColdown() const { return dashingColdown; }
	void SetDashColdown(float time) { dashingColdown = time; }

	Bullet* bullets[BULLET_AMOUNT];
	PlayerState state = PlayerState::IDLE;

private: 

	void Movement();
	void Move(C_RigidBody* rigidBody, int axisX, int axisY);
	void Dash(C_RigidBody* rigidBody, int axisX, int axisY);
	void Rotate();

	void Weapon();
	Bullet* CreateBullet(uint index);
	void FireBullet(float3 direction);
	void Reload();

	void StepSound();

	float2 MousePositionToWorldPosition(float mapPositionY = 0);

	void GetMovementVectorAxis(int &axisX, int &axisY);
	void GetAimVectorAxis(int &axisX, int &axisY);

	void HandleAmmo(int ammo);
	void HandleHp();

	// Character
	float speed = 20.0f;
	float deceleration = 200.0f;
	float acceleration = 200.0f;

	bool isStepPlaying = false;
	Timer stepTimer;
	C_AudioSource*aSource = nullptr;
	C_Animator* aAnimator = nullptr;
	bool playAnim = false;

	// Weapon
	float bulletSpeed = 100.0f;
	float fireRate = 0.25f;

	int ammo = 10;
	int maxAmmo = 10;

	bool automatic = true;

	Timer fireRateTimer;

	GameObject* bulletStorage = nullptr;

	R_Texture* ammoTex[11] = { 0 };
	bool storedAmmoTex = false;

	// Dash
	float dashSpeed = 100.0f;
	float dashingTime = 0.2f;
	float dashingColdown = 1.0f;

	float3 lastDirection = float3::zero;
	float3 lastAim = float3::zero;

	Timer dashTime;// Duration of the dash
	Timer dashColdown;

	// Hearts
	GameObject* hearts[3] = { nullptr, nullptr, nullptr };
	float heart = 3;

	GameObject* ammoUi = nullptr;

	R_Texture* full		= nullptr;
	R_Texture* half		= nullptr;
	R_Texture* empty	= nullptr;
};

#endif // !__C_PLAYERCONTROLLER__