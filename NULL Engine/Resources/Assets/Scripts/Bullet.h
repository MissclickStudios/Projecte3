#pragma once
#include "Script.h"
#include "ScriptMacros.h"

#include "Timer.h"

class GameObject;

class SCRIPTS_API Bullet : public Script 
{
public:

	Bullet();
	~Bullet();

	void Update() override;
	void CleanUp()override;

	void OnEnable() override;

	void OnCollisionEnter(GameObject* object) override;

	void SetShooter(GameObject* shooter, uint index);
	void StartAutodestructTimer() { autodestructTimer.Start(); }

	float damage = 1.0f;

	float autodestruct = 3.0f;

	// Effects
	bool freeze = false;
	bool strong = false;

private:

	std::string target;

	bool hit = false;
	GameObject* shooter = nullptr;
	uint index = 0;

	Timer autodestructTimer;
};

SCRIPTS_FUNCTION Bullet* CreateBullet();