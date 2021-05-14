#pragma once
#include "ScriptMacros.h"

// REMEMBER TO ADD A NEW ENTRY ON THE GET PERK NAME IF YOU ADD A PERK (down below line 17ish)
enum class ENGINE_ENUM PerkType 
{
	NONE,
	DAMAGE_MODIFY,
	MAXAMMO_MODIFY,
	FIRERATE_MODIFY,
	RELOAD_TIME_MODIFY,
	BULLET_LIFETIME_MODIFY,
	FREEZE_BULLETS,
	STUN_BULLETS,
	PERKS_NUM
};
// This has to be the last entry
// NOT FOLLOWING THIS RULE WILL MESS UP THE PERKS SYSTEM AND PROBABLY CRASH THE GAME
// PERKS_NUM signifies the amount of existing perks

struct Perk
{
public:

	Perk(PerkType type, float amount, float duration) : amount(amount), duration(duration) { if (type != PerkType::PERKS_NUM) this->type = type; }
	~Perk() {}

	const PerkType Type() const { return type; }
	float Amount() { return amount; }
	float Duration() { return duration; }

private:

	PerkType type = PerkType::NONE;
	float amount = 0.0f;
	float duration = 0.0f;
};