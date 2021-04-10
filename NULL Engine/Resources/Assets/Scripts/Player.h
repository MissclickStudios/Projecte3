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

enum class PlayerState;

class Weapon;
struct Projectile;

class SCRIPTS_API Player : public Script
{
public:

	Player();
	~Player();

	void Awake() override;
	void Update() override;
	void CleanUp()override;

	void TakeDamage(float damage);
	void BlasterStrongShots();
	void SniperFreezingShots();

	PlayerState state;

	// Character
	float speed = 20.0f;

	// Dash
	float dashSpeed = 100.0f;
	float dashingTime = 0.2f;
	float dashingColdown = 1.0f;

	// Health
	float health = 3.0f;
	float maxHealth = 3.0f;
	float invulnerability = 1.0f;

	// Weapons
	int weaponUsed = 1;

	// Blaster
	float blasterSpeed = 100.0f;
	float blasterRate = 0.25f;

	int blasterAmmo = 10;
	int blasterMaxAmmo = 10;
	float blasterReloadTime = 3.0f;

	bool blasterAutomatic = true;

	bool strongShots = false;

	Weapon* blaster = nullptr;
	Prefab blasterBullet;

	// Sniper
	float sniperSpeed = 200.0f;
	float sniperRate = 0.0f;

	int sniperAmmo = 1;
	int sniperMaxAmmo = 1;
	float sniperReloadTime = 5.0f;

	bool sniperAutomatic = false;

	bool freezingShots = false;

	Weapon* sniper = nullptr;
	Prefab sniperBullet;

private:

	void Animations();

	void Movement();
	void Move(C_RigidBody* rigidBody, int axisX, int axisY);
	void Dash(C_RigidBody* rigidBody, int axisX, int axisY);

	void Shooting();

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

	// Weapons
	Timer blasterReloadTimer;
	Timer sniperReloadTimer;

	R_Texture* ammoTex[11] = { 0 };
	bool storedAmmoTex = false;

	GameObject* ammoUi = nullptr;

	// Dash
	Timer dashTime;// Duration of the dash
	Timer dashColdown;

	// Health
	Timer invulnerabilityTimer;
	GameObject* hearts[3] = { nullptr, nullptr, nullptr };

	R_Texture* full = nullptr;
	R_Texture* half = nullptr;
	R_Texture* empty = nullptr;
};

SCRIPTS_FUNCTION Player* CreatePlayer();