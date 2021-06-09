#pragma once
#include "ScriptMacros.h"

#include "Entity.h"

#include "Timer.h"
#include "Prefab.h"

class C_BoxCollider;
class C_ParticleSystem;

enum class GroguState
{
	IDLE,
	MOVE,
	ATTACK_IN,
	ATTACK
};

class SCRIPTS_API Grogu : public Entity ALLOWED_INHERITANCE
{
public:

	Grogu();
	virtual ~Grogu();

	void SetUp() override;
	void Behavior() override;
	void CleanUp() override;

	void OnTriggerRepeat(GameObject* object) override;

	void SaveState(ParsonNode& groguNode);
	void LoadState(ParsonNode& groguNode);

	std::string playerName = "Mandalorian";
	std::string meshName = "Grogu Mesh";

	Prefab ParticlePrefab;

	float minDistanceToMando = 6.0f;
	float maxDistanceToMando = 11.0f;

	float power = 0.0f;
	float cooldown = 0.0f;
	float particleTime = 0.0f;

	bool deflectBullets = false;

private:

	GroguState state = GroguState::IDLE;

	// Logic
	void Move();
	void Rotate();
	void Levitate();
	
	// Actions
	void Attack();
	void GetDistance();

	GameObject* player = nullptr;
	GameObject* mesh = nullptr;

	C_BoxCollider* attackCollider = nullptr;
	C_ParticleSystem* particles = nullptr;

	float3 direction = float3::zero;
	float distance = 0.0f;

	Timer cooldownTimer;
	Timer particleTimer;

	bool stopAttack = true;
};

SCRIPTS_FUNCTION Grogu* CreateGrogu();