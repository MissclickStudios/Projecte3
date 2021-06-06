#pragma once
#include <vector>
#include <string>
#include "Script.h"
#include "ScriptMacros.h"
#include "Weapon.h"

#define MAX_HEARTS 6


class GameObject;
class C_2DAnimator;
class C_UI_Text;
class C_UI_Image;
class Player;

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

	std::string weapon1Name = "pog1";
	std::string weapon2Name = "pog2";
	std::string weapon3Name = "pog3";
	std::string weapon4Name = "pog4";

	std::vector<std::string> disabledScenes;
	bool enabled = true;

private:
	C_2DAnimator* mandoImage;
	C_2DAnimator* secondaryWeaponImage;
	C_2DAnimator* primaryWeaponImage;
	C_2DAnimator* dashImage;
	C_2DAnimator* creditsImage;

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

	WeaponType type;
	
	int halfHeart[4]{ -3892, -658, 134, 124 };
	int fullHeart[4]{ -4019, -658, 134, 124 };
	int emptyHeart[4]{ -220, -531, 134, 124 };
	int noRender[4]{ -4150, -658, 134, 124 };

	bool hitAlready;
	bool setUpHealth = true;
	float playerHealth = 12.0f;
	float playerMaxHp = 12.0f;
	int currentHealthIndex = 0;
	C_2DAnimator* heartsAnimations[MAX_HEARTS]{};
	C_UI_Image* hearts[MAX_HEARTS]{};

	void SetUpHealth(float hp, int maxHp);
};

SCRIPTS_FUNCTION HUDManager* CreateHUDManager();