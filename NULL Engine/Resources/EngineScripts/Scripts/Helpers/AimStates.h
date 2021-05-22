#pragma once

enum class AimState
{
	IDLE,
	ON_GUARD, // must use french accent
	AIMING,
	SHOOT_IN,
	SHOOT,
	RELOAD_IN,
	RELOAD,
	CHANGE_IN,
	CHANGE
};