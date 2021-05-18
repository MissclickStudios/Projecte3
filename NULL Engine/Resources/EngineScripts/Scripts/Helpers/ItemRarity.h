#pragma once
#include "Color.h"

#define COMMON_COLOR Color(0.4f, 1.0f, 0.4f)
#define RARE_COLOR Color(0.4f, 1.0f, 1.0f)
#define EPIC_COLOR Color(1.0f, 0.4f, 1.0f)
#define UNIQUE_COLOR Color(1.0f, 1.0f, 0.4f)

enum class ItemRarity
{
	COMMON,
	RARE,
	EPIC,
	UNIQUE
};
