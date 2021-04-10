#pragma once
#include "Script.h"
#include "ScriptMacros.h"

#include "MathGeoLib/include/Math/float2.h"

class GameObject;

class SCRIPTS_API Coin : public Script 
{
public:

	Coin();
	~Coin();

	void Awake() override;
	void Update() override;
	void CleanUp() override;

	void OnCollisionEnter(GameObject* object) override;

	int value = 50;

private:

	bool used = false;

	float2 direction = float2::zero;

	GameObject* mesh = nullptr;
};

SCRIPTS_FUNCTION Coin* CreateCoin() {
	Coin* script = new Coin();

	INSPECTOR_DRAGABLE_INT(script->value);

	return script;
}