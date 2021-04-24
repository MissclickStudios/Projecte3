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

SCRIPTS_FUNCTION EntityPerk* CreateEntityPerk()
{
	EntityPerk* script = new EntityPerk();

	INSPECTOR_DRAGABLE_INT(script->price);

	INSPECTOR_ENUM(script->type, "EffectType", "EngineScripts/Scripts/Helpers/Effect.h");
	INSPECTOR_DRAGABLE_FLOAT(script->duration);
	INSPECTOR_CHECKBOX_BOOL(script->permanent);

	return script;
}

// TODO: ADD INSPECTOR_TEXT() and TEST COLLECTIBLES