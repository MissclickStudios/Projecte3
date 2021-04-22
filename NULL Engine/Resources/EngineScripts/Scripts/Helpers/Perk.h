#pragma once

enum class Perk
{
	DAMAGE_UP,
	MAXAMMO_UP,
	FIRERATE_UP,
	FAST_RELOAD,
	FREEZE_BULLETS,

	// This has to be the last entry
	// NOT FOLLOWING THIS RULE WILL MESS UP THE PERKS SYSTEM AND PROBABLY CRASH THE GAME
	PERKS_NUM // This signifies the amount of existing perks
};