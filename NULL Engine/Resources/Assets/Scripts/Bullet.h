#pragma once
#include "Script.h"
#include "ScriptMacros.h"

#include "Timer.h"

class GameObject;
class Player;

class SCRIPTS_API Bullet : public Script 
{
public:

	Bullet();
	~Bullet();

	void Update() override;
	void CleanUp()override;

	void OnEnable() override;

	void OnCollisionEnter(GameObject* object) override;

	void SetShooter(Player* shooter, uint index);
	void StartAutodestructTimer() { autodestructTimer.Start(); }

	float damage = 1.0f;

	float autodestruct = 3.0f;

private:

	bool hit = false;
	Player* shooter = nullptr;
	uint index = 0;

	Timer autodestructTimer;
};

SCRIPTS_FUNCTION Bullet* CreateBullet();