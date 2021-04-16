#pragma once
#include "Script.h"

#include "Log.h"

class A : public Script
{
public:

	A() {}
	virtual ~A() {}

	virtual void Update() {}

	void BeFunny() { LOG("hahahahah"); }
};