#pragma once
#include "Weapon.h"

#include "ScriptMacros.h"

class SCRIPTS_API Blaster : public Weapon ALLOWED_INHERITANCE
{
public:

	Blaster();
	virtual ~Blaster();

	void SetUp() override;

	ShootState ShootLogic() override;
};

SCRIPTS_FUNCTION Blaster* CreateBlaster();