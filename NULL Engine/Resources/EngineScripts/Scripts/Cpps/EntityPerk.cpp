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

void EntityPerk::SetTypeInfo()
{
	type = (EffectType)typeInt;
	typeName = GetEffectName(type);
}

SCRIPTS_FUNCTION EntityPerk* CreateEntityPerk()
{
	EntityPerk* script = new EntityPerk();

	INSPECTOR_DRAGABLE_INT(script->price);

	INSPECTOR_TEXT(script->typeName);
	INSPECTOR_SLIDER_INT(script->typeInt, 0, (int)EffectType::EFFECTS_NUM - 1);
	INSPECTOR_DRAGABLE_FLOAT(script->duration);
	INSPECTOR_CHECKBOX_BOOL(script->permanent);

	return script;
}

// TODO: ADD INSPECTOR_TEXT() and TEST COLLECTIBLES