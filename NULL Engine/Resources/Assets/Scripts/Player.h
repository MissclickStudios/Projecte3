#pragma once
#include "Script.h"
#include "ScriptMacros.h"
#include "Log.h"

#include "Timer.h"

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

	void Update() override;
	void CleanUp()override;

	Projectile* bullets[BULLET_AMOUNT];
	PlayerState state;

private:

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

	// Character
	float speed = 20.0f;
	float deceleration = 200.0f;
	float acceleration = 200.0f;

	bool isStepPlaying = false;
	Timer stepTimer;
	C_AudioSource* aSource = nullptr;
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

	R_Texture* full = nullptr;
	R_Texture* half = nullptr;
	R_Texture* empty = nullptr;
};

SCRIPTS_FUNCTION Player* CreatePlayer();