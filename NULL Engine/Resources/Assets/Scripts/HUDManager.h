#pragma once
#include <vector>
#include <string>
#include "Script.h"
#include "ScriptMacros.h"
#include "Weapon.h"


class GameObject;
class C_2DAnimator;
class C_UI_Text;
class C_Material;
class Player;
class R_Texture;

class SCRIPTS_API HUDManager : public Script {
public:
	HUDManager();
	~HUDManager();

	void Start() override;
	void Update() override;
	void CleanUp() override;

	void ManageWeaponHUD();

	std::string mandoImageName = "Mando";
	std::string secondaryWeaponImageName = "SecodaryWeapon";
	std::string primaryWeaponImageName = "PrimaryWeapon";
	std::string dashImageName = "Dash";
	std::string creditsImageName = "Credits";
	std::string playerName = "Mandalorian";

	std::string creditsTextName = "CreditsText";
	std::string beskarTextName = "BeskarText";
	std::string ammoTextName = "AmmoText";


	std::string heart1Name = "Heart1";
	std::string heart2Name = "Heart2";
	std::string heart3Name = "Heart3";

	std::string weapon1Name = "pog1";
	std::string weapon2Name = "pog2";
	std::string weapon3Name = "pog3";
	std::string weapon4Name = "pog4";

	std::vector<std::string> disabledScenes;

private:
	C_2DAnimator* mandoImage;
	C_2DAnimator* secondaryWeaponImage;
	C_2DAnimator* primaryWeaponImage;
	C_2DAnimator* dashImage;
	C_2DAnimator* creditsImage;

	C_2DAnimator* heart1Image;
	C_2DAnimator* heart2Image;
	C_2DAnimator* heart3Image;

	GameObject* playerObject;
	Player* player;

	GameObject* weapon1;
	GameObject* weapon2;
	GameObject* weapon3;
	GameObject* weapon4;

	C_Canvas* hudCanvas = nullptr;

	C_UI_Text* creditsText = nullptr;
	C_UI_Text* beskarText = nullptr;
	C_UI_Text* ammoText = nullptr;

	C_Material* heart1;
	C_Material* heart2;
	C_Material* heart3;

	R_Texture* halfHeart;
	R_Texture* fullHeart;
	R_Texture* emptyHeart;

	bool hitAlready;

	bool health1;
	bool health2;
	bool health3;
	bool health4;
	bool health5;
	bool health6;

	WeaponType type;

	void ManageHeartImage(int hp);

	bool enabled = true;
};

SCRIPTS_FUNCTION HUDManager* CreateHUDManager() {
	HUDManager* script = new HUDManager();
	INSPECTOR_STRING(script->mandoImageName);
	INSPECTOR_STRING(script->secondaryWeaponImageName);
	INSPECTOR_STRING(script->primaryWeaponImageName);
	INSPECTOR_STRING(script->dashImageName);
	INSPECTOR_STRING(script->creditsImageName);
	INSPECTOR_STRING(script->playerName);

	INSPECTOR_STRING(script->creditsTextName);
	INSPECTOR_STRING(script->beskarTextName);
	INSPECTOR_STRING(script->ammoTextName);

	INSPECTOR_STRING(script->weapon1Name);
	INSPECTOR_STRING(script->weapon2Name);
	INSPECTOR_STRING(script->weapon3Name);
	INSPECTOR_STRING(script->weapon4Name);

	INSPECTOR_VECTOR_STRING(script->disabledScenes);
	return script;
}