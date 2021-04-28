#pragma once
#include "ScriptMacros.h"

// REMEMBER TO ADD A NEW ENTRY ON THE GET PERK NAME IF YOU ADD A PERK (down below line 17ish)
enum class ENGINE_ENUM Perk 
{
	NONE,
	DAMAGE_UP,
	MAXAMMO_UP,
	FIRERATE_UP,
	FAST_RELOAD,
	FREEZE_BULLETS,
	PERKS_NUM
};
// This has to be the last entry
// NOT FOLLOWING THIS RULE WILL MESS UP THE PERKS SYSTEM AND PROBABLY CRASH THE GAME
// PERKS_NUM signifies the amount of existing perks