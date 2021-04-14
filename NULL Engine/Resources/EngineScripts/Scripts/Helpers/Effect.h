#pragma once

#include "Timer.h"

enum class EffectType
{
	NONE,
	FROZEN
};

class Effect
{
public:

	Effect(EffectType type, float duration) : type(type), duration(duration) {}

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