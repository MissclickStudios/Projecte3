#pragma once
#include "Script.h"
#include "ScriptMacros.h"

#include "Timer.h"

class GameObject;

class SCRIPTS_API Bullet : public Script {
public:
	Bullet();
	~Bullet();

	void Update() override;
	void CleanUp()override;

	void OnCollisionEnter();

	void SetShooter(GameObject* shooter, uint index);
	void StartAutodestructTimer() { autodestructTimer.Start(); }

private:

	bool hit = false;
	GameObject* shooter = nullptr;
	uint index = 0;

	float autodestruct = 3.0f;
	Timer autodestructTimer;
};

SCRIPTS_FUNCTION Bullet* CreateBullet() {
	Bullet* script = new Bullet();
	return script;
}