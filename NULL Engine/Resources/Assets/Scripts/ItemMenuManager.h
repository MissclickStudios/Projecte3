#pragma once
#include "Script.h"
#include "ScriptMacros.h"

#include "Prefab.h"

#include "MathGeoLib/include/Math/float3.h"

#include <string>

class C_Canvas;
class C_UI_Button;
class C_UI_Text;
class C_UI_Image;
class C_Material;
class R_Texture;

class Player;
class GroundItem;

class SCRIPTS_API ItemMenuManager : public Script 
{
public:

	ItemMenuManager();
	~ItemMenuManager();

	void Start() override;
	void Update() override;
	void CleanUp()override;

	const GroundItem* GetItem() const { return item; }
	void SetItem(GroundItem* item);

	void SetWeapon(Prefab weapon, float3 position, std::string name, std::string description);

	std::string canvasName = "Item Menu";
	std::string buyButtonName = "Buy Button";
	std::string nameTextName = "Name Text";
	std::string descriptionTextName = "Description Text";
	std::string priceTextName = "Price Text";
	std::string rarityTextName = "Rarity Text";
	std::string itemImageName = "ItemImage";

	std::string playerName = "Mandalorian";

	C_Canvas* canvas = nullptr;

	float closeMenuThreshold = 12.0f;

private:

	C_UI_Button* buyButton = nullptr;

	C_UI_Text* nameText = nullptr;
	C_UI_Text* descriptionText = nullptr;
	C_UI_Text* priceText = nullptr;
	C_UI_Text* rarityText = nullptr;
	C_UI_Image* itemImage = nullptr;
	C_Material* itemMaterial = nullptr;
	R_Texture* currentItemTexture = nullptr;
	R_Texture* atlasTexture = nullptr;

	Player* player = nullptr;
	GroundItem* item = nullptr;

	Prefab weapon;
	float3 weaponPosition = float3::zero;
};

SCRIPTS_FUNCTION ItemMenuManager* CreateItemMenuManager();
