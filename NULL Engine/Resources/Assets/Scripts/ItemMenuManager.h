#pragma once
#include "Script.h"
#include "ScriptMacros.h"

#include <string>

class C_UI_Button;
class C_UI_Text;

class Player;
class GroundItem;

class SCRIPTS_API ItemMenuManager : public Script 
{
public:

	ItemMenuManager();
	~ItemMenuManager();

	void Start() override;
	void Update() override;

	void SetItem(GroundItem* item) { this->item = item; }

	std::string buyButtonName = "Buy Button";
	std::string nameTextName = "Name Text";
	std::string descriptionTextName = "Description Text";
	std::string priceTextName = "Price Text";

	std::string playerName = "Mandalorian";

private:

	C_UI_Button* buy = nullptr;

	C_UI_Text* name = nullptr;
	C_UI_Text* description = nullptr;
	C_UI_Text* price = nullptr;

	Player* player = nullptr;
	GroundItem* item = nullptr;
};

SCRIPTS_FUNCTION ItemMenuManager* CreateItemMenuManager()
{
	ItemMenuManager* script = new ItemMenuManager();

	INSPECTOR_STRING(script->buyButtonName);
	INSPECTOR_STRING(script->nameTextName);
	INSPECTOR_STRING(script->descriptionTextName);
	INSPECTOR_STRING(script->priceTextName);

	return script;
}
