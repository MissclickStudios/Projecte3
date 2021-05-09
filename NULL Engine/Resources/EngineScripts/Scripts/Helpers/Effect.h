#pragma once
#include "ScriptMacros.h"

#include "Timer.h"

// REMEMBER TO ADD A NEW ENTRY ON THE GET EFFECT NAME IF YOU ADD AN EFFECT (down below line 58ish)
enum class ENGINE_ENUM EffectType
{
	NONE = 0,
	FROZEN,
	HEAL,
	STUN,
	KNOCKUP,
	KNOCKBACK,
	EFFECTS_NUM
};
// This has to be the last entry
// NOT FOLLOWING THIS RULE WILL MESS UP THE EFFECTS SYSTEM AND PROBABLY CRASH THE GAME
// EFFECTS_NUM signifies the amount of existing effects

class Effect
{
public:

	Effect(EffectType type, float duration, bool permanent, void* data) : duration(duration), permanent(permanent), data(data)
	{
		if (type != EffectType::EFFECTS_NUM) // This avoids the game crashig if someone requests a EFFECTS_NUM effect by accident
			this->type = type;

		if (permanent)
			timer.Stop();
	}

	~Effect()
	{
		if (data != nullptr)
		{
			delete data;
		}
	}

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

	const EffectType Type() const { return type; }

	const float Duration() const { return duration; }
	const float RemainingDuration() const { return Duration() - timer.ReadSec(); }

	const bool Permanent() const { return permanent; }

	const void* Data() const { return data; }

private:

	EffectType type = EffectType::NONE;
	bool permanent = false;
	float duration = 0.0f;
	Timer timer;

	void* data = nullptr; // used to store any extra info about the effect, scary shit doe
};