#pragma once
#include "ScriptMacros.h"

#include "Test/LongParentTest.h"

class SCRIPTS_API Long : public VeeeeryLong ALLOWED_INHERITANCE
{
public:

	Long() {}
	virtual ~Long() {}

	void Update() { BeFunny(); }
};

SCRIPTS_FUNCTION Long* CreateLong()
{
	Long* script = new Long();
	return script;
}