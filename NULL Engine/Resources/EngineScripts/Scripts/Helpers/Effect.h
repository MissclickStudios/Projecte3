#pragma once
#include "ScriptMacros.h"

#include "Timer.h"

#include "MathGeoLib/include/Math/float3.h"

// REMEMBER TO ADD A NEW ENTRY ON THE GET EFFECT NAME IF YOU ADD AN EFFECT (down below line 58ish)
enum class ENGINE_ENUM EffectType
{
	NONE = 0,
	FROZEN,
	HEAL,
	MAX_HEALTH_MODIFY,
	SPEED_MODIFY,
	STUN,
	KNOCKBACK,
	ELECTROCUTE,
	BOSS_PIERCING,
	PRICE_MODIFY,
	COOLDOWN_MODIFY,
	EFFECTS_NUM
};
// This has to be the last entry
// NOT FOLLOWING THIS RULE WILL MESS UP THE EFFECTS SYSTEM AND PROBABLY CRASH THE GAME
// EFFECTS_NUM signifies the amount of existing effects

class Effect
{
public:

	Effect(EffectType type, float duration, bool permanent, float power = 0.0f, float chance = 0.0f, float3 direction = float3::zero, bool start = true)
		: duration(duration), permanent(permanent), power(power), chance(chance), direction(direction), start(start)
	{
		if (type != EffectType::EFFECTS_NUM) // This avoids the game crashig if someone requests a EFFECTS_NUM effect by accident
			this->type = type;

		if (permanent)
			timer.Stop();
	}

	~Effect() {}

	const bool IsActive() const
	{ 
		if (permanent)
			return true;
		else if (!timer.IsActive())
			return false;

		if (timer.ReadSec() >= duration) 
			return false; 
		return true; 
	}

	void End()
	{
		permanent = false; // Remove a possible permanent status and stop the timer
		timer.Stop();	   // so that the next .IsActive() returns a guaranteed false
	}

	void Pause()
	{
		timer.Pause();
	}

	void Resume()
	{
		timer.Resume();
	}

	const EffectType Type() const { return type; }

	const float Duration() const { return duration; }
	const float RemainingDuration() const { return Duration() - timer.ReadSec(); }

	const bool Permanent() const { return permanent; }

	const float Power() { return power; }
	const float Chance() { return chance; }
	const float3 Direction() { return direction; }

	bool start = true;

private:

	EffectType type = EffectType::NONE;
	bool permanent = false;
	float duration = 0.0f;
	Timer timer;

	float power = 0.0f;
	float chance = 0.0f;
	float3 direction = float3::zero;
};