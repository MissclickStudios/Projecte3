#pragma once

#include "Timer.h"

enum class EffectType // REMEMBER TO ADD A NEW ENTRY ON THE GET EFFECT NAME IF YOU ADD AN EFFECT (down below line 58ish)
{
	NONE = 0,
	FROZEN,

	// This has to be the last entry
	// NOT FOLLOWING THIS RULE WILL MESS UP THE EFFECTS SYSTEM AND PROBABLY CRASH THE GAME
	EFFECTS_NUM // This signifies the amount of existing effects
};

class Effect
{
public:

	Effect(EffectType type, float duration, bool permanent) : duration(duration), permanent(permanent)
	{
		if (type != EffectType::EFFECTS_NUM) // This avoids the game crashig if someone requests a EFFECTS_NUM effect by accident
			this->type = type;

		if (permanent)
			timer.Stop();
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
	const bool Permanent() const { return permanent; }

private:

	EffectType type = EffectType::NONE;
	bool permanent = false;
	float duration = 0.0f;
	Timer timer;
};

static const char* GetEffectName(EffectType effect)
{
	switch (effect)
	{
	case EffectType::NONE:
		return "NONE";
	case EffectType::FROZEN:
		return "FROZEN";
	case EffectType::EFFECTS_NUM:
		return "ERROR";
	}
	return "U forgot to add the effect to string conversion... DUMBASS";
}