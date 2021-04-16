#pragma once
#include "Script.h"

#include "Log.h"

class VeeeeryLong : public Script
{
public:

	VeeeeryLong() {}
	virtual ~VeeeeryLong() {}

	virtual void Update() {}

	void BeFunny() { LOG("jajajajaja"); }
};