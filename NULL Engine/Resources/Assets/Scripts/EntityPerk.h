#pragma once
#include "ScriptMacros.h"

#include "Collectable.h"
#include "Entity.h"

class SCRIPTS_API EntityPerk : public Collectable ALLOWED_INHERITANCE
{
public:

	EntityPerk();
	virtual ~EntityPerk();

	EffectType type = EffectType::NONE;
	float duration = 0.0f;
	bool permanent = true;

protected:

	void Contact(Player* player);
};

SCRIPTS_FUNCTION EntityPerk* CreateEntityPerk();