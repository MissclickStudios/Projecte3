#include "EntityPerk.h"

#include "Player.h"

EntityPerk::EntityPerk() : Collectable()
{
}

EntityPerk::~EntityPerk()
{
}

void EntityPerk::Contact(Player* player)
{
	player->AddEffect(type, duration, permanent);
}

void EntityPerk::SetTypeInt()
{
	typeInt = (int)type;
}

SCRIPTS_FUNCTION EntityPerk* CreateEntityPerk()
{
	EntityPerk* script = new EntityPerk();

	INSPECTOR_SLIDER_INT(script->typeInt, 0, (int)EffectType::EFFECTS_NUM);
	INSPECTOR_DRAGABLE_FLOAT(script->duration);
	INSPECTOR_CHECKBOX_BOOL(script->permanent);

	return script;
}
