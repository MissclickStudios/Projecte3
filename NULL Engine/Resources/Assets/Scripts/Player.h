#pragma once
#include "Script.h"
#include "ScriptMacros.h"
#include "Log.h"

#include "Timer.h"
#include "Prefab.h"

#include "MathGeoLib/include/Math/float3.h"

#define BULLET_AMOUNT 10

class R_Texture;

class C_AudioSource;
class C_Animator;
class C_RigidBody;
class GameObject;

struct Projectile
{
	Projectile() : inUse(false), object(nullptr) {}
	Projectile(GameObject* object) : inUse(false), object(object) {}
	Projectile(bool inUse, GameObject* object) : inUse(inUse), object(object) {}

	bool inUse;
	GameObject* object;
};
enum class PlayerState;

class SCRIPTS_API Player : public Script
{
public:

	Player();
	~Player();

	void Awake() override;
	void Update() override;
	void CleanUp()override;

	Projectile* bullets[BULLET_AMOUNT];
	PlayerState state;

	// Character
	float speed = 20.0f;

	// Weapon
	float bulletSpeed = 100.0f;
	float fireRate = 0.25f;

	int ammo = 10;
	int maxAmmo = 10;

	bool automatic = true;

	Prefab bullet;

	// Dash
	float dashSpeed = 100.0f;
	float dashingTime = 0.2f;
	float dashingColdown = 1.0f;

	// Health
	float health = 3.0f;
	float maxHealth = 3.0f;

private:

	void Animations();

	void Movement();
	void Move(C_RigidBody* rigidBody, int axisX, int axisY);
	void Dash(C_RigidBody* rigidBody, int axisX, int axisY);
	void Rotate();

	void Weapon();
	Projectile* CreateProjectile(uint index);
	void FireBullet(float3 direction);
	void Reload();

	void StepSound();

	float2 MousePositionToWorldPosition(float mapPositionY = 0);

	void GetMovementVectorAxis(int& axisX, int& axisY);
	void GetAimVectorAxis(int& axisX, int& axisY);

	void HandleAmmo(int ammo);
	void HandleHp();

	float3 lastDirection = float3::zero;
	float3 lastAim = float3::zero;

	// Character
	bool isStepPlaying = false;
	Timer stepTimer;
	C_AudioSource* aSource = nullptr;
	C_Animator* aAnimator = nullptr;
	bool playAnim = false;

	// Weapon
	Timer fireRateTimer;

	GameObject* bulletStorage = nullptr;

	R_Texture* ammoTex[11] = { 0 };
	bool storedAmmoTex = false;

	GameObject* ammoUi = nullptr;

	// Dash
	Timer dashTime;// Duration of the dash
	Timer dashColdown;

	// Health
	GameObject* hearts[3] = { nullptr, nullptr, nullptr };

	R_Texture* full = nullptr;
	R_Texture* half = nullptr;
	R_Texture* empty = nullptr;
};

SCRIPTS_FUNCTION Player* CreatePlayer();