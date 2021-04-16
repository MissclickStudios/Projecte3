#pragma once

#include "Timer.h"

enum class EffectType
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

	Effect(EffectType type, float duration) : duration(duration)
	{
		if (type != EffectType::EFFECTS_NUM) // This avoids the game crashig if someone requests a EFFECTS_NUM effect by accident
			this->type = type;
	}

	const bool IsActive() const
	{ 
		if (timer.ReadSec() >= duration) 
			return false; 
		return true; 
	}

	const EffectType Type() const { return type; }

private:

	EffectType type = EffectType::NONE;
	float duration = 0.0f;
	Timer timer;
};