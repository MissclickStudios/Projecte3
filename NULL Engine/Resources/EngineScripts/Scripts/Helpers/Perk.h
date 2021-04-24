#pragma once

enum class Perk // REMEMBER TO ADD A NEW ENTRY ON THE GET PERK NAME IF YOU ADD A PERK (down below line 17ish)
{
	NONE,
	DAMAGE_UP,
	MAXAMMO_UP,
	FIRERATE_UP,
	FAST_RELOAD,
	FREEZE_BULLETS,

	// This has to be the last entry
	// NOT FOLLOWING THIS RULE WILL MESS UP THE PERKS SYSTEM AND PROBABLY CRASH THE GAME
	PERKS_NUM // This signifies the amount of existing perks
};

static const char* GetPerkName(Perk perk)
{
	switch (perk)
	{
	case Perk::NONE:
		return "NONE";
	case Perk::DAMAGE_UP:
		return "DAMAGE_UP";
	case Perk::MAXAMMO_UP:
		return "MAXAMMO_UP";
	case Perk::FIRERATE_UP:
		return "FIRERATE_UP";
	case Perk::FAST_RELOAD:
		return "FAST_RELOAD";
	case Perk::FREEZE_BULLETS:
		return "FREEZE_BULLETS";
	case Perk::PERKS_NUM:
		return "ERROR";
	default:
		return "U forgot to add the perk to string conversion... DUMBASS";
	}
}