#pragma once
#include <string>
#include "Script.h"
#include "ScriptMacros.h"


class GameObject;
class C_2DAnimator;
class C_UI_Text;
class C_Material;
class Player;
class R_Texture;

class SCRIPTS_API QuestManager : public Script {
public:
	QuestManager();
	~QuestManager();

	void Start() override;
	void Update() override;
	void CleanUp() override;

	std::string playerName = "Mandalorian";

	std::string titleTextName = "TitleText";
	std::string descriptionTextName = "DescriptionText";

	std::string questAnimationName = "Quest";

private:

	C_2DAnimator* QuestAnimation;

	GameObject* playerObject;
	Player* player;

	C_UI_Text* titleText = nullptr;
	C_UI_Text* descriptionText = nullptr;
};

SCRIPTS_FUNCTION QuestManager* CreateQuestManager() {
	QuestManager* script = new QuestManager();
	INSPECTOR_STRING(script->playerName);
	INSPECTOR_STRING(script->questAnimationName);
	INSPECTOR_STRING(script->titleTextName);
	INSPECTOR_STRING(script->descriptionTextName);
	return script;
}